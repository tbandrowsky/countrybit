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

		class propertyinfo
		{
		public:

			property_source source;
			property_dest* first_setter,
				* last_setter;

			propertyinfo() = default;

			propertyinfo(const char* _source_name) :
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

		using property_index_type = database::sorted_index<int, propertyinfo*, 1>;

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

			propertyinfo* add_property(propertyinfo* pb)
			{
				int idx = pb->get_type_code();
				property_index.insert_or_assign(idx, pb);
				return pb;
			}

			propertyinfo* find_property(pmember* pm)
			{
				int idx = pm->get_type_code();
				auto piter = property_index[idx];
				propertyinfo* found = piter != std::end(property_index) ? piter->second : nullptr;
				return found;
			}

			virtual bool set_value(char* _base, const pobject* _src)
			{
				char* loc = _base;

				for (auto member = _src->first; member; member = member->next)
				{
					auto prop = find_property(member);
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
				MemberType item(loc);
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

		template <typename ArrayMemberType, int ArraySize>
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

			template <typename MemberType>
			scalar_property_dest<MemberType>* create_scalar_dest(const char* _dest_name, int _offset, pvalue::pvalue_types _match_type)
			{
				using scalar_dest = scalar_property_dest<MemberType>;
				char* t = data.place<scalar_dest>();
				scalar_dest *new_dest = new (t) scalar_dest(_dest_name, _offset, _match_type);
				return new_dest;
			}

			object_property_dest* create_object_dest(const char* _dest_name, int _offset, typeinfo* _property_type)
			{
				char* t = data.place<object_property_dest>();
				object_property_dest *new_dest = new (t) object_property_dest(_dest_name, _offset, _property_type);
				return new_dest;
			}

			template <typename ArrayMemberType, int ArraySize>
			object_iarray_property_dest<ArrayMemberType, ArraySize>* create_object_iarray_dest(const char* _dest_name, int _offset, typeinfo* _property_type)
			{
				using array_dest = object_iarray_property_dest<ArrayMemberType, ArraySize>;
				char* t = data.place<array_dest>();
				array_dest* new_dest = new (t) array_dest(_dest_name, _offset, _property_type);
				return new_dest;
			}

			template <typename MemberType>
			propertyinfo* create_scalar_property(pvalue::pvalue_types _match_type, const char *_source_name, const char* _dest_name, int _offset)
			{
				using scalar_dest = scalar_property_dest<MemberType>;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				t = data.place<scalar_dest>();
				scalar_dest* new_dest = new (t) scalar_dest(_dest_name, _offset, _match_type);

				pi->add_setter(new_dest);
				return pi;
			}

			template <typename MemberType>
			propertyinfo* create_scalar_property(typeinfo *parent, pvalue::pvalue_types _match_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* sp = create_scalar_property(_match_type, _source_name, _dest_name, _offset);
				parent->add_property(sp);
			}

			propertyinfo* create_object_property(typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				char *t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				t = data.place<object_property_dest>();
				object_property_dest* new_dest = new (t) object_property_dest(_dest_name, _offset, _property_type);

				pi->add_setter(new_dest);

				return pi;
			}

			propertyinfo* create_object_property(typeinfo *parent, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = create_object_property(_property_type, _source_name, _dest_name, _offset);
				parent->add_property(op);
				return op;
			}

			template <typename ArrayMemberType, int ArraySize>
			propertyinfo* create_object_iarray_property(typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				using array_dest = object_iarray_property_dest<ArrayMemberType, ArraySize>;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				char* t = data.place<array_dest>();
				array_dest* new_dest = new (t) array_dest(_dest_name, _offset, _property_type);

				pi->add_setter(new_dest);

				return pi;
			}

			template <typename ArrayMemberType, int ArraySize>
			propertyinfo* create_object_iarray_property(typeinfo* parent, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = create_object_iarray_property<ArrayMemberType,ArraySize>(_property_type, _source_name, _dest_name, _offset);
				parent->add_property(op);
				return op;
			}

			propertyinfo* create_polymorphic_property(const char* _source_name, int _num_dests, property_dest **_dests)
			{
				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				for (int i = 0; i < _num_dests; i++) {
					pi->add_setter(_dests[i]);
				}

				return pi;
			}

			propertyinfo* create_polymorphic_property(typeinfo* parent, const char* _source_name, int _num_dests, property_dest** _dests)
			{
				propertyinfo* op = create_polymorphic_property(_source_name, _num_dests, _dests);
				parent->add_property(op);
				return op;
			}

			virtual char* place(typeinfo* ti) = 0;

			void load(const pobject* _src)
			{
				typeinfo*ti = find_typeinfo(_src);
				if (ti) 
				{
					char* base = place(ti);
					ti->set_value(base, _src);
				}
			}
		};

		class schema_loader : loader
		{
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

			database::table<database::add_string_field_request> add_string_fields;
			database::table<database::add_integer_field_request> add_integer_fields;
			database::table<database::add_double_field_request> add_double_fields;
			database::table<database::add_time_field_request> add_time_fields;
			database::table<database::add_object_field_request> add_object_fields;
			database::table<database::add_point_field_request> add_point_fields;
			database::table<database::add_rectangle_field_request> add_rectangle_fields;
			database::table<database::add_color_field_request> add_color_fields;
			database::table<database::add_image_field_request> add_image_fields;
			database::table<database::add_wave_field_request> add_wave_fields;
			database::table<database::add_midi_field_request> add_midi_fields;
			database::table<database::add_color_field_request> add_color_fields;
			database::table<database::add_named_query_field_request> add_query_fields;

			database::table<error_message> error_messages;

			database::row_id_type error_messages_id;
			database::row_id_type add_string_fields_id;
			database::row_id_type add_integer_fields_id;
			database::row_id_type add_double_fields_id;
			database::row_id_type add_time_fields_id;
			database::row_id_type add_object_fields_id;
			database::row_id_type add_query_fields_id;
			database::row_id_type add_point_fields_id;
			database::row_id_type add_rectangle_fields_id;
			database::row_id_type add_color_fields_id;
			database::row_id_type add_image_fields_id;
			database::row_id_type add_wave_fields_id;
			database::row_id_type add_midi_fields_id;

			typeinfo* error_messages_ti;
			typeinfo* string_fields_ti;
			typeinfo* int8_fields_ti;
			typeinfo* int16_fields_ti;
			typeinfo* int32_fields_ti;
			typeinfo* int64_fields_ti;
			typeinfo* float_fields_ti;
			typeinfo* double_fields_ti;
			typeinfo* time_fields_ti;
			typeinfo* object_fields_ti;
			typeinfo* query_fields_ti;
			typeinfo* query_projection_ti;
			typeinfo* query_filter_ti;
			typeinfo* point_fields_ti;
			typeinfo* rectangle_fields_ti;
			typeinfo* color_fields_ti;
			typeinfo* image_fields_ti;
			typeinfo* wave_fields_ti;
			typeinfo* midi_fields_ti;

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

				string_fields_ti = create_typeinfo(member_type_name, "string", "string", 20);
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_string_field_request, name.name));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_string_field_request, name.description));
				create_scalar_property<database::int32_box>(string_fields_ti, pvalue::pvalue_types::double_value, "length", "length", offsetof(database::add_string_field_request, options.length));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_pattern", "validation_pattern", offsetof(database::add_string_field_request, options.validation_pattern));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_message", "validation_message", offsetof(database::add_string_field_request, options.validation_message));

				int8_fields_ti = create_typeinfo(member_type_name, "int8", "int8", 20);
				create_scalar_property<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_integer_field_request, options.maximum_int));

				int16_fields_ti = create_typeinfo(member_type_name, "int16", "int16", 20);
				create_scalar_property<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_integer_field_request, options.maximum_int));

				int32_fields_ti = create_typeinfo(member_type_name, "int32", "int32", 20);
				create_scalar_property<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_integer_field_request, options.maximum_int));

				int64_fields_ti = create_typeinfo(member_type_name, "int64", "int64", 20);
				create_scalar_property<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_integer_field_request, options.maximum_int));

				float_fields_ti = create_typeinfo(member_type_name, "float", "float", 20);
				create_scalar_property<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_double_field_request, name.name));
				create_scalar_property<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_double_field_request, name.description));
				create_scalar_property<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_double_field_request, options.minimum_double));
				create_scalar_property<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_double_field_request, options.maximum_double));

				double_fields_ti = create_typeinfo(member_type_name, "double", "double", 20);
				create_scalar_property<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_double_field_request, name.name));
				create_scalar_property<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_double_field_request, name.description));
				create_scalar_property<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_double_field_request, options.minimum_double));
				create_scalar_property<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_double_field_request, options.maximum_double));

				time_fields_ti = create_typeinfo(member_type_name, "time", "time", 20);
				create_scalar_property<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_time_field_request, name.name));
				create_scalar_property<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_time_field_request, name.description));
				create_scalar_property<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::add_time_field_request, options.minimum_time_t));
				create_scalar_property<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::add_time_field_request, options.maximum_time_t));

				object_fields_ti = create_typeinfo(member_type_name, "object", "object", 20);
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_object_field_request, name.name));
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_object_field_request, name.description));
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "class", "class", offsetof(database::add_object_field_request, options.class_name));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::add_object_field_request, options.dim.x));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::add_object_field_request, options.dim.y));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::add_object_field_request, options.dim.z));

				query_fields_ti = create_typeinfo(member_type_name, "query", "query", 20);
				create_scalar_property<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_named_query_field_request, name.name));
				create_scalar_property<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_named_query_field_request, name.description));

				query_projection_ti = create_typeinfo(member_type_name, "projection", "projection", 20);
				create_scalar_property<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "field", "field", offsetof(database::projection_element_request, field_name));

				query_filter_ti = create_typeinfo(member_type_name, "filter", "filter", 20);
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "target_field", "target_field", offsetof(database::filter_element_request, target_field_name));
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "comparison", "comparison", offsetof(database::filter_element_request, comparison_name));
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "parameter_field", "parameter_field", offsetof(database::filter_element_request, parameter_field_name));
				create_scalar_property<database::double_box>(query_filter_ti, pvalue::pvalue_types::double_value, "distance", "distance", offsetof(database::filter_element_request, distance_threshold));

				create_object_iarray_property<database::filter_element_request, database::max_query_filters>(query_fields_ti, query_filter_ti, "filters", "filters", offsetof(database::add_named_query_field_request, options.filter));
				create_object_iarray_property<database::projection_element_request, database::max_query_projections > (query_fields_ti, query_projection_ti, "projections", "projections", offsetof(database::add_named_query_field_request, options.projection));

				point_fields_ti = create_typeinfo(member_type_name, "point", "point", 20);
				create_scalar_property<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_point_field_request, name.name));
				create_scalar_property<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_point_field_request, name.description));

				rectangle_fields_ti = create_typeinfo(member_type_name, "rectangle", "rectangle", 20);
				create_scalar_property<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_rectangle_field_request, name.name));
				create_scalar_property<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_rectangle_field_request, name.description));

				rectangle_fields_ti = create_typeinfo(member_type_name, "color", "color", 20);
				create_scalar_property<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_color_field_request, name.name));
				create_scalar_property<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_color_field_request, name.description));

				image_fields_ti = create_typeinfo(member_type_name, "image", "image", 20);
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_image_field_request, name.name));
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_image_field_request, name.description));
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::add_image_field_request, options.image_path));

				wave_fields_ti = create_typeinfo(member_type_name, "wave", "wave", 20);
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_wave_field_request, name.name));
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_wave_field_request, name.description));
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::add_wave_field_request, options.image_path));

				midi_fields_ti = create_typeinfo(member_type_name, "midi", "midi", 20);
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::add_midi_field_request, name.name));
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::add_midi_field_request, name.description));
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::add_midi_field_request, options.image_path));
			}

			void add_error(errors _error, const pobject* _obj)
			{
				database::row_range rr;
				error_message em;
				em.error = _error;
				em.obj = _obj;
				error_messages.insert(em, rr);
			}

			virtual char* place(typeinfo* ti)
			{
				char* t = nullptr;
				database::row_range rr;

				if (ti == string_fields_ti)
				{
					auto& asf = add_string_fields.create(1, rr);
					asf.name.type_id = database::jtype::type_string;
					t = (char*) &asf;
				}
				else if (ti == int8_fields_ti)
				{
					auto& ai8f = add_integer_fields.create(1, rr);
					ai8f.name.type_id = database::jtype::type_int8;
					t = (char*)&ai8f;
				}
				else if (ti == int16_fields_ti)
				{
					auto& ai16f = add_integer_fields.create(1, rr);
					ai16f.name.type_id = database::jtype::type_int16;
					t = (char*)&ai16f;
				}
				else if (ti == int32_fields_ti)
				{
					auto& ai32f = add_integer_fields.create(1, rr);
					ai32f.name.type_id = database::jtype::type_int32;
					t = (char*)&ai32f;
				}
				else if (ti == int64_fields_ti)
				{
					auto& ai64f = add_integer_fields.create(1, rr);
					ai64f.name.type_id = database::jtype::type_int64;
					t = (char*)&ai64f;
				}
				else if (ti == float_fields_ti)
				{
					auto& af32f = add_double_fields.create(1, rr);
					af32f.name.type_id = database::jtype::type_float32;
					t = (char*)&af32f;
				}
				else if (ti == double_fields_ti)
				{
					auto& af64f = add_double_fields.create(1, rr);
					af64f.name.type_id = database::jtype::type_float64;
					t = (char*)&af64f;
				}
				else if (ti == time_fields_ti)
				{
					auto& at64f = add_time_fields.create(1, rr);
					at64f.name.type_id = database::jtype::type_datetime;
					t = (char*)&at64f;
				}
				else if (ti == object_fields_ti)
				{
					auto& aof = add_object_fields.create(1, rr);
					aof.name.type_id = database::jtype::type_object;
					t = (char*)&aof;
				}
				else if (ti == query_fields_ti)
				{
					auto& aqf = add_query_fields.create(1, rr);
					aqf.name.type_id = database::jtype::type_query;
					t = (char*)&aqf;
				}
				else if (ti == query_projection_ti)
				{
					t = (char*)nullptr;
				}
				else if (ti == query_filter_ti)
				{
					t = (char*)nullptr;
				}
				else if (ti == point_fields_ti)
				{
					auto& apf = add_point_fields.create(1, rr);
					apf.name.type_id = database::jtype::type_point;
					t = (char*)&apf;
				}
				else if (ti == rectangle_fields_ti)
				{
					auto& arf = add_rectangle_fields.create(1, rr);
					arf.name.type_id = database::jtype::type_rectangle;
					t = (char*)&arf;
				}
				else if (ti == color_fields_ti)
				{
					auto& acf = add_color_fields.create(1, rr);
					acf.name.type_id = database::jtype::type_color;
					t = (char*)&acf;
				}
				else if (ti == image_fields_ti)
				{
					auto& aimf = add_image_fields.create(1, rr);
					aimf.name.type_id = database::jtype::type_image;
					t = (char*)&aimf;
				}
				else if (ti == wave_fields_ti)
				{
					auto& awf = add_wave_fields.create(1, rr);
					awf.name.type_id = database::jtype::type_wave;
					t = (char*)&awf;
				}
				else if (ti == midi_fields_ti)
				{
					auto& amf = add_midi_fields.create(1, rr);
					amf.name.type_id = database::jtype::type_midi;
					t = (char*)&amf;
				}

				return t;
			}
		};

		class data_loader
		{

		};
	}
}

