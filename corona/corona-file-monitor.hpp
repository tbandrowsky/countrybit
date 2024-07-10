
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

		file_transaction<relative_ptr_type> poll()
		{
			json_parser jp;
			try {
				application* _app = application::get_application();
				if (!file_name.empty()) {
					std::cout << "polling " << file_name << std::endl;
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
									json temp_contents = jp.parse_object(s_contents);
									if (!jp.parse_errors.size()) {
										last_contents = contents;
										contents = temp_contents;
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
				std::cout << exc.what() << std::endl;
			}
			co_return false;
		}
	};

}

#endif
