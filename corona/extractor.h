#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <charconv>
#include "messages.h"
#include "store_box.h"
#include "string_box.h"
#include "time_box.h"
#include "sorted_index.h"
#include "list_box.h"
#include "pobject.h"
#include "pobject_transformer.h"
#include <functional>

namespace countrybit
{
	namespace system
	{

		class get_number_result : public base_parse_result 
		{
		public:
			double value;
			get_number_result() {
				block = database::block_id::number_id();
				value = 0.0;
			}
		};

		class get_dimension_result : public base_parse_result
		{
		public:
			int x, y, z;
			get_dimension_result() {
				block = database::block_id::dimension_id();
				x = y = z = 0;
			}
		};

		class get_string_result : public base_parse_result
		{
		public:
			const char *value;
			get_string_result() {
				block = database::block_id::string_id();
				value = nullptr;
			}
		};

		class get_operator_result : public base_parse_result
		{
		public:

			const char* value;

			get_operator_result() 
			{
				block = database::block_id::operator_id();
				value = nullptr;
			}

			bool is_add() { return _stricmp(value, "+") == 0; }
			bool is_minus() { return _stricmp(value, "-") == 0; }
			bool is_multiply() { return _stricmp(value, "*") == 0; }
			bool is_divide() { return _stricmp(value, "/") == 0; }
			bool is_open() { return _stricmp(value, "(") == 0; }
			bool is_close() { return _stricmp(value, ")") == 0; }
			bool is_eq() { return _stricmp(value, "=") == 0; }
			bool is_gt() { return _stricmp(value, ">") == 0; }
			bool is_lt() { return _stricmp(value, "<") == 0; }
			bool is_gte() { return _stricmp(value, ">=") == 0; }
			bool is_lte() { return _stricmp(value, "<=") == 0; }
			bool is_contains() { return _stricmp(value, "@") == 0; }
			bool is_period() { return _stricmp(value, ".") == 0; }
		};

		class get_identifier_result: public base_parse_result
		{
		public:
			const char* value;
			get_identifier_result()
			{
				block = database::block_id::identifier_id();
				value = nullptr;
			}
		};

		class get_point_result : public base_parse_result
		{
		public:
			double x, y, z;
			get_point_result()
			{
				block = database::block_id::point_id();
				x = y = z = 0;
			}
		};

		class get_rectangle_result : public base_parse_result
		{
		public:
			double x, y, w, h;
			get_rectangle_result()
			{
				block = database::block_id::rectangle_id();
				x = y = w = h = 0;
			}
		};

		class get_audio_result : public base_parse_result
		{
		public:
			double	start_seconds,
					stop_seconds;
			double	pitch_adjust,
					volume_adjust;
			bool	playing;

			get_audio_result()
			{
				block = database::block_id::audio_id();
				start_seconds = stop_seconds = pitch_adjust = volume_adjust = 0.0;
				playing = false;
			}
		};

		class get_datetime_result : public base_parse_result
		{
		public:
			int years;
			int months;
			int days;
			int hours;
			int minutes;
			int seconds;
			int milliseconds;

			get_datetime_result()
			{
				block = database::block_id::datetime_id();
				years = months = days = hours = minutes = seconds = milliseconds = 0;
			}
		};

		class get_color_result : public base_parse_result
		{
		public:
			double red;
			double green;
			double blue;
			double alpha;

			get_color_result()
			{
				block = database::block_id::color_id();
				red = green = blue = 0.0;
				alpha = 1.0;
			}
		};

		class get_expression_result;

		using expression_term_collection = database::list_box<base_parse_result *>;

		class get_path_result : public base_parse_result
		{
		public:
			database::serialized_box_container* box;
			expression_term_collection terms;

			get_path_result(database::serialized_box_container* _box) : box(_box)
			{
				block = database::block_id::path_id();
				terms = expression_term_collection::create(box);
			}

			template <typename T>
			requires (database::named_block<T>)
			bool accept(T& gor)
			{
				T* item = box->clone(gor);							
				terms.push_back(item);
				return true;
			}
		};

		class get_sub_expression_result : public base_parse_result
		{
		public:
			database::serialized_box_container *box;
			expression_term_collection terms;

			get_sub_expression_result(database::serialized_box_container* _box) : box(_box)
			{
				terms = expression_term_collection::create(box);
			}

			template <typename T>
			requires (database::named_block<T>)
			bool accept(T& gor)
			{
				if (gor.block.is_identifer()) 
				{
					if (terms.size()) {
						if (terms.last()->block.is_path())
						{
							auto* p = (get_path_result*)terms.last();
							return p->accept(gor);
						}
						else
						{
							get_path_result gpr(box);
							if (gpr.accept(gor)) {
								return accept(gpr);
							}
							else 
							{
								return false;
							}
						}
					}
					else {
						get_path_result gpr(box);
						gpr.accept(gor);
						return accept(gpr);
					}
				}
				else if (gor.block.is_operator())
				{
					auto* p = (get_operator_result*)&gor;
					if (p->is_period() && terms.size() && terms.last()->block.is_path()) {
						auto* p = (get_path_result*)terms.last();
						return p->accept(gor);
					}
					else if (p->is_open() || p->is_close())
					{
						return false;
					}
					else 
					{
						auto t = box->clone(gor);
						terms.push_back(t);
						return true;
					}
				}
				else if (gor.block.is_string() ||
					gor.block.is_number() || 
					gor.block.is_path()
					)
				{
					auto t = box->clone(gor);
					terms.push_back(t);
					return true;
				}
				else
				{
					return false;
				}
			}
		};

