#pragma once

namespace corona
{
	namespace database
	{
		namespace bridge
		{

			class object_base;
			class field_base;
			class array_field_base;
			class scalar_field_base;

			class field_base
			{
			public:
				virtual std::string get_field_name() = 0;
				virtual std::string get_field_description() = 0;
				virtual jtype get_field_type() = 0;
				virtual array_field_base* as_array_field() = 0;
				virtual scalar_field_base* as_scalar_field() = 0;
			};

			class array_field_base : public field_base
			{
			public:
				virtual std::shared_ptr<object_base> create_object() = 0;
				virtual std::shared_ptr<object_base> get_object(int _index) = 0;
				virtual int32_t get_object_count() = 0;
				virtual array_field_base* as_array_field() { return this;  };
				virtual scalar_field_base* as_scalar_field() { return nullptr;  };
			};

			class scalar_field_base : public field_base
			{
			public:
				virtual jvariant get_field_value() = 0;
				virtual bool set_field_value(jvariant _field) = 0;
				virtual array_field_base* as_array_field() { return nullptr; };
				virtual scalar_field_base* as_scalar_field() { return this; };
			};

			class object_base
			{
			public:
				virtual std::string get_class_name() = 0;
				virtual int get_field_count() = 0;
				virtual std::shared_ptr<field_base> get_field(int _field_index) = 0;
				virtual std::shared_ptr<field_base> get_field_by_name(const std::string& _name) = 0;
			};

			template <typename scalar_type> class poco_scalar_field : public scalar_field_base
			{
				scalar_type& field;
				std::string object_name;
				std::string object_description;

			public:

				poco_scalar_field(const std::string& _object_name, const std::string& _object_description, scalar_type& _field) : field(_field) 
				{
								
				}

				virtual jvariant get_field_value() 
				{
					jvariant jv(field);
					return jv;
				}

				virtual bool set_field_value(jvariant _field)
				{
					field = _field;
				}

				virtual std::string get_field_name() { return object_name; }
				virtual std::string get_field_description() { return object_description; }
				virtual jtype get_field_type() { return jtype_identifier::get().from(field); }
			};

			template <typename poco, int32_t length_items> class poco_iarray_field : public array_field_base
			{
			public:
				iarray<poco, length_items>& array_field;
				virtual std::shared_ptr<object_base> create_object() = 0;
				virtual std::shared_ptr<object_base> get_object(int _index) = 0;
				virtual int32_t get_object_count() = 0;
				virtual array_field_base* as_array_field() { return this; };
				virtual scalar_field_base* as_scalar_field() { return nullptr; };
			};

			template <int32_t istring_length> class poco_istring_field : public scalar_field_base
			{
				istring<istring_length>& field;
				std::string object_name;
				std::string object_description;

			public:

				poco_istring_field(const std::string& _object_name, const std::string& _object_description, istring<istring_length>& _field) 
					: field(_field),
					object_name(_object_name),
					object_description(_object_description)
				{

				}

				virtual jvariant get_field_value()
				{
					jvariant jv(field.c_str());
					return jv;
				}

				virtual bool set_field_value(jvariant _field)
				{
					field = _field;
				}

				virtual std::string get_field_name() { return object_name; }
				virtual std::string get_field_description() { return object_description; }
				virtual jtype get_field_type() { return jtype::type_string; }
			};

			class object_wrapper : public object_base
			{
				std::vector<std::shared_ptr<field_base>> fields;
				std::string class_name;

			public:

				object_wrapper(const std::string& _class_name) : class_name(_class_name)
				{
					
				}

				template <typename scalar_type> void bind_scalar(const std::string& _name, const std::string& _description, scalar_type& scalar)
				{
					auto base = new poco_scalar_field(_name, _description, _scalar);
					fields.push_back(base);
				}

				template <int32_t max_length> void bind_istring(const std::string& _name, const std::string& _description, istring<max_length>& _str)
				{
					auto base = new istring_scalar_field(_name, _description, _str);
					fields.push_back(base);
				}

