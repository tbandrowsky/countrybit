

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
		std::string last_contents;
	public:
		std::string filename;
		json		contents;

		json_file_watcher()
		{
			;
		}

		relative_ptr_type poll_contents(application* _app, json& item)
		{
			relative_ptr_type pt = null_row;
			if (_app->file_exists(filename)) {
				try {
					std::string file_contents = read_all_string(filename);
					if (last_contents != file_contents) {
						last_contents = file_contents;
						json_parser jp;
						json tcontents = jp.parse_object(file_contents);
						if (not tcontents.empty()) {
							contents = tcontents;
							pt = 1;
						}
					}
					item = contents;
				}
				catch (std::exception exc)
				{
					std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << filename << " failed:" << exc.what() << std::endl;
				}
			}
			return pt;
		}

		relative_ptr_type poll(application *_app)
		{
			json_parser jp;
			try {
				if (_app->file_exists(filename)) {
					file f = _app->open_file(filename, file_open_types::open_existing);
					if (f.success()) {
						auto fsize = f.size();
						buffer b(fsize + 1);
						auto result = f.read(0, b.get_ptr(), fsize);
						if (result.success) {
							crypto crypter;
							if (b.is_safe_string()) {
								std::string s_contents = b.get_ptr();
								if (s_contents != last_contents) {
									last_contents = s_contents;
									json temp_contents = jp.parse_object(s_contents);
									contents = temp_contents;
									if (not jp.parse_errors.size()) {
										return true;
									}
								}
							}
						}
					}
				}

			}
			catch (std::exception exc)
			{
				std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << filename << " failed:" << exc.what() << std::endl;
			}
			return null_row;
		}
	};

}

#endif
