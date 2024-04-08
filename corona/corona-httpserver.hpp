
#ifndef CORONA_HTTP_SERVER
#define CORONA_HTTP_SERVER

namespace corona
{
    
	using http_handler_function = std::function<service_transaction<http_response>(http_request _request)>;
	using http_handler_db_function = std::function<database_transaction<json>(json _request)>;
	using http_handler_db_method_function = std::function<database_method_transaction<json>(json _request)>;

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

		virtual void execute_request(PHTTP_REQUEST _request) = 0;
	};

	class http_server_task_launcher 
	{
		class http_server_task_job : public job
		{
		public:
			std::coroutine_handle<> handle;
			http_server_task_launcher*		metask;
			http_server_base*		mebase;
			buffer					buff;

			http_server_task_job() : job()
			{
				handle = {};
				metask = nullptr;
				buff = buffer(16384);
			}

			job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
			{
				job_notify jn;

				debug_functions&& std::cout << "http_server_task_job: receiving IO results " << GetCurrentThreadId() << std::endl;

				if (metask) 
				{
					debug_functions&& std::cout << "http_server_task_job: bytes transferred: " << _bytesTransferred << std::endl;
					metask->bytes_transferred = _bytesTransferred;
					metask->success = _success;

					if (_success) 
					{
						PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
						mebase->execute_request(prequest);
					}

					if (handle) {
						handle.resume();
						handle.destroy();
					}
				}
				else 
				{
					if (handle) {
						handle.resume();
						handle.destroy();
					}
				}

				debug_functions&& std::cout << "http_server_task_job: end:" << GetCurrentThreadId() << std::endl;

				jn.shouldDelete = true;
				return jn;
			}

			void read_http_request(HANDLE requestQueue, HTTP_REQUEST_ID requestId, ULONG flags )
			{
				PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
				HttpReceiveHttpRequest(requestQueue, requestId, flags, prequest, buff.get_size(), nullptr, &container.ovp );
			}
		};

	public:

		int bytes_transferred;
		bool success;

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

			http_server_task_launcher get_return_object() 
			{
				debug_functions&& std::cout << "http_server_task::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				http_server_task_launcher fbr(promise_coro);
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

		http_server_task_launcher()
		{
			bytes_transferred = 0;
			success = false;
			coro = nullptr;
		}

		http_server_task_launcher(const http_server_task_launcher& _src)
		{
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
		}

		http_server_task_launcher(http_server_task_launcher&& _src)
		{
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
		}

		http_server_task_launcher(std::coroutine_handle<promise_type> _coro)
		{
			bytes_transferred = 0;
			success = false;
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

			LARGE_INTEGER li;
			li.QuadPart = 0;
			http_server_task_job* frj = new http_server_task_job();
			frj->container.ovp.Offset = li.LowPart;
			frj->container.ovp.OffsetHigh = li.HighPart;
			frj->handle = coro;
			frj->metask = this;
			frj->read_http_request(requestQueue, requestId, flags);
		}

		virtual ~http_server_task_launcher()
		{
			;
		}

		bool await_ready() 
		{
			return false;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			coro = handle;
			debug_functions&& std::cout << "http_server_task: suspend" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
		}

		// and this, let's us await for io while we are awaiting our http response
		void await_suspend(std::coroutine_handle<user_transaction<bool>::promise_type> handle)
		{
			debug_functions&& std::cout << "http_server_task::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "http_server_task: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		void await_suspend(std::coroutine_handle<user_transaction<json>::promise_type> handle)
		{
			debug_functions&& std::cout << "http_server_task::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "http_server_task: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		void await_resume()
		{
			debug_functions&& std::cout << "http_server_task: resume" << " " << ::GetCurrentThreadId() << std::endl;;
		}
	};

	using handler_key = std::tuple<std::string, HTTP_VERB>;

	class http_server : public http_server_base
	{
		HTTP_SERVER_SESSION_ID session_id;
		HTTP_URL_GROUP_ID group_id;

		std::map<handler_key, std::shared_ptr<http_handler_container>> api_handlers;

		HANDLE      request_queue;

	public:

		http_server()
		{
			;
		}

		void put_handler(HTTP_VERB _verb, std::string _url, http_handler_function _handler)
		{
			std::shared_ptr<http_handler_container> hhc = std::make_shared<http_handler_container>();

			hhc->func = _handler;
			hhc->url = _url;

			auto key = std::tie( _url, _verb);

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

		int read_body(buffer_assembler& _buff, PHTTP_REQUEST _request)
		{
			int count = _request->EntityChunkCount;

			for (int i = 0; i < count; i++) 
			{
				auto &pchunk = _request->pEntityChunks[i];
				switch (pchunk.DataChunkType)
				{
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFileHandle:
					break;
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFragmentCache:
					break;
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromFragmentCacheEx:
					break;
				case HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromMemory:
					char* t = _buff.append(pchunk.FromMemory.BufferLength);
					std::copy((PUCHAR)pchunk.FromMemory.pBuffer, (PUCHAR)pchunk.FromMemory.pBuffer + pchunk.FromMemory.BufferLength, t);
					break;
				}
			}

			return count;
		}

		std::string get_header_string(PHTTP_REQUEST _request, HTTP_HEADER_ID _header_id)
		{
			std::string header_string;
			auto& headers = _request->Headers;
			auto& header = headers.KnownHeaders[_header_id];
			if (header.RawValueLength) 
			{
				buffer buff(header.RawValueLength);
				char *temp = buff.get_ptr();
				strncpy_s(temp, header.RawValueLength, header.pRawValue, header.RawValueLength);
				header_string = temp;
			}
			return header_string;
		}

		virtual void execute_request(PHTTP_REQUEST _request)
		{
			istring<2048> absPath;
			istring<2048> queryString;
			std::string sQueryString;
			std::string sabsPath;

			absPath.copy(_request->CookedUrl.pAbsPath, _request->CookedUrl.AbsPathLength);
			sabsPath = absPath.c_str();

			if (_request->CookedUrl.pQueryString) {
				queryString.copy(_request->CookedUrl.pQueryString, _request->CookedUrl.QueryStringLength);
				sQueryString = queryString.c_str();
			}
			
			std::string authorization = get_header_string(_request, HTTP_HEADER_ID::HttpHeaderAuthorization);
			std::string content_type = get_header_string(_request, HTTP_HEADER_ID::HttpHeaderContentType);

			auto key = std::tie(sabsPath, _request->Verb);

			http_request request = {};

			request.path = sabsPath;

			buffer_assembler body_builder;

			if (_request->EntityChunkCount)
			{
				read_body(body_builder, _request);
			}

			switch (_request->Verb) {
			case HttpVerbUnparsed:
			case HttpVerbUnknown:
			case HttpVerbInvalid:
				request.http_method = "Invalid";
				break;
			case HttpVerbOPTIONS:
				request.http_method = "OPTIONS";
				break;
			case HttpVerbGET:
				request.http_method = "GET";
				break;
			case HttpVerbHEAD:
				request.http_method = "HEAD";
				break;
			case HttpVerbPOST:
				request.http_method = "POST";
				break;
			case HttpVerbPUT:
				request.http_method = "PUT";
				break;
			case HttpVerbDELETE:
				request.http_method = "DELETE";
				break;
			case HttpVerbTRACE:
				request.http_method = "TRACE";
				break;
			case HttpVerbCONNECT:
				request.http_method = "CONNECT";
				break;
			case HttpVerbTRACK:
				request.http_method = "TRACK";
				break;
			case HttpVerbMOVE:
				request.http_method = "MOVE";
				break;
			case HttpVerbCOPY:
				request.http_method = "COPY";
				break;
			case HttpVerbPROPFIND:
				request.http_method = "PROPFIND";
				break;
			case HttpVerbPROPPATCH:
				request.http_method = "PROPPATCH";
				break;
			case HttpVerbMKCOL:
				request.http_method = "MKCOL";
				break;
			case HttpVerbLOCK:
				request.http_method = "LOCK";
				break;
			case HttpVerbUNLOCK:
				request.http_method = "UNLOCK";
				break;
			case HttpVerbSEARCH:
				request.http_method = "SEARCH";
				break;
			}

			if (_request->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)
			{
				buffer buff(1 << 20);
				DWORD bytes_returned;

				DWORD receive_result = HttpReceiveRequestEntityBody(
					request_queue,
					_request->RequestId,
					0,
					buff.get_ptr(),
					buff.get_size(),
					&bytes_returned,
					nullptr
				);

				while (receive_result != ERROR_HANDLE_EOF)
				{
					if (receive_result == NO_ERROR)
					{
						read_body(body_builder, (PHTTP_REQUEST)buff.get_ptr());

						receive_result = HttpReceiveRequestEntityBody(
							request_queue,
							_request->RequestId,
							0,
							buff.get_ptr(),
							buff.get_size(),
							&bytes_returned,
							nullptr
						);
					}
				}
			}

			request.body = body_builder.consolidate();

			bool unhandled = true;

			try 
			{
				for (auto handler : api_handlers) 
				{
					std::string key = std::get<0>(handler.first);
					HTTP_VERB verb = std::get<1>(handler.first);
					if (sabsPath.starts_with(key) && verb == _request->Verb)
					{
						int szk = key.size();
						int szp = sabsPath.size();
						std::string remaining_path = sabsPath.substr(szk, szp-szk);
						request.rest_path = split(remaining_path, '/');
						service_transaction<http_response> trans = handler.second->func(request);
						http_response trans_response = trans.wait();
						send_response(_request->RequestId, trans_response.http_status_code, "", trans_response.content_type, trans_response.response_body.get_ptr(), trans_response.response_body.get_size());
						unhandled = false;
					}
				}
			}
			catch (std::exception exc)
			{
				send_response(_request->RequestId, 500, "Server error, something blew up.", "text/plain", exc.what());
			}
			
			if (unhandled)
			{
				send_response(_request->RequestId, 404, "I looked, but, didn't find anything. Probably should stop by the store and grab another.", "text/plain", "Well, that was a bust.");
				std::cout << "HTTP Unhandled request:" << request.http_method << " " << request.path << std::endl;
			}
		}

		DWORD send_response(
			HTTP_REQUEST_ID _request_id, 
			int _status_code,
			std::string _reason, 
			std::string _content_type, 
			char* _buffer, 
			DWORD _buffer_length_bytes)
		{
			HTTP_RESPONSE		response = {};
			HTTP_DATA_CHUNK		dataChunk = {};
			DWORD				result;
			DWORD				bytesSent;

			istring<1025> reason = _reason;
			istring<1025> content_type = _content_type;

			response.StatusCode = _status_code;
			response.ReasonLength = reason.size();
			response.pReason = reason.c_str();

			if (_buffer) 
			{
				dataChunk.FromMemory.pBuffer = _buffer;
				dataChunk.FromMemory.BufferLength = _buffer_length_bytes;
				dataChunk.DataChunkType = HTTP_DATA_CHUNK_TYPE::HttpDataChunkFromMemory;

				response.EntityChunkCount = 1;
				response.pEntityChunks = &dataChunk;
			}

			response.Headers.KnownHeaders[HTTP_HEADER_ID::HttpHeaderContentType].pRawValue = content_type.c_str();
			response.Headers.KnownHeaders[HTTP_HEADER_ID::HttpHeaderContentType].RawValueLength = content_type.size();

			// 
			// Because the entity body is sent in one call, it is not
			// required to specify the Content-Length.
			//

			result = HttpSendHttpResponse(
				request_queue,           // ReqQueueHandle
				_request_id, // Request ID
				0,                   // Flags
				&response,           // HTTP response
				NULL,                // pReserved1
				&bytesSent,          // bytes sent  (OPTIONAL)
				NULL,                // pReserved2  (must be NULL)
				0,                   // Reserved3   (must be 0)
				NULL,                // LPOVERLAPPED(OPTIONAL)
				NULL                 // pReserved4  (must be NULL)
			);

			return result;
		}

		DWORD send_response(
			HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			json _response_json)
		{
			std::string response_data = _response_json.to_json();
			DWORD result = send_response(_request_id,
				_status_code,
				_reason,
				"application/json",
				response_data);
			return result;
		}

		DWORD send_response(HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			std::string _content_type,
			std::string _response_string)
		{
			char* buffer = (char *)_response_string.c_str();
			DWORD buffer_length_bytes = _response_string.size();
			DWORD response = send_response(_request_id,
				_status_code,
				_reason,
				_content_type,
				buffer,
				buffer_length_bytes);
			return response;
		}

		http_server_task_launcher next_request()
		{
			http_server_task_launcher launcher;

			launcher.read_request(request_queue, HTTP_NULL_ID, 0);

			return launcher;
		}
	};

}

#endif
