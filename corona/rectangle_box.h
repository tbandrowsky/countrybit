#pragma once

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>

#include "store_box.h"
#include "point_box.h"
#include "float_box.h"
#include "constants.h"

namespace countrybit
{
	namespace database
	{

		struct rectangle 
		{
			point corner;
			point size;
		};

		class rectangle_math
		{
		public:
			static point center(rectangle i)
			{
				return point{ std::midpoint(i.corner.x, i.corner.x + i.size.x), std::midpoint(i.corner.y, i.corner.y + i.size.y)};
			}
		};

		class rectangle_box : protected boxed<rectangle>
		{
		public:
			rectangle_box(char* t) : boxed<rectangle>(t)
			{
				;
			}

			rectangle_box operator = (const rectangle_box& _src)
			{
				boxed<rectangle>::operator =(_src);
				return *this;
			}

			rectangle_box operator = (rectangle_box _src)
			{
				boxed<rectangle>::operator =(_src);
				return *this;
			}

			rectangle_box operator = (rectangle _src)
			{
				boxed<rectangle>::operator =(_src);
				return *this;
			}

			operator rectangle& ()
			{
				rectangle& t = boxed<rectangle>::get_data_ref();
				return t;
			}

			rectangle value() const { return boxed<rectangle>::get_value(); }

		};

	}
}
