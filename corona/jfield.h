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
#include "sorted_index.h"

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

		using jquery = iquery<128,128>;
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

		struct query_properties_type
		{
			jquery			query;
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

			union {
				string_properties_type	string_properties;
				int_properties_type		int_properties;
				double_properties_type	double_properties;
				time_properties_type	time_properties;
				object_properties_type  object_properties;
				query_properties_type   query_properties;
			};
		};

		class add_field_request_base {
		public:
			row_id_type field_id;
			jtype		type_id;
			object_name name;
			object_description description;
		};

		class add_string_field_request : public add_field_request_base, public string_properties_type {
		public:

		};

		class add_integer_field_request : public add_field_request_base, public int_properties_type {
		public:

		};

		class add_double_field_request : public add_field_request_base, public double_properties_type {
		public:

		};

		class add_time_field_request : public add_field_request_base, public time_properties_type {
		public:

		};

		class add_object_field_request : public add_field_request_base, public object_properties_type {
		public:

		};

		class add_query_field_request : public add_field_request_base, public query_properties_type {
		public:

		};

	}
}

