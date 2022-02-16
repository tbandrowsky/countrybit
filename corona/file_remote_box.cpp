
#include "file_remote_box.h"


namespace countrybit
{
	namespace database
	{
		file_remote_box::file_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		file_remote_box::file_remote_box(file_remote_box& _src) : instance(_src.instance)
		{
			;
		}

		file_remote_box file_remote_box::operator = (const file_remote_box& _src)
		{
			instance = _src.instance;
			return *this;
		}

		file_remote_box file_remote_box::operator = (file_remote_instance _src)
		{
			instance = _src;
			return *this;
		}

		file_remote_box::operator file_remote_instance& ()
		{
			file_remote_instance& t = instance.get_data_ref();
			return t;
		}

		file_remote_instance file_remote_box::value() const { return instance.get_value(); }

	}
}

