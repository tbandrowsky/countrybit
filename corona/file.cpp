
#include "corona.h"

namespace corona
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

	}
}
