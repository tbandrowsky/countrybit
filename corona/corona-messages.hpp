/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
little objects that deal with GetLastError.

Notes

For Future Consideration
*/


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
		os_result(uint64_t _error_code);
	};

	std::ostream& operator <<(std::ostream& output, const os_result& src);


	os_result::os_result(uint64_t last_error)
	{
		error_code = last_error;
		if (error_code) {
			success = false;
			char buffer[2048] = {};
			int buffer_size = sizeof(buffer);
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, buffer, buffer_size, nullptr);
			buffer[2000] = 0;
			message = buffer;
		}
		else {
			success = true;
		}
	}
	os_result::os_result()
	{
		error_code = ::GetLastError();
		if (error_code) {
			success = false;
			char buffer[2048] = {};
			int buffer_size = sizeof(buffer);
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, buffer, buffer_size, nullptr);
			buffer[2000] = 0;
			message = buffer;
		}
		else {
			message = "Ok";
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
