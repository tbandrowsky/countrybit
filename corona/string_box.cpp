
#include "string_box.h"
#include "assert_if.h"

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

		int get_hash_code( const char *_src )
		{
			int type_code = 17;
			for (const char* c = _src; *c; c++)
			{
				type_code = type_code * 23 + *c;
			}
		}

		bool string_tests()
		{
			int r = true;
			countrybit::database::istring<5> test1 = "1234567";

			r = r && assert_if([test1]() { return test1.size() == 4; }, "Size incorrect.");
			r = r && assert_if([test1]() { return test1 == "1234"; }, "truncation incorrect.");
			return r;
		}


	}

}