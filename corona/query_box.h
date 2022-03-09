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
		class jschema;
		class jclass;

		const int max_query_filters = 32;
		const int max_path_nodes = 64;

		class path_root
		{
		public:
			object_name collection_name;
			object_name class_name;
		};

		class path_node
		{
		public:
			object_name member_name;
			row_id_type	member_id;
		};

		class path
		{
		public:
			path_root							root;
			iarray<path_node, max_path_nodes>	nodes;
		};

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
			object_name				target_field_name;
			row_id_type				target_field_id;
			object_name				comparison_name;
			filter_comparison_types	comparison;
			object_name				parameter_field_name;
			row_id_type				parameter_field_id;
			double					distance_threshold;
			const char*				error_message;
		};

		struct filter_element
		{
		public:
			row_id_type				target_field_id;
			filter_comparison_types	comparison;
			row_id_type				parameter_field_id;
			double					distance_threshold;
		};

		template <int max_filters>
		class named_query_properties_t
		{
		public:
			path		source_path;
			object_name result_field;
			iarray<filter_element_request, max_filters> filter;
		};

		using named_query_properties_type = named_query_properties_t<max_query_filters>;

		class query_instance
		{
		public:
			time_t						last_success;
			time_t						last_error;
			object_description			error_message;
		};

		class query_box 
		{
			boxed<query_instance> instance;
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			query_box(char* t, jschema *_schema, jclass *_class, jslice *_slice) : instance(t), schema(_schema), the_class(_class), slice(_slice)
			{
				;
			}

			query_box(query_box& _src) : instance(_src.instance)
			{
				;
			}

			query_box operator = (const query_box& _src)
			{
				instance = _src.instance;
				return *this;
			}

			query_box operator = (query_instance _src)
			{
				instance = _src;
				return *this;
			}

			operator query_instance& ()
			{
				query_instance& t = instance.get_data_ref();
				return t;
			}

			query_instance value() const { return instance.get_value(); }
		};

	}
}
