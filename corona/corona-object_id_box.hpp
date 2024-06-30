/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_OBJECT_ID_H
#define CORONA_OBJECT_ID_H

namespace corona {

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


#endif
