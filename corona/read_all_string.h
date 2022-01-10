/*********************************************************************
  Countrybit SAT
  (c) 2021 TJ Bandrowsky
  MIT License
**********************************************************************/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <clocale>
#include <locale>
#include "sys/stat.h"

namespace countrybit
{
	namespace util
	{
		std::string read_all_string(std::string& _filename)
		{
			std::string results;
			int length;
			struct stat stat_buf;
			int rc = stat(_filename.c_str(), &stat_buf);
			if (rc == 0)
			{
				length = stat_buf.st_size;
				FILE* fp = fopen(_filename.c_str(), "r");
				if (fp != nullptr) {
					char* buffer = new char[length + 1];
					int l = fread(buffer, 1, length, fp);
					fclose(fp);
					buffer[l] = 0;
					results = buffer;
					delete[] buffer;
				}
			}
			return results;
		}
	}
}
