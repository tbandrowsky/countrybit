
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
			std::cout << "ui job end:" << GetCurrentThreadId() << std::endl;
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

			std::cout << "ui task start:" << GetCurrentThreadId() << std::endl;

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
