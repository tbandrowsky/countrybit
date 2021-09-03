#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <stdexcept>
#include "skip_list.h"
#include "table.h"

namespace countrybit
{
	namespace database
	{

		template <uint16_t length> 
		requires(length > 0)
		class jstring
		{
			char data[length];

			static const int max_length = length;
			static const int last_char = length-1;

			void copy(const char* s)
			{
				char* d = &data[0];
				int l = 0;
				while (l <= last_char && *s)
				{
					*d = *s;
					l++;
					s++;
				}
				d[last_char] = 0;
			}

		public:

			jstring()
			{ 
				data[0] = 0; 
			}

			jstring& operator = (const std::string& src)
			{
				char* s = src.c_str();
				copy(s);
				return *this;
			}

			template <int Y> jstring& operator = (const jstring<Y> &_src)
			{
				char* s = _src.c_str();
				copy(s);
				return *this;
			}

			jstring& operator = (const char *s)
			{
				copy(s);
				return *this;
			}

			const char* c_str() 
			{ 
				return &data[0]; 
			}
		};

		enum jtype
		{
			type_unknown = 0,

			type_int8 = 1,
			type_int16 = 2,
			type_int32 = 3,
			type_int64 = 4,

			type_float8 = 5,
			type_float16 = 6,
			type_float32 = 7,
			type_float64 = 8,

			type_datetime = 9,
			type_string = 10,

			type_object = 16
		};

		struct string_properties_type
		{
			int length;
			jstring<64> validation_pattern;
			jstring<128> validation_message;
		};

		struct int_properties_type 
		{
			int64_t minimum_int;
			int64_t maximum_int;
		};

		struct double_properties_type 
		{
			double minimum_number;
			double maximum_number;
		};

		struct time_properties_type 
		{
			time_t minimum_time_t;
			time_t maximum_time_t;
		};

		struct dimensions_type
		{
			int x, y, z;
		};

		struct object_properties_type 
		{
			dimensions_type		dim;
			row_id_type			class_id;
		};

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			uint32_t				size_bytes;

			jstring<64>				name;
			jstring<256>			description;

			string_properties_type	string_properties;
			int_properties_type		int_properties;
			double_properties_type	double_properties;
			time_properties_type	time_properties;
			object_properties_type  object_properties;
		};

		class jfield_reference
		{
		public:
			row_id_type				field_id;
			uint32_t				offset;
		};

		class jclass_header
		{
		public:
			row_id_type						class_id;
			std::string						name;
			std::string						description;
			size_t							class_size_bytes;
		};

		template <row_id_type max_fields, row_id_type max_classes, row_id_type max_class_fields>
		class jdatabase
		{
		protected:

			table<jfield, max_fields> fields;
			parent_child_table<jclass_header, jfield_reference, max_classes, max_class_fields> classes;

		public:

			jdatabase()
			{

			}

			~jdatabase()
			{

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

				return _field_id;
			}

			row_id_type create_time_field(
				row_id_type _field_id,
				std::string _name,
				std::string _description,
				time_properties_type _time_properties)
			{
				return create_field(_field_id, type_datetime, _name, _description, {}, {}, {}, _time_properties, {}, sizeof(time_t));
			}

			template <typename ITF>
			requires(std::is_integral<ITF>::value)
				row_id_type create_integer_field(
					row_id_type _field_id,
					std::string _name,
					std::string _description,
					int_properties_type _int_properties)
			{
				switch (sizeof(ITF))
				{
					case sizeof(int8_t) :
						return set_field(_field_id, type_int8, _name, _description, {}, _int_properties, {}, {}, {}, sizeof(ITF));
					case sizeof(int16_t) :
						return set_field(_field_id, type_int16, _name, _description, {}, _int_properties, {}, {}, {}, sizeof(ITF));
					case sizeof(int32_t) :
						return set_field(_field_id, type_int32, _name, _description, {}, _int_properties, {}, {}, {}, sizeof(ITF));
					case sizeof(int64_t) :
						return set_field(_field_id, type_int64, _name, _description, {}, _int_properties, {}, {}, {}, sizeof(ITF));
					default:
						throw std::invalid_argument("Invalid integer type for field name:" + _name);
				}
			}

			template <typename DTF>
			requires(std::is_floating_point<DTF>::value)
				row_id_type create_double_field(
					row_id_type _field_id,
					std::string _name,
					std::string _description,
					double_properties_type _double_properties)
			{
				switch (sizeof(DTF))
				{
				case 1:
					auto set_field(_field_id, type_float8, _name, _description, {}, {}, _double_properties, {}, {}, sizeof(DTF));
				case 2:
					auto set_field(_field_id, type_float16, _name, _description, {}, {}, _double_properties, {}, {}, sizeof(DTF));
				case 4:
					auto set_field(_field_id, type_float32, _name, _description, {}, {}, _double_properties, {}, {}, sizeof(DTF));
				case 8:
					auto set_field(_field_id, type_float64, _name, _description, {}, {}, _double_properties, {}, {}, sizeof(DTF));
				default:
					throw std::invalid_argument("Invalid floating point type for field name:" + _name);
				}
			}

			row_id_type create_string_field(row_id_type _field_id, std::string _name, std::string _description, string_properties_type _string_properties)
			{
				return set_field(_field_id, type_string, _name, _description, _string_properties, {}, {}, {}, {}, _string_properties.length);
			}

			row_id_type create_object_field(row_id_type _field_id, std::string _name, std::string _description, object_properties_type _object_properties)
			{
				auto &pcr = classes[ _object_properties.class_id ];
				auto& p = pcr.parent();
				size_t size = pcr.parent().class_size_bytes;
				size_t field_size = _object_properties.dim.x * _object_properties.dim.y * _object_properties.dim.z;
				return set_field(_field_id, type_object, _name, _description, {}, {}, {}, {}, _object_properties, field_size);
			}

			row_id_type create_class(
				std::string& _class_name, 
				std::string& _class_description, 
				std::vector<row_id_type> _field_ids)
			{
				auto sz = _field_ids.size();
				auto pcr = classes.create(sz);
				auto& p = pcr.parent;

				p.class_id = pcr.row_id();
				p.name = _class_name;
				p.description = _class_description;
				p.class_size_bytes = 0;

				for (int i = 0; i < pcr.size(); i++)
				{
					auto fid = _field_ids[i];
					auto& field = fields[fid];
					pcr.child(i) = fid;
					p.class_size_bytes += field.size_bytes;
				}

				return p.class_id;
			}


		};
	}
}
