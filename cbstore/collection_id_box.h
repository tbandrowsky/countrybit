#pragma once
#pragma once

#pragma once

#include "store_box.h"
#include <ostream>
#include <bit>

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

		int compare(const collection_id_box& a, const collection_id_box& b)
		{
			collection_id_type av = a.value();
			collection_id_type bv = b.value();

			int64_t d;
			d = av.Data1 - bv.Data1;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			d = av.Data2 - bv.Data2;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			d = av.Data3 - bv.Data3;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			int64_t avl = *(int64_t*)(&av.Data4[0]);
			int64_t bvl = *(int64_t*)(&bv.Data4[0]);
			d = avl - bvl;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			return 0;
		}

		int operator<(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> int compare(const collection_id_box& a, T& b)
		{
			return compare(a, boxed(b));
		}

		template<typename T> int operator<(const collection_id_box& a, T& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const collection_id_box& a, T& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const collection_id_box& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const collection_id_box& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const collection_id_box& a, T& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const collection_id_box& a, T& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> std::ostream& operator <<(std::ostream& output, collection_id_box& src)
		{
			output << (T)src;
			return output;
		}
	}
}
