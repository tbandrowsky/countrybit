
#include "http_remote_box.h"

namespace countrybit
{
	namespace database
	{


		http_remote_box::http_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		http_remote_box::http_remote_box(http_remote_box& _src) : instance(_src.instance)
		{
			;
		}

		http_remote_box http_remote_box::operator = (const http_remote_box& _src)
		{
			instance = _src.instance;
			return *this;
		}

		http_remote_box http_remote_box::operator = (http_remote_instance _src)
		{
			instance = _src;
			return *this;
		}

		http_remote_box::operator http_remote_instance& ()
		{
			http_remote_instance& t = instance.get_data_ref();
			return t;
		}

		http_remote_instance http_remote_box::value() const { return instance.get_value(); }




	}
}
