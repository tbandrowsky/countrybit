
#include "corona.h"

namespace corona
{
	namespace database
	{
		sql_remote_box::sql_remote_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx) : boxed<sql_instance>(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		sql_remote_box::sql_remote_box(sql_remote_box& _src) : boxed<sql_instance>(_src)
		{
			;
		}

		sql_remote_box sql_remote_box::operator = (const sql_remote_box& _src)
		{
			set_data(_src);
			return *this;
		}

		sql_remote_box sql_remote_box::operator = (sql_instance _src)
		{
			set_value(_src);
			return *this;
		}

		sql_remote_box::operator sql_instance& ()
		{
			sql_instance& t = get_data_ref();
			return t;
		}

		sql_instance* sql_remote_box::operator->()
		{
			sql_instance& t = get_data_ref();
			return &t;
		}

		sql_instance sql_remote_box::value()
		{
			return get_value(); 
		}

	}
}

