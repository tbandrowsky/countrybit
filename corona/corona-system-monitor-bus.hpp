#pragma once

#ifndef CORONA_SMI
#define CORONA_SMI

namespace corona
{

	class system_monitoring_interface
	{
	public:
		char CSI[3] = { 0x1b, '[', 0 };
		char Normal[4] = { 0x1b, '[', '0', 0 };
		char Logstart[11] = { 0x1b, '[', '9', '7', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Logstop[11] = { 0x1b, '[', '9', '2', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Lognormal[11] = { 0x1b, '[', '9', '7', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Logexception[11] = { 0x1b, '[', '9', '5', 'm', 0x1b, '[', '4', '0', 'm', 0 };
		char Logwarning[11] = { 0x1b, '[', '9', '3', 'm', 0x1b, '[', '4', '0', 'm', 0 };
		char Loginformation[11] = { 0x1b, '[', '3', '7', 'm', 0x1b, '[', '4', '0', 'm', 0 };

		static system_monitoring_interface* global_mon;

		system_monitoring_interface()
		{
			
		}

		void file_line(const char* _file, int _line)
		{
			if (_file) {
				const char* last_post = _file;
				const char* fn = _file;

				while (*fn) 
				{
					if (*fn == '/' || *fn == '\\') 
					{
						last_post = fn;
					}
					fn++;
				}

				if (*last_post == '/' || *last_post == '\\')
				{
					last_post++;
				}

				std::cout << std::format("{0:<30}{1:<8}", last_post, _line);
			}
		}

		static void start()
		{
			global_mon = new system_monitoring_interface();
		}

		virtual void log_bus(std::string _request_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			std::cout << Logstart;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				_request_name,
				_message,
				GetCurrentThreadId(),
				(std::string)_request_time
			);
			file_line(_file, _line);
			std::cout << std::endl;
			std::cout << Normal;
		}

		virtual void log_bus(std::string _request_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			std::cout << Logstop;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				_request_name,
				_message,
				GetCurrentThreadId(),
				_elapsed_seconds
			);
			file_line(_file, _line);
			std::cout << std::endl;
			std::cout << Normal;
		}

		virtual void log_bus(std::string _message, const char* _file = nullptr, int _line = 0)
		{
			std::cout << Lognormal;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				_message,
				GetCurrentThreadId(),
				""
			);
			file_line(_file, _line);
			std::cout << std::endl;
			std::cout << Normal;
		}

		virtual void log_warning(std::string _message, const char *_file = nullptr, int _line = 0)
		{
			std::cout << Logwarning;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				_message,
				GetCurrentThreadId(),
				""
			);
			file_line(_file, _line);
			std::cout << std::endl;
			std::cout << Normal;

		}

		virtual void log_exception(std::exception exc, const char* _file = nullptr, int _line = 0)
		{
			std::cout << Logexception;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				exc.what(),
				GetCurrentThreadId(),
				""
			);
			file_line(_file, _line);
			std::cout << std::endl;
			std::cout << Normal;
		}

		template <typename json_type> void log_json(json_type& _src, int _indent = 2)
		{
			std::string sindent(_indent, ' ');
			if (_src.object())
			{
				auto members = _src.get_members();
				for (auto member : members) {
					auto body = member.second;
					auto key = member.first;
					std::string name = sindent + key;
					std::cout << std::format("{0:<40}:", name);
					if (body.object())
					{
						std::cout << std::format("{0:<80}:", "object");
						log_json(body, _indent + 4);
					}
					else if (body.array())
					{
						std::cout << std::format("{0:<80}:", "array");
						log_json(body, _indent + 4);
					}
					else
					{
						std::string v = body.to_json();
						std::cout << std::format("{0:<80}", v);
					}
				}
			}
			else if (_src.array())
			{
				for (int i = 0; i < _src.size(); i++)
				{
					auto item = _src.get_element(i);
					std::cout << std::format("{0:<40}:", i);
					log_json(item, _indent + 4);
				}
			}
		}
	};

	system_monitoring_interface* system_monitoring_interface::global_mon;
}

#endif
