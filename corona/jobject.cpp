
#include "corona.h"

#define _DETAIL 0
#define _TRACE_RULE 0

namespace corona
{
	namespace database
	{

		int compare(const dimensions_type& a, const dimensions_type& b)
		{
			corona_size_t t = a.z - b.z;
			if (t) return t;
			t = a.y - b.y;
			if (t) return t;
			t = a.x - b.x;
			return t;
		}

		int operator<(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) != 0;
		}

		bool init_collection_id(collection_id_type &collection_id)
		{
			::GUID gidReference;
			HRESULT hr = ::CoCreateGuid((GUID *) &collection_id);
			return hr == S_OK;
		}

		jobject actor_state::create_object(jschema* _schema, relative_ptr_type _class_id)
		{
			auto myclass = _schema->get_class(_class_id);
			auto bytes_to_allocate = myclass.item().class_size_bytes;
			relative_ptr_type location = data.reserve(bytes_to_allocate);

			dimensions_type d = { 0,0,0 };

			jobject ja(nullptr, _schema, _class_id, &data, location, d);
			ja.construct();
			return ja;
		}

		jobject actor_state::copy_object(jschema* _schema, jobject& _src)
		{
			jobject dest = create_object(_schema, _src.get_class().item().class_id);
			dest.update(_src);
			return dest;
		}

		actor_view_object actor_state::get_modified_object()
		{
			if (modified_object_id == null_row) {
				throw std::logic_error("No modified object");
			}
			return view_objects[ modified_object_id ].get_value();
		}

		bool jcollection::selector_applies(selector_collection* _selector, actor_id_type& _actor)
		{
			auto& actor = actors[_actor];
			auto selections = &actor.selections;
			selector_rule_collection* required = &_selector->rules;

			// we can allow the option to create a class if there are no rules for selection.
			// if we do create the option here, it will in fact be a new item.
			if (required->size() == 0)
			{
				return true;
			}
			// we can allow the option to create a class, if and only if we exactly satisfy the 
			// selectors on this option. if we do create on the option here, it will have the item id
			// of the class specified in the rule so that continuity will be preserved.
			else 
			{
				bool all_required_items_satisfied = 
				required->all_of([this, selections](auto& src) {
					int c = selections->count_if(
						[src, this](auto& dest) {
							return matches_class_id(dest.item,src.item.class_id);
						});
#if _TRACE_RULE
					std::cout << "  selected count of " << src.item.class_id << " " << c << std::endl;
#endif
					return c == 1;
					});

				bool all_selections_required = selections->all_of([this, required](auto& dest) {
					bool result = required->any_of([this, dest](auto& src) {
						return matches_class_id(dest.item, src.item.class_id);
						});
#if _TRACE_RULE						
					std::cout << "   check " << dest.item << " " << result << std::endl;
#endif
						return result;
					});

#if _TRACE_RULE
				std::cout << "  all_required " << all_required_items_satisfied << " " << "all_selected " << all_selections_required << std::endl;
#endif

				return all_required_items_satisfied && all_selections_required;
			}

			return false;
		}

		void jcollection::print(const char *_trace, actor_state& acr)
		{
			for (auto co : acr.create_objects)
			{
				std::cout << "create class: " << schema->get_class( co.second.class_id ).item().name << std::endl;
			}

			for (auto vo : acr.view_objects)
			{
				auto slice = get_at(vo.second.object_id);
				std::cout << "existing object: " << vo.second.object_id << " (" << slice.get_class().item().name << ") selectable:" <<  vo.second.selectable << " selected:" << vo.second.selected << " updatable:" << vo.second.updatable << std::endl;
			}
			if (_trace) {
				std::cout << _trace << std::endl;
			}
		}

		actor_state jcollection::get_actor_state(relative_ptr_type _actor, relative_ptr_type _last_modified_object, const char* _trace_msg)
		{
			actor_state acr;

			if (_trace_msg) {
				std::cout << _trace_msg << std::endl;
			}

			acr.collection_id = collection_id;

			auto model = schema->get_model(ref->model_name);
			auto& actor = actors[ _actor ];

			acr.actor = actor;

			auto& create_options = model.create_options;
			auto& select_options = model.select_options;
			auto& update_options = model.update_options;
			auto selections = &actor.selections;

			// first figure out our create_options

			for (auto oi : create_options)
			{
				auto rule = &oi.item;
				auto required = rule->selectors;

#if _TRACE_RULE
				std::cout << "check rule " << oi.item.rule_name << std::endl;
#endif

				if (selector_applies(&oi.item.selectors, _actor)) {
					create_object_request aco;

					aco.collection_id = collection_id;
					aco.actor_id = _actor;
					aco.class_id = oi.item.create_class_id;
					aco.select_on_create = oi.item.select_on_create;
					aco.template_item_id = null_row;
					aco.item_id = null_row;

					auto selected_create = selections->where([rule, this](auto& src) {
						return this->matches_class_id(src.item, rule->item_id_class);
						});
					if (selected_create != std::end(*selections)) {
						relative_ptr_type object_id = selected_create.get_object().item;
						relative_ptr_type item_id = objects[object_id].item().item_id;
						aco.item_id = item_id;
					}
					else 
					{
						aco.item_id = null_row;
					}

					auto cls = schema->get_class(rule->create_class_id);
					relative_ptr_type template_class_id = cls.item().template_class_id;
					if (template_class_id != null_row)
					{
						auto selected_template = selections->where([template_class_id, this](auto& src) {
							return this->matches_class_id(src.item, template_class_id);
							});
						if (selected_template != std::end(*selections))
						{
							relative_ptr_type object_id = selected_template.get_object().item;
							relative_ptr_type item_id = objects[object_id].item().item_id;
							aco.template_item_id = item_id;
							acr.create_objects.insert_or_assign(aco.class_id, aco);
#if _TRACE_RULE
							std::cout << " " << oi.item.rule_name << " applies " << std::endl;
#endif
						}
						else {
#if _TRACE_RULE
							std::cout << " " << oi.item.rule_name << " does not apply because the template class was not selected " << std::endl;
#endif
						}
					}
					else 
					{
						acr.create_objects.insert_or_assign(aco.class_id, aco);
#if _TRACE_RULE
						std::cout << " " << oi.item.rule_name << " applies " << std::endl;
#endif
					}
				}
				else 
				{
#if _TRACE_RULE
					std::cout << " " << oi.item.rule_name << " does not apply " << std::endl;
#endif
				}
			}

			// now for the moment, we just include all the objects in the view.  This can change for physical models, obviously.

			for (auto iter = begin(); iter != end(); iter++)
			{
				actor_view_object avo;
				avo.actor_id = _actor;
				avo.collection_id = collection_id;
				avo.object_id = iter.get_index();
				avo.class_id = iter.get_object().item.get_class_id();
				avo.selectable = false;
				avo.selected = false;
				avo.updatable = false;
				acr.view_objects.put(iter.get_index(), avo, [](actor_view_object& _dest) { ;  });
			}

			// now to select whatever is selected

			for (auto sel : actor.selections) {
				auto obj = objects[sel.item];
				actor_view_object avo;
				avo.collection_id = collection_id;
				avo.object_id = sel.item;
				avo.selectable = false;
				avo.selected = true;
				avo.updatable = false;
				acr.view_objects.put(sel.item, avo, [](actor_view_object& _dest) { _dest.selected = true; });
			}

			// now to our select options
			
			for (auto oi : select_options)
			{
				auto rule = &oi.item;
				auto required = &oi.item.selectors;

				if (selector_applies(&oi.item.selectors, _actor)) 
				{
					// we can now select objects of this class
					for (relative_ptr_type oid = 0; oid < objects.size(); oid++)
					{
						auto obj = objects[oid];
						if (obj.item().class_id == rule->select_class_id) 
						{
							actor_view_object avo;
							avo.collection_id = collection_id;
							avo.object_id = oid;
							avo.selectable = true;
							avo.selected = false;
							avo.updatable = false;
							acr.view_objects.put(oid, avo, [](actor_view_object& _dest) { 
								_dest.selectable = !_dest.selected;  });
						}
					}
				}
			}

			// and to our update options

			for (auto oi : update_options)
			{
				auto rule = &oi.item;
				auto required = &oi.item.selectors;

				if (selector_applies(&oi.item.selectors, _actor))
				{
					// we can now select objects of this class
					for (relative_ptr_type oid = 0; oid < objects.size(); oid++)
					{
						auto obj = objects[oid];
						if (obj.item().class_id == rule->update_class_id)
						{
							actor_view_object avo;
							avo.collection_id = collection_id;
							avo.object_id = oid;
							avo.selectable = false;
							avo.selected = false;
							avo.updatable = true;
							acr.view_objects.put(oid, avo, [](actor_view_object& _dest) { 								
								_dest.updatable = true; 																						
								});
						}
					}
				}
			}

			acr.modified_object_id = _last_modified_object;
			if (acr.modified_object_id != null_row)
			{
				jobject slice = get_object(acr.modified_object_id);
				acr.modified_object = acr.copy_object(schema, slice);
			}

			if (_trace_msg)
			{
				print("-------", acr);
			}
			return acr;
		}

		actor_type jcollection::create_actor(actor_type _actor)
		{
			row_range actor_location;
			auto &new_actor = actors.append(_actor, actor_location);
			new_actor.collection_id = collection_id;
			new_actor.actor_id = actor_location.start;
			return get_actor(new_actor.actor_id);
		}

		actor_id_type jcollection::find_actor(object_name& _name)
		{
			auto id = actors.first_index([_name](auto& t) { return t.item.actor_name == _name; });
			return id;
		}

		actor_type jcollection::get_actor(actor_id_type _actor_id)
		{
			if (actors.check(_actor_id)) {
				return actors[_actor_id];
			}
			else 
			{
				actor_type err;
				return err;
			}
		}

		actor_type jcollection::update_actor(actor_type _actor)
		{
			if (actors.check(_actor.actor_id)) {
				actors[_actor.actor_id] = _actor;
				return actors[_actor.actor_id];
			}
			else {
				actor_type err;
				return err;
			}
		}

		relative_ptr_type jcollection::put_actor(actor_type _actor)
		{
			actor_type modified;

			if (_actor.actor_id == null_row) 
			{
				modified = create_actor(_actor);
			}
			else 
			{
				modified = update_actor(_actor);
			}
			return modified.actor_id;
		}

		actor_state jcollection::select_object(const select_object_request& _select, const char* _trace_msg)
		{
			if (_trace_msg) {
				std::cout << "\nstart:" << _trace_msg << std::endl;
			}

			actor_state acr;
			acr.collection_id = collection_id;
			if (!actors.check(_select.actor_id))
			{
				return acr;
			}

			actor_type ac = get_actor(_select.actor_id);
			if (objects.check(_select.object_id)) 
			{
				auto slice = get_object(_select.object_id);
				auto class_id = slice.get_class_id();
				
				selections_collection temp;
				auto model = schema->get_model(ref->model_name);
				auto pmodel = &model;
				auto hierarchy_item = model.selection_hierarchy.first_value([class_id](auto& vr) { return class_id == vr.item.class_id; });
				auto phierarchy_item = &hierarchy_item;
				auto selected_levels = model.selection_hierarchy.where([phierarchy_item](auto& vr) { return vr.item.level_id <= phierarchy_item->level_id; });

				ac.breadcrumb.clear();
				ac.current_view_class_id = -1;

				temp.clear();
				int highest_level = -1;
				for (auto aci : ac.selections)
				{
					auto cls_id = get_class_id(aci.item);
					auto selected_item_level = selected_levels.where([cls_id, phierarchy_item](auto& vri) { return vri.item.class_id == cls_id; });
					if (selected_item_level != std::end(selected_levels))
					{
						if (selected_item_level.get_object().item.level_id > highest_level)
						{
							ac.breadcrumb.push_back(aci.item);
							ac.current_view_class_id = cls_id;
						}
						temp.push_back(aci.item);
					}
				}

				ac.selections = temp;

				relative_ptr_type selection = _select.object_id;
				ac.selections.push_back(selection);
				put_actor(ac);
				acr = get_actor_state(_select.actor_id, _select.object_id, _trace_msg);
				acr.modified_object_level = phierarchy_item->level_id;
				acr.modified_object_id = _select.object_id;
			}
			else {
				acr = get_actor_state(_select.actor_id, _select.object_id, _trace_msg);
			}
			return acr;
		}

