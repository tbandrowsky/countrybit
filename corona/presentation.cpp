
#include "corona.h"

#define TRACE_LAYOUT 0

namespace corona
{
	namespace database
	{

		page::page()
		{
			data.init(1 << 20);
		}

		void page::clear()
		{
			base_type::clear();
			data.init(1 << 20);
		}

		page_item* page::row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::row;
			v->box = _box;
			v->style_id = _style_id;
			v->item_space = _item_space;
			return v;
		}

		page_item* page::column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::column;
			v->box = _box;
			v->style_id = _style_id;
			v->item_space = _item_space;
			return v;
		}

		page_item* page::absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::absolute;
			v->box = _box;
			v->style_id = _style_id;
			return v;
		}

		page_item* page::canvas2d_row(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px })
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::canvas2d_row;
			v->box = _box;
			v->canvas_id = v->id;
			v->style_id = _style_id;
			v->item_uid = _item_uid;
			return v;

		}

		page_item* page::canvas2d_column(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px })
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::canvas2d_column;
			v->box = _box;
			v->canvas_id = v->id;
			v->style_id = _style_id;
			v->item_uid = _item_uid;
			return v;

		}

		page_item* page::canvas2d_absolute(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px })
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::canvas2d_absolute;
			v->box = _box;
			v->canvas_id = v->id;
			v->style_id = _style_id;
			v->item_uid = _item_uid;
			return v;
		}

		page_item* page::space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::space;
			v->box = _box;
			v->style_id = _style_id;
			return v;
		}

		page_item* page::text(page_item* _parent, relative_ptr_type _style_id, const char* _text, layout_rect _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::text;
			v->box = _box;
			v->style_id = _style_id;
			if (_text)
				v->caption = data.copy(_text, 0);
			return v;
		}

		page_item* page::field(page_item* _parent, int object_id, int field_id, jobject slice)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::column;
			v->set_parent(_parent);
			v->class_id = slice.get_class_id();
			v->object_id = object_id;
			v->box = { 0.0_pct, 0.0_pct, 100.0_px, 50.0_px };
			v->slice = slice;
			v->field = &(slice.get_field_by_id(field_id));
			return v;
		}

		page_item* page::select(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type id_name, jobject slice, relative_ptr_type _style_id, layout_rect box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::select;
			v->slice = slice;
			v->object_id = object_id;
			v->box = box;
			v->select_request = _state->create_select_request(v->object_id, false);
			v->caption = data.copy(slice.get_name(id_name), 0);
			v->slice = slice;
			v->style_id = _style_id;
			v->class_id = slice.get_class_id();

			if (slice.has_field("layout_rect"))
			{
				auto rf = slice.get_layout_rect("layout_rect");
				v->box = rf;
				return v;
			}
		}

		page_item* page::select_cell(page_item* _parent, actor_state* _state, int object_id, jobject slice, const char *_caption, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::select_cell;
			v->slice = slice;
			v->object_id = object_id;
			v->class_id = slice.get_class_id();
			v->box = _box;
			v->style_id = _style_id;
			v->select_request = _state->create_select_request(v->object_id, false);
			v->caption = data.copy(_caption, 0);
			return v;
		}

		page_item* page::navigate(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box )
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::navigate;
			v->object_id = object_id;
			v->box = _box;
			v->select_request = _state->create_select_request(v->object_id, false);
			v->style_id = _style_id;
			if (_caption) {
				v->caption = data.copy(_caption,0);
			}
			return v;
		}

		page_item* page::actor_update_fields(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection, field_layout _field_layout, const char* _object_title)
		{

			if (_state->modified_object_id != null_row)
			{
				auto avo = _state->get_modified_object();
				auto slice = _collection->get_object(avo.object_id);
				page_item* label;

				if (_object_title)
				{
					label = append();
					label->id = size();
					label->parent_id = _parent->id;
					label->layout = layout_types::label;
					label->box = { 0.0_px, 0.0_px, 300.0_px, 1.0_fntgr };
					label->slice = slice;
					label->class_id = slice.get_class_id();
					label->object_id = avo.object_id;
					label->style_id = _schema->idf_view_subtitle_style;
					label->caption = data.copy(_object_title, 0);
				}

				for (int i = 0; i < slice.size(); i++)
				{
					page_item* container = nullptr;

					jfield& fld = slice.get_field(i);
					if (!fld.display_in_user_ui)
						continue;

					switch (_field_layout)
					{
					case field_layout::label_on_left:
						container = row(_parent, _schema->idf_label_style, { 0.0_px, 0.0_px, 400.0_px, 1.1_fntgr });
						break;
					case field_layout::label_on_top:
						container = column(_parent, _schema->idf_label_style, { 0.0_px, 0.0_px, 200.0_px, 2.0_fntgr });
						break;
					}

					label = append();
					label->id = size();
					label->parent_id = container->id;
					label->layout = layout_types::label;
					label->field = &fld;
					label->box = { 0.0_px, 0.0_px, 150.0_px, 1.0_fntgr };
					label->slice = slice;
					label->object_id = avo.object_id;
					label->style_id = _schema->idf_label_style;
					label->caption = fld.description;
					label->class_id = slice.get_class_id();

					page_item* control = append();
					control->id = size();
					control->parent_id = container->id;
					control->layout = layout_types::field;
					control->field = &fld;
					control->box = { 0.0_px, 0.0_px, 200.0_px, 1.0_fntgr };
					control->slice = slice;
					control->object_id = avo.object_id;
					control->class_id = slice.get_class_id();
					control->style_id = _schema->idf_control_style;
				}
			}

			return _parent;
		}

		page_item* page::actor_create_buttons(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection, relative_ptr_type _style_id, layout_rect _box)
		{
			for (auto aco : _state->create_objects)
			{
				page_item* button = append();
				button->set_parent(_parent);
				button->id = size();
				button->layout = layout_types::create;
				button->box = _box;
				button->class_id = aco.second.class_id;
				button->field = nullptr;
				button->create_request = _state->create_create_request(aco.second.class_id);
				button->style_id = _style_id;

				object_description desc;
				desc = "Add " + _schema->get_class(aco.second.class_id).pitem()->name;
				button->caption = data.copy<char>(desc.c_str(), 0);
			}
			return _parent;
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
				auto slice = _collection->get_object(st.second.object_id);
				if (slice.has_field("layout_rect"))
				{
					auto rf = slice.get_layout_rect("layout_rect");
					v->box = rf;
					v->select_request = _state->create_select_request(v->object_id, false);
				}
			}
			return _parent;
		}

		void page::visit_impl(page_item *r, std::function<bool(page_item* _item)> fn, std::function<bool(page_item* _parent)> fout)
		{
			fn(r);
			auto children = where([r](const auto& it) {
				return it.item.parent_id == r->id;
				});
			for (auto child : children)
			{
				visit_impl(&child.item, fn, fout);
			}
			fout(r);
		}

		void page::calculate_sizes(jobject& _style_sheet, page::iterator_type children, double offx, double offy, double x, double y, double width, double height, double& remaining_width, double& remaining_height)
		{
			remaining_width = width;
			remaining_height = height;

			for (auto child : children)
			{
				if (child.item.box.width.units == measure_units::pixels)
				{
					remaining_width -= child.item.box.width.amount;
				}
				if (child.item.box.height.units == measure_units::pixels)
				{
					remaining_height -= child.item.box.height.amount;
				}
			}
		}

		void page::calculate_bounds_w(jobject& _style_sheet, page_item* _item, double width, double height, int safety)
		{
			if (safety > 2)
				return;
			if (_item->box.width.units == measure_units::percent_remaining)
			{
				_item->bounds.w = _item->box.width.amount * width / 100.0;
			}
			else if (_item->box.width.units == measure_units::percent_height)
			{
				calculate_bounds_h(_style_sheet, _item, width, height, safety+1);
				_item->bounds.w = _item->box.width.amount * _item->bounds.h / 100.0;
			}
			else if (_item->box.width.units == measure_units::font || _item->box.width.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				if (_item->style_id != null_row && _style_sheet.has_field(_item->style_id))
				{
					jobject style = _style_sheet.get_object(_item->style_id, true).get_slice({ 0,0,0 });
					font_height = style.get(style.get_schema()->idf_font_size);
				};
				_item->bounds.w = font_height * _item->box.width.amount;
				if (_item->box.width.units == measure_units::font_golden_ratio)
				{
					_item->bounds.w /= 1.618;
				}
			}
			else
				_item->bounds.w = _item->box.width.amount;
		}

		void page::calculate_bounds_h(jobject& _style_sheet, page_item* _item, double width, double height, int safety)
		{
			if (safety > 2)
				return;
			if (_item->box.height.units == measure_units::percent_remaining)
			{
				_item->bounds.h = _item->box.height.amount * height / 100.0;
			}
			else if (_item->box.height.units == measure_units::percent_width)
			{
				calculate_bounds_w(_style_sheet, _item, width, height, safety+1);
				_item->bounds.h = _item->box.height.amount * _item->bounds.w / 100.0;
			}
			else if (_item->box.height.units == measure_units::font || _item->box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				if (_item->style_id != null_row && _style_sheet.has_field(_item->style_id)) 
				{
					jobject style = _style_sheet.get_object(_item->style_id, true).get_slice({ 0,0,0 });
					font_height = style.get(style.get_schema()->idf_font_size);
				};
				_item->bounds.h = font_height * _item->box.height.amount;
				if (_item->box.height.units == measure_units::font_golden_ratio)
				{
					_item->bounds.h *= 1.618;
				}
			}
			else
				_item->bounds.h = _item->box.height.amount;
		}

		void page::set_bound_size(jobject& _style_sheet, page_item* _item, double offx, double offy, double x, double y, double width, double height)
		{
			calculate_bounds_w(_style_sheet, _item, width, height, 0);
			calculate_bounds_h(_style_sheet, _item, width, height, 0);

			if (_item->box.x.amount >= 0.0)
			{
				switch (_item->box.x.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.x = _item->box.x.amount * width / 100.0 + x + offx;
					break;
				case measure_units::pixels:
					_item->bounds.x = _item->box.x.amount + x + offx;
					break;
				case measure_units::percent_height:
					_item->bounds.x = _item->box.x.amount * _item->bounds.h / 100.0 + x + offx;
					break;
				case measure_units::percent_width:
					_item->bounds.x = _item->box.x.amount * _item->bounds.w / 100.0 + x + offx;
					break;
				}
			}
			else
			{
				switch (_item->box.x.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.x = (width - (_item->box.x.amount * width / 100.0)) + x + offx;
					break;
				case measure_units::pixels:
					_item->bounds.x = (width - _item->box.x.amount) + x + offx;
					break;
				case measure_units::percent_height:
					_item->bounds.x = width -  (_item->box.x.amount * _item->bounds.h / 100.0) + x + offx;
					break;
				case measure_units::percent_width:
					_item->bounds.x = width - (_item->box.x.amount * _item->bounds.w / 100.0) + x + offx;
					break;
				}
			}

			if (_item->box.y.amount >= 0.0)
			{
				switch (_item->box.y.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.y = _item->box.y.amount * height / 100.0 + y + offy;
					break;
				case measure_units::pixels:
					_item->bounds.y = _item->box.y.amount + y + offy;
					break;
				case measure_units::percent_height:
					_item->bounds.y = _item->box.y.amount * _item->bounds.h / 100.0 + y + offy;
					break;
				case measure_units::percent_width:
					_item->bounds.y = _item->box.y.amount * _item->bounds.w / 100.0 + y + offy;
					break;
				}
			}
			else
			{
				switch (_item->box.y.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.y = (height - (_item->box.y.amount * height / 100.0)) + y + offy;
					break;
				case measure_units::pixels:
					_item->bounds.y = (height - _item->box.y.amount) + y + offy;
					break;
				case measure_units::percent_height:
					_item->bounds.y = width - (_item->box.y.amount * _item->bounds.h / 100.0) + y + offy;
					break;
				case measure_units::percent_width:
					_item->bounds.y = width - (_item->box.y.amount * _item->bounds.w / 100.0) + y + offy;
					break;
				}
			}

#if TRACE_LAYOUT
			std::cout << std::format("p:{},c:{},l:{} bounds {},{},{},{} canvas {}, is_draw {} {}", _item->parent_id, _item->id, (int)_item->layout, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h, _item->canvas_id, _item->is_drawable(), _item->caption ? _item->caption : "") << std::endl;
#endif

		}

		void page::arrange_impl(jobject& _style_sheet, page_item* _item, double offx, double offy, double x, double y, double width, double height)
		{

			set_bound_size(_style_sheet, _item, offx, offy, x, y, width, height);

			auto children = where([_item](const auto& it) {
				return it.item.parent_id == _item->id;
				});

			double remaining_width, remaining_height;
			calculate_sizes(_style_sheet, children, offx, offy, x, y, width, height, remaining_width, remaining_height);

			if (_item->layout == layout_types::row || _item->layout == layout_types::canvas2d_row || _item->layout == layout_types::canvas3d_row)
			{
				switch (_item->item_space.units)
				{
				case measure_units::font:
					_item->item_space_amount = _item->item_space.amount * 16.0;
					break;
				case measure_units::font_golden_ratio:
					_item->item_space_amount = _item->item_space.amount * 16.0 / 1.618;
					break;
				case measure_units::percent_height:
					_item->item_space_amount = _item->item_space.amount * height / 100.0;
					break;
				case measure_units::percent_width:
					_item->item_space_amount = _item->item_space.amount * width / 100.0;
					break;
				case measure_units::percent_remaining:
					_item->item_space_amount = _item->item_space.amount * remaining_width / 100.0;
					break;
				case measure_units::pixels:
					_item->item_space_amount = _item->item_space.amount;
					break;
				}

				double startx = 0;
				for (auto child : children)
				{
					arrange_impl(_style_sheet, &child.item, startx, 0, _item->bounds.x, _item->bounds.y, remaining_width, _item->bounds.h);
					startx += (child.item.bounds.w);
					startx += _item->item_space_amount;
				}
			}
			else if (_item->layout == layout_types::column || _item->layout == layout_types::canvas2d_column || _item->layout == layout_types::canvas3d_column)
			{
				switch (_item->item_space.units)
				{
				case measure_units::font:
					_item->item_space_amount = _item->item_space.amount * 16.0;
					break;
				case measure_units::font_golden_ratio:
					_item->item_space_amount = _item->item_space.amount * 16.0 * 1.618;
					break;
				case measure_units::percent_height:
					_item->item_space_amount = _item->item_space.amount * height / 100.0;
					break;
				case measure_units::percent_width:
					_item->item_space_amount = _item->item_space.amount * width / 100.0;
					break;
				case measure_units::percent_remaining:
					_item->item_space_amount = _item->item_space.amount * remaining_width / 100.0;
					break;
				case measure_units::pixels:
					_item->item_space_amount = _item->item_space.amount;
					break;
				}

				double starty = 0;
				for (auto child : children)
				{
					arrange_impl(_style_sheet, &child.item, 0, starty, _item->bounds.x, _item->bounds.y, _item->bounds.w, remaining_height);
					starty += (child.item.bounds.h);
					starty += _item->item_space_amount;
				}
			}
			else if (_item->layout == layout_types::canvas2d_absolute || _item->layout == layout_types::canvas3d_absolute)
			{
				for (auto child : children)
				{
					arrange_impl(_style_sheet, &child.item, 0, 0, 0, 0, _item->bounds.w, _item->bounds.h);
				}
			}
			else if (_item->layout == layout_types::select_cell)
			{
				for (auto child : children)
				{
					arrange_impl(_style_sheet, &child.item, 0, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
				}
			}
			else
			{
				for (auto child : children)
				{
					arrange_impl(_style_sheet, &child.item, 0, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
				}

				if (_item->object_id != null_row) 
				{
					relative_ptr_type class_id = _item->slice.get_class_id();

					if (_item->style_id == null_row)
					{
						if (_item->slice.has_field("rectangle"))
						{
							auto rect = _item->slice.get_rectangle("rectangle");
							rect = _item->bounds;
						}
					}
				}
			}
		}

		void page::arrange(double width, double height, jobject& _style_sheet)
		{
			sort([](auto& a, auto& b) {
				return std::tie( a.parent_id, a.id ) < std::tie( b.parent_id, b.id );
				});
			for (auto pix = begin(); pix != end(); pix++)
			{
				auto pi = pix.get_object();
				if (pi.item.parent_id < 0) {
					arrange_impl(_style_sheet, &pi.item, 0, 0, 0, 0, width, height);
				}
				else 
				{
					break;
				}
			}
		}

		void page::visit(std::function<bool(page_item* _parent)> fnin, std::function<bool(page_item* _parent)> fout)
		{
			sort([](auto& a, auto& b) {
				return (a.parent_id < b.parent_id);
				});
			for (auto pi : *this)
			{
				if (pi.item.parent_id < 0) 
				{
					visit_impl(&pi.item, fnin, fout);
				}
				else
				{
					break;
				}
			}
		}
	}
}
