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
			class object_array_field_base;
			class scalar_array_field_base;
			class scalar_field_base;

			class field_base
			{
			public:
				virtual std::string get_field_name() = 0;
				virtual std::string get_field_description() = 0;
				virtual jtype get_field_type() = 0;
				virtual object_array_field_base* as_object_array_field() = 0;
				virtual scalar_array_field_base* as_scalar_array_field() = 0;
				virtual scalar_field_base* as_scalar_field() = 0;
			};

			class array_field_base : public field_base
			{
			public:
			};

			class scalar_array_field_base : public array_field_base
			{
			public:
				virtual jvariant create_scalar() = 0;
				virtual jvariant get_scalar(int _index) = 0;
				virtual jvariant put_scalar(int _index, jvariant& _src) = 0;
				virtual int32_t get_scalar_count() = 0;
				virtual object_array_field_base* as_object_array_field() { return nullptr; };
				virtual scalar_array_field_base* as_scalar_array_field() { return this; };
				virtual scalar_field_base* as_scalar_field() { return nullptr; };
			};

			class object_array_field_base : public array_field_base
			{
			public:
				virtual object_base *create_object() = 0;
				virtual object_base *get_object(int _index) = 0;
				virtual int32_t get_object_count() = 0;
				virtual object_array_field_base* as_object_array_field() { return this; };
				virtual scalar_array_field_base* as_scalar_array_field() { return nullptr; };
				virtual scalar_field_base* as_scalar_field() { return nullptr; };
			};

			class scalar_field_base : public field_base
			{
			public:
				virtual jvariant get_field_value() = 0;
				virtual bool set_field_value(jvariant _field) = 0;
				virtual object_array_field_base* as_object_array_field() { return nullptr; };
				virtual scalar_array_field_base* as_scalar_array_field() { return nullptr; };
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

			template <typename object_wrapper_type, typename poco, int32_t length_items> class object_iarray_field : public object_array_field_base
			{
			public:
				std::string object_name;
				std::string object_description;
				iarray<poco, length_items>& ref;

				object_iarray_field(const std::string& _object_name, const std::string& _object_description, iarray<poco, length_items>& _ref) :
					object_name(_object_name),
					object_description(_object_description),
					array_field(_ref)
				{

				}

				virtual std::string get_field_name() { return object_name; }
				virtual std::string get_field_description() { return object_description; }
				virtual jtype get_field_type() { return jtype::type_object; }

				virtual std::shared_ptr<object_base> create_object()
				{
					auto ptemp = ref.append();
					auto wrapper = new object_wrapper_type(*ptemp);
					return wrapper;
				}
				virtual std::shared_ptr<object_base> get_object(int _index)
				{
					auto ref_item = ref[_index];
					auto wrapper = new object_wrapper_type(ref_item);
					return wrapper;
				}
				virtual int32_t get_object_count()
				{
					return ref.size();
				}
				virtual object_array_field_base* as_object_array_field() { return this; };
				virtual scalar_array_field_base* as_scalar_array_field() { return nullptr; };
				virtual scalar_field_base* as_scalar_field() { return nullptr; };
			};

			template <typename object_wrapper_type, typename poco> class object_vector_field : public object_array_field_base
			{
			public:
				std::string object_name;
				std::string object_description;
				std::vector<poco> &ref;

				object_vector_field(const std::string& _object_name, const std::string& _object_description, std::vector<poco>& _ref) :
					object_name(_object_name),
					object_description(_object_description),
					ref(_ref)
				{

				}

				virtual std::string get_field_name() { return object_name; }
				virtual std::string get_field_description() { return object_description; }
				virtual jtype get_field_type() { return jtype::type_object; }

				virtual std::shared_ptr<object_base> create_object()
				{
					poco temp = {}
					ref.push_back(temp);
					auto& ptemp = ref.back();
					auto wrapper = new object_wrapper_type(ptemp);
					return wrapper;
				}

				virtual std::shared_ptr<object_base> get_object(int _index)
				{
					auto ref_item = ref[_index];
					auto wrapper = new object_wrapper_type(ref_item);
					return wrapper;
				}

				virtual int32_t get_object_count()
				{
					return ref.size();
				}

				virtual object_array_field_base* as_object_array_field() { return this; };
				virtual scalar_array_field_base* as_scalar_array_field() { return nullptr; };
				virtual scalar_field_base* as_scalar_field() { return nullptr; };
			};

			template <typename scalar, int length_items> class scalar_iarray_field : public scalar_array_field_base
			{
			public:
				std::string object_name;
				std::string object_description;
				iarray<scalar, length_items>& ref;

				scalar_iarray_field(const std::string& _object_name, const std::string& _object_description, iarray<scalar, length_items>& _ref) :
					object_name(_object_name),
					object_description(_object_description),
					ref(_ref)
				{

				}

				virtual std::string get_field_name() { return object_name; }
				virtual std::string get_field_description() { return object_description; }
				virtual jtype get_field_type() { return jtype::type_object; }

				virtual jvariant create_item()
				{
					auto ptemp = ref.append();
					jvariant t = *ptemp;
					return t;
				}
				virtual jvariant get_scalar(int _index)
				{
					jvariant t = ref[_index];
					return t;
				}
				virtual jvariant set_scalar(int _index, jvariant& v)
				{
					ref[_index] = v;
					return v;
				}
				virtual int32_t get_scalar_count()
				{
					return ref.size();
				}
				virtual object_array_field_base* as_object_array_field() { return nullptr; };
				virtual scalar_array_field_base* as_scalar_array_field() { return this; };
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

				template <typename wrapper_type, typename poco> void bind_object(const std::string& _name, const std::string& _description, poco& _ref)
				{
					auto base = new wrapper_type(_name, _description, _ref);
					fields.push_back(base);
				}

				template <typename wrapper_type, typename poco, int32_t max_length> void bind_object_array(const std::string& _name, const std::string& _description, iarray<poco, max_length>& _ref )
				{
					auto base = new object_iarray_field<wrapper_type, poco, max_length>(_name, _description, _str);
					fields.push_back(base);
				}

				template <typename wrapper_type, typename poco> void bind_object_array(const std::string& _name, const std::string& _description, std::vector<poco>& _ref)
				{
					auto base = new object_vector_field<wrapper_type, poco>(_name, _description, _str);
					fields.push_back(base);
				}

				template <typename scalar, int32_t max_length> void bind_scalar_array(const std::string& _name, const std::string& _description, iarray<scalar, max_length>& _ref)
				{
					auto base = new scalar_iarray_field<scalar, max_length>(_name, _description, _str);
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

			template <typename poco> class poco_object_wrapper : public object_wrapper
			{
			public:
				poco& ref;
				poco_object_wrapper(const std::string& _object_name, poco& _ref) : object_wrapper(_object_name), ref(_ref)
				{
					;
				}
			};

			class controller
			{
			public:
				
				virtual std::shared_ptr<object_base> create_object(std::string _class_name) = 0;
				virtual std::shared_ptr<array_field_base> create_array(std::string _class_name) = 0;
				virtual bool submit_object(object_base* _object) = 0;
			};

			// -- wrappers

			class string_properties_type_wrapper : public poco_object_wrapper<string_properties_type>
			{
			public:

				string_properties_type_wrapper(string_properties_type& _ref) : poco_object_wrapper("string_properties_type", _ref)
				{
					bind_scalar("length", "length", ref.length);
					bind_istring("validation_pattern", "validation_pattern", ref.validation_pattern);
					bind_istring("validation_message", "validation_message", ref.validation_message);
				}
			};

			class int_properties_type_wrapper : public poco_object_wrapper<int_properties_type>
			{
			public:

				int_properties_type_wrapper(int_properties_type& _ref) : poco_object_wrapper("int_properties_type", _ref)
				{
					bind_scalar("mininum_int", "mininum_int", ref.minimum_int);
					bind_scalar("maximum_int", "maximum_int", ref.maximum_int);
					bind_istring("format", "format", ref.format);
				}
			};

			class double_properties_type_wrapper : public poco_object_wrapper<double_properties_type>
			{
			public:
				double_properties_type_wrapper(double_properties_type& _ref) : poco_object_wrapper("double_properties_type", _ref)
				{
					bind_scalar("mininum_double", "mininum_double", ref.minimum_double);
					bind_scalar("maximum_double", "maximum_double", ref.maximum_double);
					bind_istring("format", "format", ref.format);
				}
			};

			class time_properties_type_wrapper : public poco_object_wrapper<time_properties_type>
			{
			public:
				
				time_properties_type_wrapper(time_properties_type& _ref) : poco_object_wrapper("time_properties_type", _ref)
				{
					bind_scalar("mininum_date", "mininum_date", ref.minimum_date);
					bind_scalar("maximum_date", "maximum_date", ref.maximum_date);
					bind_istring("format", "format", ref.format);
				}
			};

			class currency_properties_type_wrapper : public poco_object_wrapper<currency_properties_type>
			{
			public:
				currency_properties_type_wrapper(currency_properties_type& _ref) : poco_object_wrapper("currency_properties_type", _ref)
				{
					bind_scalar("mininum_amount", "mininum_amount", ref.minimum_amount);
					bind_scalar("maximum_amount", "maximum_amount", ref.maximum_amount);
					bind_istring("format", "format", ref.format);
				}
			};

			class point_properties_type_wrapper : public poco_object_wrapper<point_properties_type>
			{
			public:
				point_properties_type_wrapper(point_properties_type& _ref) : poco_object_wrapper("point_properties_type", _ref)
				{
					;
				}
			};

			class rectangle_properties_type_wrapper : public poco_object_wrapper<rectangle_properties_type>
			{
			public:

				rectangle_properties_type_wrapper(rectangle_properties_type& _ref) : poco_object_wrapper("rectangle_properties_type", _ref)
				{
					;
				}
			};

			class layout_rect_properties_type_wrapper : public poco_object_wrapper<layout_rect_properties_type>
			{
			public:

				layout_rect_properties_type_wrapper(layout_rect_properties_type& _ref) : poco_object_wrapper("layout_rect_properties_type", _ref)
				{
					;
				}
			};

			class color_properties_type_wrapper : public poco_object_wrapper<color_properties_type>
			{
			public:

				color_properties_type_wrapper(color_properties_type& _ref) : poco_object_wrapper<color_properties_type>("color_properties_type", _ref)
				{
					;
				}
			};

			class image_properties_type_wrapper : public poco_object_wrapper<image_properties_type>
			{
			public:

				image_properties_type_wrapper(image_properties_type& _ref) : poco_object_wrapper("image_properties_type", _ref)
				{
					bind_istring("image_path", "image_path", ref.image_path);
				}
			};

			class midi_properties_type_wrapper : public poco_object_wrapper<midi_properties_type>
			{
			public:

				midi_properties_type_wrapper(midi_properties_type& _ref) : poco_object_wrapper("midi_properties_type", _ref)
				{
					bind_istring("image_path", "image_path", _ref.image_path);
				}
			};

			class wave_properties_type_wrapper : public poco_object_wrapper<wave_properties_type>
			{
			public:
				wave_properties_type_wrapper(wave_properties_type& _ref) : poco_object_wrapper("midi_properties_type", _ref)
				{
					bind_istring("image_path", "image_path", ref.image_path);
				}
			};

			class remote_field_map_type_wrapper : public poco_object_wrapper<remote_field_map_type>
			{
			public:

				remote_field_map_type_wrapper(remote_field_map_type& _ref) : poco_object_wrapper("remote_field_map_type", _ref)
				{
					bind_istring("corona_field", "corona_field", ref.corona_field);
					bind_scalar("corona_field_id", "corona_field_id", ref.corona_field_id);
					bind_istring("remote_field", "remote_field", ref.remote_field);
				}
			};

			class remote_mapping_type_wrapper : public poco_object_wrapper<remote_mapping_type>
			{
			public:
				remote_mapping_type_wrapper(remote_mapping_type& _ref) : poco_object_wrapper("remote_mapping_type", _ref)
				{
					bind_istring("result_class_name", "result_class_name", ref.result_class_name);
					bind_scalar("result_class_id", "result_class_id", ref.result_class_id);
					bind_object_array<remote_field_map_type_wrapper, remote_field_map_type>("parameters", "parameters", ref.parameters);
					bind_object_array<remote_field_map_type_wrapper, remote_field_map_type>("fields", "fields", ref.fields);
				}
			};

			class remote_status_wrapper : public poco_object_wrapper<remote_status>
			{
			public:
				remote_status_wrapper(remote_status& _ref) : poco_object_wrapper("remote_status", _ref)
				{
					bind_scalar("last_sucess", "last_success", ref.last_success);
					bind_scalar("last_error", "last_error", ref.last_error);
					bind_scalar("error_message", "error_message", ref.error_message);
				}
			};

			class filter_term_wrapper : public poco_object_wrapper<filter_term>
			{
			public:
				filter_term_wrapper(filter_term& _ref) : poco_object_wrapper("filter_term", _ref)
				{
					bind_scalar("src_value", "src_value", ref.src_value);
					bind_scalar("target_field", "target_field", ref.target_field);
					bind_scalar("comparison", "comparison", ref.comparison);
				}
			};

			class filter_option_wrapper : public poco_object_wrapper<filter_option>
			{
			public:
				filter_option_wrapper(filter_option& _ref) : poco_object_wrapper("filter_option", _ref)
				{
					bind_scalar_array("classes", "classes", ref.classes);
					bind_object_array<filter_term_wrapper,filter_term>("options", "options", ref.options);
				}
			};

			class dimensions_type_wrapper : public poco_object_wrapper<dimensions_type>
			{
			public:
				dimensions_type_wrapper(dimensions_type& _ref) : poco_object_wrapper("dimensions", _ref)
				{
					bind_scalar("x", "x", ref.x);
					bind_scalar("y", "y", ref.y);
					bind_scalar("z", "z", ref.z);
				}
			};

			class object_properties_type_wrapper : public poco_object_wrapper<object_properties_type>
			{
			public:
				object_properties_type_wrapper(object_properties_type& _ref) : poco_object_wrapper("object_properties_type", _ref)
				{
					bind_object<dimensions_type_wrapper>("dim", "dim", _ref.dim);
					bind_istring("class_name", "class name", _ref.class_name);
					bind_scalar("class_id", "class id", _ref.class_id);
					bind_scalar("class_size_bytes", "class_size_bytes", _ref.class_size_bytes);
					bind_scalar("total_size_bytes", "total_size_bytes", _ref.total_size_bytes);
				}
			};

			struct file_definition_type
			{
				remote_file_path				file_path;
				remote_mapping_type				mapping;
				remote_status					status;
			};

			class file_definition_type_wrapper : public poco_object_wrapper<file_definition_type>
			{
			public:
				file_definition_type_wrapper(file_definition_type& _ref) : poco_object_wrapper("file_definition_type", _ref)
				{
					bind_istring("x", "x", ref.file_path);
					bind_object<remote_mapping_type_wrapper>("y", "y", ref.mapping);
					bind_object<remote_status_wrapper>("z", "z", ref.status);
				}
			};

			class query_definition_type_wrapper : public poco_object_wrapper<query_definition_type>
			{
			public:
				query_definition_type_wrapper(query_definition_type& _ref) : poco_object_wrapper("query_definition_type", _ref)
				{
					bind_istring("query", "query", ref.query);
				}
			};

			class sql_definition_type_wrapper : public poco_object_wrapper<sql_definition_type>
			{
				sql_definition_type_wrapper(sql_definition_type& _ref) : poco_object_wrapper("sql_definition_type", _ref)
				{
					bind_istring("login_type_name", "login_type_name", ref.login_type_name);
					bind_istring("username", "username", ref.username);
					bind_istring("password", "password", ref.password);
					bind_object<remote_mapping_type_wrapper, remote_mapping_type>("mapping", "mapping", ref.mapping);
					bind_object<remote_status_wrapper, remote_status>("status", "status", ref.status);
					bind_object<query_definition_type_wrapper, query_definition_type>("query", "query", ref.query);
				}
			};

			class http_definition_type_wrapper : public poco_object_wrapper<http_definition_type>
			{
				http_definition_type_wrapper(http_definition_type& _ref) : poco_object_wrapper("http_definition_type", _ref)
				{
					bind_istring("login_type_name", "login_type_name", ref.login_type_name);
					bind_istring("login_method", "login_method", ref.login_method);
					bind_istring("password", "password", ref.password);
					bind_istring("username", "username", ref.username);
					bind_istring("password", "password", ref.password);
					bind_object<remote_mapping_type_wrapper, remote_mapping_type>("mapping", "mapping", ref.mapping);
					bind_object<remote_status_wrapper, remote_status>("status", "status", ref.status);
					bind_object<query_definition_type_wrapper, query_definition_type>("query", "query", ref.query);
					bind_istring("data_method", "data_method", ref.data_method);
					bind_istring("data_url", "data_url", ref.data_url);
					bind_istring("login_method", "login_method", ref.login_method);
					bind_istring("login_url", "login_url", ref.login_url);
				}
			};

			class put_field_request_base_wrapper : public poco_object_wrapper<put_field_request_base>
			{
			public:
				put_field_request_base_wrapper(put_field_request_base& _ref) : poco_object_wrapper("put_field_request_base", _ref)
				{
					bind_scalar("field_id", "field_id", ref.field_id);
					bind_scalar("type_id", "type_id", ref.type_id);
					bind_istring("name", "name", ref.name);
					bind_istring("description", "description", ref.description);
					bind_scalar("is_key", "is_key", ref.is_key);
					bind_scalar("enumeration_class_id", "enumeration_class_id", ref.enumeration_class_id);
					bind_scalar("enumeration_display_field_id", "enumeration_display_field_id", ref.enumeration_display_field_id);
					bind_scalar("enumeration_value_field_id", "enumeration_value_field_id", ref.enumeration_value_field_id);
				}
			};

			class put_string_field_request_wrapper : public poco_object_wrapper<put_string_field_request> {
			public:
				put_string_field_request_wrapper(put_string_field_request& _ref) : poco_object_wrapper("put_string_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<string_properties_type_wrapper>("options", "options", _ref.options);
				}
			};


			class put_integer_field_request_wrapper : public poco_object_wrapper<put_integer_field_request> {
			public:
				put_integer_field_request_wrapper(put_integer_field_request& _ref) : poco_object_wrapper("put_integer_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<int_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_double_field_request_wrapper : public poco_object_wrapper<put_double_field_request> {
			public:
				put_double_field_request_wrapper(put_double_field_request& _ref) : poco_object_wrapper("put_double_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<double_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_time_field_request_wrapper : public poco_object_wrapper<put_time_field_request> {
			public:
				put_time_field_request_wrapper(put_time_field_request& _ref) : poco_object_wrapper("put_time_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<time_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_currency_field_request_wrapper : public poco_object_wrapper<put_currency_field_request> {
			public:
				put_currency_field_request_wrapper(put_currency_field_request& _ref) : poco_object_wrapper("put_currency_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<currency_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_object_field_request_wrapper : public poco_object_wrapper<put_object_field_request> {
			public:
				put_object_field_request_wrapper(put_object_field_request& _ref) : poco_object_wrapper("put_object_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<object_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_query_field_request_wrapper : public poco_object_wrapper<put_query_field_request> {
			public:
				put_query_field_request_wrapper(put_query_field_request& _ref) : poco_object_wrapper("put_query_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<query_definition_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_sql_remote_field_request_wrapper : public poco_object_wrapper<put_sql_remote_field_request> {
			public:
				put_sql_remote_field_request_wrapper(put_sql_remote_field_request& _ref) : poco_object_wrapper("put_sql_remote_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<sql_definition_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_file_remote_field_request_wrapper : public poco_object_wrapper<put_file_remote_field_request> {
			public:
				put_file_remote_field_request_wrapper(put_file_remote_field_request& _ref) : poco_object_wrapper("put_file_remote_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<file_definition_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_http_remote_field_request_wrapper : public poco_object_wrapper<put_http_remote_field_request> {
			public:
				put_http_remote_field_request_wrapper(put_http_remote_field_request& _ref) : poco_object_wrapper("put_http_remote_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<http_definition_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_point_field_request_wrapper : public poco_object_wrapper<put_point_field_request> {
			public:
				put_point_field_request_wrapper(put_point_field_request& _ref) : poco_object_wrapper("put_point_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<point_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_rectangle_field_request_wrapper : public poco_object_wrapper<put_rectangle_field_request> {
			public:
				put_rectangle_field_request_wrapper(put_rectangle_field_request& _ref) : poco_object_wrapper("put_rectangle_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<rectangle_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_layout_rect_field_request_wrapper : public poco_object_wrapper<put_layout_rect_field_request> {
			public:
				put_layout_rect_field_request_wrapper(put_layout_rect_field_request& _ref) : poco_object_wrapper("put_layout_rect_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<layout_rect_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_image_field_request_wrapper : public poco_object_wrapper<put_image_field_request> {
			public:
				put_image_field_request_wrapper(put_image_field_request& _ref) : poco_object_wrapper("put_image_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<image_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_wave_field_request_wrapper : public poco_object_wrapper<put_wave_field_request> {
			public:
				put_wave_field_request_wrapper(put_wave_field_request& _ref) : poco_object_wrapper("put_wave_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<wave_properties_type_wrapper>("options", "options", _ref.options);
				}
			};

			class put_color_field_request_wrapper : public poco_object_wrapper<put_color_field_request> {
			public:
				put_color_field_request_wrapper(put_color_field_request& _ref) : poco_object_wrapper("put_color_field_request", _ref)
				{
					bind_object<put_field_request_base_wrapper>("name", "name", _ref.name);
					bind_object<color_properties_type_wrapper>("options", "options", _ref.options);
				}
			};




		}
	}
}
