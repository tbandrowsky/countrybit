#pragma once

namespace corona
{
	namespace database
	{
		class path_stack_item
		{
		public:
			relative_ptr_type member_id;
			int				  member_idx;
			dimensions_type   current_dim;
			dimensions_type   max_dim;
		};

		using path_stack_type = iarray<path_stack_item, max_path_nodes>;

		class query_result_item
		{
		public:
			jobject object;
		};

		using query_result_collection = sorted_index<relative_ptr_type, query_result_item>;

		class query_result
		{
			relative_ptr_type result_objects_location;

			jobject create_object(jschema* _schema, relative_ptr_type _class_id);
			jobject copy_object(jschema* _schema, jobject& _src);

		public:

			dynamic_box data;
			query_result_collection result_objects;

			query_result()
			{
				data.init(100000);
				result_objects = query_result_collection::create_sorted_index(&data, result_objects_location);
			}

			query_result(query_result&& _src)
			{
				data = std::move(_src.data);
				result_objects_location = _src.result_objects_location;
				result_objects = query_result_collection::get_sorted_index(&data, result_objects_location);
				for (auto avo : result_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
			}

			query_result& operator=(query_result&& _src)
			{
				data = std::move(_src.data);
				result_objects_location = _src.result_objects_location;
				result_objects = query_result_collection::get_sorted_index(&data, result_objects_location);
				for (auto avo : result_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				return *this;
			}

			query_result operator=(const query_result& _src)
			{
				data = _src.data;
				result_objects_location = _src.result_objects_location;
				result_objects = query_result_collection::get_sorted_index(&data, result_objects_location);
				for (auto avo : result_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				return *this;
			}

			query_result(const query_result& _src)
			{
				data = _src.data;
				result_objects_location = _src.result_objects_location;
				result_objects = query_result_collection::get_sorted_index(&data, result_objects_location);
				for (auto avo : result_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
			}

			jobject append(jschema* _schema, jobject& _src)
			{
				relative_ptr_type id = result_objects.size();
				query_result_item item;
				item.object = copy_object(_schema, _src);
				result_objects.insert_or_assign(id, item);
				return item.object;
			}

			jobject append(jschema* _schema, relative_ptr_type _class_id)
			{
				relative_ptr_type id = result_objects.size();
				query_result_item item;
				item.object = create_object(_schema, _class_id);
				result_objects.insert_or_assign(id, item);
				return item.object;
			}
		};

		relative_ptr_type put_query_class(jschema* _schema, query_definition_type& qd, object_name _name);
		jobject extract_query_item(jobject& _dest, jobject& _root, path_stack_type& _stack);
		query_result run_query(object_name& _query_class_name, jschema* _schema, query_definition_type& _query, filtered_object_list* _collection);
	}
}
