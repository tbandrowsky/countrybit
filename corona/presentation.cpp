
#include "corona.h"

namespace corona
{
	namespace database
	{

		measure operator ""_px(long double px)
		{
			return measure(px, measure_units::pixels );
		}

		measure operator ""_pct(long double pct)
		{
			return measure( pct, measure_units::percent );
		}

		page::page()
		{
			data.init(1 << 20);
		}

		void page::clear()
		{
			base_type::clear();
			data.init(1 << 20);
		}

		page_item* page::row(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::row;
			v->box = _box;
			return v;
		}

		page_item* page::column(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::column;
			v->box = _box;
			return v;
		}

		page_item* page::absolute(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::absolute;
			v->box = _box;
			return v;
		}

		page_item* page::canvas2d(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::canvas2d;
			v->box = _box;
			v->canvas_id = v->id;
			return v;
		}

		page_item* page::h1(page_item* _parent, const char* _text, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::h1;
			v->field = nullptr;
			v->caption = data.copy(_text, 0);
			v->box = _box;
			return v;
		}

		page_item* page::h2(page_item* _parent, const char* _text, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::h2;
			v->caption = data.copy(_text, 0);
			v->box = _box;
			return v;
		}

		page_item* page::h3(page_item* _parent, const char* _text, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::h3;
			v->caption = data.copy(_text, 0);
			v->box = _box;
			return v;
		}

		page_item* page::space(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::space;
			v->box = _box;
			return v;
		}

		page_item* page::field(page_item* _parent, int object_id, int field_id, jslice slice)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::column;
			v->set_parent(_parent);
			v->object_id = object_id;
			v->box = { 0.0_pct, 0.0_pct, 100.0_pct, 50.0_px };
			v->slice = slice;
			return v;
		}

		page_item* page::actor_update_fields(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection)
		{
			page_item* v = append();

			v->id = size();
			v->layout = layout_types::column;
			v->set_parent(_parent);

			measure height = 0.0_px;

			if (_state->modified_object_id != null_row)
			{
				auto avo = _state->get_modified_object();
				auto slice = _collection->get_object(avo.object_id);
				for (int i = 0; i < slice.size(); i++)
				{
					jfield& fld = slice.get_field(i);
					page_item* label = append();
					label->id = size();
					label->parent_id = v->id;
					label->layout = layout_types::label;
					label->field = &fld;
					label->box = { 0.0_px, 0.0_px, 200.0_px, 20.0_px };
					label->slice = slice;
					label->object_id = avo.object_id;
					height.amount += 20.0;
					page_item* control = append();
					control->id = size();
					control->parent_id = v->id;
					control->layout = layout_types::field;
					control->field = &fld;
					control->box = { 0.0_px, 0.0_px, 200.0_px, 20.0_px };
					control->slice = slice;
					control->object_id = avo.object_id;
					height.amount += 20.0;
				}
			}

			v->box = { 10.0_px, 10.0_px, 200.0_px, height };

			return v;
		}

		page_item* page::actor_create_buttons(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::column;
			v->set_parent(_parent);

			measure height = 0.0_px;

			for (auto aco : _state->create_objects)
			{
				page_item* button = append();
				button->parent_id = v->id;
				button->id = size();
				button->layout = layout_types::create;
				button->box = { 0.0_px, 0.0_px, 200.0_px, 20.0_px };
				button->create_request = _state->create_create_request(aco.second.class_id);
				height.amount += 20.0;				

				object_description desc;
				desc = "Add " + _schema->get_class(aco.second.class_id).pitem()->name;
				button->caption = data.copy<char>(desc.c_str(), 0);
			}

			v->box = { 10.0_px, 10.0_px, 200.0_px, height };
			return v;
		}

		page_item* page::actor_select_items(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection)
		{
			for (auto st : _state->view_objects) 
			{
				page_item* v = append();
				v->id = size();
				v->set_parent(_parent);
				v->layout = layout_types::select;
				v->object_id = st.second.object_id;
				v->box.height.units = measure_units::pixels;
				v->box.width.units = measure_units::pixels;
				v->box.x.units = measure_units::pixels;
				v->box.y.units = measure_units::pixels;
				auto slice = _collection->get_object(st.second.object_id);
				auto rf = slice.get_rectangle("rectangle");
				v->box.x.amount = rf->x;
				v->box.y.amount = rf->y;
				v->box.width.amount = rf->w;
				v->box.height.amount = rf->h;
				v->select_request = _state->create_select_request(v->object_id, false);
			}
			return _parent;
		}

		void page::visit_impl(page_item *r, std::function<bool(page_item* _item)> fn)
		{
			fn(r);
			auto children = where([r](const auto& it) {
				return it.item.parent_id == r->id;
				});
			for (auto child : children)
			{
				visit_impl(&child.item, fn);
			}
		}

		void page::arrange_impl(page_item* _item, double offx, double offy, double x, double y, double width, double height)
		{
			if (_item->box.x.units == measure_units::percent)
				_item->bounds.x = _item->box.x.amount * width / 100.0 + x;
			else
				_item->bounds.x = _item->box.x.amount + x + offx;

			if (_item->box.y.units == measure_units::percent)
				_item->bounds.y = _item->box.y.amount * height / 100.0 + y;
			else
				_item->bounds.y = _item->box.y.amount + y + offy;

			if (_item->box.height.units == measure_units::percent)
				_item->bounds.h = _item->box.height.amount * height / 100.0;
			else
				_item->bounds.h = _item->box.height.amount;

			if (_item->box.width.units == measure_units::percent)
				_item->bounds.w = _item->box.width.amount * width / 100.0;
			else
				_item->bounds.w = _item->box.width.amount;

			std::cout << std::format("{},{} bounds {},{},{},{} canvas {}, is_draw {}", _item->parent_id, _item->id, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h, _item->canvas_id, _item->is_drawable()) << std::endl;

			auto children = where([_item](const auto& it) {
				return it.item.parent_id == _item->id;
				});

			if (_item->layout == layout_types::row) 
			{
				double startx = x;
				for (auto child : children)
				{
					arrange_impl(&child.item, startx, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
					startx += (child.item.bounds.w);
				}
			}
			else if (_item->layout == layout_types::column) 
			{
				double starty = y;
				for (auto child : children)
				{
					arrange_impl(&child.item, 0, starty, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
					starty += (child.item.bounds.h);
				}
			}
			else if (_item->layout == layout_types::canvas2d)
			{
				for (auto child : children)
				{
					arrange_impl(&child.item, 0, 0, 0, 0, _item->bounds.w, _item->bounds.h);
				}
			}
			else
			{
				for (auto child : children)
				{
					arrange_impl(&child.item, 0, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
				}
			}
		}

		void page::arrange(double width, double height)
		{
			sort([](auto& a, auto& b) {
				return (a.parent_id < b.parent_id);
				});
			for (auto pi : *this)
			{
				if (pi.item.parent_id < 0) {
					arrange_impl(&pi.item, 0, 0, 0, 0, width, height);
				}
				else 
				{
					break;
				}
			}
		}

		void page::visit(std::function<bool(page_item* _item)> fn)
		{
			sort([](auto& a, auto& b) {
				return (a.parent_id < b.parent_id);
				});
			for (auto pi : *this)
			{
				if (pi.item.parent_id < 0) {
					visit_impl(&pi.item, fn);
				}
				else
				{
					break;
				}
			}
		}


	}
}
