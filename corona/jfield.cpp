

#include "corona.h"

namespace corona
{
	namespace database
	{

		void model_type::create_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _selected_class_id2, relative_ptr_type _create_class_id, relative_ptr_type _from_item_class_id, bool _select_created, bool _replace_selected)
		{
			auto co = create_options.append();
			co->create_class_id = _create_class_id;
			co->item_id_class = _from_item_class_id;
			co->replace_selected = _replace_selected;
			co->select_on_create = _select_created;
			co->rule_name = "Create " + _schema->get_class(co->create_class_id).item().name;
			co->selectors.when(_selected_class_id1, _selected_class_id2 );
		}

		void model_type::create_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _create_class_id, relative_ptr_type _from_item_class_id, bool _select_created, bool _replace_selected)
		{
			auto co = create_options.append();
			co->create_class_id = _create_class_id;
			co->item_id_class = _from_item_class_id;
			co->replace_selected = _replace_selected;
			co->select_on_create = _select_created;
			co->rule_name = "Create " + _schema->get_class(co->create_class_id).item().name;
			co->selectors.when(_selected_class_id1);
		}

		void model_type::create_always(jschema* _schema, relative_ptr_type _create_class_id, relative_ptr_type _from_item_class_id, bool _select_created, bool _replace_selected)
		{
			auto co = create_options.append();
			co->create_class_id = _create_class_id;
			co->item_id_class = _from_item_class_id;
			co->replace_selected = _replace_selected;
			co->select_on_create = _select_created;
			co->rule_name = "Create " + _schema->get_class(co->create_class_id).item().name;
			co->selectors.always();
		}

		void model_type::select_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _selected_class_id2, relative_ptr_type _select_class_id)
		{
			auto so = select_options.append();
			so->select_class_id = _select_class_id;
			so->rule_name = "Select " + _schema->get_class(_select_class_id).item().name;
			so->selectors.when(_selected_class_id1, _selected_class_id2);
		}

		void model_type::select_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _select_class_id)
		{
			auto so = select_options.append();
			so->select_class_id = _select_class_id;
			so->rule_name = "Select " + _schema->get_class(_select_class_id).item().name;
			so->selectors.when(_selected_class_id1);
		}

		void model_type::select_always(jschema* _schema, relative_ptr_type _select_class_id)
		{
			auto so = select_options.append();
			so->select_class_id = _select_class_id;
			so->rule_name = "Select " + _schema->get_class(_select_class_id).item().name;
			so->selectors.always();
		}

		void model_type::update_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _selected_class_id2, relative_ptr_type _update_class_id)
		{
			auto so = update_options.append();
			so->update_class_id = _update_class_id;
			so->rule_name = "Update " + _schema->get_class(_update_class_id).item().name;
			so->selectors.when(_selected_class_id1, _selected_class_id2);
		}

		void model_type::update_when(jschema* _schema, relative_ptr_type _selected_class_id1, relative_ptr_type _update_class_id)
		{
			auto so = update_options.append();
			so->update_class_id = _update_class_id;
			so->rule_name = "Update " + _schema->get_class(_update_class_id).item().name;
			so->selectors.when(_selected_class_id1);
		}

		void model_type::update_always(jschema* _schema, relative_ptr_type _update_class_id)
		{
			auto so = update_options.append();
			so->update_class_id = _update_class_id;
			so->rule_name = "Update " + _schema->get_class(_update_class_id).item().name;
			so->selectors.always();
		}

		void model_type::navigation(std::vector<model_hierarchy_path> items)
		{
			for (auto item : items)
			{
				model_hierarchy_path hhp;
				this->selection_hierarchy.push_back(hhp);
			}
		}
	}
}
