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

		class string_box
		{

			struct string_box_data {
			public:
				uint32_t length;
				int last_char;
				char data[1];
			};

			string_box_data* hdr;

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

			string_box() : hdr(nullptr)
			{
			}

			string_box( char *c ) : hdr((string_box_data*)c)
			{
			}

			template <typename BOX>
			requires(box<BOX, char>)
			static row_id_type create(BOX* b, int chars_length)
			{
				string_box temp;
				auto location = b->pack<char>(sizeof(string_box_data)+chars_length);
				temp.hdr = b->unpack<string_box_data>(location);
				temp.hdr->last_char = chars_length - 1;
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return location;
			}

			template <typename BOX>
			requires(box<BOX, char>)
			static string_box get(BOX* b, int location)
			{
				string_box temp;
				temp.hdr = b->unpack<string_box_data>(location);
				return temp;
			}

			static string_box create(char* b, int chars_length)
			{
				string_box temp;
				temp.hdr = (string_box_data *)(b);
				temp.hdr->last_char = chars_length - (sizeof(string_box_data)-1);
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return temp;
			}

			static string_box get(char* b)
			{
				string_box temp;
				temp.hdr = (string_box_data*)(b);
				return temp;
			}

			static int get_box_size(int length)
			{
				return sizeof(string_box_data) + length;
			}

			string_box& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
				return *this;
			}

			string_box& operator = (const string_box& _src)
			{
				hdr = _src.hdr;
				return *this;
			}

			string_box& operator = (const char* s)
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

		int compare(const string_box& a, const string_box& b);
		int operator<(const string_box& a, const string_box& b);
		int operator>(const string_box& a, const string_box& b);
		int operator>=(const string_box& a, const string_box& b);
		int operator<=(const string_box& a, const string_box& b);
		int operator==(const string_box& a, const string_box& b);
		int operator!=(const string_box& a, const string_box& b);
		int compare(const string_box& a, const char* b);
		int operator<(const string_box& a, const char* b);
		int operator>(const string_box& a, const char* b);
		int operator>=(const string_box& a, const char* b);
		int operator<=(const string_box& a, const char* b);
		int operator==(const string_box& a, const char* b);
		int operator!=(const string_box& a, const char* b);

		std::string operator+(const string_box& a, const char* b);
		std::string operator+(const char* b, const string_box& a);
		std::string operator+(const string_box& a, const std::string& b);
		std::string operator+(const std::string& b, const string_box& a);
		std::ostream& operator <<(std::ostream& output, string_box& src);
		
		template <int length_bytes> struct istring 
		{
			char data[length_bytes];
			uint32_t length;

			istring() = default;

			istring(const char *_src)
			{
				copy(_src);
			}

			istring(const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
			}

			istring& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
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

			void copy(const char* s)
			{
				int last_char = length_bytes - 1;

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

		template<int l1> std::ostream& operator <<(std::ostream& output, istring<l1>& src)
		{
			output << src.c_str();
			return output;
		}
	}
}
