
#include "corona.h"

#define TRACE_LAYOUT 1

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
			page_base_type::clear();
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

		page_item* page::canvas2d_row(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
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

		page_item* page::canvas2d_column(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
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

		page_item* page::canvas2d_absolute(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
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

		page_item* page::set(page_item* _parent, actor_state* _state, object_member_path path, int field_id, dynamic_value dv, layout_rect _box)
		{
			page_item* v = append();
			v->id = size();
			v->layout = layout_types::set;
			v->set_parent(_parent);
			v->object_path = path;
			v->box = _box;
			v->dest_value = dv;
			return v;
		}

		page_item* page::select(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type id_name, jobject slice, relative_ptr_type _style_id, layout_rect box)
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::select;
			v->slice = slice;
			v->object_path.object.row_id = object_id;
			v->box = box;
			v->select_request = _state->create_select_request(v->object_path.object.row_id, false);
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
			v->object_path.object.row_id = object_id;
			v->class_id = slice.get_class_id();
			v->box = _box;
			v->style_id = _style_id;
			v->select_request = _state->create_select_request(object_id, false);
			v->caption = data.copy(_caption, 0);
			return v;
		}

		page_item* page::navigate(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box )
		{
			page_item* v = append();
			v->id = size();
			v->set_parent(_parent);
			v->layout = layout_types::navigate;
			v->object_path.object.row_id = object_id;
			v->box = _box;
			v->select_request = _state->create_select_request(object_id, false);
			v->style_id = _style_id;
			if (_caption) {
				v->caption = data.copy(_caption,0);
			}
			return v;
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

		point page::size_constants(jobject& _style_sheet, page::iterator_type children)
		{
			point p;

			p.x = 0;
			p.y = 0;

			for (auto child : children)
			{
				page_item& pi = child.item;
				pi.bounds.w = 0;
				pi.bounds.h = 0;

				if (pi.box.width.units == measure_units::pixels)
				{
					pi.bounds.w = pi.box.width.amount;
				}
				else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
				{
					double font_height = 12.0;
					if (pi.style_id != null_row && _style_sheet.has_field(pi.style_id))
					{
						jobject style = _style_sheet.get_object(pi.style_id, true).get_object({ 0,0,0 });
						font_height = style.get(style.get_schema()->idf_font_size);
					};
					pi.bounds.w = font_height * pi.box.width.amount;
					if (pi.box.width.units == measure_units::font_golden_ratio)
					{
						pi.bounds.w /= 1.618;
					}
				}

				if (pi.box.height.units == measure_units::pixels)
				{
					pi.bounds.h = pi.box.height.amount;
				}
				else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
				{
					double font_height = 12.0;
					if (pi.style_id != null_row && _style_sheet.has_field(pi.style_id))
					{
						jobject style = _style_sheet.get_object(pi.style_id, true).get_object({ 0,0,0 });
						font_height = style.get(style.get_schema()->idf_font_size);
					};
					pi.bounds.h = font_height * pi.box.height.amount;
					if (pi.box.height.units == measure_units::font_golden_ratio)
					{
						pi.bounds.h *= 1.618;
					}
				}
				p.x += pi.bounds.w;
				p.y += pi.bounds.h;
			}
			return p;
		}

		void page::size_variadic_widths(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (_item->box.width.units == measure_units::percent_remaining)
			{
				_item->bounds.w = _item->box.width.amount * _ctx.remaining_size.x / 100.0;
			}
			else if (_item->box.width.units == measure_units::percent_aspect)
			{
				size_variadic_heights(_style_sheet, _item, _ctx, safety+1);
				_item->bounds.w = _item->box.width.amount * _item->bounds.h / 100.0;
			}
			else
				_item->bounds.w = _item->box.width.amount;
		}

		void page::size_variadic_heights(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;
			if (_item->box.height.units == measure_units::percent_remaining)
			{
				_item->bounds.h = _item->box.height.amount * _ctx.remaining_size.y / 100.0;
			}
			else if (_item->box.height.units == measure_units::percent_aspect)
			{
				size_variadic_widths(_style_sheet, _item, _ctx, safety+1);
				_item->bounds.h = _item->box.height.amount * _item->bounds.w / 100.0;
			}
		}

		void page::size_variadic(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			size_variadic_heights(_style_sheet, _item, _ctx, 0);
			size_variadic_widths(_style_sheet, _item, _ctx, 0);

#if TRACE_LAYOUT
			std::cout << std::format("p:{},c:{},l:{} bounds {},{},{},{} canvas {}, is_draw {} {}", _item->parent_id, _item->id, (int)_item->layout, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h, _item->canvas_id, _item->is_drawable(), _item->caption ? _item->caption : "") << std::endl;
#endif

		}

		void page::size_variadics(jobject& _style_sheet, page::iterator_type children, layout_context _ctx)
		{
			for (auto child : children)
			{
				size_variadic(_style_sheet, &child.item, _ctx);
			}
		}

		void page::size(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			auto children = where([_item](const auto& it) {
				return it.item.parent_id == _item->id;
				});

			_ctx.remaining_size = size_constants(_style_sheet, children);
			size_variadics(_style_sheet, children, _ctx);
		}

		void page::position(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			if (_item->box.x.amount >= 0.0)
			{
				switch (_item->box.x.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.x = _item->box.x.amount * _ctx.remaining_size.x / 100.0 + _ctx.flow_origin.x;
					break;
				case measure_units::pixels:
					_item->bounds.x = _item->box.x.amount + _ctx.flow_origin.x;
					break;
				default:
					_item->bounds.x = _ctx.flow_origin.x;
					break;
				}
			}
			else
			{
				switch (_item->box.x.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.x = (_ctx.container_size.x - (_ctx.flow_origin.x + _item->box.x.amount * _ctx.remaining_size.x / 100.0));
					break;
				case measure_units::pixels:
					_item->bounds.x = (_ctx.container_size.x - (_ctx.flow_origin.x +_item->box.x.amount));
					break;
				default:
					_item->bounds.x = (_ctx.container_size.x - _ctx.flow_origin.x);
					break;
				}
			}

			if (_item->box.y.amount >= 0.0)
			{
				switch (_item->box.y.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.y = _item->box.y.amount * remaining_height / 100.0 + _ctx.flow_origin.y;
					break;
				case measure_units::pixels:
					_item->bounds.y = _item->box.y.amount + y + offy;
					break;
				case measure_units::percent_aspect:
					_item->bounds.y = _item->box.y.amount * _item->bounds.w / 100.0 + y + offy;
					break;
				}
			}
			else
			{
				switch (_item->box.y.units)
				{
				case measure_units::percent_remaining:
					_item->bounds.y = (remaining_height - (_item->box.y.amount * remaining_height / 100.0)) + y + offy;
					break;
				case measure_units::pixels:
					_item->bounds.y = (remaining_height - _item->box.y.amount) + y + offy;
					break;
				case measure_units::percent_aspect:
					_item->bounds.y = _item->box.y.amount * _item->bounds.w / 100.0 + y + offy;
					break;
				}
			}

			auto schema = _style_sheet.get_schema();
			relative_ptr_type class_id = _item->slice.get_class_id();

			if (!_item->slice.is_null() && _item->slice.has_field(schema->idf_rectangle))
			{
				auto rect = _item->slice.get_rectangle(schema->idf_rectangle);
				_item->bounds = rect;
			}
			else if (_item->layout == layout_types::row || _item->layout == layout_types::canvas2d_row || _item->layout == layout_types::canvas3d_row)
			{

				switch (_item->item_space.units)
				{
				case measure_units::font:
					_item->item_space_amount = _item->item_space.amount * 16.0;
					break;
				case measure_units::font_golden_ratio:
					_item->item_space_amount = _item->item_space.amount * 16.0 / 1.618;
					break;
				case measure_units::percent_aspect:
					_item->item_space_amount = _item->item_space.amount * height / 100.0;
					break;
				case measure_units::percent_remaining:
					_item->item_space_amount = _item->item_space.amount * remaining_width / 100.0;
					break;
				case measure_units::pixels:
					_item->item_space_amount = _item->item_space.amount;
					break;
				}

				double bx = 0.0, by = 0.0;

				bx = _item->bounds.x;
				by = _item->bounds.y;

				double startx = 0;
				for (auto child : children)
				{
					layout_item(_style_sheet, &child.item, startx, 0, bx, by, remaining_width, _item->bounds.h);
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
				case measure_units::percent_aspect:
					_item->item_space_amount = _item->item_space.amount * width / 100.0;
					break;
				case measure_units::percent_remaining:
					_item->item_space_amount = _item->item_space.amount * remaining_height / 100.0;
					break;
				case measure_units::pixels:
					_item->item_space_amount = _item->item_space.amount;
					break;
				}

				double bx = 0.0, by = 0.0;

				bx = _item->bounds.x;
				by = _item->bounds.y;

				double starty = 0;
				for (auto child : children)
				{
					layout_item(_style_sheet, &child.item, 0, starty, bx, by, _item->bounds.w, remaining_height);
					starty += (child.item.bounds.h);
					starty += _item->item_space_amount;
				}
			}
			else
			{
				for (auto child : children)
				{
					layout_item(_style_sheet, &child.item, 0, 0, _item->bounds.x, _item->bounds.y, _item->bounds.w, _item->bounds.h);
				}
			}
		}

		void page::arrange(double width, double height, jobject& _style_sheet, double _padding)
		{
			for (auto pix = begin(); pix != end(); pix++)
			{
				auto pi = pix.get_object();
				if (pi.item.parent_id < 0) {
					layout_item(_style_sheet, &pi.item, 0, 0, _padding, _padding, width - _padding * 2.0, height - _padding * 2.0);
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
