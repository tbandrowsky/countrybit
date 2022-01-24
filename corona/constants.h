#pragma once

namespace countrybit
{
	namespace database
	{
		enum jtype
		{
			type_null = 0,

			type_int8 = 1,
			type_int16 = 2,
			type_int32 = 3,
			type_int64 = 4,

			type_float32 = 5,
			type_float64 = 6,

			type_datetime = 7,
			type_object = 8,
			type_object_id = 9,
			type_collection_id = 10,
			type_string = 11,

			type_query = 12,
			type_image = 13,
			type_wave = 14,
			type_midi = 15,
			type_point = 16,
			type_rectangle = 17
		};

		typedef int32_t row_id_type;
		const row_id_type null_row = -1;

		struct collection_id_type
		{
			unsigned long  Data1;
			unsigned short Data2;
			unsigned short Data3;
			unsigned char  Data4[8];
		};

		struct object_id_type
		{
			collection_id_type collection_id;
			row_id_type	  row_id;
		};
	}
}
