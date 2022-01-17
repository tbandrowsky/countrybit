
#include "script.h"

namespace countrybit
{
	namespace database
	{
		alter_class_field::alter_class_field(object_name& _name)
		{
			projection_element pe{};
			field_type = jtype::type_null;
			field_name = _name;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, string_properties_type _request, object_description& description)
		{
			projection_element pe{};
			field_type = jtype::type_string;
			field_name = _name;
			description = _description;
			string_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int8_t _value)
		{
			projection_element pe{};
			field_type = jtype::type_int8;
			field_name = _name;
			field_description = _description;
			int_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int16_t _value)
		{
			projection_element pe{};
			field_type = jtype::type_int16;
			field_name = _name;
			field_description = _description;
			int_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int32_t _value)
		{
			projection_element pe{};
			field_type = jtype::type_int32;
			field_name = _name;
			field_description = _description;
			int_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, int_properties_type _request, int64_t _value)
		{
			projection_element pe{};
			field_type = jtype::type_int64;
			field_name = _name;
			field_description = _description;
			int_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, add_double_field_request _request, float _value)
		{
			field_type = jtype::type_float32;
			field_name = _name;
			field_description = _description;
			double_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, add_double_field_request _request, double _value)
		{
			field_type = jtype::type_float64;
			field_name = _name;
			field_description = _description;
			double_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, time_properties_type _request, time_t _value)
		{
			projection_element pe{};
			field_type = jtype::type_datetime;
			field_name = _name;
			field_description = _description;
			time_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, object_properties_type _request)
		{
			projection_element pe{};
			field_type = jtype::type_object;
			field_name = _name;
			field_description = _description;
			object_properties = _request;
		}

		alter_class_field::alter_class_field(object_name& _name, object_description& _description, query_properties_type _request)
		{
			field_type = jtype::type_query;
			field_name = _name;
			field_description = _description;
			query_properties = _request;
		}

	}
}
