#pragma once

#include "store_box.h"
#include "array_box.h"
#include "string_box.h"
#include "jfield.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{

		class jcollection;
		class jdatabase;

		enum class filter_comparison_types
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

		struct filter_element
		{
		public:
			object_name				target_field_name;
			row_id_type				target_field_id;
			filter_comparison_types	comparison;
			row_id_type				parameter_field_id;
			object_name				parameter_field_name;
			double					distance_threshold;

			static filter_element filter_eq(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_ls(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_gt(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_lseq(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_gteq(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_contains(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_inlist(row_id_type _target_field_id, row_id_type _parameter_field_id);
			static filter_element filter_distance(row_id_type _target_field_id, row_id_type _parameter_field_id, double _threshold);
		};

		struct projection_element
		{
		public:
			row_id_type				field_id;
			object_name				field_name;
		};

		template <typename query> concept jxquery =
		requires (query a, int b, filter_element fe, projection_element pe, collection_id_type ci)
		{
			fe = a.filter[b];
			pe = a.projection[b];
		};

		template <int max_filters, int max_projections> 
		class iquery 
		{
		public:
			iarray<filter_element, max_filters> filter;
			iarray<projection_element, max_projections> projection;

			void clear();
		};
	}
}
