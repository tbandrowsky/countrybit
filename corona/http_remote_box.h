#pragma once

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


		class http_remote_box
		{
			boxed<http_remote_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			http_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_idx);
			http_remote_box(http_remote_box& _src);
			http_remote_box operator = (const http_remote_box& _src);
			http_remote_box operator = (http_remote_instance _src);
			operator http_remote_instance& ();
			http_remote_instance value() const;
		};

	}
}
