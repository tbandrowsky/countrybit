#pragma once
#include <cstdint>

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
			type_rectangle = 17,
			type_color = 18,
			type_sql = 19,
			type_http = 20,
			type_file = 21,
			type_list = 22
		};

		typedef int64_t corona_size_t;
		typedef corona_size_t relative_ptr_type;

		const relative_ptr_type null_row = -1;

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
			relative_ptr_type	  row_id;
		};

		struct block_id
		{
		public:
			char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

			static block_id box();
			static block_id sorted_index_node();
			static block_id sorted_index();
			static block_id table();
			static block_id item_detail_table();
			static block_id collection();
			static block_id database();

			bool is_box();
			bool is_sorted_index_node();
			bool is_sorted_index();
			bool is_table();
			bool is_item_detail_table();
			bool is_collection();
			bool is_database();

		};

	}
}
