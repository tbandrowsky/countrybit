
#include "corona.h"

namespace corona
{
	namespace database
	{
		relative_ptr_type analytics_kit::selected_class(filtered_object_list& list)
		{
			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				if (kvp.second.selected)
					return kvp.second.class_id;
			}
			return null_row;
		}

		double analytics_kit::min_field(filtered_object_list& list, relative_ptr_type field_id)
		{
			bool has_value = false;
			double value = 0.0;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				jobject& obj = kvp.second.object;
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

		double analytics_kit::max_field(filtered_object_list& list, relative_ptr_type field_id)
		{
			bool has_value = false;
			double value = 0.0;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				jobject& obj = kvp.second.object;
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

		double analytics_kit::sum_field(filtered_object_list& list, relative_ptr_type field_id)
		{
			bool has_value = false;
			double value = 0.0;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				jobject& obj = kvp.second.object;
				if (obj.has_field(field_id)) {
					double temp_value = obj.get(field_id);
					value += temp_value;
				}
			}

			return value;
		}

		double analytics_kit::avg_field(filtered_object_list& list, relative_ptr_type field_id)
		{
			bool has_value = false;
			double value = 0.0;
			int count = 0;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				jobject& obj = kvp.second.object;
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

		double analytics_kit::count_class(filtered_object_list& list, relative_ptr_type class_id)
		{
			bool has_value = false;
			int count = 0;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				if (kvp.second.class_id == class_id) {
					count++;
				}
			}

			return count;
		}

		double analytics_kit::count_distinct(filtered_object_list& list, relative_ptr_type field_id)
		{
			bool has_value = false;
			double value = 0.0;
			std::map<std::string, int> distinct;

			for (auto item : list) {
				auto kvp = state->view_objects.get(item);
				jobject& obj = kvp.second.object;
				if (obj.has_field(field_id)) {
					std::string temp_value = obj.get(field_id);
					distinct[temp_value] = true;
				}
			}

			value = distinct.size();
			return value;
		}
	}
}