#pragma once

#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
#include "remote_box.h"
#include "jfield.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

		enum class http_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			jwt_authentication = 3,
			certificate_authentication = 4
		};

		struct named_http_properties_type
		{
			object_name					field_name;
			http_login_types			login_type;
			object_name					username;
			object_name					password;
			import_http_url				url;
			import_http_method			method;

			object_name					destination_class;
			parameter_field_maps_type	parameters;
			import_field_maps_type		import_fields_map;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		class http_remote_instance
		{
		public:
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		class http_remote_box
		{
			boxed<http_remote_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			http_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice);
			http_remote_box(http_remote_box& _src);
			http_remote_box operator = (const http_remote_box& _src);
			http_remote_box operator = (http_remote_instance _src);
			operator http_remote_instance& ();
			http_remote_instance value() const;
		};

	}
}
