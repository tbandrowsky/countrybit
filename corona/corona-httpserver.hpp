/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License


About this File
Multithreaded http server.  Uses Windows I/O completion ports.
Conceptually, you define the server, load it up with handlers for various 
paths, and off you go.

Notes

For Future Consideration
*/

#ifndef CORONA_HTTP_SERVER
#define CORONA_HTTP_SERVER


namespace corona
{


	const int debug_http_servers = 1;

	class http_server_base
	{
	public:
		virtual void execute_request(PHTTP_REQUEST _request) = 0;
		virtual DWORD send_response(
			HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			std::string _content_type,
			char* _buffer,
			DWORD _buffer_length_bytes) = 0;

		virtual DWORD send_response(
			HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			json _response_json) = 0;

		virtual DWORD send_response(HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			std::string _content_type,
			std::string _response_string) = 0;

		virtual void next_request() = 0;
	};

	class http_action_request
	{
		http_server_base* server;
		HTTP_REQUEST_ID request_id;

		friend class http_server;

	public:

		http_request request;

		std::string authorization;
		std::string content_type;

		DWORD send_response(int _status_code,
			std::string _reason,
			std::string _content_type,
			char* _buffer,
			DWORD _buffer_length_bytes);

		DWORD send_response(int _status_code,
			std::string _reason,
			json _response_json);

		DWORD send_response(int _status_code,
			std::string _reason,
			std::string _content_type,
			std::string _response_string);
	};

	using http_handler_function = std::function<void(http_action_request)>;

	class http_handler_method
	{
	public:
		http_handler_function func;
		HTTP_VERB method;
	};

	class http_handler_list
	{
	public:
		std::string url;
		std::vector<http_handler_method> functions;
	};

	class http_command_request
	{
	public:

		http_server_base*	server;
		HANDLE				requestQueue;
		HTTP_REQUEST_ID		requestId;
		ULONG				flags;
	};

	class http_command : public job
	{
	public:
		buffer						buff;
		http_command_request		request;
		int							bytes_transferred;
		bool						success;

		http_command(http_command_request& _request) : job()
		{
			request = _request;
			buff = buffer(16384);
		}
		
		virtual bool queued(job_queue *_calling_queue) override
		{
			bytes_transferred = 0;
			success = false;

			LARGE_INTEGER li;
			li.QuadPart = 0;
			container.ovp.Offset = li.LowPart;
			container.ovp.OffsetHigh = li.HighPart;
			PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
			DWORD error = HttpReceiveHttpRequest(request.requestQueue, request.requestId, request.flags, prequest, buff.get_size(), nullptr, &container.ovp);

			if (error == ERROR_IO_PENDING) {
				success = true;
			}
			return success;
		}

		job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			timer tx;
			date_time thetime = date_time::now();

			bytes_transferred = _bytesTransferred;
			success = _success;
			try {
				if (_success and bytes_transferred > 0)
				{
					system_monitoring_interface::global_mon->log_function_start("http request", "start", thetime, __FILE__, __LINE__);
					PHTTP_REQUEST prequest = (PHTTP_REQUEST)buff.get_ptr();
					request.server->execute_request(prequest);
					system_monitoring_interface::global_mon->log_function_stop("http request", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_warning(exc.what());
			}

			try
			{
				request.server->next_request();
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_warning(exc.what());
			}

			jn.shouldDelete = true;
			return jn;
		}


	};


	using handler_key = std::tuple<std::string, HTTP_VERB>;
}


namespace corona {

	class http_server : public http_server_base
	{
		HTTP_SERVER_SESSION_ID session_id;
		HTTP_URL_GROUP_ID group_id;

		std::map<std::string, std::shared_ptr<http_handler_list>> api_handlers;

		HANDLE      request_queue;
		bool shutdown;

	public:

		http_server()
		{
			shutdown = false;
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
				NULL,
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

		}

