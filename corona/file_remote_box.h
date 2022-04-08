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


		class file_remote_box : public boxed<file_remote_instance>
		{
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			file_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_idx);
			file_remote_box(file_remote_box& _src);
			file_remote_box operator = (const file_remote_box& _src);
			file_remote_box operator = (file_remote_instance _src);
			operator file_remote_instance& ();
			file_remote_instance value() const;
		};
	}
}
