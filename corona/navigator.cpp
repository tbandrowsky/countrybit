
#include "navigator.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		void navigator::visit(path_nodes::iterator pb, path_nodes::iterator pe, jmodel list)
		{
			auto li = list.get_model_slice();
			visit(pb, pe, li);
		}

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
				case jtype::type_model:
				{
					auto mdl = source_slice.get_model(pitem.item.member_index);
					visit(pb, pe, mdl);
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

	}
}
