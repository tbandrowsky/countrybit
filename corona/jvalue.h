#pragma once

namespace corona
{
	namespace database
	{

		class jvalue
		{
			relative_ptr_type		field_id;
			jtype					this_type;
			object_description		string_value;
			double					double_value;
			corona_size_t			int_value;
			time_t					time_value;
			rectangle				rectangle_value;
			layout_rect				layout_rect_value;
			point					point_value;
			color					color_value;

			void copy(const jvalue& _src);

		public:
			friend class jobject;

			jvalue() : field_id(null_row), this_type(jtype::type_null), string_value(""), double_value(0.0), int_value(0), time_value(0)
			{
				;
			}

			jvalue(const jvalue& _src);
			jvalue operator =(const jvalue& _src);

			template <explicit_char_ptr t> jvalue(relative_ptr_type _field_id, t _string_value) : field_id(_field_id), string_value(_string_value), this_type(jtype::type_string)
			{
				;
			}

			jvalue(relative_ptr_type _field_id, object_description& _string_value) : field_id(_field_id), string_value(_string_value), this_type(jtype::type_string)
			{
				;
			}

			template <explicit_double t> jvalue(relative_ptr_type _field_id, t _double_value) : field_id(_field_id), double_value(_double_value), this_type(jtype::type_float64)
			{
				;
			}

			template <explicit_int64 t> jvalue(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), this_type(jtype::type_int64)
			{
				;
			}

			template <explicit_int32 t> jvalue(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), this_type(jtype::type_int64)
			{
				;
			}

			template <explicit_bool t> jvalue(relative_ptr_type _field_id, t _bool_value) : field_id(_field_id), int_value(_bool_value), this_type(jtype::type_int64)
			{
				;
			}

			jvalue(relative_ptr_type _field_id, rectangle _rectangle_value) : field_id(_field_id), rectangle_value(_rectangle_value), this_type(jtype::type_rectangle)
			{
				;
			}

			jvalue(relative_ptr_type _field_id, layout_rect _layout_rect_value) : field_id(_field_id), layout_rect_value(_layout_rect_value), this_type(jtype::type_layout_rect)
			{
				;
			}

			jvalue(relative_ptr_type _field_id, point _point_value) : field_id(_field_id), point_value(_point_value), this_type(jtype::type_point)
			{
				;
			}

			jvalue(relative_ptr_type _field_id, color _color_value) : field_id(_field_id), color_value(_color_value), this_type(jtype::type_color)
			{
				;
			}

			bool is_integer(int64_t& v)
			{
				if ((this_type == jtype::type_int8 || this_type == jtype::type_int16 || this_type == jtype::type_int32 || this_type == jtype::type_int64))
				{
					v = int_value;
					return true;
				}
				return false;
			}

			bool is_double(double& _d)
			{
				if ((this_type == jtype::type_float32 || this_type == jtype::type_float64))
				{
					_d = double_value;
					return true;
				}
				return false;
			}

			bool compare(comparisons _comparion, jvalue& _target);

			template <explicit_floating_point t> operator t()
			{
				double r;
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					r = int_value;
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					r = double_value;
					break;
				case jtype::type_collection_id:
					throw std::logic_error("can't convert collection id to double");
					break;
				case jtype::type_color:
					throw std::logic_error("can't convert color to double");
					break;
				case jtype::type_datetime:
					r = time_value;
					break;
				case jtype::type_file:
					throw std::logic_error("can't convert file to double");
					break;
				case jtype::type_http:
					throw std::logic_error("can't convert http to double");
					break;
				case jtype::type_image:
					throw std::logic_error("can't convert image to double");
					break;
				case jtype::type_layout_rect:
					throw std::logic_error("can't convert layout_rect to double");
					break;
				case jtype::type_list:
					throw std::logic_error("can't convert list to double");
					break;
				case jtype::type_midi:
					throw std::logic_error("can't convert midi to double");
					break;
				case jtype::type_null:
					throw std::logic_error("can't convert null to double");
					break;
				case jtype::type_object:
					throw std::logic_error("can't convert object to double");
					break;
				case jtype::type_object_id:
					throw std::logic_error("can't convert object_id to double");
					break;
				case jtype::type_point:
					throw std::logic_error("can't convert point to double");
					break;
				case jtype::type_query:
					throw std::logic_error("can't convert query to double");
					break;
				case jtype::type_rectangle:
					throw std::logic_error("can't convert rectangle to double");
					break;
				case jtype::type_sql:
					throw std::logic_error("can't convert sql to double");
					break;
				case jtype::type_string:
					r = string_value.to_double();
					break;
				case jtype::type_wave:
					throw std::logic_error("can't convert wave to double");
					break;
				}
				return r;
			}

