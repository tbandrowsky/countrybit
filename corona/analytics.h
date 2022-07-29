#pragma once

namespace corona
{
	namespace database
	{

		class join_term
		{
		public:
			relative_ptr_type class_id_source;
			relative_ptr_type field_id_common;
			relative_ptr_type class_id_target;
		};

		using join_terms = iarray<join_term, 32>;

		class join_option
		{
		public:
			join_terms joins;
		};

		class flatten_option
		{
		public:
			field_list						path;
			field_list						project;
		};

		class analytics_kit
		{
			actor_state* state;
			jcollection* collection;
			jschema* schema;

			jobject create_object(relative_ptr_type _class_id);
			jobject copy_object(jobject& _src);
			dynamic_box data;

		public:

			analytics_kit() : state(nullptr), collection(nullptr)
			{
				;
			}

			analytics_kit(jschema* _schema, actor_state* _state, jcollection* _collection) : schema(_schema), state(_state), collection(_collection)
			{
				;
			}

			void init( int size = 1<<21)
			{
				data.init(size);
			}

			jobject get_object(filtered_object_list::iterator_item_type& item)
			{
				return item;
			}

			jobject get_object(filtered_actor_view_object_list::iterator_item_type& item)
			{
				return item.object;
			}

			relative_ptr_type selected_class(filtered_actor_view_object_list& list)
			{
				for (auto item : list) {
					if (item.selected)
						return item.class_id;
				}
				return null_row;
			}

			template <typename list_type> double min_field(list_type& list, relative_ptr_type field_id)
			{
				bool has_value = false;
				double value = 0.0;

				for (auto item : list) {
					jobject& obj = get_object(item);
					if (obj.has_field(field_id)) {
						double temp_value = obj.get(field_id);
						if (has_value) {
							value = temp_value < value ? temp_value : value;
						}
						else {
							value = temp_value;
						}
					}
				}

				return value;
			}

			template <typename list_type> double max_field(list_type& list, relative_ptr_type field_id)
			{
				bool has_value = false;
				double value = 0.0;

				for (auto item : list) {
					jobject& obj = get_object(item);
					if (obj.has_field(field_id)) {
						double temp_value = obj.get(field_id);
						if (has_value) {
							value = temp_value > value ? temp_value : value;
						}
						else {
							value = temp_value;
						}
					}
				}

				return value;
			}

			template <typename list_type> double sum_field(list_type& list, relative_ptr_type field_id)
			{
				bool has_value = false;
				double value = 0.0;

				for (auto item : list) {
					jobject& obj = get_object(item.object);
					if (obj.has_field(field_id)) {
						double temp_value = obj.get(field_id);
						value += temp_value;
					}
				}

				return value;
			}

			template <typename list_type> double avg_field(list_type& list, relative_ptr_type field_id)
			{
				bool has_value = false;
				double value = 0.0;
				int count = 0;

				for (auto item : list) {
					jobject& obj = get_object(item.object);
					if (obj.has_field(field_id)) {
						double temp_value = obj.get(field_id);
						value += temp_value;
						count++;
					}
				}

				if (count) {
					value /= count;
				}

				return value;
			}

			template <typename list_type> double count_class(list_type& list, relative_ptr_type class_id)
			{
				bool has_value = false;
				int count = 0;

				for (auto item : list) {
					jobject& obj = get_object(item.object);
					if (obj.get_class_id() == class_id) {
						count++;
					}
				}

				return count;
			}

			template <typename list_type> double count_distinct(list_type& list, relative_ptr_type field_id)
			{
				bool has_value = false;
				double value = 0.0;
				std::map<std::string, int> distinct;

				for (auto item : list) {
					jobject& obj = get_object(item.object);
					if (obj.has_field(field_id)) {
						std::string temp_value = obj.get(field_id);
						distinct[temp_value] = true;
					}
				}

				value = distinct.size();
				return value;
			}

			relative_ptr_type put_class(field_list& qd, object_name _name);

			filtered_object_list flatten_list(filtered_object_list* _collection, object_name& _query_class_name, jschema* _schema, flatten_option &_query);
			filtered_object_list join_list(filtered_object_list* _collection, object_name& _query_class_name, jschema* _schema, class_list classes);

		};

	}
}

