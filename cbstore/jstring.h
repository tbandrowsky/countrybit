#pragma once

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>

#include "store_box.h"
#include "constants.h"

namespace countrybit
{
	namespace database
	{

		class jstring
		{

			struct jstring_data {
			public:
				uint32_t length;
				int last_char;
				char data[];
			};

			jstring_data* hdr;

			void copy(const char* s)
			{
				char* d = &hdr->data[0];
				int l = 0;

				while (l <= hdr->last_char && *s)
				{
					*d = *s;
					l++;
					s++;
					d++;
				}

				if (l <= hdr->last_char)
				{
					hdr->length = l;
					hdr->data[l] = 0;
				}
				else
				{
					hdr->length = hdr->last_char;
					hdr->data[hdr->last_char] = 0;
				}
			}

		public:

			jstring() : hdr(nullptr)
			{
			}

			jstring( char *c ) : hdr((jstring_data*)c)
			{
			}

			template <typename BOX>
			requires(box<BOX, char>)
			static row_id_type create(BOX* b, int chars_length)
			{
				jstring temp;
				auto location = b->pack<char>(size(jstring_header)+chars_length);
				temp.hdr = b->unpack<jstring_header>(temp.location);
				temp.hdr->last_char = chars_length - 1;
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return location;
			}

			template <typename BOX>
			requires(box<BOX, char>)
			static jstring get(BOX* b, int location)
			{
				jstring temp;
				temp.hdr = b->unpack<jstring_header>(temp.location);
				return temp;
			}

			static jstring create(char* b, int chars_length)
			{
				jstring temp;
				temp.hdr = (jstring_data *)(b);
				temp.hdr->last_char = chars_length - 1;
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return temp;
			}

			static jstring get(char* b)
			{
				jstring temp;
				temp.hdr = (jstring_data*)(b);
				return temp;
			}

			static int get_box_size(int length)
			{
				return sizeof(jstring_data) + length;
			}

			jstring& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
				return *this;
			}

			jstring& operator = (const jstring& _src)
			{
				if (_src.hdr != hdr) {
					const char* s = _src.c_str();
					copy(s);
				}
				return *this;
			}

			jstring& operator = (const char* s)
			{
				copy(s);
				return *this;
			}

			const char* c_str() const
			{
				return &hdr->data[0];
			}

			uint16_t size() const
			{
				return hdr->length;
			}		
		};

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

		int compare(const jstring& a, const char *b)
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

		template <int length> class istring 
		{
			char data[length];
			uint32_t length;
			static int last_char = length - 1;

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
					length = l;
					data[l] = 0;
				}
				else
				{
					length = last_char;
					data[last_char] = 0;
				}
			}

		public:

			istring() 
			{
				data[0] = 0;
				length = 0;
			}

			istring& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
				return *this;
			}

			istring operator = (const istring& _src)
			{
				if (this != &_src) {
					const char* s = _src.c_str();
					copy(s);
				}
				return *this;
			}

			istring& operator = (const char* s)
			{
				copy(s);
				return *this;
			}

			const char* c_str() const
			{
				return &data[0];
			}

			uint16_t size() const
			{
				return length;
			}
		};

		template<int l1, int l2> int compare(const istring<l1>& a, const istring<l2>& b)
		{
			return strcmp(a.c_str(), b.c_str());
		}

		template<int l1, int l2> int operator<(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> int operator>(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> int operator>=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> int operator<=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> int operator==(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> int operator!=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> int compare(const istring<l1>& a, const char* b)
		{
			return strcmp(a.c_str(), b);
		}

		template<int l1> int operator<(const istring<l1>& a, const char* b)
		{
			return compare(a, b) < 0;
		}

		template<int l1> int operator>(const istring<l1>& a, const char* b)
		{
			return compare(a, b) > 0;
		}

		template<int l1> int operator>=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1> int operator<=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1> int operator==(const istring<l1>& a, const char* b)
		{
			return compare(a, b) == 0;
		}

		template<int l1> int operator!=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> std::string operator+(const istring<l1>& a, const char* b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		template<int l1> std::string operator+(const char* b, const istring<l1>& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

		template<int l1> std::string operator+(const istring<l1>& a, const std::string& b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
		}

		template<int l1> std::string operator+(const std::string& b, const istring<l1>& a)
		{
			std::string temp = b;
			temp += a.c_str();
			return temp;
		}

	}
}
