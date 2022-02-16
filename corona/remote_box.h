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

		enum class sql_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			certificate_authentication = 3
		};

		enum class http_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			jwt_authentication = 3,
			certificate_authentication = 4
		};

		struct named_sql_properties_type
		{
			object_name					field_name;
			sql_login_types				login_type;
			object_name					username;
			object_name					password;

			object_name					destination_class;
			parameter_field_maps_type	parameters;
			import_field_maps_type		import_fields_map;
			import_sql_query			query;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		struct named_file_properties_type
		{
			object_name					field_name;
			parameter_field_maps_type	parameters;
			import_file_path			file_path;

			object_name					destination_class;
			parameter_field_maps_type	parameters;
			import_field_maps_type		import_fields_map;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
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

			sql_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
			{
				;
			}

			sql_remote_box(sql_remote_box& _src) : instance(_src.instance)
			{
				;
			}

			sql_remote_box operator = (const sql_remote_box& _src)
			{
				instance = _src.instance;
				return *this;
			}

			sql_remote_box operator = (sql_remote_instance _src)
			{
				instance = _src;
				return *this;
			}

			operator sql_remote_instance& ()
			{
				sql_remote_instance& t = instance.get_data_ref();
				return t;
			}

			sql_remote_instance value() const { return instance.get_value(); }
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

			file_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
			{
				;
			}

			file_remote_box(file_remote_box& _src) : instance(_src.instance)
			{
				;
			}

			file_remote_box operator = (const file_remote_box& _src)
			{
				instance = _src.instance;
				return *this;
			}

			file_remote_box operator = (file_remote_instance _src)
			{
				instance = _src;
				return *this;
			}

			operator file_remote_instance& ()
			{
				file_remote_instance& t = instance.get_data_ref();
				return t;
			}

			file_remote_instance value() const { return instance.get_value(); }
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

			http_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
			{
				;
			}

			http_remote_box(http_remote_box& _src) : instance(_src.instance)
			{
				;
			}

			http_remote_box operator = (const http_remote_box& _src)
			{
				instance = _src.instance;
				return *this;
			}

			http_remote_box operator = (http_remote_instance _src)
			{
				instance = _src;
				return *this;
			}

			operator http_remote_instance& ()
			{
				http_remote_instance& t = instance.get_data_ref();
				return t;
			}

			http_remote_instance value() const { return instance.get_value(); }
		};

	}
}
