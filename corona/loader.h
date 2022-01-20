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

#include "pobject.h"
#include "extractor.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace system
	{
		class pmember_binding_base
		{
		public:

			const char* dest_name;
			const char* source_name;
			int			offset;
			int			type_code;
			bool		required;

			pmember_binding_base(const char* _dest_name, const char* _source_name, int _offset, bool _required) :
				dest_name(_dest_name),
				source_name(_source_name),
				offset(_offset),
				required(_required)
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

			virtual bool set(char* _base, const pvalue* _src) = 0;
		};

		using create_object_function = std::function<char* (const pobject*)>;
		using object_created_function = std::function<char* (char*, const pobject*)>;
		using object_member_index_type = database::sorted_index<int, pmember_binding_base*, 1>;

		class pobject_binding
		{
		public:

			object_member_index_type member_index;
			database::row_id_type member_index_location;

			create_object_function create_object;
			object_created_function object_created;

			const char* type_key;
			const char* type_value;
			const char* member_name;

			const char* class_name;

			template <typename BOX>
				requires database::box<BOX, char>
			pobject_binding(BOX* _b,
				const char* _type_key,
				const char* _type_value,
				const char* _member_name,
				const char* _class_name,
				int _max_items,
				create_object_function _create_object,
				object_created_function _object_created)
			{
				type_key = _type_key;
				type_value = _type_value;
				member_name = _member_name;
				class_name = _class_name;
				create_object = _create_object;
				object_created = _object_created;
				member_index = object_member_index_type::create_sorted_index(_b, _max_items, member_index_location);
			}

			~pobject_binding()
			{

			}

			virtual void on_object(const pobject* obj)
			{
				char* new_base = create_object(obj);
				for (const pmember* im = obj->first; im; im = im->next)
				{
					int code = im->get_type_code();
					auto member = member_index[code];
					for (auto mi : member) {
						mi.second->set(new_base, im->value);
					}
				}
				object_created(new_base, obj);
			}

			int get_type_code()
			{
				int type_code = 0;
				for (auto member : member_index) {
					int mtc = member.second->get_type_code();
					type_code = type_code ^ mtc;
				}
				return type_code;
			}
		};

		template <typename T, typename X>
		class pmember_binding : public pmember_binding_base
		{
		public:

			pmember_binding(const char* _dest_name, const char* _source_name, X* _base, T* _ptr, bool _required)
				: pmember_binding_base(_dest_name, _source_name, (char*)_ptr - (char*)_base, _required)
			{

			}

			virtual bool set(char* _base, const pvalue* _src)
			{
				char* loc = _base + offset;
				database::boxed<T> item(loc);
				_src->set_value(item);
			}

		};

		class loader 
		{
		protected:
			database::dynamic_box data;
		public:

			loader(int _size)
			{
				data.init(_size);
			}

			pobject_binding* begin_object_create_binding(
				const char* _type_key,
				const char* _type_value,
				const char* _member_name,
				const char* _class_name,
				int _max_items,
				create_object_function _create_object,
				object_created_function _object_created)
			{

				pobject_binding* new_binding = nullptr;

				const char* class_name = data.allocate_extracted(_class_name, 0);
				if (_class_name && !class_name)
					return new_binding;
				const char* type_key = data.allocate_extracted(_type_key, 0);
				if (_type_key && !type_key)
					return new_binding;
				const char* type_value = data.allocate_extracted(_type_value, 0);
				if (_type_value && !type_value)
					return new_binding;
				const char* member_name = data.allocate_extracted(_member_name, 0);
				if (_member_name && !member_name)
					return new_binding;

				char* loc = data.place<pobject_binding>();
				if (loc) {
					new_binding = new (loc) pobject_binding(&data, type_key, type_value, member_name, class_name, _max_items, _create_object, _object_created);
				}
				return new_binding;
			}

			template <typename T, typename X> pmember_binding<T, X>* add_object_member_binding(pobject_binding* object, const char* _dest_name, const char* _source_name, X* _base, T* _ptr, bool _required)
			{
				pmember_binding<T, X>* new_binding = nullptr;
				const char* dest_name = data.allocate_extracted(_dest_name, 0);
				const char* source_name = data.allocate_extracted(_source_name, 0);

				if (!dest_name && !source_name) {
					return nullptr;
				}

				if (dest_name) {
					
					if (source_name) {
						char* loc = data.place<pobject_binding>();
						if (loc) {
							new_binding = new (loc) pmember_binding<T, X>(dest_name, source_name, X, T, _required);
							int idx = new_binding->get_type_code();
							object->member_index.insert_or_assign(std::pair<int, pmember_binding_base>(idx, new_binding));
						}
					}
				}
				return new_binding;
			}
		};

		class schema_loader : loader
		{

		public:

			database::table<database::add_string_field_request> add_string_fields;
			database::table<database::add_integer_field_request> add_integer_fields;
			database::table<database::add_double_field_request> add_double_fields;
			database::table<database::add_time_field_request> add_time_fields;
			database::table<database::add_object_field_request> add_object_fields;
			database::table<database::add_query_field_request> add_query_fields;

			database::row_id_type add_string_fields_id;
			database::row_id_type add_integer_fields_id;
			database::row_id_type add_double_fields_id;
			database::row_id_type add_time_fields_id;
			database::row_id_type add_object_fields_id;
			database::row_id_type add_query_fields_id;

			schema_loader(int _size, int _num_fields) : loader(_size)
			{
				add_string_fields = database::table<database::add_string_field_request>::create_table(&data, _num_fields, add_string_fields_id);
				add_integer_fields = database::table<database::add_integer_field_request>::create_table(&data, _num_fields, add_integer_fields_id);
				add_double_fields = database::table<database::add_double_field_request>::create_table(&data, _num_fields, add_double_fields_id);
				add_time_fields = database::table<database::add_time_field_request>::create_table(&data, _num_fields, add_time_fields_id);
				add_object_fields = database::table<database::add_object_field_request>::create_table(&data, _num_fields, add_object_fields_id);
				add_query_fields = database::table<database::add_query_field_request>::create_table(&data, _num_fields, add_query_fields_id);
			}

			void bind_add_string_field_request()				
			{
				auto tb = &add_string_fields;
				auto b = begin_object_create_binding(
					nullptr,
					nullptr,
					"stringfields",
					"stringfield",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_string;
						t.options = {};
						return (char*)&t;
					},
					[](char *b, const pobject* obj) {
						return b;
					});

				database::add_string_field_request dummy;

				add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
				add_object_member_binding(b, "length", "length", &dummy, &dummy.options.length, true);
				add_object_member_binding(b, "string", "string", &dummy, &dummy.name.type, true);
				add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
				add_object_member_binding(b, "validation_message", "validation_message", &dummy, &dummy.options.validation_message, false);
				add_object_member_binding(b, "validation_pattern", "validation_pattern", &dummy, &dummy.options.validation_pattern, false);
			}

			void bind_add_integer_field_request(database::jtype jt, const char *member_name, const char *name)
			{
				auto tb = &add_integer_fields;
				auto b = begin_object_create_binding(
					nullptr,
					nullptr,
					member_name,
					name,
					100,
					[tb, jt](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = jt;
						t.options = {};
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					});

				database::add_integer_field_request dummy;

				add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
				add_object_member_binding(b, name, name, &dummy, &dummy.name.type, true);
				add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
				add_object_member_binding(b, "min_int", "min_int", &dummy, &dummy.options.minimum_int, false);
				add_object_member_binding(b, "max_int", "max_int", &dummy, &dummy.options.maximum_int, false);
			}

			void bind_integer_fields()
			{
				bind_add_integer_field_request(database::jtype::type_int16, "int16fields", "int16");
				bind_add_integer_field_request(database::jtype::type_int32, "int32fields", "int32");
				bind_add_integer_field_request(database::jtype::type_int64, "int64fields", "int64");
			}

			void bind_add_double_field_request(database::jtype jt, const char* member_name, const char* name)
			{
				auto tb = &add_double_fields;
				auto b = begin_object_create_binding(
					nullptr,
					nullptr,
					member_name,
					name,
					100,
					[tb, jt](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = jt;
						t.options = {};
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					});

				database::add_double_field_request dummy;

				add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
				add_object_member_binding(b, name, name, &dummy, &dummy.name.type, true);
				add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
				add_object_member_binding(b, "min_double", "min_double", &dummy, &dummy.options.minimum_double, false);
				add_object_member_binding(b, "max_double", "max_double", &dummy, &dummy.options.maximum_double, false);
			}

			void bind_double_fields()
			{
				bind_add_double_field_request(database::jtype::type_float32, "floatfields", "float");
				bind_add_double_field_request(database::jtype::type_float64, "doublefields", "double");
			}

			void bind_add_time_field_request()
			{
				auto tb = &add_time_fields;
				auto b = begin_object_create_binding(
					nullptr,
					nullptr,
					"datetimefields",
					"datetime",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_datetime;
						t.options = {};
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					});

				database::add_time_field_request dummy;

				add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
				add_object_member_binding(b, "datetime", "datetime", &dummy, &dummy.name.type, true);
				add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
				add_object_member_binding(b, "min_time", "min_time", &dummy, &dummy.options.minimum_time_t, false);
 				add_object_member_binding(b, "max_time", "max_time", &dummy, &dummy.options.maximum_time_t, false);
			}

			void bind_add_object_field_request()
			{
				auto tb = &add_time_fields;
				auto b = begin_object_create_binding(
					nullptr,
					nullptr,
					"objectfields",
					"object",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_object;
						t.options = {};
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					});

				database::add_object_field_request dummy;

				add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
				add_object_member_binding(b, "datetime", "datetime", &dummy, &dummy.name.type, true);
				add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
				add_object_member_binding(b, "min_time", "min_time", &dummy, &dummy.options.class_id, false);
				add_object_member_binding(b, "max_time", "max_time", &dummy, &dummy.options.dim.x, false);
				add_object_member_binding(b, "max_time", "max_time", &dummy, &dummy.options.dim.y, false);
				add_object_member_binding(b, "max_time", "max_time", &dummy, &dummy.options.dim.z, false);
			}

			void bind_add_query_field_request()
			{

			}

		};

		class data_loader
		{

		};
	}
}

