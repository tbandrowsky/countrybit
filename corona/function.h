#pragma once

#include <iostream>
#include <vector>
#include "queue.h"
#include <coroutine>
#include "messages.h"
#include <functional>
#include "application.h"

namespace countrybit
{
	namespace system 
	{

		template <typename return_value> class task_job : public job
		{
		public:
			std::coroutine_handle<> handle;
			HANDLE event;

			task_job() : job()
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
				task_job<int> my_job;
				HANDLE hevent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				std::cout << this << ", task await_suspend:" << GetCurrentThreadId() << std::endl;
				my_job.handle = coroutine;
				my_job.event = hevent;
				application::get_application().add_job(&my_job);
				std::cout << this << ", task await_suspend away:" << GetCurrentThreadId() << std::endl;
				::WaitForSingleObject(hevent, INFINITE);
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
}

