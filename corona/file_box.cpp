
#include "corona.h"

namespace corona
{
	namespace database
	{
		file_box::file_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx) : boxed<file_remote_instance>(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		file_box::file_box(file_box& _src) : boxed<file_remote_instance>(_src)
		{
			;
		}

		file_box file_box::operator = (const file_box& _src)
		{
			set_data(_src);
			return *this;
		}

		file_box file_box::operator = (file_remote_instance _src)
		{
			set_value(_src);
			return *this;
		}

		file_box::operator file_remote_instance& ()
		{
			file_remote_instance& t = get_data_ref();
			return t;
		}

		file_remote_instance file_box::value() const { return get_value(); }

	}
}

