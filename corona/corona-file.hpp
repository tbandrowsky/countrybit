/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This contains a file access object along with its associated awaiters.
file i/o in corona is based on C++20  with io completion ports.

Notes

For Future Consideration
*/


#ifndef CORONA_FILE_H
#define CORONA_FILE_H

namespace corona
{
	class file;
}

template <typename class_type>
void* operator new(size_t size, class_type& _src, int64_t, void*, int) {
	// Allocate memory and initialize with arguments
	return ::operator new(size);
}

namespace corona
{
	const int debug_file = 0;

	enum class file_open_types
	{
		create_new,
		create_always,
		open_existing,
		open_always
	};

	static const int JsonTableMaxNumberOfLevels = 32;
	static const int JsonTableMaxLevel = JsonTableMaxNumberOfLevels - 1;

	class file_batch;
	class json_node;

	struct list_block_header
	{
	public:
		int64_t		first_block;
		int64_t		last_block;
	};

	struct tree_block_header
	{
	public:
		list_block_header 	 index_list;
		int64_t				 children[256];
	};

	struct table_header_struct
	{
	public:
		int64_t			count;
		int64_t			data_root_location;
		int32_t			level;
	};

	template <typename data> class poco_node;

	enum file_commands
	{
		read = 0,
		write = 1
	};

	class file_command_result
	{
	public:
		bool		success;
		int64_t		bytes_transferred;
		int64_t		location;
		const char* buffer;
		os_result	result;
	};

	class file_command_request
	{
	public:

		file_commands		command;
		file_path			filename;
		HANDLE				hfile;

		int64_t				location;
		DWORD				size;
		void*				buffer;

		file_command_request() = default;
		file_command_request(const file_command_request& fcr) = default;
		file_command_request(file_command_request&& fcr) = default;

		file_command_request& operator = (const file_command_request& fcr) = default;
		file_command_request& operator = (file_command_request&& fcr) = default;

		file_command_request(file_commands _command, const file_path& _filename, HANDLE _file, int64_t _location, DWORD _size, void* _buffer) :
			command(_command),
			filename(_filename),
			hfile(_file),
			location(_location),
			size(_size),
			buffer(_buffer)
		{
			;
		}
	};

	class file_command : public job
	{
	public:

		file_command_request  request;
		file_command_result   result;

		HANDLE io_complete_event;

		file_command()
		{
			request = {};
			result = {};
			io_complete_event = CreateEvent(NULL, FALSE, FALSE, FALSE);
		}

		file_command(const file_command& _src) = default;
		file_command(file_command&& _src) = default;
		file_command& operator = (const file_command& _src) = default;
		file_command& operator = (file_command&& _src) = default;

		BOOL file_result;

		int file_last_error;

		virtual ~file_command()  noexcept
		{
			;
		}

		virtual bool queued(job_queue* _callingQueue) override
		{
			file_result = false;

			LARGE_INTEGER li;
			ovp = {};

			li.QuadPart = request.location;
			ovp.Offset = li.LowPart;
			ovp.OffsetHigh = li.HighPart;
			ovp.hEvent = CreateEvent(NULL, FALSE, FALSE, FALSE);

			result.buffer = (char*)request.buffer;
			result.bytes_transferred = 0;
			result.location = request.location;
			result.result = os_result(0);

			LPOVERLAPPED lp = &ovp;

			switch (request.command) {
			case file_commands::read:
				file_result = ::ReadFile(request.hfile, (void*)request.buffer, request.size, nullptr, lp);
				break;
			case file_commands::write:
				file_result = ::WriteFile(request.hfile, (void*)request.buffer, request.size, nullptr, lp);
				break;
			}

			// because these are asynch, 
			if (not file_result) {
				file_last_error = ::GetLastError();
				if (file_last_error == ERROR_IO_PENDING)
					return true;
			}
			return file_result;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			jn.shouldDelete = false;
			jn.setSignal(io_complete_event);

			result.bytes_transferred = _bytesTransferred;
			result.success = _success;

			return jn;
		}

		operator file_command_result()
		{
			return result;
		}

		file_command_result run()
		{
			global_job_queue->run_io_job(this, io_complete_event);
			return result;
		}

	};


	class file
	{
		std::string		filename;
		HANDLE			hfile;
		lockable		size_locker;
		HANDLE			resize_event;
		job_queue*		queue;


		// you actually can't put stuff like this here, 
		// because you will have multiple file users.  
		// so tempting.
		os_result		last_result;

		void open(job_queue* _queue, const std::string& _filename, file_open_types _file_open_type)
		{
			resize_event = 0;
			filename = _filename;

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
					last_result = osr;
					return;
				}
			}

			CREATEFILE2_EXTENDED_PARAMETERS params = { 0 };

			params.dwSize = sizeof(params);
			params.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
			params.dwSecurityQosFlags = 0;
			params.hTemplateFile = NULL;
			params.lpSecurityAttributes = NULL;
			params.dwFileFlags = FILE_FLAG_OVERLAPPED;

			int retry_count = 10;

