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
			uint64_t			collection_size_bytes;
			uint32_t			collection_size_items;

			jcollection_ref() = default;
		};

		using collection_table_type = table<jcollection_ref>;
		using collections_by_name_type = sorted_index<object_name, row_id_type, 1>;
		using collections_by_id_type = sorted_index<collection_id_type, row_id_type, 1>;

		struct jdatabase_control_map
		{
		public:
			char		file_kind[32];
			row_id_type schema_location;
			row_id_type collections_location;
			row_id_type collections_by_name_location;
			row_id_type collections_by_id_location;
			object_path database_folder;
			object_path filename;
		};

		using update_function_type = std::function<void(jdatabase*, jarray&)>;

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

			object_path database_filename;
			object_path database_folder;
		};

		class jdatabase_create_collection
		{
		public:
			object_name				collection_name;
			uint64_t				size_bytes;
		};

		class jdatabase_get_collection
		{
		public:
			object_name				collection_name;
		};

		class jdatabase_file_response : public base_result
		{
		public:
			os_result			result;
		};

		class jdatabase_collection_response : public base_result
		{
		public:
			collection_id_type	collection_id;
			os_result			result;
		};

		class jdatabase_size_response : public base_result
		{
		public:
			uint64_t size_bytes;
		};

		class jdatabase_object_response : public base_result
		{
		public:
			object_id_type		object_id;
			jarray_container	items;
		};

		class jdatabase
		{

			dynamic_box						database_box;

			jschema							schema;
			collection_table_type			collections;
			collections_by_name_type		collections_by_name;
			collections_by_id_type			collections_by_id;

			application						*application;
			jdatabase_control_map			*map;

		public:

			jdatabase(system::application* _application);
			~jdatabase();

			task<jdatabase_file_response> open(jdatabase_open _open);
			task<jdatabase_file_response> create(jdatabase_create _create);

			task<jdatabase_collection_response> create_collection(jdatabase_create_collection _create_collection);
			task<jdatabase_collection_response> get_collection(jdatabase_get_collection _create_collection);
		};
	}
};

