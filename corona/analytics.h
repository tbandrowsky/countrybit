#pragma once

namespace corona
{
	namespace database
	{
		class analytics_kit
		{

			actor_state* state;
			jcollection* collection;

		public:

			analytics_kit() : state(nullptr), collection(nullptr)
			{
				;
			}

			analytics_kit(actor_state* _state, jcollection* _collection) : state(_state), collection(_collection)
			{
				;
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
		};

	}
}