				template <int32_t max_length> void bind_array(const std::string& _name, const std::string& _description)
				{
					auto base = new array_field(_name, _description, _str);
					fields.push_back(base);
				}

				virtual std::string get_class_name() { return class_name; }
				virtual int get_field_count() { return fields.size(); }
				virtual std::shared_ptr<field_base> get_field(int _field_index) { return fields[_field_index]; }
				virtual std::shared_ptr<field_base> get_field_by_name(const std::string& _name) {
					auto found_it = std::find_if(fields.begin(), fields.end(), [_name](std::shared_ptr<field_base>& item) {
						return item->get_field_name() == _name;
						});
					if (found_it != std::end(fields)) {
						return *found_it;
					}
					return nullptr;
				};
			};

			class controller
			{
			public:
				
				virtual std::shared_ptr<object_base> create_object(std::string _class_name) = 0;
				virtual std::shared_ptr<array_field_base> create_array(std::string _class_name) = 0;
				virtual bool submit_object(object_base* _object) = 0;
			};

			// -- wrappers

			class string_properties_type_wrapper : public object_wrapper
			{
			public:

				string_properties_type		ref;

				string_properties_type_wrapper() : object_wrapper("string_properties_type")
				{
					bind_scalar("length", "length", ref.length);
					bind_istring("validation_pattern", "validation_pattern", ref.validation_pattern);
					bind_istring("validation_message", "validation_message", ref.validation_message);
				}
			};

			class int_properties_type_wrapper : public object_wrapper
			{
			public:
				int_properties_type ref;

				int_properties_type_wrapper() : object_wrapper("int_properties_type")
				{
					bind_scalar("mininum_int", "mininum_int", ref.minimum_int);
					bind_scalar("maximum_int", "maximum_int", ref.maximum_int);
					bind_istring("format", "format", ref.format);
				}
			};

			class double_properties_type_wrapper : public object_wrapper
			{
			public:
				double_properties_type ref;

				double_properties_type_wrapper() : object_wrapper("double_properties_type")
				{
					bind_scalar("mininum_double", "mininum_double", ref.minimum_double);
					bind_scalar("maximum_double", "maximum_double", ref.maximum_double);
					bind_istring("format", "format", ref.format);
				}
			};

			class time_properties_type_wrapper : public object_wrapper
			{
			public:
				time_properties_type ref;

				time_properties_type_wrapper() : object_wrapper("time_properties_type")
				{
					bind_scalar("mininum_date", "mininum_date", ref.minimum_date);
					bind_scalar("maximum_date", "maximum_date", ref.maximum_date);
					bind_istring("format", "format", ref.format);
				}
			};

			struct currency_properties_type
			{
				CY				minimum_amount;
				CY				maximum_amount;
				object_name		format;
			};

			class currency_properties_type_wrapper : public object_wrapper
			{
			public:
				currency_properties_type ref;

				currency_properties_type_wrapper() : object_wrapper("currency_properties_type")
				{
					bind_scalar("mininum_amount", "mininum_amount", ref.minimum_amount);
					bind_scalar("maximum_amount", "maximum_amount", ref.maximum_amount);
					bind_istring("format", "format", ref.format);
				}
			};

			class point_properties_type_wrapper : public object_wrapper
			{
			public:
				point_properties_type ref;

				point_properties_type_wrapper() : object_wrapper("point_properties_type")
				{
					;
				}
			};

			class rectangle_properties_type_wrapper : public object_wrapper			
			{
			public:
				rectangle_properties_type ref;

				rectangle_properties_type_wrapper() : object_wrapper("rectangle_properties_type")
				{
					;
				}
			};

			class layout_rect_properties_type_wrapper : public object_wrapper
			{
			public:
				layout_rect_properties_type ref;

				layout_rect_properties_type_wrapper() : object_wrapper("layout_rect_properties_type")
				{
					;
				}
			};

			class color_properties_type_wrapper : public object_wrapper
			{
			public:
				color_properties_type ref;

				color_properties_type_wrapper() : object_wrapper("color_properties_type")
				{
					;
				}
			};

