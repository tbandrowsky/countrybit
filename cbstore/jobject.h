#pragma once

#include <cstdint>
#include <ctime>
#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <stdexcept>
#include "constants.h"
#include "store_box.h"
#include "table.h"
#include "jstring.h"
#include "sorted_index.h"
#include <ctime>

namespace countrybit
{
	namespace database
	{

		const static int

			field_none = 0,
			// string units
			field_full_name = 1,
			field_first_name = 2,
			field_last_name = 4,
			field_middle_name = 5,
			field_ssn = 6,
			field_email = 7,
			field_title = 8,
			field_street = 9,
			field_substreet = 10,
			field_city = 11,
			field_state = 12,
			field_postal = 13,
			field_country_name = 14,
			field_country_code = 15,
			field_longname = 16,
			field_shortname = 17,
			field_unit = 18,
			field_code_symbol = 19,
			field_code_operator = 20,
			field_windows_path = 21,
			field_linux_path = 22,
			field_url = 23,
			field_user_name = 24,
			field_password = 25,
			field_institution_name = 26,
			field_document_title = 27,
			field_section_title = 28,
			field_block_title = 29,
			field_caption = 30,
			field_paragraph = 31,
			field_base64_block = 32,

			// date units
			field_birthday = 33,
			field_scheduled = 34,

			// int units
			field_count = 35,

			// float units
			field_quantity = 36,
			field_degrees = 37,
			field_radians = 38,
			field_meters = 39,
			field_feet = 40,
			field_grams = 41,
			field_ounces = 42,
			field_seconds = 43,
			field_minutes = 44,
			field_hours = 45,
			field_amperes = 46,
			field_kelvin = 47,
			field_mole = 42,
			field_candela = 43,
			field_latitude = 44,
			field_longitude = 45,
			field_pounds = 46,
			field_kilograms = 47,

			// currencies
			field_argentina_peso = 48,
			field_bahama_dollar = 49,
			field_bermuda_dollar = 50,
			field_belize_dollar = 51,
			field_bolivia_boliviano = 52,
			field_brazil_real = 53,
			field_canada_dollar = 54,
			field_cayman_islands_dollars = 55,
			field_chile_peso = 56,
			field_columbia_peso = 57,
			field_costa_rica_colon = 58,
			field_cuba_peso = 59,
			field_cuba_convert_peso = 61,
			field_dominican_peso = 62,
			field_east_carrbean_dollar = 63,
			field_falkland_pound = 64,
			field_guatemala_queztel = 65,
			field_guyana_dollar = 66,
			field_haiti_guorde = 67,
			field_honduras_lempira = 68,
			field_jamaica_dollar = 69,
			field_mexico_peso = 70,
			field_nicaragua_cordoba = 71,
			field_panama_balboa = 72,
			field_paraguan_guarani = 73,
			field_peru_sol = 74,
			field_surinam_dollar = 75,
			field_trinidad_tobogo_dollar = 76,
			field_united_states_dollar = 77,
			field_venezuela_bolivar = 78,

			field_united_kingdom_pounds = 79,
			field_euros = 80,
			field_india_rupees = 81,

			field_mime_type = 83;

		struct collection_id_type
		{
			unsigned long  Data1;
			unsigned short Data2;
			unsigned short Data3;
			unsigned char  Data4[8];
		};

		struct object_id_type
		{
			collection_id_type collection_id;
			row_id_type	  row_id;
		};

		using object_name = istring<32>;
		using object_description = istring<250>;
		using string_validation_pattern = istring<250>;
		using string_validation_message = istring<250>;

		struct string_properties_type
		{
			int length;
			string_validation_pattern	validation_pattern;
			string_validation_message	validation_message;
		};

		struct int_properties_type 
		{
			int64_t			minimum_int;
			int64_t			maximum_int;
		};

		struct double_properties_type 
		{
			double			minimum_number;
			double			maximum_number;
		};

		struct time_properties_type 
		{
			int64_t			minimum_time_t;
			int64_t			maximum_time_t;
		};

		struct dimensions_type
		{
			int x, y, z;
		};

		int compare(const dimensions_type& a, const dimensions_type& b);
		int operator<(const dimensions_type& a, const dimensions_type& b);
		int operator>(const dimensions_type& a, const dimensions_type& b);
		int operator>=(const dimensions_type& a, const dimensions_type& b);
		int operator<=(const dimensions_type& a, const dimensions_type& b);
		int operator==(const dimensions_type& a, const dimensions_type& b);
		int operator!=(const dimensions_type& a, const dimensions_type& b);

		struct object_properties_type 
		{
			dimensions_type		dim;
			row_id_type			class_id;
			int64_t				class_size_bytes;
			int64_t				total_size_bytes;
		};

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			int64_t					size_bytes;

