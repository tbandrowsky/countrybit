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
		const int max_query_projections = 32;
		const int max_from_elements = 32;

		struct from_element_request
		{
		public:
			row_id_type			field_id;
			collection_id_type	collection_id;
			object_name			field_name;
			const char*			error_message;
		};

		struct from_element
		{
		public:
			row_id_type			field_id;
			object_name			field_name;
		};

		enum class from_set_bases
		{
			from_current_object,
			from_current_collection,
			from_specific_collection
		};

		struct from_set_request
		{
		public:
			from_set_bases from_set_base;
			collection_id_type collection_id;
			object_name collection_name;
			iarray<from_element_request, max_from_elements> path;
		};

		struct from_set
		{
		public:
			from_set_bases from_set_base;
			collection_id_type collection_id;
			object_name collection_name;
			iarray<from_element, max_from_elements> path;
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

		struct projection_element_request
		{
		public:
			row_id_type				field_id;
			object_name				field_name;
			const char*				error_message;
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
			from_set from;
			iarray<filter_element_request, max_filters> filter;
			iarray<projection_element_request, max_projections> projection;
		};

		using named_query_properties_type = named_query_properties_t<max_query_filters, max_query_projections>;

		class query_instance
		{
		public:
			collection_id_type			collection;
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
