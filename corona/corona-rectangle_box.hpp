#ifndef CORONA_RECTANGLE_H
#define CORONA_RECTANGLE_H

namespace corona {

	struct rectangle
	{
		double x = 0, y = 0, w = 0, h = 0;

		double right() const { return x + w; }
		double bottom() const { return y + h; }
	};

	class rectangle_math
	{
	public:
		static point center(rectangle i)
		{
			return point{ std::midpoint(i.x, i.x + i.w), std::midpoint(i.y, i.y + i.h) };
		}

		static bool contains(rectangle r, double x, double y)
		{
			return (r.x <= x) && (r.right() > x) &&
				(r.y <= y) && (r.bottom() > y);
		}

		static rectangle deflate(rectangle r0, rectangle amt)
		{
			rectangle r1 = r0;
			r1.x += amt.x;
			r1.y += amt.y;
			r1.w -= (amt.w + amt.x);
			r1.h -= (amt.h + amt.y);
			if (r1.h < 0) r1.h = 0;
			if (r1.w < 0) r1.w = 0;
			return r1;
		}

		static rectangle join(rectangle r0, rectangle r1)
		{
			rectangle rv = r0;
			double ex, ey;

			rv.x = std::min(r0.x, r1.x);
			rv.y = std::min(r0.y, r1.y);
			ex = std::max(r0.right(), r1.right());
			ey = std::max(r0.bottom(), r1.bottom());

			return r1;
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

#endif
