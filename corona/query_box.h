#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
#include "jfield.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

		class jcollection;
		class jdatabase;
		class jschema;
		class jslice;

		class query_box 
		{
			boxed<query_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;
			int field_index;

		public:

			query_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_index);
			query_box(query_box& _src);
			query_box operator = (const query_box& _src);
			query_box operator = (query_instance _src);
			operator query_instance& ();
			query_instance value() const;

			void run();

		};

	}
}
