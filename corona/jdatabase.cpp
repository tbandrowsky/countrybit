
#include "jdatabase.h"



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

			countrybit::system::file dbfile = application->create_file(_create.filename.c_str());

			if (dbfile.success()) {
				map = database_box.allocate<jdatabase_control_map>(1);
				schema = jschema::create_schema(&database_box, _create.num_classes, _create.num_fields, _create.num_class_fields, _create.num_queries, _create.num_sql_remotes, _create.num_http_remotes, _create.num_http_remotes, map->schema_location);
				schema.add_standard_fields();
				collections = collection_table_type::create_table(&database_box, _create.num_collections, map->collections_location);
				collections_by_id = collections_by_id_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_id_location);
				collections_by_name = collections_by_name_type::create_sorted_index(&database_box, _create.num_collections, map->collections_by_name_location);
				auto result = co_await dbfile.write(0, database_box.data(), database_box.size());
			}

			jdatabase_file_response jfr;
			jfr.result = dbfile.result();
			jfr.success = true;
			co_return jfr;
		}

		task<jdatabase_collection_response> jdatabase::create_collection(jdatabase_create_collection _create_collection)
		{

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

