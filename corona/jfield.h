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
#include <format>
#include <ctime>

#include "constants.h"
#include "store_box.h"
#include "table.h"
#include "sorted_index.h"
#include "int_box.h"
#include "float_box.h"
#include "time_box.h"
#include "string_box.h"
#include "array_box.h"
#include "remote_box.h"

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

			field_mime_type = 83,
			field_file_name = 84,
			field_width = 85,
			field_height = 86,
			field_x = 87,
			field_y = 88,
			field_z = 89,
			field_red = 90,
			field_green = 91,
			field_blue = 92,
			field_alpha = 93,

			field_collection_id = 94,
			field_object_id = 95;

		const int max_class_fields = 128;

		struct string_properties_type
		{
			int32_t						length;
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
			double			minimum_double;
			double			maximum_double;
		};

		struct time_properties_type
		{
			int64_t			minimum_time_t;
			int64_t			maximum_time_t;
		};

		struct point_properties_type
		{

		};

		struct rectangle_properties_type
		{

		};

		struct color_properties_type
		{

		};

		struct query_properties_type
		{
			row_id_type properties_id;
		};

		struct sql_properties_type
		{
			row_id_type properties_id;
		};

		struct file_properties_type
		{
			row_id_type properties_id;
		};

		struct http_properties_type
		{
			row_id_type properties_id;
		};

		struct emphemeral_handle_type
		{
			time_t	valid_time;
			uint64_t handle;
		};

		struct image_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		struct midi_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		struct wave_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
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
			object_name			class_name;
			int64_t				class_size_bytes;
			int64_t				total_size_bytes;
		};

		struct model_properties_type
		{
			row_id_type			class_id;
			object_name			class_name;
			int64_t				model_size_bytes;
		};

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			int64_t					size_bytes;

			object_name				name;
			object_description		description;

			union
			{
				string_properties_type		string_properties;
				int_properties_type			int_properties;
				double_properties_type		double_properties;
				time_properties_type		time_properties;
				object_properties_type		object_properties;
				query_properties_type		query_properties;
				point_properties_type		point_properties;
				rectangle_properties_type   rectangle_properties;
				image_properties_type		image_properties;
				midi_properties_type		midi_properties;
				wave_properties_type		wave_properties;
				color_properties_type		color_properties;
				sql_properties_type			sql_properties;
				file_properties_type		file_properties;
				http_properties_type		http_properties;
			};

			jfield() { ; }
			~jfield() { ; }

			bool is_data_generator() 
			{
				return type_id == jtype::type_query || type_id == jtype::type_sql || type_id == jtype::type_http || type_id == jtype::type_file;
			}

			bool is_integer()
			{
				return type_id == jtype::type_int16 || type_id == jtype::type_int32 || type_id == jtype::type_int64 || type_id == jtype::type_int8;
			}

			bool is_point()
			{
				return type_id == jtype::type_point;
			}

			bool is_rectangle()
			{
				return type_id == jtype::type_rectangle;
			}

			bool is_color()
			{
				return type_id == jtype::type_color;
			}

			bool is_image()
			{
				return type_id == jtype::type_image;
			}

			bool is_wave()
			{
				return type_id == jtype::type_wave;
			}

			bool is_midi()
			{
				return type_id == jtype::type_midi;
			}

			bool is_int8()
			{
				return type_id == jtype::type_int8;
			}

			bool is_int16()
			{
				return type_id == jtype::type_int16;
			}

			bool is_int32()
			{
				return type_id == jtype::type_int32;
			}

			bool is_int64()
			{
				return type_id == jtype::type_int64;
			}

			bool is_float()
			{
				return type_id == jtype::type_float32 || type_id == jtype::type_float64;
			}

			bool is_float32()
			{
				return type_id == jtype::type_float32;
			}

			bool is_float64()
			{
				return type_id == jtype::type_float64;
			}

			bool is_string()
			{
				return type_id == jtype::type_string;
			}

			bool is_datetime()
			{
				return type_id == jtype::type_datetime;
			}

		};

		const int max_query_filters = 32;
		const int max_path_nodes = 64;

		class path_root
		{
		public:
			object_name				model_name;
			row_id_type				model_id;
		};

		enum class node_operations
		{
			traverse,
			group_by,
			calc_min,
			calc_max,
			calc_sum,
			calc_count,
			calc_stddev
		};

		class path_node
		{
		public:
			object_name		member_name;
			row_id_type		member_id;
			row_id_type		member_index;
			operation_name	node_operation_name;
			node_operations node_operation;
		};

		using path_nodes = iarray<path_node, max_path_nodes>;

		class path
		{
		public:
			path_root		root;
			path_nodes		nodes;
		};

		enum class filter_comparison_types
		{
			eq = 0,
			ls = 1,
			gt = 2,
			lseq = 3,
			gteq = 4,
			contains = 5,
			inlist = 6,
			distance = 7
		};

		struct filter_element
		{
		public:
			object_name				target_field_name;
			row_id_type				target_field_id;
			operation_name			comparison_name;
			filter_comparison_types	comparison;
			row_id_type				parameter_field_id;
			object_name				parameter_field_name;
			int64_t					parameter_offset;
			int64_t					target_offset;
			double					distance_threshold;
			std::function<bool(char* a, char* b)> compare;
			const char* error_message;
		};

		using filter_element_collection = iarray<filter_element, max_filters>;

		class query_definition_t
		{
		public:
			path						source_path;
			object_name					result_class_name;
			row_id_type					result_class_id;
			row_id_type					result_field_id;
			row_id_type					max_result_objects;
			filter_element_collection	filter;
		};

		using query_definition_type = query_definition_t;

		class query_instance
		{
		public:
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		struct file_definition_type
		{
			remote_file_path				file_path;
			object_name						parameter_field;
			row_id_type						parameter_field_id;
			row_id_type						result_field_id;
			object_name						result_class_name;
			row_id_type						result_class_id;
			int								max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class file_remote_instance
		{
		public:
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		enum class http_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			jwt_authentication = 3,
			certificate_authentication = 4
		};

		struct http_definition_type
		{
			operation_name					login_type_name;
			http_login_types				login_type;
			remote_http_url					login_url;
			remote_http_method				login_method;
			object_name						username;
			object_name						password;
			remote_http_url					data_url;
			remote_http_method				data_method;
			row_id_type						result_field_id;
			object_name						result_class_name;
			row_id_type						result_class_id;
			row_id_type						max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class http_remote_instance
		{
		public:
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		enum class sql_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			certificate_authentication = 3
		};

		struct sql_definition_type
		{
			operation_name					login_type_name;
			sql_login_types					login_type;
			object_name						username;
			object_name						password;
			row_id_type						result_field_id;
			object_name						result_class_name;
			row_id_type						result_class_id;
			row_id_type						max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			remote_sql_query				query;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class sql_remote_instance
		{
		public:
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};


		class put_field_request_base {
		public:
			row_id_type field_id;
			jtype		type_id;
			object_name name;
			object_description description;
			object_type	type;
		};

		class put_string_field_request {
		public:
			put_field_request_base name;
			string_properties_type options;
		};

		class put_integer_field_request {
		public:
			put_field_request_base name;
			int_properties_type options;
		};

		class put_double_field_request {
		public:
			put_field_request_base name;
			double_properties_type options;
		};

		class put_time_field_request {
		public:
			put_field_request_base name;
			time_properties_type options;
		};

		class put_object_field_request {
		public:
			put_field_request_base name;
			object_properties_type options;
		};

		class put_named_query_field_request {
		public:
			put_field_request_base name;
			query_definition_type options;
		};

		class put_named_sql_remote_field_request {
		public:
			put_field_request_base name;
			sql_definition_type options;
		};

		class put_named_file_import_field_request {
		public:
			put_field_request_base name;
			file_definition_type options;
		};

		class put_named_http_import_field_request
		{
		public:
			put_field_request_base name;
			http_definition_type options;
		};

		class put_point_field_request {
		public:
			put_field_request_base name;
			point_properties_type options;
		};

		class put_rectangle_field_request {
		public:
			put_field_request_base name;
			rectangle_properties_type options;
		};

		class put_image_field_request {
		public:
			put_field_request_base name;
			image_properties_type options;
		};

		class put_wave_field_request {
		public:
			put_field_request_base name;
			wave_properties_type options;
		};

		class put_midi_field_request {
		public:
			put_field_request_base name;
			midi_properties_type options;
		};

		class put_color_field_request {
		public:
			put_field_request_base name;
			color_properties_type options;
		};

		class put_file_field_request {
		public:
			put_field_request_base name;
			file_properties_type options;
		};

		class put_sql_field_request {
		public:
			put_field_request_base name;
			sql_properties_type options;
		};

		class put_http_field_request {
		public:
			put_field_request_base name;
			http_properties_type options;
		};

		enum class member_field_types
		{
			member_field = 1,
			member_class = 2,
			member_list = 3
		};

		struct member_field
		{
		public:
			member_field_types	membership_type;
			object_name			field_name;
			bool				use_id;
			object_name			membership_type_name;

			union
			{
				row_id_type field_id;
				row_id_type class_id;
			};

			dimensions_type dimensions;

			member_field() : use_id(false) { ; }

			member_field(row_id_type _field_id) :
				membership_type(member_field_types::member_field),
				use_id(true),
				field_id(_field_id),
				dimensions{ 1, 1, 1 }
			{
				;
			}

			member_field(member_field_types _member_ship_type, row_id_type _id) :
				membership_type(_member_ship_type),
				use_id(true),
				dimensions{ 1, 1, 1 }
			{
				switch (membership_type)
				{
				case member_field_types::member_class:
					class_id = _id;
					break;
				case member_field_types::member_field:
					field_id = _id;
					break;
				}
			}

			member_field(member_field_types _member_ship_type, object_name _name) :
				membership_type(_member_ship_type),
				use_id(false),
				field_name(_name),
				dimensions{ 1, 1, 1 }
			{

			}

			member_field(row_id_type _class_id, int _maximum) :
				membership_type(member_field_types::member_list),
				use_id(true),
				class_id(_class_id)
			{
				dimensions = { _maximum, 1, 1 };
			}

			member_field(row_id_type _class_id, dimensions_type dims) :
				membership_type(member_field_types::member_class),
				use_id(true),
				class_id(_class_id)
			{
				dimensions = dims;
			}

			member_field(object_name _name, int _maximum) :
				membership_type(member_field_types::member_list),
				use_id(false),
				field_name(_name)
			{
				dimensions = { _maximum, 1, 1 };
			}

			member_field(object_name _name, dimensions_type dims) :
				membership_type(member_field_types::member_class),
				use_id(true),
				field_name(_name)
			{
				dimensions = dims;
			}
		};

		using member_field_collection = iarray<member_field, max_class_fields>;

		class put_class_request
		{
		public:
			row_id_type				 class_id;
			object_name				 class_name;
			object_description		 class_description;
			member_field_collection member_fields;
		};

		struct model_state
		{
		public:
			object_name			class_name;
			row_id_type			class_id;
			bool				use_id;
			row_id_type			number_of_visits;
		};

		using model_state_collection = iarray<model_state, max_class_fields>;

		class put_model_request
		{
		public:
			row_id_type				 class_id;
			object_name				 class_name;
			object_description		 class_description;
			member_field_collection  member_fields;
			model_state_collection   model_states;
			object_name				 actor_id_field_name;
			row_id_type				 actor_id_field_id;
			row_id_type				 number_of_actors;
		};

		class jlist_instance
		{
		public:
			int32_t   allocated;
			int32_t   selection_offset;
			int32_t   slice_offset;
		};

		class jlist_state
		{
		public:
			array_box<row_id_type> selections;
			jlist_instance* instance;
			char	  *list_bytes;
		};

		class jmodel_instance
		{
		public:
			int32_t			number_of_actors;
			row_id_type		selection_offset;
			row_id_type		slice_offset;
			row_id_type		actor_id;
		};

		class jmodel_state
		{
		public:
			int32_t			number_of_actors;
			array_box<row_id_type> selections;
			char		    *model_bytes;
			jmodel_instance* instance;
		};

		// a store id is in fact, a guid
		// some overlap between a model and a class

		class jclass_header
		{
		public:
			row_id_type									class_id;
			object_name									name;
			object_description							description;
			uint64_t									class_size_bytes;
			bool										is_model;
			row_id_type									number_of_actors;
		};

		class jclass_field
		{
		public:
			row_id_type				field_id;
			uint64_t				offset;
			bool					model_state;
			int						actor_field_index;
		};

		using jclass = item_details_holder<jclass_header, jclass_field>;

		struct update_element
		{
		public:
			object_name				target_field_name;
			row_id_type				target_field_id;
			row_id_type				parameter_field_id;
			object_name				parameter_field_name;
			int64_t					parameter_offset;
			int64_t					target_offset;
			std::function<void(char* a, char* b)> assignment;
			const char* error_message;
		};

		using update_element_collection = iarray<update_element, max_filters>;

		class update_definition_t
		{
		public:
			path						source_path;
			filter_element_collection	filter;
			update_element_collection	update;
		};

		using update_definition_type = update_definition_t;

	}
}

