#pragma once

namespace corona
{
	namespace database
	{

		class jvariant
		{
			relative_ptr_type		field_id;
			jtype					variant_type;

			object_description		string_value;
			double					double_value;
			corona_size_t			int_value;
			DATE					time_value;
			CY						currency_value;

			void copy(const jvariant& _src);

		public:

			friend class jobject;

			jvariant() : field_id(null_row), variant_type(jtype::type_null)
			{
				;
			}

			jvariant(const jvariant& _src);
			jvariant operator =(const jvariant& _src);

			jvariant(relative_ptr_type _field_id) : variant_type(jtype::type_null)
			{
				;
			}

			template <explicit_char_ptr t> jvariant(relative_ptr_type _field_id, t _string_value) : field_id(_field_id), string_value(_string_value), variant_type(jtype::type_string)
			{
				;
			}

			jvariant(relative_ptr_type _field_id, object_description& _string_value) : field_id(_field_id), string_value(_string_value), variant_type(jtype::type_string)
			{
				;
			}

			template <explicit_double t> jvariant(relative_ptr_type _field_id, t _double_value) : field_id(_field_id), double_value(_double_value), variant_type(jtype::type_float64)
			{
				;
			}

			template <explicit_int64 t> jvariant(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), variant_type(jtype::type_int64)
			{
				;
			}

			template <explicit_int32 t> jvariant(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), variant_type(jtype::type_int64)
			{
				;
			}

			template <explicit_bool t> jvariant(relative_ptr_type _field_id, t _bool_value) : field_id(_field_id), int_value(_bool_value), variant_type(jtype::type_int64)
			{
				;
			}

			bool is_integer(int64_t& v)
			{
				if ((variant_type == jtype::type_int8 || variant_type == jtype::type_int16 || variant_type == jtype::type_int32 || variant_type == jtype::type_int64))
				{
					v = int_value;
					return true;
				}
				return false;
			}

			bool is_double(double& _d)
			{
				if ((variant_type == jtype::type_float32 || variant_type == jtype::type_float64))
				{
					_d = double_value;
					return true;
				}
				return false;
			}

			bool compare(comparisons _comparion, jvariant& _target);

			template <explicit_floating_point t> operator t()
			{
				double r;
				switch (variant_type)
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
				case jtype::type_datetime:
					r = time_value;
					break;
				case jtype::type_string:
					r = string_value.to_double();
					break;
				default:
					r = 0;
					break;
				}
				return r;
			}

			template <explicit_int t> operator t()
			{
				int64_t z;
				switch (variant_type)
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
				case jtype::type_datetime:
					z = time_value;
					break;
				case jtype::type_string:
					z = string_value.to_long();
					break;
				default:
					z = 0;
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
				switch (variant_type)
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
				case jtype::type_datetime:
					string_value = "datetime";
					break;
				case jtype::type_image:
					string_value = "image";
					break;
				case jtype::type_midi:
					string_value = "midi";
					break;
				case jtype::type_null:
					string_value = "(null type)";
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


		};

		class jvariant_member 
		{
		public:
			std::string		  name;
			relative_ptr_type field_id;
		};

		class jvariant_member_value : public jvariant_member
		{
		public:
			jvariant value;
		};

		class jvariant_member_array : public jvariant_member
		{
		public:
			std::vector<jvariant_member*> value;
		};

		class jvariant_object
		{
		public:
			std::string class_name;
			std::vector<jvariant_member *> members;
		};
	}
}
