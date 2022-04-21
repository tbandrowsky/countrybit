
#include <filesystem>

#include "application.h"
#include "jdatabase.h"

namespace countrybit 
{
	namespace database 
	{

		jdatabase::jdatabase(system::application* _application) :
			application( _application )
		{

		}

		system::task<db_response> jdatabase::open(open_db_request _open)
		{
			int success = 1;
			countrybit::system::file dbfile = application->open_file(_open.filename.c_str(), countrybit::system::file_open_types::open_existing);
			if (dbfile.success()) {
				auto sz = dbfile.size();
				database_box.init(sz);
				co_await dbfile.read(0, database_box.data(), sz);
				if (dbfile.success()) {
					map = database_box.unpack<jdatabase_control_map>(0);
					schema = jschema::get_schema(database_box.get_box(), map->schema_location);
					collections = collection_table_type::get_table(&database_box, map->collections_location);
					collections_by_id = collections_by_id_type::get_sorted_index(&database_box, map->collections_by_id_location);
					collections_by_name = collections_by_name_type::get_sorted_index(&database_box, map->collections_by_name_location);
				}
			} 
			db_response jfr;
			jfr.os_code = dbfile.result();
			jfr.success = true;
			co_return jfr;
		}

		system::task<db_response> jdatabase::create(create_db_request _create)
		{
			system::os_result last_err;

			int success = 1;

			int64_t schema_size_bytes =	jschema::get_box_size(_create.num_classes, _create.num_models, true);
			int64_t collection_size_bytes = collection_table_type::get_box_size(_create.num_collections);
			int64_t collection_by_name_bytes = collections_by_name_type::get_box_size(_create.num_collections);
			int64_t collection_by_id_bytes = collections_by_id_type::get_box_size(_create.num_collections);

			database_box.init(collection_size_bytes + schema_size_bytes + collection_by_name_bytes + collection_by_id_bytes + sizeof(jdatabase_control_map));
			map = database_box.allocate<jdatabase_control_map>(1);

			std::filesystem::path dbpath = _create.database_filename.c_str();
			std::filesystem::path dbfolder = dbpath.parent_path();

			map->filename = dbpath;
			map->database_folder = dbfolder;

			schema = jschema::create_schema(database_box.get_box(), _create.num_classes, _create.num_models, true, map->schema_location);
			collections = collection_table_type::create_table(&database_box, _create.num_collections, map->collections_location);
			collections_by_id = collections_by_id_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_id_location);
			collections_by_name = collections_by_name_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_name_location);

			countrybit::system::file dbfile = application->create_file(_create.database_filename);

			if (dbfile.success()) {
				auto result = co_await dbfile.write(0, database_box.data(), database_box.size());
			}

