#pragma once

#include <vector>
#include "queue.h"
#include <coroutine>
#include "messages.h"

namespace countrybit
{
	namespace system {

		enum class file_open_types
		{
			create_new,
			open_existing,
			open_always
		};

		class file
		{
			std::string filename;
			HANDLE hfile;
			job_queue* queue;
			lockable size_locker;
			HANDLE resize_event;

		protected:

			file(job_queue* _queue, std::string _filename, file_open_types _file_open_type)
				: queue(_queue),
				filename(_filename),
				hfile(INVALID_HANDLE_VALUE),
				resize_event(NULL)
			{
				DWORD disposition;

				switch (_file_open_type)
				{
				case file_open_types::create_new:
					disposition = CREATE_NEW;
					break;
				case file_open_types::open_existing:
					disposition = OPEN_EXISTING;
					break;
				case file_open_types::open_always:
					disposition = OPEN_ALWAYS;
					break;
				default:
					throw std::runtime_error("Invalid enum to open file " + _filename);
				}

				resize_event = ::CreateEvent(NULL, TRUE, TRUE, NULL);
				if (resize_event == NULL) {
					os_result osr;
					throw std::runtime_error(osr.message);
				}
				hfile = ::CreateFileA(_filename.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, NULL, disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
				if (hfile == INVALID_HANDLE_VALUE) {
					os_result osr;
					{
						CloseHandle(resize_event);
						throw std::runtime_error(osr.message);
					}
				}
				auto hfileport = ::CreateIoCompletionPort(hfile, queue->getPort(), 0, queue->getThreadCount());
				if (hfileport == NULL)
				{
					os_result osr;
					{
						CloseHandle(resize_event);
						CloseHandle(hfile);
						throw std::runtime_error(osr.message);
					}
				}
			}

			friend class application;

		public:

			file() :
				queue(nullptr),
				hfile(INVALID_HANDLE_VALUE),
				resize_event(NULL)
			{
				;
			}

			file(file&& _srcFile) noexcept
			{
				filename = _srcFile.filename;
				hfile = _srcFile.hfile;
				queue = _srcFile.queue;
				resize_event = _srcFile.resize_event;
				_srcFile.filename = "";
				_srcFile.hfile = INVALID_HANDLE_VALUE;
				_srcFile.queue = nullptr;
				_srcFile.resize_event = NULL;
			}

			file& operator = (file&& _srcFile) noexcept
			{
				filename = _srcFile.filename;
				hfile = _srcFile.hfile;
				queue = _srcFile.queue;
				resize_event = _srcFile.resize_event;
				_srcFile.filename = "";
				_srcFile.hfile = INVALID_HANDLE_VALUE;
				_srcFile.queue = nullptr;
				_srcFile.resize_event = NULL;
				return *this;
			}

			file(const file& src) = delete;
			file& operator = (const file& _srcFile) = delete;

			~file()
			{
				if (resize_event != NULL)
				{
					::CloseHandle(resize_event);
				}

				if (hfile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hfile);
				}
			}

			enum class io_types
			{
				io_read,
				io_write
			};

			struct io_awaiter : std::suspend_always
			{
				io_types io_type;
				HANDLE hfile;
				job_queue* queue;
				uint64_t location;
				char* buffer;
				int buffer_length;

				void await_suspend(std::coroutine_handle<> handle);
				void configure(io_types _io_type, HANDLE _hfile, job_queue* _queue, uint64_t _location, void* _buffer, int _buffer_length);
			};

			uint64_t add(uint64_t _bytes_to_add) // adds size_bytes to file and returns the position of the start
			{
				uint64_t position;
				LARGE_INTEGER file_size;

				::WaitForSingleObject(resize_event, INFINITE);
				::ResetEvent(resize_event);

				file_size.QuadPart = 0;
				::GetFileSizeEx(hfile, &file_size);
				position = file_size.QuadPart;
				file_size.QuadPart += _bytes_to_add;
				::SetFilePointerEx(hfile, file_size, NULL, FILE_BEGIN);
				::SetEndOfFile(hfile);
				::SetEvent(resize_event);
				return position;
			}

			auto write(uint64_t location, void* _buffer, int _buffer_length)
			{
				::WaitForSingleObject(resize_event, INFINITE);
				io_awaiter aw;
				aw.configure(io_types::io_write, hfile, queue, location, _buffer, _buffer_length);
				return aw;
			}

			auto read(uint64_t location, void* _buffer, int _buffer_length)
			{
				::WaitForSingleObject(resize_event, INFINITE);
				io_awaiter aw;
				aw.configure(io_types::io_read, hfile, queue, location, _buffer, _buffer_length);
				return aw;
			}
		};
	}
}
