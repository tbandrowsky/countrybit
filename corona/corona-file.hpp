#ifndef CORONA_FILE_H
#define CORONA_FILE_H

#include "corona-windows-lite.h"
#include "corona-messages.hpp"
#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-string_box.hpp"
#include "corona-constants.hpp"
#include "corona-messages.hpp"
#include "corona-function.hpp"

#include <exception>
#include <stdexcept>
#include <iostream>
#include <compare>

namespace corona
{
	class file_result
	{
	public:
		job_queue* queue;
		file_path	file_name;
		HANDLE		hfile;
		char*		buffer_bytes;
		DWORD		buffer_size;
		uint64_t	location;
		DWORD		bytes_transferred;
		BOOL		success;
		os_result	last_result;

		file_result() :
			queue(nullptr),
			file_name(""),
			hfile(INVALID_HANDLE_VALUE)
		{

		}

		~file_result() = default;

		file_result(job_queue* _queue, const file_path& _file_name, HANDLE _hfile) :
			queue(_queue),
			file_name(_file_name),
			hfile(_hfile),
			buffer_bytes(nullptr),
			buffer_size(0),
			location(0),
			bytes_transferred(0),
			success(false)
		{

		}
	};

	enum class file_open_types
	{
		create_new,
		create_always,
		open_existing,
		open_always
	};

	class file
	{
		file_result instance;
		lockable size_locker;
		HANDLE resize_event;

		void open(job_queue* _queue, const file_path& _filename, file_open_types _file_open_type)
		{
			instance = file_result(_queue, _filename, INVALID_HANDLE_VALUE);
			resize_event = 0;

			DWORD disposition;

			switch (_file_open_type)
			{
			case file_open_types::create_new:
				disposition = CREATE_NEW;
				break;
			case file_open_types::create_always:
				disposition = CREATE_ALWAYS;
				break;
			case file_open_types::open_existing:
				disposition = OPEN_EXISTING;
				break;
			case file_open_types::open_always:
				disposition = OPEN_ALWAYS;
				break;
			default:
				throw std::runtime_error("Invalid enum to open file ");
			}

			resize_event = CreateEventA(NULL, true, true, NULL);
			if (resize_event == NULL) {
				os_result osr;
				{
					instance.last_result = osr;
					return;
				}
			}
			iwstring<600> file_name = instance.file_name;

			CREATEFILE2_EXTENDED_PARAMETERS params = { 0 };

			params.dwSize = sizeof(params);
			params.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
			params.dwSecurityQosFlags = 0;
			params.hTemplateFile = NULL;
			params.lpSecurityAttributes = NULL;
			params.dwFileFlags = FILE_FLAG_OVERLAPPED;

			instance.hfile = ::CreateFile2(file_name.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, disposition, &params);
			if (instance.hfile == INVALID_HANDLE_VALUE) {
				os_result osr;
				{
					CloseHandle(resize_event);
					instance.last_result = osr;
					throw std::logic_error(osr.message.c_str());
				}
			}
			HANDLE hport = instance.queue->getPort();
			auto hfileport = ::CreateIoCompletionPort(instance.hfile, hport, 0, 0);
			if (hfileport == NULL)
			{
				os_result osr;
				{
					CloseHandle(resize_event);
					CloseHandle(instance.hfile);
					instance.hfile = INVALID_HANDLE_VALUE;
					resize_event = NULL;
					instance.last_result = osr;
					throw std::logic_error(osr.message.c_str());
				}
			}
		}

	public:

		file() :
			resize_event(NULL)
		{
			;
		}

		file(file&& _srcFile) noexcept
		{
			instance = _srcFile.instance;
			resize_event = _srcFile.resize_event;
			_srcFile.instance = {};
		}

		file& operator = (file&& _srcFile) noexcept
		{
			instance = _srcFile.instance;
			resize_event = _srcFile.resize_event;
			_srcFile.instance = {};
			return *this;
		}

		file(job_queue* _queue, KNOWNFOLDERID _folder_id, const file_path& _filename, file_open_types _file_open_type)
			: instance(_queue, _filename, INVALID_HANDLE_VALUE),
			resize_event(NULL)
		{
			wchar_t* wide_path = nullptr;
			::SHGetKnownFolderPath(_folder_id, KF_FLAG_DEFAULT, NULL, &wide_path );
			if (wide_path) 
			{
				istring<2048> temp = wide_path;
				temp += "\\";
				temp += _filename;
				open(_queue, temp, _file_open_type);
			}
		}

		file(job_queue* _queue, const file_path& _filename, file_open_types _file_open_type)
			: instance(_queue, _filename, INVALID_HANDLE_VALUE),
			resize_event(NULL)
		{
			open(_queue, _filename, _file_open_type);
		}

		file(const file& src) = delete;
		file& operator = (const file& _srcFile) = delete;

		~file()
		{
			if (resize_event != NULL)
			{
				::CloseHandle(resize_event);
				resize_event = INVALID_HANDLE_VALUE;
			}

			if (instance.hfile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(instance.hfile);
				instance.hfile = INVALID_HANDLE_VALUE;
			}
		}

		uint64_t add(uint64_t _bytes_to_add) // adds size_bytes to file and returns the position of the start
		{
			uint64_t position;
			LARGE_INTEGER file_size;

			if (instance.hfile == INVALID_HANDLE_VALUE)
				return 0;

			::WaitForSingleObject(resize_event, INFINITE);
			::ResetEvent(resize_event);

			file_size.QuadPart = 0;
			::GetFileSizeEx(instance.hfile, &file_size);
			position = file_size.QuadPart;
			file_size.QuadPart += _bytes_to_add;
			::SetFilePointerEx(instance.hfile, file_size, NULL, FILE_BEGIN);
			::SetEndOfFile(instance.hfile);
			::SetEvent(resize_event);
			return position;
		}

		file_task write(uint64_t location, void* _buffer, int _buffer_length)
		{
			std::cout << "write file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;
			
			ft.write(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		file_task read(uint64_t location, void* _buffer, int _buffer_length)
		{
			std::cout << "read file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;

			ft.read(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		uint64_t size()
		{
			if (instance.hfile == INVALID_HANDLE_VALUE)
				return 0;
			LARGE_INTEGER file_size;
			::WaitForSingleObject(resize_event, INFINITE);
			file_size.QuadPart = 0;
			::GetFileSizeEx(instance.hfile, &file_size);
			return file_size.QuadPart;
		}

		bool success()
		{
			return resize_event != NULL && instance.hfile != INVALID_HANDLE_VALUE && instance.last_result.success;
		}

		os_result& result()
		{
			return instance.last_result;
		}
	};

}

#endif
