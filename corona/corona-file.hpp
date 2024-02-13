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

	template <typename transaction_result> class user_transaction;
	template <typename parent_awaiter, typename transaction_result> class table_transaction;
	template <typename parent_awaiter, typename transaction_result> class file_transaction;
	template <typename parent_awaiter> class file_batch;
	template <typename parent_awaiter> class file_task;


	struct empty_awaiter
	{
		struct promise_type
		{
			promise_type()
			{
			}

			int  get_return_object() {
				return 0;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int value) {
			}

			void unhandled_exception() {
			}
		};
	};

	using empty_coroutine = std::coroutine_handle<empty_awaiter::promise_type>;

	template <typename transaction_result> 	class user_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = 0;
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
				debug_functions&& std::cout << "user_transaction::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
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

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "user_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			return false;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "user_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "user_transaction: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "user_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
			}
			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "user_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result = {};

			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
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

	template <typename transaction_result, typename parent_coro = empty_coroutine>
	class table_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = 0;
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
				debug_functions&& std::cout << "table_transaction::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "table_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
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
			return false;
		}

		// this creates the 
		void await_suspend(parent_coro handle)
		{
			debug_functions&& std::cout << "table_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "table_transaction: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "table_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "table_transaction: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "table_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
			}
			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "table_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result = {};

			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
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


	template <typename transaction_result, typename parent_awaiter = empty_awaiter> 
	class file_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = 0;
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
			return false;
		}

		void await_suspend(std::coroutine_handle<typename parent_awaiter::promise_type> handle)
		{
			debug_functions&& std::cout << "table_transaction::await_suspend, batch:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "table_transaction: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "file_batch::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "file_batch: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "file_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;
			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
			}
			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "file_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result = {};

			if (coro) {
				coro.resume();
				result = coro.promise().m_value;
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


	template <typename parent_awaiter = empty_awaiter> class file_batch
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
			return false;
		}

		void await_suspend(std::coroutine_handle<typename parent_awaiter::promise_type> handle)
		{
			debug_functions&& std::cout << "file_batch::await_suspend, batch:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "file_batch: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "file_batch::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle.resume();
			debug_functions&& std::cout << "file_batch: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
		}

		int64_t await_resume()
		{
			debug_functions&& std::cout << "file_batch::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			int64_t result;
			if (coro) {
				coro.resume();
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

	template <typename parent_awaiter> class file_task
	{
		using parent_coroutine = std::coroutine_handle<typename parent_awaiter::promise_type>;

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
			frj = std::make_shared<file_task_job>();
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
			frj = std::make_shared<file_task_job>();
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
				ReadFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj->container);
				break;
			case write_function:
				WriteFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj->container);
				break;
			case append_function:
				WriteFile(file, (void*)buffer, size, nullptr, (LPOVERLAPPED)&frj->container);
				break;
			}
		}

		virtual ~file_task()
		{
			::CloseHandle(hevent);
		}

		bool await_ready() {
			return false;
		}

		void await_suspend(std::coroutine_handle<typename parent_awaiter::promise_type> handle)
		{
			debug_functions&& std::cout << "file_task: suspend file_task" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
			debug_functions&& std::cout << "file_task: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			debug_functions&& std::cout << "file_task:io complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle.resume();
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			debug_functions&& std::cout << "file_task: suspend file_batch_result" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
			debug_functions&& std::cout << "file_task: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			debug_functions&& std::cout << "file_task:io complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle.resume();
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

		template <typename calling_awaitable> file_task<calling_awaitable> write(uint64_t location, void* _buffer, int _buffer_length)
		{
			debug_file && std::cout << "write file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;
			
			ft.write(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		template <typename calling_awaitable> file_task<calling_awaitable> read(uint64_t location, void* _buffer, int _buffer_length)
		{
			debug_file && std::cout << "read file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

			file_task ft;

			ft.read(instance.hfile, location, (char*)_buffer, _buffer_length);

			return ft;
		}

		template <typename calling_awaitable> file_task<calling_awaitable> append(uint64_t location, void* _buffer, int _buffer_length)
		{
			debug_file&& std::cout << "append file:" << location << ", thread:" << GetCurrentThreadId() << std::endl;

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
