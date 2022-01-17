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

		class jdatabase
		{
			countrybit::system::file	database_file;

			dynamic_box					database_box;
			jschema						schema;

			jdatabase();

		public:

		};
	}
};

