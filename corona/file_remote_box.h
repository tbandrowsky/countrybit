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

		struct named_file_properties_type
		{
			remote_file_path				file_path;
			object_name						parameter_field;
			object_name						result_field;
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

		class file_remote_box
		{
			boxed<file_remote_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			file_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice);
			file_remote_box(file_remote_box& _src);
			file_remote_box operator = (const file_remote_box& _src);
			file_remote_box operator = (file_remote_instance _src);
			operator file_remote_instance& ();
			file_remote_instance value() const;
		};
	}
}
