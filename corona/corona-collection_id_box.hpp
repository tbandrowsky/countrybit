/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is a collection id.  A collection in Corona is a binary representation 
of an object designed for impactful batch serialization and processing.

Notes

For Future Consideration
*/
#ifndef CORONA_COLLECTION_ID_H
#define CORONA_COLLECTION_ID_H

namespace corona
{

	class collection_id_math
	{
	public:

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


	};

	template<typename T> std::ostream& operator <<(std::ostream& output, collection_id_box& src)
	{
		output << (T)src;
		return output;
	}

	std::strong_ordering operator<=>(const collection_id_type& a, const collection_id_type& b)
	{
		int64_t ap = *((int64_t*)a.Data4);
		int64_t bp = *((int64_t*)b.Data4);
		return std::tie(a.Data1, a.Data2, a.Data3, ap) <=> std::tie(b.Data1, b.Data2, b.Data3, bp);
	}

	std::strong_ordering compare(const collection_id_type& a, const collection_id_type& b)
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

#endif
