
#include "corona.h"

#define TRACE_LAYOUT 01

namespace corona
{
	using namespace win32;

	namespace database
	{
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

		const control_base* control_base::find(int _id) const
		{
			const control_base* result = nullptr;
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

		std::weak_ptr<control_base> control_base::get(std::shared_ptr<control_base>& _root, int _id)
		{
			std::weak_ptr<control_base> result;
			if (_root->id == _id) {
				return _root;
			}
			else
			{
				for (auto child : _root->children)
				{
					result = get(child, _id);
					if (!result.expired()) {
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

		void control_base::create(std::weak_ptr<win32::win32ControllerHost> _host)
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

		row_layout& control_base::row_begin(int id)
		{
			return create<row_layout>(id);
		}

		column_layout& control_base::column_begin(int id)
		{
			return create<column_layout>(id);
		}

		absolute_layout& control_base::absolute_begin(int id)
		{
			return create<absolute_layout>(id);
		}

		control_base& control_base::end()
		{
			if (auto pparent = parent.lock()) {
				return *pparent.get();
			}
			else 
			{
				return *this;
			}
		}


		control_base& control_base::title(int id)
		{
			return create<title_control>(id);
		}

		control_base& control_base::subtitle(int id)
		{
			return create<subtitle_control>(id);
		}

		control_base& control_base::chaptertitle(int id)
		{
			return create<chaptertitle_control>(id);
		}

		control_base& control_base::chaptersubtitle(int id)
		{
			return create<chaptersubtitle_control>(id);
		}

		control_base& control_base::paragraph(int id)
		{
			return create<paragraph_control>(id);
		}

		control_base& control_base::code(int id)
		{
			return create<code_control>(id);
		}

		control_base& control_base::image(int id)
		{
			return create<image_control>(id);
		}

		control_base& control_base::title(std::string text, int id)
		{
			auto &tc = create<title_control>(id);
			tc.text = text;
			return *this;
		}

		control_base& control_base::subtitle(std::string text, int id)
		{
			auto &tc = create<subtitle_control>(id);
			tc.text = text;
			return *this;
		}

		control_base& control_base::chaptertitle(std::string text, int id)
		{
			auto &tc = create<chaptertitle_control>(id);
			tc.text = text;
			return *this;
		}

		control_base& control_base::chaptersubtitle(std::string text, int id)
		{
			auto &tc = create<chaptersubtitle_control>(id);
			tc.text = text;
			return *this;
		}

		control_base& control_base::paragraph(std::string text, int id)
		{
			auto &tc = create<paragraph_control>(id);
			tc.text = text;
			return *this;
		}

		control_base& control_base::code(std::string text, int id)
		{
			auto &tc = create<code_control>(id);
			tc.text = text;
			return *this;
		}


		control_base& control_base::label(int id)
		{
			auto &tc = create<static_control>(id);
			return *this;
		}

		control_base& control_base::button(int id)
		{
			auto &tc = create<button_control>(id);
			return *this;
		}

		control_base& control_base::listbox(int id)
		{
			auto &tc = create<listbox_control>(id);
			return *this;
		}

		control_base& control_base::combobox(int id)
		{
			auto &tc = create<combobox_control>(id);
			return *this;
		}

		control_base& control_base::edit(int id)
		{
			auto& tc = create<edit_control>(id);
			return *this;
		}

		control_base& control_base::scrollbar(int id)
		{
			auto& tc = create<scrollbar_control>(id);
			return *this;
		}

		control_base& control_base::listview(int id)
		{
			auto& tc = create<listview_control>(id);
			return *this;
		}

		control_base& control_base::treeview(int id)
		{
			auto& tc = create<treeview_control>(id);
			return *this;
		}

		control_base& control_base::header(int id)
		{
			auto& tc = create<header_control>(id);
			return *this;
		}

		control_base& control_base::toolbar(int id)
		{
			auto& tc = create<toolbar_control>(id);
			return *this;
		}

		control_base& control_base::statusbar(int id)
		{
			auto& tc = create<statusbar_control>(id);
			return *this;
		}

		control_base& control_base::hotkey(int id)
		{
			auto& tc = create<hotkey_control>(id);
			return *this;
		}

		control_base& control_base::animate(int id)
		{
			auto& tc = create<animate_control>(id);
			return *this;
		}

		control_base& control_base::richedit(int id)
		{
			auto& tc = create<richedit_control>(id);
			return *this;
		}

		control_base& control_base::draglistbox(int id)
		{
			auto& tc = create<draglistbox_control>(id);
			return *this;
		}

		control_base& control_base::rebar(int id)
		{
			auto& tc = create<rebar_control>(id);
			return *this;
		}

		control_base& control_base::comboboxex(int id)
		{
			auto& tc = create<comboboxex_control>(id);
			return *this;
		}

		control_base& control_base::datetimepicker(int id)
		{
			auto& tc = create<datetimepicker_control>(id);
			return *this;
		}

		control_base& control_base::monthcalendar(int id)
		{
			auto& tc = create<monthcalendar_control>(id);
			return *this;
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



		layout_context row_layout::get_remaining(layout_context _ctx)
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

		layout_context column_layout::get_remaining(layout_context _ctx)
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

		void column_layout::size_children(layout_context _ctx)
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

		void row_layout::size_children(layout_context _ctx)
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

		bool control_base::contains(point pt)
		{
			return rectangle_math::contains(bounds, pt.x, pt.y);
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

		control_base* control_base::set_align_base(visual_alignment _new_alignment)
		{
			alignment = _new_alignment;
			return this;
		}

		control_base* control_base::set_origin_base(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return this;
		}

		control_base* control_base::set_size_base(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return this;
		}

		control_base* control_base::set_position_base(layout_rect _new_layout)
		{
			box = _new_layout;
			return this;
		}

		control_base* control_base::set_spacing_base(measure _spacing)
		{
			this->item_space= _spacing;
			return this;
		}

		void row_layout::positions(layout_context _ctx)
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

		void column_layout::positions(layout_context _ctx)
		{
			if (alignment == visual_alignment::align_near)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
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

		void control_base::on_resize()
		{
			;
		}

		void draw_control::create(std::weak_ptr<win32::win32ControllerHost> _host)
		{
			host = _host;
			window = _host.lock()->createDirect2Window(id, bounds);
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
			if (auto pwindow = window.lock()) {
				pwindow->resize(bounds.w, bounds.h);
			}
			for (auto child : children) {
				child->on_resize();
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
					if (on_draw != nullptr) {
						on_draw(this);
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
			set_origin_base(0.0_px, 0.0_px);
			set_size_base(100.0_container, 3.0_fontgr);

			on_create = [this](draw_control* _src) 
			{				
				if (auto pwindow = this->window.lock()) 
				{
					pwindow->getContext().setSolidColorBrush(&this->text_fill_brush);
					pwindow->getContext().setTextStyle(&this->text_style );
				}
			};

			on_draw = [this](draw_control* _src) {
				if (auto pwindow = this->window.lock())
				{
					auto draw_bounds = pwindow->getContext().getCanvasSize();
					D2D1_COLOR_F colors = {};
					colors.a = 1.0;
					colors.r = .1;
					colors.g = .1;
					colors.b = .0;
					pwindow->getContext().getDeviceContext()->Clear(&colors);
					pwindow->getContext().drawText(text.c_str(), &draw_bounds, this->text_style.name, this->text_fill_brush.name);
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
			text_fill_brush.name = typeid(this).name();
			text_fill_brush.brushColor = toColor(_color.c_str());
			return *this;
		}

		text_display_control& text_display_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
		{
			text_style.name = typeid(this).name();
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

		void text_display_control::on_resize()
		{
			std::cout << "resize:" << text << " " << bounds.w << " " << bounds.h << std::endl;
			draw_control::on_resize();
		}

		image_control::image_control()
		{
		}

		title_control::title_control()
		{
			text_fill_brush.name = "title_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "title_text_style";
			text_style.fontName = "Open Sans;Arial";
			text_style.fontSize = 34;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		subtitle_control::subtitle_control()
		{
			text_fill_brush.name = "subtitle_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "subtitle_text_style";
			text_style.fontName = "Open Sans;Arial";
			text_style.fontSize = 25;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = true;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		chaptertitle_control::chaptertitle_control()
		{
			text_fill_brush.name = "chaptertitle_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "chaptertitle_text_style";
			text_style.fontName = "Century,Courier New,Arial";
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		chaptersubtitle_control::chaptersubtitle_control()
		{
			text_fill_brush.name = "chaptersubtitle_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "chaptersubtitle_text_style";
			text_style.fontName = "Century,Courier New,Arial";
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		paragraph_control::paragraph_control()
		{
			text_fill_brush.name = "chaptersubtitle_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "chaptersubtitle_text_style";
			text_style.fontName = "Century,Courier New,Arial";
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = true;
		}

		code_control::code_control()
		{
			text_fill_brush.name = "chaptersubtitle_text_fill";
			text_fill_brush.brushColor = toColor("#C0C0C0");

			text_style = {};
			text_style.name = "chaptersubtitle_text_style";
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

		void page::create(std::weak_ptr<win32::win32ControllerHost> _host)
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
			auto new_row = std::make_shared<row_layout>();
			root = new_row;
			return *new_row.get();
		}

		column_layout& page::column_begin(int id)
		{
			auto new_row = std::make_shared<column_layout>();
			root = new_row;
			return *new_row.get();
		}

		absolute_layout& page::absolute_begin(int id)
		{
			auto new_row = std::make_shared<absolute_layout>();
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
			layout_context ctx;
			ctx.space_amount = { 0.0, 0.0 };
			ctx.container_size = { width - pd, height - pd };
			ctx.container_origin = { _padding, _padding };
			ctx.flow_origin = { 0, 0 };
			ctx.remaining_size = ctx.container_size;

			if (root->box.height.units == measure_units::percent_child ||
				root->box.width.units == measure_units::percent_child)
				throw std::logic_error("Cannot use child based sizing on a root element");

//			std::cout << "Resizing:" << width << " " << height << std::endl;

			root->size_item(ctx);
			root->position(ctx);
			root->layout(ctx);
			root->on_resize();
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
