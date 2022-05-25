
#include "pch.h"

namespace corona
{
	namespace database
	{

		measure operator ""px(long double px)
		{
			return measure{ px, measure_units::pixels };
		}

		measure operator ""pct(long double pct)
		{
			return measure{ pct, measure_units::percent };
		}

		page_item* page::row(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->layout = layout_types::row;
			v->field_id = null_row;
			v->object_id = null_row;
			v->box = _box;
			return v;
		}

		page_item* page::column(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->layout = layout_types::column;
			v->field_id = null_row;
			v->object_id = null_row;
			v->box = _box;
			return v;
		}

		page_item* page::absolute(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->layout = layout_types::absolute;
			v->field_id = null_row;
			v->object_id = null_row;
			v->box = _box;
			return v;
		}

		page_item* page::canvas2d(page_item* _parent, measure_box _box)
		{
			page_item* v = append();
			v->id = size();
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->layout = layout_types::canvas2d;
			v->field_id = null_row;
			v->object_id = null_row;
			v->box = _box;
			return v;
		}

		page_item* page::field(page_item* _parent, int object_id, int field_id)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::field;
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->field_id = field_id;
			v->object_id = object_id;
			v->box = { 0.0pct, 0.0pct, 100.0pct, 50.0px };
			return v;
		}

		page_item* page::slice(page_item* _parent, int object_id, jslice& slice)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::column;
			if (_parent)
				v->parent_id = _parent->id;
			else
				v->parent_id = -1;
			v->field_id = null_row;
			v->object_id = null_row;
			v->box = { 0.0pct, 0.0pct, 200.0px, 100.0pct };

			return v;
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

			auto children = where([_item](const auto& it) {
				return it.item.parent_id == _item->id;
				});

			if (_item->layout == layout_types::row) 
			{
				double startx = x;
				for (auto child : children)
				{
					arrange_impl(&child.item, startx, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
					startx += (child.item.bounds.x + child.item.bounds.w);
				}
			}
			else if (_item->layout == layout_types::column) 
			{
				double starty = y;
				for (auto child : children)
				{
					arrange_impl(&child.item, 0, starty, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
					starty += (child.item.bounds.y + child.item.bounds.h);
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

		page_item* page::actor_update_fields(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection)
		{
			;
		}

		page_item* page::actor_create_buttons(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection)
		{
			;
		}

		page_item* page::actor_select_buttons(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection)
		{
			;
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
