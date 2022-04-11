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
		class navigator
		{
		public:

			jslice		parameters;
			dynamic_box data;
			using filter_details_type = item_details_table<row_id_type, filter_element>;
			filter_details_type filters;

		protected:

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jmodel list);
			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jarray list);
			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jlist list);
			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jslice source_slice);

			virtual void on_node(jslice& _slice) = 0;
			virtual void on_tail(jslice& _slice) = 0;
		};

	}
}
