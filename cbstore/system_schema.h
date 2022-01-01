#pragma once
#pragma once

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
		class system_schema
		{
			dynamic_box schema_box;
			jschema schema;

			row_id_type create_database_request_id;
			row_id_type open_database_request_id;

		public:

			system_schema();

			static jcollection parse_schema( row_id_type message_type, std::string& message );

		};
	}
};

