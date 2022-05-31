#pragma once

namespace corona
{
	namespace database
	{

		struct rectangle 
		{
			double x, y, w, h;
		};

		class rectangle_math
		{
		public:
			static point center(rectangle i)
			{
				return point{ std::midpoint(i.x, i.x + i.w), std::midpoint(i.y, i.y + i.h)};
			}
			static bool contains(rectangle r, double x, double y)
			{
				return (r.x <= x) &&
					((r.x + r.w) > x) &&
					(r.y <= y) &&
					((r.y + r.h) > y);
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
				set_data(_src);
				return *this;
			}

			rectangle_box operator = (rectangle _src)
			{
				set_value(_src);
				return *this;
			}

			rectangle* operator->()
			{
				rectangle& t = boxed<rectangle>::get_data_ref();
				return &t;
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
