
#include "corona.h"

namespace corona
{
	namespace database
	{


		http_box::http_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx) : boxed<http_instance>(t), schema(_schema), the_class(_class), slice(_slice)
		{
			;
		}

		http_box::http_box(http_box& _src) : boxed<http_instance>(_src)
		{
			;
		}

		http_box http_box::operator = (const http_box& _src)
		{
			set_data(_src);
			return *this;
		}

		http_box http_box::operator = (http_instance _src)
		{
			set_value(_src);
			return *this;
		}

		http_box::operator http_instance& ()
		{
			http_instance& t = get_data_ref();
			return t;
		}

		http_instance http_box::value() const { return get_value(); }
	}
}
