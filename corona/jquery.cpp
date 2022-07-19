
#include "corona.h"

namespace corona
{
	namespace database
	{
		relative_ptr_type put_query_class(jschema* _schema, query_definition_type& qd, object_name _name)
		{
			relative_ptr_type class_id;

			put_class_request request;
			request.class_id = null_row;
			request.class_description = "query class " + _name;
			request.class_name = _name;
			request.field_id_primary_key = null_row;
			request.template_class_id = null_row;
			request.base_class_id = null_row;
			for (auto fld : qd.project) {
				member_field mf(fld.item.field_id);
				request.member_fields.push_back(mf);
			}
			class_id = _schema->put_class(request);
			return class_id;
		}

		jobject extract_query_item(serialized_box_container* _container, relative_ptr_type _dest_class_id, jobject _root, path_stack_type& _stack)
		{
			auto schema = _root.get_schema();
			auto class_type = schema->get_class(_dest_class_id);
			auto total_bytes = class_type.item().class_size_bytes;
			auto location = _container->reserve(total_bytes);
			jobject new_object(nullptr, schema, _dest_class_id, _container, location, { 1, 1, 1 });
			for (auto fld : _stack) {
				new_object.update(_root);
				_root = new_object.get_object(fld.item.member_idx).get_slice(fld.item.current_dim);
			}
			return new_object;
		}

		query_result run_query(object_name& _query_class_name, jschema* _schema, query_definition_type& _query, jcollection* _collection)
		{

			query_result results;

			auto& definition = _query;

			// create our destination class
			relative_ptr_type dest_class_id = put_query_class(_schema, definition, _query_class_name);

			path_stack_type path_stack;
			// figure out how to walk through the collection

			jcollection::iterator_type iter;

			if (definition.root.query_root_type == query_root_types::root_class)
			{
				iter = _collection->where(definition.root.root_item_id);
			}
			else if (definition.root.query_root_type == query_root_types::root_object)
			{
				auto item_id = definition.root.root_item_id;
				iter = _collection->where([item_id](auto& item) { return item_id == item.location; });
			}

			for (auto item : iter)
			{
				path_stack.clear();
				auto object = item.item;
				auto root = object;

				// build the iterator

				for (auto node : definition.path)
				{
					path_stack_item psi;

					int field_index = object.get_field_index_by_id(node.item.field_id);
					if (field_index > -1)
					{
						psi.member_id = node.item.field_id;
						psi.member_idx = field_index;

						auto& fld = object.get_field(field_index);
						if (fld.is_container())
						{
							psi.max_dim = fld.object_properties.dim;
							psi.current_dim = { 0, 0, 0 };
							object = object.get_slice(psi.member_idx, { 0, 0, 0 }, false);
						}
						else
						{
							psi.max_dim = { 1, 1, 1 };
							psi.current_dim = { 0, 0, 0 };
						}
						path_stack.push_back(psi);
					}
					else
					{
						break;
					}
				}

				// run the iterator

				int bottom = path_stack.size() - 1;

				while (bottom > -1)
				{
					jobject obj = extract_query_item(&results.data, dest_class_id, root, path_stack);


					bool incrementing = true;
					while (incrementing && bottom > -1)
					{
						auto& psix = path_stack[bottom];
						incrementing = !psix.current_dim.increment(psix.max_dim);
						bottom--;
					}
				}
			}
		}
	}
}