		virtual ~http_server()
		{
			shutdown = true;		

			if (group_id) {
				HttpCloseUrlGroup(group_id);
				group_id = 0;
			}
			if (session_id) {
				HttpCloseServerSession(session_id);
				session_id = 0;
			}
			if (request_queue) {
				HttpCloseRequestQueue(request_queue);
				request_queue = nullptr;
			}
		}

		void put_handler(HTTP_VERB _verb, std::string _url, http_handler_function _handler)
		{
			std::shared_ptr<http_handler_list> handler_list;

			if (api_handlers.contains(_url)) 
			{
				handler_list = api_handlers[_url];
			}
			else 
			{
				handler_list = std::make_shared<http_handler_list>();
				api_handlers.insert_or_assign(_url, handler_list);
			}

			http_handler_method method;
			method.method = _verb;
			method.func = _handler;
			handler_list->functions.push_back(method);

			HTTP_URL_CONTEXT context = (HTTP_URL_CONTEXT)handler_list.get();
			iwstring<2048> url = _url;
			DWORD error = HttpAddUrlToUrlGroup(group_id, url.c_str(), context, 0);
			if (error != NO_ERROR) {
				os_result orx(error);
				std::string message = "Exception:" + _url + " " + orx.message;
				throw std::logic_error(message.c_str());
			}

		}

		void start()
		{
			global_job_queue->listen(request_queue);
			next_request();
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
			int length = header.RawValueLength + 1;
			if (length) 
			{
				buffer buff(length);
				char *temp = buff.get_ptr();
				strncpy_s(temp, length, header.pRawValue, header.RawValueLength);
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

			absPath.copy(_request->CookedUrl.pFullUrl, _request->CookedUrl.FullUrlLength);
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

				if (_request->UrlContext) {
					http_handler_list* hhl = (http_handler_list*)_request->UrlContext;

					for (auto handler : hhl->functions)
					{
						if (handler.method == _request->Verb)
						{
							int szk = hhl->url.size();
							int szp = sabsPath.size();
							std::string remaining_path = sabsPath.substr(szk, szp - szk);
							request.rest_path = split(remaining_path, '/');
							http_action_request harhar;
							harhar.request_id = _request->RequestId;
							harhar.request = request;
							harhar.authorization = authorization;
							harhar.server = this;
							handler.func(harhar);
							unhandled = false;
						}
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

		virtual DWORD send_response(
			HTTP_REQUEST_ID _request_id, 
			int _status_code,
			std::string _reason, 
			std::string _content_type,
			char* _buffer, 
			DWORD _buffer_length_bytes) override
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

		virtual DWORD send_response(
			HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			json _response_json) override
		{
			std::string response_data = _response_json.to_json();
			DWORD result = send_response(_request_id,
				_status_code,
				_reason,
				"application/json",
				response_data);
			return result;
		}

		virtual DWORD send_response(HTTP_REQUEST_ID _request_id,
			int _status_code,
			std::string _reason,
			std::string _content_type,
			std::string _response_string) override
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

		virtual void next_request() override
		{
			http_command_request request;

			request.flags = HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY;
			request.requestId = HTTP_NULL_ID;
			request.requestQueue = request_queue;
			request.server = this;

			http_command *command_job = new http_command(request);
			global_job_queue->listen_job(command_job);
		}
	};

	DWORD http_action_request::send_response(int _status_code,
		std::string _reason,
		std::string _content_type,
		char* _buffer,
		DWORD _buffer_length_bytes)
	{
		return server->send_response(request_id, _status_code, _reason, _content_type, _buffer, _buffer_length_bytes);
	}

	DWORD http_action_request::send_response(int _status_code,
		std::string _reason,
		json _response_json)
	{
		return server->send_response(request_id, _status_code, _reason, _response_json);
	}

	DWORD http_action_request::send_response(int _status_code,
		std::string _reason,
		std::string _content_type,
		std::string _response_string)
	{
		return server->send_response(request_id, _status_code, _reason, _content_type, _response_string);
	}

}


#endif
