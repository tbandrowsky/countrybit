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

		int compare(const point& a, const point& b);
		int operator<(const point& a, const point& b);
		int operator>(const point& a, const point& b);
		int operator>=(const point& a, const point& b);
		int operator<=(const point& a, const point& b);
		int operator==(const point& a, const point& b);
		int operator!=(const point& a, const point& b);

		point operator+(const point& a, const point& b);
		point operator-(const point& a, const point& b);
		point operator*(const point& a, const point& b);
		point operator/(const point& a, const point& b);

		class point_math
		{
		public:
			static point midpoint(const point& i, const point& j)
			{
				return point{ std::midpoint(i.x, j.x), std::midpoint(i.y, j.y), std::midpoint(i.z, j.z) };
			}
			static point lerp(const point& i, const point& j, const point& k)
			{
				return point{ std::lerp(i.x, j.x, k.x), std::lerp(i.y, j.y, k.y), std::lerp(i.z, j.z, k.z) };
			}
			static double distance(const point& i, const point& j)
			{
				return std::hypot((i.x - j.x), (i.y - j.y), (i.z - j.z));
			}
			static double dot(const point& i, const point& j)
			{
				return i.x * j.x + i.y * j.y + i.z * j.z;
			}
			static point cross(const point& a, const point& b)
			{
				// a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1
				return point{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
			}
			static double magnitude(const point& i)
			{
				return sqrt(i.x * i.x + i.y * i.y + i.z * i.z);
			}
			static double angle(const point& i, const point& j)
			{
				double costheta = dot(i, j) / magnitude(i) * magnitude(j);
				double angle = float_math<double>::acos(costheta);
				return angle;
			}
			static point rotate_2d(const point& origin, const point& pt, double angle)
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

		class point_box : public boxed<point>
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

			operator const point& () const
			{
				const point& t = boxed<point>::get_data_ref();
				return t;
			}

			point value() const { return boxed<point>::get_value(); }

		};

		std::ostream& operator <<(std::ostream& output, point_box& src);


		int compare(const point_box& a, const point_box& b)
		{
			double d = point_math::magnitude(a)-point_math::magnitude(b);
			if (d < 0.0) 
			{
				return -1;
			} 
			else if (d == 0.0)
			{
				double an = point_math::angle(a, b);
				if (an < 0.0)
				{
					return -1;
				}
				else if (an > 0.0)
				{
					return 1.0;
				}
				return 0;
			}
			else if (d > 0.0)
			{
				return 1;
			}
		}

		int operator<(const point_box& a, const point_box& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const point_box& a, const point_box& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const point_box& a, const point_box& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const point_box& a, const point_box& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const point_box& a, const point_box& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const point_box& a, const point_box& b)
		{
			return compare(a, b) != 0;
		}

		int compare(const point_box& a, const point& b)
		{
			char dummy[sizeof(point)];
			point_box pb(dummy);
			pb = b;
			return compare(a, pb);
		}

		int operator<(const point_box& a, const point& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const point_box& a, const point& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const point_box& a, const point& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const point_box& a, const point& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const point_box& a, const point& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const point_box& a, const point& b)
		{
			return compare(a, b) != 0;
		}

		bool test_points();
	}
}
