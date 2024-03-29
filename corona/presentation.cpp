
#include "corona.h"

#define TRACE_LAYOUT 01

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

		page_item* page::append(page_item* _parent, layout_types _layout, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			page_item* v = page_base_type::append();
			v->id = page_base_type::get_index(v);
			v->set_parent(_parent);
			v->layout = _layout;
			v->style_id = _style_id;
			v->box = _box;
			v->item_space = _item_space;
			v->alignment = _alignment;
			return v;
		}

		page_item* page::row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return append(_parent, layout_types::row, _style_id, _box, _item_space, _alignment);
		}

		page_item* page::column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return append(_parent, layout_types::column, _style_id, _box, _item_space, _alignment);
		}

		page_item* page::absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, visual_alignment _alignment)
		{
			return append(_parent, layout_types::absolute, _style_id, _box, 0.0_px, _alignment);
		}

		page_item* page::canvas2d_row(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			page_item* v = append(_parent, layout_types::canvas2d_row, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		page_item* page::canvas2d_column(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			page_item* v = append(_parent, layout_types::canvas2d_column, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		page_item* page::canvas2d_absolute(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			page_item* v = append(_parent, layout_types::canvas2d_absolute, _style_id, _box, _item_space, _alignment);
			v->canvas_id = v->id;
			v->item_uid = _item_uid;
			return v;
		}

		page_item* page::space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::space, _style_id, _box, 0.0_px, visual_alignment::align_near);
			return v;
		}

		page_item* page::text(page_item* _parent, relative_ptr_type _style_id, const char* _text, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::text, _style_id, _box, 0.0_px, visual_alignment::align_near);
			if (_text)
				v->caption = data.copy(_text, 0);
			return v;
		}

		page_item* page::set(page_item* _parent, actor_state* _state, object_member_path path, int field_id, dynamic_value dv, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::set, null_row, _box, 0.0_px, visual_alignment::align_near);
			v->object_path = path;
			v->dest_value = dv;
			return v;
		}

		page_item* page::select(page_item* _parent, actor_state* _state, relative_ptr_type object_id, relative_ptr_type id_name, jobject slice, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::select, _style_id, _box, 0.0_px, visual_alignment::align_near);
			v->slice = slice;
			v->object_path.object.row_id = object_id;
			v->select_request = _state->create_select_request(v->object_path.object.row_id, false);
			v->caption = data.copy(slice.get_name(id_name), 0);
			v->slice = slice;
			v->class_id = slice.get_class_id();

			if (slice.has_field("layout_rect"))
			{
				auto rf = slice.get_layout_rect("layout_rect");
				v->box = rf;
			}
			return v;
		}

		page_item* page::table_header(page_item* _parent, actor_state* _state, const char* _caption, relative_ptr_type object_id, jobject slice, relative_ptr_type _field_id, relative_ptr_type _sort_field_id, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::table_header, _style_id, _box, 0.0_px, visual_alignment::align_near);
			v->slice = slice;
			v->object_path.object.row_id = object_id;
			if (!slice.is_null()) {
				v->class_id = slice.get_class_id();
				v->field = &slice.get_field_by_id(_field_id);
			}
			v->caption = data.copy(_caption, 0);
			v->dest_value = dynamic_value(_field_id, _sort_field_id );
			return v;
		}

		page_item* page::table_cell(page_item* _parent, actor_state* _state, relative_ptr_type object_id, jobject slice, relative_ptr_type _field_id, relative_ptr_type _style_id, layout_rect _box)
		{
			page_item* v = append(_parent, layout_types::table_cell, _style_id, _box, 0.0_px, visual_alignment::align_near);
			v->slice = slice;
			v->object_path.object.row_id = object_id;
			if (!slice.is_null()) {
				v->class_id = slice.get_class_id();
				v->field = &slice.get_field_by_id(_field_id);
			}
			v->select_request = _state->create_select_request(object_id, false);
			const char* dataf = slice.get(_field_id);
			v->caption = data.copy(dataf, 0);
			return v;
		}

		page_item* page::navigate(page_item* _parent, actor_state* _state, relative_ptr_type object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box )
		{
			page_item* v = append(_parent, layout_types::navigate, _style_id, _box, 0.0_px, visual_alignment::align_near);
			v->object_path.object.row_id = object_id;
			v->select_request = _state->create_select_request(object_id, false);
			v->caption = data.copy(_caption, 0);
			return v;
		}

		void page::visit_impl(page_item *r, std::function<bool(page_item* _item)> fn, std::function<bool(page_item* _parent)> fout)
		{
			fn(r);
			for (auto child = r->get_first_child(); child != nullptr; child = child->get_next())
			{
				visit_impl(child, fn, fout);
			}
			fout(r);
		}

		void page::size_constant(jobject& _style_sheet, page_item *_item, layout_context _ctx)
		{
			
			page_item& pi = *_item;
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
			else if (pi.box.height.units == measure_units::percent_container)
			{
				pi.bounds.h = pi.box.height.amount * _ctx.container_size.y;
			}
			else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				if (pi.style_id != null_row && _style_sheet.has_field(pi.style_id))
				{
					jobject style = _style_sheet.get_object(pi.style_id, true).get_object({ 0,0,0 });
					auto schema = style.get_schema();
					font_height = style.get(schema->idf_font_size);
					if (font_height < 1.0) {
						std::cout << "WARNING: font height not specified on style:";
						if (pi.style_id) {
							std::cout << " " << schema->get_field(pi.style_id).name << " ";
						}
						if (pi.caption) {
							std::cout << " for " << pi.caption << std::endl;
						}
						else {
							std::cout << " for " << "(element)" << std::endl;
						}
					}
				};
				pi.bounds.h = font_height * pi.box.height.amount;
				if (pi.box.height.units == measure_units::font_golden_ratio)
				{
					pi.bounds.h *= 1.618;
				}
			}
		}

		void page::size_constants(jobject& _style_sheet, page_item *_item, layout_context _ctx)
		{
			for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
			{
				size_constant(_style_sheet, child, _ctx);
			}
		}

		void page::size_aspect_widths(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (_item->box.width.units == measure_units::percent_aspect)
			{
				size_aspect_heights(_style_sheet, _item, _ctx, safety+1);
				_item->bounds.w = _item->box.width.amount * _item->bounds.h;
			}
		}

		void page::size_aspect_heights(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;
			if (_item->box.height.units == measure_units::percent_aspect)
			{
				size_aspect_widths(_style_sheet, _item, _ctx, safety+1);
				_item->bounds.h = _item->box.height.amount * _item->bounds.w;
			}
		}

		void page::size_aspect(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			size_aspect_heights(_style_sheet, _item, _ctx, 0);
			size_aspect_widths(_style_sheet, _item, _ctx, 0);
		}

		void page::size_aspects(jobject& _style_sheet, page_item *_item, layout_context _ctx)
		{
			for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
			{
				size_aspect(_style_sheet, child, _ctx);
			}
		}

		layout_context page::get_remaining(jobject& _style_sheet, page_item* _pi, layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			if (_pi->layout == layout_types::canvas2d_column || _pi->layout == layout_types::column || _pi->layout == layout_types::canvas3d_column)
			{
				for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
				{
					if (child->box.height.units != measure_units::percent_remaining)
					{
						pt.y += child->bounds.h;
					}
				}
			}
			else if (_pi->layout == layout_types::canvas2d_row || _pi->layout == layout_types::row || _pi->layout == layout_types::canvas3d_row)
			{
				for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
				{
					if (child->box.width.units != measure_units::percent_remaining)
					{
						pt.x += child->bounds.w;
					}
				}
			}

			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}

		void page::size_remaining(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			if (_item->box.width.units == measure_units::percent_remaining)
			{
				_item->bounds.w = _item->box.width.amount * _ctx.remaining_size.x;
			}
			if (_item->box.height.units == measure_units::percent_remaining)
			{
				_item->bounds.h = _item->box.height.amount * _ctx.remaining_size.y;
			}
		}

		void page::size_remainings(jobject& _style_sheet, page_item* _pi, layout_context _ctx)
		{
			_ctx = get_remaining(_style_sheet, _pi, _ctx);
			for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
			{
				size_remaining(_style_sheet, child, _ctx);
			}
		}

		void page::size_children(jobject& _style_sheet, page_item* _pi, layout_context _ctx)
		{
			bool sheight = _pi->box.height.units == measure_units::percent_child;
			bool swidth = _pi->box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				if (_pi->layout == layout_types::canvas2d_column || _pi->layout == layout_types::canvas3d_column || _pi->layout == layout_types::column)
				{
					for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
					{
						auto ew = child->bounds.w + child->bounds.x - _pi->bounds.x;
						if (ew > sizes.x)
						{
							sizes.x = ew;
						}
						sizes.y += child->bounds.h;
					}
				}
				else if (_pi->layout == layout_types::canvas2d_row || _pi->layout == layout_types::canvas3d_row || _pi->layout == layout_types::row)
				{
					for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
					{
						auto eh = child->bounds.h + child->bounds.y - _pi->bounds.y;
						if (child->bounds.h > sizes.y)
						{
							sizes.y = child->bounds.h;
						}
						sizes.x += child->bounds.w;
					}
				}
				else 
				{
					for (auto child = _pi->get_first_child(); child != nullptr; child = child->get_next())
					{
						auto ew = child->bounds.w + child->bounds.x - _pi->bounds.x;
						if (ew > sizes.x)
						{
							sizes.x = ew;
						}
						auto eh = child->bounds.h + child->bounds.y - _pi->bounds.y;
						if (child->bounds.h > sizes.y)
						{
							sizes.y = child->bounds.h;
						}
					}

				}
				if (sheight)
				{
					_pi->bounds.h = sizes.y * _pi->box.height.amount;
				}
				if (swidth)
				{
					_pi->bounds.w = sizes.x * _pi->box.width.amount;
				}
			}
			size_remainings(_style_sheet, _pi, _ctx);
		}

		void page::size_item(jobject& _style_sheet, page_item* _pi, layout_context _ctx)
		{
			size_constant(_style_sheet, _pi, _ctx);
			size_aspect(_style_sheet, _pi, _ctx);
			size_remaining(_style_sheet, _pi, _ctx);
		}

		void page::size_items(jobject& _style_sheet, page_item* _pi, layout_context _ctx)
		{
			size_constants(_style_sheet, _pi, _ctx);
			size_aspects(_style_sheet, _pi, _ctx);
			size_remainings(_style_sheet, _pi, _ctx);
		}

		void page::position(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			switch (_item->box.x.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				_item->bounds.x = _item->box.x.amount * _ctx.container_size.x + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::pixels:
				_item->bounds.x = _item->box.x.amount + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				_item->bounds.x = _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			}

			switch (_item->box.y.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				_item->bounds.y = _item->box.y.amount * _ctx.container_size.y + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::pixels:
				_item->bounds.y = _item->box.y.amount + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				_item->bounds.y = _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			}

			auto schema = _style_sheet.get_schema();
			relative_ptr_type class_id = _item->slice.get_class_id();

			switch (_item->item_space.units)
			{
			case measure_units::font:
				_item->item_space_amount.x = _item->item_space.amount * 16.0;
				_item->item_space_amount.y = _item->item_space.amount * 16.0;
				break;
			case measure_units::font_golden_ratio:
				_item->item_space_amount.x = _item->item_space.amount * 16.0 / 1.618;
				_item->item_space_amount.y = _item->item_space.amount * 16.0 / 1.618;
				break;
			case measure_units::percent_aspect:
				_item->item_space_amount.x = _item->item_space.amount * _ctx.container_size.y;
				_item->item_space_amount.y = _item->item_space.amount * _ctx.container_size.x;
				break;
			case measure_units::percent_remaining:
				_item->item_space_amount.x = _item->item_space.amount * _ctx.remaining_size.x;
				_item->item_space_amount.y = _item->item_space.amount * _ctx.remaining_size.y;
				break;
			case measure_units::pixels:
				_item->item_space_amount.x = _item->item_space.amount;
				_item->item_space_amount.y = _item->item_space.amount;
				break;
			}

			if (!_item->slice.is_null() && _item->slice.has_field(schema->idf_rectangle))
			{
				auto rect = _item->slice.get_rectangle(schema->idf_rectangle);
				_item->bounds = rect;
			}
		}

		void page::positions(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{			
			visual_alignment _alignment = _item->alignment;
			layout_types _layout = _item->layout;

			if (_layout == layout_types::row || _layout == layout_types::canvas2d_row || _layout == layout_types::canvas3d_row)
			{
				if (_alignment == visual_alignment::align_near)
				{
					_ctx.flow_origin.x = 0;
					_ctx.flow_origin.y = 0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.x += (child->bounds.w);
						_ctx.flow_origin.x += _ctx.space_amount.x;
					}
				} 
				else if (_alignment == visual_alignment::align_far)
				{
					_ctx.flow_origin.x = _ctx.container_size.x;
					_ctx.flow_origin.y = 0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						_ctx.flow_origin.x -= (child->bounds.w);
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.x -= _ctx.space_amount.x;
					}
				}
				else if (_alignment == visual_alignment::align_center)
				{
					double w = 0.0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						w += child->bounds.w;
					}

					_ctx.flow_origin.x = (_ctx.container_size.x - w) / 2;
					_ctx.flow_origin.y = 0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.x += (child->bounds.w);
						_ctx.flow_origin.x += _ctx.space_amount.x;
					}
				}
			}
			else if (_layout == layout_types::column || _layout == layout_types::canvas2d_column || _layout == layout_types::canvas3d_column)
			{
				if (_alignment == visual_alignment::align_near)
				{
					_ctx.flow_origin.x = 0;
					_ctx.flow_origin.y = 0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.y += (child->bounds.h);
						_ctx.flow_origin.y += _ctx.space_amount.y;
					}
				}
				else if (_alignment == visual_alignment::align_far)
				{
					_ctx.flow_origin.x = 0;
					_ctx.flow_origin.y = _ctx.container_size.y;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						_ctx.flow_origin.y -= (child->bounds.h);
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.y -= _ctx.space_amount.x;
					}
				}
				else if (_alignment == visual_alignment::align_center)
				{
					double h = 0;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						h += child->bounds.h;
					}

					_ctx.flow_origin.x = 0;
					_ctx.flow_origin.y = (_ctx.container_size.y - h) / 2;

					for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
					{
						position(_style_sheet, child, _ctx);
						layout(_style_sheet, child, _ctx);
						_ctx.flow_origin.y += (child->bounds.h);
						_ctx.flow_origin.y += _ctx.space_amount.y;
					}
				}
			}
			else
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
				{
					position(_style_sheet, child, _ctx);
					layout(_style_sheet, child, _ctx);
				}
			}

			size_children(_style_sheet, _item, _ctx);
		}

		void page::styles(jobject& _style_sheet, int style_id, page_item *_item)
		{
			for (auto child = _item->get_first_child(); child != nullptr; child = child->get_next())
			{
				if (child->style_id <= 0) {
					child->style_id = style_id;
				}
			}
		}

		rectangle page::layout(jobject& _style_sheet, page_item* _item, layout_context _ctx)
		{
			auto children = this->where([_item](const value_reference<page_item>& _pir) { return _pir.item.parent_id == _item->id; })
				.select<page_item*, value_reference<page_item>>(&data, [](const value_reference<page_item>& _pir) {
				return &_pir.item;
					});

			_ctx.container_origin.x = _item->bounds.x;
			_ctx.container_origin.y = _item->bounds.y;
			_ctx.container_size.x = _item->bounds.w;
			_ctx.container_size.y = _item->bounds.h;
			_ctx.remaining_size = _ctx.container_size;
			_ctx.space_amount = { (double)_item->item_space.amount, (double)_item->item_space.amount};
			_ctx.flow_origin.x = 0;
			_ctx.flow_origin.y = 0;

			styles(_style_sheet, _item->style_id, _item);
			size_items(_style_sheet, _item, _ctx);
			positions(_style_sheet, _item, _ctx);

			return _item->bounds;
		}

		void page::arrange(double width, double height, jobject& _style_sheet, double _padding)
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
					size_item(_style_sheet, &pi.item, ctx);
					position(_style_sheet, &pi.item, ctx);
					layout(_style_sheet, &pi.item, ctx);
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
					database::layout_type_names[(int)_item.layout] << " " <<
					"(" << _item.bounds.x << ", " << _item.bounds.y << "  x  " << _item.bounds.w << "," << _item.bounds.h << ")" <<
					(_item.caption ? _item.caption : "") << std::endl;
			}
#endif

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
