

#ifndef CORONA_HTTP_CLIENT_H
#define CORONA_HTTP_CLIENT_H

namespace corona
{


    class url_builder
    {
        buffer_assembler ba;
        bool adding_params;

    public:

        url_builder()
        {
            adding_params = false;
        }

        url_builder& scheme(std::string _scheme)
        {
            istring<1024> temp;

            if (_scheme == "http") 
            {
                _scheme = "http://";
            }
            else if (_scheme == "https") 
            {
                _scheme = "https://";
            }
            else if (_scheme != "http://" && _scheme != "https://")
            {
                throw std::invalid_argument("Invalid scheme.  Should be http,https,http:// or https://");
            }

            ba.append(_scheme);

            return *this;
        }
        url_builder& domain(std::string _domain)
        {
            if (_domain.size() > 0) {
                while (_domain.ends_with('/'))
                    _domain.pop_back();
                ba.append(_domain);
            }
            else {
                throw std::invalid_argument("Invalid domain.");
            }
            return *this;
        }
        url_builder& path(std::string _path)
        {
            if (_path.size() > 0) 
            {                
                if (ba.size() > 0) 
                {
                    if (_path[0] != '/') {
                        _path = "/" + _path;
                    }
                }
                while (_path.ends_with('/'))
                    _path.pop_back();
                ba.append(_path);
            } 
            else 
            {
                throw std::invalid_argument("Invalid path.");
            }
            return *this;
        }
        url_builder& param(std::string _param)
        {
            if (_param.size() > 0)
            {
                if (!adding_params) {
                    adding_params = true;
                    ba.append("?");
                }
                else 
                {
                    ba.append("&");
                }
                ba.append(_param);
                ba.append("=");
            }
            else 
            {
                throw std::invalid_argument("Invalid param.");
            }
            return *this;
        }

        url_builder& value(std::string _value)
        {
            if (_value.size() > 0) 
            {
                char escape_buffer[4096] = {  };
                char* pbuffer = &escape_buffer[0];
                DWORD escape_buffer_size = sizeof(escape_buffer) / sizeof(char);

                for (char c : _value)
                {
                    if (isalnum(c) || c == '_') {
                        *pbuffer = c;
                        pbuffer++;
                        escape_buffer_size--;
                    }
                    else 
                    {
                        char temp[128];
                        int ct = c;
                        sprintf_s(temp, "%0X", ct);
                        char* t = &temp[0];
                        *pbuffer = '%';
                        pbuffer++;
                        escape_buffer_size--;
                        while (*t) 
                        {
                            if (escape_buffer_size <= 0)
                                throw std::logic_error("Url escape buffer is too small");
                            *pbuffer = *t;
                            pbuffer++;
                            t++;
                            escape_buffer_size--;
                        }
                    }
                    if (escape_buffer_size <= 0)
                        throw std::logic_error("Url escape buffer is too small");
                }
                *pbuffer = 0;

                ba.append(escape_buffer);
            }
            return *this;
        }

        buffer get_string()
        {
            return ba.consolidate();
        }

    };


	class http_client
	{
        const char *user_agent = "Corona/1.0";

        std::string get_header(std::string context, HINTERNET hRequest, DWORD header_id)
        {
            std::string header;

            BOOL bResults;
            DWORD dwHeaderSize;
            wchar_converter converter;

            BOOL success = WinHttpQueryHeaders(hRequest,
                header_id,
                WINHTTP_HEADER_NAME_BY_INDEX,
                NULL,  &dwHeaderSize, 
                0);

            os_result header_result;

            // Allocate memory for the buffer.
            if (header_result.error_code == ERROR_INSUFFICIENT_BUFFER)
            {
                buffer headerBuffer(dwHeaderSize);

                // Now, use WinHttpQueryHeaders to retrieve the header.
                bResults = WinHttpQueryHeaders(hRequest,
                    header_id,
                    WINHTTP_HEADER_NAME_BY_INDEX,
                    headerBuffer.get_ptr(), &dwHeaderSize,
                    0);

                if (bResults) {
                    wchar_t* header_w = (wchar_t*)headerBuffer.get_ptr();
                    header = converter.to_char(header_w);
                }
            }
            else {
                throw_error(context, "Reading Http Headers");
            }

            return header;
        }

        void throw_error(std::string host, std::string context)
        {
            os_result osr;
            auto str = std::format("{1}\n{0}\nCheck your config.json for the correct server name and port.", host, context);
            throw std::runtime_error(str);
        }

