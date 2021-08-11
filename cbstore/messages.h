#pragma once

#include <string>
#include <cstdint>
#include <coroutine>

namespace countrybit
{
	namespace system
	{
		class result_base
		{
		public:
			bool success;
			std::string message;

			result_base() : success(true)
			{
				;
			}
		};

		class os_result : public result_base
		{
		public:

			uint64_t error_code;

			os_result();
		};

		template <typename resultType> class task
		{
		public:
			struct promise_type;

			task(task const&) = delete;
			task(task&&) = delete;

			task& operator=(task&&) = delete;
			task& operator=(task const&) = delete;

			//std::coroutine_handle<promise_type> in the future
			using handle = std::coroutine_handle<promise_type>;

			struct promise_type {

				auto get_return_object() {
					return task{ handle::from_promise(*this) };
				}

				void return_value(resultType value) {
					m_value = value;
				}

				auto initial_suspend() {
					return std::suspend_never{};
				}

				auto final_suspend() {
					return std::suspend_always{};
				}

				void unhandled_exception() {
					;
				}

				resultType m_value;
			};

			task(handle h) : hcoroutine(h) {}

			resultType get()
			{
				return hcoroutine.promise().m_value;
			}

			~task() {
				if (hcoroutine)
					hcoroutine.destroy();
			}

		private:
			handle hcoroutine;
		};
	}

}
