
/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is the application root class.  It can be used for both 
console and desktop applications.  It manages a threadpool
that is used pervasively by Corona.  It also provides the entry
to open and work with files asynchronously.

This is a singleton.  

Notes

For Future Consideration
This is a singleton.  having a public constructor is wrong.
*/

#ifndef CORONA_APPLICATION_H
#define CORONA_APPLICATION_H

namespace corona
{

	class application
	{
		static application* home;

	public:

		application()
		{
			global_job_queue = std::make_unique<job_queue>();
			global_job_queue->start(0);
			home = this;
		}

		~application()
		{
			global_job_queue->shutDown();
		}

		static application* get_application()
		{
			if (!home) 
			{				
				home = new application();
			}
			return home;
		}

		void wait()
		{
			global_job_queue->waitForEmptyQueue();
		}

		file open_file(file_path filename, file_open_types _file_open_type)
		{
			file f(global_job_queue.get(), filename, _file_open_type);
			return f;
		}

		file open_file(KNOWNFOLDERID folderId, file_path filename, file_open_types _file_open_type)
		{
			file f(global_job_queue.get(), folderId, filename, _file_open_type);
			return f;
		}

		file create_file(KNOWNFOLDERID folderId, file_path filename)
		{
			return file(global_job_queue.get(), folderId, filename, file_open_types::create_always);
		}

		file create_file(file_path filename)
		{
			return file(global_job_queue.get(), filename, file_open_types::create_always);
		}

		void add_job(job* _job)
		{
			global_job_queue->add_job(_job);
		}

		virtual void log(const char* _msg)
		{
			std::cout << _msg << std::endl;
		}

	};

	file_batch application_tests()
	{
		try {
			auto app = application::get_application();

			auto my_file = app->create_file("file.txt");

			my_file.add(32);

			char buffer[10] = { 0 };

			strcpy_s(buffer, "test1");
			co_await my_file.write( 0, buffer, 5);
			strcpy_s(buffer, "test2");
			co_await my_file.write( 5, buffer, 5);
			co_await my_file.read( 0, buffer, 5);

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


	application* application::home;

}

#endif
