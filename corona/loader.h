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
			typeinfo* point_fields_ti;
			typeinfo* rectangle_fields_ti;
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



				int8_fields_ti = create_typeinfo(member_type_name, "int8", "int8", 20);
				

				int16_fields_ti = create_typeinfo(member_type_name, "int16", "int16", 20);

				int32_fields_ti = create_typeinfo(member_type_name, "int32", "int32", 20);

				int64_fields_ti = create_typeinfo(member_type_name, "int64", "int64", 20);

				float_fields_ti = create_typeinfo(member_type_name, "float", "float", 20);



				double_fields_ti = create_typeinfo(member_type_name, "double", "double", 20);

				time_fields_ti = create_typeinfo(member_type_name, "time", "time", 20);
				object_fields_ti = create_typeinfo(member_type_name, "object", "object", 20);
				query_fields_ti = create_typeinfo(member_type_name, "query", "query", 20);
				point_fields_ti = create_typeinfo(member_type_name, "point", "point", 20);
				rectangle_fields_ti = create_typeinfo(member_type_name, "rectangle", "rectangle", 20);
				image_fields_ti = create_typeinfo(member_type_name, "image", "image", 20);
				wave_fields_ti = create_typeinfo(member_type_name, "wave", "wave", 20);
				midi_fields_ti = create_typeinfo(member_type_name, "midi", "midi", 20);
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


		};

		class data_loader
		{

		};
	}
}

