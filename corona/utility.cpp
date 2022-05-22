
#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{
		std::vector<std::string> split(const std::string& s, char delim)
		{
			std::vector<std::string> elems;
			split(s, delim, std::back_inserter(elems));
			return elems;
		}
	}
}

#endif
