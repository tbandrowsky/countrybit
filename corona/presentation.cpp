
#include "corona.h"

#define TRACE_LAYOUT 01

namespace corona
{
	using namespace win32;

	namespace database
	{
		presentation_style_factory styles;

		int id_counter::id = 0;
		int id_counter::next() 
		{ 
			return ++id; 
		}

		int id_counter::check(int _id) { 
			if (_id > id) 
				id = _id;
			return id;
		}

		int control_base::debug_indent = 0;

		control_base* control_base::find(int _id)
		{
			control_base* result = nullptr;
			if (this->id == _id) {
				result = this;
			}
			else
			{
				for (auto child : children)
				{
					result = child->find(_id);
					if (result != nullptr) {
						break;
					}
				}
			}
			return result;
		}

		control_base *control_base::get(control_base* _root, int _id)
		{
			control_base *result = nullptr;
			if (_root->id == _id) {
				result = _root;
			}
			else
			{
				for (auto child : _root->children)
				{
					result = get(child.get(), _id);
					if (result) {
						return result;
					}
				}
			}
			return result;
		}

		void control_base::foreach(std::function<void(control_base * _root)> _item)
		{
			_item(this);
			for (auto child : children) {
				child->foreach(_item);
			}
		}

		void control_base::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{
			for (auto child : children) {
				child->create(_host);
			}
		}

		void control_base::destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void control_base::draw()
		{
			for (auto child : children) {
				child->draw();
			}
		}

		void control_base::apply(control_base& _ref)
		{

		}

		row_layout& container_control::row_begin(int id)
		{
			row_layout& temp = create<row_layout>(id);
			apply(temp);
			debug_indent += 2;
			return temp;
		}

		column_layout& container_control::column_begin(int id)
		{
			column_layout& temp = create<column_layout>(id);
			apply(temp);
			debug_indent += 2;
			return temp;
		}

		absolute_layout& container_control::absolute_begin(int id)
		{
			absolute_layout& temp = create<absolute_layout>(id);
			apply(temp);
			debug_indent += 2;
			return temp;
		}

		container_control& container_control::end()
		{
			debug_indent -= 2;
			if (debug_indent < 0) {
				debug_indent = 0;
			}
			if (parent) {
				auto& temp = *parent;
				auto string_name = typeid(temp).name();
				std::string indent(debug_indent, ' ');
				std::cout << indent << " " << typeid(*this).name() << " ->navigate " << string_name << std::endl;
				return temp;
			}
		}

		container_control& container_control::set_align(visual_alignment _new_alignment)
		{
			alignment = _new_alignment;
			return *this;
		}

		container_control& container_control::set_origin(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return *this;
		}

		container_control& container_control::set_size(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return *this;
		}

