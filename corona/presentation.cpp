
#include "corona.h"

#define TRACE_LAYOUT 01

namespace corona
{
	namespace database
	{

		std::shared_ptr<row_container_control> control_base::create_row(int id)
		{
			;
		}

		std::shared_ptr<column_container_control> control_base::create_column(int id)
		{

		}

		std::shared_ptr<absolute_container_control> control_base::create_absolute(int id)
		{

		}

		std::shared_ptr<static_control> control_base::create_static(int id)
		{
			std::shared_ptr<static_control> wnd = std::make_shared<static_control>();
		}

		std::shared_ptr<button_control> control_base::create_button(int id)
		{
			std::shared_ptr<button_control> wnd = std::make_shared<button_control>();
		}

		std::shared_ptr<listbox_control> control_base::create_listbox(int id)
		{

		}

		std::shared_ptr<combobox_control> control_base::create_combobox(int id)
		{

		}

		std::shared_ptr<edit_control> control_base::create_edit(int id)
		{

		}

		std::shared_ptr<scrollbar_control> control_base::create_scrollbar(int id)
		{

		}

		std::shared_ptr<imagelist_control> control_base::create_imagelist(int id)
		{

		}

		std::shared_ptr<listview_control> control_base::create_listview(int id)
		{

		}

		std::shared_ptr<treeview_control> control_base::create_treeview(int id)
		{

		}

		std::shared_ptr<header_control>  control_base::create_header(int id)
		{

		}

		std::shared_ptr<toolbar_control> control_base::create_toolbar(int id)
		{

		}

		std::shared_ptr<statusbar_control> control_base::create_statusbar(int id)
		{

		}

		std::shared_ptr<hotkey_control> control_base::create_hotkey(int id)
		{

		}

		std::shared_ptr<animate_control> control_base::create_animate(int id)
		{

		}

		std::shared_ptr<richedit_control> control_base::create_richedit(int id)
		{

		}

		std::shared_ptr<draglistbox_control> control_base::create_draglistbox(int id)
		{

		}

		std::shared_ptr<rebar_control> control_base::create_rebar(int id)
		{

		}

		std::shared_ptr<comboboxex_control> control_base::create_comboboxex(int id)
		{

		}

		std::shared_ptr<datetimepicker_control> control_base::create_datetimepicker(int id)
		{

		}

		std::shared_ptr<monthcalendar_control> control_base::create_monthcalendar(int id)
		{

		}

		void control_base::size_constant(layout_context _ctx)
		{

			control_base& pi = *this;
			pi.bounds.w = 0;
			pi.bounds.h = 0;

			if (pi.box.width.units == measure_units::pixels)
			{
				pi.bounds.w = pi.box.width.amount;
			}
			else if (pi.box.width.units == measure_units::percent_container)
			{
				pi.bounds.w = pi.box.width.amount * _ctx.container_size.x;
			}
			else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
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
			else if (pi.box.height.units == measure_units::percent_container)
			{
				pi.bounds.h = pi.box.height.amount * _ctx.container_size.y;
			}
			else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				pi.bounds.h = font_height * pi.box.height.amount;
				if (pi.box.height.units == measure_units::font_golden_ratio)
				{
					pi.bounds.h *= 1.618;
				}
			}
		}

		void control_base::size_constants(layout_context _ctx)
		{
			for (auto child : children)
			{
				child->size_constant(_ctx);
			}
		}

		void control_base::size_aspect_widths(layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (box.width.units == measure_units::percent_aspect)
			{
				size_aspect_heights(_ctx, safety + 1);
				bounds.w = box.width.amount * bounds.h;
			}
		}

		void control_base::size_aspect_heights(layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (box.height.units == measure_units::percent_aspect)
			{
				size_aspect_widths(_ctx, safety + 1);
				bounds.h = box.height.amount * bounds.w;
			}
		}

		void control_base::size_aspect(layout_context _ctx)
		{
			size_aspect_heights(_ctx, 0);
			size_aspect_widths(_ctx, 0);
		}

		void control_base::size_aspects(layout_context _ctx)
		{
			for (auto child : children)
			{
				child->size_aspect(_ctx);
			}
		}

