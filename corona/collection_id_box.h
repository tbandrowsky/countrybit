#pragma once

namespace corona
{
	namespace database
	{
		class jcollection;
		class jdatabase;

		auto operator<=>(const collection_id_type& a, const collection_id_type& b);
		int operator<(const collection_id_type& a, const collection_id_type& b);
		int operator>(const collection_id_type& a, const collection_id_type& b);
		int operator>=(const collection_id_type& a, const collection_id_type& b);
		int operator<=(const collection_id_type& a, const collection_id_type& b);
		int operator==(const collection_id_type& a, const collection_id_type& b);
		int operator!=(const collection_id_type& a, const collection_id_type& b);


		class collection_id_math
		{
		public:
			static jcollection get_collection(collection_id_type _collection_id, jdatabase* _database);
		};

		class collection_id_box : protected boxed<collection_id_type>
		{
		public:
			collection_id_box(char* t) : boxed<collection_id_type>(t)
			{
				;
			}

			collection_id_box operator = (const collection_id_box& _src)
			{
				set_data(_src);
				return *this;
			}

			collection_id_box operator = (collection_id_type _src)
			{
				set_value(_src);
				return *this;
			}

			operator collection_id_type& ()
			{
				collection_id_type& t = boxed<collection_id_type>::get_data_ref();
				return t;
			}

			operator collection_id_type () const 
			{
				collection_id_type t = boxed<collection_id_type>::get_data_ref();
				return t;
			}

			collection_id_type value() const { return boxed<collection_id_type>::get_value(); }

			static jcollection get_collection(jdatabase* _database);

		};

		template<typename T> std::ostream& operator <<(std::ostream& output, collection_id_box& src)
		{
			output << (T)src;
			return output;
		}
	}
}
