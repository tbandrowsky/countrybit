#ifndef CORONA_MESSAGE_H
#define CORONA_MESSAGE_H

#include "corona-windows-lite.h"
#include "corona-constants.hpp"

#include <iostream>
#include <string>
#include <compare>

namespace corona 
{

	class base_result
	{
	public:
		bool success;
		std::string message;

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
		block_id block;

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

	std::ostream& operator <<(std::ostream& output, const os_result& src);


	os_result::os_result()
	{
		error_code = ::GetLastError();
		if (error_code) {
			success = false;
			char buffer[2048];
			int buffer_size = sizeof(buffer);
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, buffer, buffer_size, nullptr);
			message = buffer;
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

}

#endif
