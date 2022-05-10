#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"

#include <ostream>
#include <bit>

namespace corona
{
	namespace database
	{

		class jcollection;
		class jdatabase;
		class jslice;
		class jschema;

		using remote_sql_query = istring<8192>;
		using remote_http_post_body = istring<8192>;
		using remote_http_url = istring<512>;
		using remote_http_method = istring<16>;
		using remote_file_path = istring<512>;

		struct remote_field_map_type
		{
			object_name corona_field;
			relative_ptr_type corona_field_id;
			object_name remote_field;
		};

		const int max_remote_fields = 256;
		const int max_remote_parameter_fields = 32;

		using remote_fields_type = iarray<remote_field_map_type, max_remote_fields>;
		using remote_parameter_fields_type = iarray<remote_field_map_type, max_remote_parameter_fields>;

	}
}
