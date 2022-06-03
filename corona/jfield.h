#pragma once

namespace corona
{
	namespace database
	{

		const int max_query_filters = 32;
		const int max_path_nodes = 64;
		const int max_projection_fields = 128;
		const int max_update_elements = 32;
		const int max_creatable_options = 100;
		const int max_selectable_options = 100;
		const int max_updatable_options = 100;
		const int max_selections = 1000;
		const int max_class_fields = 512;
		const int default_collection_object_size = 512;

		struct string_properties_type
		{
			int32_t						length;
			string_validation_pattern	validation_pattern;
			string_validation_message	validation_message;
			bool						full_text_editor;
			bool						rich_text_editor;
		};

		struct int_properties_type
		{
			int64_t			minimum_int;
			int64_t			maximum_int;
		};

		struct double_properties_type
		{
			double			minimum_double;
			double			maximum_double;
		};

		struct time_properties_type
		{
			int64_t			minimum_time_t;
			int64_t			maximum_time_t;
		};

		struct point_properties_type
		{

		};

		struct rectangle_properties_type
		{

		};

		struct layout_rect_properties_type
		{

		};

		struct color_properties_type
		{

		};

		struct query_properties_type
		{
			;
		};

		struct sql_properties_type
		{
			;
		};

		struct file_properties_type
		{
			;
		};

		struct http_properties_type
		{
			;
		};

		struct emphemeral_handle_type
		{
			time_t	valid_time;
			uint64_t handle;
		};

		struct image_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		struct midi_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		struct wave_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		struct dimensions_type
		{
			corona_size_t x, y, z;

			dimensions_type(corona_size_t _x, corona_size_t _y, corona_size_t _z) : x(_x), y(_y), z(_z) { ; }
			dimensions_type() : x(0), y(0), z(0) { ; }
		};

		int compare(const dimensions_type& a, const dimensions_type& b);
		int operator<(const dimensions_type& a, const dimensions_type& b);
		int operator>(const dimensions_type& a, const dimensions_type& b);
		int operator>=(const dimensions_type& a, const dimensions_type& b);
		int operator<=(const dimensions_type& a, const dimensions_type& b);
		int operator==(const dimensions_type& a, const dimensions_type& b);
		int operator!=(const dimensions_type& a, const dimensions_type& b);

		struct object_properties_type
		{
			dimensions_type		dim;
			relative_ptr_type			class_id;
			object_name			class_name;
			int64_t				class_size_bytes;
			int64_t				total_size_bytes;
		};

		struct model_properties_type
		{
			relative_ptr_type			class_id;
			object_name			class_name;
			int64_t				model_size_bytes;
		};

		class jfield
		{
		public:

			relative_ptr_type				field_id;
			jtype					type_id;
			int64_t					size_bytes;

			object_name				name;
			object_description		description;

			union
			{
				string_properties_type		string_properties;
				int_properties_type			int_properties;
				double_properties_type		double_properties;
				time_properties_type		time_properties;
				object_properties_type		object_properties;
				query_properties_type		query_properties;
				point_properties_type		point_properties;
				rectangle_properties_type   rectangle_properties;
				image_properties_type		image_properties;
				midi_properties_type		midi_properties;
				wave_properties_type		wave_properties;
				color_properties_type		color_properties;
				sql_properties_type			sql_properties;
				file_properties_type		file_properties;
				http_properties_type		http_properties;
				layout_rect_properties_type		layout_rect_properties;
			};

			jfield() { ; }
			~jfield() { ; }

			bool is_container()
			{
				return type_id == jtype::type_list || type_id == jtype::type_object;
			}

			bool is_scalar()
			{
				return !is_container();
			}

			bool is_data_generator() 
			{
				return type_id == jtype::type_query || type_id == jtype::type_sql || type_id == jtype::type_http || type_id == jtype::type_file;
			}

			bool is_integer()
			{
				return type_id == jtype::type_int16 || type_id == jtype::type_int32 || type_id == jtype::type_int64 || type_id == jtype::type_int8;
			}

			bool is_point()
			{
				return type_id == jtype::type_point;
			}

			bool is_rectangle()
			{
				return type_id == jtype::type_rectangle;
			}

			bool is_color()
			{
				return type_id == jtype::type_color;
			}

			bool is_image()
			{
				return type_id == jtype::type_image;
			}

			bool is_wave()
			{
				return type_id == jtype::type_wave;
			}

			bool is_midi()
			{
				return type_id == jtype::type_midi;
			}

			bool is_int8()
			{
				return type_id == jtype::type_int8;
			}

			bool is_int16()
			{
				return type_id == jtype::type_int16;
			}

			bool is_int32()
			{
				return type_id == jtype::type_int32;
			}

			bool is_int64()
			{
				return type_id == jtype::type_int64;
			}

			bool is_float()
			{
				return type_id == jtype::type_float32 || type_id == jtype::type_float64;
			}

			bool is_float32()
			{
				return type_id == jtype::type_float32;
			}

			bool is_float64()
			{
				return type_id == jtype::type_float64;
			}

