
#include "corona.h"

namespace corona
{
	namespace database
	{


		jobject extract_query_item(jobject& _dest, jobject& _root, path_stack_type& _stack)
		{
			for (auto fld : _stack) {
				_dest.update(_root);
				_root = _root.get_object(fld.item.member_idx).get_object(fld.item.current_dim);
			}
			return _dest;
		}

		relative_ptr_type analytics_kit::put_class(field_list& qd, object_name _name)
		{
			relative_ptr_type class_id;

			put_class_request request;
			request.class_id = null_row;
			request.class_description = "query class " + _name;
			request.class_name = _name;
			request.field_id_primary_key = null_row;
			request.template_class_id = null_row;
			request.base_class_id = null_row;
			for (auto fld : qd) {
				member_field mf(fld.item);
				request.member_fields.push_back(mf);
			}
			class_id = schema->put_class(request);
			return class_id;
		}

		filtered_object_list analytics_kit::flatten_list(filtered_object_list* _collection, relative_ptr_type _target_class_id, jschema* _schema, flatten_option& _query)
		{
			filtered_object_list results;

			// create our destination class
			relative_ptr_type dest_class_id = _target_class_id;

			path_stack_type path_stack;
			// figure out how to walk through the collection

			filtered_object_list::iterator_type iter = _collection->begin();

			for (auto item : *_collection)
			{
				path_stack.clear();
				auto object = item;
				auto root = object;

				// build the iterator

				for (auto node : _query.path)
				{
					path_stack_item psi;

					int field_index = object.get_field_index_by_id(node.item);
					if (field_index > -1)
					{
						psi.member_id = node.item;
						psi.member_idx = field_index;

						auto& fld = object.get_field(field_index);
						if (fld.is_container())
						{
							psi.max_dim = fld.object_properties.dim;
							psi.current_dim = { 0, 0, 0 };
							object = object.get_object(psi.member_idx, { 0, 0, 0 }, false);
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
					jobject obj = create_object(dest_class_id);						
					extract_query_item(obj, root, path_stack);
					results.push_back(obj);

					bool incrementing = true;
					while (incrementing && bottom > -1)
					{
						auto& psix = path_stack[bottom];
						incrementing = !psix.current_dim.increment(psix.max_dim);
						bottom--;
					}
				}
			}

			return results;
		}

		filtered_object_list analytics_kit::join_list(filtered_object_list* _collection, relative_ptr_type _target_class_id, jschema* _schema, relative_ptr_type _source_classic_id)
		{
			filtered_object_list dest_list;
			dest_list = filtered_object_list::create(&data);
			join_path mini_stack;

			for (auto obj : *_collection)
			{
				if (this->collection->object_is_class(obj, _source_classic_id))
				{
					join_object(dest_list, mini_stack, obj, 0, _target_class_id);
				}
			}
			return dest_list;
		}

		jobject analytics_kit::create_object(relative_ptr_type _class_id)
		{
			auto myclass = schema->get_class(_class_id);
			auto bytes_to_allocate = myclass.item().class_size_bytes;
			relative_ptr_type location = data.reserve(bytes_to_allocate);

			dimensions_type d = { 0,0,0 };

			jobject ja(nullptr, schema, _class_id, &data, location, d);
			ja.construct();
#if _TRACE_GET_OBJECT
			std::cout << "actor create object " << (void*)&data << " " << std::endl;
#endif
			return ja;
		}

		jobject analytics_kit::copy_object(jobject& _src)
		{
			jobject dest = create_object(_src.get_class().item().class_id);
			dest.update(_src);
			return dest;
		}

		void analytics_kit::join_object(filtered_object_list& _target, join_path& _ministack, jobject& parent, int _level, relative_ptr_type _dest_type_id)
		{
			auto cls = parent.get_class();
			auto pk_idx = cls.item().primary_key_idx;

			bool at_leaf = true;

			_level = _level + 1;
			for (int i = 0; i < cls.size(); i++)
			{
				if (pk_idx != i) {
					auto& fld = cls.detail(i);
					auto& fld_def = schema->get_field(fld.field_id);
					auto field_id = parent.get(fld.field_id);
					if (state->view_objects.contains(field_id)) {
						_ministack[_level] = field_id;
						jobject& jobj = state->view_objects[field_id].second.object;
						join_object(_target, _ministack, jobj, _level, _dest_type_id);
						at_leaf = false;
					}
				}
			}

			if (at_leaf)
			{
				jobject new_object = create_object(_dest_type_id);
				auto sz = _ministack.size();
				for (int i = 0; i < sz; i++)
				{
					auto mi = _ministack[i];
					auto obj = state->view_objects[mi].second.object;
					new_object.update(obj);
				}
				_target.push_back(new_object);
			}
		}
	}
}
