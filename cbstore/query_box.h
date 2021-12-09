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

		enum class query_comparison_types
		{
			eq,
			ls,
			gt,
			lseq,
			gteq,
			contains,
			inlist,
			distance
		};

		struct query_element
		{
		public:
			row_id_type				field_id;
			row_id_type				compared_field_id;
			double					distance_threshold;
			query_comparison_types	comparison;
		};

		template <int length_items> struct iquery
		{
			query_element data[length_items];
			uint32_t length;

			iquery() = default;

			iquery(const std::vector<query_element>& src)
			{
				copy(src.data(), src.size());
			}

			iquery& operator = (const std::vector<query_element>& src)
			{
				copy(src.data(), src.size());
			}

			const query_element* value() const
			{
				return &data[0];
			}

			uint16_t size() const
			{
				return length;
			}

			void copy(int srclength, query_element *src)
			{
				int last_char = length_items - 1;

				query_element* d = &data[0];
				length = 0;

				while (length <= last_char && srclength)
				{
					*d = *s;
					length++;
					s++;
					d++;
					srclength--;
				}
			}

		};

	}

}
