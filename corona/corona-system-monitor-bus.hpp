#pragma once

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT LICENSE

About this File
Color logging!

Notes

For Future Consideration
*/

#ifndef CORONA_SMI
#define CORONA_SMI



namespace corona
{
	class console_color {
	public:

		std::string color_foreground;
		std::string color_background;
	};

	std::ostream& operator << (std::ostream& _src, const console_color& _color)
	{
		char CSI[3] = { 0x1b, '[', 0 };

		// << "38;2;" << _color.color_foreground << 

		_src << CSI << "48;2;" << _color.color_background << "m";
		return _src;
	}

	class system_monitoring_interface
	{
	public:
		char CSI[3] = { 0x1b, '[', 0 };
		char Logactivity[3] = { 0x1b, 'M', 0 };

		bool enable_json_table_logging = false;
		bool enable_json_poco_logging = false;
		bool enable_json_block_logging = false;
		bool enable_put_logging = false;
		bool enable_options_display = false;

		void test_colors()
		{
			auto& xout = get_log_file();
			date_time start_time = date_time::now();
			timer tx;
			log_command_start("Startup", "Color Test", start_time, __FILE__, __LINE__);

			for (int i = 0; i < 256; i += 45) {
				xout << std::format("{0:<30}", " ");
				for (int j = 0; j < 45; j++) {
					xout << CSI << "48;5;" << std::to_string(j + i) << "m ";
				}
				xout << Loginformation << std::endl;
			}
			for (int i = 0; i < 256; i += 45) {
				xout << std::format("{0:<30}", " ");
				for (int j = 0; j < 45; j++) {
					xout << CSI << "48;2;" << "0;0;" << std::to_string(j + i) << "m ";
				}
				xout << Loginformation << std::endl;
			}
			log_command_stop("Startup", "Color Test", tx.get_elapsed_seconds(), __FILE__, __LINE__);
		}

		char Normal[5] = { 0x1b, '[', '0', 'm', 0 };


		console_color Logusercommand;
		console_color Logcommand;
		console_color Logapi;
		console_color Logfunction;
		console_color Lognormal;
		console_color Logexception;
		console_color Logwarning;
		console_color Loginformation;
		console_color Logamd;
		console_color Lognvidia;
		console_color Logintel;		

		static system_monitoring_interface* global_mon;
		static system_monitoring_interface* active_mon;

		CRITICAL_SECTION log_lock;

		system_monitoring_interface()
		{
			::InitializeCriticalSectionAndSpinCount(&log_lock, 10);
			Logusercommand.color_background = "12;59;69";
			Logcommand.color_background = "21;83;98";
			Logapi.color_background = "40;55;42";
			Logfunction.color_background = "68;60;57";
			Lognormal.color_background = "103;115;95";
			Logexception.color_background = "103;50;40";
			Logwarning.color_background = "178;94;23";
			Loginformation.color_background = "0;0;0";
			Logamd.color_background = "237;28;36";
			Logintel.color_background = "0;104;181";
			Lognvidia.color_background = "118;185;0";

			Logusercommand.color_foreground = "239;236;232";
			Logcommand.color_foreground = "239;236;232";
			Logapi.color_foreground = "220;214;209";
			Logfunction.color_foreground = "220;214;209";
			Lognormal.color_foreground = "0;0;0";
			Logexception.color_foreground = "0;0;0";
			Logwarning.color_foreground = "0;0;0";
			Loginformation.color_foreground = "220;214;209";
			Logamd.color_foreground = "255;255;255";
			Logintel.color_foreground = "255;255;255";
			Lognvidia.color_foreground = "255;255;255";
		}

		std::string log_file_name = "corona.log";
		bool is_service = false;
		std::ofstream log_file;

		std::ostream &get_log_file()
		{
			if (is_service) {
				if (not log_file.is_open()) {
					log_file.open(log_file_name, std::ios::out | std::ios::app);
				}
				return log_file;
			}

			return std::cout;
		}

