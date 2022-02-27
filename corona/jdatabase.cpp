
#include "jdatabase.h"



namespace countrybit 
{
	namespace database 
	{

		jdatabase::jdatabase(system::application* _application) :
			application( _application )
		{

		}

		task<os_result> jdatabase::open(jdatabase_open _open)
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
					collections = jcollection_set::get_sorted_index(&database_box, map->collections_location);
				}
			} 
			co_return dbfile.result();
		}

		task<os_result> jdatabase::create(jdatabase_create _create)
		{
			os_result last_err;

			int success = 1;

			int64_t schema_size_bytes =	jschema::get_box_size(_create.num_classes, _create.num_fields, _create.num_class_fields, _create.num_queries, _create.num_sql_remotes, _create.num_http_remotes, _create.num_http_remotes);
			int64_t collection_size_bytes = jcollection_set::get_box_size(_create.num_collections);

			database_box.init(collection_size_bytes + schema_size_bytes + sizeof(jdatabase_control_map));

			countrybit::system::file dbfile = application->create_file(_create.filename.c_str());

			if (dbfile.success()) {
				map = database_box.allocate<jdatabase_control_map>(1);
				schema = jschema::create_schema(&database_box, _create.num_classes, _create.num_fields, _create.num_class_fields, _create.num_queries, _create.num_sql_remotes, _create.num_http_remotes, _create.num_http_remotes, map->schema_location);
				collections = jcollection_set::create_sorted_index(&database_box, _create.num_collections, map->collections_location);
				co_await dbfile.write(0, database_box.data(), database_box.size());
			}

			co_return dbfile.result();
		}

		jdatabase::~jdatabase()
		{
			;
		}
	}
}

