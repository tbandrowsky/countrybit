#pragma once

#include "store_box.h"
#include "float_box.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		struct color
		{
			double red;
			double green;
			double blue;
			double alpha;
		};

		class color_math
		{
		public:
		};

		class color_box : protected boxed<color>
		{
		public:
			color_box(char* t) : boxed<color>(t)
			{
				;
			}

			color_box operator = (const color_box& _src)
			{
				boxed<color>::operator =(_src);
				return *this;
			}

			color_box operator = (color _src)
			{
				boxed<color>::operator =(_src);
				return *this;
			}

			operator color& ()
			{
				color& t = boxed<color>::get_data_ref();
				return t;
			}

			color value() const { return boxed<color>::get_value(); }

		};
	}
}
