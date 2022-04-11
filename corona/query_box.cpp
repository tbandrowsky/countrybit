
#include "query_box.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		class query_runner
		{

			jslice		dest_slice;
			jslice		parameters;
			jlist		target;
			jcollection collection;

			dynamic_box data;			

			using filter_details_type = item_details_table<row_id_type, filter_element>;

			filter_details_type filters;

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jmodel list)
			{
				auto li = list.get_model_slice();
				visit(pb, pe, li);
			}

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jarray list)
			{
				for (auto li : list)
				{
					visit(pb, pe, li);
				}
			}

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jlist list)
			{
				for (auto li : list)
				{
					visit(pb, pe, li);
				}
			}

			void visit(path_nodes::iterator pb, path_nodes::iterator pe, jslice source_slice)
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

					dest_slice.copy(source_slice);
				}
				else
				{
					dest_slice = target.append_slice();
				}
			}

			void run_implement(jlist _target, jslice _parameters, jslice _root_slice, query_definition_type& _query_copy)
			{
				path_nodes::iterator pb = _query_copy.source_path.nodes.begin();
				path_nodes::iterator pe = _query_copy.source_path.nodes.end();

				parameters = _parameters;

				jslice path_slice = _root_slice;

				for (auto pni = pb; pni != pe; pni++)
				{
					pni->traversal_index = null_row;
					filter_element_collection temp;
					for (int i = 0; i < path_slice.size(); i++)
					{
						auto& fld = path_slice.get_field(i);
						for (auto filter : _query_copy.filter)
						{
							if (filter.item.target_field_id == fld.field_id) {
								auto new_filter = temp.append();
								*new_filter = filter.item;
							}
						}
					}

					int ts = temp.size();
					if (ts > 0) {
						auto filter_stuff = filters.create(ts);
						auto node = *pni;
						filter_stuff.item() = node.item.member_id;
						for (row_id_type i = 0; i < filter_stuff.size(); i++)
						{
							filter_stuff.detail(i) = temp[i];
						}
						pni->traversal_index = filter_stuff.row_id();
						path_slice.set_filters(filter_stuff.pdetails(), ts, _parameters);
					}
				}

				visit(pb, pe, _root_slice);

			}

		public:

			query_runner()
			{
				;
			}

			void run(jschema* _schema, jclass* _class, jslice* _slice, int _field_index)
			{
				filters.clear();

				auto& fldref = _class->detail(_field_index);
				auto& fld = _schema->get_field(fldref.field_id);

				collection_id_type collection_id;
				init_collection_id(collection_id);

				query_definition_type query_copy = _schema->get_query_definition(fld.query_properties.properties_id);

				jclass_header* root_hdr;
				jmodel root_model;
				jslice root_slice;

				root_hdr = _class->pitem();

				while (root_hdr && root_hdr->class_id != query_copy.source_path.root.model_id)
				{
					root_model = _slice->get_parent_model();
					if (root_model.is_empty()) {
						return;
					}
					root_slice = root_model.get_model_slice();
					root_hdr = root_slice.get_class().pitem();
				}

				if (!root_hdr)
					return;

				auto result_field = _schema->get_field(query_copy.result_field_id);
				auto result_class = _schema->get_class(query_copy.result_class_id);

				int64_t size_bytes = root_hdr->class_size_bytes * 5 / 2;
				int64_t estimated_rows = root_hdr->class_size_bytes / result_class.pitem()->class_size_bytes;

				data.init(size_bytes);

				collection = _schema->create_collection(&data, collection_id, 2, root_hdr->class_size_bytes );
				target = collection.create_list(query_copy.result_class_id, estimated_rows);

				run_implement(target, *_slice, root_slice, query_copy);
			}
		};

		query_box::query_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_index) :
			boxed<query_instance>(t),
			schema(_schema),
			the_class(_class),
			slice(_slice),
			field_index(_field_index)
		{
			;
		}

		query_box::query_box(query_box& _src) : boxed<query_instance>(_src)
		{
			;
		}

		query_box query_box::operator = (const query_box& _src)
		{
			set_data(_src);
			return *this;
		}

		query_box query_box::operator = (query_instance _src)
		{
			set_value(_src);
			return *this;
		}

		query_box::operator query_instance& ()
		{
			query_instance& t = get_data_ref();
			return t;
		}

		query_instance query_box::value() const 
		{ 
			return get_value(); 
		}

		void query_box::run()
		{
			query_runner runner;

			runner.run(schema, the_class, slice, field_index);
		}
	}
}
