
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

	class task_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;

		task_job() : job()
		{
		}

		task_job(std::coroutine_handle<> _handle, HANDLE _event) : handle(_handle), event(_event)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			jn.setSignal(event);
			std::cout << "job start:" << GetCurrentThreadId() << std::endl;
			if (handle) handle();
			std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = false;
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

		struct promise_type
		{

			promise_type()
			{
				std::cout << "task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			task get_return_object()
			{
				std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				task my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(T value) {
				std::cout << "task return_value:" << value << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_always initial_suspend() {
				std::cout << "task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept {
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
			task_job tj(coroutine, hevent);
			application::get_application()->add_job(&tj);
			std::cout << this << ", task await_suspend away:" << GetCurrentThreadId() << std::endl;
			::WaitForSingleObject(hevent, INFINITE);
			::CloseHandle(hevent);
			std::cout << "task await_suspend finished:" << GetCurrentThreadId() << std::endl;
			_handle.resume();
		}

		T await_resume()
		{
			std::cout << "task await_resume:" << GetCurrentThreadId() << std::endl;
			auto t = coroutine.promise().m_value;
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

	template <typename JobType, typename IOParams> struct async_io_task : public std::suspend_always
	{
	public:
		job_queue* queue;
		IOParams	params;

		void configure(job_queue* _queue, const IOParams& _params)
		{
			queue = _queue;
			params = _params;
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
			JobType my_job;
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			std::cout << this << ", async_io_task await_suspend:" << GetCurrentThreadId() << std::endl;
			my_job.params = &params;
			my_job.handle = coroutine;
			my_job.event = hevent;
			if (my_job.run()) {
				std::cout << this << ", async_io_task await_suspend away:" << GetCurrentThreadId() << std::endl;
				::WaitForSingleObject(hevent, INFINITE);
				::CloseHandle(hevent);
				std::cout << "async_io_task await_suspend finished:" << GetCurrentThreadId() << std::endl;
			}
			else {
				std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
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