			template <explicit_int t> operator t()
			{
				int64_t z;
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					z = int_value;
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					z = double_value;
					break;
				case jtype::type_collection_id:
					throw std::logic_error("can't convert to int");
					break;
				case jtype::type_color:
					throw std::logic_error("can't convert to int");
					break;
				case jtype::type_datetime:
					z = time_value;
					break;
				case jtype::type_file:
					throw std::logic_error("can't convert file to int");
					break;
				case jtype::type_http:
					throw std::logic_error("can't convert http to int");
					break;
				case jtype::type_image:
					throw std::logic_error("can't convert image to int");
					break;
				case jtype::type_layout_rect:
					throw std::logic_error("can't convert layout_rect to int");
					break;
				case jtype::type_list:
					throw std::logic_error("can't convert list to int");
					break;
				case jtype::type_midi:
					throw std::logic_error("can't convert midi to int");
					break;
				case jtype::type_null:
					throw std::logic_error("can't convert null to int");
					break;
				case jtype::type_object:
					throw std::logic_error("can't convert object to int");
					break;
				case jtype::type_object_id:
					throw std::logic_error("can't convert object_id to int");
					break;
				case jtype::type_point:
					throw std::logic_error("can't convert point to int");
					break;
				case jtype::type_query:
					throw std::logic_error("can't convert query to int");
					break;
				case jtype::type_rectangle:
					throw std::logic_error("can't convert rectangle to int");
					break;
				case jtype::type_sql:
					throw std::logic_error("can't convert sql to int");
					break;
				case jtype::type_string:
					z = string_value.to_long();
					break;
				case jtype::type_wave:
					throw std::logic_error("can't convert wave to int");
					break;
				}
				return z;
			}

			template <explicit_bool t> operator t()
			{
				return (int)*this;
			}

			template <explicit_char_ptr t> operator t()
			{
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					string_value = std::format("{}", int_value);
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					string_value = std::format("{}", double_value);
					break;
				case jtype::type_collection_id:
					string_value = "collection_id";
					break;
				case jtype::type_color:
					string_value = "color";
					break;
				case jtype::type_datetime:
					string_value = "datetime";
					break;
				case jtype::type_file:
					string_value = "file";
					break;
				case jtype::type_http:
					string_value = "http";
					break;
				case jtype::type_image:
					string_value = "image";
					break;
				case jtype::type_layout_rect:
					string_value = "layout_rect";
					break;
				case jtype::type_list:
					string_value = "list";
					break;
				case jtype::type_midi:
					string_value = "midi";
					break;
				case jtype::type_null:
					string_value = "(null type)";
					break;
				case jtype::type_object:
					string_value = "object";
					break;
				case jtype::type_object_id:
					string_value = "object_id";
					break;
				case jtype::type_point:
					string_value = "point";
					break;
				case jtype::type_query:
					string_value = "query";
					break;
				case jtype::type_rectangle:
					string_value = "rectangle";
					break;
				case jtype::type_sql:
					string_value = "sql";
					break;
				case jtype::type_string:
					break;
				case jtype::type_wave:
					string_value = "wave";
					break;
				}
				return string_value.c_str();
			}

			operator std::string();
			operator color ();
			operator point();
			operator rectangle();
			operator layout_rect();

		};

	}
}
