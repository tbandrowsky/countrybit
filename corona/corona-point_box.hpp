#ifndef CORONA_POINT_BOX_H
#define CORONA_POINT_BOX_H


#include <cmath>
#include <iostream>
#include <numeric>
#include <compare>

#include "corona-store_box.hpp"
#include "corona-float_box.hpp"

namespace corona {

	class point
	{
	public:
		double x;
		double y;
		double z;

		point()
		{
			x = 0;
			y = 0;
			z = 0;
		}

		point(double _x, double _y, double _z = 0.0)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		point(int _x, int _y, int _z = 0)
		{
			x = _x;
			y = _y;
			z = _z;
		}
	};


	std::partial_ordering operator<=>(const point& a, const point& b)
	{
		return std::tie(a.z, a.y, a.x) <=> std::tie(b.z, b.y, b.x);
	}

	int compare(const point& a, const point& b)
	{
		return (std::tie(a.z, a.y, a.x) <=> std::tie(b.z, b.y, b.x))._Value;
	}

	int operator<(const point& a, const point& b)
	{
		return compare(a, b) < 0;
	}

	int operator>(const point& a, const point& b)
	{
		return compare(a, b) > 0;
	}

	int operator>=(const point& a, const point& b)
	{
		return compare(a, b) >= 0;
	}

	int operator<=(const point& a, const point& b)
	{
		return compare(a, b) <= 0;
	}

	int operator==(const point& a, const point& b)
	{
		return compare(a, b) == 0;
	}

	int operator!=(const point& a, const point& b)
	{
		return compare(a, b) != 0;
	}

	point operator+(const point& a, const point& b)
	{
		point pt{ a.x + b.x, a.y + b.y, b.z + b.z };
		return pt;
	}

	point operator-(const point& a, const point& b)
	{
		point pt{ a.x - b.x, a.y - b.y, b.z - b.z };
		return pt;
	}

	point operator*(const point& a, const point& b)
	{
		point pt{ a.x * b.x, a.y * b.y, b.z * b.z };
		return pt;
	}

	point operator/(const point& a, const point& b)
	{
		point pt{ a.x / b.x, a.y / b.y, b.z / b.z };
		return pt;
	}

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
			set_data(_src);
			return *this;
		}

		point_box operator = (const point& _src)
		{
			set_value(_src);
			return *this;
		}

		point* operator->()
		{
			point& t = boxed<point>::get_data_ref();
			return &t;
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

	std::ostream& operator <<(std::ostream& output, point_box& src)
	{
		output << src.value().x << ", ";
		return output;
	}

	bool test_points()
	{
		point ptA{ 0,0,0 };
		point ptB{ 0,1,0 };
		point ptC{ 0,0,1 };
		point ptD{ 0,1,1 };

		auto ptTest = ptA + ptB + ptC;
		if (ptTest != ptD) {
			std::cout << "ptTest failed add" << std::endl;
			return false;
		}

		char box[2048];
		point_box pb(box);

		pb = ptA;

		ptD = pb;
		if (ptD != ptA) {
			std::cout << "box test failed" << std::endl;
			return false;
		}

		return true;
	}

}

#endif