			bool is_string()
			{
				return type_id == jtype::type_string;
			}

			bool is_datetime()
			{
				return type_id == jtype::type_datetime;
			}

		};

		class query_definition_t
		{
		public:
			object_name								result_class_name;
			relative_ptr_type						result_class_id;
			relative_ptr_type						result_field_id;
			query_body								body;
			corona_size_t							max_result_objects;
		};

		using query_definition_type = query_definition_t;

		class query_instance
		{
		public:
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		struct file_definition_type
		{
			remote_file_path				file_path;
			object_name						parameter_field;
			relative_ptr_type						parameter_field_id;
			relative_ptr_type						result_field_id;
			object_name						result_class_name;
			relative_ptr_type						result_class_id;
			int								max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class file_remote_instance
		{
		public:
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		enum class http_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			jwt_authentication = 3,
			certificate_authentication = 4
		};

		struct http_definition_type
		{
			operation_name					login_type_name;
			http_login_types				login_type;
			remote_http_url					login_url;
			remote_http_method				login_method;
			object_name						username;
			object_name						password;
			remote_http_url					data_url;
			remote_http_method				data_method;
			relative_ptr_type						result_field_id;
			object_name						result_class_name;
			relative_ptr_type						result_class_id;
			relative_ptr_type						max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class http_remote_instance
		{
		public:
			collection_id_type			collection;
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		enum class sql_login_types
		{
			no_authentication = 0,
			windows_authentication = 1,
			basic_authentication = 2,
			certificate_authentication = 3
		};

		struct sql_definition_type
		{
			operation_name					login_type_name;
			sql_login_types					login_type;
			object_name						username;
			object_name						password;
			relative_ptr_type				result_field_id;
			object_name						result_class_name;
			relative_ptr_type				result_class_id;
			relative_ptr_type				max_result_objects;
			remote_parameter_fields_type	parameters;
			remote_fields_type				fields;
			remote_sql_query				query;
			time_t							last_success;
			time_t							last_error;
			object_description				error_message;
		};

		class sql_remote_instance
		{
		public:
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};


		class put_field_request_base {
		public:
			relative_ptr_type field_id;
			jtype		type_id;
			object_name name;
			object_description description;
			object_type	type;
		};

		class put_string_field_request {
		public:
			put_field_request_base name;
			string_properties_type options;
		};

		class put_integer_field_request {
		public:
			put_field_request_base name;
			int_properties_type options;
		};

		class put_double_field_request {
		public:
			put_field_request_base name;
			double_properties_type options;
		};

		class put_time_field_request {
		public:
			put_field_request_base name;
			time_properties_type options;
		};

		class put_object_field_request {
		public:
			put_field_request_base name;
			object_properties_type options;
		};

		class put_named_query_field_request {
		public:
			put_field_request_base name;
			query_definition_type options;
		};

		class put_named_sql_remote_field_request {
		public:
			put_field_request_base name;
			sql_definition_type options;
		};

		class put_named_file_remote_field_request {
		public:
			put_field_request_base name;
			file_definition_type options;
		};

		class put_named_http_remote_field_request
		{
		public:
			put_field_request_base name;
			http_definition_type options;
		};

		class put_point_field_request {
		public:
			put_field_request_base name;
			point_properties_type options;
		};

		class put_rectangle_field_request {
		public:
			put_field_request_base name;
			rectangle_properties_type options;
		};

		class put_layout_rect_field_request {
		public:
			put_field_request_base name;
			layout_rect_properties_type options;
		};

		class put_image_field_request {
		public:
			put_field_request_base name;
			image_properties_type options;
		};

		class put_wave_field_request {
		public:
			put_field_request_base name;
			wave_properties_type options;
		};

		class put_midi_field_request {
		public:
			put_field_request_base name;
			midi_properties_type options;
		};

		class put_color_field_request {
		public:
			put_field_request_base name;
			color_properties_type options;
		};

		class put_file_field_request {
		public:
			put_field_request_base name;
			file_properties_type options;
		};

		class put_sql_field_request {
		public:
			put_field_request_base name;
			sql_properties_type options;
		};

		class put_http_field_request {
		public:
			put_field_request_base name;
			http_properties_type options;
		};

		enum class member_field_types
		{
			member_field = 1,
			member_class = 2,
			member_list = 3
		};

		struct member_field
		{
		public:
			member_field_types	membership_type;
			object_name			field_name;
			object_name			membership_type_name;

			union
			{
				relative_ptr_type field_id;
				relative_ptr_type class_id;
			};

			dimensions_type dimensions;

			member_field() : 
				membership_type( member_field_types::member_field ),
				field_name(  ""),
				membership_type_name( ""),
				field_id(null_row)
			{ 
				; 
			}

			member_field(relative_ptr_type _field_id) :
				membership_type(member_field_types::member_field),
				field_name(""),
				field_id(_field_id),
				dimensions{ 1, 1, 1 }
			{
				;
			}

			member_field(member_field_types _member_ship_type, relative_ptr_type _id) :
				membership_type(_member_ship_type),
				field_name(""),
				dimensions{ 1, 1, 1 }
			{
				switch (membership_type)
				{
				case member_field_types::member_class:
					class_id = _id;
					break;
				case member_field_types::member_field:
					field_id = _id;
					break;
				}
			}

			member_field(member_field_types _member_ship_type, object_name _name) :
				membership_type(_member_ship_type),
				field_name(""),
				field_id(null_row),
				dimensions{ 1, 1, 1 }
			{

			}

			member_field(relative_ptr_type _class_id, int _maximum) :
				membership_type(member_field_types::member_list),
				field_name(""),
				class_id(_class_id)
			{
				dimensions = { _maximum, 1, 1 };
			}

			member_field(relative_ptr_type _class_id, dimensions_type dims) :
				membership_type(member_field_types::member_class),
				field_name(""),
				class_id(_class_id)
			{
				dimensions = dims;
			}

			member_field(const char *_name) :
				membership_type(member_field_types::member_field),
				field_name(_name),
				field_id(null_row)
			{
				dimensions = { 1, 1, 1 };
			}

			member_field(object_name _name) :
				membership_type(member_field_types::member_field),
				field_name(_name),
				class_id(null_row)
			{
				dimensions = { 1, 1, 1 };
			}

			member_field(object_name _name, int _maximum) :
				membership_type(member_field_types::member_list),
				field_name(_name),
				class_id(null_row)
			{
				dimensions = { _maximum, 1, 1 };
			}

			member_field(object_name _name, dimensions_type dims) :
				membership_type(member_field_types::member_class),
				field_name(_name),
				class_id(null_row)
			{
				dimensions = dims;
			}
		};

		using member_field_collection = iarray<member_field, max_class_fields>;

		class put_class_request
		{
		public:
			relative_ptr_type				 class_id;
			object_name				 class_name;
			object_description		 class_description;
			member_field_collection  member_fields;
		};

		class jlist_instance
		{
		public:
			corona_size_t   allocated;
			corona_size_t   selection_offset;
			corona_size_t   sort_offset;
			corona_size_t   slice_offset;
		};

		class jlist_state
		{
		public:
			array_box<relative_ptr_type> selections;
			array_box<relative_ptr_type> sort_order;
			jlist_instance* instance;
			char	  *list_bytes;
		};

		class jclass_header
		{
		public:
			relative_ptr_type									class_id;
			object_name									name;
			object_description							description;
			uint64_t									class_size_bytes;
		};

		class jclass_field
		{
		public:
			relative_ptr_type				field_id;
			uint64_t				offset;
		};

		using jclass = item_details_holder<jclass_header, jclass_field>;
		using field_array = iarray<jclass_field, max_class_fields>;
		using selections_collection = iarray<relative_ptr_type, max_selections>;

		class selector_rule
		{
		public:
			object_name class_name;
			relative_ptr_type class_id;
		};

		using selector_rule_collection = iarray<selector_rule, 8>;

		class selector_collection
		{
		public:
			selector_rule_collection rules;

			void always()
			{
				rules.clear();
			}

			void when(relative_ptr_type _class_id0)
			{
				auto *sr = rules.append();
				sr->class_id = _class_id0;
			}

			void when(relative_ptr_type _class_id0, relative_ptr_type _class_id1)
			{
				when(_class_id0);
				when(_class_id1);
			}

			void when(relative_ptr_type _class_id0, relative_ptr_type _class_id1, relative_ptr_type _class_id2)
			{
				when(_class_id0);
				when(_class_id1);
				when(_class_id2);
			}
		};

		class model_creatable_class
		{
		public:
			object_name						rule_name;
			object_name						create_class_name;
			relative_ptr_type						create_class_id;
			selector_collection				selectors;
			object_name						item_id_class_name;
			relative_ptr_type						item_id_class;
			bool							select_on_create;
			bool							replace_selected;

			model_creatable_class()
			{
				create_class_id = null_row;
				item_id_class = null_row;
				select_on_create = false;
				replace_selected = false;
			}
		};

		class model_selectable_class
		{
		public:	
			object_name						rule_name;
			object_name						select_class_name;
			relative_ptr_type						select_class_id;
			selector_collection				selectors;

			model_selectable_class()
			{
				select_class_id = null_row;
			}
		};

		class model_updatable_class
		{
		public:
			object_name						rule_name;
			object_name						update_class_name;
			relative_ptr_type						update_class_id;
			selector_collection				selectors;

			model_updatable_class()
			{
				update_class_id = null_row;
			}

		};

		using model_create_class_collection = iarray<model_creatable_class, max_creatable_options>;
		using model_select_class_collection = iarray<model_selectable_class, max_selectable_options>;
		using model_update_class_collection = iarray<model_updatable_class, max_updatable_options>;

		class model_type
		{
		public:
			object_name					  name;
			model_create_class_collection create_options;
			model_select_class_collection select_options;
			model_update_class_collection update_options;
		};

		using jmodel = model_type;
	}
}
