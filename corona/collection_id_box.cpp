
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{
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
