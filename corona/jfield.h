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
#include "int_box.h"
#include "float_box.h"
#include "time_box.h"
#include "string_box.h"
#include "collection_id_box.h"
#include "object_id_box.h"
#include "query_box.h"
#include "point_box.h"
#include "rectangle_box.h"
#include "midi_box.h"
#include "image_box.h"
#include "wave_box.h"
#include "color_box.h"
#include "sorted_index.h"
#include "sql_remote_box.h"
#include "file_remote_box.h"
#include "http_remote_box.h"

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

		using object_name = istring<32>;
		using object_description = istring<250>;
		using object_path = istring<256>;
		using object_type = istring<16>;
		using string_validation_pattern = istring<100>;
		using string_validation_message = istring<100>;

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

		class jfield
		{
		public:

			row_id_type				field_id;
			jtype					type_id;
			int64_t					size_bytes;

			object_name				name;
			object_description		description;

			union {
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

		class put_integer_field_request  {
		public:
			put_field_request_base name;
			int_properties_type options;
		};

		class put_double_field_request  {
		public:
			put_field_request_base name;
			double_properties_type options;
		};

		class put_time_field_request  {
		public:
			put_field_request_base name;
			time_properties_type options;
		};

		class put_object_field_request {
		public:
			put_field_request_base name;
			object_properties_type options;
		};

		class put_query_field_request  {
		public:
			put_field_request_base name;
			query_properties_type options;
		};

		class put_named_query_field_request {
		public:
			put_field_request_base name;
			named_query_properties_type options;
		};

		class put_named_sql_import_field_request {
		public:
			put_field_request_base name;
			sql_properties_type options;
		};

		class put_named_sql_import_field_request {
		public:
			put_field_request_base name;
			sql_properties_type options;
		};

		class put_named_sql_import_field_request {
		public:
			put_field_request_base name;
			sql_properties_type options;
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
			named_file_properties_type options;
		};

		class put_sql_field_request {
		public:
			put_field_request_base name;
			named_sql_properties_type options;
		};

		class put_http_field_request {
		public:
			put_field_request_base name;
			named_http_properties_type options;
		};

		enum class membership_types
		{
			member_field = 1,
			member_class = 2
		};

		struct member_field
		{
		public:
			object_name			field_name;
			object_name			membership_type_name;
			membership_types	membership_type;

			union
			{
				row_id_type field_id;
				row_id_type class_id;
			};

			dimensions_type dimensions;
		};

		class put_named_class_request 
		{
		public:
			row_id_type			class_id;
			object_name			class_name;
			object_description	class_description;
			iarray<member_field, max_class_fields> member_fields;
		};

	}
}

