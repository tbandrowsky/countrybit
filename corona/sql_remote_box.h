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
		enum class sql_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			certificate_authentication = 3
		};

		struct named_sql_properties_type
		{
			object_name						login_type_name;
			sql_login_types					login_type;
			object_name						username;
			object_name						password;
			object_name						parameter_field;
			object_name						result_field;
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
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		class sql_remote_box
		{
			boxed<sql_remote_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			sql_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice);
			sql_remote_box(sql_remote_box& _src);
			sql_remote_box operator = (const sql_remote_box& _src);
			sql_remote_box operator = (sql_remote_instance _src);
			operator sql_remote_instance& ();
			sql_remote_instance value();
		};

	}
}