		actor_state jcollection::delete_selected(const delete_selected_request& _select, const char* _trace_msg)
		{
			if (_trace_msg) {
				std::cout << "\nstart:" << _trace_msg << std::endl;
			}

			actor_state acr;
			acr.collection_id = collection_id;
			if (!actors.check(_select.actor_id))
			{
				return acr;
			}

			actor_type ac = get_actor(_select.actor_id);

			auto acr = get_actor_state(_select.actor_id, -1, _trace_msg);

			auto modified_time = time(nullptr);

			selections_collection retained_selections;

			for (auto sel : ac.selections)
			{
				if (acr.view_objects.contains(sel.item)) {
					auto state = acr.view_objects[sel.item];
					auto avo = state.get_value();
					if (avo.deletable) {
						objects[sel.item].item().deleted = true;
						objects[sel.item].item().last_modified = modified_time;
						objects[sel.item].item().actor_id = _select.actor_id;
					}
					else {
						retained_selections.push_back(sel.item);
					}
				}
				else {
					retained_selections.push_back(sel.item);
				}
			}

			ac.selections = retained_selections;

			acr = get_actor_state(_select.actor_id, -1, _trace_msg);
			return acr;
		}

		actor_state jcollection::create_object(create_object_request& _create, const char* _trace_msg)
		{
			if (_trace_msg) {
				std::cout << "\nstart:" << _trace_msg << std::endl;
			}

			actor_state acr;
			acr.collection_id = collection_id;
			if (!actors.check(_create.actor_id))
			{
				return acr;
			}
			actor_type ac = get_actor(_create.actor_id);
			relative_ptr_type item_id = _create.item_id;
			relative_ptr_type object_id = null_row;

			jobject new_object;
			if (_create.template_item_id != null_row) {
				auto new_class_id = create_class_from_template(_create.class_id, _create.template_item_id);
				new_object = create_object(item_id, _create.actor_id, _create.class_id, object_id);
			}
			else 
			{
				new_object = create_object(item_id, _create.actor_id, _create.class_id, object_id);
			}

			if (object_id != null_row) 
			{
				for (auto js : ac.selections)
				{
					jobject src_obj = get_object(js.item);
					new_object.update(src_obj);
				}

				if (_create.select_on_create) 
				{
					select_object_request sor;
					sor.actor_id = _create.actor_id;
					sor.collection_id = _create.collection_id;
					sor.extend = false;
					sor.object_id = object_id;
					select_object(sor, _trace_msg);
				}
			}

			acr = get_actor_state(_create.actor_id, object_id, _trace_msg);
			return acr;
		}

		actor_state jcollection::update_object(update_object_request& _update, const char* _trace_msg)
		{
			if (_trace_msg) {
				std::cout << "\nstart:" << _trace_msg << std::endl;
			}

			actor_state acr;
			acr.collection_id = collection_id;
			if (!actors.check(_update.actor_id))
			{
				return acr;
			}
			actor_type ac = get_actor(_update.actor_id);
			relative_ptr_type object_id = _update.object_id;

			if (object_id != null_row && objects.check(object_id))
			{
				acr.modified_object_id = object_id;
				auto slice = get_object(object_id);
				update_object(object_id, slice);
			}

			acr = get_actor_state(_update.actor_id, object_id, _trace_msg);
			return acr;
		}

		jobject jcollection::create_object(relative_ptr_type _item_id, relative_ptr_type _actor_id, relative_ptr_type _class_id, relative_ptr_type& _object_id)
		{
			auto myclass = schema->get_class(_class_id);

			object_name composed_class_field_name;
			dimensions_type d = { 1, 1, 1 };
			schema->get_class_field_name(composed_class_field_name, myclass.item().name, d);
			auto find_class_id = schema->find_class(composed_class_field_name);
			if (find_class_id == null_row)
			{
				put_object_field_request porf;
				porf.name.name = composed_class_field_name;
				porf.name.type_id = jtype::type_object;
				porf.options.class_id = _class_id;
				porf.options.class_name = myclass.item().name;
				porf.options.class_size_bytes = myclass.item().class_size_bytes;
				porf.options.dim = d;
				find_class_id = schema->put_object_field(porf);
			}
			find_class_id = schema->find_class(composed_class_field_name);
			auto find_class = schema->get_field(find_class_id);
			auto bytes_to_allocate = find_class.size_bytes;

			collection_object_type co;
			co.oid.collection_id = collection_id;
			co.class_field_id = find_class_id;
			co.class_id = _class_id;
			co.item_id = _item_id;
			co.otype = jtype::type_object;

			auto new_object = objects.create_item( &co, bytes_to_allocate, nullptr);
			new_object.item().last_modified = std::time(nullptr);
			co.oid.row_id = new_object.row_id();
			_object_id = new_object.row_id();
			char* bytes = new_object.pdetails();
			jarray ja(nullptr, schema, find_class_id, bytes, true);
			auto new_slice = ja.get_slice(0);
			int pkidx = myclass.pitem()->primary_key_idx;

			if (pkidx > -1) {
				auto pkfield = new_slice.get_int64(pkidx);
				pkfield = co.oid.row_id;
			}

			return new_slice;
		}

		jobject jcollection::create_object(relative_ptr_type _item_id, relative_ptr_type _actor_id, relative_ptr_type _class_id, relative_ptr_type& _object_id, std::initializer_list<dynamic_value> var)
		{
			jobject j = create_object(_item_id, _actor_id, _class_id, _object_id);
			j.set(var);
			return j;
		}

		jobject jcollection::get_object(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			if (existing_object.pitem()->otype == jtype::type_object) {
				jarray jax(nullptr, schema, existing_object.item().class_field_id, existing_object.pdetails());
				return jax.get_slice(0);
			}
			else 
			{
				jobject empty;
				return empty;
			}
		}

		relative_ptr_type jcollection::put_user_class(jobject& slice, time_t _version)
		{
			auto class_id = null_row;
			if (slice.get_class_id() != schema->idc_user_class)
			{
				throw std::logic_error("object is not a user class");
			}

			put_class_request pcr;

			pcr.class_name = slice.get_string(schema->idf_user_class_class_name, true);
			pcr.class_name = pcr.class_name + "v" + std::to_string(_version);
			object_id_type& class_object_id = slice.get_object_id(schema->idf_user_class_class_id, true);
			pcr.class_id = class_object_id.row_id;
			pcr.class_description = pcr.class_name;
			pcr.base_class_id = slice.get_int64(schema->idf_base_class_id, true);

			auto fields = slice.get_list(schema->idf_user_field_list, true);
			for (auto field : fields) {

				auto new_field_type_id = field.item.get_int64(schema->idf_field_type, true);
				auto new_field_type = (jtype)new_field_type_id.value();

				switch (new_field_type) {
				case type_int64:
				case type_int32:
				case type_int8:
				{
					put_integer_field_request pifr;
					pifr.name.name = field.item.get_string(schema->idf_field_name, true);
					pifr.name.description = field.item.get_string(schema->idf_field_description, true);
					pifr.name.type_id = new_field_type;
					//pifr.name.format = field.item.get_string(schema->id_dbf_field_description, true);
					auto options = field.item.get_object(schema->idf_int_options, true).get_slice({ 0,0,0 });
					pifr.options.maximum_int = options.get_int64(schema->idf_int_stop);
					pifr.options.minimum_int = options.get_int64(schema->idf_int_start);
					auto field_id = schema->put_integer_field(pifr);
					pcr.member_fields.push_back({ field_id });
				}
				break;
				case type_float64:
				case type_float32:
				{
					put_double_field_request pifr;
					pifr.name.name = field.item.get_string(schema->idf_field_name, true);
					pifr.name.description = field.item.get_string(schema->idf_field_description, true);
					pifr.name.type_id = new_field_type;
					//pifr.name.format = field.item.get_string(schema->id_dbf_field_description, true);
					auto options = field.item.get_object(schema->idf_double_options, true).get_slice({ 0,0,0 });
					pifr.options.maximum_double = options.get_double(schema->idf_double_stop);
					pifr.options.minimum_double = options.get_double(schema->idf_double_start);
					auto field_id = schema->put_double_field(pifr);
					pcr.member_fields.push_back({ field_id });
				}
				break;
				case type_datetime:
				{
					put_time_field_request pifr;
					pifr.name.name = field.item.get_string(schema->idf_field_name, true);
					pifr.name.description = field.item.get_string(schema->idf_field_description, true);
					pifr.name.type_id = new_field_type;
					//pifr.name.format = field.item.get_string(schema->id_dbf_field_description, true);
					auto options = field.item.get_object(schema->idf_date_options, true).get_slice({ 0,0,0 });
					pifr.options.maximum_time_t = options.get_time(schema->idf_date_stop);
					pifr.options.minimum_time_t = options.get_time(schema->idf_date_start);
					auto field_id = schema->put_time_field(pifr);
					pcr.member_fields.push_back({ field_id });
				}
				break;
				case type_string:
				{
					put_string_field_request pifr;
					pifr.name.name = field.item.get_string(schema->idf_field_name, true);
					pifr.name.description = field.item.get_string(schema->idf_field_description, true);
					pifr.name.type_id = new_field_type;
					//pifr.name.format = field.item.get_string(schema->id_dbf_field_description, true);
					auto options = field.item.get_object(schema->idf_string_options, true).get_slice({ 0,0,0 });
					pifr.options.length = options.get_int32(schema->idf_string_length);
					pifr.options.validation_message = options.get_string(schema->idf_string_validation_message);
					pifr.options.validation_pattern = options.get_string(schema->idf_string_validation_pattern);
					pifr.options.full_text_editor = options.get_int8(schema->idf_string_full_text_editor);
					pifr.options.rich_text_editor = options.get_int8(schema->idf_string_rich_text_editor);
					auto field_id = schema->put_string_field(pifr);
					pcr.member_fields.push_back({ field_id });
				}
				break;
				case type_list:
				case type_object:
				{
					put_object_field_request pifr;
					pifr.name.name = field.item.get_string(schema->idf_field_name, true);
					pifr.name.description = field.item.get_string(schema->idf_field_description, true);
					//pifr.name.format = field.item.get_string(schema->id_dbf_field_description, true);
					auto options = field.item.get_object(schema->idc_object_options, true).get_slice({ 0,0,0 });
					pifr.options.class_id = options.get_int64(schema->idf_object_class_id);
					pifr.options.dim.x = options.get_int32(schema->idf_object_x);
					pifr.options.dim.y = options.get_int32(schema->idf_object_y);
					pifr.options.dim.z = options.get_int32(schema->idf_object_z);
					auto field_id = schema->put_object_field(pifr);
					pcr.member_fields.push_back({ field_id });
				}
				break;
				}
			}
			class_id = schema->put_class(pcr);
			return class_id;
		}

		relative_ptr_type jcollection::create_class_from_template(relative_ptr_type _target_class_id, relative_ptr_type _source_template_object)
		{
			jclass cls = schema->get_class(_target_class_id);
			auto object_header = objects.get_item(_source_template_object);
			jobject obj = get_object(_source_template_object);
	
			put_class_request pcr;
			pcr.class_name = "template_" + std::to_string(_source_template_object) + "_v" + std::to_string(object_header.item().last_modified);
			pcr.class_description = "Class expanded from:" + _source_template_object;
			pcr.class_id = null_row;
			pcr.base_class_id = _target_class_id;
			int idx = cls.item().primary_key_idx;

			if (idx > -1)
			{
				pcr.field_id_primary_key = cls.detail(idx).field_id;
			}

			for (int i = 0; i < cls.size(); i++)
			{
				auto& target_class_field = cls.detail(i);
				auto& target_field = schema->get_field(target_class_field.field_id);
				auto src_field_idx = obj.get_field_index_by_id(target_field.field_id);

				if (src_field_idx != null_row)
				{
					if (target_field.is_container())
					{
						if (target_field.is_class(schema->idc_user_class))
						{
							// the object here is a user class in the source object,
							// so, we fetch it, create the class for it, then, go ahead and make that object 
							// an object (list or array) member of the final object, with the 
							// target class dimensions.
							auto user_class_obj = obj.get_object(src_field_idx, false).get_slice(0);
							relative_ptr_type field_class_id = put_user_class(user_class_obj, object_header.item().last_modified);
							auto field_class = schema->get_class(field_class_id);
							pcr.member_fields.push_back({ field_class_id, target_field.object_properties.dim });
						} 
						else 
						{
							pcr.member_fields.push_back({ target_field.object_properties.class_id, target_field.object_properties.dim });
						}
					}
					else 
					{
						pcr.member_fields.push_back({ target_field.field_id });
					}
				}
				else
				{
					pcr.member_fields.push_back({ target_field.field_id });
				}
			}
			
			auto new_class_id = schema->put_class(pcr);
			return new_class_id;
		}

		relative_ptr_type jcollection::get_class_id(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			return existing_object.pitem()->class_id;
		}

