
#ifndef CORONA_HTTP_SERVER
#define CORONA_HTTP_SERVER

#include "corona-httpclient.hpp"
#include "http.h"

namespace corona
{
    
	using http_handler_function = std::function<http_response(http_request _request)>;

	class http_handler_container
	{
	public:
		http_handler_function func;
		std::string url;
		std::wstring urlw;
	};

	class http_server_base
	{
	public:

		virtual void excecute_request(PHTTP_REQUEST _request) = 0;
	};

	class http_server_task 
	{
		class http_server_task_job : public job
		{
		public:
			std::coroutine_handle<> handle;
			HANDLE					event;
			http_server_task*		metask;
			http_server_base*		mebase;
			buffer					buff;

			http_server_task_job() : job()
			{
				handle = {};
				event = {};
				metask = nullptr;
				buff = buffer(16384);
			}

			job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
			{
				job_notify jn;

				debug_functions&& std::cout << "http_server_task_job: receiving IO results " << GetCurrentThreadId() << std::endl;

				if (metask) {
					debug_functions&& std::cout << "http_server_task_job: bytes transferred: " << _bytesTransferred << std::endl;
					metask->bytes_transferred = _bytesTransferred;
					metask->success = _success;
					/*					LARGE_INTEGER li;
										li.HighPart = container.ovp.OffsetHigh;
										li.LowPart = container.ovp.Offset;
										metask->location = li.QuadPart;
										*/
					if (_success) 
					{
						PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
						mebase->excecute_request(prequest);
					}

					if (handle) {
						handle.resume();
						handle.destroy();
					}

					jn.setSignal(metask->hevent);
				}
				else 
				{
					if (handle) {
						handle.resume();
						handle.destroy();
					}
				}

				debug_functions&& std::cout << "http_server_task_job: end:" << GetCurrentThreadId() << std::endl;

				jn.shouldDelete = false;
				return jn;
			}

			void read_http_request(HANDLE requestQueue, HTTP_REQUEST_ID requestId, ULONG flags )
			{
				PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
				HttpReceiveHttpRequest(requestQueue, requestId, flags, prequest, buff.get_size(), nullptr, &container.ovp );
			}
		};

	public:

		HANDLE file;
		HANDLE hevent;
		int64_t location;
		int bytes_transferred;
		bool success;
		http_server_task_job frj;

		HANDLE requestQueue;
		HTTP_REQUEST_ID requestId;
		ULONG flags;

		struct promise_type
		{
			std::coroutine_handle<promise_type> promise_coro;

