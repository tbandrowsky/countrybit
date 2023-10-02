module;

#include "windows.h"
#include <iostream>
#include <string>
#include <compare>

export module corona.database:messages;

import :store_box;
import :string_box;
import :constants;

using error_string = istring<256>;

export class base_result
		{
		public:
			bool success;
			error_string message;

			base_result() : success(true), message("")
			{
				;
			}
		};

export class base_parse_result : public base_result
		{
		public:
			int char_offset;
			int line_number;
			block_id block;

			base_parse_result() : char_offset(0), line_number(0)
			{
				;
			}
		};

export class os_result : public base_result
		{
		public:
			uint64_t error_code;
			os_result();
		};

export std::ostream& operator <<(std::ostream& output, const os_result& src);


		os_result::os_result()
		{
			error_code = ::GetLastError();
			if (error_code) {
				success = false;
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, message.c_str_w(), message.capacity(), nullptr);
			}
			else {
				success = true;
			}
		}

		std::ostream& operator <<(std::ostream& output, const os_result& src)
		{
			output << src.error_code << " " << src.message << " " << src.success;
			return output;
		}

