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
#include <jobject.h>

namespace countrybit
{
	namespace system
	{
		template <typename RemoteField>
		concept remote_field = requires(RemoteField rf, 
			database::object_name					n,
			database::remote_parameter_fields_type	p,
			database::remote_fields_type			r,
			database::remote_field_map_type			t,
			int										s
		)
		{
			n = rf.parameter_field;
			n = rf.result_field;
			p = rf.parameters;
			r = rf.fields;
			t = rf.parameters[0];
			t = rf.fields[0];
			s = rf.fields.size();
			s = rf.parameters.size();
		};

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
			virtual bool set_value(char* _base, const pvalue* _src) { return false; }
			virtual bool set_value(database::jslice& _base, const pvalue* _src) { return false; }
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

			void put_setter(property_dest* _new_setter)
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
			database::relative_ptr_type			property_index_id;

			const char* type_key;
			const char* type_value;
			const char* member_name;
			const char* class_name;

		public:

			database::object_name			key;

			typeinfo(database::serialized_box_container* _b,
				const char* _type_key,
				const char* _type_value,
				const char* _class_name,
				int _max_items)
			{
				type_key = _type_key;
				type_value = _type_value;
				class_name = _class_name;
				property_index = property_index_type::create_sorted_index(_b, property_index_id);

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

			propertyinfo* put_property(propertyinfo* pb)
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

			virtual bool set_value(database::jslice slice, const pobject* _src)
			{
				for (auto member = _src->first; member; member = member->next)
				{
					auto prop = find_property(member);
					if (prop) {
						auto setter = prop->get_setter(member->value);
						if (setter) {
							setter->set_value(slice, member->value);
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
				property_dest(_dest_name),
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
				if (_src->as_object() || _src->as_array()) {
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

		class corona_property_dest : public property_dest
		{
		protected:

			int						field_idx;
			database::jschema*		schema;

			pvalue::pvalue_types match_type;

		public:

			corona_property_dest(database::jschema* _schema, int _member_idx, const char* _member_name, pvalue::pvalue_types _match_type) :
				property_dest(_member_name),
				schema(_schema),
				match_type(_match_type),
				field_idx(_member_idx)
			{

			}

			virtual bool is_match(const pvalue* _src)
			{
				return	_src->pvalue_type == match_type;
			}

		};

		template <typename MemberType>
		class corona_scalar_property_dest : public corona_property_dest
		{
		public:

			corona_scalar_property_dest(database::jschema* _schema, int _member_idx, const char* _member_name, pvalue::pvalue_types _match_type) :
				corona_property_dest(_schema, _member_idx, _member_name, _match_type)
			{

			}

			virtual bool set_value(database::jslice slice, const pvalue* _src)
			{
				if (_src->as_object() || _src->as_array()) {
					throw std::logic_error("attempt to map non-scalar to scalar");
				}
				if (field_idx >= 0) {
					MemberType t = slice.get<MemberType>(field_idx);
					_src->set_value(t);
					return true;
				}
				return false;
			}
		};

		class corona_list_property_dest : public corona_property_dest
		{
			typeinfo* item_property_type;

		public:

			corona_list_property_dest(database::jschema* _schema, int _member_idx, const char* _member_name, typeinfo* _item_property_type, pvalue::pvalue_types _match_type) :
				corona_property_dest(_schema, _member_idx, _member_name, _match_type),
				item_property_type(_item_property_type)
			{

			}

			virtual bool set_value(database::jslice slice, const pvalue* _src)
			{
				if (!_src->as_array()) {
					throw std::logic_error("attempt to map non-scalar to scalar");
				}

				database::jlist list = slice.get_list(field_idx);
				database::jslice new_slice = list.append_slice();

				const pobject* pv = _src->as_object();

				if (pv == nullptr)
				{
					throw std::logic_error("attempt to map non-object to object");
				}

				for (auto member = pv->first; member; member = member->next)
				{
					auto prop = item_property_type->find_property(member);
					if (prop) {
						auto setter = prop->get_setter(member->value);
						if (setter) {
							setter->set_value(new_slice, member->value);
						}
					}
				}

				return true;
			}
		};

		class corona_array_property_dest : public corona_property_dest
		{
			typeinfo* item_property_type;

		public:

			corona_array_property_dest(database::jschema* _schema, int _member_idx, const char* _member_name, typeinfo* _item_property_type, pvalue::pvalue_types _match_type) :
				corona_property_dest(_schema, _member_idx, _member_name, _match_type),
				item_property_type(_item_property_type)
			{

			}

			virtual bool set_value(database::jslice slice, const pvalue* _src)
			{
				if (!_src->as_array()) {
					throw std::logic_error("attempt to map non-scalar to scalar");
				}

				database::jarray array = slice.get_object(field_idx);

				const pobject* pv = _src->as_object();

				if (pv == nullptr)
				{
					throw std::logic_error("attempt to map non-object to object");
				}

				auto update_slice = array.get_slice(_src->x, _src->y, _src->z);

				for (auto member = pv->first; member; member = member->next)
				{
					auto prop = item_property_type->find_property(member);
					if (prop) {
						auto setter = prop->get_setter(member->value);
						if (setter) {
							setter->set_value(update_slice, member->value);
						}
					}
				}

				return true;
			}
		};

		class loader 
		{
		public:

			enum class errors 
			{
				could_not_create_binding = 1,
				could_not_find_binding = 2,
				invalid_binding_name_type = 3,
				class_not_defined = 4,
				field_not_defined = 5,
				invalid_comparison = 6,
				invalid_mapping = 7,
				invalid_sql_use = 8,
				invalid_http_use = 9,
				invalid_file_use = 10,
				invalid_object_parse = 11
			};

			class error_message
			{
			public:
				errors error;
				const pobject* obj;
				const char* name;
				int line_number;
			};

		protected:

			database::dynamic_box data;
			database::sorted_index<database::object_name, typeinfo*> bindings_by_name;
			database::table<error_message> error_messages;
			database::relative_ptr_type error_messages_id;

		public:

			loader(int _size, int _max_types) 
			{
				database::relative_ptr_type header_location;
				data.init(_size);
				bindings_by_name.create_sorted_index(&data, header_location);
				error_messages = database::table<error_message>::create_table(&data, 250, error_messages_id);
			}

		protected:

			void put_error(errors _error, const pobject* _obj)
			{
				database::row_range rr;
				error_message em;
				em.error = _error;
				em.obj = _obj;
				em.line_number = _obj->line;
				error_messages.append(em, rr);
			}

			void put_error(errors _error, const char *_name, int _line = 0)
			{
				database::row_range rr;
				error_message em;
				em.error = _error;
				em.obj = nullptr;
				em.name = _name;
				em.line_number = _line;
				error_messages.append(em, rr);
			}

			typeinfo* create_map(
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

			typeinfo* find_typeinfo(const char *type_name)
			{
				typeinfo* ti = nullptr;

				database::object_name key = "corona";
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
			propertyinfo* map_scalar(pvalue::pvalue_types _match_type, const char *_source_name, const char* _dest_name, int _offset)
			{
				using scalar_dest = scalar_property_dest<MemberType>;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				t = data.place<scalar_dest>();
				scalar_dest* new_dest = new (t) scalar_dest(_dest_name, _offset, _match_type);

				pi->put_setter(new_dest);
				return pi;
			}

			template <typename MemberType>
			propertyinfo* map_scalar(typeinfo *item, pvalue::pvalue_types _match_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* sp = map_scalar<MemberType>(_match_type, _source_name, _dest_name, _offset);
				item->put_property(sp);
			}

			propertyinfo* map_object(typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				char *t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				t = data.place<object_property_dest>();
				object_property_dest* new_dest = new (t) object_property_dest(_dest_name, _offset, _property_type);

				pi->put_setter(new_dest);

				return pi;
			}

			propertyinfo* map_object(typeinfo *item, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = map_object(_property_type, _source_name, _dest_name, _offset);
				item->put_property(op);
				return op;
			}

			template <typename ArrayMemberType, int ArraySize>
			propertyinfo* map_iarray(typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				using array_dest = object_iarray_property_dest<ArrayMemberType, ArraySize>;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				char* t2 = data.place<array_dest>();
				array_dest* new_dest = new (t2) array_dest(_dest_name, _offset, _property_type);

				pi->put_setter(new_dest);

				return pi;
			}

			template <typename ArrayMemberType, int ArraySize>
			propertyinfo* map_iarray(typeinfo* item, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = map_iarray<ArrayMemberType,ArraySize>(_property_type, _source_name, _dest_name, _offset);
				item->put_property(op);
				return op;
			}

			propertyinfo* map_polymorphic(const char* _source_name, int _num_dests, property_dest **_dests)
			{
				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				for (int i = 0; i < _num_dests; i++) {
					pi->put_setter(_dests[i]);
				}

				return pi;
			}

			propertyinfo* map_polymorphic(typeinfo* item, const char* _source_name, int _num_dests, property_dest** _dests)
			{
				propertyinfo* op = map_polymorphic(_source_name, _num_dests, _dests);
				item->put_property(op);
				return op;
			}

			template <typename MemberType>
			propertyinfo* map_corona_scalar(database::jschema* _schema, int _member_idx, const char* _member_name, pvalue::pvalue_types _match_type)
			{
				using scalar_dest = corona_scalar_property_dest<MemberType>;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_member_name);

				t = data.place<scalar_dest>();
				scalar_dest* new_dest = new (t) scalar_dest(_schema, _member_idx, _member_name, _match_type);

				pi->put_setter(new_dest);
				return pi;
			}

			template <typename MemberType>
			propertyinfo* map_corona_scalar(typeinfo *_item, database::jschema* _schema, int _member_idx, const char* _member_name, pvalue::pvalue_types _match_type)
			{
				propertyinfo* sp = map_corona_scalar<MemberType>(_schema, _member_idx, _member_name, _match_type);
				_item->put_property(sp);
			}

			propertyinfo* map_corona_list(database::jschema* _schema, int _member_idx, const char* _member_name, typeinfo* _member_type_info )
			{
				using array_dest = corona_list_property_dest;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_member_name);

				char* t2 = data.place<array_dest>();
				array_dest* new_dest = new (t2) array_dest(  _schema, _member_idx, _member_name, _member_type_info, pvalue::pvalue_types::array_value );

				pi->put_setter(new_dest);

				return pi;
			}

			propertyinfo* map_corona_list(typeinfo* _item, database::jschema *_schema, int _member_idx, const char* _member_name, typeinfo* _member_type_info )
			{
				propertyinfo* op = map_corona_list(_schema, _member_idx, _member_name, _member_type_info);
				_item->put_property(op);
				return op;
			}

			propertyinfo* map_corona_array(database::jschema* _schema, int _member_idx, const char* _member_name, typeinfo* _member_type_info)
			{
				using array_dest = corona_array_property_dest;

				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_member_name);

				char* t2 = data.place<array_dest>();
				array_dest* new_dest = new (t2) array_dest(_schema, _member_idx, _member_name, _member_type_info, pvalue::pvalue_types::array_value);

				pi->put_setter(new_dest);

				return pi;
			}

			propertyinfo* map_corona_array(typeinfo* _item, database::jschema* _schema, int _member_idx, const char* _member_name, typeinfo* _member_type_info)
			{
				propertyinfo* op = map_corona_array(_schema, _member_idx, _member_name, _member_type_info);
				_item->put_property(op);
				return op;
			}


			virtual char* place(typeinfo* ti) = 0;

			bool load(const pobject* _src)
			{
				typeinfo*ti = find_typeinfo(_src);
				if (ti) 
				{
					char* base = place(ti);
					try 
					{
						ti->set_value(base, _src);
					}
					catch (std::logic_error err)
					{
						put_error(errors::invalid_object_parse, err.what());
					}
				}
				else 
				{
					put_error(errors::class_not_defined, _src);
				}
				return this->error_messages.size() == 0;
			}
		};

		class schema_loader : loader
		{
		public:

			struct location 
			{
				database::jtype item_type;
				database::relative_ptr_type row_id;					
			};

		private:

			database::sorted_index<database::object_name, location> fields_by_name;
			database::sorted_index<database::object_name, database::relative_ptr_type> classes_by_name;
			database::sorted_index<database::object_name, typeinfo*> class_types_by_name;

			database::table<database::put_string_field_request> put_string_fields;
			database::table<database::put_integer_field_request> put_integer_fields;
			database::table<database::put_double_field_request> put_double_fields;
			database::table<database::put_time_field_request> put_time_fields;
			database::table<database::put_object_field_request> put_object_fields;
			database::table<database::put_object_field_request> put_list_fields;
			database::table<database::put_point_field_request> put_point_fields;
			database::table<database::put_rectangle_field_request> put_rectangle_fields;
			database::table<database::put_color_field_request> put_color_fields;
			database::table<database::put_image_field_request> put_image_fields;
			database::table<database::put_wave_field_request> put_wave_fields;
			database::table<database::put_midi_field_request> put_midi_fields;
			database::table<database::put_named_query_field_request> put_query_fields;
			database::table<database::put_named_sql_remote_field_request> put_sql_fields;
			database::table<database::put_named_http_remote_field_request> put_http_fields;
			database::table<database::put_named_file_remote_field_request> put_file_fields;
			database::table<database::put_class_request> put_classes;

			database::relative_ptr_type fields_by_name_id;
			database::relative_ptr_type classes_by_name_id;
			database::relative_ptr_type class_types_by_name_id;
			database::relative_ptr_type put_string_fields_id;
			database::relative_ptr_type put_integer_fields_id;
			database::relative_ptr_type put_double_fields_id;
			database::relative_ptr_type put_time_fields_id;
			database::relative_ptr_type put_object_fields_id;
			database::relative_ptr_type put_list_fields_id;
			database::relative_ptr_type put_query_fields_id;
			database::relative_ptr_type put_sql_fields_id;
			database::relative_ptr_type put_http_fields_id;
			database::relative_ptr_type put_file_fields_id;
			database::relative_ptr_type put_point_fields_id;
			database::relative_ptr_type put_rectangle_fields_id;
			database::relative_ptr_type put_color_fields_id;
			database::relative_ptr_type put_image_fields_id;
			database::relative_ptr_type put_wave_fields_id;
			database::relative_ptr_type put_midi_fields_id;
			database::relative_ptr_type put_classes_id;

			typeinfo* string_fields_ti;
			typeinfo* int8_fields_ti;
			typeinfo* int16_fields_ti;
			typeinfo* int32_fields_ti;
			typeinfo* int64_fields_ti;
			typeinfo* float_fields_ti;
			typeinfo* double_fields_ti;
			typeinfo* time_fields_ti;
			typeinfo* object_fields_ti;
			typeinfo* list_fields_ti;
			typeinfo* point_fields_ti;
			typeinfo* rectangle_fields_ti;
			typeinfo* color_fields_ti;
			typeinfo* image_fields_ti;
			typeinfo* wave_fields_ti;
			typeinfo* midi_fields_ti;
			typeinfo* put_classes_ti;
			typeinfo* put_class_fields_ti;

			typeinfo* path_node_ti;
			typeinfo* path_ti;

			typeinfo* projection_field_ti;
			typeinfo* projection_ti;

			typeinfo* query_filter_ti;
			typeinfo* query_fields_ti;

			typeinfo* remote_parameters_ti;

			typeinfo* sql_fields_ti;
			typeinfo* file_fields_ti;
			typeinfo* http_fields_ti;

			typeinfo* color_ti;
			typeinfo* point_ti;
			typeinfo* rectangle_ti;
			typeinfo* wave_ti;
			typeinfo* midi_ti;
			typeinfo* image_ti;

			const char* member_type_name = "type";

		public:

			schema_loader(int _size, int _num_fields) : loader(_size, _num_fields)
			{

				fields_by_name = database::sorted_index<database::object_name, location>::create_sorted_index(&data, fields_by_name_id);
				classes_by_name = database::sorted_index<database::object_name, database::relative_ptr_type>::create_sorted_index(&data, classes_by_name_id);
				class_types_by_name = database::sorted_index<database::object_name, typeinfo *>::create_sorted_index(&data, class_types_by_name_id);
				put_string_fields = database::table<database::put_string_field_request>::create_table(&data, _num_fields, put_string_fields_id);
				put_integer_fields = database::table<database::put_integer_field_request>::create_table(&data, _num_fields, put_integer_fields_id);
				put_double_fields = database::table<database::put_double_field_request>::create_table(&data, _num_fields, put_double_fields_id);
				put_time_fields = database::table<database::put_time_field_request>::create_table(&data, _num_fields, put_time_fields_id);
				put_object_fields = database::table<database::put_object_field_request>::create_table(&data, _num_fields, put_object_fields_id);
				put_list_fields = database::table<database::put_object_field_request>::create_table(&data, _num_fields, put_list_fields_id);
				put_query_fields = database::table<database::put_named_query_field_request>::create_table(&data, _num_fields, put_query_fields_id);
				put_point_fields = database::table<database::put_point_field_request>::create_table(&data, _num_fields, put_point_fields_id);
				put_rectangle_fields = database::table<database::put_rectangle_field_request>::create_table(&data, _num_fields, put_rectangle_fields_id);
				put_image_fields = database::table<database::put_image_field_request>::create_table(&data, _num_fields, put_image_fields_id);
				put_wave_fields = database::table<database::put_wave_field_request>::create_table(&data, _num_fields, put_wave_fields_id);
				put_midi_fields = database::table<database::put_midi_field_request>::create_table(&data, _num_fields, put_midi_fields_id);
				put_classes = database::table<database::put_class_request>::create_table(&data, _num_fields, put_classes_id);
				put_sql_fields = database::table<database::put_named_sql_remote_field_request>::create_table(&data, _num_fields, put_sql_fields_id);
				put_http_fields = database::table<database::put_named_http_remote_field_request>::create_table(&data, _num_fields, put_http_fields_id);
				put_file_fields = database::table<database::put_named_file_remote_field_request>::create_table(&data, _num_fields, put_file_fields_id);

				string_fields_ti = create_map(member_type_name, "stringfield", "stringfield", 20);
				map_scalar<database::int32_box>(string_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_string_field_request, name.field_id));
				map_scalar<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_string_field_request, name.name));
				map_scalar<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_string_field_request, name.description));
				map_scalar<database::int32_box>(string_fields_ti, pvalue::pvalue_types::double_value, "length", "length", offsetof(database::put_string_field_request, options.length));
				map_scalar<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_pattern", "validation_pattern", offsetof(database::put_string_field_request, options.validation_pattern));
				map_scalar<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_message", "validation_message", offsetof(database::put_string_field_request, options.validation_message));

				int8_fields_ti = create_map(member_type_name, "int8field", "int8field", 20);
				map_scalar<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				map_scalar<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				map_scalar<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				map_scalar<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				map_scalar<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int16_fields_ti = create_map(member_type_name, "int16field", "int16field", 20);
				map_scalar<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				map_scalar<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				map_scalar<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				map_scalar<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				map_scalar<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int32_fields_ti = create_map(member_type_name, "int32field", "int32field", 20);
				map_scalar<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				map_scalar<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				map_scalar<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				map_scalar<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				map_scalar<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int64_fields_ti = create_map(member_type_name, "int64field", "int64field", 20);
				map_scalar<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				map_scalar<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				map_scalar<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				map_scalar<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				map_scalar<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				float_fields_ti = create_map(member_type_name, "floatfield", "floatfield", 20);
				map_scalar<database::int32_box>(float_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_double_field_request, name.field_id));
				map_scalar<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_double_field_request, name.name));
				map_scalar<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_double_field_request, name.description));
				map_scalar<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_double_field_request, options.minimum_double));
				map_scalar<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_double_field_request, options.maximum_double));

				double_fields_ti = create_map(member_type_name, "doublefield", "doublefield", 20);
				map_scalar<database::int32_box>(double_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_double_field_request, name.field_id));
				map_scalar<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_double_field_request, name.name));
				map_scalar<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_double_field_request, name.description));
				map_scalar<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_double_field_request, options.minimum_double));
				map_scalar<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_double_field_request, options.maximum_double));

				time_fields_ti = create_map(member_type_name, "timefield", "timefield", 20);
				map_scalar<database::int32_box>(double_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_time_field_request, name.field_id));
				map_scalar<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_time_field_request, name.name));
				map_scalar<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_time_field_request, name.description));
				map_scalar<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_time_field_request, options.minimum_time_t));
				map_scalar<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_time_field_request, options.maximum_time_t));

				object_fields_ti = create_map(member_type_name, "objectfield", "objectfield", 20);
				map_scalar<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_object_field_request, name.field_id));
				map_scalar<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_object_field_request, name.name));
				map_scalar<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_object_field_request, name.description));
				map_scalar<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "class_name", "class_name", offsetof(database::put_object_field_request, options.class_name));
				map_scalar<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::put_object_field_request, options.dim.x));
				map_scalar<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::put_object_field_request, options.dim.y));
				map_scalar<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::put_object_field_request, options.dim.z));

				list_fields_ti = create_map(member_type_name, "listfield", "listfield", 20);
				map_scalar<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_object_field_request, name.field_id));
				map_scalar<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_object_field_request, name.name));
				map_scalar<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_object_field_request, name.description));
				map_scalar<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "class_name", "class_name", offsetof(database::put_object_field_request, options.class_name));
				map_scalar<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::put_object_field_request, options.dim.x));
				map_scalar<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::put_object_field_request, options.dim.y));
				map_scalar<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::put_object_field_request, options.dim.z));

				point_fields_ti = create_map(member_type_name, "pointfield", "pointfield", 20);
				map_scalar<database::int32_box>(point_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_point_field_request, name.field_id));
				map_scalar<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_point_field_request, name.name));
				map_scalar<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_point_field_request, name.description));

				rectangle_fields_ti = create_map(member_type_name, "rectanglefield", "rectanglefield", 20);
				map_scalar<database::int32_box>(rectangle_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_rectangle_field_request, name.field_id));
				map_scalar<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_rectangle_field_request, name.name));
				map_scalar<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_rectangle_field_request, name.description));

				color_fields_ti = create_map(member_type_name, "colorfield", "colorfield", 20);
				map_scalar<database::int32_box>(color_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_color_field_request, name.field_id));
				map_scalar<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_color_field_request, name.name));
				map_scalar<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_color_field_request, name.description));

				image_fields_ti = create_map(member_type_name, "imagefield", "imagefield", 20);
				map_scalar<database::int32_box>(image_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_image_field_request, name.field_id));
				map_scalar<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_image_field_request, name.name));
				map_scalar<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_image_field_request, name.description));
				map_scalar<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_image_field_request, options.image_path));

				wave_fields_ti = create_map(member_type_name, "wavefield", "wavefield", 20);
				map_scalar<database::int32_box>(wave_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_wave_field_request, name.field_id));
				map_scalar<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_wave_field_request, name.name));
				map_scalar<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_wave_field_request, name.description));
				map_scalar<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_wave_field_request, options.image_path));

				midi_fields_ti = create_map(member_type_name, "midifield", "midifield", 20);
				map_scalar<database::int32_box>(midi_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_midi_field_request, name.field_id));
				map_scalar<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_midi_field_request, name.name));
				map_scalar<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_midi_field_request, name.description));
				map_scalar<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_midi_field_request, options.image_path));

				path_node_ti = create_map(member_type_name, "path_node", "path_node", 20);
				map_scalar<database::string_box>(path_node_ti, pvalue::pvalue_types::string_value, "member", "member", offsetof(database::path_node, member_name));

				path_ti = create_map(member_type_name, "path", "path", 20);
				map_scalar<database::string_box>(path_ti, pvalue::pvalue_types::string_value, "class", "class", offsetof(database::path, root.class_name));
				map_iarray<database::path, database::max_path_nodes>(path_ti, path_node_ti, "nodes", "nodes", offsetof(database::path, nodes));

				projection_field_ti = create_map(member_type_name, "projection", "projection", 20);
				map_scalar<database::string_box>(projection_field_ti, pvalue::pvalue_types::string_value, "field", "field", offsetof(database::projection_element, field_name));
				map_scalar<database::string_box>(projection_field_ti, pvalue::pvalue_types::string_value, "project", "project", offsetof(database::projection_element, projection_name));

				query_filter_ti = create_map(member_type_name, "filter", "filter", 20);
				map_scalar<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "target_field_name", "target_field_name", offsetof(database::filter_element, target_field_name));
				map_scalar<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "comparison", "comparison", offsetof(database::filter_element, comparison_name));
				map_scalar<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "parameter_field_name", "parameter_field_name", offsetof(database::filter_element, parameter_field_name));
				map_scalar<database::double_box>(query_filter_ti, pvalue::pvalue_types::double_value, "distance", "distance", offsetof(database::filter_element, distance_threshold));

				query_fields_ti = create_map(member_type_name, "queryfield", "queryfield", 20);
				map_scalar<database::int32_box>(query_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_named_query_field_request, name.field_id));
				map_scalar<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_named_query_field_request, name.name));
				map_scalar<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_named_query_field_request, name.description));
				map_object(query_fields_ti, path_ti, "path", "path", offsetof(database::put_named_query_field_request, options.source_path));
				map_iarray<database::filter_element, database::max_query_filters>(query_fields_ti, query_filter_ti, "filters", "filters", offsetof(database::put_named_query_field_request, options.filter));
				map_iarray<database::projection_element, database::max_projection_fields>(query_fields_ti, projection_field_ti, "projections", "projections", offsetof(database::put_named_query_field_request, options.projection));

				remote_parameters_ti = create_map(member_type_name, "parameter", "parameter", 20);
				map_scalar<database::string_box>(remote_parameters_ti, pvalue::pvalue_types::string_value, "corona_field", "corona_field", offsetof(database::remote_field_map_type, corona_field));
				map_scalar<database::string_box>(remote_parameters_ti, pvalue::pvalue_types::string_value, "remote_field", "remote_field", offsetof(database::remote_field_map_type, remote_field));

				sql_fields_ti = create_map(member_type_name, "sqlfield", "sqlfield", 20);
				map_scalar<database::int32_box>(sql_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_named_sql_remote_field_request, name.field_id));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_named_sql_remote_field_request, name.name));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_named_sql_remote_field_request, name.description));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "login_type", "login_type", offsetof(database::put_named_sql_remote_field_request, options.login_type_name));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "username", "username", offsetof(database::put_named_sql_remote_field_request, options.username));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "password", "password", offsetof(database::put_named_sql_remote_field_request, options.password));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "result_class", "result_class", offsetof(database::put_named_sql_remote_field_request, options.result_class_name));
				map_scalar<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "query", "query", offsetof(database::put_named_sql_remote_field_request, options.query));
				map_iarray<database::remote_field_map_type, database::max_remote_fields>(sql_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_named_sql_remote_field_request, options.parameters));
				map_iarray<database::remote_field_map_type, database::max_remote_parameter_fields >(sql_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_named_sql_remote_field_request, options.fields));

				file_fields_ti = create_map(member_type_name, "filefield", "filefield", 20);
				map_scalar<database::int32_box>(file_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_named_file_remote_field_request, name.field_id));
				map_scalar<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_named_file_remote_field_request, name.name));
				map_scalar<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_named_file_remote_field_request, name.description));
				map_scalar<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "file", "file", offsetof(database::put_named_file_remote_field_request, options.file_path));
				map_scalar<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "result_class", "result_class", offsetof(database::put_named_file_remote_field_request, options.result_class_name));
				map_iarray<database::remote_field_map_type, database::max_remote_fields>(file_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_named_file_remote_field_request, options.parameters));
				map_iarray<database::remote_field_map_type, database::max_remote_parameter_fields >(file_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_named_file_remote_field_request, options.fields));

				http_fields_ti = create_map(member_type_name, "httpfield", "httpfield", 20);
				map_scalar<database::int32_box>(http_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_named_http_remote_field_request, name.field_id));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_named_http_remote_field_request, name.name));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_named_http_remote_field_request, name.description));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_type", "login_type", offsetof(database::put_named_http_remote_field_request, options.login_type_name));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_url", "login_url", offsetof(database::put_named_http_remote_field_request, options.login_url));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_method", "login_method", offsetof(database::put_named_http_remote_field_request, options.login_method));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "username", "username", offsetof(database::put_named_http_remote_field_request, options.username));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "password", "password", offsetof(database::put_named_http_remote_field_request, options.password));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "data_url", "data_url", offsetof(database::put_named_http_remote_field_request, options.data_url));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "data_method", "data_method", offsetof(database::put_named_http_remote_field_request, options.data_method));
				map_scalar<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "result_class", "result_class", offsetof(database::put_named_http_remote_field_request, options.result_class_name));
				map_iarray<database::remote_field_map_type, database::max_remote_fields>(http_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_named_http_remote_field_request, options.parameters));
				map_iarray<database::remote_field_map_type, database::max_remote_parameter_fields >(http_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_named_http_remote_field_request, options.fields));

				put_class_fields_ti = create_map(member_type_name, "class_fields", "class_fields", 20);
				map_scalar<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "field_name", "field_name", offsetof(database::member_field, field_name));
				map_scalar<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "member_type", "member_type", offsetof(database::member_field, membership_type_name));
				map_scalar<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_x", "dim_x", offsetof(database::member_field, dimensions.x));
				map_scalar<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_y", "dim_y", offsetof(database::member_field, dimensions.y));
				map_scalar<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_z", "dim_z", offsetof(database::member_field, dimensions.z));

				put_classes_ti = create_map(member_type_name, "class", "class", 20);
				map_scalar<database::int32_box>(put_classes_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_class_request, class_id));
				map_scalar<database::string_box>(put_classes_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_class_request, class_name));
				map_scalar<database::string_box>(put_classes_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_class_request, class_description));
				map_iarray<database::member_field, database::max_class_fields >(put_classes_ti, put_class_fields_ti, "fields", "fields", offsetof(database::put_class_request, member_fields));

				color_ti = create_map(member_type_name, "color", "color", 20);
				map_scalar<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "red", "red", offsetof(database::color, red));
				map_scalar<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "green", "green", offsetof(database::color, green));
				map_scalar<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "blue", "blue", offsetof(database::color, blue));
				map_scalar<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "alpha", "alpha", offsetof(database::color, alpha));

				point_ti = create_map(member_type_name, "point", "point", 20);
				map_scalar<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::point, x));
				map_scalar<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::point, y));
				map_scalar<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::point, z));

				rectangle_ti = create_map(member_type_name, "rectangle", "rectangle", 20);
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::rectangle, x));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::rectangle, y));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "w", "w", offsetof(database::rectangle, w));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "h", "h", offsetof(database::rectangle, h));

				image_ti = create_map(member_type_name, "image", "image", 20);
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::rectangle, x));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::rectangle, y));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "w", "w", offsetof(database::rectangle, w));
				map_scalar<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "h", "h", offsetof(database::rectangle, h));

				wave_ti = create_map(member_type_name, "wave", "wave", 20);
				map_scalar<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "start_seconds", "start_seconds", offsetof(database::wave_instance, start_seconds));
				map_scalar<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "stop_seconds", "stop_seconds", offsetof(database::wave_instance, stop_seconds));
				map_scalar<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "pitch_adjust", "pitch_adjust", offsetof(database::wave_instance, pitch_adjust));
				map_scalar<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "volume_adjust", "volume_adjust", offsetof(database::wave_instance, volume_adjust));

				midi_ti = create_map(member_type_name, "midi", "midi", 20);
				map_scalar<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "start_seconds", "start_seconds", offsetof(database::midi_instance, start_seconds));
				map_scalar<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "stop_seconds", "stop_seconds", offsetof(database::midi_instance, stop_seconds));
				map_scalar<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "pitch_adjust", "pitch_adjust", offsetof(database::midi_instance, pitch_adjust));
				map_scalar<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "volume_adjust", "volume_adjust", offsetof(database::midi_instance, volume_adjust));
			}

			bool add_schema(pmember *member)
			{
				if (member->name == "schema") {
					auto fields = member->value->as_array();
					for (auto fld = fields->first; fld; fld = fld->next)
					{
						auto obj = fld->as_object();
						if (obj)
						{
							load(obj);
						}
						else
						{
							put_error(errors::invalid_object_parse, obj);
							return false;
						}
					}
					return true;
				}
				else 
				{
					put_error(errors::invalid_object_parse, member->name);
					return false;
				}
				return error_messages.size() == 0;
			}

			bool update_schema(database::jschema& schema)
			{
				index_fields();
				for (auto fld : fields_by_name) {
					put_field(schema, fld.second);
				}
				for (auto cls : put_classes) {
					put_class(schema, cls.item);
				}
				return error_messages.size() == 0;
			}

			virtual char* place(typeinfo* ti)
			{
				char* t = nullptr;
				database::relative_ptr_type rr;

				if (ti == string_fields_ti)
				{
					auto* asf = put_string_fields.create(1, rr);
					asf->name.field_id = database::null_row;
					asf->name.type_id = database::jtype::type_string;
					t = (char*)asf;
				}
				else if (ti == int8_fields_ti)
				{
					auto* ai8f = put_integer_fields.create(1, rr);
					ai8f->name.field_id = database::null_row;
					ai8f->name.type_id = database::jtype::type_int8;
					t = (char*)ai8f;
				}
				else if (ti == int16_fields_ti)
				{
					auto* ai16f = put_integer_fields.create(1, rr);
					ai16f->name.field_id = database::null_row;
					ai16f->name.type_id = database::jtype::type_int16;
					t = (char*)ai16f;
				}
				else if (ti == int32_fields_ti)
				{
					auto* ai32f = put_integer_fields.create(1, rr);
					ai32f->name.field_id = database::null_row;
					ai32f->name.type_id = database::jtype::type_int32;
					t = (char*)ai32f;
				}
				else if (ti == int64_fields_ti)
				{
					auto* ai64f = put_integer_fields.create(1, rr);
					ai64f->name.field_id = database::null_row;
					ai64f->name.type_id = database::jtype::type_int64;
					t = (char*)ai64f;
				}
				else if (ti == float_fields_ti)
				{
					auto* af32f = put_double_fields.create(1, rr);
					af32f->name.field_id = database::null_row;
					af32f->name.type_id = database::jtype::type_float32;
					t = (char*)af32f;
				}
				else if (ti == double_fields_ti)
				{
					auto* af64f = put_double_fields.create(1, rr);
					af64f->name.field_id = database::null_row;
					af64f->name.type_id = database::jtype::type_float64;
					t = (char*)af64f;
				}
				else if (ti == time_fields_ti)
				{
					auto* at64f = put_time_fields.create(1, rr);
					at64f->name.field_id = database::null_row;
					at64f->name.type_id = database::jtype::type_datetime;
					t = (char*)at64f;
				}
				else if (ti == object_fields_ti)
				{
					auto* aof = put_object_fields.create(1, rr);
					aof->name.field_id = database::null_row;
					aof->name.type_id = database::jtype::type_object;
					t = (char*)aof;
				}
				else if (ti == list_fields_ti)
				{
					auto* aof = put_list_fields.create(1, rr);
					aof->name.field_id = database::null_row;
					aof->name.type_id = database::jtype::type_list;
					t = (char*)aof;
				}
				else if (ti == query_fields_ti)
				{
					auto* aqf = put_query_fields.create(1, rr);
					aqf->name.field_id = database::null_row;
					aqf->name.type_id = database::jtype::type_query;
					t = (char*)aqf;
				}
				else if (ti == point_fields_ti)
				{
					auto* apf = put_point_fields.create(1, rr);
					apf->name.field_id = database::null_row;
					apf->name.type_id = database::jtype::type_point;
					t = (char*)apf;
				}
				else if (ti == rectangle_fields_ti)
				{
					auto* arf = put_rectangle_fields.create(1, rr);
					arf->name.field_id = database::null_row;
					arf->name.type_id = database::jtype::type_rectangle;
					t = (char*)arf;
				}
				else if (ti == color_fields_ti)
				{
					auto* acf = put_color_fields.create(1, rr);
					acf->name.field_id = database::null_row;
					acf->name.type_id = database::jtype::type_color;
					t = (char*)acf;
				}
				else if (ti == image_fields_ti)
				{
					auto* aimf = put_image_fields.create(1, rr);
					aimf->name.field_id = database::null_row;
					aimf->name.type_id = database::jtype::type_image;
					t = (char*)aimf;
				}
				else if (ti == wave_fields_ti)
				{
					auto* awf = put_wave_fields.create(1, rr);
					awf->name.field_id = database::null_row;
					awf->name.type_id = database::jtype::type_wave;
					t = (char*)awf;
				}
				else if (ti == midi_fields_ti)
				{
					auto* amf = put_midi_fields.create(1, rr);
					amf->name.field_id = database::null_row;
					amf->name.type_id = database::jtype::type_midi;
					t = (char*)amf;
				}
				else if (ti == put_classes_ti)
				{
					auto *amf = put_classes.create(1, rr);
					amf->class_id = database::null_row;
					t = (char*)amf;
				}
				else if (ti == sql_fields_ti)
				{
					auto* sqf = put_sql_fields.create(1, rr);
					sqf->name.field_id = database::null_row;
					sqf->name.type_id = database::jtype::type_sql;
					t = (char*)sqf;
				}
				else if (ti == file_fields_ti)
				{
					auto* flf = put_file_fields.create(1, rr);
					flf->name.field_id = database::null_row;
					flf->name.type_id = database::jtype::type_file;
					t = (char*)flf;
				}
				else if (ti == http_fields_ti)
				{
					auto* htf = put_http_fields.create(1, rr);
					htf->name.field_id = database::null_row;
					htf->name.type_id = database::jtype::type_http;
					t = (char*)htf;
				}
				else if (ti == query_filter_ti)
				{
					t = (char*)nullptr;
				}
				else if (ti == remote_parameters_ti)
				{
					t = (char*)nullptr;
				}

				return t;
			}

			void index_fields()
			{
				location loc;

				loc.item_type = database::jtype::type_string;
				for (auto asf : put_string_fields)
				{
					loc.row_id = asf.location;
					fields_by_name.insert_or_assign(asf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_int32;
				for (auto aif : put_integer_fields)
				{
					loc.row_id = aif.location;
					fields_by_name.insert_or_assign(aif.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_float32;
				for (auto adf : put_double_fields)
				{
					loc.row_id = adf.location;
					fields_by_name.insert_or_assign(adf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_datetime;
				for (auto atf : put_time_fields)
				{
					loc.row_id = atf.location;
					fields_by_name.insert_or_assign(atf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_point;
				for (auto apf : put_point_fields)
				{
					loc.row_id = apf.location;
					fields_by_name.insert_or_assign(apf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_rectangle;
				for (auto arf : put_rectangle_fields)
				{
					loc.row_id = arf.location;
					fields_by_name.insert_or_assign(arf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_color;
				for (auto acf : put_color_fields)
				{
					loc.row_id = acf.location;
					fields_by_name.insert_or_assign(acf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_image;
				for (auto aimf : put_image_fields)
				{
					loc.row_id = aimf.location;
					fields_by_name.insert_or_assign(aimf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_wave;
				for (auto awvf : put_wave_fields)
				{
					loc.row_id = awvf.location;
					fields_by_name.insert_or_assign(awvf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_image;
				for (auto amdf : put_midi_fields)
				{
					loc.row_id = amdf.location;
					fields_by_name.insert_or_assign(amdf.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_object;
				for (auto off : put_object_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_list;
				for (auto off : put_list_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_query;
				for (auto off : put_query_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_sql;
				for (auto off : put_sql_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_file;
				for (auto off : put_file_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				loc.item_type = database::jtype::type_http;
				for (auto off : put_http_fields)
				{
					loc.row_id = off.location;
					fields_by_name.insert_or_assign(off.item.name.name, loc);
				}

				for (auto cls : put_classes)
				{
					classes_by_name.insert_or_assign(cls.item.class_name, cls.location);
				}
			}

			bool is_field_mapping_valid(database::jschema* pschema, database::remote_field_map_type& remote)
			{
				database::remote_field_map_type* premote = &remote;

				bool valid = false;

				database::relative_ptr_type field_id = pschema->find_field(remote.corona_field);

				if (!valid) 
				{
					put_error(errors::invalid_mapping, remote.corona_field.c_str());
				}

				return valid;
			}

			template <int number> 
			bool check_field_mapping_valid(database::jschema& schema, database::iarray<database::remote_field_map_type, number>& remotes)
			{
				database::jschema* pschema = &schema;
				bool valid = std::all_of(remotes.begin(), remotes.end(), [this,pschema](const auto& src) {
					return is_field_mapping_valid(pschema, src.item);
					});
				return valid;
			}

			template <typename RemoteField, typename SrcTable> 
			bool validate_remote_field(database::jschema& schema, RemoteField& rf)
			{
				bool validParams = check_field_mapping_valid(schema, rf.parameter_field, rf.parameters);
				bool validFields = check_field_mapping_valid(schema, rf.results_field, rf.fields);
				return (validParams && validFields);
			}

			void put_field(database::jschema& schema, database::put_named_sql_remote_field_request& fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.fields);
				schema.put_sql_remote_field(fld);
			}

			void put_field(database::jschema& schema, database::put_named_file_remote_field_request & fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.fields);
				schema.put_file_remote_field(fld);
			}

			void put_field(database::jschema& schema, database::put_named_http_remote_field_request& fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.fields);
				schema.put_http_remote_field(fld);
			}

			void put_field(database::jschema& schema, location loc)
			{

				switch (loc.item_type) {
				case database::jtype::type_collection_id:
					break;
				case database::jtype::type_int8:
				case database::jtype::type_int16:
				case database::jtype::type_int32:
				case database::jtype::type_int64:
					{
						auto fld = put_integer_fields[ loc.row_id ];
						schema.put_integer_field(fld);
					}
					break;
				case database::jtype::type_float32:
				case database::jtype::type_float64:
					{
						auto fld = put_double_fields[loc.row_id];
						schema.put_double_field(fld);
					}
					break;
				case database::jtype::type_image:
					{
						auto fld = put_image_fields[loc.row_id];
						schema.put_image_field(fld);
					}
					break;
				case database::jtype::type_color:
					{
						auto fld = put_color_fields[loc.row_id];
						schema.put_color_field(fld);
					}
					break;
				case database::jtype::type_datetime:
					{
						auto fld = put_time_fields[loc.row_id];
						schema.put_time_field(fld);
					}
					break;
				case database::jtype::type_midi:
					{
						auto fld = put_midi_fields[loc.row_id];
						schema.put_midi_field(fld);
					}
					break;
				case database::jtype::type_object:
					{
						auto fld = put_object_fields[loc.row_id];
						put_object_field(schema, fld);
					}
					break;
				case database::jtype::type_list:
					{
						auto fld = put_list_fields[loc.row_id];
						put_list_field(schema, fld);
					}
					break;
				case database::jtype::type_point:
					{
						auto fld = put_point_fields[loc.row_id];
						schema.put_point_field(fld);
					}
					break;
				case database::jtype::type_query:
					{
						auto fld = put_query_fields[loc.row_id];
						put_query_field(schema, fld);
					}
					break;
				case database::jtype::type_rectangle:
					{
						auto fld = put_rectangle_fields[loc.row_id];
						schema.put_rectangle_field(fld);
					}
					break;
				case database::jtype::type_string:
					{
						auto fld = put_string_fields[loc.row_id];
						schema.put_string_field(fld);
					}
					break;
				case database::jtype::type_wave:
					{
						auto fld = put_wave_fields[loc.row_id];
						schema.put_wave_field(fld);
					}
					break;
				case database::jtype::type_sql:
					{
						auto fld = put_sql_fields[loc.row_id];
						put_field(schema, fld);
					}
					break;
				case database::jtype::type_file:
					{
						auto fld = put_file_fields[loc.row_id];
						put_field(schema, fld);
					}
					break;
				case database::jtype::type_http:
					{
						auto fld = put_http_fields[loc.row_id];
						put_field(schema, fld);
					}
					break;
				}
			}

			void put_query_field(database::jschema& schema, database::put_named_query_field_request& aorf)
			{
				if (schema.put_query_field(aorf) == database::null_row) {
					put_error(errors::invalid_object_parse, nullptr, 0);
				}
			}

			void put_object_field(database::jschema& schema, database::put_object_field_request& aorf)
			{

				bool valid = true;

				auto class_row_id = schema.find_class(aorf.options.class_name);
				if (class_row_id == database::null_row)
				{
					auto class_name_iter = classes_by_name[ aorf.options.class_name ];
					if (class_name_iter == std::end(classes_by_name)) 
					{
						put_error( errors::class_not_defined, aorf.options.class_name.c_str(), 0 );
						valid = false;
					}
					else 
					{
						auto def_row_id = class_name_iter.get_value();
						auto &class_def = put_classes[def_row_id];
						put_class( schema, class_def );
					}
				}
				if (valid) 
				{
					schema.put_object_field(aorf);
				}
			}

			void put_list_field(database::jschema& schema, database::put_object_field_request& aorf)
			{

				bool valid = true;

				auto class_row_id = schema.find_class(aorf.options.class_name);
				if (class_row_id == database::null_row)
				{
					auto class_name_iter = classes_by_name[aorf.options.class_name];
					if (class_name_iter == std::end(classes_by_name))
					{
						put_error(errors::class_not_defined, aorf.options.class_name.c_str(), 0);
						valid = false;
					}
					else
					{
						auto def_row_id = class_name_iter.get_value();
						auto& class_def = put_classes[def_row_id];
						put_class(schema, class_def);
					}
				}
				if (valid)
				{
					schema.put_list_field(aorf);
				}
			}

			database::relative_ptr_type put_class(database::jschema& schema, database::put_class_request& aorf)
			{
				for (auto fn : aorf.member_fields) 
				{
					if (fn.item.membership_type == database::member_field_types::member_field) 
					{
						auto field_name_iter = fields_by_name[fn.item.field_name];
						if (field_name_iter == std::end(fields_by_name))
						{
							put_error(errors::field_not_defined, fn.item.field_name.c_str(), 0);
						}
						else {
							auto& loc = field_name_iter.get_value();
							auto found_field_id = schema.find_field(fn.item.field_name);
							if (found_field_id == database::null_row)
							{
								put_field(schema, loc);
							}
						}
					}
					else if (fn.item.membership_type == database::member_field_types::member_class)
					{
						auto found_class_id = schema.find_class(fn.item.field_name);
						if (found_class_id == database::null_row)
						{
							auto class_name_iter = classes_by_name[fn.item.field_name];
							if (class_name_iter == std::end(classes_by_name))
							{
								put_error(errors::class_not_defined, fn.item.field_name.c_str());
							}
							else
							{
								auto def_row_id = class_name_iter.get_value();
								auto& class_def = put_classes[def_row_id];
								put_class(schema, class_def);
							}
						}
					}
				}

				return schema.put_class(aorf);
			}

		};

		class data_loader : loader
		{
		public:

			typeinfo* map_corona_class(database::jschema& _schema, database::relative_ptr_type class_id)
			{
				auto class_data = _schema.get_class(class_id);

				auto class_name = class_data.item().name.c_str();
				typeinfo* class_type = find_typeinfo(class_name);
				if (class_type != nullptr) {
					return class_type;
				}
				class_type = create_map("corona", class_name, class_name, class_data.size() + 1);
				for (int i = 0; i < class_data.size(); i++)
				{
					auto class_field = class_data.detail(i);
					auto field = _schema.get_field(class_field.field_id);
					switch (field.type_id)
					{
					case database::jtype::type_int8:
						map_corona_scalar<database::int8_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_int16:
						map_corona_scalar<database::int16_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_int32:
						map_corona_scalar<database::int32_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_int64:
						map_corona_scalar<database::int64_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_float32:
						map_corona_scalar<database::float_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_float64:
						map_corona_scalar<database::double_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::double_value);
						break;
					case database::jtype::type_datetime:
						map_corona_scalar<database::time_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::time_value);
						break;
					case database::jtype::type_object:
					{
						typeinfo* item_type_info = map_corona_class(_schema, field.object_properties.class_id);
						map_corona_array(class_type, &_schema, i, field.name.c_str(), item_type_info);
					}
					break;
					case database::jtype::type_list:
					{
						typeinfo* item_type_info = map_corona_class(_schema, field.object_properties.class_id);
						map_corona_list(class_type, &_schema, i, field.name.c_str(), item_type_info);
					}
					break;
					case database::jtype::type_object_id:
						map_corona_scalar<database::object_id_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_collection_id:
						map_corona_scalar<database::collection_id_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_string:
						map_corona_scalar<database::string_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_image:
						map_corona_scalar<database::image_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_wave:
						map_corona_scalar<database::wave_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_midi:
						map_corona_scalar<database::midi_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_point:
						map_corona_scalar<database::point_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_rectangle:
						map_corona_scalar<database::rectangle_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_color:
						map_corona_scalar<database::color_box>(class_type, &_schema, i, field.name.c_str(), pvalue::pvalue_types::string_value);
						break;
					case database::jtype::type_sql:
						break;
					case database::jtype::type_http:
						break;
					case database::jtype::type_file:
						break;
					case database::jtype::type_query:
						break;
					}
				}
			}

			const char* member_type_name = "type";

		public:

			data_loader(int _size, int _num_fields) : loader(_size, _num_fields)
			{

			}

			bool put_slice(database::jschema& _schema, database::jslice& _slice, pobject* _obj)
			{
				auto type_member = _obj->get_member(member_type_name);
				if (type_member) {
					const char* class_name = type_member->value->as_string();
					auto class_id = _schema.find_class(class_name);
					if (class_id == database::null_row)
					{
						put_error(errors::class_not_defined, _obj);
						return false;
					}
				}
				else
				{
					put_error(errors::class_not_defined, _obj);
					return false;
				}
			}

		};
	}
}
