

#include "pch.h"

namespace corona
{
	namespace database
	{
		application *application::home;

		corona::database::sync<int> application_tests()
		{
			try {
				auto app = application::get_application();

				auto my_file = app->create_file("file.txt");

				my_file.add(32);

				char buffer[10];
				strcpy_s(buffer, "test1");
				co_await my_file.write(0, buffer, 5);
				strcpy_s(buffer, "test2");
				co_await my_file.write(5, buffer, 5);
				co_return true;
			}
			catch (std::exception exc)
			{
				std::cout << __LINE__ << " " << exc.what() << std::endl;
				co_return false;
			}
		}
	}
}
