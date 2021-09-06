#pragma once

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>

namespace countrybit
{
	namespace database
	{

		template <uint16_t length>
		requires(length > 0)
			class jstring
		{
		public:

			char data[length];

			static const int max_length = length;
			static const int last_char = length - 1;

			void copy(const char* s)
			{
				char* d = &data[0];
				int l = 0;

				while (l <= last_char && *s)
				{
					*d = *s;
					l++;
					s++;
					d++;
				}

				if (l <= last_char) 
				{
					d[l] = 0;
				}
				else 
				{
					d[last_char] = 0;
				}
			}

			jstring()
			{
				data[0] = 0;
			}

			jstring(const std::string& src)
			{
				char* s = src.c_str();
				copy(s);
			}

			jstring(const char *src)
			{
				copy(src);
			}

			jstring& operator = (const std::string& src)
			{
				char* s = src.c_str();
				copy(s);
				return *this;
			}

			template <int Y> jstring& operator = (const jstring<Y>& _src)
			{
				char* s = _src.c_str();
				copy(s);
				return *this;
			}

			jstring& operator = (const char* s)
			{
				copy(s);
				return *this;
			}

			const char* c_str() const
			{
				return &data[0];
			}

		};

		template<int l1, int l2> int compare(const jstring<l1>& a, const jstring<l2>& b)
		{
			return strcmp(a.c_str(), b.c_str());
		}

		template<int l1, int l2> int operator<(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> int operator>(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> int operator>=(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> int operator<=(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> int operator==(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> int operator!=(const jstring<l1>& a, const jstring<l2>& b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> int compare(const jstring<l1>& a, const char *b)
		{
			return strcmp(a.c_str(), b);
		}

		template<int l1> int operator<(const jstring<l1>& a, const char *b)
		{
			return compare(a, b) < 0;
		}

		template<int l1> int operator>(const jstring<l1>& a, const char* b)
		{
			return compare(a, b) > 0;
		}

		template<int l1> int operator>=(const jstring<l1>& a, const char* b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1> int operator<=(const jstring<l1>& a, const char* b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1> int operator==(const jstring<l1>& a, const char* b)
		{
			return compare(a, b) == 0;
		}

		template<int l1> int operator!=(const jstring<l1>& a, const char* b)
		{
			return compare(a, b) != 0;
		}

		template <int l1> std::ostream& operator <<(std::ostream& output, jstring<l1>& src)
		{
			output << src.c_str();
			return output;
		}

	}
}