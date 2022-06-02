#pragma once

#include "corona.h"

namespace corona
{
	namespace database
	{

		bool test_ints()
		{
			char test_buffer64[sizeof(int64_t)];
			char test_buffer32[sizeof(int32_t)];
			char test_buffer16[sizeof(int16_t)];
			char test_buffer8[sizeof(int8_t)];

			int64_box box64(test_buffer64);
			int32_box box32(test_buffer32);
			int16_box box16(test_buffer16);
			int16_box box16b(test_buffer16);
			int8_box box8(test_buffer8);

			box64 = 1024;
			box32 = 2048;
			box16 = 2048;

			if (box64 > box32) return false;
			if (box64 >= box32) return false;
			if (box32 < box64) return false;
			if (box32 <= box64) return false;
			if (box32 == box64) return false;
			if (box16 != box32) return false;
			if (box16 < box32) return false;
			if (box16 > box32) return false;
			if (!(box16 == box32)) return false;
			if (!(box16 >= box32)) return false;
			if (!(box16 <= box32)) return false;

			auto d = distance(box64, box32);
			if (d != 1024) return false;

			box16b = 512;
			box16 = box16b;
			if (box16 != box16b) return false;

			return true;
		}
	}
}