        bool run(http_params& params)
        {
            std::cout << ::GetCurrentThreadId() << " http request thread" << std::endl;

            wchar_converter converter;

            DWORD       dwSize = 0;
            DWORD       dwHeaderSize = 0;
            DWORD       dwDownloaded = 0;
            BOOL        bResults = FALSE;
            HINTERNET   hSession = NULL,
                hConnect = NULL,
                hRequest = NULL;

            url_builder builder;

            const wchar_t* wuser_agent = converter.to_wchar_t("Corona 1.0.0");
            const wchar_t* whost = converter.to_wchar_t(params.request.host.c_str());
            const wchar_t* wpath = converter.to_wchar_t(params.request.path.c_str());
            const wchar_t* wmethod = converter.to_wchar_t(params.request.http_method.c_str());
            const wchar_t* wheaders = converter.to_wchar_t(params.request.headers.c_str());

            std::vector<LPCWSTR> allowed_types;
            LPCWSTR* allowed_types_lies = nullptr;

            auto body_length = params.request.body.get_size();

            for (auto ati : params.request.allowed_types)
            {
                LPCWSTR wallowed_types = converter.to_wchar_t(ati.c_str());
                allowed_types.push_back(wallowed_types);
            }
            allowed_types.push_back(nullptr);
            allowed_types_lies = allowed_types.data();

            int wheader_length = 0;
            if (wheaders) {
                wheader_length = wcslen(wheaders);
            }

            buffer_assembler ba;

            // Use WinHttpOpen to obtain a session handle.
            hSession = WinHttpOpen(wuser_agent,
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS, 0);

            // Specify an HTTP server.
            if (hSession)
            {
                hConnect = WinHttpConnect(hSession, whost,
                    params.request.port, 0);
            }
            else 
            {
                throw_error(params.request.host, "System issue: Cannot open connection.");
            }

            // Create an HTTP request handle.
            if (hConnect) {
                hRequest = WinHttpOpenRequest(hConnect,
                    wmethod,
                    wpath,
                    NULL,
                    WINHTTP_NO_REFERER,
                    allowed_types_lies,
                    WINHTTP_FLAG_SECURE);
            }
            else 
            {
                throw_error(params.request.host, "System issue: Cannot open request.");
            }

            int total_length = body_length;

            // Send a request.
            if (hRequest) {
                auto body_buffer = params.request.body.get_ptr();
                bResults = WinHttpSendRequest(hRequest,
                    wheaders,
                    -1,
                    (LPVOID)body_buffer,
                    body_length,
                    total_length, 0);
            }
            else 
            {
                throw_error(params.request.host + "/" + params.request.path, "Connection issue: Unable to send request.");
            }

            // End the request.
            if (bResults) 
            {
                bResults = WinHttpReceiveResponse(hRequest, NULL);
            }
            else
            {
                throw_error(params.request.host + "/" + params.request.path, "Connection issue: Unable to get response.");
            }

            // Check the headers

            params.response.content_type = get_header(params.request.host + "/" + params.request.path, hRequest, WINHTTP_QUERY_CONTENT_TYPE);
            params.response.content_length = get_header(params.request.host + "/" + params.request.path, hRequest, WINHTTP_QUERY_CONTENT_LENGTH);

            // Keep checking for data until there is nothing left.
            if (bResults)
            {
                do
                {
                    // Check for available data.
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                    {
                        throw_error(params.request.host + "/" + params.request.path, "Server issue: Did not receive any data.");
                    }

                    if (dwSize) {
                        // Allocate space for the buffer.
                        char* buf = ba.append(dwSize);
                        if (!buf)
                        {
                            params.response.system_result.success = false;
                            params.response.system_result.message = "Out of memory";
                            throw_error(params.request.host + "/" + params.request.path, "Client Issue: Do not have enough memory.");
                            dwSize = 0;
                        }
                        else
                        {
                            // Read the data.
                            ZeroMemory(buf, dwSize + 1);

                            if (!WinHttpReadData(hRequest, buf, dwSize, &dwDownloaded))
                            {
                                params.response.system_result = os_result();
                                dwSize = 0;
                                throw_error(params.request.host + "/" + params.request.path, "Server issue: Did not receive body.");
                            }
                        }
                    }
                } while (dwSize > 0);

                params.response.response_body = ba.consolidate();

            }

            // Close any open handles.
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);

            return true;
        }

	public:

        http_params get(
            const char *_host, 
            int _port,
            const char *_url, 
            json _params,
            const char* _headers = nullptr)
		{
            http_params params;

            params.request.host = _host;
            params.request.http_method = "GET";
            params.request.port = _port;

            url_builder ub;
            ub.path(_url);
            auto items = _params.get_members();
            for (auto ky : items)
            {
                ub.param(ky.first);
                ub.value(ky.second->to_string());
            }
            buffer url_buffer = ub.get_string();
            params.request.path = url_buffer.get_ptr();
            params.request.allowed_types = {
                "application/json",
                "text/plain"
            };
            if (_headers)
                params.request.headers = _headers;

            run(params);

            return params;
		}

        http_params get(const char* _host, int _port, const char* _url, const char *_headers = nullptr)
		{
            http_params params;

            params.request.host = _host;
            params.request.http_method = "GET";
            params.request.path = _url;
            params.request.port = _port;

            params.request.allowed_types = {
                "application/json",
                "text/plain"
            };
            if (_headers)
                params.request.headers = _headers;

            run(params);

            return params;
        }

		http_params post(const char* _host, int _port, const char* _url, json _body, const char* _headers = nullptr)
		{
            http_params params;

            params.request.host = _host;
            params.request.http_method = "POST";
            params.request.port = _port;

            params.request.path = _url;
            params.request.headers = "Content-Type: application/json\r\n";
            params.request.allowed_types = {
                "application/json",
                "text/plain"
            };
            std::string body_string = _body.to_json();
            params.request.body = buffer(body_string.c_str());
            if (_headers)
                params.request.headers = _headers;

            run(params);
            return params;
        }
	};
}

#endif

