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

    public:
        std::string api_key;
        std::string host;
        int port;
        std::string sender_name = "";
        std::string sender_email = "";

		sendgrid_client()
		{
			api_key = default_api_key;
			host = "api.sendgrid.com";
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
			std::string authorization_header = "Authorization:Bearer " + api_key + "\n";
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
            "email": "p1",
            "name" : "p2"
      },
      "reply_to" : {
            "email": "p1",
            "name" : "p2"
      },
      "subject" : "p3",
      "content" : [
        {
          "type": "text/html",
          "value" : "<p>Hello from Twilio SendGrid!</p><p>Sending with the email service trusted by developers and marketers for <strong>time-savings</strong>, <strong>scalability</strong>, and <strong>delivery expertise</strong>.</p><p>%open-track%</p>"
        }
      ]
}			
)";
            std::string recipient_user_name = _user[user_name_field];
            std::string recipient_first_name = _user[user_first_name_field];
            std::string recipient_last_name = _user[user_last_name_field];
            std::string recipient_user_email = _user[user_email_field];
            std::string recipient_display_name;

            if (recipient_user_email.empty()) {
                recipient_user_email = recipient_user_name;
            }

            if (recipient_first_name.empty() && recipient_last_name.empty()) {
                recipient_display_name = recipient_user_name;
            }
            else 
            {
                recipient_display_name = recipient_first_name + " " + recipient_last_name;
            }

            json email_request = jp.parse_object(email_template);

            json to_json = jp.create_object();
            to_json.put_member("email", recipient_user_email);
            to_json.put_member("name", recipient_display_name);

            json from_json = jp.create_object();
            from_json.put_member("email", std::string(sender_email));
            from_json.put_member("name", std::string(sender_name));

            json reply_json = jp.create_object();
            reply_json.put_member("email", std::string(sender_email));
            reply_json.put_member("name", std::string(sender_name));

            email_request.put_member("from", from_json);
            email_request.put_member("reply_to", reply_json);
            email_request.put_member("subject", _subject);

            json personalizations = jp.create_array();
            json personal = jp.create_object();
            json to_array = jp.create_array();
            to_array.push_back(to_json);
            personal.put_member("to", to_array);
            personalizations.push_back(personal);
            email_request.put_member("personalizations", personalizations);

            json content = jp.create_object();
            content.put_member("type", _body_mime);
            content.put_member("value", _body);

            json contents = jp.create_array();
            contents.push_back(content);
            email_request.put_member("content", contents);

            std::string header = "Content-Type: application/json\r\n" + authorization_header;

			http_params params = send_grid_client.post(host.c_str(), port, "/v3/mail/send", email_request, header.c_str(), true);

            return params;
		}
	};
}

#endif
