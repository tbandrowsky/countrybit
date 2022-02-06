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

		class property_source
		{
			const char* source_name;
			int			type_code;

		public:

			property_source() = default;

			property_source(const char* _source_name) : source_name(_source_name), type_code(17)
			{
				for (const char* c = source_name; *c; c++)
				{
					type_code = type_code * 23 + *c;
				}
			}

			const char* get_source_name()
			{
				return source_name;
			}

			int get_type_code()
			{
				return type_code;
			}

		};

		class property_dest
		{
		public:
			const char* dest_name;
			property_dest* next;

			property_dest() = default;

			property_dest(const char *_dest_name) :
				dest_name(_dest_name),
				next(nullptr)
			{
				;
			}

			virtual bool is_match(const pvalue* _src) = 0;
			virtual bool set_value(char* _base, const pvalue* _src) = 0;
		};

		class property_base
		{
		public:

			property_source source;
			property_dest* first_setter,
				* last_setter;

			property_base() = default;

			property_base(const char* _source_name) :
				source(_source_name),
				first_setter(nullptr),
				last_setter(nullptr)
			{

			}

			inline int get_type_code()
			{
				return source.get_type_code();
			}

			void add_setter(property_dest* _new_setter)
			{
				if (!first_setter) {
					first_setter = _new_setter;
					last_setter = _new_setter;
				}
				else
				{
					last_setter->next = _new_setter;
					last_setter = _new_setter;
				}
			}

			property_dest* get_setter(const pvalue* src)
			{
				for (auto pd = first_setter; pd; pd = pd->next)
				{
					if (pd->is_match(src))
						return pd;
				}

				return nullptr;
			}
		};

		using property_index_type = database::sorted_index<int, property_base*, 1>;

		class typeinfo
		{
			property_index_type				property_index;
			database::row_id_type			property_index_id;

			const char* type_key;
			const char* type_value;
			const char* member_name;
			const char* class_name;

		public:

			database::object_name			key;

			template <typename BOX>
				requires database::box<BOX, char>
			typeinfo(BOX* _b,
				const char* _type_key,
				const char* _type_value,
				const char* _class_name,
				int _max_items)
			{
				type_key = _type_key;
				type_value = _type_value;
				class_name = _class_name;
				create_object = _create_object;
				property_index = object_member_index_type::create_sorted_index(_b, _max_items, property_index_id);

				key = type_key;
				key = key + "_";
				key = key + type_value;
			}

			~typeinfo()
			{

			}

			virtual bool is_match(const pvalue* obj)
			{
				if (auto pvo = obj->as_object()) {
					auto m = pvo->get_member(type_key);
					if (m && m->value && m->value->pvalue_type == pvalue::pvalue_types::string_value && m->value->string_value)
					{
						return strcmp(m->value->string_value, type_value) == 0;
					}
				}
				return false;
			}

			property_base* add_property(property_base* pb)
			{
				int idx = pb->get_type_code();
				property_index.insert_or_assign(idx, pb);
				return pb;
			}

			property_base* find_property(pmember* pm)
			{
				int idx = pm->get_type_code();
				auto piter = property_index[idx];
				property_base* found = piter != std::end(property_index) ? piter->second : nullptr;
				return found;
			}
		};

		template <typename MemberType>
		class scalar_property_dest : public property_dest
		{
		public:
			int			offset;
			pvalue::pvalue_types match_type;

			scalar_property_dest(const char* _dest_name, int _offset, pvalue::pvalue_types _match_type) :
				property_dest(_dest_name, _offset),
				offset(_offset),
				match_type(_match_type)
			{
				;
			}

			virtual bool is_match(const pvalue* _src)
			{
				return	_src->pvalue_type == match_type;
			}

			virtual bool set_value(char* _base, const pvalue* _src)
			{

				if (_src->as_object() || _src->as_string()) {
					throw std::logic_error("attempt to map non-scalar to scalar");
				}
				char* loc = _base + offset;
				database::boxed<MemberType> item(loc);
				_src->set_value(item);
				return true;
			}
		};

		class object_property_dest : public property_dest
		{
		public:

			int				offset;
			typeinfo*		property_type;

			object_property_dest(const char* _dest_name, int _offset, typeinfo* _property_type) :
				property_dest(_dest_name),
				offset(_offset),
				property_type(_property_type)
			{
				;
			}

			virtual bool is_match(const pvalue* _src)
			{
				return property_type->is_match(_src);
			}

			virtual bool set_value(char* _base, const pvalue* _src)
			{
				char* loc = _base + offset;

				const pobject* pv = _src->as_object();

				if (pv == nullptr) 
				{
					throw std::logic_error("attempt to map non-object to object");
				}

				for (auto member = pv->first; member; member = member->next)
				{
					auto prop = property_type->find_property(member);
					if (prop) {
						auto setter = prop->get_setter(member->value);
						if (setter) {
							setter->set_value(loc, member->value);
						}
					}
				}

				return true;
			}
		};

		template <typename ArrayMemberType, typename ArraySize>
		class object_iarray_property_dest : public property_dest
		{
		public:

			int				offset;
			typeinfo* property_type;

			object_iarray_property_dest(const char* _dest_name, int _offset, typeinfo* _property_type) :
				property_dest(_dest_name),
				offset(_offset),
				property_type(_property_type)
			{
				;
			}

			virtual bool is_match(const pvalue* _src)
			{
				return property_type->is_match(_src);
			}

			virtual bool set_value(char* _base, const pvalue* _src)
			{
				char* loc = _base + offset;

				database::iarray<ArrayMemberType, ArraySize> *objarray = (database::iarray<ArrayMemberType, ArraySize> *)( loc );

				ArrayMemberType *new_item = objarray->append();

				char* new_base = (char *)new_item;

				const pobject* pv = _src->as_object();

				if (pv == nullptr)
				{
					throw std::logic_error("attempt to map non-object to object");
				}

				for (auto member = pv->first; member; member = member->next)
				{
					auto prop = property_type->find_property(member);
					if (prop) {
						auto setter = prop->get_setter(member->value);
						if (setter) {
							setter->set_value(new_base, member->value);
						}
					}
				}

				return true;
			}
		};

		class loader 
		{
		protected:

			database::dynamic_box data;
			database::sorted_index<database::object_name, typeinfo*> bindings_by_name;

		public:

			loader(int _size, int _max_types) 
			{
				database::row_id_type header_location;
				data.init(_size);
				bindings_by_name.create_sorted_index(&data, _max_types, header_location);
			}

			typeinfo* create_typeinfo(
				const char* _type_key,
				const char* _type_value,
				const char* _class_name,
				int _max_items)
			{

				typeinfo* nti = nullptr;

				char* loc = data.place<typeinfo>();
				if (loc) {
					nti = new (loc) typeinfo(&data, _type_key, _type_value, _class_name, _max_items);
					auto inserted = bindings_by_name.insert_or_assign(nti->key, nti);
					if (inserted == std::end(bindings_by_name))
					{
						nti = nullptr;
					}
				}
				return nti;
			}

			typeinfo* find_typeinfo(const pobject* obj)
			{
				typeinfo* ti = nullptr;

				pmember* pm = obj->type_member;

				if (!pm)
					return nullptr;

				const char *type_name = pm->value->as_string();
				if (!type_name)
					return nullptr;

				database::object_name key = pm->name;
				key = key + "_";
				key = key + type_name;

				auto iter = bindings_by_name[key];
				if (iter != std::end(bindings_by_name))
				{
					ti = iter->second;
					return ti;
				}

				return ti;
			}

			void load(const pvalue* _src)
			{

			}

		};

		class schema_loader : loader
		{
			class load_id
			{
			public:
				database::jtype			request_item_type;
				database::row_id_type	request_id;
			};

			enum class errors {
				could_not_create_binding = 1,
				could_not_find_binding = 2,
				invalid_binding_name_type = 3,
			};

			class error_message
			{
			public:
				errors error;
				const pobject* obj;
			};

			database::table<error_message> error_messages;
			database::table<database::add_string_field_request> add_string_fields;
			database::table<database::add_integer_field_request> add_integer_fields;
			database::table<database::add_double_field_request> add_double_fields;
			database::table<database::add_time_field_request> add_time_fields;
			database::table<database::add_object_field_request> add_object_fields;
			database::table<database::add_named_query_field_request> add_query_fields;
			database::table<database::add_point_field_request> add_point_fields;
			database::table<database::add_rectangle_field_request> add_rectangle_fields;
			database::table<database::add_image_field_request> add_image_fields;
			database::table<database::add_wave_field_request> add_wave_fields;
			database::table<database::add_midi_field_request> add_midi_fields;
			database::table<database::add_color_field_request> add_color_fields;
			database::table<load_id> field_load_order;

			database::row_id_type error_messages_id;
			database::row_id_type add_string_fields_id;
			database::row_id_type add_integer_fields_id;
			database::row_id_type add_double_fields_id;
			database::row_id_type add_time_fields_id;
			database::row_id_type add_object_fields_id;
			database::row_id_type add_query_fields_id;
			database::row_id_type add_point_fields_id;
			database::row_id_type add_rectangle_fields_id;
			database::row_id_type add_image_fields_id;
			database::row_id_type add_wave_fields_id;
			database::row_id_type add_midi_fields_id;
			database::row_id_type field_load_order_id;

			const char* member_type_name = "type";

		public:

			schema_loader(int _size, int _num_fields) : loader(_size, _num_fields)
			{
				error_messages = database::table<error_message>::create_table(&data, 250, error_messages_id);
				add_string_fields = database::table<database::add_string_field_request>::create_table(&data, _num_fields, add_string_fields_id);
				add_integer_fields = database::table<database::add_integer_field_request>::create_table(&data, _num_fields, add_integer_fields_id);
				add_double_fields = database::table<database::add_double_field_request>::create_table(&data, _num_fields, add_double_fields_id);
				add_time_fields = database::table<database::add_time_field_request>::create_table(&data, _num_fields, add_time_fields_id);
				add_object_fields = database::table<database::add_object_field_request>::create_table(&data, _num_fields, add_object_fields_id);
				add_query_fields = database::table<database::add_named_query_field_request>::create_table(&data, _num_fields, add_query_fields_id);
				add_point_fields = database::table<database::add_point_field_request>::create_table(&data, _num_fields, add_point_fields_id);
				add_rectangle_fields = database::table<database::add_rectangle_field_request>::create_table(&data, _num_fields, add_rectangle_fields_id);
				add_image_fields = database::table<database::add_image_field_request>::create_table(&data, _num_fields, add_image_fields_id);
				add_wave_fields = database::table<database::add_wave_field_request>::create_table(&data, _num_fields, add_wave_fields_id);
				add_midi_fields = database::table<database::add_midi_field_request>::create_table(&data, _num_fields, add_midi_fields_id);
				field_load_order = database::table<load_id>::create_table(&data, _num_fields, field_load_order_id);
			}

			bool init()
			{
				return add_bindings();
			}

			void add_error(errors _error, const pobject* _obj)
			{
				database::row_range rr;
				error_message em;
				em.error = _error;
				em.obj = _obj;
				error_messages.insert(em, rr);
			}

		protected:

			bool bind_add_string_field_request()				
			{
				auto tb = &add_string_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"string",
					nullptr,
					"string",
					10,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_string;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_string;
						return (char*)&t;
					},
					[](char *b, const pobject* obj) {
						return b;
					},
					[](char*b, const pobject*obj, const pmember*m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b)
				{
					database::add_string_field_request dummy;
					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "length", "length", &dummy, &dummy.options.length, true);
					add_object_member_binding(b, "string", "string", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "validation_message", "validation_message", &dummy, &dummy.options.validation_message, false);
					add_object_member_binding(b, "validation_pattern", "validation_pattern", &dummy, &dummy.options.validation_pattern, false);
					return true;
				}
				else 
				{
					add_error(errors::could_not_create_binding, nullptr);
				}

				return false;
			}

			bool bind_add_integer_field_request(database::jtype jt, const char *member_name, const char *name)
			{
				auto tb = &add_integer_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					name,
					nullptr,
					name,
					100,
					[tb, jt](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = jt;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = jt;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {

					database::add_integer_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, name, name, &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "min_int", "min_int", &dummy, &dummy.options.minimum_int, false);
					add_object_member_binding(b, "max_int", "max_int", &dummy, &dummy.options.maximum_int, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_integer_fields()
			{
				bool r = true;
				r = bind_add_integer_field_request(database::jtype::type_int16, "int16_fields", "int16");
				if (!r) return r;
				r = bind_add_integer_field_request(database::jtype::type_int32, "int32_fields", "int32");
				if (!r) return r;
				r = bind_add_integer_field_request(database::jtype::type_int64, "int64_fields", "int64");
				if (!r) return r;
				return r;
			}

			bool bind_add_double_field_request(database::jtype jt, const char* member_name, const char* name)
			{
				auto tb = &add_double_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					name,
					nullptr,
					name,
					100,
					[tb, jt](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = jt;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = jt;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_double_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, name, name, &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "min_double", "min_double", &dummy, &dummy.options.minimum_double, false);
					add_object_member_binding(b, "max_double", "max_double", &dummy, &dummy.options.maximum_double, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_double_fields()
			{
				bool r = true;
				r = bind_add_double_field_request(database::jtype::type_float32, "float_fields", "float");
				if (!r) return r;
				r = bind_add_double_field_request(database::jtype::type_float64, "double_fields", "double");
				if (!r) return r;
				return r;
			}

			bool bind_add_time_field_request()
			{
				auto tb = &add_time_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"datetime",
					nullptr,
					"datetime",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_datetime;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_datetime;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue *pv) {
						return b;
					});

				if (b) {
					database::add_time_field_request dummy;
					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "datetime", "datetime", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "min_time", "min_time", &dummy, &dummy.options.minimum_time_t, false);
					add_object_member_binding(b, "max_time", "max_time", &dummy, &dummy.options.maximum_time_t, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_object_field_request()
			{
				auto tb = &add_object_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"object",
					nullptr,
					"object",
					10,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_object;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_object;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_object_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "datetime", "datetime", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "class_name", "class_name", &dummy, &dummy.options.class_name, false);
					add_object_member_binding(b, "x", "x", &dummy, &dummy.options.dim.x, false);
					add_object_member_binding(b, "y", "y", &dummy, &dummy.options.dim.y, false);
					add_object_member_binding(b, "z", "z", &dummy, &dummy.options.dim.z, false);
				}
				else 
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
			}

			bool bind_add_query_field_request()
			{
				auto tb = &add_query_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"query",
					nullptr,
					"query",
					10,
					[tb,this](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_query;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_query;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						// living with this for the moment
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						database::add_named_query_field_request *anqr = (database::add_named_query_field_request*)b;
						if (stricmp(m->name, "filter")) {
							database::filter_element_request *fer = anqr->options.filter.append();
						}
						else if (stricmp(m->name, "projection")) {
							anqr->options.projection
						}
						return b;
					});

				if (b) {
					database::add_query_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "query", "query", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_query_projection_request()
			{
				auto cq = current_query;
				auto b = create_object_binding(
					member_type_name,
					"projection",
					nullptr,
					"projection",
					100,
					[cq](const pobject* obj) {
						auto t = cq->options.projection.append();
						if (t) {
							t->error_message = nullptr;
						}
						return (char*)t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::projection_element_request dummy;
					add_object_member_binding(b, "field_name", "field_name", &dummy, &dummy.field_name, true);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_query_filter_request()
			{
				auto cq = current_query;
				auto b = create_object_binding(
					member_type_name,
					"filter",
					nullptr,
					"filter",
					100,
					[cq](const pobject* obj) {
						auto t = cq->options.filter.append();
						if (t) {
							t->error_message = nullptr;
						}
						return (char*)t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::filter_element_request dummy;
					add_object_member_binding(b, "target_field", "target_field", &dummy, &dummy.target_field_name, true);
					add_object_member_binding(b, "comparison", "comparison", &dummy, &dummy.comparison_name, true);
					add_object_member_binding(b, "parameter_field", "parameter_field", &dummy, &dummy.parameter_field_name, true);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}


			bool bind_add_point_field_request()
			{
				auto tb = &add_point_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"point",
					nullptr,
					"point",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_point;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_point;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_time_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "point", "point", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_rectangle_field_request()
			{
				auto tb = &add_rectangle_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"rectangle",
					nullptr,
					"rectangle",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_rectangle;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_rectangle;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_rectangle_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "rectangle", "rectangle", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}

				return false;
			}

			bool bind_add_image_field_request()
			{
				auto tb = &add_rectangle_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"image",
					nullptr,
					"image",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_image;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_image;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_image_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "image", "image", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "filename", "filename", &dummy, &dummy.options.image_path, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_wave_field_request()
			{
				auto tb = &add_rectangle_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"wave",
					nullptr,
					"wave",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_wave;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_wave;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_wave_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "image", "image", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "filename", "filename", &dummy, &dummy.options.image_path, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool bind_add_midi_field_request()
			{
				auto tb = &add_rectangle_fields;
				auto fl = &field_load_order;
				auto b = create_object_binding(
					member_type_name,
					"midi",
					nullptr,
					"midi",
					100,
					[tb](const pobject* obj) {
						database::row_range rr;
						auto& t = tb->create(1, rr);
						t.name = {};
						t.name.field_id = 0;
						t.name.type_id = database::jtype::type_midi;
						t.options = {};
						load_id li;
						li.request_id = rr.start;
						li.request_item_type = database::jtype::type_midi;
						return (char*)&t;
					},
					[](char* b, const pobject* obj) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m) {
						return b;
					},
					[](char* b, const pobject* obj, const pmember* m, const pvalue* pv) {
						return b;
					});

				if (b) {
					database::add_midi_field_request dummy;

					add_object_member_binding(b, "name", "name", &dummy, &dummy.name.name, true);
					add_object_member_binding(b, "image", "image", &dummy, &dummy.name.type, true);
					add_object_member_binding(b, "description", "description", &dummy, &dummy.name.description, false);
					add_object_member_binding(b, "filename", "filename", &dummy, &dummy.options.image_path, false);
					return true;
				}
				else
				{
					add_error(errors::could_not_create_binding, nullptr);
				}
				return false;
			}

			bool add_bindings()
			{
				bool r = true;
				r = bind_add_string_field_request();
				if (!r) return false;
				r = bind_add_double_fields();
				if (!r) return false;
				r = bind_add_integer_fields();
				if (!r) return false;
				r = bind_add_time_field_request();
				if (!r) return false;
				r = bind_add_object_field_request();
				if (!r) return false;
				r = bind_add_query_field_request();
				if (!r) return false;
				r = bind_add_point_field_request();
				if (!r) return false;
				r = bind_add_rectangle_field_request();
				if (!r) return false;
				r = bind_add_image_field_request();
				if (!r) return false;
				r = bind_add_wave_field_request();
				if (!r) return false;
				r = bind_add_midi_field_request();
				if (!r) return false;
				return r;
			}

			char *load(const pobject* obj)
			{
				identify_result ir = identify(obj);

				typeinfo* binding = ir.binding;
				if (!binding)
				{
					add_error(errors::could_not_find_binding, obj);
					return false;
				}

				char* new_base = binding->create_object(obj);
				if (new_base) {
					char* child_base = nullptr;

					for (const pmember* im = obj->first; im; im = im->next)
					{
						if (im != ir.binding_member) {
							switch (im->value->pvalue_type)
							{
							case pvalue::pvalue_types::array_value:
								{
									binding->create_array_member(new_base, obj, im);
									for (auto imo = im->value->array_value->first; imo; imo = imo->next)
									{
										binding->add_array_item(new_base, obj, im, imo);
									}
								}
								break;
							case pvalue::pvalue_types::object_value:
								child_base = load(im->value->object_value);
								if (!child_base) {
									return nullptr;
								}
								break;
							default:
								auto on = im->get_type_code();
								auto member = binding->member_index[on];
								for (auto mi : member) {
									mi.second->set(new_base, im->value);
								}
							}
						}
					}
					binding->object_created(new_base, obj);

				}
			}

		};

		class data_loader
		{

		};
	}
}

