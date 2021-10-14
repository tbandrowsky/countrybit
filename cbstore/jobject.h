#pragma once

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

namespace countrybit
{
	namespace database
	{
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

		struct string_properties_type
		{
			int length;
			jstring		validation_pattern;
			jstring		validation_message;
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
			time_t			minimum_time_t;
			time_t			maximum_time_t;
		};

		struct dimensions_type
		{
			int x, y, z;
		};

		struct object_properties_type 
		{
			dimensions_type		dim;
			row_id_type			class_id;
			size_t				class_size_bytes;
			size_t				total_size_bytes;
		};

		using object_name = istring<32>;
		using object_description = istring<250>;

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			size_t					size_bytes;

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

		class jschema
		{

		protected:


			table<jfield> fields;
			parent_child_table<jclass_header, jclass_field> classes;
			sorted_index<object_name, row_id_type> classes_by_name;
			sorted_index<object_name, row_id_type> fields_by_name;

		public:

			jschema();
			~jschema();

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
				size_t _size_bytes)
			{
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

			class create_class_request  {
			public:
				std::string class_name;
				std::string class_description;
				std::vector<row_id_type> field_ids;
				std::vector<create_object_field_request> child_classes;
			};

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
				size_t size = pcr.parent().class_size_bytes;
				request.class_size_bytes = size;
				request.total_size_bytes = request.dim.x * request.dim.y * request.dim.z * size;
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

			row_id_type find_class(object_name& class_name)
			{
				auto citer = classes_by_name[class_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			row_id_type find_field(object_name& field_name)
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

		};

		class jarray;

		class jslice
		{
			jschema& schema;
			char* bytes;
			jclass& the_class;

			template <typename T>
			T get_boxed(int field_idx, jtype jt)
			{
				jclass_field& jcf = the_class.child(field_idx);
				jfield jf = schema.get_field(jcf.field_id);
				if (jf.type_id != jt) {
					throw std::invalid_argument("Invalid field type " + std::to_string(jt) + " for field idx " + std::to_string(field_idx));
				}
				T b = &bytes[jcf.offset];
				return b;
			}

		public:

			jslice(jclass& _the_class, jschema& _schema, char* _bytes);
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
			jschema& schema;
			row_id_type class_field_id;
			char* bytes;

		public:

			jarray(jschema& _schema, row_id_type& _class_field_id, char* _bytes);
			jslice get_slice(int x, int y = 0, int z = 0);
			jslice get_slice(dimensions_type dims);
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
		};

		class jcollection
		{

			jschema& schema;
			collection_id_type collection_id;
			parent_child_table<jobject_header, char> objects;

			jcollection(jschema& _schema, collection_id_type _collection_id, parent_child_table<jobject_header, char> _objects) :
				schema( _schema ),
				collection_id( _collection_id ),
				objects( _objects )
			{

			}

			jcollection(jcollection& _src) :
				schema(_src.schema),
				collection_id(_src.collection_id),
				objects(_src.objects)
			{

			}

		public:

			jarray construct(row_id_type _class_field_id)
			{
				auto myclassfield = schema.get_field(_class_field_id);
				auto myclass = schema.get_class(myclassfield.object_properties.class_id);
				auto bytes_to_allocate = myclass.parent().class_size_bytes;
				auto new_object = objects.create(bytes_to_allocate);
				new_object.parent().oid.collection_id = collection_id;
				new_object.parent().oid.row_id = new_object.row_id();
				return jarray(schema, _class_field_id, new_object.pchild());
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			static row_id_type create(B* _b, jschema& _schema, collection_id_type _collection_id, int _number_of_objects, int *_class_field_ids)
			{

				if (!_class_field_ids)
				{
					throw std::invalid_argument("cannot create collection with 0 class size");
				}

				int max_size = 0;
				while (*_class_field_ids) 
				{
					auto myclassfield = _schema.get_field(*_class_field_ids);
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
				jcm.table_id = parent_child_table<jobject_header, char>::create(_b, _number_of_objects, max_size * _number_of_objects);
				row_id_type jcm_row = _b->pack(jcm);
				return jcm_row;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			static jcollection get(B* _b, jschema& _schema, row_id_type _location)
			{

				jcollection_map jcm;
				jcm = _b->unpack<jcollection_map>(_location);
				auto pct = parent_child_table<jobject_header, char>::get(_b, jcm.table_id);
				return jcollection( _schema, jcm.collection_id, pct );
			}
		};
	}
}