			promise_type()
			{
				debug_functions&& std::cout << "http_server_task::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			http_server_task get_return_object() 
			{
				debug_functions&& std::cout << "http_server_task::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				http_server_task fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_void() 
			{
				debug_functions&& std::cout << "http_server_task::promise return_void:" << " " << this << GetCurrentThreadId() << std::endl;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "http_server_task::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		http_server_task()
		{
			file = nullptr;
			location = 0;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			coro = nullptr;
		}

		http_server_task(const http_server_task& _src)
		{
			file = _src.file;
			location = _src.location;
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			frj = _src.frj;
		}

		http_server_task(http_server_task&& _src)
		{
			file = _src.file;
			location = _src.location;
			hevent = _src.hevent;
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
			_src.hevent = nullptr;
			frj = std::move(_src.frj);
		}

		http_server_task(std::coroutine_handle<promise_type> _coro)
		{
			location = 0;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			coro = _coro;
		}

		void read_request(HANDLE _requestQueue,
						  HTTP_REQUEST_ID _requestId,
						  ULONG _flags)
		{
			requestQueue = _requestQueue;
			requestId = _requestId;
			flags = _flags;
		}

		void initiate()
		{
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = 0;
			frj.container.ovp.Offset = li.LowPart;
			frj.container.ovp.OffsetHigh = li.HighPart;
			frj.handle = coro;
			frj.metask = this;
			frj.read_http_request(requestQueue, requestId, flags);
		}

		virtual ~http_server_task()
		{
			::CloseHandle(hevent);
		}

		bool await_ready() 
		{
			return false;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "http_server_task: suspend file_batch_result" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
		}

		void await_resume()
		{
			;
		}
	};

	using handler_key = std::tuple<std::wstring, HTTP_VERB>;

	class http_server : public http_server_base
	{
		HTTP_SERVER_SESSION_ID session_id;
		HTTP_URL_GROUP_ID group_id;

		std::map<handler_key, std::shared_ptr<http_handler_container>> api_handlers;

		std::string listen_domain;
		std::string listen_url;
		int			listen_port;

		HANDLE      request_queue;

	public:


		http_server(std::string _domain, std::string _url, int _port)
		{
			;
		}

		void put_handler(HTTP_VERB _verb, std::string _url, http_handler_function _handler)
		{
			std::shared_ptr<http_handler_container> hhc = std::make_shared<http_handler_container>();

			hhc->func = _handler;
			hhc->url = _url;

			iwstring<2048> url;
			url = _url;
			hhc->urlw = url.c_str_w();

			auto key = std::tie( hhc->urlw, _verb);

			api_handlers.insert_or_assign(key, hhc);
		}

		void start()
		{
			//
			// Create a Request Queue Handle
			//
			ULONG retCode = HttpInitialize(
				HTTPAPI_VERSION_2,
				HTTP_INITIALIZE_SERVER | HTTP_INITIALIZE_CONFIG,    // Flags
				NULL                       // Reserved
			);

			if (retCode != NO_ERROR)
			{
				std::string s = std::format("HttpInitialize failed with {} \n", retCode);
				throw std::logic_error(s);
			}

			//
			// Create a Request Queue Handle
			//
			retCode = HttpCreateRequestQueue(
				HTTPAPI_VERSION_2,
				L"Corona Http Server",
				NULL,
				0,
				&request_queue
			);

			if (retCode != NO_ERROR)
			{
				std::string s = std::format("HttpCreateHttpHandle failed with {} \n", retCode);
				throw std::logic_error(s);
			}

			retCode = HttpCreateServerSession(HTTPAPI_VERSION_2, &session_id, 0);

			//
			// Create a Request Group Handle
			//
			retCode = HttpCreateUrlGroup(
				session_id,
				&group_id,
				NULL
			);

			if (retCode != NO_ERROR)
			{
				std::string s = std::format("HttpCreateUrlGroup failed with {} \n", retCode);
				throw std::logic_error(s);
			}

			//
			// Bind the server binding
			//
			HTTP_BINDING_INFO serverBinding = {};

			serverBinding.Flags.Present = 1;
			serverBinding.RequestQueueHandle = request_queue;

			retCode = HttpSetUrlGroupProperty(
				group_id,
				HTTP_SERVER_PROPERTY::HttpServerBindingProperty,
				&serverBinding,
				sizeof(serverBinding)
			);

			if (retCode != NO_ERROR)
			{
				std::string s = std::format("Could not set server binding failed with {} \n", retCode);
				throw std::logic_error(s);
			}

			//
			// Listen on the server
			//
			HTTP_LISTEN_ENDPOINT_INFO serverEndpointInfo = {};

			serverEndpointInfo.Flags.Present = 1;
			serverEndpointInfo.EnableSharing = 1;

			retCode = HttpSetUrlGroupProperty(
				group_id,
				HTTP_SERVER_PROPERTY::HttpServerListenEndpointProperty,
				&serverEndpointInfo,
				sizeof(serverEndpointInfo)
			);

			if (retCode != NO_ERROR)
			{
				std::string s = std::format("Could not set server binding failed with {} \n", retCode);
				throw std::logic_error(s);
			}

			// Add the handlers
			for (auto h : api_handlers)
			{
				HTTP_URL_CONTEXT context = {};
				HttpAddUrlToUrlGroup(group_id, h.second->urlw.c_str(), context, 0);
			}

			global_job_queue->listen(request_queue);
		}

		virtual void excecute_request(PHTTP_REQUEST _request)
		{
			iwstring<2048> absPath;
			absPath.copy(_request->CookedUrl.pAbsPath, _request->CookedUrl.AbsPathLength);
			std::wstring path = absPath.c_str();

			auto key = std::tie(path, verb);
		}

	};

}

#endif
