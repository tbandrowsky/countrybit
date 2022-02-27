#pragma once

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>

#include "jobject.h"
#include "application.h"
#include "messages.h"
#include "function.h"

namespace countrybit
{
	namespace database
	{

		using namespace countrybit::system;

		class jcollection_ref
		{
		public:

			object_name			collection_name;
			object_path			collection_file_name;
			collection_id_type	collection_id;

			jcollection_ref()
			{

			}
		};

		using jcollection_set = sorted_index<object_name, jcollection_ref, 1>;

		class jdatabase_open 
		{
		public:
			object_path filename;
		};

		class jdatabase_create 
		{
		public:
			int num_collections;

			int num_classes;
			int num_class_fields;
			int num_fields;
			int num_queries;
			int num_http_remotes;
			int num_file_remotes;
			int num_sql_remotes;

			object_path filename;
		};
		
		struct jdatabase_control_map
		{
		public:
			char		file_kind[32];
			row_id_type schema_location;
			row_id_type collections_location;
		};

		class jdatabase
		{

			dynamic_box						database_box;

			jschema							schema;
			jcollection_set					collections;

			application						*application;
			jdatabase_control_map			*map;

		public:

			jdatabase(system::application* _application);

			task<os_result> open(jdatabase_open _open);
			task<os_result> create(jdatabase_create _create);

			~jdatabase();
		};
	}
};

