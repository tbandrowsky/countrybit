
#ifndef CORONA_FILE_MONITOR
#define CORONA_FILE_MONITOR

namespace corona
{

	class json_file_watcher
	{
		std::string last_contents;
	public:
		std::string file_name;
		json		contents;

		json_file_watcher()
		{
			;
		}

		relative_ptr_type poll_contents(application* _app, json& item)
		{
			relative_ptr_type pt = null_row;
			if (_app->file_exists(file_name)) {
				try {
					std::string file_contents = read_all_string(file_name);
					if (last_contents != file_contents) {
						last_contents = file_contents;
						json_parser jp;
						json tcontents = jp.parse_object(file_contents);
						if (!tcontents.empty()) {
							contents = tcontents;
							pt = 1;
						}
					}
					item = contents;
				}
				catch (std::exception exc)
				{
					std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << file_name << " failed:" << exc.what() << std::endl;
				}
			}
			return pt;
		}

		file_transaction<relative_ptr_type> poll(application *_app)
		{
			json_parser jp;
			try {
				if (_app->file_exists(file_name)) {
					file f = _app->open_file(file_name, file_open_types::open_existing);
					if (f.success()) {
						auto fsize = f.size();
						buffer b(fsize + 1);
						auto result = co_await f.read(0, b.get_ptr(), fsize);
						if (result.success) {
							crypto crypter;
							if (b.is_safe_string()) {
								std::string s_contents = b.get_ptr();
								if (s_contents != last_contents) {
									last_contents = s_contents;
									json temp_contents = jp.parse_object(s_contents);
									contents = temp_contents;
									if (!jp.parse_errors.size()) {
										co_return true;
									}
								}
							}
						}
					}
				}

			}
			catch (std::exception exc)
			{
				std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << file_name << " failed:" << exc.what() << std::endl;
			}
			co_return null_row;
		}
	};

}

#endif
