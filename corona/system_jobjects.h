#pragma once

namespace corona
{
	namespace database
	{
		class schema_builder 
		{
		public:

			void build_api_objects(jschema *_schema)
			{
				put_class_request pcr;

				pcr.class_name = "common_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system field request base";

				pcr.member_fields = {
					member_field({ "field_id", jtype::type_int64 } ),
					member_field({ "field_type", jtype::type_string } ),
					member_field({ "field_name", jtype::type_string } ),
					member_field({ "field_description", jtype::type_string } ),
					member_field({ "field_key", jtype::type_int32}),
					member_field({ "field_enumeration_class", jtype::type_string }),
					member_field({ "field_display_field", jtype::type_string	}),
					member_field({ "field_value_field", jtype::type_string })

				};

				_schema->put_class(pcr);

				pcr.class_name = "string_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system string field";

				pcr.member_fields = {
					member_field({ "str_length", jtype::type_int64 }),
					member_field({ "str_pattern", jtype::type_string }),
					member_field({ "str_message", jtype::type_string }),
					member_field({ "str_full_text", jtype::type_int8 }),
					member_field({ "str_rich_text", jtype::type_int8 })
				};

				_schema->put_class(pcr);

				pcr.class_name = "int_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system int field";

				pcr.member_fields = {
					member_field({ "int_minimum", jtype::type_int64 }),
					member_field({ "int_maximum", jtype::type_int64 }),
					member_field({ "int_format", jtype::type_string }),
				};

				_schema->put_class(pcr);

				pcr.class_name = "double_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system double field";

				pcr.member_fields = {
					member_field({ "dbl_minimum", jtype::type_float64 }),
					member_field({ "dbl_maximum", jtype::type_float64 }),
					member_field({ "dbl_format", jtype::type_string }),
				};

				_schema->put_class(pcr);

				pcr.class_name = "time_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system datetime field";

				pcr.member_fields = {
					member_field({ "date_minimum", jtype::type_datetime }),
					member_field({ "date_maximum", jtype::type_datetime }),
					member_field({ "date_format", jtype::type_string }),
				};

				_schema->put_class(pcr);

				pcr.class_name = "currency_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system currency field";

				pcr.member_fields = {
					member_field({ "currency_minimum", jtype::type_currency}),
					member_field({ "currency_maximum", jtype::type_currency }),
					member_field({ "currency_format", jtype::type_string }),
				};

				_schema->put_class(pcr);

				pcr.class_name = "image_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system image field";

				pcr.member_fields = {
					member_field({ "image_path", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "wave_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system wave field";

				pcr.member_fields = {
					member_field({ "wave_path", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "midi_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system midi field";

				pcr.member_fields = {
					member_field({ "midi_path", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "dim";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system dimension field";

				pcr.member_fields = {
					member_field({ "x", jtype::type_int64}),
					member_field({ "y", jtype::type_int64}),
					member_field({ "z", jtype::type_int64}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "object_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system dimension field";

				pcr.member_fields = {
					member_field({ "class_name", jtype::type_string}),
					member_field({ member_field_types::member_class, "dim" }),
				};

				_schema->put_class(pcr);

				pcr.class_name = "remote_field_map";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote mapping of a single field";

				pcr.member_fields = {
					member_field({ "local_name", jtype::type_string}),
					member_field({ "remote_name", jtype::type_string}),
				};

				pcr.class_name = "remote_parameter_map";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote mapping of a single parameter";

				pcr.member_fields = {
					member_field({ "local_name", jtype::type_string}),
					member_field({ "remote_name", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "remote_maps";
				pcr.auto_primary_key = false;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote mapping field set";

				pcr.member_fields = {
					member_field({ "result_class_name", jtype::type_string}),
					member_field({ "remote_parameter_map", 64, jtype::type_list}),
					member_field({ "remote_field_map", 128, jtype::type_list}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "filter_term";
				pcr.auto_primary_key = false;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "filter_term";

				pcr.member_fields = {
					member_field({ "src_value", jtype::type_string}),
					member_field({ "target_field", jtype::type_string}),
					member_field({ "comparison", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "filter_props";
				pcr.auto_primary_key = false;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "filter options";

				pcr.member_fields = {
					member_field({ "filter_term", 64, jtype::type_list}),
				};
				_schema->put_class(pcr);

				pcr.class_name = "query_status";
				pcr.auto_primary_key = false;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system query status";

				pcr.member_fields = {
					member_field({ "last_success"}),
					member_field({ "last_error", jtype::type_datetime}),
					member_field({ "error_message", jtype::type_string}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "file_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote_file";

				pcr.member_fields = {
					member_field({ "file_path", jtype::type_string }),
					member_field({ "remote_maps", {1,1,1}}),
					member_field({ "query_status", {1,1,1}}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "http_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote_file";

				pcr.member_fields = {
					member_field({ "login_type_name", jtype::type_string }),
					member_field({ "login_url", jtype::type_string }),
					member_field({ "login_method", jtype::type_string }),
					member_field({ "username", jtype::type_string }),
					member_field({ "password", jtype::type_string }),
					member_field({ "data_url", jtype::type_string }),
					member_field({ "data_method", jtype::type_string }),
					member_field({ "data_query", jtype::type_string }),
					member_field({ "remote_maps", {1,1,1}}),
					member_field({ "query_status", {1,1,1}}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "sql_props";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system remote_file";

				pcr.member_fields = {
					member_field({ "login_type_name", jtype::type_string }),
					member_field({ "login_url", jtype::type_string }),
					member_field({ "username", jtype::type_string }),
					member_field({ "password", jtype::type_string }),
					member_field({ "connection", jtype::type_string }),
					member_field({ "query", jtype::type_string }),
					member_field({ "remote_maps", {1,1,1}}),
					member_field({ "query_status", {1,1,1}}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_string_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a string field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "string_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_integer_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create an integer field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "string_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_double_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a double field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "double_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_time_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a double field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "time_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_object_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to put an object field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "object_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_filter_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a filter field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "filter_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_sql_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a sql field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "sql_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_file_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a file field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "file_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_http_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create an http field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "http_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_point_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a point field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "point_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_rectangle_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a rectangle field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "rectangle_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_layout_rect_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a layout rectangle field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "layout_rect_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_image_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create an field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "image_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);


				pcr.class_name = "put_wave_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a wave field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "wave_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);


				pcr.class_name = "put_midi_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a midi field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "midi_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "put_color_field_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a color field";

				pcr.member_fields = {
					member_field({ "common_props", { 1, 1, 1 } }),
					member_field({ "color_props", { 1, 1, 1 } })
				};

				_schema->put_class(pcr);

				pcr.class_name = "object_dim";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "system dimension field";

				pcr.member_fields = {
					member_field({ "x", jtype::type_int64}),
					member_field({ "y", jtype::type_int64}),
					member_field({ "z", jtype::type_int64}),
				};

				_schema->put_class(pcr);

				pcr.class_name = "class_member";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "a class member";

				pcr.member_fields = {
					member_field({ "field_name", jtype::type_string }),
					member_field({ "list_use", jtype::type_int8 }),
					member_field({ "list_size", jtype::type_int32 }),
					member_field({ "object_use", jtype::type_int8 }),
					member_field({ "object_dim", {1,1,1}, jtype::type_object }),
				};

				_schema->put_class(pcr);


				pcr.class_name = "put_class_request";
				pcr.auto_primary_key = true;
				pcr.base_class_id = null_row;
				pcr.class_id = null_row;
				pcr.field_id_primary_key = null_row;
				pcr.class_description = "request to create a class";

				pcr.member_fields = {
					member_field({ "class_name", jtype::type_string }),
					member_field({ "class_description", jtype::type_string }),
					member_field({ "create_prompt", jtype::type_string }),
					member_field({ "auto_primary_key", jtype::type_int8 }),
					member_field({ "primary_key_field", jtype::type_string }),
					member_field({ "base_class", jtype::type_string }),
					member_field({ "template_class", jtype::type_string }),
					member_field({ "class_member", 32, jtype::type_list })
				};

				_schema->put_class(pcr);


			}

		};



	}
}
