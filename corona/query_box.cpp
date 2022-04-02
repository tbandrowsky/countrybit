
#include "query_box.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		class query_runner
		{

			jslice dest_slice;
			jlist target;
			item_details_table<row_id_type, filter_element> filters;

		public:

			query_runner() 
			{
				;
			}

			void visit(path_nodes::iterator pn, path_nodes::iterator pne, jmodel list, filter_element_collection& collection, row_id_type depth)
			{
				auto li = list.get_model_slice();
				visit(pn, pne, li, collection, depth);
			}

			void visit(path_nodes::iterator pn, path_nodes::iterator pne, jarray list, filter_element_collection& collection, row_id_type depth)
			{
				for (auto li : list)
				{
					visit(pn, pne, li, collection, depth);
				}
			}

			void visit(path_nodes::iterator pn, path_nodes::iterator pne, jlist list, filter_element_collection& collection, row_id_type depth)
			{
				for (auto li : list)
				{
					visit(pn, pne, li, collection, depth);
				}
			}

			void visit(path_nodes::iterator pn, path_nodes::iterator pne, jslice source_slice, filter_element_collection& collection, row_id_type depth)
			{
				path_nodes::iterator pstart;

				dest_slice.copy(source_slice);

				if (filters.size() <= depth)
				{
					filter_element_collection temp;

					for (int i = 0; i < source_slice.size(); i++)
					{
						auto& fld = source_slice.get_field(i);
						for (auto filter : collection)
						{
							if (filter.item.target_field_id == fld.field_id) {
								auto new_filter = temp.append();
								*new_filter = filter.item;
							}
						}
					}
					
					int ts = temp.size();
					if (ts > 0) {
						auto filter_stuff = filters.create_at(depth, ts);
						auto pni = *pn;
						filter_stuff.item() = pni.item.member_id;
						for (row_id_type i = 0; i < filter_stuff.size(); i++)
						{
							filter_stuff.detail(i) = temp[i];
						}
					}
					else 
					{
						auto filter_stuff = filters.create_at(depth, 1);
						filter_element fe;
						fe.target_field_id = null_row;
						fe.parameter_field_id = null_row;
						fe.comparison = filter_comparison_types::eq;
						filter_stuff.detail(0) = fe;
					}
				}

				auto fe = filters[depth];
				for (row_id_type i = 0; i < fe.size(); i++)
				{
					auto filterx = fe.detail(i);

				}

				pstart = pn;

				if (pstart != pne)
				{
					auto pitem = *pstart;
					auto& mf = source_slice.get_field(pitem.item.member_id);
					pstart++;
					depth++;

					switch (mf.type_id)
					{
					case jtype::type_list:
						auto lst = source_slice.get_list(pitem.item.member_index);
						visit(pstart, pne, lst, collection, depth);
						break;
					case jtype::type_object:
						auto obj = source_slice.get_object(pitem.item.member_index);
						visit(pstart, pne, obj, collection, depth);
						break;
					case jtype::type_model:
						auto mdl = source_slice.get_model(pitem.item.member_index);
						visit(pstart, pne, mdl, collection, depth);
						break;
					}
				}
				else
				{
					dest_slice = target.append_slice();
				}
			}

			int run(jlist _target, path_nodes::iterator pn, path_nodes::iterator pe, jmodel root_model, filter_element_collection& collection)
			{
				filters.clear();
				target = _target;
				dest_slice = target.append_slice();
				visit(pn, pe, root_model, collection, 0);
				return target.size();
			}

		};

		query_box::query_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_index) :
			instance(t),
			schema(_schema),
			the_class(_class),
			slice(_slice),
			field_index(_field_index)
		{
			;
		}

		query_box::query_box(query_box& _src) : instance(_src.instance)
		{
			;
		}

		query_box query_box::operator = (const query_box& _src)
		{
			instance = _src.instance;
			return *this;
		}

		query_box query_box::operator = (query_instance _src)
		{
			instance = _src;
			return *this;
		}

		query_box::operator query_instance& ()
		{
			query_instance& t = instance.get_data_ref();
			return t;
		}

		query_instance query_box::value() const 
		{ 
			return instance.get_value(); 
		}

		void query_box::run()
		{
			auto& fldref = the_class->detail(field_index);
			auto& fld = schema->get_field(fldref.field_id);
			auto query = schema->get_query_definition(fld.query_properties.properties_id);

			jclass_header *root_hdr;
			jmodel root_model;
			jslice root_slice;

			root_hdr = the_class->pitem();

			while (root_hdr && root_hdr->class_id != query.source_path.root.model_id)
			{
				root_model = slice->get_parent_model();
				if (root_model.is_empty()) {
					return;
				}
				root_slice = root_model.get_model_slice();
				root_hdr = root_slice.get_class().pitem();
			}

			if (!root_hdr)
				return;

			path_nodes traversal;

			for (auto pn : query.source_path.nodes) 
			{
				if (pn.item.node_operation == node_operations::traverse) 
				{
					traversal.push_back(pn.item);
				}
			}

			path_nodes::iterator pn = traversal.begin();
			path_nodes::iterator pne = traversal.end();

			jlist results;


		}
	}
}
