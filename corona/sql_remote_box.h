#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
#include "remote_box.h"
#include "jfield.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

		class sql_remote_box
		{
			boxed<sql_remote_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			sql_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_idx);
			sql_remote_box(sql_remote_box& _src);
			sql_remote_box operator = (const sql_remote_box& _src);
			sql_remote_box operator = (sql_remote_instance _src);
			operator sql_remote_instance& ();
			sql_remote_instance value();
		};

	}
}
