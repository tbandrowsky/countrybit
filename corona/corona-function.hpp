
#ifndef CORONA_FUNCTION_H
#define CORONA_FUNCTION_H

#include "corona-windows-all.h"
#include "corona-queue.hpp"
#include "corona-json.hpp"

#include <functional>
#include <coroutine>
#include <iostream>
#include <memory>
#include <compare>

namespace corona
{

	const int WM_CORONA_JOB_COMPLETE = WM_APP + 1;
	const int WM_CORONA_TASK_COMPLETE = WM_APP + 2;

	template <typename parameter_type> class task_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		std::shared_ptr<parameter_type> params;
		std::function<void (std::shared_ptr<parameter_type>) > on_run;

		task_job() : job()
		{
		}

		task_job(std::coroutine_handle<> _handle, 
			HANDLE _event, 
			std::shared_ptr<parameter_type> _params) : handle(_handle), event(_event), params(_params)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			std::cout << "job start:" << GetCurrentThreadId() << std::endl;

			try 
			{
				if (on_run) {
					on_run(params);
				}

				if (handle) {
					handle.resume();
					handle.destroy();
				}
			}
			catch (...)
			{
				;
			}

			jn.setSignal(event);
			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class ui_task_result
	{
	public:
		json parameters;
		runnable on_gui;

		ui_task_result(json _param, runnable _on_gui) : parameters(_param), on_gui(_on_gui)
		{
			;
		}
	};

	class ui_task_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		json params;
		std::function<void(json)> on_gui;
		std::function<void(json)> on_run;

		ui_task_job() : job()
		{
		}

		ui_task_job(std::coroutine_handle<> _handle,
			HANDLE _event,
			json _params,
			std::function<void(json)> _on_gui, 
			std::function<void(json)> _on_run) :
			handle(_handle), 
			event(_event), 
			params(_params), 
			on_gui(_on_gui),
			on_run(_on_run)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			std::cout << "ui task start:" << GetCurrentThreadId() << std::endl;

			json blank_params;

			runnable cylon = [this]() -> void
			{
				//on_gui();
			};

			auto result = new ui_task_result(
				blank_params,
				cylon
			);

			try
			{
				if (on_run)
				{
					on_run(params);
				}
				if (handle) {
					handle.resume();
					handle.destroy();
				}
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, TRUE, (LPARAM)result);
			}
			catch (...)
			{
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
			}
			
			jn.setSignal(event);
			std::cout << "ui task end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class general_ui_job : public job
	{
	public:
		runnable on_gui;
		runnable on_run;

		general_ui_job() : job()
		{
		}

		general_ui_job(
			runnable _on_run,
			runnable _on_gui) :
			on_run(_on_run),
			on_gui(_on_gui)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			json empty_default;

//			std::cout << "gen ui task start:" << GetCurrentThreadId() << std::endl;

			auto result = new ui_task_result(empty_default, on_gui);

			try
			{
				if (on_run)
				{
					on_run();
				}
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, TRUE, (LPARAM)result);
			}
			catch (...)
			{
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
			}