			object_name				name;
			object_description		description;

			string_properties_type	string_properties;
			int_properties_type		int_properties;
			double_properties_type	double_properties;
			time_properties_type	time_properties;
			object_properties_type  object_properties;

		};

		// a store id is in fact, a guid

		class jclass_header
		{
		public:
			row_id_type						class_id;
			object_name						name;
			object_description				description;
			uint64_t						class_size_bytes;
		};

		class jclass_field
		{
		public:
			row_id_type				field_id;
			uint64_t				offset;
		};

		using jclass_table = parent_child_table<jclass_header, jclass_field>;
		using jclass = parent_child_holder<jclass_header, jclass_field>;

		struct jschema_map
		{
			row_id_type fields_table_id;
			row_id_type classes_table_id;
			row_id_type classes_by_name_id;
			row_id_type fields_by_name_id;
		};

		class jcollection_map
		{
		public:
			collection_id_type collection_id;
			row_id_type table_id;
		};

		class jobject_header
		{
		public:
			object_id_type oid;
			row_id_type class_field_id;
		};

		class jschema;
		class jarray;

		class jslice
		{
			jschema* schema;
			row_id_type class_field_id;
			char* bytes;
			dimensions_type dim;
			jfield* class_field;
			jclass the_class;

			size_t get_offset(jtype field_type_id, int field_idx);

			template <typename T> T get_boxed(jtype jt, int field_idx)
			{
				size_t offset = get_offset(jt, field_idx);
				T b = &bytes[offset];
				return b;
			}

		public:

			jslice();
			jslice(jschema* _schema, row_id_type _class_field_id, char* _bytes, dimensions_type _dim);

			int8_box get_int8(int field_idx);
			int16_box get_int16(int field_idx);
			int32_box get_int32(int field_idx);
			int64_box get_int64(int field_idx);
			float_box get_float(int field_idx);
			double_box get_double(int field_idx);
			time_box get_time(int field_idx);
			jstring get_string(int field_idx);
			jarray get_object(int field_idx);
			int size();
		};

		class jarray
		{
			jschema* schema;
			row_id_type class_field_id;
			char* bytes;

		public:

			jarray() : schema(nullptr), class_field_id(null_row), bytes(nullptr)
			{
				;
			}

			jarray(jschema* _schema, row_id_type _class_field_id, char* _bytes) : schema( _schema ), class_field_id( _class_field_id ), bytes( _bytes )
			{

			}

			dimensions_type dimensions();

			jslice get_slice(int x, int y = 0, int z = 0);
			jslice get_slice(dimensions_type dims);

			class iterator
			{
				jarray* base;
				dimensions_type current;
				dimensions_type maxd;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = jslice;
				using pointer = jslice*;  // or also value_type*
				using reference = jslice&;  // or also value_type&

				iterator(jarray* _base, dimensions_type _current) :
					base(_base),
					current(_current)
				{
					maxd = base->dimensions();
				}

				iterator() : base(nullptr), current({ 0, 0, 0 }), maxd( { 0, 0, 0 })
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jslice operator *()
				{
					return base->get_slice(current);
				}

