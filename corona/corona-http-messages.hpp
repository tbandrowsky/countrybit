/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
Various statuses and http messages.

Notes

For Future Consideration
*/


#ifndef CORONA_HTTP_MESSAGES_H
#define CORONA_HTTP_MESSAGES_H

namespace corona
{
	class http_response
	{
	public:
		os_result               system_result;

		int						http_status_code;

		buffer                  response_body;
		std::string             content_type;
		std::string             content_length;
		std::string             server;
	};

	class http_request
	{
	public:
		std::string                  host;
		int                          port;
		std::string                  path;
		std::vector<std::string>     rest_path;
		std::string                  http_method;
		std::vector<std::string>     allowed_types;
		std::string                  headers;
		buffer                       body;
		std::string                  query_string;
	};

	class http_params
	{
	public:
		http_request  request;
		http_response response;
	};

	class call_status
	{
	public:
		bool success;
		std::string message;
		http_request request;
		http_response response;
		int http_code;
		time_t call_time;
		std::string function_path;

		call_status()
		{
			time(&call_time);
			http_code = 0;
			success = false;
		}
	};
}

#endif