		relative_ptr_type jcollection::get_base_id(relative_ptr_type _object_id)
		{
			relative_ptr_type class_id = get_class_id(_object_id);
			auto class_def = schema->get_class(class_id);
			return class_def.item().base_class_id;
		}

		bool jcollection::matches_class_id(jobject& obj, relative_ptr_type _class_id)
		{
			relative_ptr_type class_id = obj.get_class_id();
			if (class_id == _class_id) {
				return true;
			}
			relative_ptr_type base_class_id = obj.get_base_class_id();
			if (base_class_id == _class_id) {
				return true;
			}
			while (base_class_id != null_row) {
				auto new_class = schema->get_class(base_class_id);
				if (new_class.item().class_id == _class_id || new_class.item().base_class_id == _class_id) {
					return true;
				}
				base_class_id = new_class.item().base_class_id;
			}
			return false;
		}

		bool jcollection::matches_class_id(relative_ptr_type _object_id, relative_ptr_type _class_id)
		{
			relative_ptr_type class_id = get_class_id(_object_id);
			if (class_id == _class_id) {
				return true;
			}
			relative_ptr_type base_class_id = get_base_id(_object_id);
			if (base_class_id == _class_id) {
				return true;
			}
			while (base_class_id != null_row) {
				auto new_class = schema->get_class(base_class_id);
				if (new_class.item().class_id == _class_id || new_class.item().base_class_id == _class_id) {
					return true;
				}
				base_class_id = new_class.item().base_class_id;
			}
			return false;
		}

		jobject jcollection::get_at(relative_ptr_type _object_id)
		{
			return get_object(_object_id);
		}

		collection_object_type &jcollection::get_object_reference(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			return existing_object.item();
		}

		jobject jcollection::update_object(relative_ptr_type _object_id, jobject _slice)
		{
			auto existing_object = objects.get_item(_object_id);
			if (existing_object.pitem()->otype == jtype::type_object) {
				existing_object.item().last_modified = std::time(nullptr);
				jarray jax(nullptr, schema, existing_object.item().class_field_id, existing_object.pdetails());
				auto slice_to_update = jax.get_slice(0);
				slice_to_update.update(_slice);
				return slice_to_update;
			}
			else
			{
				jobject empty;
				return empty;
			}
		}