		container_control& container_control::set_background_color(solidBrushRequest _brushFill)
		{
			background_brush = _brushFill;
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& container_control::set_background_color(std::string _color)
		{
			background_brush.brushColor = toColor(_color.c_str());
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& container_control::set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		container_control& container_control::set_margin(measure _item_space)
		{
			margin = _item_space;
			return *this;
		}

		container_control& container_control::image(int id)
		{
			auto& tc = create<image_control>(id);
			return *this;
		}

		container_control& container_control::title(std::string text, int id)
		{
			auto &tc = create<title_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::subtitle(std::string text, int id)
		{
			auto &tc = create<subtitle_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::chaptertitle(std::string text, int id)
		{
			auto &tc = create<chaptertitle_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::chaptersubtitle(std::string text, int id)
		{
			auto &tc = create<chaptersubtitle_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::paragraph(std::string text, int id)
		{
			auto &tc = create<paragraph_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::code(std::string text, int id)
		{
			auto &tc = create<code_control>(id);
			apply(tc);
			tc.text = text;
			return *this;
		}

		container_control& container_control::label(int id)
		{
			auto &tc = create<label_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::label(std::string _text, int id)
		{
			auto& tc = create<label_control>(id);
			apply(tc);
			tc.set_text(_text);
			return *this;
		}

		container_control& container_control::push_button(int id)
		{
			auto& tc = create<pushbutton_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::radio_button(int id)
		{
			auto& tc = create<radiobutton_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::checkbox(int id)
		{
			auto& tc = create<checkbox_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::push_button(std::string _text, int id)
		{
			auto& tc = create<pushbutton_control>(id);
			apply(tc);
			tc.set_text(_text);
			return *this;
		}

		container_control& container_control::radio_button(std::string _text, int id)
		{
			auto& tc = create<radiobutton_control>(id);
			apply(tc);
			tc.set_text(_text);
			return *this;
		}

		container_control& container_control::checkbox(std::string _text, int id)
		{
			auto& tc = create<checkbox_control>(id);
			apply(tc);
			tc.set_text(_text);
			return *this;
		}

		container_control& container_control::listbox(int id)
		{
			auto &tc = create<listbox_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::combobox(int id)
		{
			auto &tc = create<combobox_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::edit(int id)
		{
			auto& tc = create<edit_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::scrollbar(int id)
		{
			auto& tc = create<scrollbar_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::listview(int id)
		{
			auto& tc = create<listview_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::treeview(int id)
		{
			auto& tc = create<treeview_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::header(int id)
		{
			auto& tc = create<header_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::toolbar(int id)
		{
			auto& tc = create<toolbar_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::statusbar(int id)
		{
			auto& tc = create<statusbar_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::hotkey(int id)
		{
			auto& tc = create<hotkey_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::animate(int id)
		{
			auto& tc = create<animate_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::richedit(int id)
		{
			auto& tc = create<richedit_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::draglistbox(int id)
		{
			auto& tc = create<draglistbox_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::rebar(int id)
		{
			auto& tc = create<rebar_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::comboboxex(int id)
		{
			auto& tc = create<comboboxex_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::datetimepicker(int id)
		{
			auto& tc = create<datetimepicker_control>(id);
			apply(tc);
			return *this;
		}

		container_control& container_control::monthcalendar(int id)
		{
			auto& tc = create<monthcalendar_control>(id);
			apply(tc);
			return *this;
		}

		double control_base::get_margin(measure margin)
		{
			double sz = 0.0;

			control_base& pi = *this;

			switch (margin.units) {
			case measure_units::pixels:
				sz = margin.amount;
				break;
			case measure_units::percent_container:
				sz = margin.amount * bounds.w;
				break;
			case measure_units::percent_remaining:
				sz = margin.amount * bounds.w;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
				double font_height = 12.0;
				sz = font_height * pi.box.width.amount;
				if (pi.box.width.units == measure_units::font_golden_ratio)
				{
					sz /= 1.618;
				}
				break;
			}
			return sz;
		}

		point control_base::get_size(rectangle _ctx, point _remaining)
		{
			point sz;

			control_base& pi = *this;

			if (pi.box.width.units == measure_units::pixels)
			{
				sz.x = pi.box.width.amount;
			}
			else if (box.width.units == measure_units::percent_remaining)
			{
				sz.x = box.width.amount * _remaining.x;
			}
			else if (pi.box.width.units == measure_units::percent_container)
			{
				sz.x = pi.box.width.amount * _ctx.w;
			}
			else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
			{
				double font_height = get_font_size();
				sz.x = font_height * pi.box.width.amount;
				if (pi.box.width.units == measure_units::font_golden_ratio)
				{
					sz.x /= 1.618;
				}
			}

			if (pi.box.height.units == measure_units::pixels)
			{
				sz.y = pi.box.height.amount;
			}
			else if (box.height.units == measure_units::percent_remaining)
			{
				sz.y = pi.box.height.amount * _remaining.y;
			}
			else if (pi.box.height.units == measure_units::percent_container)
			{
				sz.y = pi.box.height.amount * _ctx.h;
			}
			else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = get_font_size();
				sz.y = font_height * pi.box.height.amount;
				if (pi.box.height.units == measure_units::font_golden_ratio)
				{
					sz.y *= 1.618;
				}
			}

			if (box.width.units == measure_units::percent_aspect)
			{
				sz.x = box.width.amount * bounds.h;
			}

			if (box.height.units == measure_units::percent_aspect)
			{
				sz.x = box.height.amount * bounds.w;
			}

			return sz;
		}

		point control_base::get_position(rectangle _ctx)
		{
			point pos;

			switch (box.x.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				pos.x = box.x.amount * _ctx.w;
				break;
			case measure_units::pixels:
				pos.x = box.x.amount;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:				
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				pos.x = 0;
				break;
			}

			switch (box.y.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				pos.y = box.y.amount * _ctx.h;
				break;
			case measure_units::pixels:
				pos.y = box.y.amount;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				pos.y = 0;
				break;
			}

			return pos;
		}

		point control_base::get_remaining(point _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };
			pt = _ctx - pt;
			return pt;
		}

		void container_control::apply(control_base& _ref)
		{
			if (item_alignment != visual_alignment::align_none) {
				_ref.alignment = item_alignment;
			}

			if (item_box.height.amount > 0 && item_box.width.amount > 0)
			{
				_ref.box = item_box;
			}

			if (item_margin.amount > 0)
			{
				_ref.margin = item_margin;
			}
		}

		container_control& container_control::set_item_align(visual_alignment _new_alignment)
		{
			item_alignment = _new_alignment;
			return *this;
		}

		container_control& container_control::set_item_origin(measure _x, measure _y)
		{
			item_box.x = _x;
			item_box.y = _y;
			return *this;
		}

		container_control& container_control::set_item_size(measure _width, measure _height)
		{
			if (_width.amount > 0)
				item_box.width = _width;
			if (_height.amount > 0)
				item_box.height = _height;
			return *this;
		}

		container_control& container_control::set_item_position(layout_rect _new_layout)
		{
			item_box = _new_layout;
			return *this;
		}

		container_control& container_control::set_item_margin(measure _item_margin)
		{
			item_margin = _item_margin;
			return *this;
		}

		point row_layout::get_remaining(point _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.width.units != measure_units::percent_remaining)
				{
					pt.x += child->bounds.w;
				}
			}

			pt = _ctx - pt;
			return pt;
		}

		point column_layout::get_remaining(point _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.height.units != measure_units::percent_remaining)
				{
					pt.y += child->bounds.h;
				}
			}

			pt = _ctx - pt;
			return pt;
		}

		bool control_base::contains(point pt)
		{
			return rectangle_math::contains(bounds, pt.x, pt.y);
		}

		void control_base::arrange(rectangle _bounds, int zorder)
		{
			bounds = _bounds;
			margin_amount.x = margin_amount.y = get_margin(margin);
			on_resize();
		}

		void control_base::arrange_children(rectangle _bounds, 
			int zorder,
			std::function<point(rectangle *_bounds, control_base*)> _initial_origin,
			std::function<point (point* _origin, rectangle *_bounds, control_base *)> _next_origin)
		{
			point origin = { _bounds.x, _bounds.y, 0 };
			point remaining = { _bounds.w, _bounds.h, 0 };

			on_resize();

			origin = _initial_origin(&bounds, this);

			for (auto child : children)
			{

				auto sz = child->get_size(_bounds, remaining);
				auto pos = child->get_position(_bounds);

				child->bounds.x = origin.x + pos.x;
				child->bounds.y = origin.y + pos.y;
				child->bounds.w = sz.x;
				child->bounds.h = sz.y;

				origin = _next_origin(&origin, &bounds, child.get());
			}

			remaining = get_remaining(remaining);

			origin = _initial_origin(&bounds, this);
	//		std::cout << "origin start:" << typeid(*this).name() << origin.x << ", " << origin.y << std::endl;

			for (auto child : children)
			{
				auto sz = child->get_size(_bounds, remaining);
				auto pos = child->get_position(_bounds);

//				std::cout << typeid(*this).name() << " sz:" << typeid(*child).name() << " " << sz.x << ", " << sz.y << std::endl;
//				std::cout << typeid(*this).name() << " pos:" << typeid(*child).name() << " " << pos.x << ", " << pos.y << std::endl;

				child->bounds.x = origin.x + pos.x;
				child->bounds.y = origin.y + pos.y;
				child->bounds.w = sz.x;
				child->bounds.h = sz.y;

				std::cout << typeid(*this).name() << " arrange:" << typeid(*child).name() << " " << child->bounds.x << "," << child->bounds.y << " x " << child->bounds.w << "," << child->bounds.h << std::endl;

				origin = _next_origin(&origin, &bounds, child.get());

//				std::cout << "   next origin:" << typeid(*this).name() << "  child:" << typeid(*child).name() << " " << origin.x << ", " << origin.y << std::endl;

				child->arrange(child->bounds, zorder + 1 );
			}

		}

		void absolute_layout::arrange(rectangle _bounds, int zorder)
		{
			bounds = _bounds;
			margin_amount.x = margin_amount.y = get_margin(margin);

			point origin = { _bounds.x, _bounds.y, 0 };
			point remaining = { _bounds.w, _bounds.h, 0 };

			arrange_children(bounds, zorder,
				[this](rectangle* _bounds, control_base* _item) {
					point temp = { _bounds->x, _bounds->y };
					return temp;
				},
				[this](point* _origin, rectangle* _bounds, control_base* _item) {
					point temp = { _bounds->x, _bounds->y };
					return temp;
				}
			);
		}


		void row_layout::arrange(rectangle _bounds, int zorder)
		{
			point origin = { 0, 0, 0 };
			margin_amount.x = margin_amount.y = get_margin(margin);
			auto item_margin_amount = get_margin(item_margin);

			bounds = _bounds;

			if (alignment == visual_alignment::align_near)
			{
				arrange_children(bounds, zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {
						point temp = { 0, 0, 0 };
						temp.x = _bounds->x + item_margin_amount;
						temp.y = _bounds->y;
						return temp;
					},
					[this](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.x += _item->bounds.w;
						temp.x += _item->margin_amount.x;
						return temp;
					}
					);

			}
			else if (alignment == visual_alignment::align_far)
			{
				arrange_children(bounds, zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {

						double w = item_margin_amount;
						point remaining;
						remaining.x = _bounds->w - item_margin_amount * 2.0;
						remaining.y = _bounds->h;

						for (auto child : children)
						{
							auto sz = child->get_size(*_bounds, remaining);
							w += sz.x;
							w += get_margin(child->margin);
						}

						point temp = { 0, 0, 0 };
						temp.x = _bounds->x + _bounds->w - w;
						temp.y = _bounds->y;
						return temp;
					},
					[this](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.x -= _item->bounds.w + get_margin(_item->margin);
						return temp;
					}
					);
			}
			else if (alignment == visual_alignment::align_center)
			{

				arrange_children(bounds, zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {

						double w = 0.0;
						point origin = { 0, 0, 0 };
						point remaining = { 0, 0, 0 };
						remaining.x = _bounds->w;
						remaining.y = _bounds->h;
						remaining = this->get_remaining(remaining);

						for (auto child : children)
						{
							auto sz = child->get_size(*_bounds, remaining);
							w += sz.x;
						}

						origin.x = (bounds.x + bounds.w - (w+ item_margin_amount)) / 2;
						origin.y = bounds.y;
						return origin;
					},
					[this](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.x += (_item->bounds.w + _item->margin_amount.x);
						return temp;
					}
				);
			}
		}

		void column_layout::arrange(rectangle _bounds, int zorder)
		{			
			point origin = { 0, 0, 0 };
			margin_amount.x = margin_amount.y = get_margin(margin);
			auto item_margin_amount = get_margin(item_margin);

			bounds = _bounds;

			if (alignment == visual_alignment::align_near)
			{
				arrange_children(bounds,
					zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {
						point temp = { 0, 0, 0 };
						temp.x = _bounds->x;
						temp.y = _bounds->y + item_margin_amount;
						return temp;
					},
					[this, item_margin_amount](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.y += _item->bounds.h;
						temp.y += _item->margin_amount.y;
						return temp;
					}
				);

			}
			else if (alignment == visual_alignment::align_far)
			{
				arrange_children(bounds, zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {
						point temp = { 0, 0, 0 };

						double h = item_margin_amount;
						point remaining = { };
						remaining.x = _bounds->w;
						remaining.y = _bounds->h;
						remaining = this->get_remaining(remaining);

						for (auto child : children)
						{
							auto sz = child->get_size(*_bounds, remaining);
							h += sz.y;
						}

						temp.x = _bounds->x;
						temp.y = _bounds->y +_bounds->h - h;
						return temp;
					},
					[this, item_margin_amount](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.y += (_item->bounds.h + _item->margin_amount.y);
						return temp;
					}
				);
			}
			else if (alignment == visual_alignment::align_center)
			{

				arrange_children(bounds, zorder,
					[this, item_margin_amount](rectangle* _bounds, control_base* _item) {

						double h = 0.0;
						point origin = { 0, 0, 0 };
						point remaining = { 0, 0, 0 };
						remaining.x = _bounds->w;
						remaining.y = _bounds->h;
						remaining = this->get_remaining(remaining);

						for (auto child : children)
						{
							auto sz = child->get_size(*_bounds, remaining);
							h += sz.y;
						}

						origin.x = bounds.x;
						origin.y = (bounds.y + bounds.h - h) / 2;
						return origin;
					},
					[this, item_margin_amount](point* _origin, rectangle* _bounds, control_base* _item) {
						point temp = *_origin;
						temp.y += (_item->bounds.h + _item->margin_amount.y);
						return temp;
					}
				);
			}
		}

		void control_base::on_resize()
		{
			auto ti = typeid(*this).name();
//			std::cout << "resize control_base:" << ti << " " << bounds.x << "," << bounds.y << " x " << bounds.w << " " << bounds.h << std::endl;
		}


		container_control::container_control()
		{
			parent = nullptr;
			id = id_counter::next();
		}

		container_control::container_control(container_control* _parent, int _id)
		{
			parent = _parent;
			id = id_counter::next();
		}

		draw_control::draw_control()
		{
			background_brush = {};
			parent = nullptr;
			id = id_counter::next();
		}

		draw_control::draw_control(container_control* _parent, int _id)
		{
			background_brush = {};
			parent = _parent;
			id = _id;
		}

		void draw_control::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{

			host = _host;
			if (auto phost = _host.lock()) {
				window = phost->createDirect2Window(id, bounds);
			}
			if (on_create) {
				on_create(this);
			}
			for (auto child : children) {
				child->create(_host);
			}
		}

		void draw_control::destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void draw_control::on_resize()
		{
			auto ti = typeid(*this).name();
//			std::cout << "resize draw_control:" << ti << " " << bounds.x << "," << bounds.y << " x " << bounds.w << " " << bounds.h << std::endl;

			if (auto pwindow = window.lock()) {
				pwindow->moveWindow(bounds.x, bounds.y, bounds.w, bounds.h);
			}
		}

		void draw_control::draw()
		{
			bool adapter_blown_away = false;

			if (auto pwindow = window.lock()) 
			{
				pwindow->beginDraw(adapter_blown_away);
				if (!adapter_blown_away)
				{
					auto context = pwindow->getContext();

					auto& bc = background_brush.brushColor;

					if (background_brush.active) 
					{
						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor(bc);
						dc->Clear(color);
					}

					if (on_draw != nullptr) {
						on_draw(this);
					}
					else 
					{
						
					}
				}
				pwindow->endDraw(adapter_blown_away);
			}
			for (auto child : children) {
				child->draw();
			}
		}
 
		/*
		class scrollbar_control : public windows_control<WTL::CScrollBar, WS_VISIBLE | WS_BORDER | WS_CHILD>
		{
		public:
		};
		*/

		text_display_control::text_display_control()
		{
			init();
		}

		text_display_control::text_display_control(container_control* _parent, int _id)
			: draw_control(_parent, _id)
		{
			init();
		}

		void text_display_control::init()
		{
			set_origin(0.0_px, 0.0_px);
			set_size(1.0_container, 1.2_fontgr);

			on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					pwindow->getContext().setSolidColorBrush(&this->text_fill_brush);
					pwindow->getContext().setTextStyle(&this->text_style);
				}
			};

			on_draw = [this](draw_control* _src) {
				if (auto pwindow = this->window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = bounds;

						auto bdips = pwindow->getBoundsDips();

						bounds.x = 0;
						bounds.y = 0;

						pwindow->getContext().drawText(text.c_str(), &draw_bounds, this->text_style.name, this->text_fill_brush.name);
//						pwindow->getContext().drawRectangle(&draw_bounds, this->text_fill_brush.name, 4, nullptr);
					}
				}
			};
		}

		text_display_control& text_display_control::set_text(std::string _text)
		{
			text = _text;
			return *this;
		}

		text_display_control& text_display_control::set_text_fill(solidBrushRequest _brushFill)
		{
			text_fill_brush = _brushFill;
			return *this;
		}

		text_display_control& text_display_control::set_text_fill(std::string _color)
		{
			text_fill_brush.name = typeid(*this).name();
			text_fill_brush.brushColor = toColor(_color.c_str());
			return *this;
		}

		text_display_control& text_display_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
		{
			text_style.name = typeid(*this).name();
			text_style.fontName = _font_name;
			text_style.fontSize = _font_size;
			text_style.bold = _bold;
			text_style.underline = _underline;
			text_style.italics = _italic;
			text_style.strike_through = _strike_through;
			return *this;
		}

		text_display_control& text_display_control::set_text_style(textStyleRequest request)
		{
			text_style = request;
			return *this;
		}

		image_control::image_control()
		{
		}


		void title_control::set_default_styles()
		{

			auto& st = styles.get_style();

			background_brush.name = "title_fill";
			background_brush.brushColor = toColor(st.TitleBackgroundColor);

			text_fill_brush.name = "title_text_fill";
			text_fill_brush.brushColor = toColor(st.TitleTextColor);

			text_style = {};
			text_style.name = "title_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 34;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		title_control::title_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		title_control::title_control()
		{
			set_default_styles();
		}


		void subtitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "subtitle_fill";
			background_brush.brushColor = toColor(st.SubtitleBackgroundColor);

			text_fill_brush.name = "subtitle_text_fill";
			text_fill_brush.brushColor = toColor(st.SubtitleTextColor);

			text_style = {};
			text_style.name = "subtitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 25;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = true;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		subtitle_control::subtitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		subtitle_control::subtitle_control()
		{
			set_default_styles();
		}

		void chaptertitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "chaptertitle_fill";
			background_brush.brushColor = toColor(st.ChapterTitleBackgroundColor);

			text_fill_brush.name = "chaptertitle_text_fill";
			text_fill_brush.brushColor = toColor(st.ChapterTitleTextColor);

			text_style = {};
			text_style.name = "chaptertitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		chaptertitle_control::chaptertitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		chaptertitle_control::chaptertitle_control()
		{
			set_default_styles();
		}

		void chaptersubtitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "chaptersubtitle_fill";
			background_brush.brushColor = toColor(st.SubchapterTitleBackgroundColor);

			text_fill_brush.name = "chaptersubtitle_text_fill";
			text_fill_brush.brushColor = toColor(st.SubchapterTitleTextColor);

			text_style = {};
			text_style.name = "chaptersubtitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}


		chaptersubtitle_control::chaptersubtitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		chaptersubtitle_control::chaptersubtitle_control()
		{
			set_default_styles();
		}

		void paragraph_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "paragraph_fill";
			background_brush.brushColor = toColor(st.ParagraphBackgroundColor);

			text_fill_brush.name = "paragraph_text_fill";
			text_fill_brush.brushColor = toColor(st.ParagraphTextColor);

			text_style = {};
			text_style.name = "paragraph_text_style";
			text_style.fontName = styles.get_style().PrimaryFont;
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		paragraph_control::paragraph_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		paragraph_control::paragraph_control()
		{
			set_default_styles();
		}
		
		void code_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "paragraph_fill";
			background_brush.brushColor = toColor(st.CodeBackgroundColor);

			text_fill_brush.name = "paragraph_text_fill";
			text_fill_brush.brushColor = toColor(st.CodeTextColor);

			text_style = {};
			text_style.name = "code_text_style";
			text_style.fontName = "Cascadia Mono,Courier New";
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = false;
		}

		code_control::code_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		code_control::code_control()
		{
			set_default_styles();
		}

		void label_control::set_default_styles()
		{
			text_fill_brush.name = "label_text_fill";
			text_fill_brush.brushColor = toColor(styles.get_style().TextColor.c_str());

			text_style = {};
			text_style.name = "label_text_style";
			text_style.fontName = styles.get_style().PrimaryFont;
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_near;
			text_style.vertical_align = visual_alignment::align_far;
			text_style.wrap_text = false;
		}

		label_control::label_control()
		{
			set_default_styles();
		}

		label_control::label_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		comboboxex_control::comboboxex_control()
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		comboboxex_control::comboboxex_control(container_control* _parent, int _id) : windows_control<WTL::CComboBoxEx, ComboExWindowStyles>(_parent, _id)
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		void comboboxex_control::data_changed()
		{
			if (window.IsWindow()) {
				window.ResetContent();
				for (auto element : choices.items.items())
				{
					auto c = element.value();
					int lid = c[choices.id_field].template get<int>();
					std::string description = c[choices.text_field].template get<std::string>();

					COMBOBOXEXITEM cbex = {};
					cbex.mask = CBEIF_TEXT | CBEIF_LPARAM;
					cbex.iItem = -1;
					cbex.pszText = (LPTSTR)description.c_str();
					cbex.iImage = 0;
					cbex.iSelectedImage = 0;
					cbex.iIndent = 0;
					cbex.lParam = lid;
					window.InsertItem(&cbex);
				}
			}
		}

		void comboboxex_control::set_list(list_data& _choices)
		{
			choices = _choices;
			data_changed();
		}

		void comboboxex_control::on_create()
		{
			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(bounds);

				RECT r;
				r.left = boundsPixels.x;
				r.top = boundsPixels.y;
				r.right = boundsPixels.x + bounds.w;
				r.bottom = boundsPixels.y + text_style.fontSize * 8;
				window.MoveWindow(&r);
			}

			data_changed();
		}

		void comboboxex_control::on_resize()
		{
			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(control_base::bounds);

				RECT r;
				r.left = boundsPixels.x;
				r.top = boundsPixels.y;
				r.right = boundsPixels.x + boundsPixels.w;
				r.bottom = boundsPixels.y + windows_control<WTL::CComboBoxEx, ComboExWindowStyles>::text_style.fontSize * 8;
				windows_control<WTL::CComboBoxEx, ComboExWindowStyles>::window.MoveWindow(&r);
			}
		}



		void richedit_control::set_html(const std::string& _text)
		{

		}

		std::string richedit_control::get_html()
		{
			return "";
		}

		void datetimepicker_control::set_text(const std::string& _text)
		{

		}

		std::string datetimepicker_control::get_text()
		{
			return "";
		}

		/*
		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD>
		{
		public:
		};
		*/

		bool animate_control::open(const std::string& _name)
		{
			return window.Open(_name.c_str());
		}

		bool animate_control::open(DWORD resource_id)
		{
			return window.Open(resource_id);
		}

		bool animate_control::play(UINT from, UINT to, UINT rep)
		{
			return window.Play(from, to, rep);
		}

		bool animate_control::play()
		{
			return window.Play(0, -1, 1);
		}

		bool animate_control::stop()
		{
			return window.Stop();
		}

		page::page(const char* _name)
		{
			name = _name != nullptr ? _name : "Test";
			root = std::make_shared<column_layout>();
		}

		page::~page()
		{
			destroy();
		}

		void page::clear()
		{
			destroy();
			root = std::make_shared<column_layout>();
		}

		void page::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{
			if (auto whost = _host.lock()) {
				auto pos = whost->getWindowClientPos();
				arrange(pos.w, pos.h);
				if (root.get())
				{
					root->create(_host);
				}
			}
		}

		void page::destroy()
		{
			if (root.get())
			{
				root->destroy();
			}
		}

		void page::draw()
		{
			if (root.get())
			{
				root->draw();
			}
		}

		void page::update(double _elapsedSeconds, double _totalSeconds)
		{
			if (update_event) {
				update_event(this, _elapsedSeconds, _totalSeconds);
			}
		}

		row_layout& page::row_begin(int id)
		{
			std::cout << "create: row"<< std::endl;
			auto new_row = std::make_shared<row_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		column_layout& page::column_begin(int id)
		{
			std::cout << "create: column" << std::endl;
			auto new_row = std::make_shared<column_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		absolute_layout& page::absolute_begin(int id)
		{
			std::cout << "create: begin" << std::endl;
			auto new_row = std::make_shared<absolute_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		presentation::presentation()
		{
			;
		}

		presentation::~presentation()
		{
			;
		}

		void page::arrange(double width, double height, double _padding)
		{

			double pd = _padding * 2.0;
			rectangle bounds;
			bounds.x = 0;
			bounds.y = 0;
			bounds.w = width;
			bounds.h = height;

			std::cout << "page arrange: " << bounds.w << " " << bounds.h << std::endl;

			int zOrder = 0;
			root->arrange(bounds, zOrder);
			std::cout << std::endl;
			control_base::debug_indent = 0;
		}

		void page::on_key_up(int _control_id, std::function< void(key_up_event) > handler)
		{
			auto evt = std::make_shared<key_up_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_up = handler;
			key_up_events[_control_id] = evt;
		}

		void page::on_key_down(int _control_id, std::function< void(key_down_event) >  handler)
		{
			auto evt = std::make_shared<key_down_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_down = handler;
			key_down_events[_control_id] = evt;
		}

		void page::on_mouse_move(int _control_id, std::function< void(mouse_move_event) > handler)
		{
			auto evt = std::make_shared<mouse_move_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_move = handler;
			mouse_move_events[_control_id] = evt;
		}

		void page::on_mouse_click(int _control_id, std::function< void(mouse_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_click_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_click = handler;
			mouse_click_events[_control_id] = evt;
		}

		void page::on_item_changed(int _control_id, std::function< void(item_changed_event) > handler)
		{
			auto evt = std::make_shared<item_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			item_changed_events[_control_id] = evt;
		}

		void page::on_list_changed(int _control_id, std::function< void(list_changed_event) > handler)
		{
			auto evt = std::make_shared<list_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			list_changed_events[_control_id] = evt;
		}

		void page::on_update(update_function fnc)
		{
			update_event = fnc;
		}

		void page::handle_key_up(int _control_id, key_up_event evt)
		{
			if (key_up_events.contains(_control_id)) {
				key_up_events[_control_id]->on_key_up(evt);
			}
		}

		void page::handle_key_down(int _control_id, key_down_event evt)
		{
			if (key_down_events.contains(_control_id)) {
				key_down_events[_control_id]->on_key_down(evt);
			}
		}

		void page::handle_mouse_move(int _control_id, mouse_move_event evt)
		{
			mouse_move_event* pevt = &evt;

			if (mouse_move_events.contains(_control_id)) {
				auto& ptrx = mouse_move_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->bounds.x;
					evt.relative_point.y = evt.absolute_point.y - temp->bounds.y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_move(evt);
				}
			}

			if (!_control_id) 
			{
				for (auto evh : mouse_move_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->bounds.x;
						evt.relative_point.y = evt.absolute_point.y - lck->bounds.y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_move(evt);
					}
				}
			}

		}

		void page::handle_mouse_click(int _control_id, mouse_click_event evt)
		{
			mouse_click_event* pevt = &evt;

			if (mouse_click_events.contains(_control_id)) {
				auto& ptrx = mouse_click_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->bounds.x;
					evt.relative_point.y = evt.absolute_point.y - temp->bounds.y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_click(evt);
				}
			}

			if (!_control_id)
			{
				for (auto evh : mouse_click_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->bounds.x;
						evt.relative_point.y = evt.absolute_point.y - lck->bounds.y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_click(evt);
					}
				}
			}
		}

		void page::handle_item_changed(int _control_id, item_changed_event evt)
		{
			item_changed_event* pevt = &evt;

			if (item_changed_events.contains(_control_id)) {
				auto& ptrx = item_changed_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_change(evt);
				}
			}
		}

		void page::handle_list_changed(int _control_id, list_changed_event evt)
		{
			list_changed_event* pevt = &evt;

			if (list_changed_events.contains(_control_id)) {
				auto& ptrx = list_changed_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_change(evt);
				}
			}
		}

		page& presentation::create_page(std::string _name)
		{
			auto new_page = std::make_shared<page>();
			pages[_name] = new_page;
			if (current_page.expired()) {
				current_page = new_page;
			}
			return *new_page.get();
		}

		void presentation::select_page(const std::string& _page_name)
		{
			if (pages.contains(_page_name)) {
				current_page = pages[_page_name];
			}
		}

		void presentation::onCreated()
		{
			auto cp = current_page.lock();
			if (cp) {
				auto host = getHost();
				auto post = host->getWindowClientPos();
				cp->arrange(post.w, post.h);
				cp->create(host);
			}
		}

		bool presentation::drawFrame()
		{
			auto cp = current_page.lock();
			if (cp) {
				cp->draw();
			}
			return false;
		}

		bool presentation::update(double _elapsedSeconds, double _totalSeconds)
		{
			auto cp = current_page.lock();
			if (cp) {
				cp->update(_elapsedSeconds, _totalSeconds );
			}
			return true;
		}

		void presentation::keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key)
		{
			auto cp = current_page.lock();
			if (cp) {
				auto children = win->getChildren();
				key_down_event kde;
				kde.control_id = 0;
				kde.key = _key;
				cp->handle_key_down(0, kde);
			}
		}

		void presentation::keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key)
		{
			auto cp = current_page.lock();
			if (cp) {
				key_up_event kde;
				kde.control_id = 0;
				kde.key = _key;
				cp->handle_key_up(0, kde);
			}
		}

		void presentation::mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			auto cp = current_page.lock();
			if (cp) {
				mouse_move_event kde;
				kde.control_id = 0;
				kde.absolute_point = *_point;
				cp->handle_mouse_move(0, kde);
			}
		}

		void presentation::mouseClick(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			auto cp = current_page.lock();
			if (cp) {
				mouse_click_event kde;
				kde.control_id = 0;
				kde.absolute_point = *_point;
				cp->handle_mouse_click(0, kde);
			}
		}

		void presentation::pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color)
		{
			;
		}

		void presentation::onCommand(int buttonId)
		{
			;
		}

		void presentation::onTextChanged(int textControlId)
		{
			std::string newText = getHost()->getEditText(textControlId);
		}

		void presentation::onDropDownChanged(int dropDownId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getComboSelectedText(dropDownId);
				int index = ptr->getComboSelectedIndex(dropDownId);
				int value = ptr->getComboSelectedValue(dropDownId);
				list_changed_event lce;
				lce.control_id = dropDownId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(dropDownId, lce);
				}
			}
		}

		void presentation::onListBoxChanged(int dropDownId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getListSelectedText(dropDownId);
				int index = ptr->getListSelectedIndex(dropDownId);
				int value = ptr->getListSelectedValue(dropDownId);
				list_changed_event lce;
				lce.control_id = dropDownId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(dropDownId, lce);
				}
			}
		}

		void presentation::onListViewChanged(int listViewId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getListViewSelectedText(listViewId);
				int index = ptr->getListViewSelectedIndex(listViewId);
				int value = ptr->getListViewSelectedValue(listViewId);
				list_changed_event lce;
				lce.control_id = listViewId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(listViewId, lce);
				}
			}
		}

		int presentation::onHScroll(int controlId, win32::scrollTypes scrollType)
		{
			return 0;
		}

		int presentation::onVScroll(int controlId, win32::scrollTypes scrollType)
		{
			return 0;
		}

		int presentation::onResize(const rectangle& newSize, double d2dScale)
		{
			auto pg = current_page.lock();
			if (pg) {
				pg->arrange(newSize.w, newSize.h);
			}
			return 0;
		}

		int presentation::onSpin(int controlId, int newPosition)
		{
			int value = newPosition;
			return 0;
		}

	}
}
