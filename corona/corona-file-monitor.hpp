

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

		json_file_watcher()
		{
			;
		}

		void poll(application *_app, std::function<void(json&)> _handler)
		{
			json_parser jp;
			relative_ptr_type type;

			try {
				if (_app->file_exists(filename)) {
					file f = _app->open_file(filename, file_open_types::open_existing);
					if (f.success()) {
						f.read([this, _handler](std::string& _string) 
							{
								if (_string.empty()) {
									return;
								}
								if (_string != last_contents) {
									last_contents = _string;
									json_parser jp;
									json temp = jp.parse_object(_string);
									contents = temp;
									if (_handler) {
										_handler(contents);
									}
								}
							}
						);
					}
				}

			}
			catch (std::exception exc)
			{
				std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << filename << " failed:" << exc.what() << std::endl;
			}
		}
	};

}

#endif
