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
	const int debug_file = 0;

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

	static const int JsonTableMaxNumberOfLevels = 40;
	static const int JsonTableMaxLevel = JsonTableMaxNumberOfLevels - 1;

	template <typename transaction_result> class user_transaction;
	template <typename transaction_result> class table_transaction;
	template <typename transaction_result> class file_transaction;

	class file_batch;
	class file_task;
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
				debug_functions&& std::cout << "user_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			user_transaction  get_return_object() {
				debug_functions&& std::cout << "user_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				user_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "user_transaction::promise return_value, thread:" <<  GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "user_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		user_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "user_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		user_transaction()
		{
			debug_functions&& std::cout << "user_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		bool await_ready()
		{
			debug_functions&& std::cout << "user_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
			debug_functions&& std::cout << "user_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "user_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}

			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "user_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "user_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}
	};

	class file_task_result
	{
	public:
		bool success;
		int64_t bytes_transferred;
		int64_t location;
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
				debug_functions&& std::cout << "user_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			database_transaction  get_return_object() {
				debug_functions&& std::cout << "user_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "user_transaction::promise return_value, thread:" << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "user_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "database_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		database_transaction()
		{
			debug_functions&& std::cout << "database_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "database_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
			debug_functions&& std::cout << "database_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "database_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "database_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "database_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "database_composed_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			database_composed_transaction get_return_object() {
				debug_functions&& std::cout << "database_composed_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_composed_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "database_composed_transaction::promise return_value, thread:" << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "database_composed_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_composed_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "database_composed_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		database_composed_transaction()
		{
			debug_functions&& std::cout << "database_composed_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "database_composed_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();

			debug_functions&& std::cout << "database_composed_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
		}


		transaction_result await_resume()
		{
			debug_functions&& std::cout << "database_composed_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}				
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "database_composed_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "database_composed_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "database_method_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			database_method_transaction  get_return_object() {
				debug_functions&& std::cout << "database_method_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				database_method_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "database_method_transaction::promise return_value, thread:" << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "database_method_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		database_method_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "database_method_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		database_method_transaction()
		{
			debug_functions&& std::cout << "database_method_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "database_method_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "database_method_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "database_method_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}

			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "database_method_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "database_method_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "table_array_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			table_array_transaction  get_return_object() {
				debug_functions&& std::cout << "table_array_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_array_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "table_array_transaction::promise return_value,thread:" << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "table_array_transaction::promise unhandled exception, thread:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_array_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "table_array_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		table_array_transaction()
		{
			debug_functions&& std::cout << "table_array_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter
		bool await_ready()
		{
			debug_functions&& std::cout << "table_array_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();
			debug_functions&& std::cout << "table_array_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		transaction_result await_resume()
		{
			debug_functions&& std::cout << "table_array_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "table_array_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "table_array_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "table_group_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			table_group_transaction  get_return_object() {
				debug_functions&& std::cout << "table_group_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_group_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "table_group_transaction::promise return_value,thread:" << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "table_group_transaction::promise unhandled exception, thread:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_group_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "table_group_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		table_group_transaction()
		{
			debug_functions&& std::cout << "table_group_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter
		bool await_ready()
		{
			debug_functions&& std::cout << "table_group_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			handle();

			debug_functions&& std::cout << "table_group_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		transaction_result await_resume()
		{
			debug_functions&& std::cout << "table_group_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "table_group_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "table_group_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "table_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			table_transaction  get_return_object() {
				debug_functions&& std::cout << "table_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "table_transaction::promise return_value,thread:" << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "table_transaction::promise unhandled exception, thread:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "table_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		table_transaction()
		{
			debug_functions&& std::cout << "table_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter
		bool await_ready()
		{
			debug_functions&& std::cout << "table_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "table_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			debug_functions&& std::cout << "table_transaction::await_suspend post coro:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
			debug_functions&& std::cout << "table_transaction::await_suspend post handle:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		transaction_result await_resume()
		{
			debug_functions&& std::cout << "table_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}

			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "table_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}

			debug_functions&& std::cout << "table_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "table_method_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			table_method_transaction  get_return_object() {
				debug_functions&& std::cout << "table_method_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				table_method_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "table_method_transaction::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "table_method_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		table_method_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "table_method_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		table_method_transaction()
		{
			debug_functions&& std::cout << "table_method_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter
		bool await_ready()
		{
			debug_functions&& std::cout << "table_method_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "table_method_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}


		// this creates the 
		transaction_result await_resume()
		{
			debug_functions&& std::cout << "table_method_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "table_method_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}


			debug_functions&& std::cout << "table_method_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "compare_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			compare_transaction  get_return_object() {
				debug_functions&& std::cout << "compare_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				compare_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int64_t value) {
				debug_functions&& std::cout << "compare_transaction::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "compare_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		compare_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "compare_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		compare_transaction()
		{
			debug_functions&& std::cout << "compare_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter
		bool await_ready()
		{
			debug_functions&& std::cout << "compare_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "compare_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}

		// this creates the 
		int64_t await_resume()
		{
			debug_functions&& std::cout << "compare_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			int64_t result;
			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		int64_t wait()
		{
			debug_functions&& std::cout << "compare_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}


			debug_functions&& std::cout << "compare_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "file_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			file_transaction  get_return_object() {
				debug_functions&& std::cout << "file_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "file_transaction::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "file_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		file_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "file_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		file_transaction()
		{
			debug_functions&& std::cout << "file_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "file_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "file_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "file_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;

			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}


			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "file_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
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
				std::cout << exc.what() << std::endl;
			}


			debug_functions&& std::cout << "file_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

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
				debug_functions&& std::cout << "file_batch::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			file_batch get_return_object() {
				debug_functions&& std::cout << "file_batch::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_batch fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int64_t value) {
				debug_functions&& std::cout << "file_batch::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "file_batch::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		file_batch(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "file_batch: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		file_batch()
		{
			debug_functions&& std::cout << "file_batch: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "file_batch::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "file_batch::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}

		int64_t await_resume()
		{
			debug_functions&& std::cout << "file_batch::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			int64_t result;
			if (coro) {
				result = coro.promise().m_value;
			}

			return result;
		}

		int64_t wait()
		{
			debug_functions&& std::cout << "file_batch::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			int64_t result = -1;

			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
			}

			debug_functions&& std::cout << "file_batch: complete" << " " << ::GetCurrentThreadId() << std::endl;

			return result;
		}

		operator int()
		{
			int64_t result = coro.promise().m_value;
			return result;
		}
	};

	enum file_task_function
	{
		void_function = 0,
		read_function = 1,
		write_function = 2,
		append_function = 3,
	};

	class file_task
	{
		class file_task_job : public job
		{
		public:
			std::coroutine_handle<> handle;
			HANDLE					event;
			file_task* metask;

			file_task_job() : job()
			{
				handle = {};
				event = {};
				metask = nullptr;
			}

			job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
			{
				job_notify jn;

				debug_functions&& std::cout << "file_task_job: receiving IO results " << GetCurrentThreadId() << std::endl;

				if (metask) {
					debug_functions&& std::cout << "file_task_job: bytes transferred: " << _bytesTransferred << std::endl;
					metask->bytes_transferred = _bytesTransferred;
					metask->success = _success;
/*					LARGE_INTEGER li;
					li.HighPart = container.ovp.OffsetHigh;
					li.LowPart = container.ovp.Offset;
					metask->location = li.QuadPart;
					*/
					jn.setSignal(metask->hevent);
				}

				if (handle) {
					handle.resume();
					handle.destroy();
				}

				debug_functions&& std::cout << "file_task_job: end:" << GetCurrentThreadId() << std::endl;

				jn.shouldDelete = false;
				return jn;
			}
		};

	public:

		HANDLE file;
		HANDLE hevent;
		int64_t location;
		char* buffer;
		int size;
		int bytes_transferred;
		bool success;
		file_task_job frj;
		file_task_function fun;
		std::function<void(HANDLE _fi, int64_t _location, char* _buffer, int _size)> chumpy;


		struct promise_type
		{
			file_task_result m_value;
			std::coroutine_handle<promise_type> promise_coro;

			promise_type()
			{
				debug_functions&& std::cout << "file_task::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			file_task get_return_object() {
				debug_functions&& std::cout << "file_batch_result::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_task fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(file_task_result value) {
				debug_functions&& std::cout << "file_task::promise return_value:" << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "file_task::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		file_task()
		{
			file = nullptr;
			location = 0;
			buffer = nullptr;
			size = 0;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			coro = nullptr;
			fun = {};
		}

		file_task(const file_task& _src)
		{
			file = _src.file;
			location = _src.location;
			buffer = _src.buffer;
			size = _src.size;
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			frj = _src.frj;
			fun = _src.fun;
		}

		file_task(file_task&& _src)
		{
			file = _src.file;
			location = _src.location;
			buffer = _src.buffer;
			size = _src.size;
			hevent = _src.hevent;
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
			_src.hevent = nullptr;
			frj = std::move(_src.frj);
			fun = _src.fun;
		}

		file_task(std::coroutine_handle<promise_type> _coro)
		{
			location = 0;
			buffer = nullptr;
			size = 0;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			coro = _coro;
			fun = {};
		}

		void read(HANDLE _fi, int64_t _location, char* _buffer, int _size)
		{
			fun = read_function;
			file = _fi;
			location = _location;
			buffer = _buffer;
			size = _size;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = location;
			frj.container.ovp.Offset = li.LowPart;
			frj.container.ovp.OffsetHigh = li.HighPart;
			frj.handle = coro;
			frj.metask = this;
		}

		void write(HANDLE _fi, int64_t _location, char* _buffer, int _size)
		{
			fun = write_function;
			file = _fi;
			location = _location;
			buffer = _buffer;
			size = _size;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = location;
			frj.container.ovp.Offset = li.LowPart;
			frj.container.ovp.OffsetHigh = li.HighPart;
			frj.handle = coro;
			frj.metask = this;
		}

		void append(HANDLE _fi, char* _buffer, int _size)
		{
			fun = write_function;
			file = _fi;
			buffer = _buffer;
			size = _size;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = location;
			frj.container.ovp.Offset = 0xFFFFFFFF;
			frj.container.ovp.OffsetHigh = 0xFFFFFFFF;
			frj.handle = coro;
			frj.metask = this;
		}

		void initiate()
		{
			switch (fun) {
			case read_function:
				ReadFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj.container);
				break;
			case write_function:
				WriteFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj.container);
				break;
			case append_function:
				WriteFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj.container);
				break;
			}
		}

		virtual ~file_task()
		{
			::CloseHandle(hevent);
		}

		bool await_ready() {
			debug_functions&& std::cout << "file_task: await_ready" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "file_task: suspend file_batch_result" << " " << ::GetCurrentThreadId() << std::endl;
			debug_functions&& std::cout << "file_task: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			debug_functions&& std::cout << "file_task:io complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle();
			debug_functions&& std::cout << "file_task:suspend complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		file_task_result await_resume()
		{
			file_task_result ftr = {};
			ftr.success = success;
			ftr.bytes_transferred = bytes_transferred;
			ftr.location = location;
			debug_functions&& std::cout << "file_task: await_resume:" << ftr.success << " bytes:" << ftr.bytes_transferred << " " << ::GetCurrentThreadId() << std::endl;
			return ftr;
		}

		operator file_task_result()
		{
			file_task_result ftr = {};
			ftr.success = success;
			ftr.bytes_transferred = bytes_transferred;
			ftr.location = location;
			debug_functions&& std::cout << "file_task: result:" << ftr.success << " bytes:" << ftr.bytes_transferred << " " << ::GetCurrentThreadId() << std::endl;
			return ftr;
		}
	};

	class file
	{
		file_result instance;
		lockable size_locker;
		HANDLE resize_event;
		LARGE_INTEGER end_of_file_position;

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
					std::string temp = instance.file_name + ":" + osr.message;
					throw std::logic_error(temp.c_str());
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
					std::string temp = instance.file_name + ":" + osr.message;
					throw std::logic_error(temp.c_str());
				}
			}
			::GetFileSizeEx(instance.hfile, &end_of_file_position);
			instance.last_result = os_result(0);
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
			_srcFile.instance = file_result();
		}

		file& operator = (file&& _srcFile) noexcept
		{
			instance = _srcFile.instance;
			resize_event = _srcFile.resize_event;
			_srcFile.instance = file_result();
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

			if (instance.hfile == INVALID_HANDLE_VALUE)
				return 0;

			::WaitForSingleObject(resize_event, INFINITE);
			::ResetEvent(resize_event);

			position = end_of_file_position.QuadPart;
			end_of_file_position.QuadPart += _bytes_to_add;
			::SetFilePointerEx(instance.hfile, end_of_file_position, &end_of_file_position, FILE_BEGIN);
			::SetEndOfFile(instance.hfile);
			::SetEvent(resize_event);
			return position;
		}

		file_task write(uint64_t location, void* _buffer, int _buffer_length)
		{
			debug_file && std::cout << "write file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;
			
			ft.write(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		file_task read(uint64_t location, void* _buffer, int _buffer_length)
		{
			debug_file && std::cout << "read file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;

			ft.read(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		file_task append(void* _buffer, int _buffer_length)
		{
			debug_file&& std::cout << "append file:" << end_of_file_position.QuadPart <<  ", thread:" << GetCurrentThreadId() << std::endl;

			int64_t file_position = add(_buffer_length);

			file_task ft;

			ft.write(instance.hfile, file_position, (char*)_buffer, _buffer_length);

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
