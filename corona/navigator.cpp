
#include "navigator.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		void navigator::visit(path_nodes::iterator pb, path_nodes::iterator pe, jarray list)
		{
			for (auto li : list)
			{
				visit(pb, pe, li);
			}
		}

		void navigator::visit(path_nodes::iterator pb, path_nodes::iterator pe, jlist list)
		{
			for (auto li : list)
			{
				visit(pb, pe, li);
			}
		}

		void navigator::visit(path_nodes::iterator pb, path_nodes::iterator pe, jslice source_slice)
		{
			path_nodes::iterator pstart;

			if (pb != pe)
			{
				auto fe = filters[pb->traversal_index];
				if (fe.detail(0).target_field_id != null_row) {
					if (!source_slice.filter(fe.pdetails(), fe.size(), parameters))
						return;
				}

				auto pitem = *pstart;
				auto& mf = source_slice.get_field(pitem.item.member_id);

				pb++;

				switch (mf.type_id)
				{
				case jtype::type_list:
				{
					auto lst = source_slice.get_list(pitem.item.member_index);
					visit(pb, pe, lst);
				}
				break;
				case jtype::type_object:
				{
					auto obj = source_slice.get_object(pitem.item.member_index);
					visit(pb, pe, obj);
				}
				break;
				}

				on_node(source_slice);
			}
			else
			{
				on_tail(source_slice);
			}
		}

		void navigator::visit(jslice _parameters, jslice _root_slice, path& source_path, filter_element_collection& source_filters)
		{
			path_nodes::iterator pb = source_path.nodes.begin();
			path_nodes::iterator pe = source_path.nodes.end();

			parameters = _parameters;

			jslice path_slice = _root_slice;

			for (auto pni = pb; pni != pe; pni++)
			{
				pni->traversal_index = null_row;
				filter_element_collection temp;
				for (int i = 0; i < path_slice.size(); i++)
				{
					auto& fld = path_slice.get_field(i);
					for (auto filter : source_filters)
					{
						if (filter.item.target_field_id == fld.field_id) {
							auto new_filter = temp.append();
							*new_filter = filter.item;
						}
					}
				}

				int ts = temp.size();
				if (ts > 0) {
					auto node = *pni;
					auto filter_stuff = filters.create_item(&node.item.member_id, ts, temp.data);
					pni->traversal_index = filter_stuff.row_id();
					path_slice.set_filters(filter_stuff.pdetails(), ts, _parameters);
				}
			}

			visit(pb, pe, _root_slice);

		}


	}
}
