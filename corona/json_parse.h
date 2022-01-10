#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include "messages.h"
#include "store_box.h"
#include <functional>

namespace countrybit
{
	namespace system
	{

		class pobject;
		class parray;

		class pvalue
		{
		public:

			enum class pvalue_types
			{
				string_value,
				double_value,
				time_value,
				array_value,
				object_value
			};

			pvalue_types pvalue_type;

			const char* string_value;
			double double_value;
			time_t time_value;
			const pobject* object_value;
			const parray* array_value;

			const pvalue* next;
		};

		class parray
		{
		public:
			int num_elements;
			const pvalue* first;
		};

		class pmember
		{
		public:
			const char* name;
			const pvalue* value;
			pmember* next;
		};

		class pobject
		{
		public:
			int num_members;
			pmember *first;
		};

		class get_number_result : public base_parse_result 
		{
		public:
			double value;
		};

		class get_string_result : public base_parse_result
		{
		public:
			const char *value;
		};

		class get_identifier_result : public base_parse_result
		{
		public:
			char *value;
		};

		class parse_json_object_result : public base_parse_result
		{
		public:
			pobject *value;
		};

		class parse_json_value_result : public base_parse_result
		{
		public:
			pvalue* value;
		};

		class parse_json_array_result : public base_parse_result
		{
		public:
			parray *value;
		};

		class parser
		{
			struct match_group
			{
				enum class search_counts
				{
					search_optional_one,
					search_one,
					search_optional_many,
					search_many
				} search_counts;

				enum class search_types
				{
					digits,
					digits_and_seps,
					period,
					space,
					plusminus,
					E,
					dollar,
					comma,
					alpha,
					identifier
				} search_type;

				int match;
			};

			int get_pattern_count(int& start_index, const std::function<bool(char c)>& item);
			int match(int start_index, int num_groups, match_group* group);

		public:

			std::string_view view;
			int index;
			int line;
			database::dynamic_box data;
			char currentChar[2];

			parser() : line(1), index(0)
			{
				currentChar[0] = 0;
				currentChar[1] = 1;
			}

			parser(char *_str, int _length, int _data_length) : view(_str, _length), index(0), line(1)
			{
				data.init(_data_length);
			}

			parser(std::string& _str, int _data_length) : view(_str), index(0), line(1)
			{
				data.init(_data_length);
			}

			parser(std::string_view& _view, int _data_length) : view(_view), index(0), line(1)
			{
				data.init(_data_length);
			}

			parser(const parser& _ctx) : view(_ctx.view), index(_ctx.index), data(_ctx.data), line(_ctx.line)
			{
				
			}

			inline char operator [](int idx)
			{
				return at(idx);
			}

			inline char at(int idx)
			{
				char c = idx >= view.length() ? 0 : view[idx];
				currentChar[0] = c;
				return c;
			}

			bool is_end()
			{
				return index >= view.length();
			}

			inline char get_advance(char c1)
			{
				char c = at(index);
				if (c == c1) {
					index++;
					return c;
				}
				return 0;
			}

			inline char get_char()
			{
				char c = at(index);
				index++;
				return c;
			}

			inline char get_quote()
			{
				return get_advance('"');
			}

			inline char get_escaped_quote()
			{
				char c = at(index);
				char c1 = at(index + 1);
				if ((c == '\\' && c1 == '"') || (c == '"' && c1 == '"')) {
					index += 2;
					return c1;
				}
				return 0;
			}
			
			struct get_char_struct 
			{
				char c;
				bool escaped;
			};

			inline get_char_struct get_string_char()
			{

				get_char_struct gcs;

				gcs.c = 0;
				gcs.escaped = false;

				char c = at(index);
				char c1 = at(index + 1);

				if ((c == '\\' && c1 == '"') || (c == '"' && c1 == '"')) 
				{
					index += 2;
					gcs.c = '"';
					gcs.escaped = true;
					return gcs;
				}
				else if (c == '"')
				{
					gcs.c = '"';
					gcs.escaped = false;
					index++;
					return gcs;
				}
				else if (c) 
				{
					index++;
					gcs.c = c;
					gcs.escaped = false;
					return gcs;
				}
				
				return gcs;
			}

			inline char get_space()
			{
				char c = at(index);
				if (c <= 0x20) {
					index ++;
					if (c == '\n') {
						line++;
					}
					return c;
				}
				return 0;
			}

			inline char get_escaped_char()
			{
				char c = get_advance('\\');
				if (c) {
					c = at(0);
				}
				return c;
			}

			inline char get_colon()
			{
				return get_advance(':');
			}

			inline char get_comma()
			{
				return get_advance(',');
			}

			inline char get_object_start()
			{
				return get_advance('{');
			}

			inline char get_object_stop()
			{
				return get_advance('}');
			}

			inline char get_array_start()
			{
				return get_advance('[');
			}

			inline char get_array_stop()
			{
				return get_advance(']');
			}

			inline char get_escape()
			{
				return get_advance('\\');
			}

			inline char get_digit()
			{
				char c = at(index);
				if (std::isalnum(c)) {
					index++;
					return c;
				}
				return 0;
			}

			void skip_whitespace();

			get_number_result get_number();
			get_identifier_result get_identifier();
			get_string_result get_string();

			parse_json_value_result parse_json_value();
			parse_json_object_result parse_json_object();
			parse_json_array_result parse_json_array();

			static bool test_basics();
			static bool test_json(int _case_line, const std::string& _src, int _expected_failure_line);
			static bool test_json();

		};

	}
}
