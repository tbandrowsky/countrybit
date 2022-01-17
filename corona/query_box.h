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
			row_id_type				target_field_id;
			filter_comparison_types	comparison;
			row_id_type				parameter_field_id;
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
			row_id_type				parent_field_id;
			jtype					new_field_type;

			union 
			{
				string_properties_type	string_properties;
				int_properties_type		int_properties;
				double_properties_type	double_properties;
				time_properties_type	time_properties;
				object_properties_type  object_properties;
				query_properties_type   query_properties;
			};

			object_name				name;
			object_description		description;

			object_description		string_constant;
			int64_t					int_constant;
			double					float_constant;
			time_t					time_constant;

			static projection_element project_field(row_id_type _parent_field_id, row_id_type _field_id);
			static projection_element project_string(row_id_type _parent_field_id, object_name& _name, object_description& _description, string_properties_type _request, object_description& description);
			static projection_element project_int8(row_id_type _parent_field_id, object_name& _name, object_description& _description, int_properties_type _request, int8_t _value);
			static projection_element project_int16(row_id_type _parent_field_id, object_name& _name, object_description& _description, int_properties_type _request, int16_t _value);
			static projection_element project_int32(row_id_type _parent_field_id, object_name& _name, object_description& _description, int_properties_type _request, int32_t _value);
			static projection_element project_int64(row_id_type _parent_field_id, object_name& _name, object_description& _description, int_properties_type _request, int64_t _value);
			static projection_element project_float(row_id_type _parent_field_id, object_name& _name, object_description& _description, add_double_field_request _request, float _value);
			static projection_element project_double(row_id_type _parent_field_id, object_name& _name, object_description& _description,add_double_field_request _request, double _value);
			static projection_element project_time(row_id_type _parent_field_id, object_name& _name, object_description& _description, time_properties_type _request, time_t _value);
			static projection_element project_object(row_id_type _parent_field_id, object_name& _name, object_description& _description, object_properties_type _request);
			static projection_element project_query(row_id_type _parent_field_id, object_name& _name, object_description& _description, query_properties_type _request);
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
