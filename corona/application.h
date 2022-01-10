#pragma once

#include "file.h"

namespace countrybit
{
	namespace system
	{

		using namespace std;

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

			static application& get_application()
			{
				return *home;
			}

			void wait()
			{
				queue.waitForEmptyQueue();
			}

			file open_file(std::string filename, system::file_open_types _file_open_type)
			{
				file f(&queue, filename, _file_open_type);
				return f;
			}

			file create_file(std::string filename)
			{
				return file(&queue, filename, system::file_open_types::create_new);
			}

			void add_job(job* _job)
			{
				queue.postJobMessage(_job);
			}

		private:

		};

	}

}

