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

		public:

			query_box(char* t, jschema *_schema, jclass *_class, jslice *_slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
			{
				;
			}

			query_box(query_box& _src) : instance(_src.instance)
			{
				;
			}

			query_box operator = (const query_box& _src)
			{
				instance = _src.instance;
				return *this;
			}

			query_box operator = (query_instance _src)
			{
				instance = _src;
				return *this;
			}

			operator query_instance& ()
			{
				query_instance& t = instance.get_data_ref();
				return t;
			}

			query_instance value() const { return instance.get_value(); }

			jcollection execute()
			{

			}
		};

	}
}
