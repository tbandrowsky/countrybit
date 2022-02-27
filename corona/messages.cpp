#pragma once

#include <string>
#include <cstdint>
#include <coroutine>
#include "messages.h"
#include "windows.h"

namespace countrybit
{
	namespace system
	{
		os_result::os_result()
		{
			error_code = ::GetLastError();
			if (error_code) {
				success = false;
				::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, message.c_str_w(), message.size() - 1, nullptr);
			}
			else {
				success = true;
			}
		}
	}
}