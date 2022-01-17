#pragma once

#include "jfield.h"
#include <json_parse.h>

namespace countrybit
{
	namespace database
	{

		enum class script_step_types 
		{
			use_database_step = 1,
			create_class_step = 2,
			create_collection_step = 3,
			update_step = 4,
			query_step = 5
		};

		class script_header
		{
		public:
			

		};

		class use_database_step
		{
		public:
			object_name database_name;
		};

		class script_step_header
		{
		public:
			script_step_types script_step_type;			
		};

		class alter_class_field
		{
		public:

			jtype					field_type;
			object_name				field_name;
			object_description		field_description;

			union
			{
				string_properties_type	string_properties;
				int_properties_type		int_properties;
				double_properties_type	double_properties;
				time_properties_type	time_properties;
				object_properties_type  object_properties;
				query_properties_type   query_properties;
			};

			object_description		string_constant;
			int64_t					int_constant;
			double					float_constant;
			time_t					time_constant;

			alter_class_field() = default;
			alter_class_field(object_name& _name);
			alter_class_field(object_name& _name, object_description& _description, string_properties_type _request, object_description& description);
			alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int8_t _value);
			alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int16_t _value);
			alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int32_t _value);
			alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int64_t _value);
			alter_class_field(object_name& _name, object_description& _description, add_double_field_request _request, float _value);
			alter_class_field(object_name& _name, object_description& _description, add_double_field_request _request, double _value);
			alter_class_field(object_name& _name, object_description& _description, time_properties_type _request, time_t _value);
			alter_class_field(object_name& _name, object_description& _description, object_properties_type _request);
			alter_class_field(object_name& _name, object_description& _description, query_properties_type _request);

		};

		class alter_class
		{
		public:
			object_name			class_name;
			int					num_fields;
			alter_class_field	fields[];
		};

		class create_collection_class
		{
		public:
			object_name			collection_name;
		};

		class create_collection
		{
		public:
			int						num_classes;
			create_collection_class	classes[];
		};

		class create_object
		{
		public:
			object_name collection_name;
			object_name class_name;
		};

		class update_objects
		{
		public:
			int num_filter_elements;
			filter_element filters[];
		};	

		class delete_collection
		{
		public:
			object_name collection_name;
		};

		class query_objects
		{
		public:

		};

		class script
		{
			dynamic_box box;

			system::base_parse_result add_script(system::pobject* obj);
			system::base_parse_result add_script_items(system::parray* item);
			system::base_parse_result add_script_item(system::pobject* item);

			system::base_parse_result add_alter_class(system::pobject* obj);
			system::base_parse_result add_create_collection(system::pobject* obj);
			system::base_parse_result add_drop_collection(system::pobject* obj);

		public:

			object_name database_name;
		};

	}
}
