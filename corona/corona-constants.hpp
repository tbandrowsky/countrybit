#ifndef CORONA_CONSTANTS_H
#define CORONA_CONSTANTS_H

#include <cstring>
#include <concepts>
#include <cstdlib>
#include <compare>

namespace corona
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
		type_string = 8,
		type_currency = 9,
		type_image = 10,
		type_wave = 11,
		type_midi = 12
	};

	enum class control_type
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

	using corona_size_t = int64_t;
	using relative_ptr_type = corona_size_t;

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

		static  block_id box_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "box", sizeof(box_id.name));
			return box_id;
		}

		static  block_id sorted_index_node_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "slnode", sizeof(box_id.name));
			return box_id;
		}

		static  block_id sorted_index_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "sl", sizeof(box_id.name));
			return box_id;
		}

		static  block_id list_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "lst", sizeof(box_id.name));
			return box_id;
		}

		static  block_id item_detail_table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "idtble", sizeof(box_id.name));
			return box_id;
		}

		static  block_id group_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "group", sizeof(box_id.name));
			return box_id;
		}

		static  block_id table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "tble", sizeof(box_id.name));
			return box_id;
		}

		static  block_id expression_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "expr", sizeof(box_id.name));
			return box_id;
		}

		static  block_id expression_term_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "exprt", sizeof(box_id.name));
			return box_id;
		}

		static  block_id collection_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "corcol", sizeof(box_id.name));
			return box_id;
		}

		static  block_id database_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "cordb", sizeof(box_id.name));
			return box_id;
		}

		static  block_id operator_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "oper", sizeof(box_id.name));
			return box_id;
		}

		static  block_id value_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "value", sizeof(box_id.name));
			return box_id;
		}

		static  block_id identifier_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "ident", sizeof(box_id.name));
			return box_id;
		}

		static  block_id string_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "string", sizeof(box_id.name));
			return box_id;
		}

		static  block_id number_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "number", sizeof(box_id.name));
			return box_id;
		}

		static  block_id color_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "color", sizeof(box_id.name));
			return box_id;
		}

		static  block_id wave_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "wave", sizeof(box_id.name));
			return box_id;
		}

		static block_id audio_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "audio", sizeof(box_id.name));
			return box_id;
		}


		static block_id dimension_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dim", sizeof(box_id.name));
			return box_id;
		}

		static block_id datetime_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dtm", sizeof(box_id.name));
			return box_id;
		}

		static block_id path_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "path", sizeof(box_id.name));
			return box_id;
		}

		static block_id json_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "json", sizeof(box_id.name));
			return box_id;
		}

		static block_id general_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "gen", sizeof(box_id.name));
			return box_id;
		}

		bool is_box()
		{
			block_id t = block_id::box_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_json()
		{
			block_id t = block_id::json_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_general()
		{
			block_id t = block_id::general_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_sorted_index_node()
		{
			block_id t = block_id::sorted_index_node_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_sorted_index()
		{
			block_id t = block_id::sorted_index_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_list()
		{
			block_id t = block_id::list_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_item_detail_table()
		{
			block_id t = block_id::item_detail_table_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_table()
		{
			block_id t = block_id::table_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_group()
		{
			block_id t = block_id::group_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_expression()
		{
			block_id t = block_id::expression_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_collection()
		{
			block_id t = block_id::collection_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_database()
		{
			block_id t = block_id::database_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_operator()
		{
			block_id t = block_id::operator_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_value()
		{
			block_id t = block_id::value_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_identifer()
		{
			block_id t = block_id::identifier_id();
			return strcmp(t.name, name) == 0;
			;
		}

		bool is_string()
		{
			block_id t = block_id::string_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_number()
		{
			block_id t = block_id::number_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_color()
		{
			block_id t = block_id::color_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_wave()
		{
			block_id t = block_id::wave_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_audio()
		{
			block_id t = block_id::audio_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_dimension()
		{
			block_id t = block_id::dimension_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_datetime()
		{
			block_id t = block_id::datetime_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_path()
		{
			block_id t = block_id::path_id();
			return strcmp(t.name, name) == 0;
		}

	};

	struct block_header_struct
	{
		block_id	block_type;
		int64_t		object_id;
		int64_t		data_length;
		int64_t		data_location;
		int64_t		next_free_block;
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
		a.block.is_datetime();
		a.block.is_path();

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
	concept explicit_char_ptr = std::same_as<T, const char*>;

	class jtype_identifier
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

	auto toHex(char i)
	{
		struct char_pair
		{
			char str[4];
		} cp;

		int major, minor;
		major = i / 16;
		minor = i & 16;

		if (major < 10)
		{
			cp.str[0] = major + '0';
		}
		else
		{
			cp.str[0] = (major - 10) + 'A';
		}

		if (minor < 10)
		{
			cp.str[1] = minor + '0';
		}
		else
		{
			cp.str[1] = (minor - 10) + 'A';
		}
		cp.str[2] = 0;

		return cp;
	}

	int toInt(char hex, int shift)
	{
		int d = {};
		hex = toupper(hex);

		if (hex >= 'A' && hex <= 'F')
		{
			d = hex - 'A' + 10;
		}
		else if (hex >= '0' && hex <= '9')
		{
			d = hex - '0';
		}
		d <<= shift;
		return d;
	}

	int toInt2(const std::string& item, int _baseIndex)
	{
		int r = toInt(item[_baseIndex], 4) + toInt(item[_baseIndex + 1], 0);
		return r;
	}

	class buffer
	{
		size_t buffer_size;
		std::unique_ptr<char[]> buffer_bytes;

	public:

		buffer() 
		{
			buffer_size = 0;
			buffer_bytes = nullptr;
		}

		buffer(const char* _src) {
			buffer_size = strlen(_src);
			buffer_bytes = std::make_unique<char[]>(buffer_size + 1);
			std::copy(_src, _src + buffer_size, buffer_bytes.get());
		}

		buffer(size_t _size)
		{
			init(_size);
		}

		buffer(buffer&& _src)
		{
			buffer_bytes = std::move(_src.buffer_bytes);
			buffer_size = _src.buffer_size;
		}

		buffer(const buffer& _src)
		{
			init(_src.buffer_size);
			std::copy(_src.buffer_bytes.get(), _src.buffer_bytes.get() + buffer_size, buffer_bytes.get());
		}

		void init(size_t _size)
		{
			buffer_bytes = std::make_unique<char[]>(_size + 1);
			buffer_size = _size;
		}

		buffer& operator = (const buffer& _src)
		{
			init(_src.buffer_size);
			std::copy(_src.buffer_bytes.get(), _src.buffer_bytes.get() + buffer_size, buffer_bytes.get());
			return *this;
		}

		template <typename poco_type> buffer& operator = (const poco_type& _src)
		{
			init(sizeof(poco_type));
			char* p = (char*)&_src;
			std::copy(p, p + buffer_size, buffer_bytes.get());
			return *this;
		}

		buffer operator = (buffer&& _src)
		{
			buffer_bytes = std::move(_src.buffer_bytes);
			buffer_size = _src.buffer_size;
			return *this;
		}

		char* get_ptr()
		{
			return buffer_bytes.get();
		}

		char* get_ptr_end()
		{
			return buffer_bytes.get() + buffer_size;
		}

		size_t get_size()
		{
			return buffer_size;
		}
	};

	class buffer_assembler
	{
		std::vector<buffer> buffers;

	public:

		char* append(std::string _str)
		{
			buffer temp(_str.size());
			char* d = temp.get_ptr();
			const char* s = _str.c_str();
			while (*s) {
				*d = *s;
				s++;
				d++;
			}
			*d = 0;
			buffers.push_back(std::move(temp));
			return buffers[buffers.size() - 1].get_ptr();
		}

		char* append(size_t _size)
		{
			buffer temp(_size);
			buffers.push_back(std::move(temp));
			return buffers[buffers.size() - 1].get_ptr();
		}

		buffer consolidate()
		{
			size_t total = 0;
			for (auto& b : buffers) {
				total += b.get_size();
			}
			size_t end_buffer = total;
			total++;
			buffer consolidated(total);
			char* t = consolidated.get_ptr();
			for (auto& b : buffers) {
				std::copy(b.get_ptr(), b.get_ptr_end(), t);
				t += b.get_size();
			}
			*t = 0;
			return consolidated;
		}

		int size()
		{
			return buffers.size();
		}
	};


}

#endif
