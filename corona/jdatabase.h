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
		class jdatabase
		{
			dynamic_box data_box;
			countrybit::system::file data_file;


			jschema schema;

			jdatabase();

		public:

			static jcollection create_database(jcollection& request);
			static jcollection open_database(jcollection& response);

		};
	}
};

