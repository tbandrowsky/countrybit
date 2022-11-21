#pragma once

#define _CRT_SECURE_NO_WARNINGS

namespace corona
{
	namespace database
	{

		struct string_box_data {
		public:
			uint32_t length;
			int last_char;
			char data[1];
		};

		bool str_has_any(const char* _src, const char* _charlist);
		bool str_has_any(const wchar_t* _src, const wchar_t* _charlist);

		template <int len> class istring;
		template <int len> class iwstring;

		class string_box
		{

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

			string_box(const string_box& _src)
			{
				hdr = _src.hdr;
			}

			template <typename BOX>
			requires(box<BOX, char>)
			static relative_ptr_type create(BOX* b, int chars_length)
			{
				string_box temp;
				auto location = b->put_object<char>(sizeof(string_box_data)+chars_length);
				temp.hdr = b->get_object<string_box_data>(location);
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
				temp.hdr = b->get_object<string_box_data>(location);
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

			void copy(const wchar_t* _src, corona_size_t max_len = -1)
			{
				if (max_len < 0)
					max_len = wcslen(_src);

				if (max_len >= hdr->last_char)
					max_len = hdr->last_char;

				int result = WideCharToMultiByte(CP_UTF8, 0, _src, max_len, NULL, 0, NULL, NULL);
				if (result > 0)
				{
					while (result > hdr->last_char) 
					{
						max_len--;
						result = WideCharToMultiByte(CP_UTF8, 0, _src, max_len, NULL, 0, NULL, NULL);
					}

					if (result > 0)
					{
						WideCharToMultiByte(CP_UTF8, 0, _src, max_len, hdr->data, result, NULL, NULL);
						hdr->data[result] = 0;
						hdr->length = result;
					}
					else 
					{
						hdr->data[0] = 0;
						hdr->length = 0;
					}
				}
				else 
				{
					hdr->data[0] = 0;
					hdr->length = 0;
				}
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

			string_box& operator = (const std::wstring& src)
			{
				const wchar_t* s = src.c_str();
				copy(s);
				return *this;
			}

			template <int len> istring<len> to_istring()
			{
				istring<len> temp;
				temp = c_str();
				return temp;
			}

			template <int len> iwstring<len> to_iwstring()
			{
				iwstring<len> temp;
				temp = c_str();
				return temp;
			}

			string_box& operator = (const wchar_t* s)
			{
				copy(s);
				return *this;
			}

			string_box& operator = (double d)
			{
				char buff[256];
				sprintf_s(buff, "%f", d);
				copy(buff);
				return *this;
			}

			const char* c_str() const
			{
				return &hdr->data[0];
			}

			std::string value() const
			{
				return std::string(c_str());
			}

			uint16_t size() const
			{
				return hdr->length;
			}		

			bool starts_with(const char* _data)
			{
				int i = 0;
				while (*_data) {
					if (i >= size() || *_data != hdr->data[i])
						return false;
					_data++;
					i++;
				}
				return true;
			}

			bool has_any(const char* _pattern)
			{
				return str_has_any(hdr->data, _pattern);
			}

			double to_double()
			{
				return std::strtod(c_str(), nullptr);
			}

			long to_long()
			{
				long l = std::strtol(c_str(), nullptr, 10);
				return l;
			}

			operator const char* () const
			{
				return c_str();
			}

			std::strong_ordering operator<=>(const string_box& _src) const
			{
				int x = std::strcmp(hdr->data, _src.hdr->data);
				if (x < 0) 
				{
					return std::strong_ordering::less;
				}
				else if (x == 0)
				{
					return std::strong_ordering::equal;
				}
				else if (x > 0)
				{
					return std::strong_ordering::greater;
				}
			}

			std::strong_ordering operator<=>(const char *_src) const
			{
				int x = std::strcmp(hdr->data, _src);
				if (x < 0)
				{
					return std::strong_ordering::less;
				}
				else if (x == 0)
				{
					return std::strong_ordering::equal;
				}
				else if (x > 0)
				{
					return std::strong_ordering::greater;
				}
			}

		};

		int get_hash_code(const char* _src);

		std::string operator+(const string_box& a, const char* b);
		std::string operator+(const char* b, const string_box& a);
		std::string operator+(const string_box& a, const std::string& b);
		std::string operator+(const std::string& b, const string_box& a);
		std::ostream& operator <<(std::ostream& output, string_box& src);
		
		template <int32_t length_bytes> class istring 
		{
			int32_t length;
			int32_t last_char;
			char data[length_bytes];
		public:

			istring() : last_char( length_bytes - 1 ), length(0)
			{
				copy("");
			}

			istring(const char *_src) : last_char(length_bytes - 1), length(0)
			{
				copy(_src);
			}

			istring(const std::string& src) : last_char(length_bytes - 1), length(0)
			{
				const char* s = src.c_str();
				copy(s);
			}

			template <int length> istring(const istring<length>& src) : last_char(length_bytes - 1), length(0)
			{
				const char* s = src.c_str();
				copy(s);
			}

			istring& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				length = 0;
				copy(s);
				return *this;
			}

			istring& operator = (const char* s)
			{
				length = 0;
				copy(s);
				return *this;
			}

			istring(const wchar_t* _src) 
			{
				length = 0;
				copy(_src);
			}

			istring(const std::wstring& src)
			{
				length = 0;
				const wchar_t* s = src.c_str();
				copy(s);
			}

			istring& operator = (const std::wstring& src)
			{
				length = 0;
				const wchar_t* s = src.c_str();
				copy(s);
				return *this;
			}

			template <int length> istring& operator = (const istring<length>& src)
			{
				length = 0;
				const char* s = src.c_str();
				copy(s);
				return *this;
			}

			istring& operator = (const wchar_t* s)
			{
				length = 0;
				copy(s);
				return *this;
			}

			const char* c_str() const
			{
				return &data[0];
			}

			char* c_str_w()
			{
				return &data[0];
			}

			operator const char* () const
			{
				return c_str();
			}

			double to_double() const
			{
				double d = std::strtod(value(), nullptr);
				return d;
			}

			long to_long() const
			{
				long l = std::strtol(value(), nullptr, 10);
				return l;
			}

			const char* value() const
			{
				return &data[0];
			}

			uint32_t size() const
			{
				return length;
			}

			uint32_t capacity() const
			{
				return length_bytes - 1;
			}

			void copy(const wchar_t* _src, int max_len = -1)
			{
				if (max_len < 0)
					max_len = wcslen(_src);

				int total_len = max_len + length;

				if (total_len >= last_char) {
					max_len = total_len - last_char;
					total_len = last_char;
				}

				int result = WideCharToMultiByte(CP_UTF8, 0, _src, max_len, NULL, 0, NULL, NULL);
				if (result > 0)
				{
					while (result > last_char) {
						max_len--;
						result = WideCharToMultiByte(CP_UTF8, 0, _src, max_len, NULL, 0, NULL, NULL);
					}
					if (result > 0)
					{
						WideCharToMultiByte(CP_UTF8, 0, _src, max_len, data + length, result, NULL, NULL);
						data[result + length] = 0;
						length = result + length;
					}
					else {
						data[length] = 0;
					}
				}
				else {
					data[length] = 0;
				}
			}

			void copy(const char* s)
			{
				char* d = &data[length];
				int l = length;

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

			bool has_any(const char* _pattern)
			{
				return str_has_any(data, _pattern);
			}

			istring& operator += ( const char *_src )
			{
				copy(_src);
				return *this;
			}

			istring& operator = (double d)
			{
				sprintf_s(data, "%f", d);
				return *this;
			}

			char* next_token(char delim, int& start)
			{
				if (start < 0) start = 0;
				if (start >= size()) {
					start = size();
					return nullptr;
				}
				char* s = &data[start];
				char* e = s;
				while (*e && *e != delim) {
					start++;
					e = &data[start];
				}
				*e = 0;
				start++;
				return s;
			}

		};

		template<int l1, int l2> int compare(const istring<l1>& a, const istring<l2>& b)
		{
			return strcmp(a.c_str(), b.c_str());
		}

		template<int l1, int l2> bool operator<(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> bool operator>(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> bool operator>=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> bool operator<=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> bool operator==(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> bool operator!=(const istring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> int compare(const istring<l1>& a, const char* b)
		{
			return strcmp(a.c_str(), b);
		}

		template<int l1> bool operator<(const istring<l1>& a, const char* b)
		{
			return compare(a, b) < 0;
		}

		template<int l1> bool operator>(const istring<l1>& a, const char* b)
		{
			return compare(a, b) > 0;
		}

		template<int l1> bool operator>=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1> bool operator<=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1> bool operator==(const istring<l1>& a, const char* b)
		{
			return compare(a, b) == 0;
		}

		template<int l1> bool operator!=(const istring<l1>& a, const char* b)
		{
			return compare(a, b) != 0;
		}

		template<int l1, int l2> std::string operator+(const istring<l1>& a, const istring<l1>& b)
		{
			std::string temp = a.c_str();
			temp += b;
			return temp;
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


		template <int length_wchars> class iwstring
		{
			uint32_t length;
			int last_char;
			wchar_t data[length_wchars];
		public:

			iwstring() : last_char(length_wchars - 1)
			{

			}

			iwstring(const char* _src) : last_char(length_wchars - 1)
			{
				copy(_src);
			}

			iwstring(const std::string& src) : last_char(length_wchars - 1)
			{
				const char* s = src.c_str();
				copy(s);
			}

			iwstring(const wchar_t* _src) : last_char(length_wchars - 1)
			{
				copy(_src);
			}

			iwstring(const std::wstring& src) : last_char(length_wchars - 1)
			{
				const wchar_t* s = src.c_str();
				copy(s);
			}

			template <int src_length> iwstring(const istring<src_length>& srcp) : last_char(length_wchars - 1)
			{
				copy(srcp.c_str());
			}

			iwstring& operator = (const std::wstring& src)
			{
				const wchar_t* s = src.c_str();
				copy(s);
				return *this;
			}

			iwstring& operator = (const wchar_t* s)
			{
				copy(s);
				return *this;
			}

			iwstring& operator = (const std::string& src)
			{
				const char* s = src.c_str();
				copy(s);
				return *this;
			}

			iwstring& operator = (const char* s)
			{
				copy(s);
				return *this;
			}

			template <int src_length> iwstring& operator = (const istring<src_length>& srcp)
			{
				copy(srcp.c_str());
				return *this;
			}

			const wchar_t* c_str() const
			{
				return &data[0];
			}

			wchar_t* c_str_w() const
			{
				return &data[0];
			}

			const wchar_t* value() const
			{
				return &data[0];
			}

			uint32_t size() const
			{
				return length;
			}

			uint32_t capacity() const
			{
				return length_wchars -1;
			}

			void copy(const char* s)
			{
				int max_len = strlen(s);
				if (max_len >= last_char)
					max_len = last_char;

				int result = MultiByteToWideChar( CP_UTF8, 0, s, max_len, NULL, 0 );
				if (result > 0) 
				{
					while (result > last_char) {
						max_len--;
						result = MultiByteToWideChar(CP_UTF8, 0, s, max_len, NULL, 0);
					}
					if (result > 0)
					{
						MultiByteToWideChar(CP_UTF8, 0, s, max_len, data, result);
						data[result] = 0;
						length = result;
					}
					else {
						data[0] = 0;
						length = 0;
					}
				}
				else {
					data[0] = 0;
					length = 0;
				}
			}

			void copy(const wchar_t* s)
			{
				wchar_t* d = &data[0];
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

			bool has_any(const wchar_t* _pattern)
			{
				return str_has_any(data, _pattern);
			}

		};

		template<int l1, int l2> int compare(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return wcscmp(a.c_str(), b.c_str());
		}

		template<int l1, int l2> bool operator<(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> bool operator>(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> bool operator>=(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> bool operator<=(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> bool operator==(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> bool operator!=(const iwstring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> bool compare(const iwstring<l1>& a, const wchar_t* b)
		{
			return wcscmp(a.c_str(), b);
		}

		template<int l1> bool operator<(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) < 0;
		}

		template<int l1> bool operator>(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) > 0;
		}

		template<int l1> bool operator>=(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1> bool operator<=(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1> bool operator==(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) == 0;
		}

		template<int l1> bool operator!=(const iwstring<l1>& a, const wchar_t* b)
		{
			return compare(a, b) != 0;
		}

		template<int l1> std::wstring operator+(const iwstring<l1>& a, const wchar_t* b)
		{
			std::wstring temp = a.c_str();
			temp += b;
			return temp;
		}

		template<int l1> std::wstring operator+(const wchar_t* b, const iwstring<l1>& a)
		{
			std::wstring temp = b;
			temp += a.c_str();
			return temp;
		}

		template<int l1> std::wstring operator+(const iwstring<l1>& a, const std::wstring& b)
		{
			std::wstring temp = a.c_str();
			temp += b;
			return temp;
		}

		template<int l1> std::wstring operator+(const std::wstring& b, const iwstring<l1>& a)
		{
			std::wstring temp = b;
			temp += a.c_str();
			return temp;
		}

		template<int l1> std::ostream& operator <<(std::ostream& output, iwstring<l1>& src)
		{
			output << src.c_str();
			return output;
		}


		template<int l1, int l2> int compare(const iwstring<l1>& a, const istring<l2>& b)
		{
			iwstring<l2> temp = b;
			return wcscmp(a.c_str(), temp.c_str());
		}

		template<int l1, int l2> bool operator<(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> bool operator>(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> bool operator>=(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> bool operator<=(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> bool operator==(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> int operator!=(const iwstring<l1>& a, const istring<l2>& b)
		{
			return compare(a, b) != 0;
		}

		template<int l1, int l2> int compare(const istring<l1>& a, const iwstring<l2>& b)
		{
			iwstring<l2> temp = a;
			return wcscmp(temp.c_str(), b.c_str());
		}

		template<int l1, int l2> bool operator<(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) < 0;
		}

		template<int l1, int l2> bool operator>(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) > 0;
		}

		template<int l1, int l2> bool operator>=(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) >= 0;
		}

		template<int l1, int l2> bool operator<=(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) <= 0;
		}

		template<int l1, int l2> bool operator==(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) == 0;
		}

		template<int l1, int l2> int operator!=(const istring<l1>& a, const iwstring<l2>& b)
		{
			return compare(a, b) != 0;
		}


		bool string_tests();

		using object_name = istring<100>;
		using object_name_composed = istring<150>;
		using operation_name = istring<16>;
		using object_description = istring<200>;
		using object_path = istring<260>;
		using object_type = istring<16>;
		using string_validation_pattern = istring<100>;
		using string_validation_message = istring<100>;
		using query_body = istring<8192>;
		using file_path = istring<260>;

	}
}
