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

namespace countrybit
{
	namespace database
	{

		class jcollection_ref
		{
		public:

			object_name			collection_name;
			object_path			collection_file_name;
			collection_id_type	collection_id;
			uint64_t			collection_size_bytes;
			uint32_t			collection_size_items;

			row_id_type			collection_location;
			dynamic_box			data;

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

		class jdatabase_open 
		{
		public:
			object_path filename;
		};

		class jdatabase_create 
		{
		public:
			int num_collections;

			object_path database_filename;
			object_path database_folder;
		};

		class jdatabase_response : public system::base_result
		{
		public:
			system::os_result os_code;
		};

		class jdatabase_create_collection
		{
		public:
			object_name				collection_name;
			uint32_t				model_name;
			uint32_t				max_actors;
			uint32_t				max_objects;
		};

		class jdatabase_collection_response : public jdatabase_response
		{
		public:
			collection_id_type	collection_id;
		};

		class jdatabase
		{

			dynamic_box						database_box;

			jschema							schema;
			collection_table_type			collections;
			collections_by_name_type		collections_by_name;
			collections_by_id_type			collections_by_id;

			system::application				*application;
			jdatabase_control_map			*map;

		public:

			jdatabase(system::application* _application);
			~jdatabase();

			bool alter_schema(std::function<bool(jschema* _schema)> schema_proc);

			system::task<jdatabase_response> open(jdatabase_open _open);
			system::task<jdatabase_response> create(jdatabase_create _create);

			system::task<jdatabase_collection_response> create_collection(jdatabase_create_collection _create_collection);

			system::task<actor_type> create_actor(actor_type _actor);
			system::task<actor_type> get_actor(actor_id_type _actor_id);
			system::task<actor_type> update_actor(actor_type _actor);
			system::task<actor_type> put_actor(actor_type _actor);

			system::task<actor_command_response> get_command_result(row_id_type _actor);
			system::task<actor_command_response> select_object(const actor_select_object& _select);
			system::task<actor_command_response> create_object(actor_create_object& _create);
			system::task<actor_command_response> update_object(actor_update_object& _update);
			
		};
	}
};

