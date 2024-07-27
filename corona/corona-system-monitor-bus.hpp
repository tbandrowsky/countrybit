#pragma once

#ifndef CORONA_SMI
#define CORONA_SMI

namespace corona
{

	class system_monitoring_interface
	{
	public:
		char CSI[3] = { 0x1b, '[', 0 };
		char Normal[3] = { 0x1b, '[', 0 };
		char Logstart[11] = { 0x1b, '[', '3', '7', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Logstop[11] = { 0x1b, '[', '3', '2', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Lognormal[11] = { 0x1b, '[', '3', '7', 'm', 0x1b, '[', '4', '4', 'm', 0 };
		char Logexception[11] = { 0x1b, '[', '3', '5', 'm', 0x1b, '[', '4', '0', 'm', 0 };
		char Logwarning[11] = { 0x1b, '[', '3', '3', 'm', 0x1b, '[', '4', '0', 'm', 0 };

		static system_monitoring_interface* global_mon;

		system_monitoring_interface()
		{
			global_mon = this;
		}

		virtual void log_bus(std::string _request_name, std::string _message, date_time _request_time)
		{
			std::cout << Logstart;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				_request_name,
				_message,
				GetCurrentThreadId(),
				(std::string)_request_time
			) << std::endl;
			std::cout << Normal;
		}

		virtual void log_bus(std::string _request_name, std::string _message, double _elapsed_seconds)
		{
			std::cout << Logstop;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				_request_name,
				_message,
				GetCurrentThreadId(),
				_elapsed_seconds
			) << std::endl;
			std::cout << Normal;
		}

		virtual void log_bus(std::string _message)
		{
			std::cout << Lognormal;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				_message,
				GetCurrentThreadId(),
				""
			) << std::endl;
			std::cout << Normal;
		}

		virtual void log_warning(std::string _message)
		{
			std::cout << Logwarning;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				_message,
				GetCurrentThreadId(),
				""
			) << std::endl;
			std::cout << Normal;
		}

		virtual void log_exception(std::exception exc)
		{
			std::cout << Logexception;
			std::cout << std::format("{0:<25}{1:<50}{2:<10}{3:<25}",
				"",
				exc.what(),
				GetCurrentThreadId(),
				""
			) << std::endl;
			std::cout << Normal;
		}
	};

	system_monitoring_interface* system_monitoring_interface::global_mon;
}

#endif
