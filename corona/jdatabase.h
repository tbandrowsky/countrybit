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

		class open_db_request
		{
		public:
			object_path filename;
		};

		class create_db_request
		{
		public:
			int num_collections;
			int num_classes;
			int num_models;

			object_path database_filename;
			object_path database_folder;
		};

		class db_response : public system::base_result
		{
		public:
			dynamic_box response_data;
			system::os_result os_code;
		};

		class create_collection_request 
		{
		public:
			object_name model_name;
			object_name collection_name;
			int max_actors;
			int max_objects;
		};

		class get_collection_request
		{
		public:
			object_name collection_name;
			collection_id_type collection_id;
		};

		class collection_response : public db_response
		{
		public:
			jcollection collection;
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

			system::task<db_response> open(open_db_request _open);
			system::task<db_response> create(create_db_request _create);

			row_id_type put_string_field(put_string_field_request request);
			row_id_type put_time_field(put_time_field_request request);
			row_id_type put_integer_field(put_integer_field_request request);
			row_id_type put_double_field(put_double_field_request request);
			row_id_type put_query_field(put_named_query_field_request request);
			row_id_type put_sql_remote_field(put_named_sql_remote_field_request request);
			row_id_type put_http_remote_field(put_named_http_remote_field_request request);
			row_id_type put_file_remote_field(put_named_file_remote_field_request request);
			row_id_type put_point_field(put_point_field_request request);
			row_id_type put_rectangle_field(put_rectangle_field_request request);
			row_id_type put_image_field(put_image_field_request request);
			row_id_type put_wave_field(put_wave_field_request request);
			row_id_type put_midi_field(put_midi_field_request request);
			row_id_type put_color_field(put_color_field_request request);
			jfield get_field(object_name name);

			row_id_type put_class(put_class_request request);		
			jclass get_class(object_name name);

			jmodel put_model(jmodel request);
			jmodel get_model(object_name name);

			collection_response create_collection(create_collection_request _create_collection);
			collection_response get_collection(get_collection_request _get_collection);

			actor_type create_actor(actor_type _actor);
			actor_type get_actor(actor_id_type _actor_id);
			actor_type update_actor(actor_type _actor);
			actor_type put_actor(actor_type _actor);

			actor_command_response get_command_result(row_id_type _actor);
			actor_command_response select_object(const actor_select_object& _select);
			actor_command_response create_object(actor_create_object& _create);
			actor_command_response update_object(actor_update_object& _update);
			
		};
	}
};