//			std::cout << "gen ui task end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	template <typename Param> concept task_io_params = requires(Param param) {
		param.bytes_transferred = 0;
		param.success = false;
	};

	template <typename Param>
	requires(task_io_params<Param>)
	class async_io_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		Param* params;

		async_io_job() : job()
		{
			params = {};
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			params->bytes_transferred = _bytesTransferred;
			params->success = _success;
			jn.setSignal(event);
			std::cout << "job start:" << GetCurrentThreadId() << std::endl;
			if (handle) {
				handle.resume();
				handle.destroy();
			}
			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = false;
			return jn;
		}
	};

	class file_batch;

	class file_batch_job : public job
	{
	public:
		file_batch				*metask;
		HANDLE					event;
		std::coroutine_handle<> routine;

		file_batch_job() : job()
		{
			event = {};
			metask = nullptr;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
	};

	class file_batch
	{
	public:
		HANDLE hevent;
		std::shared_ptr<file_batch_job> fbj;

		struct promise_type
		{
			int m_value;

			promise_type()
			{
				std::cout << "file_batch::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			file_batch get_return_object() {
				std::cout << "file_batch::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_batch fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(int value) {
				std::cout << "file_batch::promise return_value:" << " " << value << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				std::cout << "file_batch::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		file_batch(std::coroutine_handle<promise_type> _promise_coro)
		{
			hevent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			coro = _promise_coro;
			fbj = std::make_shared<file_batch_job>();
			fbj->event = hevent;
			fbj->routine = coro;
			fbj->metask = this;
			std::cout << "file_batch: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		file_batch()
		{
			hevent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			std::cout << "file_batch: empty ctor:" << ::GetCurrentThreadId() << std::endl;
			fbj = std::make_shared<file_batch_job>();
			fbj->event = hevent;
			fbj->routine = nullptr;
			fbj->metask = this;
		}

		// awaiter

		bool await_ready() 
		{
			std::cout << "file_batch::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			return true;
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<> handle)
		{
			std::cout << "file_batch::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			global_job_queue->add_job(fbj.get());
			std::cout << "file_batch: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			std::cout << "file_batch: batch complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle.resume();
		}

		int await_resume()
		{
			std::cout << "file_batch::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			int result = coro.promise().m_value;
			return result;
		}

		int wait()
		{
			std::cout << "file_batch::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			int result = -1;

			std::cout << "file_batch::create_job:" << this << " " << GetCurrentThreadId() << std::endl;
			global_job_queue->add_job(fbj.get());
			std::cout << "file_batch: wait for job" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			std::cout << "file_batch: complete" << " " << ::GetCurrentThreadId() << std::endl;

			if (coro) {
				result = coro.promise().m_value;
			}
			return result;
		}

		operator int()
		{
			int result = coro.promise().m_value;
			return result;
		}
	};

	job_notify file_batch_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jn;

		std::cout << "file_batch_job: execution." << GetCurrentThreadId() << std::endl;

		if (routine) {
			routine.resume();
		}

		if (metask) {
			std::cout << "file_batch_job: executed: " << _bytesTransferred << std::endl;
			jn.setSignal(metask->hevent);
		}

		std::cout << "file_result_job end:" << GetCurrentThreadId() << std::endl;

		jn.shouldDelete = false;
		return jn;
	}

	class file_task;

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

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
	};

	class file_task_result
	{
	public:
		bool success;
		int  bytes_transferred;
	};

	enum file_function {
		no_function,
		read_function,
		write_function
	};

	class file_task 
	{

	public:

		file_function fun;
		HANDLE file;
		HANDLE hevent;
		int64_t location;
		char* buffer;
		int size;
		int bytes_transferred;
		bool success;
		std::shared_ptr<file_task_job> frj;
		std::function<void(HANDLE _fi, int64_t _location, char* _buffer, int _size)> chumpy;

		struct promise_type
		{
			file_task_result m_value;
			std::coroutine_handle<promise_type> promise_coro;

			promise_type()
			{
				std::cout << "file_task::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			file_task get_return_object() { 
				std::cout << "file_batch_result::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				file_task fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(file_task_result value) {
				std::cout << "file_task::promise return_value:" << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				std::cout << "file_task::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		file_task()
		{
			fun = file_function::no_function;
			frj = std::make_shared<file_task_job>();
			file = nullptr;
			location = 0;
			buffer = nullptr;
			size = 0;
			bytes_transferred = 0;
			success = false;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			coro = nullptr;
		}

		file_task(const file_task& _src)
		{
			fun = _src.fun;
			frj = std::make_shared<file_task_job>();
			file = _src.file;
			location = _src.location;
			buffer = _src.buffer;
			size = _src.size;
			bytes_transferred = _src.bytes_transferred;
			success = _src.success;
			hevent = CreateEvent(NULL, FALSE, FALSE, nullptr);
			frj = _src.frj;
		}

		file_task(file_task&& _src)
		{
			fun = _src.fun;
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
			frj->container.ovp.Offset = li.LowPart;
			frj->container.ovp.OffsetHigh = li.HighPart;
			frj->handle = coro;
			frj->metask = this;
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
			frj->container.ovp.Offset = li.LowPart;
			frj->container.ovp.OffsetHigh = li.HighPart;
			frj->handle = coro;
			frj->metask = this;			
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
			}
		}

		virtual ~file_task()
		{
			::CloseHandle(hevent);
		}

		bool await_ready() {
			return false;
		}

		void await_suspend(std::coroutine_handle<promise_type> handle)
		{
			std::cout << "file_task: suspend file_task" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
			std::cout << "file_task: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			std::cout << "file_task:io complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle.resume();
		}

		// this creates the 
		void await_suspend(std::coroutine_handle<file_batch::promise_type> handle)
		{
			std::cout << "file_task: suspend file_batch_result" << " " << ::GetCurrentThreadId() << std::endl;
			initiate();
			std::cout << "file_task: suspend initiate" << " " << ::GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			std::cout << "file_task:io complete" << " " << ::GetCurrentThreadId() << std::endl;
			handle.resume();
		}

		file_task_result await_resume()
		{
			file_task_result ftr = {};
			ftr.success = success;
			ftr.bytes_transferred = bytes_transferred;
			std::cout << "file_task: await_resume:" << ftr.success << " bytes:" << ftr.bytes_transferred << " " << ::GetCurrentThreadId() << std::endl;
			return ftr;
		}

		operator file_task_result()
		{
			file_task_result ftr = {};
			ftr.success = success;
			ftr.bytes_transferred = bytes_transferred;
			std::cout << "file_task: result:" << ftr.success << " bytes:" << ftr.bytes_transferred << " " << ::GetCurrentThreadId() << std::endl;
			return ftr;
		}
	};

	job_notify file_task_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jn;

		std::cout << "file_task_job: receiving IO results " << GetCurrentThreadId() << std::endl;
		
		if (metask) {
			std::cout << "file_task_job: bytes transferred: " << _bytesTransferred << std::endl;
			metask->bytes_transferred = _bytesTransferred;
			metask->success = _success;
			jn.setSignal(metask->hevent);
		}

		if (handle) {
			handle.resume();
			handle.destroy();
		}

		std::cout << "file_task_job: end:" << GetCurrentThreadId() << std::endl;

		jn.shouldDelete = false;
		return jn;
	}

	template <typename IOParams> struct async_io_task : public std::suspend_always
	{
	public:
		job_queue* queue;
		IOParams	params;
		std::function<bool(HANDLE hevent, IOParams* _src, std::coroutine_handle<> _handle)> runner;

		void configure(job_queue* _queue, const IOParams& _params, std::function<bool(HANDLE hevent, IOParams* _src, std::coroutine_handle<> _handle)> _runner)
		{
			std::cout << "configure:" << this << GetCurrentThreadId() << std::endl;
			queue = _queue;
			params = _params;
			runner = _runner;
		}

		struct promise_type
		{
			IOParams m_value;

			promise_type()
			{
				std::cout << "async_io_task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			async_io_task<IOParams> get_return_object()
			{
				std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				async_io_task<IOParams> my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(IOParams value) {
				std::cout << "async_io_task return_value:" << value << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_always initial_suspend() {
				std::cout << "async_io_task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				std::cout << "async_io_task final_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {
				std::cout << "async_io_task unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coroutine;

		void await_suspend(std::coroutine_handle<> handle)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << this << ", async_io_task await_suspend:" << GetCurrentThreadId() << std::endl;
			if (runner) {
				if (runner(hevent, &params, handle)) {
					std::cout << this << ", async_io_task await_suspend away:" << GetCurrentThreadId() << std::endl;
					::WaitForSingleObject(hevent, INFINITE);
					std::cout << "async_io_task await_suspend finished:" << GetCurrentThreadId() << std::endl;
				}
				else 
				{
					std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
				}
			}
			::CloseHandle(hevent);
		}

		IOParams await_resume()
		{
			IOParams t = {};
			std::cout << "async_io_task await_resume:" << GetCurrentThreadId() << std::endl;
			if (coroutine) {
				t = coroutine.promise().m_value;
			}
			return t;
		}

		operator IOParams() {
			std::cout << this << ", async_io_task cast to T:" << GetCurrentThreadId() << std::endl;
			return coroutine.promise().m_value;
		}

		async_io_task(std::coroutine_handle<> _coroutine) : coroutine(_coroutine)
		{
			std::cout << typeid(*this).name() << " ctor:" << GetCurrentThreadId() << std::endl;
		}

		async_io_task()
		{
			std::cout << typeid(*this).name() << " ctor" << GetCurrentThreadId() << std::endl;
		}

	};


	template <typename T> class sync 
	{
	public:

		T value;

		struct promise_type
		{
			T m_value;

			promise_type()
			{
				m_value = {};
				std::cout << typeid(*this).name() << " promise_type(" << m_value << ")" << GetCurrentThreadId() << std::endl;
			}

			sync<T> get_return_object()
			{
				std::cout << typeid(*this).name() << " get_return_object(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				return m_value;
			}

			void return_value(T value) {
				std::cout << typeid(*this).name() << " return_value(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_never initial_suspend() {
				std::cout << typeid(*this).name() << " initial_suspend(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				std::cout << typeid(*this).name() << " final_suspend(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {}
		};

		std::coroutine_handle<> coro;

		sync()
		{
			value = {};
			coro = {};
		}

		sync(const T& _value)
		{
			value = _value;
			coro = {};
			std::cout << typeid(*this).name() << " ctor (" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		sync(const sync & _src)
		{
			value = _src.value;
			coro = {};
			std::cout << typeid(*this).name() << " ctor (" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		sync operator =(const sync& _src)
		{
			value = _src.value;
			coro = {};
			std::cout << typeid(*this).name() << " op= (" << value << ")" << GetCurrentThreadId() << std::endl;
			return *this;
		}

		bool await_ready()
		{
			std::cout << typeid(*this).name() << " await_ready(" << value << ")" <<  GetCurrentThreadId() << std::endl;
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			coro = handle;
			std::cout << typeid(*this).name() << " await_suspend(" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		T await_resume()
		{
			std::cout << typeid(*this).name() << " await_resume.(" << value << ")" <<  GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return value;
		}

		operator T() {
			std::cout << typeid(*this).name() << " cast (" << value << ")" <<  GetCurrentThreadId() << std::endl;
			return value;
		}

	};

	struct task
	{
		struct promise_type
		{
			task get_return_object() { return {}; }
			std::suspend_never initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			void return_void() {}
			void unhandled_exception() {}
		};
	};

	class threadomatic 
	{
	public:

		static void run(runnable _runnable)
		{
			general_job* gj = new general_job(_runnable);
			global_job_queue->add_job(gj);
		}

		static void run_complete(runnable _runnable, runnable _ui_complete)
		{
			general_ui_job* guj = new general_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
		}

		template <typename IOParams> static void run_io(job_queue* queue, IOParams params, std::function<bool(HANDLE hevent, IOParams* _src)> runner)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << "run_io_task start:" << GetCurrentThreadId() << std::endl;
			if (runner) {
				if (runner(hevent, &params)) {
					std::cout << "run_io await_suspend away : " << GetCurrentThreadId() << std::endl;
					::WaitForSingleObject(hevent, INFINITE);
					std::cout << "run_io await_suspend finished:" << GetCurrentThreadId() << std::endl;
				}
				else
				{
					std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
				}
			}
			::CloseHandle(hevent);
		}
	};
}

#endif
