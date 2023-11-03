
#ifndef CORONA_FUNCTION_H
#define CORONA_FUNCTION_H

#include "corona-windows-all.h"
#include "corona-queue.hpp"

#include <functional>
#include <coroutine>
#include <iostream>
#include <memory>
#include <compare>

namespace corona
{

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
			global_job_queue->postJobMessage(tj);
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
					::CloseHandle(hevent);
					std::cout << "async_io_task await_suspend finished:" << GetCurrentThreadId() << std::endl;
				}
				else 
				{
					std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
				}
			}
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
				std::cout << "sync return_value:" << value << " " << this << GetCurrentThreadId() << std::endl;
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
}

#endif
