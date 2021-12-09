#pragma once

#include "store_box.h"
#include "float_box.h"
#include "point_box.h"
#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

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

	}
}
