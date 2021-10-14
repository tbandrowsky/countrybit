#pragma once

namespace countrybit
{
	namespace database
	{
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

		typedef int32_t row_id_type;
		const row_id_type null_row = -1;
	}
}
