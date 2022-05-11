
#include "pch.h"

namespace corona
{
	namespace database
	{


		http_remote_box::http_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_idx) : boxed<http_remote_instance>(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		http_remote_box::http_remote_box(http_remote_box& _src) : boxed<http_remote_instance>(_src)
		{
			;
		}

		http_remote_box http_remote_box::operator = (const http_remote_box& _src)
		{
			set_data(_src);
			return *this;
		}

		http_remote_box http_remote_box::operator = (http_remote_instance _src)
		{
			set_value(_src);
			return *this;
		}

		http_remote_box::operator http_remote_instance& ()
		{
			http_remote_instance& t = get_data_ref();
			return t;
		}

		http_remote_instance http_remote_box::value() const { return get_value(); }




	}
}
