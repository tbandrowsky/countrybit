/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT LICENSE

About this File
A send grid client

Notes

For Future Consideration
*/


#ifndef CORONA_SENDGRID_CLIENT_H
#define CORONA_SENDGRID_CLIENT_H

namespace corona
{
	class sendgrid_client
	{

		const char *default_api_key = "";
        const char* sender_name = "Todd Bandrowsky";
        const char* sender_email = "todd.bandrowsky@countryvideogames.com";

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
            "email": "todd.bandrowsky@countryvideogames.com",
            "name" : "Todd Bandrowsky"
      },
      "reply_to" : {
            "email": "todd.bandrowsky@countryvideogames.com",
            "name" : "Todd Bandrowsky"
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
            from_json.put_member("email", std::string_view(sender_email));
            from_json.put_member("name", std::string_view(sender_name));

            json reply_json = jp.create_object();
            reply_json.put_member("email", std::string_view(sender_email));
            reply_json.put_member("name", std::string_view(sender_name));

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
