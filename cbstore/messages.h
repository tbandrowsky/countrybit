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

	}

}
