/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This contains a file access object along with its associated awaiters.
file i/o in corona is based on C++20 co_await with io completion ports.

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
void* operator new(size_t size, class_type& _src, uint64_t, void*, int) {
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

	static const int JsonTableMaxNumberOfLevels = 40;
	static const int JsonTableMaxLevel = JsonTableMaxNumberOfLevels - 1;

	template <typename transaction_result> class user_transaction;
	template <typename transaction_result> class table_transaction;
	template <typename transaction_result> class file_transaction;

	class file_batch;
	class json_node;

	struct index_header_struct
	{
	public:
		int64_t		header_node_location;
		int64_t		count;
		long		level;
	};

	template <typename data> class poco_node;

	template <typename transaction_result> 	class user_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			user_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				user_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		user_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		user_transaction()
		{
		}

		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}

			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

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
		file_path			file_name;
		HANDLE				hfile;

		int64_t				location;
		DWORD				size;
		void*				buffer;

		file_command_request() = default;
		file_command_request(const file_command_request& fcr) = default;
		file_command_request(file_command_request&& fcr) = default;

		file_command_request& operator = (const file_command_request& fcr) = default;
		file_command_request& operator = (file_command_request&& fcr) = default;

		file_command_request(file_commands _command, const file_path& _file_name, HANDLE _file, int64_t _location, DWORD _size, void* _buffer) :
			command(_command),
			file_name(_file_name),
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

		std::coroutine_handle<> caller_routine;

		struct promise_type
		{
			file_command_request				request;
			file_command_result					result;

			promise_type(file_command_request& _request)
			{
				request = _request;
			}

			file_command get_return_object() 
			{
				file_command  fbr(this);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(file_command_result _result) 
			{
				result = _result;
			}

			void unhandled_exception() 
			{

			}
		};

		file_command_request  request;
		file_command_result   result;
		HANDLE				  whore_wait;

		file_command()
		{
			request = {};
			result = {};
		}

		file_command(promise_type *_owner_promise) 
		{
			request = _owner_promise->request;
			result = {};
		}

		file_command(const file_command& _src) = default;
		file_command(file_command&& _src) = default;
		file_command& operator = (const file_command& _src) = default;
		file_command& operator = (file_command&& _src) = default;

		virtual ~file_command()  noexcept
		{
			;
		}

		virtual bool queued(job_queue* _callingQueue)
		{
			bool r = false;

			switch (request.command) {
			case file_commands::read:
				r = ::ReadFile(request.hfile, (void*)request.buffer, request.size, nullptr, (LPOVERLAPPED)&container);
				break;
			case file_commands::write:
				r = ::WriteFile(request.hfile, (void*)request.buffer, request.size, nullptr, (LPOVERLAPPED)&container);
				break;
			}

			// because these are asynch, 
			if (!r) {
				r = ::GetLastError();
				return r == ERROR_IO_PENDING;
			}
			return false;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			jn.shouldDelete = false;

			result.bytes_transferred = _bytesTransferred;
			result.result.success = _success;

			if (whore_wait)
				::SetEvent(whore_wait);

			if (caller_routine) {
				caller_routine();
			}

			return jn;
		}

		bool await_ready()
		{
			bool ready = false;
			return ready;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			caller_routine = handle;

			container.ovp = {};
			LARGE_INTEGER li;

			li.QuadPart = request.location;
			container.ovp.Offset = li.LowPart;
			container.ovp.OffsetHigh = li.HighPart;

			global_job_queue->listen_job(this);
		}

		file_command_result await_resume()
		{
			try
			{
				return result;
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}
		}

		operator file_command_result()
		{
			return result;
		}

		file_command_result wait()
		{
			whore_wait = CreateEvent(NULL, FALSE, FALSE, FALSE);

			caller_routine = nullptr;

			container.ovp = {};
			LARGE_INTEGER li;

			li.QuadPart = request.location;
			container.ovp.Offset = li.LowPart;
			container.ovp.OffsetHigh = li.HighPart;

			global_job_queue->listen_job(this);

			::WaitForSingleObject(whore_wait, INFINITE);

		}

	};

	template <typename transaction_result> 	class database_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			database_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		database_transaction()
		{
		}

		// awaiter

		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result> 	class database_composed_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			database_composed_transaction get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_composed_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_composed_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		database_composed_transaction()
		{
		}

		// awaiter

		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}


		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}				
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result> 	class database_method_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			database_method_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_method_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_method_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		database_method_transaction()
		{
		}

		// awaiter

		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};


	template <typename transaction_result>
	class table_array_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			table_array_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_array_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_array_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		table_array_transaction()
		{
		}

		// awaiter
		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		// this creates the 
		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result>
	class table_group_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			table_group_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_group_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_group_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		table_group_transaction()
		{
		}

		// awaiter
		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		// this creates the 
		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result>
	class table_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			table_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		table_transaction()
		{
		}

		// awaiter
		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		// this creates the 
		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result>
	class table_method_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			table_method_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_method_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_method_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		table_method_transaction()
		{
		}

		// awaiter
		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}


		// this creates the 
		transaction_result await_resume()
		{
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};


	class compare_transaction
	{
	public:

		struct promise_type
		{
			int64_t m_value;

			promise_type()
			{
				m_value = 0;
			}

			compare_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				compare_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int64_t value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		compare_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		compare_transaction()
		{
		}

		// awaiter
		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		// this creates the 
		int64_t await_resume()
		{
			int64_t result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		int64_t wait()
		{
			int64_t result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator int64_t()
		{
			int64_t result = coro.promise().m_value;
			return result;
		}
	};

	template <typename transaction_result> 
	class file_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
			}

			file_transaction  get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		file_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		file_transaction()
		{
		}

		// awaiter

		bool await_ready()
		{
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		transaction_result await_resume()
		{
			transaction_result result;

			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			return result;
		}

		transaction_result wait()
		{
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}	
	};


	class file_batch
	{

	public:

		struct promise_type
		{
			int64_t m_value;

			promise_type()
			{
				m_value = 0;
			}

			file_batch get_return_object() {
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_batch fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int64_t value) {
				m_value = value;
			}

			void unhandled_exception() {
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		file_batch(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
		}

		file_batch()
		{
		}

		// awaiter

		bool await_ready()
		{
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
		}

		int64_t await_resume()
		{
			int64_t result;
			if (coro) {
				result = coro.promise().m_value;
				coro.resume();
			}

			return result;
		}

		int64_t wait()
		{
			int64_t result = -1;

			if (coro) {
				result = coro.promise().m_value;
				coro.resume();
			}

			return result;
		}

		operator int()
		{
			int64_t result = coro.promise().m_value;
			return result;
		}
	};

	class file
	{
		file_path		file_name;
		HANDLE			hfile;
		lockable		size_locker;
		HANDLE			resize_event;
		job_queue*		queue;


		// you actually can't put stuff like this here, 
		// because you will have multiple file users.  
		// so tempting.
		os_result		last_result;

		void open(job_queue* _queue, const file_path& _filename, file_open_types _file_open_type)
		{
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
					last_result = osr;
					return;
				}
			}
			iwstring<600> wfile_name = file_name;

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
				hfile = ::CreateFile2(wfile_name.c_str(), (GENERIC_READ | GENERIC_WRITE), 0, disposition, &params);
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
					std::string temp = file_name + ":" + osr.message;
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
					std::string temp = file_name + ":" + osr.message;
					throw std::logic_error(temp.c_str());
				}
			}
			last_result = os_result(0);
		}

	public:

		void copy(const file& _src)
		{
			file_name = _src.file_name;
			if (_src.hfile && _src.hfile != INVALID_HANDLE_VALUE) {
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
			: queue(_queue), file_name(_filename), hfile(INVALID_HANDLE_VALUE),
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
			: queue(_queue), file_name(_filename), hfile(INVALID_HANDLE_VALUE),
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

		uint64_t add(uint64_t _bytes_to_add) // adds size_bytes to file and returns the position of the start
		{
			if (hfile == INVALID_HANDLE_VALUE)
				return 0;

			::WaitForSingleObject(resize_event, INFINITE);
			::ResetEvent(resize_event);

			LARGE_INTEGER position, new_position;
			::GetFileSizeEx(hfile, &position);
			new_position = position;
			new_position.QuadPart += _bytes_to_add;
			::SetFilePointerEx(hfile, new_position, nullptr, FILE_BEGIN);
			::SetEndOfFile(hfile);
			::SetEvent(resize_event);

			return new_position.QuadPart;
		}

		file_command write(uint64_t location, void* _buffer, int _buffer_length)
		{		
			file_command_request fcr(file_commands::write, file_name, hfile, location, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			return fc;
		}

		file_command read(uint64_t location, void* _buffer, int _buffer_length)
		{
			file_command_request fcr(file_commands::write, file_name, hfile, location, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			return fc;
		}

		file_command append(void* _buffer, int _buffer_length)
		{
			int64_t file_position = add(_buffer_length);

			file_command_request fcr(file_commands::write, file_name, hfile, file_position, _buffer_length, _buffer);
			file_command fc;
			fc.request = fcr;
			return fc;
		}

		uint64_t size()
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
			return resize_event != NULL && hfile != INVALID_HANDLE_VALUE && last_result.success;
		}

		os_result& result()
		{
			return last_result;
		}
	};
}


#endif
