
#include "string_box.h"

namespace countrybit
{
	namespace database
	{


		int compare(const string_box& a, const string_box& b)
		{
			return strcmp(a.c_str(), b.c_str());
		}

		int operator<(const string_box& a, const string_box& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const string_box& a, const string_box& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const string_box& a, const string_box& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const string_box& a, const string_box& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const string_box& a, const string_box& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const string_box& a, const string_box& b)
		{
			return compare(a, b) != 0;
		}

		int compare(const string_box& a, const char* b)
		{
			return strcmp(a.c_str(), b);
		}

		int operator<(const string_box& a, const char* b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const string_box& a, const char* b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const string_box& a, const char* b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const string_box& a, const char* b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const string_box& a, const char* b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const string_box& a, const char* b)
		{
			return compare(a, b) != 0;
		}

		std::string operator+(const string_box& a, const char* b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		std::string operator+(const char* b, const string_box& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

		std::string operator+(const string_box& a, const std::string& b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		std::string operator+(const std::string& b, const string_box& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

		std::ostream& operator <<(std::ostream& output, string_box& src)
		{
			output << src.c_str();
			return output;
		}

	}

}