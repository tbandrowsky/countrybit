#pragma once

#include <ostream>
#include <bit>

#include "store_box.h"
#include "string_box.h"

namespace countrybit
{
	namespace database
	{
		class jcollection;
		class jdatabase;

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
				boxed<collection_id_type>::operator =(_src);
				return *this;
			}

			collection_id_box operator = (collection_id_type _src)
			{
				boxed<collection_id_type>::operator =(_src);
				return *this;
			}

			operator collection_id_type& ()
			{
				collection_id_type& t = boxed<collection_id_type>::get_data_ref();
				return t;
			}

			collection_id_type value() const { return boxed<collection_id_type>::get_value(); }

			static jcollection get_collection(jdatabase* _database);

		};

		int compare(const collection_id_box& a, const collection_id_box& b);

		int operator<(const collection_id_box& a, const collection_id_box& b);
		int operator>(const collection_id_box& a, const collection_id_box& b);
		int operator>=(const collection_id_box& a, const collection_id_box& b);
		int operator<=(const collection_id_box& a, const collection_id_box& b);
		int operator==(const collection_id_box& a, const collection_id_box& b);
		int operator!=(const collection_id_box& a, const collection_id_box& b);

		template<typename T> std::ostream& operator <<(std::ostream& output, collection_id_box& src)
		{
			output << (T)src;
			return output;
		}
	}
}
