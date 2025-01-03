/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This file is for monitoring for directory changes.

Notes
Needs unit test script

For Future Consideration
*/


#ifndef CORONA_DIRECTORY_H
#define CORONA_DIRECTORY_H

namespace corona
{


	class directory_change_instance
	{
	public:
		job_queue* queue;
		file_path		directory_name;
		HANDLE					hdirectory;
		char* buffer_bytes;
		DWORD					buffer_size;
		os_result				last_result;
		DWORD					bytes_transferred;
		BOOL					success;

		directory_change_instance() :
			queue(nullptr),
			directory_name(""),
			hdirectory(INVALID_HANDLE_VALUE),
			buffer_bytes(nullptr),
			buffer_size(0),
			bytes_transferred(0),
			success(false)
		{

		}

		~directory_change_instance() = default;

		directory_change_instance(job_queue* _queue, const object_path& _directory_name, HANDLE _hdirectory) :
			queue(_queue),
			directory_name(_directory_name),
			hdirectory(_hdirectory),
			bytes_transferred(0),
			success(false),
			buffer_bytes(nullptr),
			buffer_size(0)
		{

		}

		class iterator
		{
			directory_change_instance* base;
			FILE_NOTIFY_INFORMATION* current;

		public:

			struct value_ref
			{
				object_path directory_name;
				object_path filename;

				value_ref& from(directory_change_instance* _base, FILE_NOTIFY_INFORMATION* change)
				{
					directory_name = _base->directory_name;
					if (change) {
						filename.copy(change->FileName, change->FileNameLength);
					}
					else {
						filename = "";
					}
				}
			};

			value_ref current_value;

			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = value_ref;
			using pointer = value_ref*;  // or also value_type*
			using reference = value_ref&;  // or also value_type&

			iterator(directory_change_instance* _base, FILE_NOTIFY_INFORMATION* _current) :
				base(_base),
				current(_current)
			{
				current_value.from(base, current);
			}

			iterator() : base(nullptr), current(0)
			{

			}

			iterator& operator = (const iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				return *this;
			}

			inline value_ref operator *()
			{
				return current_value;
			}

			inline iterator begin() const
			{
				return iterator(base, current);
			}

			inline iterator end() const
			{
				return iterator(base, nullptr);
			}

			inline iterator operator++()
			{
				if (current)
				{
					if (current->NextEntryOffset)
					{
						current = (FILE_NOTIFY_INFORMATION*)((char*)current + current->NextEntryOffset);
					}
					else
					{
						current = nullptr;
					}
				}
				return iterator(base, current);
			}

			inline iterator operator++(int)
			{
				iterator tmp(*this);
				operator++();
				return tmp;
			}

			bool operator == (const iterator& _src) const
			{
				return _src.current == current;
			}

			bool operator != (const iterator& _src)
			{
				return _src.current != current;
			}

		};

		directory_change_instance::iterator begin()
		{
			FILE_NOTIFY_INFORMATION* current = (this->bytes_transferred > 0) ? (FILE_NOTIFY_INFORMATION*)this->buffer_bytes : nullptr;
			return iterator(this, current);
		}

		directory_change_instance::iterator end()
		{
			return iterator(this, nullptr);
		}
	};

	class directory_changes_job : public async_io_job<directory_change_instance>
	{
	public:

		void run()
		{
			::ReadDirectoryChangesW(params->hdirectory, params->buffer_bytes, params->buffer_size, false, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr, (LPOVERLAPPED)this, nullptr);
		}
	};

	class directory
	{

		directory_change_instance instance;

	protected:

		directory(job_queue* _queue, std::string _directory_name)
			: instance(_queue, _directory_name, INVALID_HANDLE_VALUE)
		{
			instance.hdirectory = CreateFileA(directory_name.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, nullptr, OPEN_EXISTING);

			if (instance.hdirectory == INVALID_HANDLE_VALUE) {
				os_result osr;
				{
					instance.last_result = osr;
					return;
				}
			}
			auto hfileport = CreateIoCompletionPort(instance.hdirectory, instance.queue->getPort(), 0, instance.queue->getThreadCount());
			if (hfileport == nullptr)
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

		/*

		auto get_changes(char* _buffer_bytes, DWORD _buffer_size)
		{
			directory_change_instance dci;
			dci = instance;
			dci.buffer_bytes = _buffer_bytes;
			dci.buffer_size = _buffer_size;
			async_io_task<directory_changes_job, directory_change_instance> aw;
			aw.configure(instance.queue, dci);
			return aw;
		}

		*/

		os_result& result()
		{
			return instance.last_result;
		}
	};

}

#endif
