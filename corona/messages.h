#pragma once

#include <string>
#include <cstdint>
#include <coroutine>

#include "string_box.h"

namespace corona
{
	namespace database
	{
		using error_string = istring<256>;

		class base_result
		{
		public:
			bool success;
			error_string message;

			base_result() : success(true), message("")
			{
				;
			}
		};

		class base_parse_result : public base_result
		{
		public:
			int char_offset;
			int line_number;
			database::block_id block;

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

		std::ostream& operator <<(std::ostream& output, os_result& src);

	}

}
