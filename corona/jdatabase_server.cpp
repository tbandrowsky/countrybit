
#include "corona.h"

namespace corona
{
	namespace database
	{

		jdatabase::jdatabase(application* _application) : current_application(_application)
		{

		}

		task<db_response> jdatabase::open(open_db_request _open)
		{
			int success = 1;
			database_box.open(current_application, _open.filename);
			map = database_box.get_object<jdatabase_control_map>(0);
			collections_by_id = collections_by_id_type::get_sorted_index(&database_box, map->collections_by_id_location);
			db_response jfr;
			jfr.success = true;
			co_return jfr;
		}

		task<db_response> jdatabase::create(create_db_request _create)
		{
			os_result last_err;

			int success = 1;

			int64_t schema_size_bytes = jschema::get_box_size(_create.num_classes, _create.num_fields, _create.total_class_fields, true);
			int64_t collection_by_name_bytes = collections_by_name_type::get_box_size();
			int64_t collection_by_id_bytes = collections_by_id_type::get_box_size();

			database_box.create(current_application, _create.database_filename);
			map = database_box.allocate<jdatabase_control_map>(1);

			std::filesystem::path dbpath = _create.database_filename.c_str();
			std::filesystem::path dbfolder = dbpath.parent_path();

			map->filename = dbpath;
			map->database_folder = dbfolder;

			schema = jschema::create_schema(&database_box, _create.num_classes, true, map->schema_location);
			collections_by_id = collections_by_id_type::create_sorted_index(&database_box, map->collections_by_id_location);
			collections_by_name = collections_by_name_type::create_sorted_index(&database_box, map->collections_by_name_location);
			database_box.commit();

			db_response jfr;
			jfr.success = true;
			co_return jfr;
		}

		collection_response jdatabase::create_collection(create_collection_request _create_collection)
		{
			collection_response response;

			try {

				if (_create_collection.collection_name.has_any("./\\:"))
				{
					response.success = false;
					response.message = "Invalid collection name.  Collection names may not contain path characters.";
					return response;
				}

				if (collections_by_name.contains(_create_collection.collection_name))
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
				new_collection.data = nullptr;

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

				collections_by_id.insert_or_assign(new_collection.collection_id, new_collection);
				collections_by_name.insert_or_assign(new_collection.collection_name, new_collection.collection_id);

				database_box.commit();

				response = get_collection(new_collection.collection_id);
			}
			catch (std::exception exc)
			{
				response.success = false;
				response.message = exc.what();
			}

			return response;
		}

		collection_response jdatabase::get_collection(object_name _name)
		{
			collection_response response;
			try {

				response.success = false;
				if (!collections_by_name.contains(_name)) {
					response.message = "[" + _name + "] not found";
					return response;
				}
				auto collection_id = collections_by_name[_name].second;
				response = get_collection(collection_id);
			}
			catch (std::exception exc)
			{
				response.success = false;
				response.message = exc.what();
			}
			return response;
		}

		collection_response jdatabase::get_collection(collection_id_type _id)
		{
			collection_response response;
			if (!collections_by_id.contains(_id)) {
				response.message = "collection not found";
				return response;
			}
			auto ref = collections_by_id[_id];
			if (ref.second.data == nullptr) {
				ref.second.data = new persistent_box();
				ref.second.data->open(current_application, ref.second.collection_file_name);
			}
			response.collection = jcollection(&schema, &ref.second);
			response.success = true;
			return response;
		}

