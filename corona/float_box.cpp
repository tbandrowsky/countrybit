#pragma once

#include "pch.h"

namespace corona
{
	namespace database
	{
		double distance(double a, double b)
		{
			return std::abs(a - b);
		}

		int compare(double a, double b)
		{
			return (a <=> b)._Value;
		}

		bool test_doubles()
		{
			char test_buffer64[sizeof(double)];
			char test_buffer32[sizeof(float)];
			char test_buffer32b[sizeof(float)];

			double_box box64(test_buffer64);
			float_box box32(test_buffer32);
			float_box box32b(test_buffer32b);

			box64 = 1024.0;
			box32 = 2048.0;
			box32b = 2048.0;

			if (box64 > box32) return false;
			if (box64 >= box32) return false;
			if (box32 < box64) return false;
			if (box32 <= box64) return false;
			if (box32 == box64) return false;
			if (box32b != box32) return false;
			if (box32b < box32) return false;
			if (box32b > box32) return false;
			if (!(box32b == box32)) return false;
			if (!(box32b >= box32)) return false;
			if (!(box32b <= box32)) return false;

			auto d = distance(box64, box32);
			if (d != 1024) return false;

			box32 = 512;
			box32b = box32;
			if (box32b != box32) return false;

			return true;
		}
	}
}
