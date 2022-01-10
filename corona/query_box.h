#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
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
			row_id_type				target_field_id;
			query_comparison_types	comparison;
			row_id_type				parameter_field_id;
			double					distance_threshold;
		};

		template <typename query> concept query_path =
		requires (query a, int b, query_element c, collection_id_type ci) 
		{
			c = a[b];
			b = a.size();
			ci = a.collection_id();
		};

		template <int max_items> 
		class iquery 
		{
		public:
			iarray<query_element, max_items> terms;
			istring<255> collection_template_name;
		};
	}
}
