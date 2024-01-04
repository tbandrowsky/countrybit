
#ifndef CORONA_PRESENTATION_CONTROLS_CALICO_H
#define CORONA_PRESENTATION_CONTROLS_CALICO_H

#include "corona-presentation-controls-dx.hpp"
#include "corona-presentation-controls-dx-container.hpp"
#include "corona-presentation-controls-dx-text.hpp"
#include "corona-presentation-menu.hpp"
#include "corona-calicoclient.hpp"
#include "corona-dataplane.hpp"

namespace corona
{
	class calico_button_control : public gradient_button_control
	{
	public:

		std::string					text;
		solidBrushRequest			text_fill_brush;
		textStyleRequest			text_style;
		double						icon_width;
		std::string					source_name;
		std::string					function_name;
		json						function_parameters;
		std::shared_ptr<data_lake>	lake;

		calico_button_control() 
		{
			init();
		}

		calico_button_control(const calico_button_control& _src) : gradient_button_control(_src) {
			init();
			text = _src.text;
			text_fill_brush = _src.text_fill_brush;
			text_style = _src.text_style;
			icon_width = _src.icon_width;
		}
		calico_button_control(container_control_base* _parent, int _id);
		virtual ~calico_button_control() { ; }

		void init();
		virtual double get_font_size() { return text_style.fontSize; }
		calico_button_control& set_text(std::string _text);
		calico_button_control& set_text_fill(solidBrushRequest _brushFill);
		calico_button_control& set_text_fill(std::string _color);
		calico_button_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
		calico_button_control& set_text_style(textStyleRequest request);

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

		virtual void draw_button(std::function<void(gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape)
		{
			if (auto pwindow = window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					draw_bounds.x = 0;
					draw_bounds.y = 0;

					auto& context = pwindow->getContext();

					if (mouse_left_down.value())
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceDown.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(this, &face_bounds, &foregroundDown);
					}
					else if (mouse_over.value())
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceOver.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(this, &face_bounds, &foregroundOver);
					}
					else
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceNormal.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(this, &face_bounds, &foregroundNormal);
					}
				}
			}
		}
	};

	calico_button_control::calico_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
	{
		init();
	}

	void calico_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
			{
				lake->call_function(source_name, function_name, function_parameters);
			});
	}

	void calico_button_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(200.0_px, 1.0_container);
		icon_width = 30;

		auto st = styles.get_style();

		text_style = {};
		text_style.name = "tab_text_style";
		text_style.fontName = st->PrimaryFont;
		text_style.fontSize = 14;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = st->PrevailingAlignment;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;

		auto ctrl = this;

		on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					pwindow->getContext().setSolidColorBrush(&this->text_fill_brush);
					pwindow->getContext().setTextStyle(&this->text_style);
				}
			};

		on_draw = [this](control_base* _item)
			{
				if (auto pwindow = window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						std::function<void(gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						point shape_origin;
						point* porigin = &shape_origin;

						auto& context = pwindow->getContext();
						auto pcontext = &context;

						draw_shape = [this, porigin, pcontext](gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {

							rectangle icon_bounds = *_bounds;

							if (is_mouse_left_down()) {
								point start;
								point stop;
								start.x = icon_bounds.x + icon_width / 2;
								start.y = icon_bounds.y;
								stop.x = start.x;
								stop.y = icon_bounds.y + icon_bounds.h;
								pcontext->drawLine(&start, &stop, _foreground->name, 4);
								_bounds->y -= 4;
							}
							else
							{
								point center;
								point radius;
								center.x = icon_bounds.x + icon_width / 2;
								center.y = icon_bounds.y + icon_bounds.h / 2;
								radius.x = icon_width / 2;
								radius.y = icon_bounds.h / 2;
								pcontext->drawEllipse(&center, &radius, _foreground->name, 4, nullptr);
								_bounds->y -= 8;
							}

							_bounds->x += icon_width + 4;
							_bounds->w -= icon_width + 4;

							pcontext->drawText(text.c_str(), _bounds, this->text_style.name, _foreground->name);

							};

						draw_button(draw_shape);
					}
				}
			};
	}

	calico_button_control& calico_button_control::set_text(std::string _text)
	{
		text = _text;
		return *this;
	}

	calico_button_control& calico_button_control::set_text_fill(solidBrushRequest _brushFill)
	{
		text_fill_brush = _brushFill;
		return *this;
	}

	calico_button_control& calico_button_control::set_text_fill(std::string _color)
	{
		text_fill_brush.name = typeid(*this).name();
		text_fill_brush.brushColor = toColor(_color);
		return *this;
	}

	calico_button_control& calico_button_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
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

	calico_button_control& calico_button_control::set_text_style(textStyleRequest request)
	{
		text_style = request;
		return *this;
	}

}

#endif
