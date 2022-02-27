#pragma once

#include <vector>
#include "queue.h"
#include <coroutine>
#include "messages.h"
#include "string_box.h"
#include "function.h"

namespace countrybit
{
	namespace system 
	{

		class directory_change_instance
		{
		public:
			job_queue*  queue;
			file_path	directory_name;
			HANDLE		hdirectory;
			char		*buffer_bytes;
			DWORD		buffer_size;
			os_result	last_result;
			DWORD		bytes_transferred;
			BOOL		success;

			directory_change_instance() :
				queue(nullptr),
				directory_name(""),
				hdirectory(INVALID_HANDLE_VALUE)
			{

			}

			~directory_change_instance() = default;

			directory_change_instance(job_queue* _queue, const file_path& _directory_name, HANDLE _hdirectory) :
				queue(_queue),
				directory_name(_directory_name),
				hdirectory(_hdirectory),
				bytes_transferred(0),
				success(false),
				buffer_bytes(nullptr),
				buffer_size(0)
			{

			}
		};

		class directory_changes_job : public async_io_job<directory_change_instance>
		{
		public:
			
			void run()
			{
				::ReadDirectoryChangesW(params->hdirectory, params->buffer_bytes, params->buffer_size, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, (LPOVERLAPPED)this, NULL);
			}
		};

		class directory 
		{

			directory_change_instance instance;

		protected:

			directory(job_queue* _queue, const file_path& _directory_name )
				: instance(_queue, _directory_name, INVALID_HANDLE_VALUE)
			{
				DWORD disposition;

				instance.hdirectory = ::CreateFileA(instance.directory_name.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, NULL, disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
				if (instance.hdirectory == INVALID_HANDLE_VALUE) {
					os_result osr;
					{
						instance.last_result = osr;
						return;
					}
				}
				auto hfileport = ::CreateIoCompletionPort(instance.hdirectory, instance.queue->getPort(), 0, instance.queue->getThreadCount());
				if (hfileport == NULL)
				{
					os_result osr;
					{
						CloseHandle(instance.hdirectory);
						instance.hdirectory = INVALID_HANDLE_VALUE;
						instance.last_result = osr;
						return;
					}
				}
			}

			friend class application;

		public:

			directory() 
			{
				;
			}

			directory(directory&& _srcFile) noexcept
			{
				instance = _srcFile.instance;
				_srcFile.instance = {};
			}

			directory& operator = (directory&& _srcFile) noexcept
			{
				instance = _srcFile.instance;
				_srcFile = {};
				return *this;
			}

			directory(const directory& src) = delete;
			directory& operator = (const directory& _srcFile) = delete;

			~directory()
			{
				if (instance.hdirectory != INVALID_HANDLE_VALUE)
				{
					CloseHandle(instance.hdirectory);
					instance.hdirectory = INVALID_HANDLE_VALUE;
				}
			}

			auto get_changes(char *_buffer_bytes, DWORD _buffer_size)
			{
				directory_change_instance dci;
				dci = instance;
				dci.buffer_bytes = _buffer_bytes;
				dci.buffer_size = _buffer_size;
				async_io_task<directory_changes_job, directory_change_instance> aw;
				aw.configure(instance.queue, dci);
				return aw;
			}

			os_result& result()
			{
				return instance.last_result;
			}
		};
	}
}
