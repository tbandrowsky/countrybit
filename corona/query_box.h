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

		struct filter_element_request
		{
		public:
			const char* target_field_name;
			row_id_type				target_field_id;
			const char* comparison_name;
			filter_comparison_types	comparison;
			const char* parameter_field_name;
			row_id_type				parameter_field_id;
			double					distance_threshold;
			const char* error_message;
		};

		struct filter_element
		{
		public:
			row_id_type				target_field_id;
			filter_comparison_types	comparison;
			row_id_type				parameter_field_id;
			double					distance_threshold;
		};

		struct projection_element_request
		{
		public:
			row_id_type				field_id;
			const char* field_name;
			const char* error_message;
		};

		struct projection_element
		{
		public:
			row_id_type				field_id;
		};

		template <int max_filters, int max_projections>
		class named_query_properties_t
		{
		public:
			iarray<filter_element_request, max_filters> filter;
			iarray<projection_element_request, max_projections> projection;
		};

		using named_query_properties_type = named_query_properties_t<256, 256>;

	}
}
