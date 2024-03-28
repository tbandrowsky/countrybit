#ifndef CORONA_SENDGRID_CLIENT_H
#define CORONA_SENDGRID_CLIENT_H

#include "corona-httpclient.hpp"
#include "corona-function.hpp"
#include "corona-time_box.hpp"
#include "corona-json.hpp"

namespace corona
{
	class sendgrid_client
	{

		const char *default_api_key = "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";
        const char* sender_name = "Willie Otis";
        const char* sender_email = "willie.otis@countrybit.com";

    public:
        std::string api_key;
        std::string host;
        int port;

		sendgrid_client()
		{
			api_key = default_api_key;
			host = "https://api.sendgrid.com";
			port = 443;
		}

		http_params send_email(json _user, 
            std::string _subject, 
            std::string _body, 
            std::string _body_mime)
		{
			json_parser jp;
			http_client send_grid_client;
			std::string send_grid_url;
			std::string user_mail = _user["Email"];
			std::string authorization_header = "Authorization:Bearer " + api_key;
            std::string email_template =  R"(
{
      "personalizations": [
        {
          "to": [
            {
              "email": "john_doe@example.com",
              "name" : "John Doe"
            },
          ]
        }
      ],
      "from": {
            "email": "willie.otis@countrybit.com",
            "name" : "Willie Otis"
      },
      "reply_to" : {
            "email": "willie.otis@countrybit.com",
            "name" : "Willie Otis"
      },
      "subject" : "Countrybit Access Code",
      "content" : [
        {
          "type": "text/html",
          "value" : "<p>Hello from Twilio SendGrid!</p><p>Sending with the email service trusted by developers and marketers for <strong>time-savings</strong>, <strong>scalability</strong>, and <strong>delivery expertise</strong>.</p><p>%open-track%</p>"
        }
      ]
}			
)";
            std::string recipient_name = _user["Name"];
            std::string recipient_email = _user["Email"];

            json email_request = jp.parse_object(email_template);

            json to_json = jp.create_object();
            to_json.put_member("email", recipient_email);
            to_json.put_member("name", recipient_name);

            json from_json = jp.create_object();
            from_json.put_member("email", sender_email);
            from_json.put_member("name", sender_name);

            json reply_json = jp.create_object();
            reply_json.put_member("email", sender_email);
            reply_json.put_member("name", sender_name);

            email_request.put_member("from", from_json);
            email_request.put_member("reply_to", reply_json);
            email_request.put_member("subject", _subject);

            json personalizations = email_request["Personalizations"];
            personalizations.put_element(0, to_json);

            json content = jp.create_object();
            content.put_member("type", _body_mime);
            content.put_member("value", _body);

            json contents = email_request["content"];
            contents.put_element(0, content);

			http_params params = send_grid_client.post(host.c_str(), port, "/v3/mail/send", email_request, authorization_header.c_str());

            return params;
		}
	};
}

#endif