			do
			{
				hfile = ::CreateFileA(filename.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, nullptr, disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr );
				if (hfile != INVALID_HANDLE_VALUE) {
					break;
				}
				os_result what_wrong;
				std::cout << what_wrong.message << std::endl;
				if (what_wrong.error_code < 90) {
					Sleep(200);
					retry_count--;
				}
				else {
					break;
				}
			} while (retry_count >= 5);
			if (hfile == INVALID_HANDLE_VALUE) {
				os_result osr;
				{
					CloseHandle(resize_event);
					last_result = osr;
					std::string temp = filename + ":" + osr.message;
					throw std::logic_error(temp.c_str());
				}
			}
			HANDLE hport = queue->getPort();
			auto hfileport = ::CreateIoCompletionPort(hfile, hport, 0, 0);
			if (hfileport == NULL)
			{
				os_result osr;
				{
					CloseHandle(resize_event);
					CloseHandle(hfile);
					hfile = INVALID_HANDLE_VALUE;
					resize_event = NULL;
					last_result = osr;
					std::string temp = filename + ":" + osr.message;
					throw std::logic_error(temp.c_str());
				}
			}
			last_result = os_result(0);
		}


	public:

		friend class file_block;
		friend class file_buffer;

		void copy(const file& _src)
		{
			filename = _src.filename;
			if (_src.hfile and _src.hfile != INVALID_HANDLE_VALUE) {
				HANDLE hprocess = GetCurrentProcess();
				DuplicateHandle(hprocess, _src.hfile, NULL, &hfile, 0, 0, 0);
			}
			resize_event = _src.resize_event;
			queue = _src.queue;
			last_result = _src.last_result;
		}

		file() = default;

		file(const file& _src) {
			copy(_src);
		}

		file& operator=(const file& _src) {
			copy(_src);
			return *this;
		}


		file(job_queue* _queue, KNOWNFOLDERID _folder_id, const file_path& _filename, file_open_types _file_open_type)
			: queue(_queue), filename(_filename), hfile(INVALID_HANDLE_VALUE),
			resize_event(NULL)
		{
			wchar_t* wide_path = nullptr;
			::SHGetKnownFolderPath(_folder_id, KF_FLAG_DEFAULT, NULL, &wide_path );
			if (wide_path) 
			{
				istring<2048> temp = wide_path;
				temp += "\\";
				temp += _filename;
				open(_queue, temp.c_str(), _file_open_type);
			}
		}

		file(job_queue* _queue, const std::string& _filename, file_open_types _file_open_type)
			: queue(_queue), filename(_filename), hfile(INVALID_HANDLE_VALUE),
			resize_event(NULL)
		{
			open(_queue, _filename, _file_open_type);
		}

		~file()
		{
			if (resize_event != NULL)
			{
				::CloseHandle(resize_event);
				resize_event = INVALID_HANDLE_VALUE;
			}

			if (hfile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hfile);
				hfile = INVALID_HANDLE_VALUE;
			}
		}

		std::string read()
		{
			file_command_result fcr;

			std::unique_ptr<char[]> buffer_bytes;

			int file_size = size();
			int blen = file_size + 8;
			buffer_bytes = std::make_unique<char[]>(file_size + 8);
			char* s = buffer_bytes.get();
			std::fill(s, s + blen, 0);
			read(0, s, file_size);

			std::string temp = s;
			return temp;
		}

		file_command_result write(const std::string& _src)
		{
			file_command_result fcr;

			if (not _src.empty()) {
				int length = _src.size();
				int64_t location = add(length);
				fcr = write(location, (void *)_src.c_str(), length);
			}
			return fcr;
		}

		file_command_result write(int64_t location, void* _buffer, int _buffer_length)
		{
			file_command_request fcr(file_commands::write, filename, hfile, location, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			file_command_result result = fc.run();
			return result;
		}

		file_command_result read(int64_t location, void* _buffer, int _buffer_length)
		{
			file_command_request fcr(file_commands::read, filename, hfile, location, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			file_command_result result = fc.run();
			return result;
		}

		int64_t add(int64_t _bytes_to_add) // adds size_bytes to file and returns the position of the start
		{
			if (hfile == INVALID_HANDLE_VALUE)
				return -1;

			::WaitForSingleObject(resize_event, INFINITE);
			::ResetEvent(resize_event);

			LARGE_INTEGER position, new_position;
			BOOL success = ::GetFileSizeEx(hfile, &position);
			if (not success) {
				os_result err;
				system_monitoring_interface::global_mon->log_warning(std::format("GetFileSizeEx failed on {0} with error #{1} - {2}", filename, err.message, err.success), __FILE__, __LINE__);
			}
			new_position = position;
			new_position.QuadPart += _bytes_to_add;
			::SetFilePointerEx(hfile, new_position, nullptr, FILE_BEGIN);
			::SetEndOfFile(hfile);
			::SetEvent(resize_event);

			return position.QuadPart;
		}

		int64_t trim(int64_t _location) // trims the file size to _location
		{
			if (hfile == INVALID_HANDLE_VALUE)
				return -1;

			::WaitForSingleObject(resize_event, INFINITE);
			::ResetEvent(resize_event);

			LARGE_INTEGER new_position;
			new_position.QuadPart = _location;
			::SetFilePointerEx(hfile, new_position, nullptr, FILE_BEGIN);
			::SetEndOfFile(hfile);
			::SetEvent(resize_event);

			return _location;
		}

		file_command_result append(void* _buffer, int _buffer_length)
		{
			int64_t file_position = add(_buffer_length);

			file_command_request fcr(file_commands::write, filename, hfile, file_position, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			return fc;
		}

		int64_t size()
		{
			if (hfile == INVALID_HANDLE_VALUE)
				return 0;
			LARGE_INTEGER file_size;
			::WaitForSingleObject(resize_event, INFINITE);
			file_size.QuadPart = 0;
			::GetFileSizeEx(hfile, &file_size);
			return file_size.QuadPart;
		}

		bool success()
		{
			return resize_event != NULL and hfile != INVALID_HANDLE_VALUE and last_result.success;
		}

		os_result& result()
		{
			return last_result;
		}
	};
}


#endif
