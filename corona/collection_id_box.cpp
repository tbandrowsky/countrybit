
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{
		auto operator<=>(const collection_id_type& a, const collection_id_type& b)
		{
			int64_t ap = *((int64_t*)a.Data4);
			int64_t bp = *((int64_t*)b.Data4);
			return std::tie(a.Data1, a.Data2, a.Data3, ap) <=> std::tie(b.Data1, b.Data2, b.Data3, bp);
		}

		auto compare(const collection_id_type& a, const collection_id_type& b)
		{
			return a <=> b;
		}

		int operator<(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const collection_id_type& a, const collection_id_type& b)
		{
			return compare(a, b) != 0;
		}


	}
}
