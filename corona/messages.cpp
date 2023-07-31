#pragma once

#include "corona.h"

namespace corona
{
	namespace database
	{
		os_result::os_result()
		{
			error_code = ::GetLastError();
			if (error_code) {
				success = false;
				::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, message.c_str_w(), message.capacity(), nullptr);
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
}