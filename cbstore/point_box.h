#pragma once

#include "store_box.h"
#include "float_box.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		struct point
		{
			double x;
			double y;
			double z;
		};

		class point_math
		{
		public:
			static point midpoint(point i, point j)
			{
				return point{ std::midpoint(i.x, j.x), std::midpoint(i.y, j.y), std::midpoint(i.z, j.z) };
			}
			static point lerp(point i, point j, point k)
			{
				return point{ std::lerp(i.x, j.x, k.x), std::lerp(i.y, j.y, k.y), std::lerp(i.z, j.z, k.z) };
			}
			static double distance(point i, point j)
			{
				return std::hypot((i.x - j.x), (i.y - j.y), (i.z - j.z));
			}
			static double dot(point i, point j)
			{
				return i.x * j.x + i.y * j.y + i.z * j.z;
			}
			static point cross(point a, point b)
			{
				// a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1
				return point{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
			}
			static double magnitude(point i)
			{
				return sqrt(i.x * i.x + i.y * i.y + i.z * i.z);
			}
			static double angle(point i, point j)
			{
				double costheta = dot(i, j) / magnitude(i) * magnitude(j);
				double angle = float_math<double>::acos(costheta);
				return angle;
			}
			static point rotate_2d(point origin, point pt, double angle)
			{
				double sinangle = float_math<double>::sin(angle);
				double cosangle = float_math<double>::cos(angle);
				point pold = pt - origin;
				point pnew;
				pnew.x = pold.x * cosangle - pold.y * sinangle;
				pnew.y = pold.x * sinangle - pold.y * cosangle;
				point presult = pnew + origin;
				return presult;
			}
		};

		class point_box : protected boxed<point>
		{
		public:
			point_box(char* t) : boxed<point>(t)
			{
				;
			}

			point_box operator = (const point_box& _src)
			{
				boxed<point>::operator =(_src);
				return *this;
			}

			point_box operator = (point _src)
			{
				boxed<point>::operator =(_src);
				return *this;
			}

			operator point& ()
			{
				point& t = boxed<point>::get_data_ref();
				return t;
			}

			point value() const { return boxed<point>::get_value(); }

		};

		int compare(const point& a, const point& b);
		int operator<(const point& a, const point& b);
		int operator>(const point& a, const point& b);
		int operator>=(const point& a, const point& b);
		int operator<=(const point& a, const point& b);
		int operator==(const point& a, const point& b);
		int operator!=(const point& a, const point& b);

		point operator+(const point& a, const point& b);
		point operator-(const point& a, const point& b);
		point operator*(const point & a, const point & b);
		point operator/(const point& a, const point& b);
		std::ostream& operator <<(std::ostream& output, point_box& src);
	}
}
