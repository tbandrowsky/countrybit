

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
Polls a file to see if its contents have changed, then 
parses the json

Notes

For Future Consideration
*/
#ifndef CORONA_FILE_MONITOR
#define CORONA_FILE_MONITOR

namespace corona
{

	class json_file_watcher
	{
	public:
		std::string filename;
		json		contents;
		std::string last_contents;
		std::string error_message;

		json_file_watcher()
		{
			;
		}

		bool poll(application *_app, std::function<void(json&)> _handler)
		{
			json_parser jp;
			relative_ptr_type type;
            bool success = false;

			try {
				if (_app->file_exists(filename)) {
					std::string file_string = read_all_string(filename);
					if (file_string.empty()) {
                        error_message = std::format("File {0} is empty", filename);
						return false;
					}
					if (file_string != last_contents) {
						last_contents = file_string;
						json_parser jp;
						json temp = jp.parse_object(file_string);
						contents = temp;
						if (_handler) {
							_handler(contents);
						}
                        success = true;
					}
				}
			}
			catch (std::exception exc)
			{
				error_message = std::format("Exception:{0}, {1}", filename, exc.what());
				system_monitoring_interface::global_mon->log_warning(error_message.c_str(), __FILE__, __LINE__);
                success = false;
			}

			return success;
		}
	};

}

#endif
