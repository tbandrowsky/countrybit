#pragma once

#include "file.h"

namespace countrybit
{
	namespace system
	{
		class application
		{
			job_queue queue;

		public:

			application()
			{
				queue.start(0);
			}

			~application()
			{
				queue.shutDown();
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
		};

	}

}