				inline jslice operator->()
				{
					return base->get_slice(current);
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end()
				{
					auto temp = base->dimensions();
					return iterator(base, temp);
				}

				inline iterator operator++()
				{
					current.x++;
					if (current.x >= maxd.x) {
						current.y++;
						current.x = 0;
						if (current.y >= maxd.y) {
							current.z++;
							current.y = 0;
						}
					}
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

		};

		class jcollection
		{

			jschema* schema;
			collection_id_type collection_id;
			parent_child_table<jobject_header, char> objects;

		public:

			jcollection() : schema( nullptr )
			{
				;
			}

			jcollection(jschema* _schema, collection_id_type _collection_id, parent_child_table<jobject_header, char>& _objects) :
				schema(_schema),
				collection_id(_collection_id),
				objects(_objects)
			{
				;
			}

			jarray create_object(row_id_type _class_field_id);

			jarray get_object(row_id_type _object_id)
			{
				auto new_object = objects.get(_object_id);
				return jarray(schema, new_object.parent().class_field_id, new_object.pchild());
			}

			object_id_type get_object_id(row_id_type _object_id)
			{
				auto new_object = objects.get(_object_id);
				return new_object.parent().oid;
			}

			int size()
			{
				return objects.size();
			}

			class iterator
			{
				jcollection* base;
				row_id_type current;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = jarray;
				using pointer = jarray*;  // or also value_type*
				using reference = jarray&;  // or also value_type&

				iterator(jcollection* _base, row_id_type _current) :
					base(_base),
					current(_current)
				{

				}

				iterator() : base(nullptr), current(null_row)
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jarray operator *()
				{
					return base->get_object(current);
				}

				inline jarray operator->()
				{
					return base->get_object(current);
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end() const
				{
					return iterator(base, null_row);
				}

				inline iterator operator++()
				{
					current = current++;
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			inline iterator begin()
			{
				return iterator(this, 0);
			}

			inline iterator end()
			{
				return iterator(this, null_row);
			}

		};

		class jschema
		{

		protected:

			using field_store_type = table<jfield>;
			using class_store_type = parent_child_table<jclass_header, jclass_field>;
			using class_index_type = sorted_index<object_name, row_id_type>;
			using field_index_type = sorted_index<object_name, row_id_type>;

			field_store_type fields;
			class_store_type classes;
			class_index_type classes_by_name;
			field_index_type fields_by_name;

		public:

			class create_field_request_base {
			public:
				row_id_type field_id;
				jtype		type_id;
				std::string name;
				std::string description;
			};

			class create_string_field_request : public create_field_request_base, public string_properties_type {
			public:

			};

			class create_integer_field_request : public create_field_request_base, public int_properties_type {
			public:

			};

			class create_double_field_request : public create_field_request_base, public double_properties_type {
			public:

			};

			class create_time_field_request : public create_field_request_base, public time_properties_type {
			public:

			};

			class create_object_field_request : public create_field_request_base, public object_properties_type {
			public:

			};

			class create_class_request {
			public:
				std::string class_name;
				std::string class_description;
				std::vector<row_id_type> field_ids;
				std::vector<create_object_field_request> child_classes;
			};

			jschema() = default;
			~jschema() = default;

			template <typename B>
			requires (box<B, jfield>
				&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static row_id_type reserve_schema(B *_b, int _num_classes, int _num_fields, int _num_class_fields)
			{
				jschema_map schema_map, *pschema_map;
				schema_map.classes_table_id = null_row;
				schema_map.fields_table_id = null_row;
				schema_map.classes_by_name_id = null_row;
				schema_map.fields_by_name_id = null_row;

				row_id_type rit = _b->pack(schema_map);
				pschema_map = _b->unpack<jschema_map>(rit);
				pschema_map->fields_table_id = field_store_type::reserve_table(_b, _num_fields);
				pschema_map->classes_by_name_id = field_index_type::reserve_sorted_index(_b, _num_classes);
				pschema_map->classes_table_id = class_store_type::reserve_table(_b, _num_classes, _num_class_fields );
				pschema_map->fields_by_name_id = class_index_type::reserve_sorted_index(_b, _num_fields);
				return rit;
			}

			template <typename B>
			requires (box<B, jfield>
				&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static jschema get_schema(B* _b, row_id_type _row)
			{
				jschema schema;
				jschema_map *pschema_map;
				pschema_map = _b->unpack<jschema_map>(_row);
				schema.classes = class_store_type::get_table(_b, pschema_map->classes_table_id);
				schema.fields = field_store_type::get_table(_b, pschema_map->fields_table_id);
				schema.classes_by_name = class_index_type::get_sorted_index(_b, pschema_map->classes_by_name_id);
				schema.fields_by_name = field_index_type::get_sorted_index(_b, pschema_map->fields_by_name_id);
				return schema;
			}

			template <typename B>
				requires (box<B, jfield>
			&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static jschema create_schema(B* _b, int _num_classes, int _num_fields, int _num_class_fields, row_id_type& _row)
			{
				_row = reserve_schema(_b, _num_classes, _num_fields, _num_class_fields);
				jschema schema = get_schema(_b, _row);
				return schema;
			}

			void create_standard_fields();

			row_id_type create_field()
			{
				return fields.create(1).start;
			}

			row_id_type create_field(
				row_id_type _field_id,
				jtype _field_type,
				std::string _name,
				std::string _description,
				string_properties_type _string_properties,
				int_properties_type _int_properties,
				double_properties_type _double_properties,
				time_properties_type _time_properties,
				object_properties_type _object_properties,
				int64_t _size_bytes)
			{
				if (_field_id == null_row) {
					_field_id = create_field();
				}

				auto& jf = fields[_field_id];

				jf.type_id = _field_type;
				jf.name = _name;
				jf.description = _description;
				jf.string_properties = _string_properties;
				jf.int_properties = _int_properties;
				jf.double_properties = _double_properties;
				jf.time_properties = _time_properties;
				jf.object_properties = _object_properties;
				jf.size_bytes = _size_bytes;

				fields_by_name.insert_or_assign(jf.name, _field_id);
				return _field_id;
			}

			row_id_type create_string_field(create_string_field_request request)
			{
				return create_field(request.field_id, type_string, request.name, request.description, request, {}, {}, {}, {}, request.length + sizeof(jstring));
			}

			row_id_type create_time_field(create_time_field_request request)
			{
				return create_field(request.field_id, type_datetime, request.name, request.description, {}, {}, {}, request, {}, sizeof(time_t));
			}

			row_id_type create_integer_field(create_integer_field_request request)
			{
				switch (request.type_id)
				{
					case jtype::type_int8:
						return create_field(request.field_id, type_int8, request.name, request.description, {}, request, {}, {}, {}, sizeof(int8_t));
					case jtype::type_int16:
						return create_field(request.field_id, type_int16, request.name, request.description, {}, request, {}, {}, {}, sizeof(int16_t));
					case jtype::type_int32:
						return create_field(request.field_id, type_int32, request.name, request.description, {}, request, {}, {}, {}, sizeof(int32_t));
					case jtype::type_int64:
						return create_field(request.field_id, type_int64, request.name, request.description, {}, request, {}, {}, {}, sizeof(int64_t));
					default:
						throw std::invalid_argument("Invalid integer type for field name:" + request.name);
				}
			}

			row_id_type create_double_field(create_double_field_request request)
			{
				switch (request.type_id)
				{
					case type_float32:
						return create_field(request.field_id, type_float32, request.name, request.description, {}, {}, request, {}, {}, sizeof(float));
					case type_float64:
						return create_field(request.field_id, type_float64, request.name, request.description, {}, {}, request, {}, {}, sizeof(double));
					default:
						throw std::invalid_argument("Invalid floating point type for field name:" + request.name);
				}
			}

			row_id_type create_object_field(create_object_field_request request)
			{
				auto pcr = classes[ request.class_id ];
				auto& p = pcr.parent();
				int64_t sizeb = pcr.parent().class_size_bytes;
				request.class_size_bytes = sizeb;
				request.total_size_bytes = request.dim.x * request.dim.y * request.dim.z * sizeb ;
				return create_field(request.field_id, type_object, request.name, request.description, {}, {}, {}, {}, request, request.total_size_bytes);
			}

			row_id_type create_class( create_class_request request )
			{
				auto sz = request.field_ids.size();
				auto pcr = classes.create(sz);
				auto& p = pcr.parent();

				p.class_id = pcr.row_id();
				p.name = request.class_name;
				p.description = request.class_description;
				p.class_size_bytes = 0;

				for (int i = 0; i < pcr.size(); i++)
				{
					auto fid = request.field_ids[i];
					auto& field = fields[fid];
					auto& ref = pcr.child(i);
					ref.field_id = fid;
					ref.offset = p.class_size_bytes;
					p.class_size_bytes += field.size_bytes;
				}

				return p.class_id;
			}

			row_id_type find_class(const object_name& class_name)
			{
				auto citer = classes_by_name[class_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			row_id_type find_field(const object_name& field_name)
			{
				auto citer = fields_by_name[field_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			jclass get_class(row_id_type class_id)
			{
				auto the_class = classes[class_id];
				return the_class;
			}

			jfield &get_field(row_id_type field_id)
			{
				auto& the_field = fields[field_id];
				return the_field;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			row_id_type reserve_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, int* _class_field_ids)
			{
				if (!_class_field_ids)
				{
					throw std::invalid_argument("cannot create collection with 0 class size");
				}

				int max_size = 0;
				while (*_class_field_ids != null_row)
				{
					auto myclassfield = get_field(*_class_field_ids);
					if (myclassfield.size_bytes > max_size) {
						max_size = myclassfield.size_bytes;
					}
					_class_field_ids++;
				}

				if (!max_size)
				{
					throw std::invalid_argument("cannot create collection with 0 class size");
				}

				jcollection_map jcm;
				jcm.collection_id = _collection_id;
				jcm.table_id = parent_child_table<jobject_header, char>::reserve_table(_b, _number_of_objects, max_size * _number_of_objects);
				row_id_type jcm_row = _b->pack(jcm);
				return jcm_row;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection get_collection(B* _b, row_id_type _location)
			{
				jcollection_map *jcm;
				jcm = _b->unpack<jcollection_map>(_location);
				auto obj = parent_child_table<jobject_header, char>::get_table(_b, jcm->table_id);
				jcollection collection(this, jcm->collection_id, obj);
				return collection;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection create_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, row_id_type* _class_field_ids)
			{
				auto reserved_id = reserve_collection(_b, _collection_id, _number_of_objects, _class_field_ids);
				jcollection tmp = get_collection(_b, reserved_id);
				return tmp;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection create_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, row_id_type _class_field_id)
			{
				row_id_type class_field_ids[2] = { _class_field_id, null_row };
				row_id_type reserved_id = reserve_collection(_b, _collection_id, _number_of_objects, class_field_ids);
				jcollection tmp = get_collection(_b, reserved_id);
				return tmp;
			}

		};

		class jarray;

		bool schema_tests();
		bool collection_tests();
		bool array_tests();
		bool slice_tests();

	}
}
