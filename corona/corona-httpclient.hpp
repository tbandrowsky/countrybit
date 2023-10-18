

#ifndef CORONA_HTTP_CLIENT_H
#define CORONA_HTTP_CLIENT_H

#include "corona-windows-lite.h"
#include "corona-messages.hpp"
#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-string_box.hpp"
#include "corona-constants.hpp"
#include "corona-messages.hpp"
#include "corona-function.hpp"
#include "corona-json.hpp"
#include "corona-wchart_convert.hpp"

#include <exception>
#include <stdexcept>
#include <iostream>
#include <compare>

namespace corona
{

	class http_result 
	{
	public:

        std::string				host;
		std::string				url;
		std::string				method;
		json_navigator			request_body;

        std::unique_ptr<char[]> result_body;
		json_navigator			result;

        os_result               system_result;

		int						http_status_code;
		std::string				mime_type;
	};

	class http_client
	{
    public:

        const char *user_agent = "Corona/1.0";

    private:

		http_result read_internet(const char *_host, const char* _path, const char* _method, const unsigned char* _body, int _bodyLength)
        {

            http_result result;

            wchar_converter converter;

            DWORD       dwSize = 0;
            DWORD       dwDownloaded = 0;
            BOOL        bResults = FALSE;
            HINTERNET   hSession = NULL,
                        hConnect = NULL,
                        hRequest = NULL;

            const wchar_t* wuser_agent = converter.to_wchar_t(user_agent);
            const wchar_t* whost = converter.to_wchar_t(_host);
            const wchar_t* wpath = converter.to_wchar_t(_path);
            const wchar_t* wmethod = converter.to_wchar_t(_method);

            std::vector<std::unique_ptr<char[]>> buffers;
            std::unique_ptr<char[]> buffer;

            // Use WinHttpOpen to obtain a session handle.
            hSession = WinHttpOpen(wuser_agent,
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS, 0);

            // Specify an HTTP server.
            if (hSession)
                hConnect = WinHttpConnect(hSession, whost,
                    INTERNET_DEFAULT_HTTPS_PORT, 0);

            // Create an HTTP request handle.
            if (hConnect)
                hRequest = WinHttpOpenRequest(hConnect, wmethod, NULL,
                    NULL, WINHTTP_NO_REFERER,
                    WINHTTP_DEFAULT_ACCEPT_TYPES,
                    WINHTTP_FLAG_SECURE);

            // Send a request.
            if (hRequest)
                bResults = WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0,
                    0, 0);


            // End the request.
            if (bResults)
                bResults = WinHttpReceiveResponse(hRequest, NULL);

            // Keep checking for data until there is nothing left.
            if (bResults)
            {
                do
                {
                    // Check for available data.
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                        result.system_result = os_result();
                    }

                    if (dwSize) {
                        // Allocate space for the buffer.
                        buffer = std::make_unique< char[] >(dwSize + 1);
                        if (!buffer)
                        {
                            result.system_result.success = false;
                            result.system_result.message = "Out of memory";
                            dwSize = 0;
                        }
                        else
                        {
                            // Read the data.
                            ZeroMemory(buffer.get(), dwSize + 1);

                            if (!WinHttpReadData(hRequest, (LPVOID)buffer.get(),
                                dwSize, &dwDownloaded))
                            {
                                result.system_result = os_result();
                            }
                            else
                            {

                            }
                            // Free the memory allocated to the buffer.
                            buffers.push_back(std::move(buffer));
                        }
                    }
                } while (dwSize > 0);

                if (buffers.size()) 
                {

                }
            }

            // Report any errors.
            if (!bResults)
                printf("Error %d has occurred.\n", GetLastError());

            // Close any open handles.
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
		}

	public:

		task<http_result> get(std::string _url, json_navigator _params)
		{
			;
		}

		task<http_result> get(std::string _url)
		{
			;
		}

		task<http_result> post(std::string _url, json_navigator _body)
		{
			;
		}
	};
}

#endif

