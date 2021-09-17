#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <stdexcept>
#include "table.h"
#include "jstring.h"
#include "sorted_index.h"

namespace countrybit
{
	namespace database
	{
		class constants 
		{
		public:
			static const int max_fields = 2048;
			static const int max_classes = 256;
			static const int max_class_fields = 256;
		};

		enum jtype
		{
			type_unknown = 0,

			type_int8 = 1,
			type_int16 = 2,
			type_int32 = 3,
			type_int64 = 4,

			type_float32 = 5,
			type_float64 = 6,

			type_datetime = 7,
			type_string = 8,

			type_object = 9,
			type_object_id = 10
		};

		struct store_id_type
		{
			unsigned long  Data1;
			unsigned short Data2;
			unsigned short Data3;
			unsigned char  Data4[8];
		};

		struct object_id_type
		{
			store_id_type store_id;
			row_id_type	  row_id;
		};

		struct string_properties_type
		{
			int length;
			jstring<64>		validation_pattern;
			jstring<128>	validation_message;
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

		using object_name = jstring<64>;
		using object_description = jstring<256>;

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			uint32_t				size_bytes;

			object_name				name;
			object_description		description;

			string_properties_type	string_properties;
			int_properties_type		int_properties;
			double_properties_type	double_properties;
			time_properties_type	time_properties;
			object_properties_type  object_properties;
		};

		// a store id is in fact, a guid

		struct jobject_data
		{
			object_id_type			id;
			object_properties_type* props;
			char*					bytes;
		};

		template <int max_objects, row_id_type bytes_per_object>
		class jstore
		{
			parent_child_table<object_properties_type, char, max_objects, bytes_per_object> data;
			store_id_type store_id;

		public:

			jstore();
			~jstore();

			jobject_data create_object( object_properties_type& _object_properties )
			{
				jobject_data jo;

				auto pt = data.create(_object_properties.total_size_bytes);
				jp.props = pt.pparent();
				jp.bytes = pt.pchild();
				jp.id.store_id = store_id;
				jp.id.row_id = pt.row_id();
				return jp;
			}

			jobject_data get_object( object_id_type _id )
			{
				auto pt = data.get(_id.row_id);
				jp.props = pt.pparent();
				jp.bytes = pt.pchild();
				jp.id.store_id = store_id;
				jp.id.row_id = pt.row_id();
				return jp;
			}

			size_t size() const
			{
				return sizeof(*this);
			}
		};

		class jclass_header
		{
		public:
			row_id_type						class_id;
			object_name						name;
			object_description				description;
			uint64_t						class_size_bytes;
		};

		class jfield_instance
		{
		public:
			row_id_type				field_id;
			uint64_t				offset;
		};

		class jclass
		{
			parent_child_holder<jclass_header, jfield_instance> source;
			jschema* schema;

		public:

			jclass();
			~jclass();

			friend class jschema;
		};

		class jschema
		{
		protected:

			table<jfield, constants::max_fields> fields;
			parent_child_table<jclass_header, jfield_instance, constants::max_classes, constants::max_class_fields> classes;
			sorted_index<object_name, row_id_type, constants::max_classes> classes_by_name;
			sorted_index<object_name, row_id_type, constants::max_fields> fields_by_name;

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
				int _size_bytes)
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
				return create_field(request.field_id, type_string, request.name, request.description, request, {}, {}, {}, {}, request.length);
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

			row_id_type find_field(object_name& class_name)
			{
				auto citer = classes_by_name[class_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			jclass get_class_details(row_id_type class_id)
			{
				jclass jc;

				jc.schema = this;
				jc.source = classes[class_id];

				return jc;
			}

		};

		/*
		
		The overall schema that we are implementing

		cb_store_app :
		{
			fields : [ {}, ...  ],
			classes: [ {}, ... ],
			collections: [ ],
		}
		
		*/

		class jcollection 
		{
		public:
			row_id_type		root_class;
		};

		class jbase_db
		{
			jschema schema;

		public:

			jbase_db()
			{
			}
		};
	}
}