		void file_line(const char* _file, int _line)
		{
			auto &xout = get_log_file();

			if (_file) {
				const char* last_post = _file;
				const char* fn = _file;

				while (*fn)
				{
					if (*fn == '/' or *fn == '\\')
					{
						last_post = fn;
					}
					fn++;
				}

				if (*last_post == '/' or *last_post == '\\')
				{
					last_post++;
				}

				xout << std::format("{0:<30}{1:<8}", last_post, _line);
			}
			else {
				xout << std::format("{0:<30}{1:<8}", "", "");
			}
		}

		static void start()
		{
			if (global_mon == nullptr) {
				global_mon = new system_monitoring_interface();
				active_mon = global_mon;
				global_mon->test_colors();
			}
		}

		virtual void log_user_command_start(std::string _command_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{


			if (_command_name.empty())
				_command_name = " ";
			if (_message.empty())
				_message = " ";

            const char* cmessage = _message.c_str();
            const char* ccommand = _command_name.c_str();
            const char* cfilename = get_file_name(_file);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "UserCommandStart"),
				TraceLoggingValue(ccommand, "Command"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_line, "Line")
			);

			::EnterCriticalSection(&log_lock);

			try {

				auto& xout = get_log_file();

				xout << Logusercommand;
				xout << std::format("{0:<30}{1:<45}{2:<10}{3:<25}",
					_command_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					_request_time.format("%D %H:%M start")
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}

			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_user_command_stop(std::string _command_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {

				const char* cmessage = _message.c_str();
				const char* ccommand = _command_name.c_str();
				const char* cfilename = get_file_name(_file);

				TraceLoggingWrite(
					global_corona_provider,
					"Corona",
					TraceLoggingLevel(WINEVENT_LEVEL_INFO),
					TraceLoggingStruct(3, "UserCommandStop"),
					TraceLoggingValue(ccommand, "Command"),
					TraceLoggingValue(cmessage, "Message"),
					TraceLoggingValue(cfilename, "File"),
					TraceLoggingValue(_elapsed_seconds, "ExecutionTime"),
					TraceLoggingValue(_line, "Line")
				);

				auto& xout = get_log_file();

				xout << Logusercommand;
				xout << std::format("{0:<30}{1:<45}{2:<10}{3:<25}",
					_command_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					std::format("{0} secs",_elapsed_seconds)
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}


		virtual void log_command_start(std::string _command_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			const char* cmessage = _message.c_str();
			const char* ccommand = _command_name.c_str();
			const char* cfilename = get_file_name(_file);
			if (cfilename) {
		
				TraceLoggingWrite(
					global_corona_provider,
					"Corona",
					TraceLoggingLevel(WINEVENT_LEVEL_INFO),
					TraceLoggingStruct(3, "CommandStart"),
					TraceLoggingValue(ccommand, "Command"),
					TraceLoggingValue(cmessage, "Message"),
					TraceLoggingValue(cfilename, "File"),
					TraceLoggingValue(_line, "Line")
				);
			}

			try {
				auto& xout = get_log_file();

				xout << Logcommand;
				xout << std::format("{0:<30}{1:<45}{2:<10}{3:<25}",
					_command_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					_request_time.format("%D %H:%M start")
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_command_stop(std::string _command_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			const char* cmessage = _message.c_str();
			const char* ccommand = _command_name.c_str();
			const char* cfilename = get_file_name(_file);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "CommandStop"),
				TraceLoggingValue(ccommand, "Command"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_elapsed_seconds, "ExecutionTime"),
				TraceLoggingValue(_line, "Line")
			);
			try {
				auto& xout = get_log_file();

			if (_command_name.empty())
				_command_name = " ";
			if (_message.empty())
				_message = " ";

			xout << Logcommand;
			xout << std::format("{0:<30}{1:<45}{2:<10}{3:<25}",
				_command_name,
				_message,
				GetCurrentThreadId(),
				std::format("{0} secs", _elapsed_seconds)
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}

			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_job_start(std::string _api_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			const char* cmessage = _message.c_str();
			const char* ccommand = _api_name.c_str();
			const char* cfilename = get_file_name(_file);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "JobStart"),
				TraceLoggingValue(ccommand, "Api"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_line, "Line")
			);

			try {
				auto& xout = get_log_file();

			if (_api_name.empty())
				_api_name = " ";
			if (_message.empty())
				_message = " ";
			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<25}{1:<45}{2:<10}{3:<25}",
				_api_name,
				trim(_message, 45),
				GetCurrentThreadId(),
				_request_time.format("%D %H:%M start")
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}

			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_job_stop(std::string _api_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			const char* cmessage = _message.c_str();
			const char* ccommand = _api_name.c_str();
			const char* cfilename = get_file_name(_file);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "JobStop"),
				TraceLoggingValue(ccommand, "Api"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_elapsed_seconds, "ExecutionTime"),
				TraceLoggingValue(_line, "Line")
			);

			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			if (_api_name.empty())
				_api_name = " ";
			if (_message.empty())
				_message = " ";

			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<25}{1:<45}{2:<10}{3:<25}",
				_api_name,
				trim(_message, 45),
				GetCurrentThreadId(),
				std::format("{0} secs", _elapsed_seconds)
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_job_section_start(std::string _api_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

				if (_api_name.empty())
					_api_name = " ";
				if (_message.empty())
					_message = " ";
				xout << Logcommand;
				xout << std::format("{0:<5}", " ");
				xout << Logapi;
				xout << std::format(" {0:<24}{1:<45}{2:<10}{3:<25}",
					_api_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					_request_time.format("%D %H:%M start")
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_job_section_stop(std::string _api_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			const char* cmessage = _message.c_str();
			const char* ccommand = _api_name.c_str();
			const char* cfilename = get_file_name(_file);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "JobSectionStop"),
				TraceLoggingValue(ccommand, "Api"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_elapsed_seconds, "ExecutionTime"),
				TraceLoggingValue(_line, "Line")
			);

			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

				if (_api_name.empty())
					_api_name = " ";
				if (_message.empty())
					_message = " ";

				xout << Logcommand;
				xout << std::format("{0:<5}", " ");
				xout << Logapi;
				xout << std::format(" {0:<24}{1:<45}{2:<10}{3:<25}",
					_api_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					std::format("{0} secs", _elapsed_seconds)
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_function_start(std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			const char* cmessage = _message.c_str();
			const char* ccommand = _function_name.c_str();
			const char* cfilename = get_file_name(_file);


			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}{1:<45}{2:<10}{3:<25}",
				_function_name,
				trim(_message, 45),
				GetCurrentThreadId(),
				_request_time.format("%D %H:%M start")
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_function_stop(std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			const char* cmessage = _message.c_str();
			const char* ccommand = _function_name.c_str();
			const char* cfilename = get_file_name(_file);

			::EnterCriticalSection(&log_lock);

			TraceLoggingWrite(
				global_corona_provider,
				"Corona",
				TraceLoggingLevel(WINEVENT_LEVEL_INFO),
				TraceLoggingStruct(3, "FunctionComplete"),
				TraceLoggingValue(ccommand, "Function"),
				TraceLoggingValue(cmessage, "Message"),
				TraceLoggingValue(cfilename, "File"),
				TraceLoggingValue(_elapsed_seconds, "ExecutionTime"),
				TraceLoggingValue(_line, "Line")
			);

			try {
				auto& xout = get_log_file();
				
				xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}{1:<45}{2:<10}{3:<25}",
				_function_name,
				trim(_message, 45),
				GetCurrentThreadId(),
				std::format("{0} secs", _elapsed_seconds)
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}



			::LeaveCriticalSection(&log_lock);
		}


		virtual void log_base_block_start(int _indent, std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

				std::string sindent(_indent, ' ');
				_function_name = sindent + _function_name;
				xout << Logcommand;
				xout << std::format("{0:<5}", " ");
				xout << Logapi;
				xout << std::format("{0:<5}", " ");
				xout << Logfunction;
				xout << std::format("{0:<20}{1:<45}{2:<10}{3:<25}",
					_function_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					_request_time.format("%D %H:%M start")
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_base_block_stop(int _indent, std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();


				std::string sindent(_indent, ' ');
				_function_name = sindent + _function_name;
				xout << Logcommand;
				xout << std::format("{0:<5}", " ");
				xout << Logapi;
				xout << std::format("{0:<5}", " ");
				xout << Logfunction;
				xout << std::format("{0:<20}{1:<45}{2:<10}{3:<25}",
					_function_name,
					trim(_message, 45),
					GetCurrentThreadId(),
					std::format("{0} secs", _elapsed_seconds)
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}



			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_table_start(std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_table_logging) {
				log_base_block_start(0, _function_name, _message, _request_time, _file, _line);
			}
		}

		virtual void log_table_stop(std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_table_logging) {
				log_base_block_stop(0, _function_name, _message, _elapsed_seconds, _file, _line);
			}
		}


		virtual void log_json_start(std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_poco_logging) {
				log_base_block_start(2, _function_name, _message, _request_time, _file, _line);
			}
		}

		virtual void log_json_stop(std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_poco_logging) {
				log_base_block_stop(2, _function_name, _message, _elapsed_seconds, _file, _line);
			}
		}

		virtual void log_poco_start(std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_poco_logging) {
				log_base_block_start(2, _function_name, _message, _request_time, _file, _line);
			}
		}

		virtual void log_poco_stop(std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_poco_logging) {
				log_base_block_stop(2, _function_name, _message, _elapsed_seconds, _file, _line);
			}
		}

