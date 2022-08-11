

#include "corona.h"

namespace corona
{
	namespace database
	{

		bool dimensions_type::increment(dimensions_type& _constraint)
		{
			x++;
			if (x >= _constraint.x)
			{
				x = 0;
				y++;
				if (y >= _constraint.y)
				{
					z++;
					y = 0;
					if (z >= _constraint.z)
					{
						z = 0;
						return false;
					}
				}
			}
			return true;
		}

		void model_type::add_path(std::map<relative_ptr_type, int>& _target, relative_ptr_type _leaf)
		{
			for (auto sel_node : select_options)
			{
				if (sel_node.item.select_class_id == _leaf) {
					for (auto rule_node : sel_node.item.selectors.rules) {
						_target[rule_node.item.class_id] = true;
						add_path(_target, rule_node.item.class_id );
					}
				}
			}
		}

		std::vector<relative_ptr_type> model_type::get_selection_classes(relative_ptr_type _leaf, bool _include_leaf)
		{
			std::map<relative_ptr_type, int> path_items_temp;
			if (_include_leaf) {
				path_items_temp[_include_leaf] = true;
			}
			add_path(path_items_temp, _leaf);
			std::vector<relative_ptr_type> ret;
			for (auto m : path_items_temp) {
				ret.push_back(m.first);
			}
			return ret;
		}

		void model_type::create_when(jschema* _schema, std::vector<relative_ptr_type> when_selected,
			relative_ptr_type _create_class_id,
			relative_ptr_type _from_item_class_id,
			bool _select_created,
			bool _replace_selected,
			int _max_creatable_count,
			std::vector<relative_ptr_type> on_created)
		{
			auto co = create_options.append();
			co->create_class_id = _create_class_id;
			co->item_id_class = _from_item_class_id;
			co->replace_selected = _replace_selected;
			co->select_on_create = _select_created;
			co->rule_name = "Create " + _schema->get_class(co->create_class_id).item().name;
			co->selectors.when(when_selected);
			co->create_on_create = on_created;
			co->max_creatable_count = _max_creatable_count;
		}

		void model_type::select_when(jschema* _schema, std::vector<relative_ptr_type> when_selected,
			relative_ptr_type _select_class_id,
			std::vector<relative_ptr_type> on_selected_clear,
			std::vector<relative_ptr_type> on_created,
			view_options _view_options)
		{
			auto so = select_options.append();
			so->select_class_id = _select_class_id;
			so->rule_name = "Select " + _schema->get_class(_select_class_id).item().name;
			so->selectors.when(when_selected);
			so->clear_on_select = on_selected_clear;
			so->create_on_select = on_created;
			so->view_options = _view_options;
			so->view_options.view_class_id = _select_class_id;
		}

		void model_type::update_when(jschema* _schema, std::vector<relative_ptr_type> when_selected, relative_ptr_type _update_class_id,
			std::vector<relative_ptr_type> on_created)
		{
			auto so = update_options.append();
			so->update_class_id = _update_class_id;
			so->rule_name = "Update " + _schema->get_class(_update_class_id).item().name;
			so->selectors.when(when_selected);
			so->create_on_update = on_created;
		}

		void model_type::delete_when(jschema* _schema, std::vector<relative_ptr_type> when_selected, relative_ptr_type _delete_class_id, std::vector<relative_ptr_type> on_created)
		{
			auto so = delete_options.append();
			so->delete_class_id = _delete_class_id;
			so->rule_name = "Delete " + _schema->get_class(_delete_class_id).item().name;
			so->selectors.when(when_selected);
			so->create_on_delete = on_created;
		}
	}
}