			db_response jfr;
			jfr.os_code = dbfile.result();
			jfr.success = true;
			co_return jfr;
		}

		collection_response jdatabase::create_collection(create_collection_request _create_collection)
		{
			collection_response response;

			if (_create_collection.collection_name.has_any("./\\:"))
			{
				response.success = false;
				response.message = "Invalid collection name.  Collection names may not contain path characters.";
				return response;
			}

			auto iter = collections_by_name[_create_collection.collection_name];

			if (iter != std::end(collections_by_name)) 
			{
				response.success = false;
				response.message = "Collection exists";
				return response;
			}
			
			jcollection_ref new_collection;
			new_collection.collection_name = _create_collection.collection_name;
			new_collection.model_name = _create_collection.model_name;
			new_collection.max_actors = _create_collection.max_actors;
			new_collection.max_objects = _create_collection.max_objects;

			if (!init_collection_id(new_collection.collection_id))
			{
				response.success = false;
				response.message = "Could not create collection id.";
				return response;
			}

			std::filesystem::path collection_path = map->database_folder.c_str();
			collection_path /= new_collection.collection_name.c_str();
			collection_path /= ".corc";
			new_collection.collection_file_name = collection_path;

			new_collection.collection_size_bytes = schema.get_collection_size(&new_collection, nullptr);
			if (new_collection.collection_size_bytes == null_row)
			{
				response.success = false;
				response.message = "Could not estimate collection size.";
				return response;
			}

			row_range rr;
			jcollection_ref& jref = collections.create(1, rr);
			jref = new_collection;
			jref.data = new dynamic_box();
			response.collection = schema.create_collection(&jref, nullptr);

			collections_by_id.insert_or_assign(jref.collection_id, rr.start);
			collections_by_name.insert_or_assign(jref.collection_name, rr.start);

			return response;
		}

		row_id_type jdatabase::put_string_field(put_string_field_request request)
		{
			return schema.put_string_field(request);
		}

		row_id_type jdatabase::put_time_field(put_time_field_request request)
		{
			return schema.put_time_field(request);
		}

		row_id_type jdatabase::put_integer_field(put_integer_field_request request)
		{
			return schema.put_integer_field(request);
		}

		row_id_type jdatabase::put_double_field(put_double_field_request request)
		{
			return schema.put_double_field(request);
		}

		row_id_type jdatabase::put_query_field(put_named_query_field_request request)
		{
			return schema.put_query_field(request);
		}

		row_id_type jdatabase::put_sql_remote_field(put_named_sql_remote_field_request request)
		{
			return schema.put_sql_remote_field(request);
		}

		row_id_type jdatabase::put_http_remote_field(put_named_http_remote_field_request request)
		{
			return schema.put_http_remote_field(request);
		}

		row_id_type jdatabase::put_file_remote_field(put_named_file_remote_field_request request)
		{
			return schema.put_file_remote_field(request);
		}

		row_id_type jdatabase::put_point_field(put_point_field_request request)
		{
			return schema.put_point_field(request);
		}

		row_id_type jdatabase::put_rectangle_field(put_rectangle_field_request request)
		{
			return schema.put_rectangle_field(request);
		}

		row_id_type jdatabase::put_image_field(put_image_field_request request)
		{
			return schema.put_image_field(request);
		}

		row_id_type jdatabase::put_wave_field(put_wave_field_request request) 
		{
			return schema.put_wave_field(request);
		}

		row_id_type jdatabase::put_sql_remote_field(put_named_sql_remote_field_request request)
		{
			return schema.put_sql_remote_field(request);
		}

		row_id_type jdatabase::put_http_remote_field(put_named_http_remote_field_request request)
		{
			return schema.put_http_remote_field(request);
		}

		row_id_type jdatabase::put_file_remote_field(put_named_file_remote_field_request request)
		{
			return schema.put_file_remote_field(request);
		}

		row_id_type jdatabase::put_point_field(put_point_field_request request)
		{
			return schema.put_point_field(request);
		}

		row_id_type jdatabase::put_rectangle_field(put_rectangle_field_request request)
		{
			return schema.put_rectangle_field(request);
		}

		row_id_type jdatabase::put_image_field(put_image_field_request request)
		{
			return schema.put_image_field(request); 
		}

		row_id_type jdatabase::put_sql_remote_field(put_named_sql_remote_field_request request)
		{
			return schema.put_sql_remote_field(request);
		}

		row_id_type jdatabase::put_http_remote_field(put_named_http_remote_field_request request)
		{
			return schema.put_http_remote_field(request);
		}

		row_id_type jdatabase::put_file_remote_field(put_named_file_remote_field_request request)
		{
			return schema.put_file_remote_field(request);
		}

		row_id_type jdatabase::put_point_field(put_point_field_request request)
		{
			return schema.put_point_field(request);
		}

		row_id_type jdatabase::put_rectangle_field(put_rectangle_field_request request)
		{
			return schema.put_rectangle_field(request);
		}

		row_id_type jdatabase::put_image_field(put_image_field_request request)
		{
			return schema.put_image_field(request);;
		}

		row_id_type jdatabase::put_midi_field(put_midi_field_request request)
		{
			return schema.put_midi_field(request);
		}

		row_id_type jdatabase::put_sql_remote_field(put_named_sql_remote_field_request request)
		{
			return schema.put_sql_remote_field(request);
		}

		row_id_type jdatabase::put_http_remote_field(put_named_http_remote_field_request request)
		{
			return schema.put_http_remote_field(request);
		}

		row_id_type jdatabase::put_file_remote_field(put_named_file_remote_field_request request)
		{
			return schema.put_file_remote_field(request);
		}

		row_id_type jdatabase::put_point_field(put_point_field_request request)
		{
			return schema.put_point_field(request);
		}

		row_id_type jdatabase::put_rectangle_field(put_rectangle_field_request request)
		{
			return schema.put_rectangle_field(request);
		}

		row_id_type jdatabase::put_image_field(put_image_field_request request)
		{
			return schema.put_image_field(request);
		}

		row_id_type jdatabase::put_color_field(put_color_field_request request)
		{
			return schema.put_color_field(request);
		}

		jfield jdatabase::get_field(object_name name)
		{
			auto field_id = schema.find_field(name);
			return schema.get_field(field_id);
		}

		row_id_type jdatabase::put_class(put_class_request request)
		{
			return schema.put_class(request);
		}

		jmodel jdatabase::put_model(jmodel request)
		{
			return schema.put_model(request);
		}

		jclass jdatabase::get_class(object_name name)
		{
			auto id = schema.find_class(name);
			return schema.get_class(id);
		}

		jmodel jdatabase::get_model(object_name name) 
		{
			auto id = schema.find_model(name);
			return schema.get_model(id);
		}

		actor_type jdatabase::create_actor(actor_type _actor)
		{
			
		}

		actor_type jdatabase::get_actor(actor_id_type _actor_id)
		{
			;
		}

		actor_type jdatabase::update_actor(actor_type _actor)
		{
			;
		}

		actor_type jdatabase::put_actor(actor_type _actor)
		{
			;
		}

		actor_type get_command_result(row_id_type _actor)
		{
			;
		}

		actor_type select_object(const actor_select_object& _select)
		{
			;
		}

		actor_command_response create_object(actor_create_object& _create)
		{
			;
		}

		actor_command_response update_object(actor_update_object& _update)
		{
			;
		}

		jdatabase::~jdatabase()
		{
			;
		}
	}
}