		virtual void log_block_start(std::string _function_name, std::string _message, date_time _request_time, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_block_logging) {
				log_base_block_start(4, _function_name, _message, _request_time, _file, _line);
			}
		}

		virtual void log_block_stop(std::string _function_name, std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			if (enable_json_block_logging) {
				log_base_block_stop(4, _function_name, _message, _elapsed_seconds, _file, _line);
			}
		}

		virtual void log_information(std::string _message, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			xout << Loginformation;
			if (_message.size() < 45) {
				xout << std::format("{0:<45}{1:<10}{2:<25}",
					_message,
					GetCurrentThreadId(),
					" "
				);
				file_line(_file, _line);
			}
			else {
				xout << _message;
			}
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_activity(std::string _message, date_time _time, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();
				xout << Logactivity;
			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			xout << Loginformation;
			xout << std::format("{0:<45}{1:<10}{2:<25}",
				trim(_message, 45),
				GetCurrentThreadId(),
				_time.format("%D %H:%M")
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_activity(std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			xout << Logactivity;
			xout << std::format("{0:<45}{1:<10}{2:<25}",
				trim(_message, 45),
				GetCurrentThreadId(),
				std::format("{0} secs", _elapsed_seconds)
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_put(std::string _message, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{

			if (not enable_put_logging)
				return;

			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

				xout << Logcommand;
				xout << std::format("{0:<5}", " ");
				xout << Logapi;
				xout << std::format("{0:<5}", " ");
				xout << Logfunction;
				xout << std::format("{0:<20}", " ");
				xout << Loginformation;
				xout << std::format("{0:<45}{1:<10}{2:<25}",
					trim(_message, 45),
					GetCurrentThreadId(),
					std::format("{0} secs", _elapsed_seconds)
				);
				file_line(_file, _line);
				xout << Normal;
				xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_adapter(std::string _message)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();
				xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			if (_message.find("Intel") != std::string::npos) {
				xout << Logintel;
			}
			else if (_message.find("AMD") != std::string::npos) {
				xout << Logamd;
			}
			else if (_message.find("NVIDIA") != std::string::npos) {
				xout << Lognvidia;
			}
			xout << std::format("{0:<80}",
				trim(_message, 80)
			);
			xout << Normal;
			xout << std::endl;

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_warning(std::string _message, const char *_file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			xout << Logwarning;
			xout << std::format("{0:<80}",
				trim(_message, 80)
			);
			file_line(_file, _line);
			xout << Normal;
			xout << std::endl;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		virtual void log_exception(std::exception exc, const char* _file = nullptr, int _line = 0)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();
				xout << Logcommand;
			xout << std::format("{0:<5}", " ");
			xout << Logapi;
			xout << std::format("{0:<5}", " ");
			xout << Logfunction;
			xout << std::format("{0:<20}", " ");
			xout << Logexception;
			xout << std::format("{0:<80}",
				trim(exc.what(), 80)
			);
			file_line(_file, _line);
			xout << std::endl;
			xout << Normal;
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);
		}

		template <typename json_type> void log_json(json_type _src, int _indent = 2)
		{
			::EnterCriticalSection(&log_lock);

			try {
				auto& xout = get_log_file();

			std::string sindent(_indent, ' ');
			if (_src.object())
			{
				auto members = _src.get_members();
				for (auto member : members) {
					auto body = member.second;
					auto key = member.first;
					std::string name = sindent + key;
					xout << Logcommand;
					xout << std::format("{0:<5}", " ");
					xout << Logapi;
					xout << std::format("{0:<5}", " ");
					xout << Logfunction;
					xout << std::format("{0:<20}", " ");
					xout << Loginformation;
					xout << std::format("{1:<30}:", " ", name);
					if (body.object())
					{
						std::string rightArrow = "->";
						xout << std::format("{0:<45}:", "{object}" + rightArrow) << std::endl;
						log_json<json_type>(body, _indent + 4);
					}
					else if (body.array())
					{
						std::string rightArrow = "->";
						xout << std::format("{0:<45}:", "[array]" + rightArrow ) << std::endl;
						log_json<json_type>(body, _indent + 4);
					}
					else
					{
						log_json<json_type>(body, _indent + 4);
					}
				}
			}
			else if (_src.array())
			{
				bool too_many = false;
				int max_items = _src.size();
				if (max_items > 10) {
					max_items = 10;
					too_many = true;
				}
				for (int i = 0; i < max_items; i++)
				{
					auto body = _src.get_element(i);
					std::string sindex = sindent + std::to_string(i);
					xout << Logcommand;
					xout << std::format("{0:<5}", " ");
					xout << Logapi;
					xout << std::format("{0:<5}", " ");
					xout << Logfunction;
					xout << std::format("{0:<20}", " ");
					xout << Loginformation;
					xout << std::format("{0:<30}:", sindex);
					if (body.object())
					{
						std::string rightArrow = "->";
						xout << std::format("{0:<45}:", "{object}" + rightArrow) << std::endl;
						log_json<json_type>(body, _indent + 4);
					}
					else if (body.array())
					{
						std::string rightArrow = "->";
						xout << std::format("{0:<45}:", "[array]" + rightArrow) << std::endl;
						log_json<json_type>(body, _indent + 4);
					}
					else
					{
						log_json<json_type>(body, _indent + 4);
					}
					xout << std::endl;
				}

				if (too_many) {
					std::string sindex = sindent + std::to_string(max_items);
					xout << Logcommand;
					xout << std::format("{0:<5}", " ");
					xout << Logapi;
					xout << std::format("{0:<5}", " ");
					xout << Logfunction;
					xout << std::format("{0:<20}", " ");
					xout << Loginformation;
					xout << std::format("{0:<30}:", "", sindex);
					xout << "*more than this*";
					xout << std::endl;
				}
			}
			else {
				std::string v = _src.to_json();
				v = trim(v, 45);
				xout << std::format("{0:<45}", v);
				xout << std::endl;
			}

			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}


			::LeaveCriticalSection(&log_lock);

		}
	};

	system_monitoring_interface* system_monitoring_interface::global_mon = nullptr;
	system_monitoring_interface* system_monitoring_interface::active_mon = nullptr;

	void log_warning(const std::string& _src)
	{
		if (system_monitoring_interface::global_mon) {
			system_monitoring_interface::active_mon->log_warning(_src);
		}
	}

}

#endif
