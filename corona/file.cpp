
#include "file.h"

namespace countrybit
{

	namespace system
	{


		class read_job : public io_job
		{
		public:


			read_job(std::coroutine_handle<> _handle) : io_job(_handle)
			{
				;
			}

			void read(HANDLE hdirectory, uint64_t location, void* destination, uint32_t bytes_to_read)
			{
				LARGE_INTEGER li;
				li.QuadPart = location;
				ovp.OffsetHigh = li.HighPart;
				ovp.Offset = li.LowPart;
				::ReadFileEx(hdirectory, destination, bytes_to_read, (OVERLAPPED*)this, nullptr);
			}
		};

		class write_job : public io_job
		{
		public:

			write_job(std::coroutine_handle<> _handle) : io_job(_handle)
			{
				;
			}

			void write(HANDLE hdirectory, uint64_t location, void* destination, uint32_t bytes_to_read)
			{
				LARGE_INTEGER li;
				li.QuadPart = location;
				ovp.OffsetHigh = li.HighPart;
				ovp.Offset = li.LowPart;
				::WriteFileEx(hdirectory, destination, bytes_to_read, (OVERLAPPED*)this, nullptr);
			}
		};

		void file::io_awaiter::await_suspend(std::coroutine_handle<> handle)
		{
			if (io_type == io_types::io_write)
			{
				write_job writeomatic(handle);
				writeomatic.write(hfile, location, buffer, buffer_length);
			}
			else if (io_type == io_types::io_read)
			{
				read_job readomatic(handle);
				readomatic.read(hfile, location, buffer, buffer_length);
			}
		}

		void file::io_awaiter::configure(io_types _io_type, HANDLE _hfile, job_queue* _queue, uint64_t _location, void* _buffer, int _buffer_length)
		{
			io_type = _io_type;
			hfile = _hfile;
			queue = _queue;
			location = _location;
			buffer = (char *)_buffer;
			buffer_length = _buffer_length;
		}
	}
}
