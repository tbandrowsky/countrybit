
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
		std::function<void(std::shared_ptr<parameter_type>_params)> run;

		task_job() : job()
		{
		}

		task_job(std::coroutine_handle<> _handle, 
			HANDLE _event, 
			std::shared_ptr<parameter_type> _params,
			std::function<void(std::shared_ptr<parameter_type>_params)> _run) : handle(_handle), event(_event), params(_params), run(_run)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			std::cout << "job start:" << GetCurrentThreadId() << std::endl;

			try 
			{
				if (run)
				{
					run(params);
				}
			}
			catch (...)
			{
				;
			}

			jn.setSignal(event);
			if (handle) handle();
			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class ui_task_result
	{
	public:
		json parameters;
		std::function<void(json)> on_gui;

		ui_task_result(json _param, std::function<void(json)> _on_gui)
			: parameters(_param),
			on_gui(_on_gui)
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
		std::function<void(json)> run;
		std::function<void(json)> on_gui;

		ui_task_job() : job()
		{
		}

		ui_task_job(std::coroutine_handle<> _handle,
			HANDLE _event,
			json _params,
			std::function<void(json)> _run,
			std::function<void(json)> _on_gui) :
			handle(_handle), 
			event(_event), 
			params(_params), 
			run(_run),
			on_gui(_on_gui)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			std::cout << "ui task start:" << GetCurrentThreadId() << std::endl;

			auto result = new ui_task_result(params, on_gui);

			try
			{
				if (run)
				{
					run(params);
					::PostMessage(NULL, WM_CORONA_TASK_COMPLETE, TRUE, (LPARAM)result);
				}
				else {
					::PostMessage(NULL, WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
				}
			}
			catch (...)
			{
				::PostMessage(NULL, WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
			}
			
			jn.setSignal(event);
			if (handle) handle();
			std::cout << "ui job end:" << GetCurrentThreadId() << std::endl;
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
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			params->bytes_transferred = _bytesTransferred;
			params->success = _success;
			jn.setSignal(event);
			std::cout << "job start:" << GetCurrentThreadId() << std::endl;
			if (handle) handle();
			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = false;
			return jn;
		}
	};

	template <typename T> class task : public std::suspend_always
	{
	public:

		std::shared_ptr<T> params;
		std::function<void(std::shared_ptr<T>_params)> runner;

		void configure(std::shared_ptr<T> _params, std::function<void(std::shared_ptr<T>_params)> _runner)
		{
			params = _params;
			runner = _runner;
		}

		struct promise_type
		{

			promise_type()
			{
				std::cout << "task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
				m_value = {};
			}

			task<T> get_return_object()
			{
				std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				task<T> my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(T value) 
			{
				std::cout << "task return_value:" << value << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_always initial_suspend() 
			{
				std::cout << "task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept 
			{
				std::cout << "task final_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {}

			T m_value;
		};

		std::coroutine_handle<promise_type> coroutine;

		void await_suspend(std::coroutine_handle<> _handle)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << this << ", task await_suspend:" << GetCurrentThreadId() << std::endl;
			task_job<T> *tj = new task_job<T>(coroutine, hevent, params, runner);
			global_job_queue->add_job(tj);
			std::cout << this << ", task await_suspend away:" << GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			::CloseHandle(hevent);
			std::cout << "task await_suspend finished:" << GetCurrentThreadId() << std::endl;
			_handle.resume();
		}

		T await_resume()
		{
			std::cout << "task await_resume:" << GetCurrentThreadId() << std::endl;
			T t = {};
			if (coroutine) {
				t = coroutine.promise().m_value;
			}
			return t;
		}

		operator T() {
			std::cout << this << ", task cast to T:" << GetCurrentThreadId() << std::endl;
			return coroutine.promise().m_value;
		}

		task(std::coroutine_handle<> _coroutine) : coroutine(_coroutine)
		{
			std::cout << this << ", task ctor coro:" << GetCurrentThreadId() << std::endl;
		}

		task()
		{
			std::cout << this << ", task ctor:" << GetCurrentThreadId() << std::endl;
		}

	};


	class ui_task : public std::suspend_always
	{
	public:

		json params;
		std::function<void(json)> runner;
		std::function<void(json)> on_gui;

		void configure(json _params, 
			std::function< void(json)> _runner,
			std::function< void(json)> _on_gui)
		{
			params = _params;
			runner = _runner;
			on_gui = _on_gui;
		}

		struct promise_type
		{

			promise_type()
			{
				std::cout << "task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
				m_value = {};
			}

			ui_task get_return_object()
			{
				std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				ui_task my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(json value)
			{
				std::cout << "task return_value:" << (std::string)value << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_always initial_suspend()
			{
				std::cout << "task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept
			{
				std::cout << "task final_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {}

			json m_value;
		};

		std::coroutine_handle<promise_type> coroutine;

		void await_suspend(std::coroutine_handle<> _handle)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << this << ", task await_suspend:" << GetCurrentThreadId() << std::endl;
			ui_task_job* tj = new ui_task_job(coroutine, hevent, params, runner, on_gui);
			global_job_queue->add_job(tj);
			std::cout << this << ", task await_suspend away:" << GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			::CloseHandle(hevent);
			std::cout << "task await_suspend finished:" << GetCurrentThreadId() << std::endl;
			_handle.resume();
		}

		json await_resume()
		{
			std::cout << "task ui_await_resume:" << GetCurrentThreadId() << std::endl;
			json t = {};
			if (coroutine) {
				t = coroutine.promise().m_value;
			}
			return t;
		}

		operator json() {
			std::cout << this << ", ui_task cast to json:" << GetCurrentThreadId() << std::endl;
			return coroutine.promise().m_value;
		}

		ui_task(std::coroutine_handle<promise_type> _coroutine) : coroutine(_coroutine)
		{
			std::cout << this << ", ui_task ctor coro:" << GetCurrentThreadId() << std::endl;
		}

		ui_task()
		{
			std::cout << this << ", ui_task ctor:" << GetCurrentThreadId() << std::endl;
		}

	};


	template <typename IOParams> struct async_io_task : public std::suspend_always
	{
	public:
		job_queue* queue;
		IOParams	params;
		std::function<bool(HANDLE hevent, IOParams* _src)> runner;

		void configure(job_queue* _queue, const IOParams& _params, std::function<bool(HANDLE hevent, IOParams* _src)> _runner)
		{
			queue = _queue;
			params = _params;
			runner = _runner;
		}

		struct promise_type
		{

			promise_type()
			{
				std::cout << "task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			async_io_task get_return_object()
			{
				std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				async_io_task my_task;
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

			void unhandled_exception() {}

			IOParams m_value;
		};

		std::coroutine_handle<promise_type> coroutine;

		void await_suspend(std::coroutine_handle<> handle)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << this << ", async_io_task await_suspend:" << GetCurrentThreadId() << std::endl;
			if (runner) {
				if (runner(hevent, &params)) {
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
			handle.resume();
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
			std::cout << this << ", async_io_task ctor coro:" << GetCurrentThreadId() << std::endl;
		}

		async_io_task()
		{
			std::cout << this << ", async_io_task ctor:" << GetCurrentThreadId() << std::endl;
		}

	};

	template <typename T> class sync : public std::suspend_never
	{
	public:

		struct promise_type
		{

			promise_type()
			{
				std::cout << "sync promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			sync get_return_object()
			{
				std::cout << "sync get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				sync my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(T value) {
				std::cout << "sync return_value:" << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_never initial_suspend() {
				std::cout << "sync initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_never final_suspend() noexcept {
				std::cout << "sync final_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {}

			T m_value;
		};

		std::coroutine_handle<promise_type> coroutine;

		bool await_ready()
		{
			return false;
		}

		void await_suspend(std::coroutine_handle<> _handle)
		{
			std::cout << this << ",sync  await_suspend:" << GetCurrentThreadId() << std::endl;
			_handle();
			std::cout << "sync await_suspend finished:" << GetCurrentThreadId() << std::endl;
		}

		T await_resume()
		{
			std::cout << "sync await_resume:" << GetCurrentThreadId() << std::endl;
			auto t = coroutine.promise().m_value;
			return t;
		}

		operator T() {
			std::cout << this << ", sync cast to T:" << GetCurrentThreadId() << std::endl;
			return coroutine.promise().m_value;
		}

		sync(std::coroutine_handle<> _coroutine) : coroutine(_coroutine)
		{
			std::cout << this << ", sync  ctor coro:" << GetCurrentThreadId() << std::endl;
		}

		sync()
		{
			std::cout << this << ", sync  ctor:" << GetCurrentThreadId() << std::endl;
		}

	};

	class background_job : public job
	{
	public:
		json data;
		std::function<void(json)> run;
		std::function<void(json)> on_gui;

		background_job() : job()
		{
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			std::cout << "job start:" << GetCurrentThreadId() << std::endl;

			ui_task_result* uir = new ui_task_result(data, on_gui);

			try
			{
				if (run)
				{
					run(data);
					::PostMessage(NULL, WM_CORONA_JOB_COMPLETE, TRUE, (LPARAM)uir);
				}
				else {
					::PostMessage(NULL, WM_CORONA_JOB_COMPLETE, FALSE, (LPARAM)uir);
				}
			}
			catch (...)
			{
				::PostMessage(NULL, WM_CORONA_JOB_COMPLETE, FALSE, (LPARAM)uir);
			}

			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class background
	{

	public:

		void run(json _data, std::function<void(json)> _task, std::function<void(json)> _on_ui)
		{
			background_job *bj = new background_job();
			bj->data = _data;
			bj->run = _task;
			bj->on_gui = _on_ui;
			global_job_queue->add_job(bj);
		}
	};

}

#endif
