#pragma once

#include "jobject.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		class navigator
		{
		protected:

			jslice		parameters;
			dynamic_box data;
			using filter_details_type = item_details_table<relative_ptr_type, filter_element>;
			filter_details_type filters;

			void visit(jslice _parameters, jslice _root_slice, path& source_path, filter_element_collection& source_filters);
			virtual void on_node(jslice& _slice) = 0;
			virtual void on_tail(jslice& _slice) = 0;

		private:

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jarray list);
			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jlist list);
			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jslice source_slice);
		};

	}
}
