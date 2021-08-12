#pragma once

#include <vector>
#include "queue.h"
#include <coroutine>
#include "messages.h"
#include <functional>

namespace countrybit
{
	namespace system 
	{

		template <class fn_type> class background_job : public job
		{
		public:
			std::coroutine_handle<> handle;
			std::function<fn_type> fn;

			background_job(std::function<fn_type> _fn, std::coroutine_handle<> _handle) : handle(_handle), fn(_fn)
			{
				;
			}

			virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
			{
				job_notify jn;
				jn.setCoroutine(handle);
				jn.shouldDelete = false;
				fn();
				return jn;
			}
		};

		class background_function
		{
			job_queue* queue;

		protected:

			background_function(job_queue* _queue) : queue(_queue)
			{
			}

			friend class application;

		public:

			background_function() :
				queue(nullptr)
			{
				;
			}

			background_function(background_function&& _srcFn) noexcept
			{
				queue = _srcFn.queue;
			}

			background_function& operator = (background_function&& _srcFn) noexcept
			{
				queue = _srcFn.queue;
				return *this;
			}

			background_function(const background_function& src) = delete;
			background_function& operator = (const background_function& _srcFile) = delete;

			~background_function()
			{
			}

			template <typename f_type> struct function_awaiter : std::suspend_always
			{
				std::function<f_type> fn;
				job_queue* queue;

				void await_suspend(std::coroutine_handle<> handle)
				{
					background_job<f_type> bj(queue, handle);
					queue->postJobMessage(&bj);
				}

				void configure(job_queue *_queue, std::function<f_type>& _fn)
				{
					queue = _queue;
					fn = _fn;
				}
			};

			template <typename f_type> auto run(std::function<f_type> fn) // adds size_bytes to file and returns the position of the start
			{
				function_awaiter<f_type> aw;
				aw.configure(queue, fn);
				return aw;
			}
		};
	}
}

