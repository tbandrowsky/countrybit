
#ifndef CORONA_READ_ALL_STRING_H
#define CORONA_READ_ALL_STRING_H

namespace corona {

	std::string read_all_string(std::string& _filename)
	{
		std::string results;

		std::filesystem::path p = _filename;

		int64_t length = std::filesystem::file_size(p);
		if (length > 0)
		{
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

#endif