		void dynamic_value::copy(const dynamic_value& _src)
		{
			field_id = _src.field_id;
			this_type = _src.this_type;
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				int_value = _src.int_value;
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				double_value = _src.double_value;
				break;
			case jtype::type_collection_id:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_color:
				color_value = _src.color_value;
				break;
			case jtype::type_datetime:
				time_value = _src.time_value;
				break;
			case jtype::type_file:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_http:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_image:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_layout_rect:
				layout_rect_value = _src.layout_rect_value;
				break;
			case jtype::type_list:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_midi:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_null:
				break;
			case jtype::type_object:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_object_id:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_point:
				point_value = _src.point_value;
				break;
			case jtype::type_query:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_rectangle:
				rectangle_value = _src.rectangle_value;
				break;
			case jtype::type_sql:
				break;
			case jtype::type_string:
				string_value = _src.string_value;
				break;
			case jtype::type_wave:
				break;
			}
		}

		dynamic_value::dynamic_value(const dynamic_value& _src)
		{
			copy(_src);
		}

		dynamic_value dynamic_value::operator =(const dynamic_value& _src)
		{
			copy(_src);
			return *this;
		}

		dynamic_value::operator std::string()
		{
			std::string z;
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				z = std::format("{}", int_value);
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				z = std::format("{}", double_value);
				break;
			case jtype::type_collection_id:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_file:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_http:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_list:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_object:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_object_id:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_query:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_sql:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_string:
				z = string_value;
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to string");
				break;
			}
			return z;
		}

		dynamic_value::operator color ()
		{
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_collection_id:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_color:
				return color_value;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_file:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_http:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_list:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_object:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_object_id:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_query:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_sql:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to color");
				break;
			}
		}

		dynamic_value::operator point()
		{
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_collection_id:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_file:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_http:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_list:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_object:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_object_id:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_point:
				return point_value;
			case jtype::type_query:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_sql:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to point");
				break;
			}
		}

		dynamic_value::operator rectangle()
		{
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_collection_id:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_file:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_http:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_list:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_object:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_object_id:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_query:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_rectangle:
				return rectangle_value;
			case jtype::type_sql:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to rectangle");
				break;
			}
		}

		dynamic_value::operator layout_rect()
		{
			switch (this_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_collection_id:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_file:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_http:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_layout_rect:
				return layout_rect_value;
			case jtype::type_list:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_object:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_object_id:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_query:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_sql:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to layout_rect");
				break;
			}
		}

		dynamic_value jobject::get(relative_ptr_type _field_id)
		{

			dynamic_value sma;

			int _field_idx = get_field_index_by_id(_field_id);
			jfield& field = get_field(_field_idx);
			switch (field.type_id)
			{
			case jtype::type_int8:
				sma = dynamic_value( _field_idx, (int64_t)get_int8(_field_idx) );
				break;
			case jtype::type_int16:
				sma = dynamic_value(_field_idx, (int64_t)get_int16(_field_idx) );
				break;
			case jtype::type_int32:
				sma = dynamic_value(_field_idx, (int64_t)get_int32(_field_idx) );
				break;
			case jtype::type_int64:
				sma = dynamic_value(_field_idx, (int64_t)get_int64(_field_idx) );
				break;
			case jtype::type_float32:
				sma = dynamic_value(_field_idx, (double)get_float(_field_idx) );
				break;
			case jtype::type_float64:
				sma = dynamic_value(_field_idx, (double)get_double(_field_idx) );
				break;
			case jtype::type_collection_id:
				break;
			case jtype::type_color:
				sma = dynamic_value(_field_idx, (color)get_color(_field_idx));
				break;
			case jtype::type_datetime:
				sma = dynamic_value(_field_idx, (time_t)get_time(_field_idx));
				break;
			case jtype::type_file:
				break;
			case jtype::type_http:
				break;
			case jtype::type_image:
				break;
			case jtype::type_layout_rect:
				sma = dynamic_value(_field_idx, get_layout_rect(_field_idx));
				break;
			case jtype::type_list:
				break;
			case jtype::type_midi:
				break;
			case jtype::type_null:
				break;
			case jtype::type_object:
				break;
			case jtype::type_object_id:
				break;
			case jtype::type_point:
				sma = dynamic_value(_field_idx, get_point(_field_idx));
				break;
			case jtype::type_query:
				break;
			case jtype::type_rectangle:
				sma = dynamic_value(_field_idx, get_rectangle(_field_idx));
				break;
			case jtype::type_sql:
				break;
			case jtype::type_string:
				sma = dynamic_value(_field_idx, get_string(_field_idx).c_str());
				break;
			case jtype::type_wave:
				break;
			}

			return sma;
		}

		dynamic_value jobject::operator[](relative_ptr_type field_id)
		{
			return get(field_id);
		}

		jobject::jobject() : schema(nullptr), class_id(null_row), bytes(nullptr), box(nullptr), location(null_row)
		{
			;
		}

		jobject::jobject(jobject *_parent, jschema* _schema, relative_ptr_type _class_id, char* _bytes, dimensions_type _dim) 
			: parent(_parent), 
			schema(_schema), 
			class_id(_class_id), 
			bytes(_bytes), 
			dim(_dim), 
			box(nullptr), 
			location(null_row)
		{
			the_class = schema->get_class(_class_id);
		}

		jobject::jobject(jobject* _parent, jschema* _schema, relative_ptr_type _class_id, serialized_box_container *_box, relative_ptr_type _location, dimensions_type _dim) : 
			parent(_parent), 
			schema(_schema), 
			class_id(_class_id), 
			bytes(nullptr), 
			dim(_dim), 
			box(_box), 
			location(_location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject::jobject(const jobject& _src) :
			parent(_src.parent),
			schema(_src.schema),
			class_id(_src.class_id),
			bytes(_src.bytes),
			dim(_src.dim),
			box(_src.box),
			location(_src.location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject jobject::operator =(const jobject& _src)
		{
			parent = _src.parent;
			schema = _src.schema;
			class_id = _src.class_id;
			bytes = _src.bytes;
			dim = _src.dim;
			box = _src.box;
			location = _src.location;
			the_class = schema->get_class(class_id);
			return *this;
		}

		jobject::jobject(jobject&& _src) : 
			parent(_src.parent),
			schema(_src.schema),
			class_id(_src.class_id),
			bytes(_src.bytes),
			dim(_src.dim),
			box(_src.box),
			location(_src.location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject& jobject::operator =(jobject&& _src)
		{
			parent = _src.parent;
			schema = _src.schema;
			class_id = _src.class_id;
			bytes = _src.bytes;
			dim = _src.dim;
			box = _src.box;
			location = _src.location;
			the_class = schema->get_class(class_id);
			return *this;
		}

		jobject& jobject::get_parent_slice()
		{
			return *parent;
		}

		jclass jobject::get_class()
		{
			return the_class;
		}

		jschema *jobject::get_schema()
		{
			return schema;
		}

		size_t jobject::get_offset(int field_idx, jtype _type)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || get_bytes() == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.detail(field_idx);
#if _DEBUG
			if (_type != jtype::type_null) {
				jfield& f = schema->get_field(jcf.field_id);
				if (f.type_id != _type) {
					istring<100> error = f.name + " accessed incorrectly.";
					throw std::logic_error(error.c_str());
				}
			}
#endif
			return jcf.offset;
		}

		/*
					auto offset1 = get_offset(_type, _src_idx);
			auto offset2 = _src_slice.get_offset(_type, _dst_idx);
			char* c1 = bytes + offset1;
			char* c2 = bytes + offset2;
*/
		std::partial_ordering jobject::compare_express(jtype _type, char *c1, char *c2)
		{
			switch (_type)
			{
			case jtype::type_int8:
				{
					int8_box b1(c1);
					int8_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int16:
				{
					int16_box b1(c1);
					int16_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int32:
				{
					int32_box b1(c1);
					int32_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int64:
				{
					int64_box b1(c1);
					int64_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_float32:
				{
					float_box b1(c1);
					float_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_float64:
				{
					double_box b1(c1);
					double_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_datetime:
				{
					time_box b1(c1);
					time_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_string:
				{
					auto b1 = string_box::get(c1);
					auto b2 = string_box::get(c2);
					return b1 <=> b2;
				}
				break;
			default:
				return c1 <=> c2;
			}
		}

		dimensions_type jobject::get_dim() 
		{
			return dim; 
		}

		void jobject::construct()
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				jfield jf = schema->get_field(jcf.field_id);
				int offset = jcf.offset;
				char* c = get_bytes() + offset;
				switch (jf.type_id) 
				{
				case jtype::type_null:
					break;
				case jtype::type_int8:
					{
						int8_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int16:
					{
						int16_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int32:
					{
						int32_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int64:
					{
						int64_box b(c);
						b = 0;
					}
					break;
				case jtype::type_float32:
					{
						float_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_float64:
					{
						double_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_datetime:
					{
						time_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_object:
					{
						jarray ja(this, schema, jcf.field_id, c);

						for (auto jai : ja)
						{
							jai.item.construct();
						}
					}
					break;
				case jtype::type_list:
					{
						jlist jax(this, schema, jcf.field_id, c, true);
					}
					break;
				case jtype::type_object_id:
					break;
				case jtype::type_string:
					{
						string_box::create(c, jf.string_properties.length);
					}
					break;
				case jtype::type_point:
					{
						point_box b(c);
						b = point { };
					}
					break;
				case jtype::type_rectangle:
					{
						rectangle_box b(c);
						b = rectangle {};
					}
					break;
				case jtype::type_layout_rect:
					{
						layout_rect_box b(c);
						b = layout_rect{};
					}
					break;
				case jtype::type_color:
					{
						color_box b(c);
						b = color{};
					}
					break;
				case jtype::type_image:
					{
						image_box b(c);
						b = image_instance {};
					}
					break;
				case jtype::type_wave:
					{
						wave_box b(c);
						b = wave_instance {};
					}
					break;
				case jtype::type_midi:
					{
						midi_box b(c);
						b = midi_instance {};
					}
					break;
				case jtype::type_query:
					{
						query_box b(c, schema, &the_class, this, i);
						b = query_instance{};
					}
					break;
				case jtype::type_sql:
					{
						sql_remote_box b(c, schema, &the_class, this, i);
						b = sql_remote_instance{};
					}
					break;
				case jtype::type_file:
					{
						file_remote_box b(c, schema, &the_class, this, i);;
						b = file_remote_instance{};
					}
					break;
				case jtype::type_http:
					{
						http_remote_box b(c, schema, &the_class, this, i);;
						b = http_remote_instance{};
					}
					break;
				}
			}
		}

		jfield& jobject::get_field(int field_idx)
		{
			jclass_field& jcf = the_class.detail(field_idx);
			jfield &jf = schema->get_field(jcf.field_id);
			return jf;
		}

		int jobject::get_field_index_by_id(relative_ptr_type field_id)
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				if (jcf.field_id == field_id)
				{
					return i;
				}
			}
			return -1;
		}

		int jobject::get_field_index_by_name(const object_name& name)
		{
			relative_ptr_type field_id = schema->find_class(name);
			return get_field_index_by_id(field_id);
		}

		jclass_field& jobject::get_class_field(int field_idx)
		{
			jclass_field& jcf = the_class.detail(field_idx);
			return jcf;
		}

		bool jobject::has_field(const object_name& name)
		{
			return schema && get_field_index_by_name( name ) > -1;
		}

		bool jobject::has_field(relative_ptr_type field_id)
		{
			return get_field_index_by_id(field_id) > -1;
		}

		bool jobject::is_class(relative_ptr_type class_id)
		{
			return the_class.pitem()->class_id == class_id;
		}

		jfield& jobject::get_field_by_id(relative_ptr_type field_id)
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				if (jcf.field_id == field_id)
				{
					jfield& jf = schema->get_field(jcf.field_id);
					return jf;
				}
			}
			return schema->get_empty();
		}

		int8_box jobject::get_int8(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<int8_box>(field_idx, jtype::type_int8);
		}

		int16_box jobject::get_int16(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<int16_box>(field_idx, jtype::type_int16);
		}

		int32_box jobject::get_int32(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return jobject::get_boxed<int32_box>(field_idx, jtype::type_int32);
		}

		int64_box jobject::get_int64(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return jobject::get_boxed<int64_box>(field_idx, jtype::type_int64);
		}

		float_box jobject::get_float(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return jobject::get_boxed<float_box>(field_idx, jtype::type_float32);
		}

		double_box jobject::get_double(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return jobject::get_boxed<double_box>(field_idx, jtype::type_float64);
		}

		time_box jobject::get_time(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<time_box>(field_idx, jtype::type_datetime);
		}

		point_box jobject::get_point(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<point_box>(field_idx, jtype::type_point);
		}

		rectangle_box jobject::get_rectangle(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<rectangle_box>(field_idx, jtype::type_rectangle);
		}

		layout_rect_box jobject::get_layout_rect(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<layout_rect_box>(field_idx, jtype::type_layout_rect);
		}

		image_box jobject::get_image(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<image_box>(field_idx, jtype::type_image);
		}

		wave_box jobject::get_wave(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<wave_box>(field_idx, jtype::type_wave);
		}

		midi_box jobject::get_midi(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<midi_box>(field_idx, jtype::type_midi);
		}

		color_box jobject::get_color(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed<color_box>(field_idx, jtype::type_color);
		}

		query_box jobject::get_query(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed_ex<query_box>(field_idx, jtype::type_query);
		}

		sql_remote_box jobject::get_sql_remote(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed_ex<sql_remote_box>(field_idx, jtype::type_sql);
		}

		http_remote_box jobject::get_http_remote(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed_ex<http_remote_box>(field_idx, jtype::type_http);
		}

		file_remote_box jobject::get_file_remote(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return get_boxed_ex<file_remote_box>(field_idx, jtype::type_file);
		}

		string_box jobject::get_string(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			size_t offset = get_offset(field_idx, jtype::type_string);
			char *b = get_bytes() + offset;
			auto temp = string_box::get(b);
			return temp;
		}

		jarray jobject::get_object(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);

#if _DEBUG
			if (schema == nullptr || class_id == null_row) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.detail(field_idx);
#if _DEBUG
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != jtype::type_object) {
				throw std::invalid_argument("Invalid field type " + std::to_string(jf.type_id) + " for field idx " + std::to_string(field_idx));
			}
#endif
			char *b = get_bytes() + jcf.offset;
			jarray jerry(this, schema, jcf.field_id, b);
			return jerry;
		}

		jobject jobject::get_slice(int field_idx, dimensions_type _dim, bool _use_id)
		{
			jarray arr = get_object(field_idx, _use_id);
			return arr.get_slice(_dim);
		}

		jlist jobject::get_list(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);

#if _DEBUG
			if (schema == nullptr || class_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.detail(field_idx);
#if _DEBUG
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != jtype::type_list) {
				throw std::invalid_argument("Invalid field type " + std::to_string(jf.type_id) + " for field idx " + std::to_string(field_idx));
			}
#endif
			char* b = get_bytes() + jcf.offset;
			jlist jerry(this, schema, jcf.field_id, b);
			return jerry;
		}

		collection_id_box jobject::get_collection_id(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);

			return jobject::get_boxed<collection_id_box>(field_idx, jtype::type_collection_id);
		}

		object_id_box jobject::get_object_id(int field_idx, bool _use_id)
		{
			if (_use_id) field_idx = get_field_index_by_id(field_idx);
			return jobject::get_boxed<object_id_box>(field_idx, jtype::type_object_id);
		}

		int8_box jobject::get_int8(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_int8(index);
		}

		int16_box jobject::get_int16(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_int16(index);
		}

		int32_box jobject::get_int32(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_int32(index);
		}

		int64_box jobject::get_int64(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_int64(index);
		}

		float_box jobject::get_float(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_float(index);
		}

		double_box jobject::get_double(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_double(index);
		}

		time_box jobject::get_time(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_time(index);
		}

		string_box jobject::get_string(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_string(index);
		}

		jarray jobject::get_object(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_object(index);
		}

		jlist jobject::get_list(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_list(index);
		}

		collection_id_box jobject::get_collection_id(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_collection_id(index);
		}

		object_id_box jobject::get_object_id(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_object_id(index);
		}

		point_box jobject::get_point(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_point(index);
		}

		rectangle_box jobject::get_rectangle(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_rectangle(index);
		}

		layout_rect_box jobject::get_layout_rect(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_layout_rect(index);
		}

		image_box jobject::get_image(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_image(index);
		}

		wave_box jobject::get_wave(object_name field_name) {
			int  index = get_field_index_by_name(field_name);
			return get_wave(index);
		}

		midi_box jobject::get_midi(object_name field_name) {
			int  index = get_field_index_by_name(field_name);
			return get_midi(index);
		}

		color_box jobject::get_color(object_name field_name) {
			int  index = get_field_index_by_name(field_name);
			return get_color(index);
		}

		query_box jobject::get_query(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_query(index);
		}

		sql_remote_box jobject::get_sql_remote(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_sql_remote(index);
		}

		http_remote_box jobject::get_http_remote(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_http_remote(index);
		}

		file_remote_box jobject::get_file_remote(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_file_remote(index);
		}

		void jobject::set_value(const dynamic_value& _member_assignment)
		{
			int index = get_field_index_by_id(_member_assignment.field_id);
			auto fld = get_field(index);

			switch (_member_assignment.this_type)
			{
			case jtype::type_float64:
			case jtype::type_float32:
				switch (fld.type_id)
				{
					case jtype::type_float64:
					{
						auto fb = get_double(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_float32:
					{
						auto fb = get_float(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int64:
					{
						auto fb = get_int64(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int32:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int16:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int8:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_string:
					{
						auto fb = get_string(index);
						fb = _member_assignment.double_value;
					}
					break;
				}
				break;
			case jtype::type_int64:
			case jtype::type_int32:
			case jtype::type_int16:
			case jtype::type_int8:
				switch (fld.type_id)
				{
					case jtype::type_float64:
					{
						auto fb = get_double(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_float32:
					{
						auto fb = get_float(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int64:
					{
						auto fb = get_int64(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int32:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int16:
					{
						auto fb = get_int16(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int8:
					{
						auto fb = get_int8(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_string:
					{
						auto fb = get_string(index);
						fb = _member_assignment.int_value;
						break;
					}
				}
				break;
			case jtype::type_datetime:
				switch (fld.type_id)
				{
					case jtype::type_datetime:
					{
						auto fb = get_time(index);
						fb = _member_assignment.time_value;
					}
				}
				break;
			case jtype::type_point:
				switch (fld.type_id)
				{
					case jtype::type_point:
					{
						auto fb = get_point(index);
						fb = _member_assignment.point_value;
					}
				}
				break;
			case jtype::type_rectangle:
				switch (fld.type_id)
				{
					case jtype::type_rectangle:
					{
						auto fb = get_rectangle(index);
						fb = _member_assignment.rectangle_value;
					}
				}
				break;
			case jtype::type_layout_rect:
				switch (fld.type_id)
				{
					case jtype::type_layout_rect:
					{
						auto fb = get_layout_rect(index);
						fb = _member_assignment.layout_rect_value;
					}
				}
				break;
			case jtype::type_string:
				switch (fld.type_id)
				{
					case jtype::type_string:
						{
							auto fb = get_string(index);
							fb = _member_assignment.string_value.c_str();
						}
						break;
					case jtype::type_float64:
						{
							auto fb = get_double(index);
							fb = _member_assignment.string_value.to_double();
						}
						break;
					case jtype::type_float32:
						{
							auto fb = get_float(index);
							fb = _member_assignment.string_value.to_double();
						}
						break;
					case jtype::type_color:
						{
							auto fb = get_color(index);
							string_extractor ex(_member_assignment.string_value.c_str(), _member_assignment.string_value.size(), 8192, nullptr);
							if (_member_assignment.string_value.size() > 7) 
							{
								auto r = ex.get_color_alpha();
								if (r.success) {
									fb->red = r.red;
									fb->green = r.green;
									fb->blue = r.blue;
									fb->alpha = r.alpha;
								}
								else 
								{
									fb->red = 0;
									fb->green = 0;
									fb->blue = 0;
									fb->alpha = 1.0;
								}
							}
							else 
							{
								auto r = ex.get_color();
								if (r.success) {
									fb->red = r.red;
									fb->green = r.green;
									fb->blue = r.blue;
									fb->alpha = r.alpha;
								}
								else
								{
									fb->red = 0;
									fb->green = 0;
									fb->blue = 0;
									fb->alpha = 1.0;
								}
							}
						}
						break;
					case jtype::type_int64:
						{
							auto fb = get_int64(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int32:
						{
							auto fb = get_int32(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int16:
						{
							auto fb = get_int16(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int8:
						{
							auto fb = get_int8(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
				}
			}
		}

		int jobject::size()
		{
			auto the_class = schema->get_class(class_id);
			return the_class.size();
		}

		void jobject::update(jobject& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				std::copy(_src_slice.get_bytes(), _src_slice.get_bytes() + size(), get_bytes());
			}
			else 
			{
				relative_ptr_type fis, fid, ssf;

				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto fld_source = _src_slice.get_field(fis);
					auto fld_dest_idx = get_field_index_by_id(fld_source.field_id);
					if (fld_dest_idx != null_row)
					{
						auto& sf = _src_slice.get_class_field(fis);
						auto& df = get_class_field(fld_dest_idx);
						std::copy(_src_slice.get_bytes() + sf.offset, _src_slice.get_bytes() + sf.offset + fld_source.size_bytes, get_bytes() + df.offset);
					}
				}
			}
		}
		
		std::partial_ordering  jobject::compare(int _dst_idx, jobject& _src_slice, int _src_idx)
		{
			auto field_type = get_field(_dst_idx).type_id;
			auto offset1 = get_offset(_dst_idx);
			auto offset2 = _src_slice.get_offset(_src_idx);
			char* c1 = get_bytes() + offset1;
			char* c2 = _src_slice.get_bytes() + offset2;
			return compare_express(field_type, c1, c2);
		}

		std::partial_ordering jobject::compare(jobject& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				relative_ptr_type fis, fid, ssf;
				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto &fld_source = _src_slice.get_field(fis);
					auto offset1 = get_offset(fis);
					auto offset2 = _src_slice.get_offset(fis);
					char* c1 = get_bytes() + offset1;
					char* c2 = _src_slice.get_bytes() + offset2;
					auto x = compare_express(fld_source.type_id, c1, c2);
					if (x != std::strong_ordering::equal) {
						return x;
					}
				}
				return std::strong_ordering::equal;
			}
			else 
			{
				relative_ptr_type fis, fid, ssf;
				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto fld_idx_source = fis;
					auto& fld_source = _src_slice.get_field(fld_idx_source);
					auto fld_idx_dest = get_field_index_by_id(fld_source.field_id);

					if (fld_idx_dest == null_row) 
					{
						throw std::invalid_argument( "Invalid field index");
					}

					auto& fld_dest = get_field(fld_idx_dest);

					auto offset1 = get_offset(fld_idx_dest, fld_source.type_id);
					auto offset2 = _src_slice.get_offset(fld_idx_source, fld_dest.type_id);
					char* c1 = get_bytes() + offset1;
					char* c2 = _src_slice.get_bytes() + offset2;
					auto x = compare_express(fld_source.type_id, c1, c2);
					if (x != std::strong_ordering::equal) {
						return x;
					}
				}
				return std::strong_ordering::equal;
			}
		}

		std::partial_ordering jobject::compare(jobject& _src_slice, relative_ptr_type* field_ids)
		{
			relative_ptr_type fid;
			while (*field_ids != null_row)
			{
				auto fld_idx_source = _src_slice.get_field_index_by_id(*field_ids);
				auto fld_idx_dest = get_field_index_by_id(*field_ids);

				if (fld_idx_source == null_row) 
				{
					return std::partial_ordering::less;
				}
				else if (fld_idx_dest == null_row)
				{
					return std::partial_ordering::greater;
				}

				auto& fld_source = _src_slice.get_field(fld_idx_source);
				auto& fld_dest = get_field(fld_idx_dest);

				auto offset1 = get_offset(fld_idx_dest, fld_source.type_id);
				auto offset2 = _src_slice.get_offset(fld_idx_source, fld_dest.type_id);
				char* c1 = get_bytes() + offset1;
				char* c2 = _src_slice.get_bytes() + offset2;
				auto x = compare_express(fld_source.type_id, c1, c2);
				if (x != std::strong_ordering::equal) {
					return x;
				}

				field_ids++;
			}
			return std::strong_ordering::equal;
		}

		jobject jobject::convert(serialized_box_container* _box, relative_ptr_type _class_id)
		{
			if (_class_id == class_id)
				return *this;

			auto myclass = schema->get_class(_class_id);
			auto bytes_to_allocate = myclass.item().class_size_bytes;
			relative_ptr_type location = _box->reserve(bytes_to_allocate);

			dimensions_type d = { 0,0,0 };

			jobject ja(nullptr, schema, _class_id, box, location, d);
			ja.construct();
			ja.update(*this);

			return ja;
		}

		jarray::jarray() : schema(nullptr), class_field_id(null_row), bytes(nullptr)
		{
			;
		}

		jarray::jarray(jobject *_parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init) : item(_parent), schema(_schema), class_field_id(_class_field_id), bytes(_bytes)
		{
			if (_init) {
				for (auto jai : *this)
				{
					jai.item.construct();
				}
			}
		}

		jarray::jarray(dynamic_box& _dest, jarray& _src)
		{
			schema = _src.schema;
			class_field_id = _src.class_field_id;
			auto fld = schema->get_field(class_field_id);
			_dest.init(fld.size_bytes);
			bytes = _dest.allocate<char>(fld.size_bytes);
			std::copy(_src.bytes, _src.bytes + fld.size_bytes, bytes);
			item = _src.item;
		}

		dimensions_type jarray::dimensions()
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim;
		}

		jobject jarray::get_slice(int x, int y, int z)
		{
			dimensions_type dims;
			dims.x = x;
			dims.y = y;
			dims.z = z;
			return get_slice(dims);
		}

		jobject jarray::get_slice(dimensions_type pos)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((pos.x >= dim.x) ||
				(pos.y >= dim.y) ||
				(pos.z >= dim.z)) {
					throw std::invalid_argument("dimension out of range on jarray");
			}
			char* b = &bytes[ ((pos.z * dim.y * dim.x) + (pos.y * dim.x) + pos.x ) * field.object_properties.class_size_bytes ];
			jobject slice(item, schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		uint64_t jarray::get_size_bytes()
		{
			jfield& field = schema->get_field(class_field_id);
			return field.size_bytes;
		}

		jobject jarray::get_at(relative_ptr_type _index)
		{
			if (_index < 0) throw std::invalid_argument("index out of range on jarray");
			jfield& field = schema->get_field(class_field_id);
			dimensions_type index_dim;
			relative_ptr_type index_calc = _index;
			auto array_bounds = dimensions();
			index_dim.x = index_calc % array_bounds.x;
			index_calc /= array_bounds.x;
			index_dim.y = index_calc % array_bounds.y;
			index_calc /= array_bounds.y;
			index_dim.z = index_calc % array_bounds.z;
			char* b = &bytes[_index * field.object_properties.class_size_bytes];
			jobject slice(item, schema, field.object_properties.class_id, b, index_dim);
			return slice;
		}

		corona_size_t jarray::size()
		{
			corona_size_t the_size;
			auto array_bounds = dimensions();
			the_size = array_bounds.x * array_bounds.y * array_bounds.z;
			return the_size;
		}

		jarray_container::jarray_container()
		{
			;
		}

		jarray_container::jarray_container(collection_id_type& _collection, jarray& _objects)
		{
			set(_collection, _objects);
		}

		void jarray_container::set(collection_id_type& _collection, jarray& _objects)
		{
			collection = _collection;
			objects = jarray(data, _objects);
		}

		jarray& jarray_container::get()
		{
			return objects;
		}

		//

		jlist::jlist() : schema(nullptr), class_field_id(null_row)
		{
			;
		}

		jlist::jlist(jobject *_parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init) 
			: item(_parent), schema(_schema), class_field_id(_class_field_id)
		{
			auto& field_def = schema->get_field(_class_field_id);
			jclass model_class_def = schema->get_class(field_def.object_properties.class_id);
			auto box_size = field_def.size_bytes;

			data.instance = nullptr;
			model_box = inline_box(_bytes, box_size);

			if (_init)
			{
				data.instance = model_box.allocate<jlist_instance>(1);
				array_box<relative_ptr_type>::create(&model_box, field_def.object_properties.dim.x, data.instance->selection_offset);
				data.instance->slice_offset = model_box.reserve(0);
				data.instance->allocated = 0;
			}
			else
			{
				data.instance = model_box.unpack<jlist_instance>(0);
			}

			data.list_bytes = model_box.unpack<char>(data.instance->slice_offset);
			data.selections = array_box<relative_ptr_type>::get(&model_box, data.instance->selection_offset);
		}

		jlist::jlist(serialized_box_container& _dest, jlist& _src)
			: schema(_src.schema), class_field_id(_src.class_field_id)
		{
			auto& field_def = schema->get_field(_src.class_field_id);
			auto box_size = field_def.size_bytes;

			char* t = _dest.allocate<char>(box_size);
			model_box = inline_box(t, box_size);
			std::copy((char*)_src.data.instance, (char*)_src.data.instance + box_size, (char*)data.instance);
			data.instance = model_box.unpack<jlist_instance>(0);
			data.list_bytes = model_box.unpack<char>(data.instance->slice_offset);
			data.selections = array_box<relative_ptr_type>::get(&model_box, data.instance->selection_offset);
			item = _src.item;
		}

		corona_size_t jlist::capacity()
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim.x;
		}

		corona_size_t jlist::size()
		{
			return data.instance->allocated;
		}

		jobject jlist::get_at(corona_size_t idx)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= data.instance->allocated) || (idx < 0)) {
				return jobject(item, schema, field.object_properties.class_id, nullptr, dim);
			}
			dimensions_type pos = { idx, 0, 0 };
			char* b = &data.list_bytes[idx * field.object_properties.class_size_bytes];
			jobject slice(item, schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		bool jlist::erase(corona_size_t idx)
		{
			int oidx = idx;
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= dim.x) || (idx < 0)) 
			{
				return false;
			}
			else if (data.instance->allocated <= 0)
			{
				data.instance->allocated = 0;
				return false;
			}
			else if (idx >= data.instance->allocated)
			{
				data.instance->allocated--;
			}
			else 
			{
				auto class_size = field.object_properties.class_size_bytes;
				char* b1 = &data.list_bytes[idx * class_size];
				char* b2 = &data.list_bytes[(idx + 1) * class_size];
				int32_t length_objects = data.instance->allocated - idx;
				int32_t length_bytes = length_objects * class_size;
				std::copy(b2, b2 + length_bytes, b1);
				data.instance->allocated--;
			}
			return true;
		}

		bool jlist::chop()
		{
			bool result = false;
			if (data.instance->allocated > 0) {
				data.instance->allocated--;
				result = true;
			}
			return result;
		}

		jobject jlist::append_slice()
		{
			if (data.instance->allocated < capacity()) {
				auto index = data.instance->allocated;
				data.instance->allocated++;
				jobject new_slice = get_at(index);
				new_slice.construct();
				return new_slice;
			}
			return get_at(-1);
		}

		bool jlist::select_slice(corona_size_t idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			data.selections[idx] = 1;
			return true;
		}

		bool jlist::deselect_slice(corona_size_t idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			data.selections[idx] = 0;
			return true;
		}

		void jlist::deselect_all()
		{
			for (int i = 0; i < data.instance->allocated; i++) 
			{
				data.selections[i] = 0;
			}
		}

		void jlist::select_all()
		{
			for (int i = 0; i < data.instance->allocated; i++)
			{
				data.selections[i] = 1;
			}
		}

		void jlist::clear()
		{
			data.instance->allocated = 0;
			deselect_all();
		}

		char* jlist::get_bytes()
		{
			return model_box.data();
		}

		uint64_t jlist::get_size_bytes()
		{
			jfield& field = schema->get_field(class_field_id);
			return field.size_bytes;
		}

		void jschema::add_standard_fields() 
		{
			put_string_field_request string_fields[43] = {
				{ { null_row, jtype::type_string ,"full_name", "Full Name" }, { 75, "", "" } },
				{ { null_row, jtype::type_string ,"first_name", "First Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string ,"last_name", "Last Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string ,"middle_name", "Middle Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string ,"ssn", "SSN" }, { 10, "", "" }},
				{ { null_row, jtype::type_string, "email", "eEmail" }, { 200, "", ""  }},
				{ { null_row, jtype::type_string, "title", "Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "street", "Street" },{  200, "", "" } },
				{ { null_row, jtype::type_string, "suiteapt", "Suite/Apt" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "city", "City" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "state", "State" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "postal", "Postal Code" }, { 50, "", ""  }},
				{ { null_row, jtype::type_string, "country_name", "Country Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string, "country_code", "Country Code" }, { 3, "", ""  }},
				{ { null_row, jtype::type_string, "institution_name", "Institution Name" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "long_name", "Long Name" }, { 200, "", ""  }},
				{ { null_row, jtype::type_string, "short_name", "Short Name" },{  50, "", ""  }},
				{ { null_row, jtype::type_string, "unit", "Unit" }, { 10, "", "" } },
				{ { null_row, jtype::type_string, "symbol", "Symbol" }, { 10, "", "" } },
				{ { null_row, jtype::type_string, "operator", "Operator" }, { 10, "", ""  }},
				{ { null_row, jtype::type_string, "windows_path", "Windows Path" }, { 512, "", ""  }},
				{ { null_row, jtype::type_string, "linux_path", "Linux Path" }, { 512, "", "" } },
				{ { null_row, jtype::type_string, "url", "Url" }, { 512, "", "" } },
				{ { null_row, jtype::type_string, "username", "User Name" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "password", "Password" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "doc_title", "Document Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "section_title", "Section Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "block_title", "Block Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "caption", "Caption" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "paragraph", "Paragraph" }, { 4000, "", "" } },
				{ { null_row, jtype::type_string, "mime_type", "MimeType" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "base64", "Base64" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "font_name", "Font" }, { 32, "", "" } },
				{ { null_row, jtype::type_string, "name", "Object Name" }, { 32, "", "" } },
				{ { null_row, jtype::type_string, "field_name", "Field Name" }, { 64, "", "" } },
				{ { null_row, jtype::type_string, "field_description", "Field Display Name" }, { 64, "", "" } },
				{ { null_row, jtype::type_string, "field_format", "Field Format" }, { 64, "", "" } },
				{ { null_row, jtype::type_string, "string_validation_pattern", "Validation Pattern" }, { 64, "", "" } },
				{ { null_row, jtype::type_string, "string_validation_message", "Validation Message" }, { 64, "", "" } },
				{ { null_row, jtype::type_string, "user_class_class_name", "User Class Name" }, { 64, "", "" } }
			};

			put_time_field_request time_fields[4] = {
				{ { null_row, jtype::type_datetime, "birthday", "Birthday" }, 0, INT64_MAX },
				{ { null_row, jtype::type_datetime, "scheduled", "Scheduled" }, 0, INT64_MAX },
				{ { null_row, jtype::type_datetime, "date_start", "Min Date" }, 0, INT64_MAX },
				{ { null_row, jtype::type_datetime, "date_stop", "Max Date" }, 0, INT64_MAX }
			};

			put_integer_field_request int_fields[22] = {
				{ { null_row, jtype::type_int64, "count", "Count" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int8, "bold", "Bold" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int8, "italic", "Italic" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int8, "underline", "Underline" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int8, "strike_through", "Strike Through" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int8, "vertical_alignment", "Vertical alignment" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int8, "horizontal_alignment", "Vertical alignment" }, 0, INT8_MAX }, 
				{ { null_row, jtype::type_int8, "wrap_text", "Wrap text" }, 0, INT8_MAX },
				{ { null_row, jtype::type_int64, "int_start", "Min. Value" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "int_stop", "Max. Value" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "text_style", "Text Style" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "style_sheet", "Style Sheet" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "user_class_root", "User Classes" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "user_class", "User Class" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "user_field", "User Field" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int16, "field_type", "Field Type" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "user_class_class_id", "User Class Id" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int64, "base_class_id", "Base Class Id" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int32, "object_x", "X Dim" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int32, "object_y", "Y Dim" }, 0, INT64_MAX },
				{ { null_row, jtype::type_int32, "object_z", "Z Dim" }, 0, INT64_MAX },
			};

			put_double_field_request double_fields[20] = {
				{ { null_row, jtype::type_float64, "quantity", "Quantity" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "latitude", "Latitude" }, -90, 90 },
				{ { null_row, jtype::type_float64, "longitude", "Longitude" }, -180, 180 },
				{ { null_row, jtype::type_float64, "meters", "Meters" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "feet", "Feet" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "kilograms", "Kilograms" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "pounds", "Pounds" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "seconds", "Seconds" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "minutes", "Minutes" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "hours", "Hours" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "amperes", "Amperes" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "kelvin", "Kelvin" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float64, "moles", "Moles" }, -1E40, 1E40 },
				{ { null_row, jtype::type_float32, "gradient_position", "Gradient stop position" }, 0, 1E3 },
				{ { null_row, jtype::type_float32, "font_size", "Font size" }, 0, 1E3 },
				{ { null_row, jtype::type_float32, "line_spacing", "Line Spacing" }, 0, 1E3 },
				{ { null_row, jtype::type_float32, "box_border_thickness", "Box Border Thickness" }, 0, 1E3 },
				{ { null_row, jtype::type_float32, "shape_border_thickness", "Shape Border Thickness" }, 0, 1E3 },
				{ { null_row, jtype::type_float64, "double_start", "Min. Value" }, -11E30, 1E30 },
				{ { null_row, jtype::type_float64, "double_stop", "Max. Value" }, -1E30, 1E30 }
			};

			put_color_field({ null_row, jtype::type_color, "color", "color" });
			put_color_field({ null_row, jtype::type_color, "shape_fill_color", "shape_fill_color" });
			put_color_field({ null_row, jtype::type_color, "box_fill_color", "box_fill_color" });
			put_color_field({ null_row, jtype::type_color, "shape_border_color", "shape_border_color" });
			put_color_field({ null_row, jtype::type_color, "box_border_color", "box_border_color" });

			put_point_field({ null_row, jtype::type_point, "point", "point" });
			put_point_field({ null_row, jtype::type_point, "position_point", "position_point" });
			put_point_field({ null_row, jtype::type_point, "selection_point", "selection_point" });

			put_rectangle_field({ null_row, jtype::type_rectangle, "rectangle", "rectangle" });

			put_layout_rect_field({ null_row, jtype::type_layout_rect, "layout_rect", "layout_rect" });

			for (int i = 0; i < sizeof(string_fields) / sizeof(string_fields[0]); i++) {
				put_string_field(string_fields[i]);
			}

			for (int i = 0; i < sizeof(time_fields) / sizeof(time_fields[0]); i++) {
				put_time_field(time_fields[i]);
			}

			for (int i = 0; i < sizeof(int_fields) / sizeof(int_fields[0]); i++) {
				put_integer_field(int_fields[i]);
			}

			for (int i = 0; i < sizeof(double_fields) / sizeof(double_fields[0]); i++) {
				put_double_field(double_fields[i]);
			}

			idf_full_name = find_field("full_name");
			idf_first_name = find_field("first_name");
			idf_last_name = find_field("last_name");
			idf_middle_name = find_field("middle_name");
			idf_ssn = find_field("ssn");
			idf_email = find_field("email");
			idf_title = find_field("title");
			idf_street = find_field("street");
			idf_suiteapt = find_field("suiteapt");
			idf_city = find_field("city");
			idf_state = find_field("state");
			idf_postal = find_field("postal");
			idf_country_name = find_field("country_name");
			idf_country_code = find_field("country_code");
			idf_institution_name = find_field("institution_name");
			idf_long_name = find_field("long_name");
			idf_short_name = find_field("short_name");
			idf_unit = find_field("unit");
			idf_symbol = find_field("symbol");
			idf_operator = find_field("operator");
			idf_windows_path = find_field("windows_path");
			idf_linux_path = find_field("linux_path");
			idf_url = find_field("url");
			idf_username = find_field("username");
			idf_password = find_field("password");
			idf_doc_title = find_field("doc_title");
			idf_section_title = find_field("section_title");
			idf_block_title = find_field("block_title");
			idf_caption = find_field("caption");
			idf_paragraph = find_field("paragraph");
			idf_mimeType = find_field("mime_type");
			idf_base64 = find_field("base64");
			idf_file_name = find_field("file_name");
			idf_font_name = find_field("font_name");
			idf_name = find_field("name");

			idf_birthday = find_field("birthday");
			idf_scheduled = find_field("scheduled");

			idf_count = find_field("count");
			idf_bold = find_field("bold");
			idf_italic = find_field("italic");
			idf_underline = find_field("underline");
			idf_strike_through = find_field("strike_through");
			idf_vertical_alignment = find_field("vertical_alignment");
			idf_horizontal_alignment = find_field("horizontal_alignment");
			idf_wrap_text = find_field("wrap_text");
			idf_user_class_class_id = find_field("user_class_class_id");
			idf_base_class_id = find_field("base_class_id");

			idf_quantity = find_field("quantity");
			idf_latitude = find_field("latitude");
			idf_longitude = find_field("longitude");
			idf_meters = find_field("meters");
			idf_feet = find_field("feet");
			idf_kilograms = find_field("kilograms");
			idf_pounds = find_field("pounds");
			idf_seconds = find_field("seconds");
			idf_minutes = find_field("minutes");
			idf_hours = find_field("hours");
			idf_amperes = find_field("amperes");
			idf_kelvin = find_field("kelvin");
			idf_moles = find_field("moles");
			idf_gradient_position = find_field("gradient_position");
			idf_font_size = find_field("font_size");
			idf_line_spacing = find_field("line_spacing");
			idf_box_border_thickness = find_field("box_border_thickness");
			idf_shape_border_thickness = find_field("shape_border_thickness");

			idf_color = find_field("color");
			idf_shape_fill_color = find_field("shape_fill_color");
			idf_box_fill_color = find_field("box_fill_color");
			idf_shape_border_color = find_field("shape_border_color");
			idf_box_border_color = find_field("box_border_color");
			idf_point = find_field("point");
			idf_position_point = find_field("position_point");
			idf_selection_point = find_field("selection_point");
			idf_rectangle = find_field("rectangle");
			idf_layout_rect = find_field("layout_rect");

			put_class_request pcr;
			pcr.class_name = "text_style";
			pcr.class_description = "styles of text for ui";
			pcr.member_fields = { idf_name, idf_font_name, idf_font_size, idf_bold, idf_italic, idf_underline, idf_strike_through, idf_line_spacing, idf_horizontal_alignment, idf_vertical_alignment,
				idf_shape_fill_color, idf_shape_border_thickness, idf_shape_border_color, idf_box_fill_color, idf_box_border_thickness, idf_box_border_color };
			idc_text_style = put_class(pcr);

			put_object_field_request object_fields[44] = {
				{ { null_row, jtype::type_object, "view_background_style", "View Background Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "view_title_style", "View Title Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "view_subtitle_style", "View Subtitle Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "view_section_style", "View Section Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "view_style", "View Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "disclaimer_style", "Disclaimer Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "copyright_style", "Copyright Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "h1_style", "H1 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "h2_style", "H2 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "h3_style", "H3 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "column_number_head_style", "Column Number Head Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "column_text_head_style", "Column Text Head Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "column_data_style", "Column Data Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "label_style", "Label Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "control_style", "Control Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "chart_axis_style", "Chart Axis Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "chart_legend_style", "Chart Legend Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "chart_block_style", "Chart Block Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "tooltip_style", "Tooltip Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "breadcrumb_style", "Breadcrumb Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "error_style", "Error Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "home_style", "Home Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "client_style", "Client Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "carrier_style", "Carrier Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "product_style", "Product Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "coverage_style", "Coverage Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "system_style", "System Style" }, { {1,1,1}, idc_text_style }},

				{ { null_row, jtype::type_object, "company_a1_style", "Company Chart A1 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_a2_style", "Company Chart A2 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_a3_style", "Company Chart A3 Style" }, { {1,1,1}, idc_text_style }},

				{ { null_row, jtype::type_object, "company_b1_style", "Company Chart B1 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_b2_style", "Company Chart B2 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_b3_style", "Company Chart B3 Style" }, { {1,1,1}, idc_text_style }},

				{ { null_row, jtype::type_object, "company_c1_style", "Company Chart C1 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_c2_style", "Company Chart C2 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_c3_style", "Company Chart C3 Style" }, { {1,1,1}, idc_text_style }},

				{ { null_row, jtype::type_object, "company_d1_style", "Company Chart D1 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_d2_style", "Company Chart D2 Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_d3_style", "Company Chart D3 Style" }, { {1,1,1}, idc_text_style }},
				
				{ { null_row, jtype::type_object, "company_deductible_style", "Company Chart Deductible Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_neutral1_style", "Company Chart Deductible Style" }, { {1,1,1}, idc_text_style }},
				{ { null_row, jtype::type_object, "company_neutral2_style", "Company Chart Deductible Style" }, { {1,1,1}, idc_text_style }},

				{ { null_row, jtype::type_object, "string_options", "String Field Options" }, { {1,1,1}, idc_string_options }},
				{ { null_row, jtype::type_object, "double_options", "Double Field Options" }, { {1,1,1}, idc_double_options }},
				{ { null_row, jtype::type_object, "int_options", "Int Field Options" }, { {1,1,1}, idc_int_options }},
				{ { null_row, jtype::type_object, "date_options", "Date Field Options" }, { {1,1,1}, idc_date_options }},
			};

			for (int i = 0; i < sizeof(object_fields) / sizeof(object_fields[0]); i++) {
				put_object_field(object_fields[i]);
			}

			idf_view_background_style = find_field("view_background_style");
			idf_view_title_style = find_field("view_title_style");
			idf_view_subtitle_style = find_field("view_subtitle_style");
			idf_view_section_style = find_field("view_section_style");
			idf_view_style = find_field("view_style");
			idf_disclaimer_style = find_field("disclaimer_style");
			idf_copyright_style = find_field("copyright_style");
			idf_h1_style = find_field("h1_style");
			idf_h2_style = find_field("h2_style");
			idf_h3_style = find_field("h3_style");
			idf_column_number_head_style = find_field("column_number_head_style");
			idf_column_text_head_style = find_field("column_text_head_style");
			idf_column_data_style = find_field("column_data_style");
			idf_label_style = find_field("label_style");
			idf_control_style = find_field("control_style");
			idf_chart_axis_style = find_field("chart_axis_style");
			idf_chart_legend_style = find_field("chart_legend_style");
			idf_chart_block_style = find_field("chart_block_style");
			idf_tooltip_style = find_field("tooltip_style");
			idf_breadcrumb_style = find_field("breadcrumb_style");
			idf_error_style = find_field("error_style");
			idf_client_style = find_field("client_style");
			idf_carrier_style = find_field("carrier_style");
			idf_coverage_style = find_field("coverage_style");
			idf_product_style = find_field("product_style");
			idf_system_style = find_field("system_style");
			idf_home_style = find_field("home_style");
			idf_login_style = find_field("login_style");

			idf_company_a1_style = find_field("company_a1_style");
			idf_company_a2_style = find_field("company_a2_style");
			idf_company_a3_style = find_field("company_a3_style");

			idf_company_b1_style = find_field("company_b1_style");
			idf_company_b2_style = find_field("company_b2_style");
			idf_company_b3_style = find_field("company_b3_style");

			idf_company_c1_style = find_field("company_c1_style");
			idf_company_c2_style = find_field("company_c2_style");
			idf_company_c3_style = find_field("company_c3_style");

			idf_company_d1_style = find_field("company_d1_style");
			idf_company_d2_style = find_field("company_d2_style");
			idf_company_d3_style = find_field("company_d3_style");

			idf_company_deductible_style = find_field("company_deductible_style");
			idf_company_neutral1_style = find_field("company_neutral1_style");
			idf_company_neutral2_style = find_field("company_neutral2_style");

			idf_field_type = find_field("field_type");
			idf_string_options = find_field("string_options");
			idf_double_options = find_field("double_options");
			idf_date_options = find_field("date_options");
			idf_int_options = find_field("int_options");

			idf_style_sheet = find_field("style_sheet");
			pcr.class_name = "style_sheet";
			pcr.class_description = "collection of styles for ui";
			pcr.member_fields = { idf_style_sheet, idf_name, idf_view_background_style, idf_view_title_style, idf_view_subtitle_style, idf_view_section_style, idf_view_style, idf_disclaimer_style, idf_copyright_style,
				idf_h1_style, idf_h2_style, idf_h3_style, idf_column_number_head_style,idf_column_text_head_style,idf_column_data_style,idf_label_style,idf_control_style,idf_chart_axis_style,idf_chart_legend_style,idf_chart_block_style,idf_tooltip_style,
				idf_error_style, idf_client_style, idf_carrier_style, idf_coverage_style, idf_home_style, idf_login_style, idf_product_style,
				idf_company_a1_style, idf_company_a2_style, idf_company_a3_style, idf_company_b1_style, idf_company_b2_style, idf_company_b3_style, idf_company_c1_style, idf_company_c2_style, idf_company_c3_style,idf_company_d1_style, idf_company_d2_style, idf_company_d3_style, 
				idf_company_deductible_style, idf_company_neutral1_style, idf_company_neutral2_style
			};
			pcr.field_id_primary_key = idf_style_sheet;
			idc_style_sheet = put_class(pcr);

			idf_string_length = find_field("string_length");
			idf_string_validation_pattern = find_field("string_validation_pattern");
			idf_string_validation_message = find_field("string_validation_message");
			idf_string_full_text_editor = find_field("string_full_text_editor");
			idf_string_rich_text_editor = find_field("string_rich_text_editor");

			idf_date_start = find_field("date_start");
			idf_date_stop = find_field("date_stop");
			idf_date_format = find_field("date_format");

			idf_double_start = find_field("double_start");
			idf_double_stop = find_field("double_stop");
			idf_double_format = find_field("double_format");

			idf_int_start = find_field("int_start");
			idf_int_stop = find_field("int_stop");
			idf_int_format = find_field("int_format");

			idf_object_x = find_field("object_x");
			idf_object_y = find_field("object_y");
			idf_object_z = find_field("object_z");

			pcr.class_name = "string_options";
			pcr.class_description = "Options for string field";
			pcr.member_fields = { idf_string_length, idf_string_validation_message, idf_string_validation_pattern, idf_string_full_text_editor, idf_string_rich_text_editor };
			idc_string_options = put_class(pcr);

			pcr.class_name = "double_options";
			pcr.class_description = "Options for double field";
			pcr.member_fields = { idf_double_start, idf_double_stop };
			idc_double_options = put_class(pcr);

			pcr.class_name = "date_options";
			pcr.class_description = "Options for date field";
			pcr.member_fields = { idf_date_start, idf_date_stop };
			idc_date_options = put_class(pcr);

			pcr.class_name = "int_options";
			pcr.class_description = "Options for int field";
			pcr.member_fields = { idf_int_start, idf_int_stop };
			idc_int_options = put_class(pcr);

			pcr.class_name = "object_options";
			pcr.class_description = "Options for object field";
			pcr.member_fields = { idf_object_class_id, idf_object_is_list, idf_object_x, idf_object_y, idf_object_y };
			idc_object_options = put_class(pcr);

			pcr.class_name = "user_class_root";
			pcr.class_description = "custom class root";
			pcr.member_fields = { idf_user_class_root };
			pcr.field_id_primary_key = idf_user_class_root;
			idc_user_class_root = put_class(pcr);

			pcr.class_name = "user_field";
			pcr.class_description = "User field specification";
			pcr.member_fields = { idf_user_field, idf_field_name, idf_field_description, idf_field_format, idf_string_options, idf_double_options, idf_date_options, idf_int_options, idf_object_options };
			pcr.field_id_primary_key = idf_user_field;
			idc_user_field = put_class(pcr);

			put_object_field_request user_field_list_def =
			{ { null_row, jtype::type_list, "user_field_list", "User Fields" }, { {50,1,1}, idc_user_field } };
			idf_user_field_list = put_object_field(user_field_list_def);

			pcr.class_name = "user_class";
			pcr.class_description = "Custom class created by user";
			pcr.member_fields = { idf_user_class, idf_base_class_id, idf_user_class_root, idf_user_class_class_name, idf_user_class_class_id, idf_user_field_list };
			pcr.field_id_primary_key = idf_user_class;
			idc_user_class = put_class(pcr);

			// TODO, these will have to be added later
			/*
			
					struct gradientStopDto {
			float position;
			colorDto color;
		};

			struct bitmapDto {
				std::string name;
				std::string filename;
				bool cropEnabled;
				marginDto crop;
				std::list<sizeIntDto> sizes;
			};

			struct bitmapBrushDto {
				std::string name;
				std::string bitmapName;
			};

			struct solidBrushDto {
				std::string name;
				colorDto color;
				bool stock;
			};

			struct linearGradientBrushDto {
				std::string name;
				pointDto	start,
					stop;
				std::list<gradientStopDto> gradientStops;
				bool stock;
			};

			struct radialGradientBrushDto {
				std::string name;
				pointDto	center,
					offset;
				float		radiusX,
					radiusY;
				bool stock;
				std::list<gradientStopDto> gradientStops;
			};
			
			*/

		}

		bool schema_tests()
		{
			try {
				dynamic_box box;
				box.init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema( &box, 20, true, schema_id );

				relative_ptr_type quantity_field_id = null_row;
				relative_ptr_type last_name_field_id = null_row;
				relative_ptr_type first_name_field_id = null_row;
				relative_ptr_type birthday_field_id = null_row;
				relative_ptr_type count_field_id = null_row;
				relative_ptr_type title_field_id = null_row;
				relative_ptr_type institution_field_id = null_row;

				schema.bind_field("quantity", quantity_field_id);
				schema.bind_field("lastName", last_name_field_id);
				schema.bind_field("firstName", first_name_field_id);
				schema.bind_field("birthday", birthday_field_id);
				schema.bind_field("count", count_field_id);
				schema.bind_field("title", title_field_id);
				schema.bind_field("institutionName", institution_field_id);

				if (quantity_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed:" << std::endl;
					return false;
				}

				if (last_name_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (first_name_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (birthday_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (title_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (count_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				relative_ptr_type failed_field_id = schema.find_class("badFieldName");

				if (failed_field_id != null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, title_field_id, count_field_id, quantity_field_id };
				relative_ptr_type person_class_id = schema.put_class(person);

				if (person_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				jclass person_class = schema.get_class(person_class_id);

				if (person_class.size() != 6) {
					std::cout << __LINE__ << ":class size failed failed" << std::endl;
					return false;
				}

				int offset_start = 0;
				for (int i = 0; i < person_class.size(); i++) {
					auto& fldref = person_class.detail(i);
					auto& fld = schema.get_field(fldref.field_id);
					//				std::cout << fld.name << " " << fld.description << " " << fldref.offset << " " << fld.size_bytes << std::endl;
					if (offset_start && offset_start != fldref.offset) {
						std::cout << __LINE__ << ":class alignment failed" << std::endl;

					}
					offset_start += fld.size_bytes;
				}

				put_class_request company;
				company.class_name = "company";
				company.class_description = "a company is a collection of people";
				company.member_fields = { institution_field_id, member_field(person_class_id, dimensions_type { 10, 1, 1 }) };
				relative_ptr_type company_class_id = schema.put_class(company);

				if (company_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}

			return true;
		}

		bool collection_tests()
		{

			try {
				dynamic_box box;
				box.init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema(&box, 20, true, schema_id);

				relative_ptr_type quantity_field_id = null_row;
				relative_ptr_type last_name_field_id = null_row;
				relative_ptr_type first_name_field_id = null_row;
				relative_ptr_type birthday_field_id = null_row;
				relative_ptr_type count_field_id = null_row;
				relative_ptr_type title_field_id = null_row;
				relative_ptr_type institution_field_id = null_row;

				schema.bind_field("quantity", quantity_field_id);
				schema.bind_field("lastName", last_name_field_id);
				schema.bind_field("firstName", first_name_field_id);
				schema.bind_field("birthday", birthday_field_id);
				schema.bind_field("count", count_field_id);
				schema.bind_field("title", title_field_id);
				schema.bind_field("institutionName", institution_field_id);

				jmodel sample_model;
				sample_model.name = "my model";

				schema.put_model(sample_model);

				jcollection_ref ref;
				ref.data = &box;
				ref.model_name = sample_model.name;
				ref.max_actors = 2;
				ref.max_objects = 50;
				ref.collection_size_bytes = 1 << 19;

				init_collection_id(ref.collection_id);

				jcollection people = schema.create_collection(&ref);

				jactor sample_actor;
				sample_actor.actor_name = "sample actor";
				sample_actor.actor_id = null_row;
				sample_actor = people.create_actor(sample_actor);

				put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, count_field_id, quantity_field_id };
				relative_ptr_type person_class_id = schema.put_class(person);

				if (person_class_id == null_row)
				{
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				jarray pa;

				int birthdaystart = 1941;
				int countstart = 12;
				double quantitystart = 10.22;
				int increment = 5;

				relative_ptr_type people_object_id;

				auto sl = people.create_object(0, sample_actor.actor_id, person_class_id, people_object_id);
				auto last_name = sl.get_string(0);
				auto first_name = sl.get_string(1);
				auto birthday = sl.get_time(2);
				auto count = sl.get_int64(3);
				auto qty = sl.get_double(4);
				last_name = "last 1";
				first_name = "first 1";
				birthday = birthdaystart + increment * 0;
				count = countstart + increment * 0;
				qty = quantitystart + increment * 0;

				sl = people.create_object(1, sample_actor.actor_id, person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 2";
				first_name = "first 2";
				birthday = birthdaystart + increment * 1;
				count = countstart + increment * 1;
				qty = quantitystart + increment * 1;

				sl = people.create_object(2, sample_actor.actor_id, person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 3";
				first_name = "first 3";
				birthday = birthdaystart + increment * 2;
				count = countstart + increment * 2;
				qty = quantitystart + increment * 2;

				sl = people.create_object(3, sample_actor.actor_id, person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 4";
				first_name = "first 4";
				birthday = birthdaystart + increment * 3;
				count = countstart + increment * 3;
				qty = quantitystart + increment * 3;

				sl = people.create_object(4, sample_actor.actor_id, person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
				first_name = "first 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
				birthday = birthdaystart + increment * 4;
				count = countstart + increment * 4;
				qty = quantitystart + increment * 4;

				int inc_count = 0;

				for (auto sl : people)
				{
					last_name = sl.item.get_string(0);
					if (!last_name.starts_with("last")) {
						std::cout << __LINE__ << ":last name failed" << std::endl;
						return false;
					}
					first_name = sl.item.get_string(1);
					if (!first_name.starts_with("first")) {
						std::cout << __LINE__ << ":first name failed" << std::endl;
						return false;
					}
					birthday = sl.item.get_time(2);
					count = sl.item.get_int64(3);
					qty = sl.item.get_double(4);

					if (birthday != birthdaystart + increment * inc_count) {
						std::cout << __LINE__ << ":birthday failed" << std::endl;
						return false;
					}

					if (count != countstart + increment * inc_count) {
						std::cout << __LINE__ << ":count failed" << std::endl;
						return false;
					}

					if (qty != quantitystart + increment * inc_count) {

						std::cout << __LINE__ << ":qty failed" << std::endl;
						return false;
					}

					inc_count++;
				}

				return true;
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}
			
		}

		bool array_tests()
		{

			try {
				dynamic_box box;
				box.init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema(&box, 50, true, schema_id);

				put_class_request sprite_frame_request;

				sprite_frame_request.class_name = "spriteframe";
				sprite_frame_request.class_description = "sprite frame";
				sprite_frame_request.member_fields = { "shortName", "rectangle", "color" };
				relative_ptr_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

				if (sprite_frame_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				put_class_request sprite_class_request;
				sprite_class_request.class_name = "sprite";
				sprite_class_request.class_description = "sprite";
				sprite_class_request.member_fields = { "shortName", "rectangle", member_field(sprite_frame_class_id, { 10, 10, 1 }) };
				relative_ptr_type sprite_class_id = schema.put_class(sprite_class_request);

				if (sprite_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				collection_id_type colid;

				init_collection_id(colid);

				relative_ptr_type classesb[2] = { sprite_class_id, null_row };

				model_type sprite_model;

				sprite_model.name = "sprite model";
				schema.put_model(sprite_model);

				jcollection_ref ref;
				ref.data = &box;
				ref.model_name = sprite_model.name;
				ref.max_actors = 2;
				ref.max_objects = 50;
				ref.collection_size_bytes = 1 << 19;


				init_collection_id(ref.collection_id);

				jcollection sprites = schema.create_collection(&ref);

				actor_type sprite_boy;
				sprite_boy.actor_id = null_row;
				sprite_boy.actor_name = "sprite_boy";

				sprite_boy = sprites.create_actor(sprite_boy);
				relative_ptr_type new_sprite_id = null_row;

				for (int i = 0; i < 10; i++) {
					auto slice = sprites.create_object(i, sprite_boy.actor_id, sprite_class_id, new_sprite_id);
					auto image_name = slice.get_string(0);
					auto image_rect = slice.get_rectangle(1);
					auto frame_array = slice.get_object(2);

					image_name = std::format("{} #{}", "image", i);
					image_rect->x = 0;
					image_rect->y = 0;
					image_rect->w = 1000;
					image_rect->h = 1000;

#if _DETAIL
					std::cout << "before:" << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

					for (auto frame : frame_array)
					{
						auto dim = frame.item.get_dim();
						auto frame_name = frame.item.get_string(0);
						auto frame_rect = frame.item.get_rectangle(1);
						frame_name = std::format("{} #{}", "frame", dim.x);
						frame_rect->x = dim.x * 100.0;
						frame_rect->y = dim.y * 100.0;
						frame_rect->w = 100.0;
						frame_rect->h = 100.0;
					}

#if _DETAIL
					std::cout << "after:" << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

				}

				int scount = 0;

				for (auto slice : sprites)
				{
					auto image_name = slice.item.get_string(0);
					auto image_rect = slice.item.get_rectangle(1);

#if _DETAIL
					std::cout << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

					if (image_rect->w != 1000 || image_rect->h != 1000) {

						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}

					auto frames = slice.item.get_object(2);

					int fcount = 0;
					for (auto frame : frames)
					{
						auto image_rect = slice.item.get_rectangle(1);
						if (image_rect->w != 1000 || image_rect->h != 1000)
						{
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						}

						auto frame_rect = frame.item.get_rectangle(1);

#if _DETAIL
						std::cout << frame_rect->w << " " << frame_rect->h << " " << frame_rect->x << " " << frame_rect->y << std::endl;
#endif

						if (frame_rect->w != 100 || frame_rect->h != 100)
						{
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						}

						auto dim = frame.item.get_dim();
						if (frame_rect->x != dim.x * 100.0) {
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						};
						if (frame_rect->y != dim.y * 100.0) {
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						}
					}
				}

				return true;
			}
			catch (std::exception& exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}
		}

		bool model_tests()
		{

			try 
			{
				dynamic_box box;
				box.init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema(&box, 50, true, schema_id);

				put_double_field_request dfr;
				dfr.name.name = "limit";
				dfr.name.description = "Maximum amount paid by policy";
				dfr.name.type_id = jtype::type_float64;
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				relative_ptr_type limit_field_id = schema.put_double_field(dfr);

				dfr.name.name = "attachment";
				dfr.name.description = "Point at which policy begins coverage";
				dfr.name.type_id = jtype::type_float64; 
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				relative_ptr_type attachment_field_id = schema.put_double_field(dfr);

				dfr.name.name = "deductible";
				dfr.name.description = "Point at which policy begins paying";
				dfr.name.type_id = jtype::type_float64; 
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				relative_ptr_type deductible_field_id = schema.put_double_field(dfr);

				put_string_field_request sfr;
				sfr.name.name = "comment";
				sfr.name.description = "Descriptive text";
				sfr.options.length = 512;
				relative_ptr_type comment_field_id = schema.put_string_field(sfr);

				sfr.name.name = "program_name";
				sfr.name.description = "name of a program";
				sfr.options.length = 200;
				relative_ptr_type program_name_field_id = schema.put_string_field(sfr);

				sfr.name.name = "program_description";
				sfr.name.description = "name of a program";
				sfr.options.length = 512;
				relative_ptr_type program_description_field_id = schema.put_string_field(sfr);

				sfr.name.name = "coverage_name";
				sfr.name.description = "name of a coverage";
				sfr.options.length = 200;
				relative_ptr_type coverage_name_id = schema.put_string_field(sfr);

				sfr.name.name = "carrier_name";
				sfr.name.description = "name of a carrier";
				sfr.options.length = 200;
				relative_ptr_type carrier_name_id = schema.put_string_field(sfr);

				put_class_request pcr;

				pcr.class_name = "program";
				pcr.class_description = "program summary";
				pcr.member_fields = { "program_name", "program_description" };
				relative_ptr_type program_class_id = schema.put_class(pcr);

				if (program_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "coverage";
				pcr.class_description = "coverage frame";
				pcr.member_fields = { "coverage_name", "comment", "rectangle" };
				relative_ptr_type coverage_class_id = schema.put_class(pcr);

				if (coverage_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "coverage_spacer";
				pcr.class_description = "spacer frame";
				pcr.member_fields = { "rectangle" };
				relative_ptr_type coverage_spacer_id = schema.put_class(pcr);

				if (coverage_spacer_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "carrier";
				pcr.class_description = "carrier frame";
				pcr.member_fields = { "carrier_name", "comment", "rectangle", "color" };
				relative_ptr_type carrier_class_id = schema.put_class(pcr);

				if (coverage_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy";
				pcr.class_description = "policy block";
				pcr.member_fields = { "coverage_name", "carrier_name", "comment", "rectangle", "color", "limit", "attachment" };
				relative_ptr_type policy_class_id = schema.put_class(pcr);

				if (policy_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy_deductible";
				pcr.class_description = "deductible block";
				pcr.member_fields = { "coverage_name", "comment", "rectangle", "color", "deductible" };
				relative_ptr_type policy_deductible_class_id = schema.put_class(pcr);

				if (policy_deductible_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy_umbrella";
				pcr.class_description = "deductible block";
				pcr.member_fields = { "comment", "rectangle", "color", "limit", "attachment"};
				relative_ptr_type policy_umbrella_class_id = schema.put_class(pcr);

				if (policy_deductible_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				jmodel jm;

				jm.name = "program_chart";

				model_creatable_class* mcr;
				model_selectable_class* msr;
				model_updatable_class* mur;
				selector_rule* sr;

				mcr = jm.create_options.append();
				mcr->rule_name = "add coverage";
				mcr->selectors.always();
				mcr->create_class_id = coverage_class_id;
				mcr->replace_selected = false;
				mcr->select_on_create = true;
				mcr->item_id_class = null_row;

				mcr = jm.create_options.append();
				mcr->rule_name = "add carrier";
				mcr->selectors.always();
				mcr->create_class_id = carrier_class_id;
				mcr->replace_selected = false;
				mcr->select_on_create = true;
				mcr->item_id_class = null_row;

				mcr = jm.create_options.append();
				mcr->rule_name = "add coverage spacer";
				mcr->selectors.when(coverage_class_id);
				mcr->create_class_id = coverage_spacer_id;
				mcr->select_on_create = false;
				mcr->replace_selected = false;
				mcr->item_id_class = null_row;

				mcr = jm.create_options.append();
				mcr->rule_name = "add policy";
				mcr->selectors.when(coverage_class_id, carrier_class_id);
				mcr->create_class_id = policy_class_id;
				mcr->select_on_create = true;
				mcr->replace_selected = false;
				mcr->item_id_class = null_row;

				mcr = jm.create_options.append();
				mcr->rule_name = "add deductible";
				mcr->selectors.when(coverage_class_id);
				mcr->create_class_id = policy_deductible_class_id;
				mcr->select_on_create = true;
				mcr->replace_selected = false;
				mcr->item_id_class = null_row;

				mcr = jm.create_options.append();
				mcr->rule_name = "add umbrella";
				mcr->selectors.when(policy_class_id);
				mcr->create_class_id = policy_umbrella_class_id;
				mcr->select_on_create = true;
				mcr->replace_selected = true;
				mcr->item_id_class = null_row;

				msr = jm.select_options.append();
				msr->rule_name = "select coverage";
				msr->select_class_id = coverage_class_id;

				msr = jm.select_options.append();
				msr->rule_name = "select carrier";
				msr->select_class_id = carrier_class_id;

				msr = jm.select_options.append();
				msr->rule_name = "select coverage spacer";
				msr->select_class_id = coverage_spacer_id;

				msr = jm.select_options.append();
				msr->rule_name = "select policy";
				msr->select_class_id = policy_class_id;

				msr = jm.select_options.append();
				msr->rule_name = "select deductible";
				msr->select_class_id = policy_deductible_class_id;

				msr = jm.select_options.append();
				msr->rule_name = "select umbrella";
				msr->select_class_id = policy_umbrella_class_id;

				mur = jm.update_options.append();
				mur->rule_name = "update coverage";
				mur->update_class_id = coverage_class_id;

				mur = jm.update_options.append();
				mur->rule_name = "update carrier";
				mur->update_class_id = carrier_class_id;

				mur = jm.update_options.append();
				mur->rule_name = "update coverage spacer";
				mur->update_class_id = coverage_spacer_id;

				mur = jm.update_options.append();
				mur->rule_name = "update policy";
				mur->update_class_id = policy_class_id;

				mur = jm.update_options.append();
				mur->rule_name = "update deductible";
				mur->update_class_id = policy_deductible_class_id;

				mur = jm.update_options.append();
				mur->rule_name = "update umbrella";
				mur->update_class_id = policy_umbrella_class_id;

				schema.put_model(jm);

				jcollection_ref ref;
				ref.data = &box;
				ref.model_name = jm.name;
				ref.max_actors = 2;
				ref.max_objects = 100;
				ref.collection_size_bytes = 1 << 19;

				if (!init_collection_id(ref.collection_id))
				{
					std::cout << __LINE__ << "collection id failed" << std::endl;
				}

				jcollection program_chart = schema.create_collection(&ref);

				jactor sample_actor;
				sample_actor.actor_name = "sample actor";
				sample_actor.actor_id = null_row;
				sample_actor = program_chart.create_actor(sample_actor);

				auto result = program_chart.get_actor_state(sample_actor.actor_id, null_row, "initial state");

				// now, we want to get the command for creating a pair of objects and test.
				// first we shall see if we can create a carrier
				auto& create_carrier_option = result.create_objects
					.where([carrier_class_id](const auto& acokp) { return acokp.second.class_id == carrier_class_id; })
					.get_object()
					.get_value();

				// then we shall see if we can create a coverage
				auto& create_coverage_option = result.create_objects
					.where([coverage_class_id](const auto& acokp) { return acokp.second.class_id == coverage_class_id; })
					.get_object()
					.get_value();

				// now, we will create a carrier
				auto result2 = program_chart.create_object(create_carrier_option, "create a carrier");

				// and we should have a created carrier
				auto new_carrier = result2.get_modified_object();
				auto new_carrier_name = program_chart.get_at(new_carrier.object_id).get_string(carrier_name_id, true);
				new_carrier_name = "Test Carrier";

				if (new_carrier_name.value() != "Test Carrier")
				{
					std::cout << "new carrier does not have right text" << std::endl;
				}

				// and now we should also be able to create a coverage
				create_coverage_option = result.create_objects
					.where([coverage_class_id](const auto& acokp) { return acokp.second.class_id == coverage_class_id; })
					.get_object()
					.get_value();

				auto result3 = program_chart.create_object(create_coverage_option, "create a coverage");

				// and we should have a created coverage
				auto new_coverage = result3.get_modified_object();
				auto new_coverage_name = program_chart.get_at(new_coverage.object_id).get_string(coverage_name_id, true);
				new_coverage_name = "Test Coverage";

				if (new_coverage_name.value() != "Test Coverage")
				{
					std::cout << "new coverage does not have right text" << std::endl;
				}

				// and now that we have a coverage and a carrier, we should be able to select them both to create a program chart item

				auto select_coverage = result3.create_select_request(new_coverage.object_id, false);
				auto result4 = program_chart.select_object(select_coverage, "select a coverage");

				auto select_carrier = result4.create_select_request(new_carrier.object_id, true);
				auto result5 = program_chart.select_object(select_carrier, "select a carrier");

				return true;
			}
			catch (std::exception& exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}
		}

	}
}
