#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
#include "jfield.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

		class jcollection;
		class jdatabase;
		class jslice;
		class jschema;

		using import_sql_query = istring<8192>;
		using import_http_post_body = istring<8192>;
		using import_http_url = istring<512>;
		using import_http_method = istring<16>;
		using import_file_path = istring<512>;

		struct import_field_map_type
		{
			object_name corona_field;
			row_id_type corona_field_id;
			object_name source_field;
		};

		const int max_import_fields = 256;
		const int max_import_parameter_fields = 32;

		using import_field_maps_type = iarray<import_field_map_type, max_import_fields>;
		using parameter_field_maps_type = iarray<import_field_map_type, max_import_parameter_fields>;

	}
}
