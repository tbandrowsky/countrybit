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

			database::row_id_type	class_id;
			database::jclass		cls;
			int						field_idx;
			database::jclass_field	class_field;
			database::jschema*		schema;

			pvalue::pvalue_types match_type;

		public:

			corona_property_dest(database::jschema* _schema, const char* _class_name, const char* _dest_name, pvalue::pvalue_types _match_type) :
				property_dest(_dest_name),
				schema(_schema),
				match_type(_match_type)
			{
				class_id = schema->find_class(_class_name);
				cls = schema->get_class(class_id);

				for (int i = 0; i < cls.size(); i++)
				{
					field_idx = i;
					class_field = cls.child(i);
				}
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

			corona_scalar_property_dest(database::jschema* _schema, const char* _class_name, const char* _dest_name, pvalue::pvalue_types _match_type) : 
				corona_property_dest(_schema, _class_name, _dest_name, _match_type)
			{

			}

			virtual bool set_value(database::jslice slice, const pvalue* _src)
			{
				if (_src->as_object() || _src->as_array()) {
					throw std::logic_error("attempt to map non-scalar to scalar");
				}
				MemberType item;
				slice.get_box(item, field_idx);
				_src->set_value(item);
				return true;
			}
		};

		class corona_list_property_dest : public corona_property_dest
		{
			typeinfo* property_type;

		public:

			corona_list_property_dest(database::jschema* _schema, typeinfo* _property_type, const char* _class_name, const char* _dest_name, pvalue::pvalue_types _match_type) :
				corona_property_dest(_schema, _class_name, _dest_name, _match_type)
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
					auto prop = property_type->find_property(member);
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
			typeinfo* property_type;

		public:

			corona_array_property_dest(database::jschema* _schema, typeinfo* _property_type, const char* _class_name, const char* _dest_name, pvalue::pvalue_types _match_type) :
				corona_property_dest(_schema, _class_name, _dest_name, _match_type)
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
					auto prop = property_type->find_property(member);
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
			database::row_id_type error_messages_id;

		public:

			loader(int _size, int _max_types) 
			{
				database::row_id_type header_location;
				data.init(_size);
				bindings_by_name.create_sorted_index(&data, _max_types, header_location);
				error_messages = database::table<error_message>::create_table(&data, 250, error_messages_id);
			}

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

				pi->put_setter(new_dest);
				return pi;
			}

			template <typename MemberType>
			propertyinfo* create_scalar_property(typeinfo *parent, pvalue::pvalue_types _match_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* sp = create_scalar_property(_match_type, _source_name, _dest_name, _offset);
				parent->put_property(sp);
			}

			propertyinfo* create_object_property(typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				char *t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				t = data.place<object_property_dest>();
				object_property_dest* new_dest = new (t) object_property_dest(_dest_name, _offset, _property_type);

				pi->put_setter(new_dest);

				return pi;
			}

			propertyinfo* create_object_property(typeinfo *parent, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = create_object_property(_property_type, _source_name, _dest_name, _offset);
				parent->put_property(op);
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

				pi->put_setter(new_dest);

				return pi;
			}

			template <typename ArrayMemberType, int ArraySize>
			propertyinfo* create_object_iarray_property(typeinfo* parent, typeinfo* _property_type, const char* _source_name, const char* _dest_name, int _offset)
			{
				propertyinfo* op = create_object_iarray_property<ArrayMemberType,ArraySize>(_property_type, _source_name, _dest_name, _offset);
				parent->put_property(op);
				return op;
			}

			propertyinfo* create_polymorphic_property(const char* _source_name, int _num_dests, property_dest **_dests)
			{
				char* t = data.place<propertyinfo>();
				propertyinfo* pi = new (t) propertyinfo(_source_name);

				for (int i = 0; i < _num_dests; i++) {
					pi->put_setter(_dests[i]);
				}

				return pi;
			}

			propertyinfo* create_polymorphic_property(typeinfo* parent, const char* _source_name, int _num_dests, property_dest** _dests)
			{
				propertyinfo* op = create_polymorphic_property(_source_name, _num_dests, _dests);
				parent->put_property(op);
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

		class corona_loader : loader
		{
		public:

			struct location 
			{
				database::jtype item_type;
				database::row_id_type row_id;					
			};

		private:

			database::sorted_index<database::object_name, location> fields_by_name;
			database::sorted_index<database::object_name, database::row_id_type> classes_by_name;
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
			database::table<database::put_color_field_request> put_color_fields;
			database::table<database::put_named_query_field_request> put_query_fields;
			database::table<database::put_sql_field_request> put_sql_fields;
			database::table<database::put_http_field_request> put_http_fields;
			database::table<database::put_file_field_request> put_file_fields;
			database::table<database::put_class_request> put_classes;

			database::row_id_type fields_by_name_id;
			database::row_id_type classes_by_name_id;
			database::row_id_type class_types_by_name_id;
			database::row_id_type put_string_fields_id;
			database::row_id_type put_integer_fields_id;
			database::row_id_type put_double_fields_id;
			database::row_id_type put_time_fields_id;
			database::row_id_type put_object_fields_id;
			database::row_id_type put_list_fields_id;
			database::row_id_type put_query_fields_id;
			database::row_id_type put_sql_fields_id;
			database::row_id_type put_http_fields_id;
			database::row_id_type put_file_fields_id;
			database::row_id_type put_point_fields_id;
			database::row_id_type put_rectangle_fields_id;
			database::row_id_type put_color_fields_id;
			database::row_id_type put_image_fields_id;
			database::row_id_type put_wave_fields_id;
			database::row_id_type put_midi_fields_id;
			database::row_id_type put_classes_id;

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

			corona_loader(int _size, int _num_fields) : loader(_size, _num_fields)
			{

				fields_by_name = database::sorted_index<database::object_name, location>::create_sorted_index(&data, _num_fields, fields_by_name_id);
				classes_by_name = database::sorted_index<database::object_name, database::row_id_type>::create_sorted_index(&data, _num_fields, classes_by_name_id);
				class_types_by_name = database::sorted_index<database::object_name, typeinfo *>::create_sorted_index(&data, _num_fields, class_types_by_name_id);
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

				string_fields_ti = create_typeinfo(member_type_name, "string", "string", 20);
				create_scalar_property<database::int32_box>(string_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_string_field_request, name.field_id));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_string_field_request, name.name));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_string_field_request, name.description));
				create_scalar_property<database::int32_box>(string_fields_ti, pvalue::pvalue_types::double_value, "length", "length", offsetof(database::put_string_field_request, options.length));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_pattern", "validation_pattern", offsetof(database::put_string_field_request, options.validation_pattern));
				create_scalar_property<database::string_box>(string_fields_ti, pvalue::pvalue_types::string_value, "validation_message", "validation_message", offsetof(database::put_string_field_request, options.validation_message));

				int8_fields_ti = create_typeinfo(member_type_name, "int8", "int8", 20);
				create_scalar_property<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				create_scalar_property<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int8_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int8_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int16_fields_ti = create_typeinfo(member_type_name, "int16", "int16", 20);
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				create_scalar_property<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int16_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int32_fields_ti = create_typeinfo(member_type_name, "int32", "int32", 20);
				create_scalar_property<database::int32_box>(int16_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				create_scalar_property<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int32_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int32_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				int64_fields_ti = create_typeinfo(member_type_name, "int64", "int64", 20);
				create_scalar_property<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_integer_field_request, name.field_id));
				create_scalar_property<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_integer_field_request, name.name));
				create_scalar_property<database::string_box>(int64_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_integer_field_request, name.description));
				create_scalar_property<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_integer_field_request, options.minimum_int));
				create_scalar_property<database::int32_box>(int64_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_integer_field_request, options.maximum_int));

				float_fields_ti = create_typeinfo(member_type_name, "float", "float", 20);
				create_scalar_property<database::int32_box>(float_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_double_field_request, name.field_id));
				create_scalar_property<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_double_field_request, name.name));
				create_scalar_property<database::string_box>(float_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_double_field_request, name.description));
				create_scalar_property<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_double_field_request, options.minimum_double));
				create_scalar_property<database::double_box>(float_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_double_field_request, options.maximum_double));

				double_fields_ti = create_typeinfo(member_type_name, "double", "double", 20);
				create_scalar_property<database::int32_box>(double_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_double_field_request, name.field_id));
				create_scalar_property<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_double_field_request, name.name));
				create_scalar_property<database::string_box>(double_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_double_field_request, name.description));
				create_scalar_property<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_double_field_request, options.minimum_double));
				create_scalar_property<database::double_box>(double_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_double_field_request, options.maximum_double));

				time_fields_ti = create_typeinfo(member_type_name, "time", "time", 20);
				create_scalar_property<database::int32_box>(double_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_time_field_request, name.field_id));
				create_scalar_property<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_time_field_request, name.name));
				create_scalar_property<database::string_box>(time_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_time_field_request, name.description));
				create_scalar_property<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "minimum", "minimum", offsetof(database::put_time_field_request, options.minimum_time_t));
				create_scalar_property<database::int64_box>(time_fields_ti, pvalue::pvalue_types::double_value, "maximum", "maximum", offsetof(database::put_time_field_request, options.maximum_time_t));

				object_fields_ti = create_typeinfo(member_type_name, "object", "object", 20);
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_object_field_request, name.field_id));
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_object_field_request, name.name));
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_object_field_request, name.description));
				create_scalar_property<database::string_box>(object_fields_ti, pvalue::pvalue_types::string_value, "class_name", "class_name", offsetof(database::put_object_field_request, options.class_name));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::put_object_field_request, options.dim.x));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::put_object_field_request, options.dim.y));
				create_scalar_property<database::int32_box>(object_fields_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::put_object_field_request, options.dim.z));

				list_fields_ti = create_typeinfo(member_type_name, "list", "list", 20);
				create_scalar_property<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_object_field_request, name.field_id));
				create_scalar_property<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_object_field_request, name.name));
				create_scalar_property<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_object_field_request, name.description));
				create_scalar_property<database::string_box>(list_fields_ti, pvalue::pvalue_types::string_value, "class_name", "class_name", offsetof(database::put_object_field_request, options.class_name));
				create_scalar_property<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::put_object_field_request, options.dim.x));
				create_scalar_property<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::put_object_field_request, options.dim.y));
				create_scalar_property<database::int32_box>(list_fields_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::put_object_field_request, options.dim.z));

				point_fields_ti = create_typeinfo(member_type_name, "point", "point", 20);
				create_scalar_property<database::int32_box>(point_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_point_field_request, name.field_id));
				create_scalar_property<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_point_field_request, name.name));
				create_scalar_property<database::string_box>(point_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_point_field_request, name.description));

				rectangle_fields_ti = create_typeinfo(member_type_name, "rectangle", "rectangle", 20);
				create_scalar_property<database::int32_box>(rectangle_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_rectangle_field_request, name.field_id));
				create_scalar_property<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_rectangle_field_request, name.name));
				create_scalar_property<database::string_box>(rectangle_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_rectangle_field_request, name.description));

				color_fields_ti = create_typeinfo(member_type_name, "color", "color", 20);
				create_scalar_property<database::int32_box>(color_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_color_field_request, name.field_id));
				create_scalar_property<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_color_field_request, name.name));
				create_scalar_property<database::string_box>(color_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_color_field_request, name.description));

				image_fields_ti = create_typeinfo(member_type_name, "image", "image", 20);
				create_scalar_property<database::int32_box>(image_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_image_field_request, name.field_id));
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_image_field_request, name.name));
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_image_field_request, name.description));
				create_scalar_property<database::string_box>(image_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_image_field_request, options.image_path));

				wave_fields_ti = create_typeinfo(member_type_name, "wave", "wave", 20);
				create_scalar_property<database::int32_box>(wave_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_wave_field_request, name.field_id));
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_wave_field_request, name.name));
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_wave_field_request, name.description));
				create_scalar_property<database::string_box>(wave_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_wave_field_request, options.image_path));

				midi_fields_ti = create_typeinfo(member_type_name, "midi", "midi", 20);
				create_scalar_property<database::int32_box>(midi_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_midi_field_request, name.field_id));
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_midi_field_request, name.name));
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_midi_field_request, name.description));
				create_scalar_property<database::string_box>(midi_fields_ti, pvalue::pvalue_types::string_value, "path", "path", offsetof(database::put_midi_field_request, options.image_path));

				path_node_ti = create_typeinfo(member_type_name, "node", "node", 20);
				create_scalar_property<database::string_box>(path_node_ti, pvalue::pvalue_types::string_value, "member", "member", offsetof(database::path_node, member_name));

				path_ti = create_typeinfo(member_type_name, "path", "path", 20);
				create_scalar_property<database::string_box>(path_ti, pvalue::pvalue_types::string_value, "collection", "collection", offsetof(database::path, root.collection_name));
				create_object_iarray_property<database::path, database::max_path_nodes>(path_ti, path_node_ti, "nodes", "nodes", offsetof(database::path, nodes));

				query_filter_ti = create_typeinfo(member_type_name, "filter", "filter", 20);
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "target_field_name", "target_field_name", offsetof(database::filter_element_request, target_field_name));
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "comparison", "comparison", offsetof(database::filter_element_request, comparison_name));
				create_scalar_property<database::string_box>(query_filter_ti, pvalue::pvalue_types::string_value, "parameter_field_name", "parameter_field_name", offsetof(database::filter_element_request, parameter_field_name));
				create_scalar_property<database::double_box>(query_filter_ti, pvalue::pvalue_types::double_value, "distance", "distance", offsetof(database::filter_element_request, distance_threshold));

				query_fields_ti = create_typeinfo(member_type_name, "query", "query", 20);
				create_scalar_property<database::int32_box>(query_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_named_query_field_request, name.field_id));
				create_scalar_property<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_named_query_field_request, name.name));
				create_scalar_property<database::string_box>(query_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_named_query_field_request, name.description));
				create_object_property(query_fields_ti, path_ti, "path", "path", offsetof(database::put_named_query_field_request, options.source_path));
				create_object_iarray_property<database::filter_element_request, database::max_query_filters>(query_fields_ti, query_filter_ti, "filters", "filters", offsetof(database::put_named_query_field_request, options.filter));

				remote_parameters_ti = create_typeinfo(member_type_name, "parameter", "parameter", 20);
				create_scalar_property<database::string_box>(remote_parameters_ti, pvalue::pvalue_types::string_value, "corona_field", "corona_field", offsetof(database::remote_field_map_type, corona_field));
				create_scalar_property<database::string_box>(remote_parameters_ti, pvalue::pvalue_types::string_value, "remote_field", "remote_field", offsetof(database::remote_field_map_type, remote_field));

				sql_fields_ti = create_typeinfo(member_type_name, "sql", "sql", 20);
				create_scalar_property<database::int32_box>(sql_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_sql_field_request, name.field_id));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_sql_field_request, name.name));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_sql_field_request, name.description));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "login_type", "login_type", offsetof(database::put_sql_field_request, options.login_type_name));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "username", "username", offsetof(database::put_sql_field_request, options.username));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "password", "password", offsetof(database::put_sql_field_request, options.password));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "parameter_field", "parameter_field", offsetof(database::put_sql_field_request, options.parameter_field));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "result_field", "result_field", offsetof(database::put_sql_field_request, options.result_field));
				create_scalar_property<database::string_box>(sql_fields_ti, pvalue::pvalue_types::string_value, "query", "query", offsetof(database::put_sql_field_request, options.query));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_fields>(sql_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_sql_field_request, options.parameters));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_parameter_fields >(sql_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_sql_field_request, options.fields));

				file_fields_ti = create_typeinfo(member_type_name, "file", "file", 20);
				create_scalar_property<database::int32_box>(file_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_file_field_request, name.field_id));
				create_scalar_property<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_file_field_request, name.name));
				create_scalar_property<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_file_field_request, name.description));
				create_scalar_property<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "parameter_field", "parameter_field", offsetof(database::put_file_field_request, options.parameter_field));
				create_scalar_property<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "result_field", "result_field", offsetof(database::put_file_field_request, options.result_field));
				create_scalar_property<database::string_box>(file_fields_ti, pvalue::pvalue_types::string_value, "file", "file", offsetof(database::put_file_field_request, options.file_path));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_fields>(file_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_file_field_request, options.parameters));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_parameter_fields >(file_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_file_field_request, options.fields));

				http_fields_ti = create_typeinfo(member_type_name, "http", "http", 20);
				create_scalar_property<database::int32_box>(http_fields_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_http_field_request, name.field_id));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_http_field_request, name.name));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_http_field_request, name.description));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_type", "login_type", offsetof(database::put_http_field_request, options.login_type_name));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_url", "login_url", offsetof(database::put_http_field_request, options.login_url));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "login_method", "login_method", offsetof(database::put_http_field_request, options.login_method));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "username", "username", offsetof(database::put_http_field_request, options.username));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "password", "password", offsetof(database::put_http_field_request, options.password));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "data_url", "data_url", offsetof(database::put_http_field_request, options.data_url));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "data_method", "data_method", offsetof(database::put_http_field_request, options.data_method));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "parameter_field", "parameter_field", offsetof(database::put_http_field_request, options.parameter_field));
				create_scalar_property<database::string_box>(http_fields_ti, pvalue::pvalue_types::string_value, "result_field", "result_field", offsetof(database::put_http_field_request, options.result_field));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_fields>(http_fields_ti, remote_parameters_ti, "parameters", "parameters", offsetof(database::put_http_field_request, options.parameters));
				create_object_iarray_property<database::remote_field_map_type, database::max_remote_parameter_fields >(http_fields_ti, remote_parameters_ti, "fields", "fields", offsetof(database::put_http_field_request, options.fields));

				put_class_fields_ti = create_typeinfo(member_type_name, "class_fields", "class_fields", 20);
				create_scalar_property<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "field_name", "field_name", offsetof(database::member_field, field_name));
				create_scalar_property<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "member_type", "member_type", offsetof(database::member_field, membership_type_name));
				create_scalar_property<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_x", "dim_x", offsetof(database::member_field, dimensions.x));
				create_scalar_property<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_y", "dim_y", offsetof(database::member_field, dimensions.y));
				create_scalar_property<database::string_box>(put_class_fields_ti, pvalue::pvalue_types::string_value, "dim_z", "dim_z", offsetof(database::member_field, dimensions.z));

				put_classes_ti = create_typeinfo(member_type_name, "class", "class", 20);
				create_scalar_property<database::int32_box>(put_classes_ti, pvalue::pvalue_types::double_value, "id", "id", offsetof(database::put_class_request, class_id));
				create_scalar_property<database::string_box>(put_classes_ti, pvalue::pvalue_types::string_value, "name", "name", offsetof(database::put_class_request, class_name));
				create_scalar_property<database::string_box>(put_classes_ti, pvalue::pvalue_types::string_value, "description", "description", offsetof(database::put_class_request, class_description));
				create_object_iarray_property<database::member_field, database::max_class_fields >(put_classes_ti, put_class_fields_ti, "fields", "fields", offsetof(database::put_class_request, member_fields));

				color_ti = create_typeinfo(member_type_name, "color", "color", 20);
				create_scalar_property<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "red", "red", offsetof(database::color, red));
				create_scalar_property<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "green", "green", offsetof(database::color, green));
				create_scalar_property<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "blue", "blue", offsetof(database::color, blue));
				create_scalar_property<database::color_box>(color_ti, pvalue::pvalue_types::double_value, "alpha", "alpha", offsetof(database::color, alpha));

				point_ti = create_typeinfo(member_type_name, "point", "point", 20);
				create_scalar_property<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::point, x));
				create_scalar_property<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::point, y));
				create_scalar_property<database::point_box>(point_ti, pvalue::pvalue_types::double_value, "z", "z", offsetof(database::point, z));

				rectangle_ti = create_typeinfo(member_type_name, "rectangle", "rectangle", 20);
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::rectangle, corner.x));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::rectangle, corner.y));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "w", "w", offsetof(database::rectangle, size.x));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "h", "h", offsetof(database::rectangle, size.y));

				image_ti = create_typeinfo(member_type_name, "image", "image", 20);
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "x", "x", offsetof(database::rectangle, corner.x));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "y", "y", offsetof(database::rectangle, corner.y));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "w", "w", offsetof(database::rectangle, size.x));
				create_scalar_property<database::rectangle_box>(point_ti, pvalue::pvalue_types::double_value, "h", "h", offsetof(database::rectangle, size.y));

				wave_ti = create_typeinfo(member_type_name, "wave", "wave", 20);
				create_scalar_property<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "start_seconds", "start_seconds", offsetof(database::wave_instance, start_seconds));
				create_scalar_property<database::wave_box>(wave_ti, pvalue::pvalue_types::double_value, "stop_seconds", "stop_seconds", offsetof(database::wave_instance, stop_seconds));

				midi_ti = create_typeinfo(member_type_name, "midi", "midi", 20);
				create_scalar_property<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "start_seconds", "start_seconds", offsetof(database::midi_instance, start_seconds));
				create_scalar_property<database::wave_box>(midi_ti, pvalue::pvalue_types::double_value, "stop_seconds", "stop_seconds", offsetof(database::midi_instance, stop_seconds));
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
				return error_messages.size() == 0;
			}

			private:

			typeinfo* get_type_info(database::jschema& schema, database::object_name class_name)
			{
				auto citer = class_types_by_name[class_name];
				if (citer == std::end(class_types_by_name)) 
				{
					auto class_id = schema.find_class(class_name);
					if (class_id != database::null_row) 
					{
						auto class_def = schema.get_class(class_id);
						typeinfo *new_class_ti = create_typeinfo(member_type_name, class_name.c_str(), class_name.c_str(), class_def.size() + 4);
						for (database::row_id_type id = 0; id < class_def.size(); id++)
						{
							auto& fld_ref = class_def.child(id);
							auto& fld = schema.get_field(fld_ref.field_id);
							switch (fld.type_id) {
							case database::jtype::type_datetime:
								create_scalar_property<database::basic_time_box>(new_class_ti, database::pvalue::pvalue_types::time_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_float32:
								create_scalar_property<database::float_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_float64:
								create_scalar_property<database::double_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_int64:
								create_scalar_property<database::int64_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_int32:
								create_scalar_property<database::int32_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_int16:
								create_scalar_property<database::int16_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_int8:
								create_scalar_property<database::int8_box>(new_class_ti, database::pvalue::pvalue_types::double_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_color:
								create_object_property(new_class_ti, color_ti, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_point:
								create_object_property(new_class_ti, point_ti, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_rectangle:
								create_object_property(new_class_ti, rectangle_ti, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_string:
								create_scalar_property<database::string_box>(new_class_ti, database::pvalue::pvalue_types::string_value, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_wave:
								create_object_property(new_class_ti, wave_ti, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_image:
								create_object_property(new_class_ti, image_ti, fld.name.c_str(), fld.name.c_str(), fld_ref.offset);
								break;
							case database::jtype::type_list:
								break;
							case database::jtype::type_object:
								break;
							case database::jtype::type_collection_id:
								break;
							case database::jtype::type_http:
								break;
							case database::jtype::type_file:
								break;
							case database::jtype::type_sql:
								break;
							}
						}
					}
					else 
					{
						put_error(errors::class_not_defined, class_name.c_str());
						return nullptr;
					}
				}
				else 
				{
					return citer.get_value();
				}
			}

			virtual char* place(typeinfo* ti)
			{
				char* t = nullptr;
				database::row_range rr;

				if (ti == string_fields_ti)
				{
					auto& asf = put_string_fields.create(1, rr);
					asf.name.field_id = database::null_row;
					asf.name.type_id = database::jtype::type_string;
					t = (char*) &asf;
				}
				else if (ti == int8_fields_ti)
				{
					auto& ai8f = put_integer_fields.create(1, rr);
					ai8f.name.field_id = database::null_row;
					ai8f.name.type_id = database::jtype::type_int8;
					t = (char*)&ai8f;
				}
				else if (ti == int16_fields_ti)
				{
					auto& ai16f = put_integer_fields.create(1, rr);
					ai16f.name.field_id = database::null_row;
					ai16f.name.type_id = database::jtype::type_int16;
					t = (char*)&ai16f;
				}
				else if (ti == int32_fields_ti)
				{
					auto& ai32f = put_integer_fields.create(1, rr);
					ai32f.name.field_id = database::null_row;
					ai32f.name.type_id = database::jtype::type_int32;
					t = (char*)&ai32f;
				}
				else if (ti == int64_fields_ti)
				{
					auto& ai64f = put_integer_fields.create(1, rr);
					ai64f.name.field_id = database::null_row;
					ai64f.name.type_id = database::jtype::type_int64;
					t = (char*)&ai64f;
				}
				else if (ti == float_fields_ti)
				{
					auto& af32f = put_double_fields.create(1, rr);
					af32f.name.field_id = database::null_row;
					af32f.name.type_id = database::jtype::type_float32;
					t = (char*)&af32f;
				}
				else if (ti == double_fields_ti)
				{
					auto& af64f = put_double_fields.create(1, rr);
					af64f.name.field_id = database::null_row;
					af64f.name.type_id = database::jtype::type_float64;
					t = (char*)&af64f;
				}
				else if (ti == time_fields_ti)
				{
					auto& at64f = put_time_fields.create(1, rr);
					at64f.name.field_id = database::null_row;
					at64f.name.type_id = database::jtype::type_datetime;
					t = (char*)&at64f;
				}
				else if (ti == object_fields_ti)
				{
					auto& aof = put_object_fields.create(1, rr);
					aof.name.field_id = database::null_row;
					aof.name.type_id = database::jtype::type_object;
					t = (char*)&aof;
				}
				else if (ti == list_fields_ti)
				{
					auto& aof = put_list_fields.create(1, rr);
					aof.name.field_id = database::null_row;
					aof.name.type_id = database::jtype::type_list;
					t = (char*)&aof;
				}
				else if (ti == query_fields_ti)
				{
					auto& aqf = put_query_fields.create(1, rr);
					aqf.name.field_id = database::null_row;
					aqf.name.type_id = database::jtype::type_query;
					t = (char*)&aqf;
				}
				else if (ti == point_fields_ti)
				{
					auto& apf = put_point_fields.create(1, rr);
					apf.name.field_id = database::null_row;
					apf.name.type_id = database::jtype::type_point;
					t = (char*)&apf;
				}
				else if (ti == rectangle_fields_ti)
				{
					auto& arf = put_rectangle_fields.create(1, rr);
					arf.name.field_id = database::null_row;
					arf.name.type_id = database::jtype::type_rectangle;
					t = (char*)&arf;
				}
				else if (ti == color_fields_ti)
				{
					auto& acf = put_color_fields.create(1, rr);
					acf.name.field_id = database::null_row;
					acf.name.type_id = database::jtype::type_color;
					t = (char*)&acf;
				}
				else if (ti == image_fields_ti)
				{
					auto& aimf = put_image_fields.create(1, rr);
					aimf.name.field_id = database::null_row;
					aimf.name.type_id = database::jtype::type_image;
					t = (char*)&aimf;
				}
				else if (ti == wave_fields_ti)
				{
					auto& awf = put_wave_fields.create(1, rr);
					awf.name.field_id = database::null_row;
					awf.name.type_id = database::jtype::type_wave;
					t = (char*)&awf;
				}
				else if (ti == midi_fields_ti)
				{
					auto& amf = put_midi_fields.create(1, rr);
					amf.name.field_id = database::null_row;
					amf.name.type_id = database::jtype::type_midi;
					t = (char*)&amf;
				}
				else if (ti == put_classes_ti)
				{
					auto& amf = put_classes.create(1, rr);
					amf.class_id = database::null_row;
					t = (char*)&amf;
				}
				else if (ti == sql_fields_ti)
				{
					auto& sqf = put_sql_fields.create(1, rr);
					sqf.name.field_id = database::null_row;
					sqf.name.type_id = database::jtype::type_sql;
					t = (char*)&sqf;
				}
				else if (ti == file_fields_ti)
				{
					auto& flf = put_file_fields.create(1, rr);
					flf.name.field_id = database::null_row;
					flf.name.type_id = database::jtype::type_file;
					t = (char*)&flf;
				}
				else if (ti == http_fields_ti)
				{
					auto& htf = put_http_fields.create(1, rr);
					htf.name.field_id = database::null_row;
					htf.name.type_id = database::jtype::type_http;
					t = (char*)&htf;
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

			bool is_field_mapping_valid(database::jschema* pschema, database::row_id_type class_row, database::remote_field_map_type& remote)
			{
				if (class_row == database::null_row)
				{
					return false;
				}

				database::jclass the_class = pschema->get_class(class_row);

				database::remote_field_map_type* premote = &remote;

				bool valid = false;

				for (int i = 0; i < the_class.size(); i++) 
				{
					auto &c = the_class.child(i);
					auto &f = pschema->get_field(c.field_id);
					if (remote.corona_field == f.name) {
						valid = true;
						remote.corona_field_id = c.field_id;
						break;
					}
				}

				if (!valid) 
				{
					put_error(errors::invalid_mapping, remote.corona_field.c_str());
				}

				return valid;
			}

			template <int number> 
			bool check_field_mapping_valid(database::jschema& schema, database::object_name& class_name, database::iarray<database::remote_field_map_type, number>& remotes)
			{
				database::jschema* pschema = &schema;
				database::row_id_type class_row = schema.find_class(class_name);
				if (class_row == null_row) 
				{
					auto class_name_iter = classes_by_name[class_name];
					if (class_name_iter == std::end(classes_by_name))
					{
						put_error(errors::class_not_defined, class_name.c_str());
					}
					else
					{
						auto def_row_id = class_name_iter.get_value();
						auto& class_def = put_classes[def_row_id];
						class_row = put_class(schema, class_def);
					}
				}
				bool valid = std::all_of(remotes.begin(), remotes.end(), [pschema, class_row](database::remote_field_map_type& src) {
					return is_field_mapping_valid(pschema, class_row, r.item);
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

			void put_field(database::jschema& schema, database::put_sql_field_request& fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameter_field, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.result_field, fld.options.fields);
				schema.put_sql_remote_field(fld);
			}

			void put_field(database::jschema& schema, database::put_file_field_request& fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameter_field, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.result_field, fld.options.fields);
				schema.put_file_remote_field(fld);
			}

			void put_field(database::jschema& schema, database::put_http_field_request& fld)
			{
				bool validParams = check_field_mapping_valid(schema, fld.options.parameter_field, fld.options.parameters);
				bool validFields = check_field_mapping_valid(schema, fld.options.result_field, fld.options.fields);
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
				bool valid = true;

				database::row_id_type new_field = database::null_row;

				int filterSize = aorf.options.filter.size();

				for (int i = 0; i < filterSize; i++)
				{
					auto& filter = aorf.options.filter[i];
					filter.error_message = nullptr;

					if (filter.comparison_name == "$eq") {
						filter.comparison = database::filter_comparison_types::eq;
					}
					else if (filter.comparison_name == "$gte") {
						filter.comparison = database::filter_comparison_types::gteq;
					}
					else if (filter.comparison_name == "$lte") {
						filter.comparison = database::filter_comparison_types::lseq;
					}
					else if (filter.comparison_name == "$gt") {
						filter.comparison = database::filter_comparison_types::gt;
					}
					else if (filter.comparison_name == "$lt") {
						filter.comparison = database::filter_comparison_types::ls;
					}
					else if (filter.comparison_name == "$inside") {
						filter.comparison = database::filter_comparison_types::distance;
					}
					else if (filter.comparison_name == "$in") {
						filter.comparison = database::filter_comparison_types::inlist;
					}
					else if (filter.comparison_name == "$contains") {
						filter.comparison = database::filter_comparison_types::contains;
					}
					else
					{
						put_error(errors::invalid_comparison, filter.comparison_name.c_str(), 0);
						valid = false;
					}

					auto parameter_field_id = schema.find_field(filter.parameter_field_name);
					if (parameter_field_id != database::null_row)
					{
						filter.parameter_field_id = parameter_field_id;
					}
					else
					{
						put_error(errors::field_not_defined, filter.parameter_field_name.c_str(), 0);
						valid = false;
					}

					auto target_field_id = schema.find_field(filter.target_field_name);
					if (target_field_id != database::null_row)
					{
						filter.target_field_id = target_field_id;
					}
					else
					{
						put_error(errors::field_not_defined, filter.target_field_name.c_str(), 0);
						valid = false;
					}
				}

				if (valid) 
				{
					schema.put_query_field(aorf);
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

			database::row_id_type put_class(database::jschema& schema, database::put_class_request& aorf)
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

			bool put_array(database::jschema& _schema, database::jslice& slice, parray *src_array)
			{

			}

			bool put_value(database::jschema& _schema, database::jslice& slice, pvalue* src_pobj)
			{

			}

			bool put_slice(database::jschema& _schema, database::jslice& slice, pobject* _obj)
			{
				for (auto m = _obj->first; m != nullptr; m = m->next)
				{
					switch (m->value->pvalue_type) 
					{
					case database::pvalue::pvalue_types::array_value:
						break;
					case database::pvalue::pvalue_types::object_value:
						break;
					case database::pvalue::pvalue_types::double_value:
						break;
					case database::pvalue::pvalue_types::string_value:
						break;
					case database::pvalue::pvalue_types::time_value:
						break;
					}
				}
			}

			bool put_object(database::jschema& _schema, database::jcollection& _collection, pobject *_obj)
			{
				auto type_member = _obj->get_member(member_type_name);
				if (type_member) {
					database::row_id_type class_id = _schema.find_class(type_member->name);
					if (class_id != database::null_row) 
					{
						database::jarray new_object = _collection.create_object(class_id);
						if (new_object.get_bytes()) {
							database::jslice slice = new_object.get_slice(0);
							return put_slice(_schema, slice, _obj);
						}
					}
					else 
					{
						put_error(errors::class_not_defined, _obj);
						return false;
					}
				}
				else 
				{
					put_error(errors::field_not_defined, _obj);
					return false;
				}
			}
		};
	}
}

