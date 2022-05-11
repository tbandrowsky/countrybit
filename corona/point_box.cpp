#pragma once

#include "pch.h"

namespace corona
{
	namespace database
	{
		auto operator<=>(const point& a, const point& b)
		{
			return std::tie(a.z, a.y, a.x) <=> std::tie(b.z, b.y, b.x);
		}

		double distance(const point& a, const point& b)
		{
			return point_math::distance(a, b);
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

		std::ostream& operator <<(std::ostream& output, point_box& src)
		{
			output << src.value().x << ", ";
			return output;
		}

		point operator+(const point& a, const point& b)
		{
			point pt { a.x + b.x, a.y + b.y, b.z + b.z };
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
}
