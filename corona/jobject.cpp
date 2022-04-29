
#include "jobject.h"
#include "combaseapi.h"
#include "extractor.h"

namespace countrybit
{
	namespace database
	{

		int compare(const dimensions_type& a, const dimensions_type& b)
		{
			int t = a.z - b.z;
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

		jslice actor_command_response::create_object(jschema* _schema, row_id_type _class_id)
		{
			auto myclass = _schema->get_class(_class_id);
			auto bytes_to_allocate = myclass.item().class_size_bytes;
			row_id_type location = data.reserve(bytes_to_allocate);

			dimensions_type d = { 0,0,0 };

			jslice ja(nullptr, _schema, _class_id, &data, location, d);
			ja.construct();
			return ja;
		}

		jslice actor_command_response::copy_object(jschema* _schema, jslice& _src)
		{
			jslice dest = create_object(_schema, _src.get_class().item().class_id);
			dest.update(_src);
			return dest;
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
			else if (
				required->all_of([this, selections](selector_rule& src) {
					int c = selections->count_if(
						[src, this](row_id_type& dest) {
							auto obj = this->objects[dest];
							row_id_type class_id = obj.item().class_id;
							return class_id == src.class_id;
						});
					return c == 1;
					})
				&&
				selections->all_of([this, required](row_id_type& dest) {
				return required->any_of([this, dest](selector_rule& src) {
					return src.class_id == dest;
					});
					})
				)
			{
				return true;
			}

			return false;
		}

		actor_command_response jcollection::get_command_result(row_id_type _actor)
		{
			actor_command_response acr;

			acr.collection_id = collection_id;

			auto model = schema->get_model(ref->model_name);
			auto& actor = actors[ _actor ];

			auto& create_options = model.create_options;
			auto& select_options = model.select_options;
			auto& update_options = model.update_options;
			auto selections = &actor.selections;

			// first figure out our create_options

			for (auto oi : create_options)
			{
				auto rule = &oi.item;
				auto required = &oi.item.selectors;

				if (selector_applies(&oi.item.selectors, _actor)) {
					actor_create_object aco;
					auto selected_create = selections->where([rule, this](row_id_type& src) {
						return objects[src].item().class_id == rule->item_id_class;
						});
					if (selected_create != std::end(*selections)) {
						row_id_type object_id = selected_create.get_value().item;
						row_id_type item_id = objects[object_id].item().item_id;
						aco.item_id = item_id;
					}
					else 
					{
						aco.item_id = null_row;
					}
					aco.collection_id = collection_id;
					aco.actor_id = _actor;
					aco.class_id = oi.item.create_class_id;
					aco.item = acr.create_object(schema, aco.class_id);
					aco.select_on_create = oi.item.select_on_create;
					row_id_type create_id = acr.create_objects.size();
					acr.create_objects.insert_or_assign(create_id, aco);
				}
			}

			// now to our select options
			
			for (auto oi : select_options)
			{
				auto rule = &oi.item;
				auto required = &oi.item.selectors;

				if (selector_applies(&oi.item.selectors, _actor)) 
				{
					// we can now select objects of this class
					for (row_id_type oid = 0; oid < objects.size(); oid++)
					{
						auto obj = objects[oid];
						if (obj.item().class_id == rule->select_class_id) 
						{
							actor_view_object avo;
							avo.collection_id = collection_id;
							avo.object_id = oid;
							avo.selectable = false;
							avo.selected = false;
							avo.updateable = false;
							acr.view_objects.put(oid, avo, [](actor_view_object& _dest) { _dest.selectable = true;  });
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
					for (row_id_type oid = 0; oid < objects.size(); oid++)
					{
						auto obj = objects[oid];
						if (obj.item().class_id == rule->update_class_id)
						{
							actor_view_object avo;
							avo.collection_id = collection_id;
							avo.object_id = oid;
							avo.selectable = false;
							avo.selected = false;
							avo.updateable = false;
							acr.view_objects.put(oid, avo, [](actor_view_object& _dest) { _dest.updateable = true;  });
						}
					}
				}
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
			auto id = actors.first_index([_name](auto& t) { return t.actor_name == _name; });
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

		row_id_type jcollection::put_actor(actor_type _actor)
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

		actor_command_response jcollection::select_object(const actor_select_object& _select)
		{
			actor_command_response acr;
			acr.collection_id = collection_id;
			if (!actors.check(_select.actor_id))
			{
				return acr;
			}
			actor_type ac = get_actor(_select.actor_id);
			if (!_select.extend) {
				ac.selections.clear();
			}
			if (objects.check(_select.object_id)) {
				row_id_type selection = _select.object_id;
				ac.selections.push_back(selection);
			}
			acr = get_command_result(_select.actor_id);
			return acr;
		}

		actor_command_response jcollection::create_object(actor_create_object& _create)
		{
			actor_command_response acr;
			acr.collection_id = collection_id;
			if (!actors.check(_create.actor_id))
			{
				return acr;
			}
			actor_type ac = get_actor(_create.actor_id);
			row_id_type item_id = _create.item_id;
			row_id_type object_id = null_row;

			create_object(item_id, _create.actor_id, _create.class_id, object_id);
			if (object_id != null_row) 
			{
				auto slice = get_object(object_id);
				update_object(object_id, slice);
				if (_create.select_on_create) {
					ac.selections.clear();
					ac.selections.push_back(object_id);
					put_actor(ac);
				}
			}

			acr = get_command_result(_create.actor_id);
			return acr;
		}

		actor_command_response jcollection::update_object(actor_update_object& _update)
		{
			actor_command_response acr;
			acr.collection_id = collection_id;
			if (!actors.check(_update.actor_id))
			{
				return acr;
			}
			actor_type ac = get_actor(_update.actor_id);
			row_id_type object_id = _update.object_id;

			if (object_id != null_row && objects.check(object_id))
			{
				auto slice = get_object(object_id);
				update_object(object_id, slice);
			}

			acr = get_command_result(_update.actor_id);
			return acr;
		}

		jslice jcollection::create_object(row_id_type _item_id, row_id_type _actor_id, row_id_type _class_id, row_id_type& object_id)
		{
			auto myclass = schema->get_class(_class_id);

			object_name composed_class_field_name;
			dimensions_type d = { 1, 1, 1 };
			schema->get_class_field_name(composed_class_field_name, myclass.item().name, d);
			auto find_field_id = schema->find_field(composed_class_field_name);
			if (find_field_id == null_row)
			{
				put_object_field_request porf;
				porf.name.name = composed_class_field_name;
				porf.name.type_id = jtype::type_object;
				porf.options.class_id = _class_id;
				porf.options.class_name = myclass.item().name;
				porf.options.class_size_bytes = myclass.item().class_size_bytes;
				porf.options.dim = d;
				find_field_id = schema->put_object_field(porf);
			}
			find_field_id = schema->find_field(composed_class_field_name);
			auto find_field = schema->get_field(find_field_id);
			auto bytes_to_allocate = find_field.size_bytes;

			collection_object_type co;
			co.oid.collection_id = collection_id;
			co.class_field_id = find_field_id;
			co.class_id = _class_id;
			co.item_id = _item_id;
			co.otype = jtype::type_object;

			auto new_object = objects.create_item( &co, bytes_to_allocate, nullptr);
			co.oid.row_id = new_object.row_id();
			char* bytes = new_object.pdetails();
			jarray ja(nullptr, schema, find_field_id, bytes, true);
			return ja.get_slice(0);
		}

		jslice jcollection::get_object(row_id_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			if (existing_object.pitem()->otype == jtype::type_object) {
				jarray jax(nullptr, schema, existing_object.item().class_field_id, existing_object.pdetails());
				return jax.get_slice(0);
			}
			else 
			{
				jslice empty;
				return empty;
			}
		}

		jslice jcollection::update_object(row_id_type _object_id, jslice _slice)
		{
			auto existing_object = objects.get_item(_object_id);
			if (existing_object.pitem()->otype == jtype::type_object) {
				jarray jax(nullptr, schema, existing_object.item().class_field_id, existing_object.pdetails());
				auto slice_to_update = jax.get_slice(0);
				slice_to_update.update(_slice);
				return slice_to_update;
			}
			else
			{
				jslice empty;
				return empty;
			}
		}

		jslice::jslice() : schema(nullptr), class_id(null_row), bytes(nullptr)
		{
			;
		}

		jslice::jslice(jslice *_parent, jschema* _schema, row_id_type _class_id, char* _bytes, dimensions_type _dim) : parent(_parent), schema(_schema), class_id(_class_id), bytes(_bytes), dim(_dim), box(nullptr), location(null_row)
		{
			the_class = schema->get_class(_class_id);
		}

		jslice::jslice(jslice* _parent, jschema* _schema, row_id_type _class_id, serialized_box_container *_box, row_id_type _location, dimensions_type _dim) : parent(_parent), schema(_schema), class_id(_class_id), bytes(nullptr), dim(_dim), box(_box), location(_location)
		{
			the_class = schema->get_class(_class_id);
		}

		jslice& jslice::get_parent_slice()
		{
			return *parent;
		}

		jclass jslice::get_class()
		{
			return the_class;
		}

		size_t jslice::get_offset(int field_idx, jtype _type)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || bytes == nullptr) {
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
		std::partial_ordering jslice::compare_express(jtype _type, char *c1, char *c2)
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

		dimensions_type jslice::get_dim() 
		{
			return dim; 
		}

		void jslice::construct()
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
							jai.construct();
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

		jfield& jslice::get_field(int field_idx)
		{
			jclass_field& jcf = the_class.detail(field_idx);
			jfield &jf = schema->get_field(jcf.field_id);
			return jf;
		}

		int jslice::get_field_index_by_id(row_id_type field_id)
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

		jclass_field& jslice::get_class_field(int field_idx)
		{
			jclass_field& jcf = the_class.detail(field_idx);
			return jcf;
		}

		jfield& jslice::get_field_by_id(row_id_type field_id)
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

		int8_box jslice::get_int8(int field_idx)
		{
			return get_boxed<int8_box>(field_idx, jtype::type_int8);
		}

		int16_box jslice::get_int16(int field_idx)
		{
			return get_boxed<int16_box>(field_idx, jtype::type_int16);
		}

		int32_box jslice::get_int32(int field_idx)
		{
			return jslice::get_boxed<int32_box>(field_idx, jtype::type_int32);
		}

		int64_box jslice::get_int64(int field_idx)
		{
			return jslice::get_boxed<int64_box>(field_idx, jtype::type_int64);
		}

		float_box jslice::get_float(int field_idx)
		{
			return jslice::get_boxed<float_box>(field_idx, jtype::type_float32);
		}

		double_box jslice::get_double(int field_idx)
		{
			return jslice::get_boxed<double_box>(field_idx, jtype::type_float64);
		}

		time_box jslice::get_time(int field_idx)
		{
			return get_boxed<time_box>(field_idx, jtype::type_datetime);
		}

		point_box jslice::get_point(int field_idx)
		{
			return get_boxed<point_box>(field_idx, jtype::type_point);
		}

		rectangle_box jslice::get_rectangle(int field_idx)
		{
			return get_boxed<rectangle_box>(field_idx, jtype::type_rectangle);
		}

		image_box jslice::get_image(int field_idx)
		{
			return get_boxed<image_box>(field_idx, jtype::type_image);
		}

		wave_box jslice::get_wave(int field_idx)
		{
			return get_boxed<wave_box>(field_idx, jtype::type_wave);
		}

		midi_box jslice::get_midi(int field_idx)
		{
			return get_boxed<midi_box>(field_idx, jtype::type_midi);
		}

		color_box jslice::get_color(int field_idx)
		{
			return get_boxed<color_box>(field_idx, jtype::type_color);
		}

		query_box jslice::get_query(int field_idx)
		{
			return get_boxed_ex<query_box>(field_idx, jtype::type_query);
		}

		sql_remote_box jslice::get_sql_remote(int field_idx)
		{
			return get_boxed_ex<sql_remote_box>(field_idx, jtype::type_sql);
		}

		http_remote_box jslice::get_http_remote(int field_idx)
		{
			return get_boxed_ex<http_remote_box>(field_idx, jtype::type_http);
		}

		file_remote_box jslice::get_file_remote(int field_idx)
		{
			return get_boxed_ex<file_remote_box>(field_idx, jtype::type_file);
		}

		string_box jslice::get_string(int field_idx)
		{
			size_t offset = get_offset(field_idx, jtype::type_string);
			char *b = get_bytes() + offset;
			auto temp = string_box::get(b);
			return temp;
		}

		jarray jslice::get_object(int field_idx)
		{
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

		jlist jslice::get_list(int field_idx)
		{
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

		collection_id_box jslice::get_collection_id(int field_idx)
		{
			return jslice::get_boxed<collection_id_box>(field_idx, jtype::type_collection_id);
		}

		object_id_box jslice::get_object_id(int field_idx)
		{
			return jslice::get_boxed<object_id_box>(field_idx, jtype::type_object_id);
		}

		int jslice::size()
		{
			auto the_class = schema->get_class(class_id);
			return the_class.size();
		}

		void jslice::update(jslice& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				std::copy(_src_slice.get_bytes(), _src_slice.get_bytes() + size(), get_bytes());
			}
			else 
			{
				row_id_type fis, fid, ssf;

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
		
		std::partial_ordering jslice::compare(int _dst_idx, jslice& _src_slice, int _src_idx)
		{
			auto field_type = get_field(_dst_idx).type_id;
			auto offset1 = get_offset(_dst_idx);
			auto offset2 = _src_slice.get_offset(_src_idx);
			char* c1 = get_bytes() + offset1;
			char* c2 = _src_slice.get_bytes() + offset2;
			return compare_express(field_type, c1, c2);
		}

		std::partial_ordering jslice::compare(jslice& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				row_id_type fis, fid, ssf;
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
				row_id_type fis, fid, ssf;
				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto& fld_source = _src_slice.get_field(fis);
					fid = get_field_index_by_id(fld_source.field_id);
					auto& fld_dest = get_field(fid);
					if (!schema->is_empty(fld_dest)) {
						auto offset1 = get_offset(fld_source.type_id);
						auto offset2 = _src_slice.get_offset(fld_dest.type_id);
						char* c1 = get_bytes() + offset1;
						char* c2 = _src_slice.get_bytes() + offset2;
						auto x = compare_express(fld_source.type_id, c1, c2);
						if (x != std::strong_ordering::equal) {
							return x;
						}
					}
				}
				return std::strong_ordering::equal;
			}
		}

		void implement_pointer_comparison(filter_element& _src)
		{
			switch (_src.comparison) {
			case filter_comparison_types::eq:
				_src.compare = [](char* a, char* b) {
					return a == b;
				};
				break;
			case filter_comparison_types::ls:
				_src.compare = [](char* a, char* b) {
					return a < b;
				};
				break;
			case filter_comparison_types::gt:
				_src.compare = [](char* a, char* b) {
					return a > b;
				};
				break;
			case filter_comparison_types::lseq:
				_src.compare = [](char* a, char* b) {
					return a <= b;
				};
				break;
			case filter_comparison_types::gteq:
				_src.compare = [](char* a, char* b) {
					return a >= b;
				};
				break;
			case filter_comparison_types::distance:
				_src.compare = [_src](char* a, char* b) {
					return abs(a - b) <= _src.distance_threshold;
				};
				break;
			}
		}

		template <typename BoxAType, typename BoxBType> void implement_comparison(filter_element& _src)
		{
			switch (_src.comparison) {
			case filter_comparison_types::eq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return boxa == boxb;
				};
				break;
			case filter_comparison_types::ls:
				_src
					.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return boxa < boxb;
				};
				break;
			case filter_comparison_types::gt:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return boxa > boxb;
				};
				break;
			case filter_comparison_types::lseq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return boxa <= boxb;
				};
				break;
			case filter_comparison_types::gteq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return boxa >= boxb;
				};
				break;
			case filter_comparison_types::distance:
				_src.compare = [_src](char* a, char* b) {
					BoxAType boxa(a);
					BoxBType boxb(b);
					return distance(boxa, boxb) <= _src.distance_threshold;
				};
				break;
			}
		}

		template <typename BoxBType, typename BoxBPrimitive> void implement_string_a_numeric_comparison(filter_element& _src)
		{
			switch (_src.comparison) {
			case filter_comparison_types::eq:
				_src.compare = [](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return f == boxb;
				};
				break;
			case filter_comparison_types::ls:
				_src.compare = [](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return f < boxb;
				};
				break;
			case filter_comparison_types::gt:
				_src.compare = [](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return f > boxb;
				};
				break;
			case filter_comparison_types::lseq:
				_src.compare = [](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return f <= boxb;
				};
				break;
			case filter_comparison_types::gteq:
				_src.compare = [](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return f >= boxb;
				};
				break;
			case filter_comparison_types::distance:
				_src.compare = [_src](char* a, char* b) {
					string_box boxa = string_box::get(a);
					BoxBPrimitive f = boxa.to_double();
					BoxBType boxb(b);
					return distance(f, boxb) <= _src.distance_threshold;
				};
				break;
			}
		}

		template <typename BoxAType, typename BoxAPrimitive> void implement_string_b_numeric_comparison(filter_element& _src)
		{
			switch (_src.comparison) {
			case filter_comparison_types::eq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f = boxb.to_double();
					return boxa == f;
				};
				break;
			case filter_comparison_types::ls:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f = boxb.to_double();
					return boxa < f;
				};
				break;
			case filter_comparison_types::gt:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f = boxb.to_double();
					return boxa > f;
				};
				break;
			case filter_comparison_types::lseq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f = boxb.to_double();
					return boxa <= f;
				};
				break;
			case filter_comparison_types::gteq:
				_src.compare = [](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f = boxb.to_double();
					return boxa >= f;
				};
				break;
			case filter_comparison_types::distance:
				_src.compare = [_src](char* a, char* b) {
					BoxAType boxa(a);
					string_box boxb = string_box::get(b);
					BoxAPrimitive f1 = (double)boxa.get_value();
					BoxAPrimitive f2 = boxb.to_double();
					return distance(f1, f2) <= _src.distance_threshold;
				};
				break;
			}
		}

		bool jslice::set_projection(projection_element_collection& _src)
		{
			for (int i = 0; i < _src.size(); i++)
			{
				auto sc = _src[i];
				row_id_type fid = get_field_index_by_id(sc.field_id);
				auto fld_dest = get_field(fid);
				sc.field_offset = get_offset(fid);
				sc.field_type = fld_dest.type_id;
			}
			return true;
		}

		bool jslice::set_filters(filter_element* _srcz, int _count, jslice& _parameters)
		{
			for (int i = 0; i < _count; i++)
			{
				auto& _src = _srcz[i];
				row_id_type fip = _parameters.get_field_index_by_id(_src.parameter_field_id);
				row_id_type fid = get_field_index_by_id(_src.parameter_field_id);
				auto fld_param = _parameters.get_field(fip);
				auto fld_dest = get_field(fid);

				_src.parameter_offset = get_offset(fip);
				_src.target_offset = _parameters.get_offset(fid);

				if (fld_param.is_int64() && fld_dest.is_int8())
				{
					implement_comparison<int64_box, int8_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int16())
				{
					implement_comparison<int64_box, int16_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int32())
				{
					implement_comparison<int64_box, int32_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int64())
				{
					implement_comparison<int64_box, int64_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int8())
				{
					implement_comparison<int32_box, int8_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int16())
				{
					implement_comparison<int32_box, int16_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int32())
				{
					implement_comparison<int32_box, int32_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int64())
				{
					implement_comparison<int32_box, int64_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int8())
				{
					implement_comparison<int16_box, int8_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int16())
				{
					implement_comparison<int16_box, int16_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int32())
				{
					implement_comparison<int16_box, int32_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int64())
				{
					implement_comparison<int16_box, int64_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int8())
				{
					implement_comparison<int8_box, int8_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int16())
				{
					implement_comparison<int8_box, int16_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int32())
				{
					implement_comparison<int8_box, int32_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int64())
				{
					implement_comparison<int8_box, int64_box>(_src);
				}
				else if (fld_param.is_float64() && fld_dest.is_float32())
				{
					implement_comparison<double_box, float_box>(_src);
				}
				else if (fld_param.is_float32() && fld_dest.is_float64())
				{
					implement_comparison<float_box, double_box>(_src);
				}
				else if (fld_param.is_float32() && fld_dest.is_float32())
				{
					implement_comparison<float_box, float_box>(_src);
				}
				else if (fld_param.is_float64() && fld_dest.is_float64())
				{
					implement_comparison<double_box, double_box>(_src);
				}
				else if (fld_param.is_float32() && fld_dest.is_string())
				{
					implement_string_b_numeric_comparison<float_box, float>(_src);
				}
				else if (fld_param.is_float64() && fld_dest.is_string())
				{
					implement_string_b_numeric_comparison<double_box, double>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_float32())
				{
					implement_string_a_numeric_comparison<float_box, float>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_float64())
				{
					implement_string_a_numeric_comparison<double_box, double>(_src);
				}
				else if (fld_param.is_point() && fld_dest.is_point())
				{
					implement_comparison<point_box, point_box>(_src);
				}
				else if (fld_param.is_datetime() && fld_dest.is_datetime())
				{
					implement_comparison<time_box, time_box>(_src);
				}
				else
				{
					implement_pointer_comparison(_src);
				}
			}
			return true;
		}

		template <typename BoxAType, typename BoxBType> void implement_update(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				BoxAType boxa(a);
				BoxBType boxb(b);
				boxa = boxb.value();
			};
		}

		template <typename BoxType> void implement_update_number_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				BoxType boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_number();
				if (result.success) {
					boxa = result.value;
				}
			};
		}

		void implement_update_color_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				color_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_color();
				if (result.success) {
					color c;
					c.alpha = result.alpha / 256.0;
					c.blue = result.blue / 256.0;
					c.green = result.green / 256.0;
					c.red = result.red / 256.0;
					boxa = c;
				}
			};
		}

		void implement_update_point_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				point_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_point();
				if (result.success) {
					point c;
					c.x = result.x;
					c.y = result.y;
					boxa = c;
				}
			};
		}

		void implement_update_rectangle_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				rectangle_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_rectangle();
				if (result.success) {
					rectangle c;
					c.x = result.x;
					c.y = result.y;
					c.w = result.w;
					c.h = result.h;
					boxa = c;
				}
			};
		}

		void implement_update_image_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				image_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_rectangle();
				if (result.success) {
					image_instance c;
					c.source.x = result.x;
					c.source.y = result.y;
					c.source.w = result.w;
					c.source.h = result.h;
					boxa = c;
				}
			};
		}

		void implement_update_midi_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				midi_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_audio();
				if (result.success) {
					midi_instance c;
					c.start_seconds = result.start_seconds;
					c.stop_seconds = result.stop_seconds;
					c.pitch_adjust = result.pitch_adjust;
					c.volume_adjust = result.volume_adjust;
					c.playing = result.playing;
					boxa = c;
				}
			};
		}

		void implement_update_wave_from_string(update_element& _src)
		{
			_src.assignment = [_src](char* a, char* b) {
				wave_box boxa(a);
				string_box boxb(b);
				system::string_extractor extractor(boxb, 100, nullptr);
				auto result = extractor.get_audio();
				if (result.success) {
					wave_instance c;
					c.start_seconds = result.start_seconds;
					c.stop_seconds = result.stop_seconds;
					c.pitch_adjust = result.pitch_adjust;
					c.volume_adjust = result.volume_adjust;
					c.playing = result.playing;
					boxa = c;
				}
			};
		}

		bool jslice::set_updates(update_element_collection& _srcx, jslice& _parameters)
		{
			for (int i = 0; i < _srcx.size(); i++)
			{
				auto& _src = _srcx[i];
				row_id_type fip = _parameters.get_field_index_by_id(_src.parameter_field_id);
				row_id_type fid = get_field_index_by_id(_src.parameter_field_id);
				auto fld_param = _parameters.get_field(fip);
				auto fld_dest = get_field(fid);

				_src.parameter_offset = get_offset(fip);
				_src.target_offset = _parameters.get_offset(fid);

				if (fld_param.is_int64() && fld_dest.is_int8())
				{
					implement_update<int64_box, int8_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int16())
				{
					implement_update<int64_box, int16_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int32())
				{
					implement_update<int64_box, int32_box>(_src);
				}
				else if (fld_param.is_int64() && fld_dest.is_int64())
				{
					implement_update<int64_box, int64_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int8())
				{
					implement_update<int32_box, int8_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int16())
				{
					implement_update<int32_box, int16_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int32())
				{
					implement_update<int32_box, int32_box>(_src);
				}
				else if (fld_param.is_int32() && fld_dest.is_int64())
				{
					implement_update<int32_box, int64_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int8())
				{
					implement_update<int16_box, int8_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int16())
				{
					implement_update<int16_box, int16_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int32())
				{
					implement_update<int16_box, int32_box>(_src);
				}
				else if (fld_param.is_int16() && fld_dest.is_int64())
				{
					implement_update<int16_box, int64_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int8())
				{
					implement_update<int8_box, int8_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int16())
				{
					implement_update<int8_box, int16_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int32())
				{
					implement_update<int8_box, int32_box>(_src);
				}
				else if (fld_param.is_int8() && fld_dest.is_int64())
				{
					implement_update<int8_box, int64_box>(_src);
				}

				else if (fld_param.is_string() && fld_dest.is_int8())
				{
					implement_update_number_from_string<int8_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_int16())
				{
					implement_update_number_from_string<int16_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_int32())
				{
					implement_update_number_from_string<int32_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_int64())
				{
					implement_update_number_from_string<int64_box>(_src);
				}

				else if (fld_param.is_float64() && fld_dest.is_float32())
				{
					implement_update<double_box, float_box>(_src);
				}
				else if (fld_param.is_float32() && fld_dest.is_float64())
				{
					implement_update<float_box, double_box>(_src);
				}
				else if (fld_param.is_float32() && fld_dest.is_float32())
				{
					implement_update<float_box, float_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_float32())
				{
					implement_update_number_from_string<float_box>(_src);
				}
				else if (fld_param.is_float64() && fld_dest.is_float64())
				{
					implement_update<double_box, double_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_float64())
				{
					implement_update_number_from_string<double_box>(_src);
				}



				else if (fld_param.is_point() && fld_dest.is_point())
				{
					implement_update<point_box, point_box>(_src);
				}
				else if (fld_param.is_color() && fld_dest.is_color())
				{
					implement_update<color_box, color_box>(_src);
				}
				else if (fld_param.is_rectangle() && fld_dest.is_rectangle())
				{
					implement_update<rectangle_box, rectangle_box>(_src);
				}
				else if (fld_param.is_image() && fld_dest.is_image())
				{
					implement_update<image_box, image_box>(_src);
				}
				else if (fld_param.is_wave() && fld_dest.is_wave())
				{
					implement_update<wave_box, wave_box>(_src);
				}
				else if (fld_param.is_midi() && fld_dest.is_midi())
				{
					implement_update<midi_box, midi_box>(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_point())
				{
					implement_update_point_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_color())
				{
					implement_update_color_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_rectangle())
				{
					implement_update_rectangle_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_image())
				{
					implement_update_image_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_wave())
				{
					implement_update_wave_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_midi())
				{
					implement_update_midi_from_string(_src);
				}
				else if (fld_param.is_string() && fld_dest.is_string())
				{
					implement_update<string_box, string_box>(_src);
				}
			}
			return true;
		}

		bool jslice::set_filters(filter_element_collection& _srcz, jslice& _parameters)
		{
			return set_filters(_srcz.get_ptr(0), _srcz.size(), _parameters);
		}

		bool jslice::filter(filter_element* _src, int _count, jslice& _parameters)
		{
			for (int i = 0; i < _count; i++)
			{
				if (_src->compare)
				{
					bool result = _src->compare(get_bytes() + _src->target_offset, _parameters.bytes + _src->parameter_offset);
					if (!result) {
						return false;
					}
				}
				_src++;
			}
			return true;
		}

		bool jslice::filter(filter_element_collection& _srcc, jslice& _parameters)
		{
			for (int i = 0; i < _srcc.size(); i++)
			{
				auto& _src = _srcc[i];
				if (_src.compare) 
				{
					bool result = _src.compare(get_bytes() + _src.target_offset, _parameters.bytes + _src.parameter_offset);
					if (!result) {
						return false;
					}
				}
			}
			return true;
		}

		bool jslice::update(update_element_collection& _srcc, jslice& _parameters)
		{
			for (int i = 0; i < _srcc.size(); i++)
			{
				auto& _src = _srcc[i];
				if (_src.assignment)
				{
					_src.assignment(get_bytes() + _src.target_offset, _parameters.bytes + _src.parameter_offset);
				}
			}
			return true;
		}

		std::partial_ordering jslice::compare(projection_element_collection& collection, jslice& _dest_slice)
		{
			for (int i = 0; i < collection.size(); i++)
			{
				auto& src = collection[i];
				if (src.projection == projection_operations::group_by) {
					char* this_bytes = src.field_offset + get_bytes();
					char* dest_bytes = src.field_offset + _dest_slice.get_bytes();

					auto result = compare_express(src.field_type, this_bytes, dest_bytes);
					if (result != std::strong_ordering::equal)
					{
						return result;
					}
				}
			}
			return std::strong_ordering::equal;
		}

		jarray::jarray() : schema(nullptr), class_field_id(null_row), bytes(nullptr)
		{
			;
		}

		jarray::jarray(jslice *_parent, jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init) : item(_parent), schema(_schema), class_field_id(_class_field_id), bytes(_bytes)
		{
			if (_init) {
				for (auto jai : *this)
				{
					jai.construct();
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

		jslice jarray::get_slice(int x, int y, int z)
		{
			dimensions_type dims;
			dims.x = x;
			dims.y = y;
			dims.z = z;
			return get_slice(dims);
		}

		jslice jarray::get_slice(dimensions_type pos)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((pos.x >= dim.x) ||
				(pos.y >= dim.y) ||
				(pos.z >= dim.z)) {
					return jslice(item, schema, field.object_properties.class_id, nullptr, dim);
			}
			char* b = &bytes[ ((pos.z * dim.y * dim.x) + (pos.y * dim.x) + pos.x ) * field.object_properties.class_size_bytes ];
			jslice slice(item, schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		uint64_t jarray::get_size_bytes()
		{
			jfield& field = schema->get_field(class_field_id);
			return field.size_bytes;
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

		jlist::jlist(jslice *_parent, jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init) 
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
				array_box<row_id_type>::create(&model_box, field_def.object_properties.dim.x, data.instance->selection_offset);
				array_box<row_id_type>::create(&model_box, field_def.object_properties.dim.x, data.instance->sort_offset);
				data.instance->slice_offset = model_box.reserve(0);
				data.instance->allocated = 0;
			}
			else
			{
				data.instance = model_box.unpack<jlist_instance>(0);
			}

			data.list_bytes = model_box.unpack<char>(data.instance->slice_offset);
			data.selections = array_box<row_id_type>::get(&model_box, data.instance->selection_offset);
			data.sort_order = array_box<row_id_type>::get(&model_box, data.instance->sort_offset);
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
			data.selections = array_box<row_id_type>::get(&model_box, data.instance->selection_offset);
			data.sort_order = array_box<row_id_type>::get(&model_box, data.instance->sort_offset);
			item = _src.item;
		}

		uint32_t jlist::capacity()
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim.x;
		}

		uint32_t jlist::size()
		{
			return data.instance->allocated;
		}

		jslice jlist::get_slice(int idx)
		{
			idx = data.sort_order[idx];
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= data.instance->allocated) || (idx < 0)) {
				return jslice(item, schema, field.object_properties.class_id, nullptr, dim);
			}
			dimensions_type pos = { idx, 0, 0 };
			char* b = &data.list_bytes[idx * field.object_properties.class_size_bytes];
			jslice slice(item, schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		jslice jlist::get_slice_direct(int idx)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= data.instance->allocated) || (idx < 0)) {
				return jslice(item, schema, field.object_properties.class_id, nullptr, dim);
			}
			dimensions_type pos = { idx, 0, 0 };
			char* b = &data.list_bytes[idx * field.object_properties.class_size_bytes];
			jslice slice(item, schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		bool jlist::erase_slice(int idx)
		{
			int oidx = idx;
			idx = data.sort_order[idx];
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
				for (int i = oidx; i < data.instance->allocated-1; i++)
				{
					data.sort_order[i] = data.sort_order[i + 1];
				}
				for (int i = 0; i < data.instance->allocated; i++) {
					if (data.sort_order[i] > idx) {
						data.sort_order[i]--;
					}
				}
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

		jslice jlist::append_slice()
		{
			if (data.instance->allocated < capacity()) {
				auto index = data.instance->allocated;
				data.sort_order.push_back(index);
				data.instance->allocated++;
				jslice new_slice = get_slice(index);
				new_slice.construct();
				return new_slice;
			}
			return get_slice(-1);
		}

		bool jlist::select_slice(int idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			idx = data.sort_order[idx];
			data.selections[idx] = 1;
			return true;
		}

		bool jlist::deselect_slice(int idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			idx = data.sort_order[idx];
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

		void jlist::sort(projection_element_collection& projections)
		{
			if (size()) 
			{
				jslice slice = get_slice(0);
				slice.set_projection(projections);			
				projection_element_collection* pprojection = &projections;

				data.sort_order.sort([this, pprojection](row_id_type& a, row_id_type& b)
					{
						jslice aslice = this->get_slice_direct(a);
						jslice bslice = this->get_slice_direct(b);

						auto compare_result = aslice.compare(*pprojection, bslice);

						return compare_result == std::partial_ordering::less;
					});
			}
		}

		void jschema::add_standard_fields() 
		{
			put_string_field_request string_fields[33] = {
				{ { null_row, jtype::type_string , "fullName", "Full Name" }, { 75, "", "" } },
				{ { null_row, jtype::type_string , "firstName", "First Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string , "lastName", "Last Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string , "middleName", "Middle Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string , "ssn", "SSN" }, { 10, "", "" }},
				{ { null_row, jtype::type_string, "email", "eEmail" }, { 200, "", ""  }},
				{ { null_row, jtype::type_string, "title", "Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "street", "Street" },{  200, "", "" } },
				{ { null_row, jtype::type_string, "suiteapt", "Suite/Apt" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "city", "City" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "state", "State" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "postal", "Postal Code" }, { 50, "", ""  }},
				{ { null_row, jtype::type_string, "countryName", "Country Name" }, { 50, "", "" } },
				{ { null_row, jtype::type_string, "countryCode", "Country Code" }, { 3, "", ""  }},
				{ { null_row, jtype::type_string, "institutionName", "Institution Name" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "longName", "Long Name" }, { 200, "", ""  }},
				{ { null_row, jtype::type_string, "shortName", "Short Name" },{  50, "", ""  }},
				{ { null_row, jtype::type_string, "unit", "Unit" }, { 10, "", "" } },
				{ { null_row, jtype::type_string, "symbol", "Symbol" }, { 10, "", "" } },
				{ { null_row, jtype::type_string, "operator", "Operator" }, { 10, "", ""  }},
				{ { null_row, jtype::type_string, "windowsPath", "Windows Path" }, { 512, "", ""  }},
				{ { null_row, jtype::type_string, "linuxPath", "Linux Path" }, { 512, "", "" } },
				{ { null_row, jtype::type_string, "url", "Url" }, { 512, "", "" } },
				{ { null_row, jtype::type_string, "userName", "User Name" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "passWord", "Password" }, { 100, "", ""  }},
				{ { null_row, jtype::type_string, "docTitle", "Document Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "sectionTitle", "Section Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "blockTitle", "Block Title" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "caption", "Caption" }, { 200, "", "" } },
				{ { null_row, jtype::type_string, "paragraph", "Paragraph" }, { 4000, "", "" } },
				{ { null_row, jtype::type_string, "mimeType", "MimeType" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "base64", "Base64" }, { 100, "", "" } },
				{ { null_row, jtype::type_string, "fileName", "fileName" }, { 512, "", "" } }
			};

			put_time_field_request time_fields[2] = {
				{ { null_row, jtype::type_datetime, "birthday", "Birthday" }, 0, INT64_MAX },
				{ { null_row, jtype::type_datetime, "scheduled", "Scheduled" }, 0, INT64_MAX },
			};

			put_integer_field_request int_fields[1] = {
				{ { null_row, jtype::type_int64, "count", "Count" }, 0, INT64_MAX },
			};

			put_double_field_request double_fields[13] = {
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
			};

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

			put_color_field({ null_row, jtype::type_color, "color", "color" });

			put_point_field({ null_row, jtype::type_point, "point", "point" });
			put_point_field({ null_row, jtype::type_point, "position_point", "position_point" });
			put_point_field({ null_row, jtype::type_point, "selection_point", "selection_point" });

			put_rectangle_field({ null_row, jtype::type_rectangle, "rectangle", "rectangle" });
			put_rectangle_field({ null_row, jtype::type_rectangle, "position_rectangle", "position_rectangle" });
			put_rectangle_field({ null_row, jtype::type_rectangle, "selection_rectangle", "selection_rectangle" });

		}

		bool schema_tests()
		{
			try {
				dynamic_box box;
				box.init(1 << 21);

				jschema schema;
				row_id_type schema_id;

				schema = jschema::create_schema( & box, 20, true, schema_id);

				row_id_type quantity_field_id = null_row;
				row_id_type last_name_field_id = null_row;
				row_id_type first_name_field_id = null_row;
				row_id_type birthday_field_id = null_row;
				row_id_type count_field_id = null_row;
				row_id_type title_field_id = null_row;
				row_id_type institution_field_id = null_row;

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

				row_id_type failed_field_id = schema.find_field("badFieldName");

				if (failed_field_id != null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				countrybit::database::put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, title_field_id, count_field_id, quantity_field_id };
				row_id_type person_class_id = schema.put_class(person);

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

				countrybit::database::put_class_request company;
				company.class_name = "company";
				company.class_description = "a company is a collection of people";
				company.member_fields = { institution_field_id, member_field(person_class_id, dimensions_type { 10, 1, 1 }) };
				row_id_type company_class_id = schema.put_class(company);

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
				row_id_type schema_id;

				schema = jschema::create_schema(&box, 20, true, schema_id);

				row_id_type quantity_field_id = null_row;
				row_id_type last_name_field_id = null_row;
				row_id_type first_name_field_id = null_row;
				row_id_type birthday_field_id = null_row;
				row_id_type count_field_id = null_row;
				row_id_type title_field_id = null_row;
				row_id_type institution_field_id = null_row;

				schema.bind_field("quantity", quantity_field_id);
				schema.bind_field("lastName", last_name_field_id);
				schema.bind_field("firstName", first_name_field_id);
				schema.bind_field("birthday", birthday_field_id);
				schema.bind_field("count", count_field_id);
				schema.bind_field("title", title_field_id);
				schema.bind_field("institutionName", institution_field_id);

				jmodel sample_model;
				sample_model.model_name = "my model";

				schema.put_model(sample_model);

				jcollection_ref ref;
				ref.data = &box;
				ref.model_name = sample_model.model_name;
				ref.max_actors = 2;
				ref.max_objects = 50;

				init_collection_id(ref.collection_id);

				jcollection people = schema.create_collection(&ref, nullptr);

				jactor sample_actor;
				sample_actor.actor_name = "sample actor";
				sample_actor.actor_id = null_row;
				sample_actor = people.create_actor(sample_actor);

				countrybit::database::put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, count_field_id, quantity_field_id };
				row_id_type person_class_id = schema.put_class(person);

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

				row_id_type people_object_id;

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
					last_name = sl.get_string(0);
					if (!last_name.starts_with("last")) {
						std::cout << __LINE__ << ":last name failed" << std::endl;
						return false;
					}
					first_name = sl.get_string(1);
					if (!first_name.starts_with("first")) {
						std::cout << __LINE__ << ":first name failed" << std::endl;
						return false;
					}
					birthday = sl.get_time(2);
					count = sl.get_int64(3);
					qty = sl.get_double(4);

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
				row_id_type schema_id;

				schema = jschema::create_schema(&box, 50, true, schema_id);

				countrybit::database::put_class_request sprite_frame_request;

				sprite_frame_request.class_name = "spriteframe";
				sprite_frame_request.class_description = "sprite frame";
				sprite_frame_request.member_fields = { "shortName", "rectangle", "color" };
				row_id_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

				if (sprite_frame_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				countrybit::database::put_class_request sprite_class_request;
				sprite_class_request.class_name = "sprite";
				sprite_class_request.class_description = "sprite";
				sprite_class_request.member_fields = { "shortName", "rectangle", member_field(sprite_frame_class_id, { 10, 10, 1 }) };
				row_id_type sprite_class_id = schema.put_class(sprite_class_request);

				if (sprite_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				collection_id_type colid;

				init_collection_id(colid);

				row_id_type classesb[2] = { sprite_class_id, null_row };

				model_type sprite_model;

				sprite_model.model_name = "sprite model";
				schema.put_model(sprite_model);

				jcollection_ref ref;
				ref.data = &box;
				ref.model_name = sprite_model.model_name;
				ref.max_actors = 2;
				ref.max_objects = 50;

				init_collection_id(ref.collection_id);

				jcollection sprites = schema.create_collection(&ref, nullptr);

				actor_type sprite_boy;
				sprite_boy.actor_id = null_row;
				sprite_boy.actor_name = "sprite_boy";

				sprite_boy = sprites.create_actor(sprite_boy);
				row_id_type new_sprite_id;

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
						auto dim = frame.get_dim();
						auto frame_name = frame.get_string(0);
						auto frame_rect = frame.get_rectangle(1);
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
					auto image_name = slice.get_string(0);
					auto image_rect = slice.get_rectangle(1);

#if _DETAIL
					std::cout << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

					if (image_rect->w != 1000 || image_rect->h != 1000) {

						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}

					auto frames = slice.get_object(2);

					int fcount = 0;
					for (auto frame : frames)
					{
						auto image_rect = slice.get_rectangle(1);
						if (image_rect->w != 1000 || image_rect->h != 1000)
						{
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						}

						auto frame_rect = frame.get_rectangle(1);
						if (frame_rect->w != 100 || frame_rect->h != 100)
						{
							std::cout << __LINE__ << ":array failed" << std::endl;
							return false;
						}

						//					std::cout << std::format("{} {}x{} - {}x{}", frame.get_string(0).value(), frame.get_float(1).value(), frame.get_float(2).value(), frame.get_float(3).value(), frame.get_float(4).value()) << std::endl;
						auto dim = frame.get_dim();
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
				row_id_type schema_id;

				schema = jschema::create_schema(&box, 50, true, schema_id);

				put_double_field_request dfr;
				dfr.name.name = "limit";
				dfr.name.description = "Maximum amount paid by policy";
				dfr.name.type_id = jtype::type_float64;
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				row_id_type limit_field_id = schema.put_double_field(dfr);

				dfr.name.name = "attachment";
				dfr.name.description = "Point at which policy begins coverage";
				dfr.name.type_id = jtype::type_float64; 
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				row_id_type attachment_field_id = schema.put_double_field(dfr);

				dfr.name.name = "deductible";
				dfr.name.description = "Point at which policy begins paying";
				dfr.name.type_id = jtype::type_float64; 
				dfr.options.minimum_double = 0.0;
				dfr.options.maximum_double = 1E10;
				row_id_type deductible_field_id = schema.put_double_field(dfr);

				put_string_field_request sfr;
				sfr.name.name = "comment";
				sfr.name.description = "Descriptive text";
				sfr.options.length = 512;
				row_id_type comment_field_id = schema.put_string_field(sfr);

				sfr.name.name = "program_name";
				sfr.name.description = "name of a program";
				sfr.options.length = 200;
				row_id_type program_name_field_id = schema.put_string_field(sfr);

				sfr.name.name = "program_description";
				sfr.name.description = "name of a program";
				sfr.options.length = 512;
				row_id_type program_description_field_id = schema.put_string_field(sfr);

				sfr.name.name = "coverage_name";
				sfr.name.description = "name of a coverage";
				sfr.options.length = 200;
				row_id_type coverage_field_id = schema.put_string_field(sfr);

				sfr.name.name = "carrier_name";
				sfr.name.description = "name of a carrier";
				sfr.options.length = 200;
				row_id_type carrier_field_id = schema.put_string_field(sfr);

				countrybit::database::put_class_request pcr;

				pcr.class_name = "program";
				pcr.class_description = "program summary";
				pcr.member_fields = { "program_name", "program_description" };
				row_id_type program_class_id = schema.put_class(pcr);

				if (program_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "coverage";
				pcr.class_description = "coverage frame";
				pcr.member_fields = { "coverage_name", "comment", "rectangle" };
				row_id_type coverage_class_id = schema.put_class(pcr);

				if (coverage_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "coverage_spacer";
				pcr.class_description = "spacer frame";
				pcr.member_fields = { "rectangle" };
				row_id_type coverage_spacer_id = schema.put_class(pcr);

				if (coverage_spacer_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "carrier";
				pcr.class_description = "carrier frame";
				pcr.member_fields = { "carrier_name", "comment", "rectangle", "color" };
				row_id_type carrier_class_id = schema.put_class(pcr);

				if (coverage_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy";
				pcr.class_description = "policy block";
				pcr.member_fields = { "coverage_name", "carrier_name", "comment", "rectangle", "color", "limit", "attachment" };
				row_id_type policy_class_id = schema.put_class(pcr);

				if (policy_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy_deductible";
				pcr.class_description = "deductible block";
				pcr.member_fields = { "coverage_name", "comment", "rectangle", "color", "deductible" };
				row_id_type policy_deductible_class_id = schema.put_class(pcr);

				if (policy_deductible_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				pcr.class_name = "policy_umbrella";
				pcr.class_description = "deductible block";
				pcr.member_fields = { "comment", "rectangle", "color", "limit", "attachment"};
				row_id_type policy_umbrella_class_id = schema.put_class(pcr);

				if (policy_deductible_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				jmodel jm;

				jm.model_name = "program_chart";

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
				ref.model_name = jm.model_name;
				ref.max_actors = 2;
				ref.max_objects = 100;

				if (!init_collection_id(ref.collection_id))
				{
					std::cout << __LINE__ << "collection id failed" << std::endl;
				}

				jcollection program_chart = schema.create_collection(&ref, nullptr);

				jactor sample_actor;
				sample_actor.actor_name = "sample actor";
				sample_actor.actor_id = null_row;
				sample_actor = program_chart.create_actor(sample_actor);

				auto result = program_chart.get_command_result(sample_actor.actor_id);

				

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
