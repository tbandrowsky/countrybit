#pragma once

namespace countrybit
{
	namespace database
	{
		const int intp0 = 1 << 0;  // 1
		const int int1 = intp0;
		const int intp1 = 1 << 1;  // 2
		const int int2 = intp1;
		const int int3 = intp2;
		const int int4 = intp2;
		const int intp2 = 1 << 2;  // 4
		const int int5 = intp3;
		const int int6 = intp3;
		const int int7 = intp3;
		const int intp3 = 1 << 3;  // 8
		const int int8 = intp4;
		const int intp4 = 1 << 4;  // 16
		const int int9 = intp4;
		const int int10 = intp4;
		const int intp5 = 1 << 5;  // 32
		const int int50 = intp5;
		const int intp6 = 1 << 6;  // 64
		const int intp7 = 1 << 7;  // 128 
		const int int100 = intp7;
		const int intp8 = 1 << 8;  // 256
		const int int200 = intp8;
		const int int250 = intp8;
		const int intp9 = 1 << 9;  // 512
		const int int500 = intp9;
		const int intp10 = 1 << 10;  // 1024
		const int int1000 = intp10;
		const int intp11 = 1 << 11;
		const int intp12 = 1 << 12;
		const int intp13 = 1 << 13;
		const int intp14 = 1 << 14;
		const int int10000 = intp14;
		const int intp15 = 1 << 15;
		const int intp16 = 1 << 16;
		const int intp17 = 1 << 17;
		const int int100000 = intp17;
		const int intp18 = 1 << 18;
		const int intp19 = 1 << 19;
		const int intp20 = 1 << 20;  // 1M
		const int int1M = intp20;
		const int intp21 = 1 << 21;  // 2M
		const int intp22 = 1 << 22;  // 4M
		const int intp23 = 1 << 23;  // 8M
		const int intp24 = 1 << 24;  // 16M
		const int int10M = intp24;
		const int intp25 = 1 << 25;  // 32M
		const int intp26 = 1 << 26;  // 64M
		const int intp27 = 1 << 27;  // 128M
		const int int100M = intp27;
		const int intp28 = 1 << 28;  // 256M
		const int intp29 = 1 << 29;  // 512M
		const int intp30 = 1 << 30;  // 1024M
		const int int1B = intp20;

		enum jtype
		{
			type_unknown = 0,

			type_int8 = 1,
			type_int16 = 2,
			type_int32 = 3,
			type_int64 = 4,

			type_float32 = 5,
			type_float64 = 6,

			type_datetime = 7,
			type_object = 8,
			type_object_id = 9,

			type_string = 10
		};
	}
}
