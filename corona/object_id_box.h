#pragma once

namespace corona
{
	namespace database
	{
		class jarray;
		class jdatabase;

		class object_id_math
		{
		public:
			jarray get_object(object_id_type _object_id, jdatabase* _db);
		};

		class object_id_box : protected boxed<object_id_type>
		{
		public:
			object_id_box(char* t) : boxed<object_id_type>(t)
			{
				;
			}

			object_id_box operator = (const object_id_box& _src)
			{
				set_data(_src);
				return *this;
			}

			object_id_box operator = (object_id_type _src)
			{
				set_value(_src);
				return *this;
			}

			operator object_id_type& ()
			{
				object_id_type& t = boxed<object_id_type>::get_data_ref();
				return t;
			}

			object_id_type value() const { return boxed<object_id_type>::get_value(); }

			jarray get_object(jdatabase* _db);

		};

		int compare(const object_id_box& a, const object_id_box& b);

		int operator<(const object_id_box& a, const object_id_box& b);
		int operator>(const object_id_box& a, const object_id_box& b);
		int operator>=(const object_id_box& a, const object_id_box& b);
		int operator<=(const object_id_box& a, const object_id_box& b);
		int operator==(const object_id_box& a, const object_id_box& b);
		int operator!=(const object_id_box& a, const object_id_box& b);

		template<typename T> std::ostream& operator <<(std::ostream& output, object_id_box& src)
		{
			output << (T)src;
			return output;
		}
	}
}
