module;

#include <cstring>
#include <concepts>

export module corona.database:constants;

import :stdapi;

export enum jtype
{
	type_null = 0,
	type_int8 = 1,
	type_int16 = 2,
	type_int32 = 3,
	type_int64 = 4,
	type_float32 = 5,
	type_float64 = 6,
	type_datetime = 7,
	type_string = 8,
	type_currency = 9,
	type_image = 10,
	type_wave = 11,
	type_midi = 12
};

export enum class control_type
{
	ct_none = 0,
	ct_numeric = 1,
	ct_date_picker = 2,
	ct_text = 2,
	ct_currency = 4,
	ct_dropdown = 5,
	ct_radio = 6,
	ct_checkbox = 7,
	ct_color_picker = 9
};

export typedef int64_t corona_size_t;
export typedef corona_size_t relative_ptr_type;

export const relative_ptr_type null_row = -1;
export const relative_ptr_type first_row = 0;

export struct collection_id_type
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
};

export struct object_id_type
{
	collection_id_type collection_id;
	relative_ptr_type	  row_id;
};

export struct block_id
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
	bool is_datetime();
	bool is_path();

};

export template <typename blocked_type> concept named_block =
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
	a.block.is_datetime();
	a.block.is_path();

};

export template <class T>
concept explicit_int32 = std::same_as<T, int32_t>;

export template <class T>
concept explicit_int64 = std::same_as<T, int64_t>;

export template <class T>
concept explicit_int = std::same_as<T, int64_t> || std::same_as<T, int32_t>;

export template <class T>
concept explicit_double = std::same_as<T, double>;

export template <class T>
concept explicit_float = std::same_as<T, float>;

export template <class T>
concept explicit_floating_point = std::same_as<T, double> || std::same_as<T, float>;

export template <class T>
concept explicit_bool = std::same_as<T, bool>;

export template <class T>
concept explicit_char_ptr = std::same_as<T, const char*>;

export class jtype_identifier
{
public:
	static jtype_identifier get() { return jtype_identifier(); }
	template<explicit_int32 t> jtype from(t _dummy) { return jtype::type_int32; }
	template<explicit_int64 t> jtype from(t _dummy) { return jtype::type_int64; }
	template<explicit_int t> jtype from(t _dummy) { return jtype::type_int64; }
	template<explicit_double t> jtype from(t _dummy) { return jtype::type_float64; }
	template<explicit_float t> jtype from(t _dummy) { return jtype::type_float32; }
	template<explicit_bool t> jtype from(t _dummy) { return jtype::type_int8; }
	template<explicit_char_ptr t> jtype from(t _dummy) { return jtype::type_string; }
};

const int styles_count = 5;
const int style_normal = 0;
const int style_over = 1;
const int style_selected = 2;
const int style_busy = 3;
const int style_disabled = 4;

block_id block_id::box_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "box", sizeof(box_id.name));
	return box_id;
}

block_id block_id::sorted_index_node_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "slnode", sizeof(box_id.name));
	return box_id;
}

block_id block_id::sorted_index_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "sl", sizeof(box_id.name));
	return box_id;
}

block_id block_id::list_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "lst", sizeof(box_id.name));
	return box_id;
}

block_id block_id::item_detail_table_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "idtble", sizeof(box_id.name));
	return box_id;
}

block_id block_id::group_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "group", sizeof(box_id.name));
	return box_id;
}

block_id block_id::table_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "tble", sizeof(box_id.name));
	return box_id;
}

block_id block_id::expression_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "expr", sizeof(box_id.name));
	return box_id;
}

block_id block_id::expression_term_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "exprt", sizeof(box_id.name));
	return box_id;
}

block_id block_id::collection_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "corcol", sizeof(box_id.name));
	return box_id;
}

block_id block_id::database_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "cordb", sizeof(box_id.name));
	return box_id;
}

block_id block_id::operator_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "oper", sizeof(box_id.name));
	return box_id;
}

block_id block_id::value_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "value", sizeof(box_id.name));
	return box_id;
}

block_id block_id::identifier_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "ident", sizeof(box_id.name));
	return box_id;
}

block_id block_id::string_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "string", sizeof(box_id.name));
	return box_id;
}

block_id block_id::number_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "number", sizeof(box_id.name));
	return box_id;
}

block_id block_id::color_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "color", sizeof(box_id.name));
	return box_id;
}

block_id block_id::wave_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "wave", sizeof(box_id.name));
	return box_id;
}

block_id block_id::audio_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "audio", sizeof(box_id.name));
	return box_id;
}


block_id block_id::dimension_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "dim", sizeof(box_id.name));
	return box_id;
}

block_id block_id::datetime_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "dtm", sizeof(box_id.name));
	return box_id;
}

block_id block_id::path_id()
{
	block_id box_id;
	strncpy_s(box_id.name, "path", sizeof(box_id.name));
	return box_id;
}

bool block_id::is_box()
{
	block_id t = block_id::box_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_sorted_index_node()
{
	block_id t = block_id::sorted_index_node_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_sorted_index()
{
	block_id t = block_id::sorted_index_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_list()
{
	block_id t = block_id::list_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_item_detail_table()
{
	block_id t = block_id::item_detail_table_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_table()
{
	block_id t = block_id::table_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_group()
{
	block_id t = block_id::group_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_expression()
{
	block_id t = block_id::expression_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_collection()
{
	block_id t = block_id::collection_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_database()
{
	block_id t = block_id::database_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_operator()
{
	block_id t = block_id::operator_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_value()
{
	block_id t = block_id::value_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_identifer()
{
	block_id t = block_id::identifier_id();
	return strcmp(t.name, name) == 0;
;
}

bool block_id::is_string()
{
	block_id t = block_id::string_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_number()
{
	block_id t = block_id::number_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_color()
{
	block_id t = block_id::color_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_wave()
{
	block_id t = block_id::wave_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_audio()
{
	block_id t = block_id::audio_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_dimension()
{
	block_id t = block_id::dimension_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_datetime()
{
	block_id t = block_id::datetime_id();
	return strcmp(t.name, name) == 0;
}

bool block_id::is_path()
{
	block_id t = block_id::path_id();
	return strcmp(t.name, name) == 0;
}

