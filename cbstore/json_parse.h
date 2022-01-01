#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include "messages.h"
#include "store_box.h"

namespace countrybit
{
	namespace system
	{

		class jsobject;

		class jsvalue
		{
		public:

			enum class jvalue_types
			{
				string_value,
				double_value,
				time_value,
				array_value,
				object_value
			};

			const char* name;
			jvalue_types jvalue_type;

			const char* string_value;
			double double_value;
			time_t time_value;

			int num_values;
			const jsobject* object_values[];
		};

		class jsobject
		{
		public:
			int num_members;
			jsvalue *members[];
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
			const char *value;
		};

		class get_object_result : public base_parse_result
		{
		public:
			const jsobject *value;
		};

		class get_array_result : public base_parse_result
		{
		public:
			int num_values;
			const jsobject *value[];
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

			int match(int start_index, int num_groups, match_group* group);

		public:

			std::string_view view;
			int index;
			int line;
			database::dynamic_box data;

			parser() = default;

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
				return is_end() ? 0 : view[idx];
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
			get_object_result get_object();
			get_array_result get_array();

		};

	}
}
