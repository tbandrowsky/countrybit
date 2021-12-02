#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>

#include "jobject.h"


namespace countrybit
{
	namespace database
	{
		class database;

		class collection_query
		{
			dynamic_box box;
			jschema query_schema;
			row_id_type schema_id;
			jschema *database_schema;
			row_id_type root_class_id;

		public:

			collection_query(jschema *_database_schema) : database_schema(_database_schema), schema_id(null_row)
			{
				box.init(1 << 20);
				query_schema = jschema::create_schema(&box, 10, 200, 500, schema_id);
			}

			collection_query& begin_query(collection_id_type collection_id, std::vector<row_id_type> object_class_ids)
			{
				using namespace countrybit::database;

				box.init(1 << 20);
				query_schema = jschema::create_schema(&box, 20, 200, 500, schema_id);
				query_schema.create_standard_fields();

				jschema::create_class_request request;
				request.class_name = "queryRoot";
				request.class_description = "Root class of query";
				request.field_ids = {  };
				root_class_id = query_schema.create_class(request);

				return *this;
			}

			collection_query& add_level(row_id_type object_member_id, std::vector<row_id_type> fields)
			{
				return *this;
			}

		};

		class database
		{

		};
	}
};

