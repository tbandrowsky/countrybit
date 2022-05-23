#pragma once

namespace corona
{
	namespace database
	{

		class view_box_header
		{
		public:
			relative_ptr_type root_view;
		};

		class view_node_data
		{
		public:
			int node_id;

			relative_ptr_type	slice_class_id;
			relative_ptr_type	slice_offset;
			corona_size_t		slice_length_bytes;
			relative_ptr_type	child_list;
		};

		class view_node_

		class view_box
		{
		public:
			


		};
	}
}