		layout_context control_base::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };
			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}


		layout_context row_container_control::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.width.units != measure_units::percent_remaining)
				{
					pt.x += child->bounds.w;
				}
			}

			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}

		layout_context column_container_control::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.height.units != measure_units::percent_remaining)
				{
					pt.y += child->bounds.h;
				}
			}

			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}


		void control_base::size_remaining(layout_context _ctx)
		{
			if (box.width.units == measure_units::percent_remaining)
			{
				bounds.w = box.width.amount * _ctx.remaining_size.x;
			}
			if (box.height.units == measure_units::percent_remaining)
			{
				bounds.h = box.height.amount * _ctx.remaining_size.y;
			}
		}

		void control_base::size_remainings(layout_context _ctx)
		{
			_ctx = get_remaining(_ctx);
			for (auto child : children)
			{
				child->size_remaining(_ctx);
			}
		}

		void control_base::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto ew = child->bounds.w + child->bounds.x - bounds.x;
					if (ew > sizes.x)
					{
						sizes.x = ew;
					}
					auto eh = child->bounds.h + child->bounds.y - bounds.y;
					if (child->bounds.h > sizes.y)
					{
						sizes.y = child->bounds.h;
					}
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		void column_container_control::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto ew = child->bounds.w + child->bounds.x - bounds.x;
					if (ew > sizes.x)
					{
						sizes.x = ew;
					}
					sizes.y += child->bounds.h;
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		void row_container_control::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto eh = child->bounds.h + child->bounds.y - bounds.y;
					if (child->bounds.h > sizes.y)
					{
						sizes.y = child->bounds.h;
					}
					sizes.x += child->bounds.w;
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		void control_base::size_item(layout_context _ctx)
		{
			size_constant(_ctx);
			size_aspect(_ctx);
			size_remaining(_ctx);
		}

		void control_base::size_items(layout_context _ctx)
		{
			size_constants(_ctx);
			size_aspects(_ctx);
			size_remainings(_ctx);
		}

		void control_base::position(layout_context _ctx)
		{
			switch (box.x.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				bounds.x = box.x.amount * _ctx.container_size.x + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::pixels:
				bounds.x = box.x.amount + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				bounds.x = _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			}

			switch (box.y.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				bounds.y = box.y.amount * _ctx.container_size.y + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::pixels:
				bounds.y = box.y.amount + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				bounds.y = _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			}


			switch (item_space.units)
			{
			case measure_units::font:
				item_space_amount.x = item_space.amount * 16.0;
				item_space_amount.y = item_space.amount * 16.0;
				break;
			case measure_units::font_golden_ratio:
				item_space_amount.x = item_space.amount * 16.0 / 1.618;
				item_space_amount.y = item_space.amount * 16.0 / 1.618;
				break;
			case measure_units::percent_aspect:
				item_space_amount.x = item_space.amount * _ctx.container_size.y;
				item_space_amount.y = item_space.amount * _ctx.container_size.x;
				break;
			case measure_units::percent_remaining:
				item_space_amount.x = item_space.amount * _ctx.remaining_size.x;
				item_space_amount.y = item_space.amount * _ctx.remaining_size.y;
				break;
			case measure_units::pixels:
				item_space_amount.x = item_space.amount;
				item_space_amount.y = item_space.amount;
				break;
			}
		}

		void control_base::positions(layout_context _ctx)
		{
			_ctx.flow_origin.x = 0;
			_ctx.flow_origin.y = 0;

			for (auto child : children)
			{
				child->position(_ctx);
				child->layout(_ctx);
			}

			size_children(_ctx);
		}

		void row_container_control::positions(layout_context _ctx)
		{
			if (alignment == visual_alignment::align_near)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x += (child->bounds.w);
					_ctx.flow_origin.x += _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_far)
			{
				_ctx.flow_origin.x = _ctx.container_size.x;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					_ctx.flow_origin.x -= (child->bounds.w);
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x -= _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_center)
			{
				double w = 0.0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					w += child->bounds.w;
				}

				_ctx.flow_origin.x = (_ctx.container_size.x - w) / 2;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x += (child->bounds.w);
					_ctx.flow_origin.x += _ctx.space_amount.x;
				}
			}

			size_children(_ctx);
		}

		void column_container_control::positions(layout_context _ctx)
		{	
			if (alignment == visual_alignment::align_near)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position( _ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y += (child->bounds.h);
					_ctx.flow_origin.y += _ctx.space_amount.y;
				}
			}
			else if (alignment == visual_alignment::align_far)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = _ctx.container_size.y;

				for (auto child : children)
				{
					_ctx.flow_origin.y -= (child->bounds.h);
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y -= _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_center)
			{
				double h = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					h += child->bounds.h;
				}

				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = (_ctx.container_size.y - h) / 2;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y += (child->bounds.h);
					_ctx.flow_origin.y += _ctx.space_amount.y;
				}
			}

			size_children(_ctx);
		}

		rectangle control_base::layout(layout_context _ctx)
		{
			_ctx.container_origin.x = bounds.x;
			_ctx.container_origin.y = bounds.y;
			_ctx.container_size.x = bounds.w;
			_ctx.container_size.y = bounds.h;
			_ctx.remaining_size = _ctx.container_size;
			_ctx.space_amount = { (double)item_space.amount, (double)item_space.amount };
			_ctx.flow_origin.x = 0;
			_ctx.flow_origin.y = 0;

			size_items(_ctx);
			positions(_ctx);

			return bounds;
		}

		page::page()
		{
		}

		void page::clear()
		{
		}

		control_base* page::append(control_base* _parent, control_types _layout, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			control_base* v = page_base_type::append();
			v->id = page_base_type::get_index(v);
			v->set_parent(_parent);
			v->control_type = _layout;
			v->style_id = _style_id;
			v->box = _box;
			v->item_space = _item_space;
			v->alignment = _alignment;
			return v;
		}

		control_base* page::row(control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return append(_parent, control_types::row, _style_id, _box, _item_space, _alignment);
		}

		control_base* page::column(control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return append(_parent, control_types::column, _style_id, _box, _item_space, _alignment);
		}

		control_base* page::absolute(control_base* _parent, relative_ptr_type _style_id, layout_rect _box, visual_alignment _alignment)
		{
			return append(_parent, control_types::absolute, _style_id, _box, 0.0_px, _alignment);
		}

		control_base* page::canvas2d_row(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			control_base* v = append(_parent, control_types::d2d_row, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		control_base* page::canvas2d_column(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			control_base* v = append(_parent, control_types::d2d_column, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		control_base* page::canvas2d_absolute(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			control_base* v = append(_parent, control_types::d2d_absolute, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		control_base* page::space(control_base* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			control_base* v = append(_parent, control_types::space, _style_id, _box, 0.0_px, visual_alignment::align_near);
			return v;
		}

		control_base* page::text(control_base* _parent, relative_ptr_type _style_id, const char* _text, layout_rect _box)
		{
			control_base* v = append(_parent, control_types::text, _style_id, _box, 0.0_px, visual_alignment::align_near);
			if (_text)
				v->caption = data.copy(_text, 0);
			return v;
		}

		void page::visit_impl(control_base *r, std::function<bool(control_base* _item)> fn, std::function<bool(control_base* _parent)> fout)
		{
			fn(r);
			for (auto child = r->get_first_child(); child != nullptr; child = child->get_next())
			{
				visit_impl(child, fn, fout);
			}
			fout(r);
		}


		void page::arrange(double width, double height, double _padding)
		{
			for (auto pix = begin(); pix != end(); pix++)
			{
				auto pi = pix.get_object();
				if (pi.item.parent_id < 0) {
					double pd = _padding * 2.0;
					layout_context ctx;
					ctx.space_amount = { 0.0, 0.0 };
					ctx.container_size = { width - pd, height - pd };
					ctx.container_origin = { _padding, _padding };
					ctx.flow_origin = { 0, 0 };
					ctx.remaining_size = ctx.container_size;
					if (pi.item.box.height.units == measure_units::percent_child ||
						pi.item.box.width.units == measure_units::percent_child)
						throw std::logic_error("Cannot use child based sizing on a root element");
					size_item( &pi.item, ctx);
					position( &pi.item, ctx);
					layout(&pi.item, ctx);
				}
				else 
				{
					break;
				}
			}

#if TRACE_LAYOUT
			std::cout << std::endl;
			for (auto pix = begin(); pix != end(); pix++)
			{
				auto& _item = pix.get_object().item;
				std::cout << _item.parent_id << "." << _item.id << " " <<
					database::control_type_names[(int)_item.control_type] << " " <<
					"(" << _item.bounds.x << ", " << _item.bounds.y << "  x  " << _item.bounds.w << "," << _item.bounds.h << ")" <<
					(_item.caption ? _item.caption : "") << std::endl;
			}
#endif

		}

		void page::visit(std::function<bool(control_base* _parent)> fnin, std::function<bool(control_base* _parent)> fout)
		{
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