		using expression_result_collection = std::vector<get_sub_expression_result*>;

		class get_expression_result : public base_parse_result
		{
			database::dynamic_box box;
			expression_result_collection stack;
			get_sub_expression_result* current;

			void push()
			{
				get_sub_expression_result gsr(&box);
				current = box.clone(gsr);
				stack.push_back(current);
			}

			void pop()
			{
				stack.pop_back();
				if (stack.size()) 
				{
					current = stack[stack.size()-1];
				}
			}

		public:

			get_expression_result() 
			{
				
			}

			void start()
			{
				push();
			}

			void stop()
			{
				pop();
			}

			template <typename T>
			requires (database::named_block<T>)
			bool accept(T& t)
			{
				if (!t.success)
				{
					message = t.message;
					line_number = t.line_number;
					char_offset = t.char_offset;
					return false;
				}

				if (t.block.is_operator())
				{
					get_operator_result* p = (get_operator_result*)&t;
					if (p->is_open())
					{
						push();
					}
					else if (p->is_close())
					{						
						pop();
					}
					else 
					{
						return current->accept(t);
					}
				}
				else 
				{
					return current->accept(t);
				}
			}
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

		class string_extractor
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
					identifier,
					datesep,
					operchars
				} search_type;

				int match;
				int count;
			};

			class match_result 
			{
				const char* src;
				match_group* groups;
				int start_index;
				int num_groups;

			public:

				int end_index;

				match_result() :
					src(nullptr),
					num_groups(0),
					groups(nullptr),
					start_index(0)
				{

				}

				bool is_empty()
				{
					return src == nullptr;
				}

				static match_result empty()
				{
					match_result empty_result;
					return empty_result;
				}

				match_result(const char* _src, int _num_groups, match_group* _groups, int _start_index) : 
					src(_src),
					num_groups(_num_groups),
					groups(_groups),
					start_index(_start_index)
				{
					;
				}

				int get_number(int group_index)
				{
					if (group_index >= num_groups || group_index < 0) {
						return 0;
					}
					auto g = groups[group_index];
					const char* first = g.match + src;
					const char* last = g.count + first;
					int value = 0;
					auto fcr = std::from_chars(first, last, value);
					return value;
				}

				std::string_view get_string(int group_index)
				{
					if (group_index >= num_groups || group_index < 0) {
						return std::string_view("");
					}
					auto g = groups[group_index];
					const char* first = g.match + src;
					return std::string_view(first, g.count);
				}

				const char* begin()
				{
					return start_index + src;
				}

				const char* end()
				{
					return end_index + src;
				}

			};

			int get_pattern_count(int& start_index, const std::function<bool(char c)>& item);
			match_result match(int start_index, int num_groups, match_group* group);

		public:

			std::string_view view;
			int index;
			int line;
			database::dynamic_box data;
			char currentChar[2];
			const char* type_member_name;

			string_extractor() : line(1), index(0), type_member_name(nullptr)
			{
				currentChar[0] = 0;
				currentChar[1] = 1;
			}

			string_extractor(database::string_box& src, int _data_length, const char* _type_member_name) : view(src.c_str(), src.size()), index(0), line(1), type_member_name(_type_member_name)
			{				
				data.init(_data_length);
			}

			string_extractor(const char *_str, int _length, int _data_length, const char *_type_member_name) : view(_str, _length), index(0), line(1), type_member_name(_type_member_name)
			{
				data.init(_data_length);
			}

			string_extractor(std::string& _str, int _data_length, const char *_type_member_name) : view(_str), index(0), line(1), type_member_name(_type_member_name)
			{
				data.init(_data_length);
			}

			string_extractor(std::string_view& _view, int _data_length, const char* _type_member_name) : view(_view), index(0), line(1), type_member_name(_type_member_name)
			{
				data.init(_data_length);
			}

			string_extractor(const string_extractor& _ctx) : view(_ctx.view), index(_ctx.index), data(_ctx.data), line(_ctx.line), type_member_name(_ctx.type_member_name)
			{
				
			}

			parse_json_array_result parse_matrix();
			parse_json_array_result parse_csv();

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

			bool skip_whitespace();

			get_number_result get_number();
			get_identifier_result get_identifier();
			get_string_result get_string();
			get_dimension_result get_dimensions();
			get_datetime_result get_date();
			get_color_result get_color();
			get_rectangle_result get_rectangle();
			get_point_result get_point();
			get_audio_result get_audio();
			get_operator_result get_operator();
			get_expression_result parse_expression();

			parse_json_value_result parse_json_value();
			parse_json_object_result parse_json_object();
			parse_json_array_result parse_json_array();

			static bool test_basics();
			static bool test_json(int _case_line, const std::string& _src, int _expected_failure_line);
			static bool test_json();
			static bool test_parser();

		};

	}
}
