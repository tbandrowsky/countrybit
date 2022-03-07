
#include "jdatabase.h"

#include <filesystem>

namespace countrybit 
{
	namespace database 
	{

		jdatabase::jdatabase(system::application* _application) :
			application( _application )
		{

		}

		task<jdatabase_file_response> jdatabase::open(jdatabase_open _open)
		{
			int success = 1;
			countrybit::system::file dbfile = application->open_file(_open.filename.c_str(), countrybit::system::file_open_types::open_existing);
			if (dbfile.success()) {
				auto sz = dbfile.size();
				database_box.init(sz);
				co_await dbfile.read(0, database_box.data(), sz);
				if (dbfile.success()) {
					map = database_box.unpack<jdatabase_control_map>(0);
					schema = jschema::get_schema(&database_box, map->schema_location);
					collections = collection_table_type::get_table(&database_box, map->collections_location);
					collections_by_id = collections_by_id_type::get_sorted_index(&database_box, map->collections_by_id_location);
					collections_by_name = collections_by_name_type::get_sorted_index(&database_box, map->collections_by_name_location);
				}
			} 
			jdatabase_file_response jfr;
			jfr.result = dbfile.result();
			jfr.success = true;
			co_return jfr;
		}

		task<jdatabase_file_response> jdatabase::create(jdatabase_create _create)
		{
			os_result last_err;

			int success = 1;

			int64_t schema_size_bytes =	jschema::get_box_size(_create.num_classes, _create.num_fields, _create.num_class_fields, _create.num_queries, _create.num_sql_remotes, _create.num_http_remotes, _create.num_http_remotes);
			int64_t collection_size_bytes = collection_table_type::get_box_size(_create.num_collections);
			int64_t collection_by_name_bytes = collections_by_name_type::get_box_size(_create.num_collections);
			int64_t collection_by_id_bytes = collections_by_id_type::get_box_size(_create.num_collections);

			database_box.init(collection_size_bytes + schema_size_bytes + collection_by_name_bytes + collection_by_id_bytes + sizeof(jdatabase_control_map));
			map = database_box.allocate<jdatabase_control_map>(1);

			std::filesystem::path dbpath = _create.database_filename.c_str();
			std::filesystem::path dbfolder = dbpath.parent_path();

			map->filename = dbpath;
			map->database_folder = dbfolder;

			schema = jschema::create_schema(&database_box, _create.num_classes, _create.num_fields, _create.num_class_fields, _create.num_queries, _create.num_sql_remotes, _create.num_http_remotes, _create.num_http_remotes, map->schema_location);
			schema.add_standard_fields();
			collections = collection_table_type::create_table(&database_box, _create.num_collections, map->collections_location);
			collections_by_id = collections_by_id_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_id_location);
			collections_by_name = collections_by_name_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_name_location);

			countrybit::system::file dbfile = application->create_file(_create.database_filename);

			if (dbfile.success()) {
				auto result = co_await dbfile.write(0, database_box.data(), database_box.size());
			}

			jdatabase_file_response jfr;
			jfr.result = dbfile.result();
			jfr.success = true;
			co_return jfr;
		}

		task<jdatabase_collection_response> jdatabase::create_collection(jdatabase_create_collection _create_collection)
		{
			jdatabase_collection_response response;

			if (_create_collection.collection_name.has_any("./\\:"))
			{
				response.success = false;
				response.message = "Invalid collection name.  Collection names may not contain path characters.";
				co_return response;
			}

			auto iter = collections_by_name[_create_collection.collection_name];

			if (iter != std::end(collections_by_name)) 
			{
				response.success = false;
				response.message = "Collection exists";
				co_return response;
			}

			row_id_type  clsid = schema.find_class(_create_collection.collection_class.class_name);
			if (clsid == null_row) 
			{
				response.success = false;
				response.message = "Class is not defined.";
				co_return response;
			}

			row_id_type classes[2] = { clsid, null_row };
			uint64_t box_size = schema.estimate_collection_size(_create_collection.number_of_objects, classes);

			if (box_size == null_row) 
			{
				response.success = false;
				response.message = "Could not estimate collection size.";
				co_return response;
			}

			collection_id_type colid;
			if (!init_collection_id(colid)) 
			{
				response.success = false;
				response.message = "Could not create collection id.";
				co_return response;
			}

			row_range rr;
			jcollection_ref& jref = collections.create(1, rr);
			jref.collection_size_bytes = box_size;
			jref.collection_name = _create_collection.collection_name;
			jref.collection_id = colid;
			std::filesystem::path collection_path = map->database_folder.c_str();
			collection_path /= jref.collection_name.c_str();
			collection_path /= ".corc";
			jref.collection_file_name = collection_path;

			co_return response;
		}

		task<jdatabase_object_response> jdatabase::create_object(jdatabase_create_object _request)
		{

		}

		task<jdatabase_object_response> jdatabase::get_object(jdatabase_get_object _request)
		{

		}

		task<jdatabase_object_response> jdatabase::put_object(jdatabase_put_object _request)
		{

		}

		task<jdatabase_object_response> jdatabase::update_object(jdatabase_update_object _request)
		{

		}

		jcollection jdatabase::query(named_query_properties_type& query)
		{

		}

		jdatabase::~jdatabase()
		{
			;
		}
	}
}

