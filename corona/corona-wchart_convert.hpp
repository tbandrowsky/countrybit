/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT LICENSE

About this File
Wide character conversions.  
Useful for, when Windows needs 16 bit unicode characters.
Corona is meant to be compiled in 8 bit characters 
with UTF-8 character set.

Notes

For Future Consideration
*/


#ifndef CORONA_WCHAR_CONVERT_H
#define CORONA_WCHAR_CONVERT_H

namespace corona 
{

	class wchar_converter 
	{
		std::vector<std::unique_ptr<wchar_t[]>> converted_to_wchars;
		std::vector<std::unique_ptr<char[]>> converted_to_chars;

	public:
		wchar_converter(const wchar_converter& _src) = delete;
		wchar_converter(wchar_converter&& _src) = delete;
		wchar_converter operator = (const wchar_converter& _src) = delete;
		wchar_converter& operator = (const wchar_converter&& _src) = delete;

		wchar_converter()
		{

		}

		~wchar_converter()
		{
		}

		wchar_t* to_wchar_t( const char *_src )
		{
			if (not _src)
				return nullptr;

			size_t max_len = strlen(_src);
			size_t max_len_bytes = max_len * sizeof(wchar_t);
			std::unique_ptr<wchar_t[]> temp;

			int result = MultiByteToWideChar(CP_UTF8, 0, _src, max_len_bytes, NULL, 0);
			if (result > 0)
			{
				int result_bytes = result * sizeof(wchar_t);
				temp = std::make_unique< wchar_t[] >(result + 1);
				if (temp) {
					MultiByteToWideChar(CP_UTF8, 0, _src, result_bytes, temp.get(), result);
					temp[result] = 0;
				}
			}
			else
			{
				temp = std::make_unique< wchar_t[] >(result + 1);
				if (temp) {
					temp[0] = 0;
				}
			}
			wchar_t* ret_value = temp.get();
			converted_to_wchars.push_back(std::move(temp));
			return ret_value;
		}

		char* to_char(const wchar_t* _src)
		{
			if (not _src)
				return nullptr;

			size_t max_len = wcslen(_src);
			std::unique_ptr<char[]> temp;

			int result = WideCharToMultiByte(CP_UTF8, 0, _src, max_len, NULL, 0, NULL, NULL);
			if (result > 0)
			{
				temp = std::make_unique< char[] >(result + 1);
				if (temp) {
					WideCharToMultiByte(CP_UTF8, 0, _src, result, temp.get(), result, NULL, NULL);
					temp[result] = 0;
				}
			}
			else
			{
				temp = std::make_unique< char[] >(result + 1);
				if (temp) {
					temp[0] = 0;
				}
			}
			char* ret_value = temp.get();
			converted_to_chars.push_back(std::move(temp));
			return ret_value;
		}
	};
}


#endif
