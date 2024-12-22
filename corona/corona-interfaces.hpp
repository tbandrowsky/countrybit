#pragma once

#ifndef CORONA_INTERFACES_H

namespace corona
{
	class corona_client_response
	{
	public:
		bool			success;
		std::string		message;
		double			execution_time;
		json			data;

		corona_client_response& operator = (http_params& _params)
		{
			json_parser jp;
			json response = jp.parse_object(_params.response.server.c_str());
			data = response[data_field];
			execution_time = response["execution_time_seconds"];
			success = (bool)response[success_field];
			message = (bool)response[message_field];
			return *this;
		}
	};
}

#endif
