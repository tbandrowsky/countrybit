#pragma once

#include "pch.h"

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
	}
}