		field_response jdatabase::put_point_field(put_point_field_request request)
		{
			return field_invoke<put_point_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_point_field(r); }, request);
		}

		field_response jdatabase::put_rectangle_field(put_rectangle_field_request request)
		{
			return field_invoke<put_rectangle_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_rectangle_field(r); }, request);
		}

		field_response jdatabase::put_string_field(put_string_field_request request)
		{
			return field_invoke<put_string_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_string_field(r); }, request);
		}

		field_response jdatabase::put_time_field(put_time_field_request request)
		{
			return field_invoke<put_time_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_time_field(r); }, request);
		}

		field_response jdatabase::put_integer_field(put_integer_field_request request)
		{
			return field_invoke<put_integer_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_integer_field(r); }, request);
		}

		field_response jdatabase::put_double_field(put_double_field_request request)
		{
			return field_invoke<put_double_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_double_field(r); }, request);
		}

		field_response jdatabase::put_color_field(put_color_field_request request)
		{
			return field_invoke<put_color_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_color_field(r); }, request);
		}

		field_response jdatabase::put_query_field(put_filter_field_request request)
		{
			return field_invoke<put_filter_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_query_field(r); }, request);
		}

		field_response jdatabase::put_sql_remote_field(put_sql_remote_field_request request)
		{
			return field_invoke<put_sql_remote_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_sql_remote_field(r); }, request);
		}

		field_response jdatabase::put_http_remote_field(put_http_remote_field_request request)
		{
			return field_invoke<put_http_remote_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_http_remote_field(r); }, request);
		}

		field_response jdatabase::put_file_remote_field(put_file_remote_field_request request)
		{
			return field_invoke<put_file_remote_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_file_remote_field(r); }, request);
		}

		field_response jdatabase::put_image_field(put_image_field_request request)
		{
			return field_invoke<put_image_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_image_field(r); }, request);
		}

		field_response jdatabase::put_wave_field(put_wave_field_request request)
		{
			return field_invoke<put_wave_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_wave_field(r); }, request);
		}

		field_response jdatabase::get_field(object_name name)
		{
			return field_invoke<object_name>(name.c_str(), [this](auto& r) { return schema.find_field(r); }, name);
		}

		class_response jdatabase::put_class(put_class_request request)
		{
			return class_invoke<put_class_request>(request.class_name.c_str(), [this](auto& r) { return schema.put_class(r); }, request);
		}

		class_response jdatabase::get_class(object_name name)
		{
			return class_invoke<object_name>(name.c_str(), [this](auto& r) { return schema.find_class(r); }, name);
		}

		model_response jdatabase::put_model(jmodel request)
		{
			return schema_put_named<model_response, jmodel>(request, [this](jmodel& r) { schema.put_model(r); }, [this](object_name& r) { return schema.get_model(r); });
		}

		model_response jdatabase::get_model(object_name name)
		{
			return schema_get_by_name<model_response, jmodel>(name, [this](object_name& r) { return schema.get_model(r); });
		}

		actor_response jdatabase::put_actor(jactor _actor)
		{
			return actor_invoke<jactor>(_actor.actor_name.c_str(), [this](jcollection& col, auto& r) { return col.put_actor(r); }, _actor);
		}

		actor_response jdatabase::get_actor(get_actor_request _request)
		{
			return actor_invoke<get_actor_request>(_request.name.c_str(), [this](jcollection& col, auto& r) { return col.find_actor(r.name); }, _request);
		}

		actor_state_response jdatabase::get_actor_state(get_actor_request _request)
		{
			return command_invoke<get_actor_request>(_request, [](jcollection& _collection, get_actor_request& p) {
				auto actor = _collection.find_actor(p.name);
				return _collection.get_actor_state(actor);
				});
		}

		actor_state_response jdatabase::select_object(select_object_request _select)
		{
			return command_invoke<select_object_request>(_select, [](jcollection& _collection, select_object_request& p) {
				return _collection.select_object(p);
				});
		}

		actor_state_response jdatabase::create_object(create_object_request _create)
		{
			return command_invoke<create_object_request>(_create, [](jcollection& _collection, create_object_request& p) {
				return _collection.create_object(p);
				});
		}

		actor_state_response jdatabase::update_object(update_object_request _update)
		{
			return command_invoke<update_object_request>(_update, [](jcollection& _collection, update_object_request& p) {
				return _collection.update_object(p);
				});
		}

		jdatabase::~jdatabase()
		{
			;
		}
	}
}