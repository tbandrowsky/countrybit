
#include "sql_remote_box.h"


namespace countrybit
{
	namespace database
	{
		sql_remote_box::sql_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		sql_remote_box::sql_remote_box(sql_remote_box& _src) : instance(_src.instance)
		{
			;
		}

		sql_remote_box sql_remote_box::operator = (const sql_remote_box& _src)
		{
			instance = _src.instance;
			return *this;
		}

		sql_remote_box sql_remote_box::operator = (sql_remote_instance _src)
		{
			instance = _src;
			return *this;
		}

		sql_remote_box::operator sql_remote_instance& ()
		{
			sql_remote_instance& t = instance.get_data_ref();
			return t;
		}

		sql_remote_instance sql_remote_box::value()
		{
			return instance.get_value(); 
		}

	}
}

