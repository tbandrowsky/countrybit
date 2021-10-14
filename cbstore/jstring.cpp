
#include "jstring.h"

namespace countrybit
{
	namespace database
	{


		int compare(const jstring& a, const jstring& b)
		{
			return strcmp(a.c_str(), b.c_str());
		}

		int operator<(const jstring& a, const jstring& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const jstring& a, const jstring& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const jstring& a, const jstring& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const jstring& a, const jstring& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const jstring& a, const jstring& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const jstring& a, const jstring& b)
		{
			return compare(a, b) != 0;
		}

		int compare(const jstring& a, const char* b)
		{
			return strcmp(a.c_str(), b);
		}

		int operator<(const jstring& a, const char* b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const jstring& a, const char* b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const jstring& a, const char* b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const jstring& a, const char* b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const jstring& a, const char* b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const jstring& a, const char* b)
		{
			return compare(a, b) != 0;
		}

		std::string operator+(const jstring& a, const char* b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		std::string operator+(const char* b, const jstring& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

		std::string operator+(const jstring& a, const std::string& b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		std::string operator+(const std::string& b, const jstring& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

		std::ostream& operator <<(std::ostream& output, jstring& src)
		{
			output << src.c_str();
			return output;
		}

	}

}