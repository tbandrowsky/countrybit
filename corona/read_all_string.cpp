
#include "corona.h"

namespace corona
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
				FILE* fp = nullptr;
				int err_no = fopen_s(&fp, _filename.c_str(), "r");
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
