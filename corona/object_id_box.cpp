
#include "jdatabase.h"

namespace corona
{
	namespace database
	{
		int compare(const object_id_box& a, const object_id_box& b)
		{
			object_id_type av = a.value();
			object_id_type bv = b.value();

			int64_t d;
			d = av.collection_id.Data1 - bv.collection_id.Data1;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			d = av.collection_id.Data2 - bv.collection_id.Data2;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			d = av.collection_id.Data3 - bv.collection_id.Data3;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			int64_t avl = *(int64_t*)(&av.collection_id.Data4[0]);
			int64_t bvl = *(int64_t*)(&bv.collection_id.Data4[0]);
			d = avl - bvl;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			d = av.row_id - bv.row_id;
			if (d < 0) return -1;
			else if (d > 0) return 1;
			return 0;
		}

		int operator<(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const object_id_box& a, const object_id_box& b)
		{
			return compare(a, b) != 0;
		}


	}
}