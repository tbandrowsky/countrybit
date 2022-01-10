
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{
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

		int operator>(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const collection_id_box& a, const collection_id_box& b)
		{
			return compare(a, b) != 0;
		}

	}
}
