
#include "corona.h"

namespace corona
{
	namespace database
	{
		bool str_has_any(const char* _src, const char* _charlist)
		{
			for (const char* s = _src; *s; s++)
			{
				for (const char* c = _charlist; *c; c++)
				{
					if (*c == *s) return true;
				}
			}
			return false;
		}

		bool str_has_any(const wchar_t* _src, const wchar_t* _charlist)
		{
			for (const wchar_t* s = _src; *s; s++)
			{
				for (const wchar_t* c = _charlist; *c; c++)
				{
					if (*c == *s) return true;
				}
			}
			return false;
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
			return type_code;
		}

		bool string_tests()
		{
			int r = true;
			corona::database::istring<5> test1 = "1234567";

			r = r && assert_if([test1]() { return test1.size() == 4; }, "Size incorrect.");
			r = r && assert_if([test1]() { return test1 == "1234"; }, "truncation incorrect.");

			corona::database::iwstring<5> test2 = L"1234567";

			r = r && assert_if([test2]() { return test2.size() == 4; }, L"Size incorrect.");
			r = r && assert_if([test2]() { return test2 == L"1234"; }, L"truncation incorrect.");

			r = r && assert_if([test1, test2]() { return test1 == test2; }, L"Equality incorrect.");
			r = r && assert_if([test1, test2]() { return test2 == test1; }, L"Equality incorrect.");

			test1 = "ABC";
			test2 = "ABCD";

			r = r && assert_if([test1, test2]() { return test2 > test1; }, L"> incorrect.");
			r = r && assert_if([test1, test2]() { return test1 < test2; }, L"< incorrect.");

			return r;
		}
	}

}