#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include "messages.h"
#include "store_box.h"
#include "string_box.h"
#include "time_box.h"
#include "sorted_index.h"

#include <functional>

namespace countrybit
{
	namespace system
	{

		class pobject;
		class parray;
		class pvalue;

		class pmember_binding_base
		{
		public:

			const char* dest_name;
			const char* source_name;
			int			offset;
			int			type_code;

			pmember_binding_base( const char * _dest_name, const char* _source_name, int _offset ) :
				dest_name(_dest_name),
				source_name(_source_name), 
				offset(_offset)
			{
				type_code = 17;
				for (const char* c = source_name; *c; c++)
				{
					type_code = type_code * 23 + *c;
				}
			}

			int get_type_code()
			{
				return type_code;
			}

			virtual bool set(char *_base, const pvalue *_src) = 0;
		};

		class pvalue
		{
		public:

			int line;
			int index;

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

			bool set_value(database::string_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::string_value:
					dest = string_value;
					break;
				case pvalue_types::double_value:
					dest = double_value;
					break;
				}
			}

			void set_value(database::double_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = double_value;
					break;
				default:
					dest = 0.0;
					break;
				}
			}

			void set_value(database::float_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int8_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int8_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int16_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int16_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int32_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int32_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int64_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int64_t)double_value;
					break;
				}
			}

			void set_value(database::time_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::time_value:
					dest = (int64_t)time_value;
					break;
				}
			}

			const pvalue* next;
		};

		template <typename T, typename X> 
		class pmember_binding : public pmember_binding_base
		{
		public:

			pmember_binding( const char* _dest_name, const char* _source_name, X *_base, T *_ptr ) 
				: pmember_binding_base(_dest_name, _source_name, (char *)_ptr - (char *)_base)
			{

			}

			virtual bool set(char* _base, const pvalue* _src)
			{
				char *loc = _base + offset;
				boxed<T> item(loc);
				_src->set_value(item);
			}

		};

		class pobject_binding
		{
		public:

			using member_index_type = database::sorted_index<int, pmember_binding_base, 1>;
			using create_object_function = std::function<char* (const pobject*)>;
			using object_created_function = std::function<char* (char *, const pobject*)>;

			member_index_type member_index;
			database::row_id_type member_index_location;		

			create_object_function create_object;
			object_created_function object_created;

			template <typename BOX> 
			requires database::box<BOX,char>
			pobject_binding(BOX* _b, 
				int _max_items, 
				create_object_function _create_object,
				object_created_function _object_created)
			{
				create_object = _create;
				object_created = _created;
				member_index = member_index_type::create_sorted_index(_b, _max_items, member_index_location);
			}

			~pobject_binding()
			{

			}

			virtual void on_class(const pobject* obj)
			{
				char* new_base = create_object(obj);
				for (const pmember* im = obj->first; im; im = im->next)
				{
					int code = im->get_type_code();
					auto member = member_index[code];
					for (auto mi : member) {
						mi.second.set(new_base, im->value);
					}
				}
				object_created(new_base, obj);
			}

			int get_type_code()
			{
				int type_code = 0;
				for (auto member : member_index) {
					int mtc = member.second.get_type_code();
					type_code = type_code ^ mtc;
				}
				return type_code;
			}
		};

		class parray
		{
		public:
			int line;
			int index;

			int num_elements;
			const pvalue* first;
		};

		class pmember
		{
		public:
			int line;
			int index;

			const char* name;
			const pvalue* value;
			pmember* next;

			int get_type_code() const
			{
				int code = 17;
				for (const char* c = name; *c; c++)
				{
					code = code * 23 + *c;
				}
				return code;
			}
		};

		class pobject
		{
		public:
			int line;
			int index;

			int num_members;
			pmember *first;

			int get_type_code() const
			{
				int type_code = 0;
				for (pmember* mb = first; mb; mb = mb->next)
				{
					int mtc = mb->get_type_code();
					type_code = type_code ^ mtc;
				}
				return type_code;
			}
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

			pobject_binding *create_object_binding()
			{
				;
			}

			void create_member_binding()
			{
				;
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
