#pragma once

namespace corona
{
	namespace database
	{
		class application
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

		corona::database::sync<int> application_tests();
	}

}

