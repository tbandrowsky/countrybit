
#include "query_box.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		filter_element filter_element::filter_eq(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::eq;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_ls(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::ls;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_gt(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::gt;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_lseq(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::lseq;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_gteq(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::gteq;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;

		}
		filter_element filter_element::filter_contains(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::contains;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_inlist(row_id_type _target_field_id, row_id_type _parameter_field_id)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::inlist;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			return fe;
		}
		filter_element filter_element::filter_distance(row_id_type _target_field_id, row_id_type _parameter_field_id, double _distance)
		{
			filter_element fe;
			fe.comparison = filter_comparison_types::inlist;
			fe.target_field_id = _target_field_id;
			fe.parameter_field_id = _parameter_field_id;
			fe.distance_threshold = _distance;
			return fe;
		}

	}
}
