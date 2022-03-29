
#include "query_box.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

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

		void visit(path_nodes::iterator pn, path_nodes::iterator pne, jmodel list, jslice dest_slice)
		{
			auto li = list.get_model_slice();
			visit(pn, pne, list, dest_slice);
		}

		void visit(path_nodes::iterator pn, path_nodes::iterator pne, jarray list, jslice dest_slice)
		{
			for (auto li : list) 
			{
				visit(pn, pne, li, dest_slice);
			}
		}

		void visit(path_nodes::iterator pn, path_nodes::iterator pne, jlist list, jslice dest_slice)
		{
			for (auto li : list) 
			{
				visit( pn, pne, li, dest_slice);
			}
		}

		void visit(path_nodes::iterator pn, path_nodes::iterator pne, jslice source_slice, jslice dest_slice)
		{
			path_nodes::iterator pstart;
			pstart = pn;

			while (pstart != pne && (*pstart).item.node_operation != node_operations::traverse)
			{
				pstart++;
			}

			dest_slice.copy(source_slice);

			if (pstart != pne) 
			{
				auto pitem = *pstart;
				auto& mf = source_slice.get_field(pitem.item.member_id);
				switch (mf.type_id)
				{
				case jtype::type_list:
					break;
				case jtype::type_object:
					break;
				case jtype::type_model:
					break;
				}
			}
			else 
			{

			}
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
				traversal.push_back(pn.item);
			}

			path_nodes::iterator pn = traversal.begin();
			path_nodes::iterator pne = traversal.end();

			auto new_class = schema->get_class(query.result_class_id);
			new_class.pitem()->class_size_bytes;



		}
	}
}
