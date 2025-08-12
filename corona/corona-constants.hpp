/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the old school jam the numbers into the constants file slop.
Many of these constants are for the jcollection / jobject binary 
object kit.

Notes

For Future Consideration
*/


#ifndef CORONA_CONSTANTS_H
#define CORONA_CONSTANTS_H

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

	const char *get_file_name(const char *_filename)
	{
		const char *file_name = _filename;
		const char* last_slash = file_name;
		while (*file_name) {
			last_slash = file_name;
			file_name++;
		}
		return last_slash;
    }

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

	struct object_reference_type
	{
		std::string class_name;
		int64_t		object_id;

		operator bool()
		{
			return not class_name.empty() and object_id > 0;
		}

		object_reference_type& operator = (std::string& _src)
		{
			std::regex ortexpr("([A-Za-z0-9\_]+)([:])([0-9]+)", std::regex_constants::syntax_option_type::ECMAScript);

			std::smatch results;
			class_name = "";
			object_id = 0;

			if (std::regex_match(_src, results, ortexpr)) {
				int count = 0;
				for (auto result : results)
				{
					std::string text = result.str();
					switch (count)
					{
					case 0:
						class_name = text;
						break;
					case 1:
						object_id = std::strtoll(text.c_str(), nullptr, 10);
						break;
					}
					count++;
				}
			}


			return *this;
		}

		operator std::string()
		{
			return std::format("{0}:{1}", class_name, object_id);
		}
	};

	int64_t giga_to_bytes(int _ct)
	{
		int64_t t = _ct;
		t <<= 30;
		return t;
	}

	struct block_id
	{
	public:
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		static  block_id box_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "box", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id sorted_index_node_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "slnode", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id allocated_space_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "alloc", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id sorted_index_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "sl", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id list_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "lst", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id item_detail_table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "idtble", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id group_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "group", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "tble", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id expression_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "expr", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id expression_term_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "exprt", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id collection_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "corcol", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id database_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "cordb", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id operator_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "oper", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id value_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "value", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id identifier_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "ident", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id string_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "string", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id number_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "number", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id color_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "color", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static  block_id wave_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "wave", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static block_id audio_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "audio", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}


		static block_id dimension_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dim", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static block_id datetime_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dtm", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static block_id path_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "path", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static block_id json_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "json", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		static block_id general_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "gen", sizeof(box_id.name));
			box_id.name[7] = 0;
			return box_id;
		}

		bool is_box()
		{
			name[7] = 0;
			block_id t = block_id::box_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_json()
		{
			name[7] = 0;
			block_id t = block_id::json_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_general()
		{
			name[7] = 0;
			block_id t = block_id::general_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_sorted_index_node()
		{
			name[7] = 0;
			block_id t = block_id::sorted_index_node_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_sorted_index()
		{
			name[7] = 0;
			block_id t = block_id::sorted_index_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_list()
		{
			name[7] = 0;
			block_id t = block_id::list_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_item_detail_table()
		{
			name[7] = 0;
			block_id t = block_id::item_detail_table_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_table()
		{
			name[7] = 0;
			block_id t = block_id::table_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_group()
		{
			name[7] = 0;
			block_id t = block_id::group_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_expression()
		{
			name[7] = 0;
			block_id t = block_id::expression_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_collection()
		{
			name[7] = 0;
			block_id t = block_id::collection_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_database()
		{
			name[7] = 0;
			block_id t = block_id::database_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_operator()
		{
			name[7] = 0;
			block_id t = block_id::operator_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_value()
		{
			name[7] = 0;
			block_id t = block_id::value_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_identifer()
		{
			name[7] = 0;
			block_id t = block_id::identifier_id();
			return strcmp(t.name, name) == 0;
			;
		}

		bool is_string()
		{
			name[7] = 0;
			block_id t = block_id::string_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_number()
		{
			name[7] = 0;
			block_id t = block_id::number_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_color()
		{
			name[7] = 0;
			block_id t = block_id::color_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_wave()
		{
			name[7] = 0;
			block_id t = block_id::wave_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_audio()
		{
			name[7] = 0;

			block_id t = block_id::audio_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_dimension()
		{
			name[7] = 0;

			block_id t = block_id::dimension_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_datetime()
		{
			name[7] = 0;

			block_id t = block_id::datetime_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_path()
		{
			name[7] = 0;

			block_id t = block_id::path_id();
			return strcmp(t.name, name) == 0;
		}

		bool is_allocated_space()
		{
			name[7] = 0;

			block_id t = block_id::allocated_space_id();
			return strcmp(t.name, name) == 0;
		}
	};

	/*
	struct block_header_struct
	{
		block_id	block_type;
		int64_t		block_location;
		int64_t		data_location;
		int64_t		data_capacity;
		int64_t		data_size;
		int64_t		next_block;
	};
	*/

	class data_block_struct
	{
	public:
		relative_ptr_type		block_location;
		relative_ptr_type		data_location;
		int32_t					data_capacity;
		int32_t					data_size;

		data_block_struct() = default;
		data_block_struct(const data_block_struct& _src) = default;
		data_block_struct(data_block_struct&& _src) = default;
		data_block_struct& operator =(const data_block_struct& _src) = default;
		data_block_struct& operator =(data_block_struct&& _src) = default;
	};

	struct allocation_block_struct
	{
		relative_ptr_type		data_location;
		relative_ptr_type		next_block;
		int32_t					data_capacity;
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
	concept explicit_int = std::same_as<T, int64_t> or std::same_as<T, int32_t>;

	template <class T>
	concept explicit_double = std::same_as<T, double>;

	template <class T>
	concept explicit_float = std::same_as<T, float>;

	template <class T>
	concept explicit_floating_point = std::same_as<T, double> or std::same_as<T, float>;

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

	const std::string class_name_field = "class_name";
	const std::string base_class_name_field = "base_class_name";
	const std::string object_id_field = "object_id";
	const std::string include_children_field = "include_children";
	const std::string data_field = "data";
	const std::string success_field = "success";
	const std::string message_field = "message";
	const std::string token_field = "token";
	const std::string signature_field = "signature";
	const std::string user_class_field = "sys_users";
	const std::string user_first_name_field = "first_name";
	const std::string user_last_name_field = "last_name";
	const std::string user_name_field = "user_name";
	const std::string user_email_field = "email";
	const std::string user_mobile_field = "mobile";
	const std::string user_street1_field = "street1";
	const std::string user_street2_field = "street2";
	const std::string user_city_field = "city";
	const std::string user_state_field = "state";
	const std::string user_zip_field = "zip";
	const std::string user_children_field = "children";
	const std::string user_contacts_field = "contacts";
	const std::string user_password_field = "password";

	const std::string authorization_field = "authorization";
	const std::string data_hash_field = "data_hash";
	const std::string seconds_field = "seconds";
	const std::string token_expires_field = "expires";
	const std::string sys_user_name_field = "sys_user_name";
	const std::string sys_user_password_field = "sys_user_password";
	const std::string sys_user_email_field = "sys_user_email";
	const std::string sys_default_team_field = "new_user_default_team";
	const std::string parse_error_class = "sys_parse_error";
	const std::string sys_record_cache_field = "sys_record_cache_bytes";

	const std::string sys_default_api_title_field = "api_title";
	const std::string sys_default_api_description_field = "api_description";
	const std::string sys_default_api_version_field = "api_version";
	const std::string sys_default_api_author_field = "api_author";

	enum class field_types {
		ft_none = 0,
		ft_wildcard = 'w',
		ft_double = 'd',
		ft_int64 = 'i',
		ft_reference = 'r',
		ft_string = 's',
		ft_bool = 'b',
		ft_datetime = 't',
		ft_object = 'o',
		ft_array = 'a',
		ft_function = 'f',
		ft_query = 'q',
		ft_blob = 'x'
	};
	
	const int field_type_size = 12;

	enum class ft_operations : std::int8_t {
		fto_lt = 0,
		fto_eq = 1,
		fto_neq = 2,
		fto_gt = 3
	};

	const int field_type_operation_size = 4;

	template <typename E>
	constexpr auto to_underlying(E e) noexcept
	{
		return static_cast<std::underlying_type_t<E>>(e);
	};

	std::map<std::string, field_types> allowed_field_types = {
{ "object", field_types::ft_object },
{ "array", field_types::ft_array },
{ "double", field_types::ft_double },
{ "number", field_types::ft_double },
{ "int64", field_types::ft_int64 },
{ "reference", field_types::ft_reference },
{ "string", field_types::ft_string },
{ "bool", field_types::ft_bool },
{ "datetime", field_types::ft_datetime },
{ "function", field_types::ft_function },
{ "query", field_types::ft_query }
	};

	std::map<field_types, std::string> field_type_names = {
{ field_types::ft_object, "object"  },
{ field_types::ft_array , "array" },
{ field_types::ft_double, "double"  },
{ field_types::ft_int64, "int64"  },
{ field_types::ft_string, "string"  },
{ field_types::ft_bool, "bool"  },
{ field_types::ft_datetime, "datetime"  },
{ field_types::ft_function, "function"  },
{ field_types::ft_query, "query"  }
	};

	std::map<field_types, std::string> field_type_names_openapi = {
{ field_types::ft_object, "object"  },
{ field_types::ft_array , "array" },
{ field_types::ft_double, "number"  },
{ field_types::ft_int64, "number"  },
{ field_types::ft_string, "string"  },
{ field_types::ft_bool, "boolean"  },
{ field_types::ft_datetime, "string"  },
{ field_types::ft_function, "array"  },
{ field_types::ft_query, "array"  }
	};

	auto toHex(unsigned char i)
	{
		struct char_pair
		{
			char str[4];
		} cp;

		int major, minor;
		major = i / 16;
		minor = i % 16;

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

		if (hex >= 'A' and hex <= 'F')
		{
			d = hex - 'A' + 10;
		}
		else if (hex >= '0' and hex <= '9')
		{
			d = hex - '0';
		}
		d <<= shift;
		return d;
	}

	int toInt2(const std::string_view& item, int _baseIndex)
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
			char* p = buffer_bytes.get();
			std::fill(p, p + _size, 0);
			buffer_size = _size;
		}

		void init(const void *_src, size_t _size)
		{
			buffer_bytes = std::make_unique<char[]>(_size + 1);
			unsigned char* p = (unsigned char *)buffer_bytes.get();
			unsigned char* s = (unsigned char*)_src;
			std::copy(s, s + _size, p);
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

		std::string to_hex(size_t _bytes = 0)
		{
			std::string temp = "";
			if (_bytes > buffer_size or _bytes <= 0) {
				_bytes = buffer_size;
			}
			for (int i = 0; i < buffer_size; i++) {
				unsigned char bi = buffer_bytes[i];
				auto ret = corona::toHex(bi);
				temp += ret.str;
			}
			return temp;
		}

		void from_hex(std::string _hex_string)
		{
			std::vector<char> bytes;
			int index_lists = 0;
			int byte_count = 0;

			while (index_lists < _hex_string.size())
			{
				unsigned char t = corona::toInt2(_hex_string, index_lists);
				bytes.push_back(t);
				index_lists += 2;
				byte_count++;
			}

			buffer_size = byte_count;
			buffer_bytes = std::make_unique<char[]>(byte_count);
			std::copy(&bytes[0], &bytes[0] + byte_count, buffer_bytes.get());
		}

		void set_hex(std::string _hex_string)
		{
			int index_lists = 0;
			int byte_count = 0;

			while (index_lists < _hex_string.size() and byte_count < buffer_size)
			{
				unsigned char t = corona::toInt2(_hex_string, index_lists);
				buffer_bytes[byte_count] = t;
				index_lists += 2;
				byte_count++;
			}

			while (byte_count < buffer_size)
			{
				buffer_bytes[byte_count] = 0;
				byte_count++;
			}
		}

		void set_buffer(std::string _string)
		{
			int byte_count = 0;

			while (byte_count < buffer_size and byte_count < _string.size())
			{
				unsigned char t = _string[byte_count];
				buffer_bytes[byte_count] = t;
				byte_count++;
			}

			while (byte_count < buffer_size)
			{
				buffer_bytes[byte_count] = 0;
				byte_count++;
			}
		}

		void randomize()
		{
			for (int i = 0; i < buffer_size; i++) {
				buffer_bytes[i] = rand();
			}
		}

		unsigned char* get_uptr()
		{
			return (unsigned char *)buffer_bytes.get();
		}

		bool is_safe_string()
		{
			if (not buffer_bytes)
				return false;
			char* t = buffer_bytes.get() + buffer_size - 1;
			return *t == 0;
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

		char* append(const char *_src)
		{
			if (_src) 
			{
				size_t len = strlen(_src);
				buffer temp(len+1);
				char* d = temp.get_ptr();
				const char* s = _src;
				while (*s) {
					*d = *s;
					s++;
					d++;
				}
				*d = 0;
				buffers.push_back(std::move(temp));
				return buffers[buffers.size() - 1].get_ptr();
			}
			else 
			{
				return nullptr;
			}
		}

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

		char* append(buffer _b)
		{
			buffers.push_back(std::move(_b));
			return buffers[buffers.size() - 1].get_ptr();
		}

		buffer consolidate()
		{
			size_t total = 0;
			for (auto& b : buffers) {
				total += b.get_size();
			}
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

		size_t size()
		{
			return buffers.size();
		}
	};

	using db_object_id_type = int64_t;

}

#endif
