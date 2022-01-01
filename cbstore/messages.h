#pragma once

#include <string>
#include <cstdint>
#include <coroutine>

namespace countrybit
{
	namespace system
	{
		class base_result
		{
		public:
			bool success;
			std::string message;

			base_result() : success(true)
			{
				;
			}
		};

		class base_parse_result : public base_result
		{
		public:
			int char_offset;
			int line_number;

			base_parse_result() : char_offset(0), line_number(0)
			{
				;
			}
		};

		class os_result : public base_result
		{
		public:

			uint64_t error_code;

			os_result();
		};

	}

}