			class image_properties_type_wrapper : public object_wrapper
			{
			public:
				image_properties_type ref;

				image_properties_type_wrapper() : object_wrapper("image_properties_type")
				{
					bind_istring("image_path", "image_path", ref.image_path);
				}
			};

			class midi_properties_type_wrapper : public object_wrapper
			{
			public:
				midi_properties_type ref;

				midi_properties_type_wrapper() : object_wrapper("midi_properties_type")
				{
					bind_istring("image_path", "image_path", ref.image_path);
				}
			};

			class wave_properties_type_wrapper : public object_wrapper
			{
			public:
				wave_properties_type ref;

				wave_properties_type_wrapper() : object_wrapper("midi_properties_type")
				{
					bind_istring("image_path", "image_path", ref.image_path);
				}
			};

			struct remote_field_map_type
			{
				object_name corona_field;
				relative_ptr_type corona_field_id;
				object_name remote_field;
			};

			class remote_field_map_type_wrapper : public object_wrapper
			{
			public:
				remote_field_map_type ref;

				remote_field_map_type_wrapper() : object_wrapper("remote_field_map_type")
				{
					bind_istring("corona_field", "corona_field", ref.corona_field);
					bind_scalar("corona_field_id", "corona_field_id", ref.corona_field_id);
					bind_istring("remote_field", "remote_field", ref.remote_field);
				}
			};

			struct remote_mapping_type
			{
				object_name						result_class_name;
				relative_ptr_type				result_class_id;
				remote_parameter_fields_type	parameters;
				remote_fields_type				fields;
			};

			struct remote_status
			{
				DATE						last_success;
				DATE						last_error;
				object_description			error_message;
			};

			class filter_term
			{
			public:
				object_name		  src_value;
				relative_ptr_type target_field;
				comparisons		  comparison;
			};

			class filter_option
			{
			public:
				class_list classes;
				iarray<filter_term, 32> options;
			};


			struct dimensions_type
			{
				corona_size_t x, y, z;

				dimensions_type(corona_size_t _x, corona_size_t _y, corona_size_t _z) : x(_x), y(_y), z(_z) { ; }
				dimensions_type() : x(0), y(0), z(0) { ; }
				bool increment(dimensions_type& _constraint);
			};

			struct file_definition_type
			{
				remote_file_path				file_path;
				remote_mapping_type				mapping;
				remote_status					status;
			};

			struct query_definition_type
			{
				query_body						query;
			};

			struct sql_definition_type
			{
				operation_name					login_type_name;
				sql_login_types					login_type;
				object_name						username;
				object_name						password;
				remote_mapping_type				mapping;
				remote_status					status;
				query_body						query;
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
				remote_mapping_type				mapping;
				remote_status					status;
				query_body						query;
			};

			class put_field_request_base_bridge : public object_base
			{
			public:
				relative_ptr_type		field_id;
				jtype					type_id;
				object_name				name;
				object_description		description;
				bool					is_key;

				relative_ptr_type		enumeration_class_id;
				relative_ptr_type		enumeration_display_field_id;
				relative_ptr_type		enumeration_value_field_id;

				virtual std::string get_class_name() 
				{ 
					return "put_field_request_base"; 
				}
				virtual int get_field_count() 
				{
					return 8;
				}
				virtual std::shared_ptr<array_field_base> get_object_field(int _field_index) = 0;
				virtual std::shared_ptr<scalar_field_base> get_scalar_field(int _field_index) = 0;
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

			class put_currency_field_request {
			public:
				put_field_request_base name;
				currency_properties_type options;
			};

			class put_object_field_request {
			public:
				put_field_request_base name;
				object_properties_type options;
			};

			class put_query_field_request {
			public:
				put_field_request_base name;
				query_definition_type  options;
			};

			class put_sql_remote_field_request {
			public:
				put_field_request_base name;
				sql_definition_type options;
			};

			class put_file_remote_field_request {
			public:
				put_field_request_base name;
				file_definition_type options;
			};

			class put_http_remote_field_request
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

		}
	}
}
