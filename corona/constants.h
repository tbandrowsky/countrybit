#pragma once

namespace corona
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
			type_currency = 12,

			type_query = 13,
			type_image = 14,
			type_wave = 15,
			type_midi = 16,
			type_point = 17,
			type_rectangle = 18,
			type_layout_rect = 19,
			type_color = 20,
			type_sql = 21,
			type_http = 22,
			type_file = 23,
			type_list = 24
		};

		enum class control_type
		{
			ct_none = 0,
			ct_text = 1,
			ct_big_text = 2,
			ct_numeric = 3,
			ct_currency = 4,
			ct_dropdown = 5,
			ct_radio = 6,
			ct_checkbox = 7,
			ct_date_picker = 8,
			ct_color_picker = 9
		};

		typedef int64_t corona_size_t;
		typedef corona_size_t relative_ptr_type;

		const relative_ptr_type null_row = -1;
		const relative_ptr_type first_row = 0;

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

			static block_id box_id();
			static block_id sorted_index_node_id();
			static block_id sorted_index_id();
			static block_id list_id();
			static block_id table_id();
			static block_id item_detail_table_id();
			static block_id group_id();
			static block_id expression_id();
			static block_id expression_term_id();
			static block_id collection_id();
			static block_id database_id();

			static block_id operator_id();
			static block_id value_id();
			static block_id identifier_id();
			static block_id dimension_id();
			static block_id string_id();
			static block_id number_id();
			static block_id color_id();
			static block_id wave_id();
			static block_id audio_id();
			static block_id rectangle_id();
			static block_id point_id();
			static block_id datetime_id();
			static block_id path_id();

			bool is_box();
			bool is_sorted_index_node();
			bool is_sorted_index();
			bool is_list();
			bool is_table();
			bool is_item_detail_table();
			bool is_group();
			bool is_expression();
			bool is_collection();
			bool is_database();

			bool is_operator();
			bool is_value();
			bool is_identifer();
			bool is_dimension();
			bool is_string();
			bool is_number();
			bool is_color();
			bool is_wave();
			bool is_audio();
			bool is_rectangle();
			bool is_point();
			bool is_datetime();
			bool is_path();

		};

		template <typename blocked_type> concept named_block =
			requires (blocked_type a) {
			a.block.is_operator();
			a.block.is_box();
			a.block.is_sorted_index_node();
			a.block.is_sorted_index();
			a.block.is_table();
			a.block.is_item_detail_table();
			a.block.is_expression();
			a.block.is_collection();
			a.block.is_database();

			a.block.is_operator();
			a.block.is_value();
			a.block.is_identifer();
			a.block.is_dimension();
			a.block.is_string();
			a.block.is_number();
			a.block.is_color();
			a.block.is_wave();
			a.block.is_audio();
			a.block.is_rectangle();
			a.block.is_point();
			a.block.is_datetime();
			a.block.is_path();

		};

		template <typename itemType> class and_functions {
		public:

			using boolean_fn = std::function<bool(const itemType&)>;

		private:

			std::vector<boolean_fn> functions;

		public:

			and_functions()
			{
			}

			and_functions(boolean_fn fn)
			{
				functions.push_back(fn);
			}

			and_functions operator = (const and_functions& srcs)
			{
				functions = srcs.functions;
				return *this;
			}

			and_functions& operator = (and_functions&& srcs)
			{
				functions = std::move(srcs.functions);
				return *this;
			}

			and_functions(const and_functions& srcs)
			{
				functions = srcs.functions;
			}

			and_functions(and_functions&& srcs)
			{
				functions = std::move(srcs.functions);
			}

			void and_fn(boolean_fn fn)
			{
				functions.push_back(fn);
			}

			bool operator()(const itemType& x)
			{
				return std::all_of(functions.begin(), functions.end(), [x](boolean_fn fn) { return fn(x); });
			}
		};

		template <class T>
		concept explicit_int32 = std::same_as<T, int32_t>;

		template <class T>
		concept explicit_int64 = std::same_as<T, int64_t>;

		template <class T>
		concept explicit_int = std::same_as<T, int64_t> || std::same_as<T, int32_t>;

		template <class T>
		concept explicit_double = std::same_as<T, double>;

		template <class T>
		concept explicit_float = std::same_as<T, float>;

		template <class T>
		concept explicit_floating_point = std::same_as<T, double> || std::same_as<T, float>;

		template <class T>
		concept explicit_bool = std::same_as<T, bool>;

		template <class T>
		concept explicit_char_ptr = std::same_as<T, const char *>;

		const int styles_count = 5;
		const int style_normal = 0;
		const int style_over = 1;
		const int style_selected = 2;
		const int style_busy = 3;
		const int style_disabled = 4;
	}
}
