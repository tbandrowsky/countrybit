module;

#include <iostream>
#include <string>
#include <cinttypes>
#include <compare>

export module corona:jfield;
import "corona-windows-all.h";
import :constants;
import :store_box;
import :string_box;
import :iarray;
import :table;

export const int max_query_filters = 32;
export const int max_path_nodes = 16;
export const int max_projection_fields = 128;
export const int max_update_elements = 32;
export const int max_creatable_options = 100;
export const int max_selectable_options = 100;
export const int max_updatable_options = 100;
export const int max_deletable_options = 100;
export const int max_selections = 1000;
export const int max_class_fields = 512;
export const int default_collection_object_size = 512;
export const int max_selection_depth = 32;

export using class_list = iarray<relative_ptr_type, 16>;
export using field_list = iarray<relative_ptr_type, 16>;

export enum class comparisons
		{
			none = 0,
			gt = 1,
			gte = 2,
			eq = 3,
			lte = 4,
			lt = 5,
			cont = 6
		};

export struct comparison_field
		{
			relative_ptr_type field_id_source;
			relative_ptr_type field_id_target;
			comparisons		  comparison;

			comparison_field() : comparison(comparisons::none), field_id_source(null_row), field_id_target(null_row)
			{
				;
			}
		};

export using comparison_list = iarray<comparison_field, 20>;

		export struct string_properties_type
		{
			int32_t						length;
			string_validation_pattern	validation_pattern;
			string_validation_message	validation_message;
			bool						full_text_editor;
			bool						rich_text_editor;
		};

		export struct int_properties_type
		{
			int64_t			minimum_int;
			int64_t			maximum_int;
			object_name		format;
		};

		export struct double_properties_type
		{
			double			minimum_double;
			double			maximum_double;
			object_name		format;
		};

		export struct time_properties_type
		{
			DATE			minimum_date;
			DATE			maximum_date;
			object_name		format;
		};

		export struct currency_properties_type
		{
			CY				minimum_amount;
			CY				maximum_amount;
			object_name		format;
		};

		export struct emphemeral_handle_type
		{
			time_t	valid_time;
			uint64_t handle;
		};

		export struct image_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		export struct midi_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		export struct wave_properties_type
		{
			object_path				image_path;
			emphemeral_handle_type	handle;
		};

		export struct dimensions_type
		{
			corona_size_t x, y, z;

			dimensions_type(corona_size_t _x, corona_size_t _y, corona_size_t _z) : x(_x), y(_y), z(_z) { ; }
			dimensions_type() : x(0), y(0), z(0) { ; }
			bool increment(dimensions_type& _constraint);
		};

		export int compare(const dimensions_type& a, const dimensions_type& b);
		export int operator<(const dimensions_type& a, const dimensions_type& b);
		export int operator>(const dimensions_type& a, const dimensions_type& b);
		export int operator>=(const dimensions_type& a, const dimensions_type& b);
		export int operator<=(const dimensions_type& a, const dimensions_type& b);
		export int operator==(const dimensions_type& a, const dimensions_type& b);
		export int operator!=(const dimensions_type& a, const dimensions_type& b);


		export class jfield
		{
		public:

			relative_ptr_type		field_id;
			jtype					type_id;
			control_type			control_id;
			int64_t					size_bytes;
			bool					user_defined;
			bool					is_key;

			object_name				name;
			object_description		description;

			relative_ptr_type		enumeration_class_id;
			relative_ptr_type		enumeration_display_field_id;
			relative_ptr_type		enumeration_value_field_id;

			union
			{
				string_properties_type		string_properties;
				int_properties_type			int_properties;
				double_properties_type		double_properties;
				time_properties_type		time_properties;
				currency_properties_type	currency_properties;
				image_properties_type		image_properties;
				midi_properties_type		midi_properties;
				wave_properties_type		wave_properties;
			};

			jfield() { ; }
			~jfield() { ; }

			bool is_integer()
			{
				return type_id == jtype::type_int16 || type_id == jtype::type_int32 || type_id == jtype::type_int64 || type_id == jtype::type_int8;
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

		export class put_field_request_base {
		public:
			relative_ptr_type field_id;
			jtype		type_id;
			object_name name;
			object_description description;
			bool		is_key;

			relative_ptr_type		enumeration_class_id;
			relative_ptr_type		enumeration_display_field_id;
			relative_ptr_type		enumeration_value_field_id;

			put_field_request_base() : field_id(null_row),
				type_id(jtype::type_null),
				name(""),
				description(""),
				enumeration_class_id(null_row),
				enumeration_display_field_id(null_row),
				enumeration_value_field_id(null_row),
				is_key(false)

			{
				;
			}

			put_field_request_base(jtype _type_id, const char* _name, const char* _description, bool _key = false) :
				field_id(null_row),
				type_id(_type_id),
				name(_name),
				description(_description),
				enumeration_class_id(null_row),
				enumeration_display_field_id(null_row),
				enumeration_value_field_id(null_row),
				is_key(_key)

			{
				;
			}

			put_field_request_base(jtype _type_id, const char* _name, const char* _description, bool _key, relative_ptr_type _enumeration_class_id, relative_ptr_type _enumeration_display_field_id, relative_ptr_type _enumeration_value_field_id) :
				field_id(null_row),
				type_id(_type_id),
				name(_name),
				description(_description),
				enumeration_class_id(_enumeration_class_id),
				enumeration_display_field_id(_enumeration_display_field_id),
				enumeration_value_field_id(_enumeration_value_field_id),
				is_key(_key)
			{
				;
			}
		};

		export class put_string_field_request {
		public:
			put_field_request_base name;
			string_properties_type options;
		};

		export class put_integer_field_request {
		public:
			put_field_request_base name;
			int_properties_type options;
		};

		export class put_double_field_request {
		public:
			put_field_request_base name;
			double_properties_type options;
		};

		export class put_time_field_request {
		public:
			put_field_request_base name;
			time_properties_type options;
		};

		export class put_currency_field_request {
		public:
			put_field_request_base name;
			currency_properties_type options;
		};

		export class put_image_field_request {
		public:
			put_field_request_base name;
			image_properties_type options;
		};

		export class put_wave_field_request {
		public:
			put_field_request_base name;
			wave_properties_type options;
		};

		export class put_midi_field_request {
		public:
			put_field_request_base name;
			midi_properties_type options;
		};

		export enum class member_field_types
		{
			member_field = 1,
			member_class = 2,
			member_list = 3
		};

		export struct member_field
		{
		public:
			object_name			field_name;
			relative_ptr_type	field_id;

			member_field() :
				field_name(""),
				field_id(null_row)
			{
				;
			}

			member_field(relative_ptr_type _field_id) :
				field_name(""),
				field_id(_field_id)
			{
				;
			}

			member_field(object_name _name) :
				field_name(_name),
				field_id(null_row)
			{

			}

			member_field(const char* _name) :
				field_name(_name),
				field_id(null_row)
			{

			}

		};

		export using member_field_collection = iarray<member_field, max_class_fields>;

		export class put_class_request
		{
		public:
			relative_ptr_type				class_id;
			object_name						class_name;
			object_description				class_description;
			object_description				create_prompt;
			member_field_collection			member_fields;
			bool							auto_primary_key;
			relative_ptr_type				field_id_primary_key;
			relative_ptr_type				base_class_id;
			relative_ptr_type				template_class_id;

			put_class_request() :
				class_id(null_row),
				auto_primary_key(false),
				field_id_primary_key(null_row),
				base_class_id(null_row),
				template_class_id(null_row)
			{
				;
			}
		};

		class jclass_header
		{
		public:
			relative_ptr_type							class_id;
			object_name									name;
			object_description							description;
			object_description							create_prompt;
			uint64_t									class_size_bytes;
			bool										user_defined;
			int											primary_key_idx;
			relative_ptr_type							base_class_id;
			relative_ptr_type							template_class_id;
		};

		export class jclass_field
		{
		public:
			relative_ptr_type				field_id;
			uint64_t						offset;
		};

		export using jclass = item_details_holder<jclass_header, jclass_field>;
		export using field_array = iarray<jclass_field, max_class_fields>;
		export using selections_collection = iarray<relative_ptr_type, max_selections>;


		bool dimensions_type::increment(dimensions_type& _constraint)
		{
			x++;
			if (x >= _constraint.x)
			{
				x = 0;
				y++;
				if (y >= _constraint.y)
				{
					z++;
					y = 0;
					if (z >= _constraint.z)
					{
						z = 0;
						return false;
					}
				}
			}
			return true;
		}


