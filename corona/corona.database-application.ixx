module;

import <string>;
import <cstring>;
import <coroutine>;
import <compare>;

export module corona.database:application;

import :function;
import :file;
import :file;
import :queue;

//application* application::home;

export class application
{
	job_queue queue;

	static application* home;

public:

	application()
	{
		queue.start(0);
		home = this;
	}

	~application()
	{
		queue.shutDown();
	}

	static application* get_application()
	{
		return home;
	}

	void wait()
	{
		queue.waitForEmptyQueue();
	}

	file open_file(file_path filename, file_open_types _file_open_type)
	{
		file f(&queue, filename, _file_open_type);
		return f;
	}

	file create_file(file_path filename)
	{
		return file(&queue, filename, file_open_types::create_new);
	}

	void add_job(job* _job)
	{
		queue.postJobMessage(_job);
	}

	virtual void log(const char* _msg)
	{
		std::cout << _msg << std::endl;
	}

};


export sync<int> application_tests()
{
	try {
		auto app = application::get_application();

		auto my_file = app->create_file("file.txt");

		my_file.add(32);

		char buffer[10] = { 0 };

		strcpy_s(buffer, "test1");
		co_await my_file.write(0, buffer, 5);
		strcpy_s(buffer, "test2");
		co_await my_file.write(5, buffer, 5);
		co_await my_file.read(0, buffer, 5);

		if (strcmp(buffer, "test1") != 0)
		{
			std::cout << "read / write test failed" << std::endl;
		}

		co_return true;
	}
	catch (std::exception exc)
	{
		std::cout << __LINE__ << " " << exc.what() << std::endl;
		co_return false;
	}
}
