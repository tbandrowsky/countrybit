#ifndef CORONA_PRESENTATION_CONTROLS_DX_TEXT_H
#define CORONA_PRESENTATION_CONTROLS_DX_TEXT_H

#include "corona-presentation-controls-dx.hpp"

namespace corona
{
	class text_display_control : public draw_control, public cloneable<text_display_control>
	{
	public:
		std::string			text;
		solidBrushRequest	text_fill_brush;
		textStyleRequest	text_style;

		text_display_control(const text_display_control& _src) = default;
		text_display_control();
		text_display_control(container_control_base* _parent, int _id);

		void init();
		virtual double get_font_size() { return text_style.fontSize; }
		text_display_control& set_text(std::string _text);
		text_display_control& set_text_fill(solidBrushRequest _brushFill);
		text_display_control& set_text_fill(std::string _color);
		text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
		text_display_control& set_text_style(textStyleRequest request);

	};

	class title_control : public text_display_control, public cloneable<title_control>
	{
		void set_default_styles();
	public:
		title_control();
		title_control(container_control_base* _parent, int _id);
		virtual ~title_control();
	};

	class subtitle_control : public text_display_control, public cloneable<subtitle_control>
	{
		void set_default_styles();
	public:
		subtitle_control();
		subtitle_control(container_control_base* _parent, int _id);
		virtual ~subtitle_control();
	};

	class chaptertitle_control : public text_display_control, public cloneable<chaptertitle_control>
	{
		void set_default_styles();
	public:
		chaptertitle_control();
		chaptertitle_control(container_control_base* _parent, int _id);
		virtual ~chaptertitle_control();
	};

	class chaptersubtitle_control : public text_display_control, public cloneable<chaptersubtitle_control>
	{
		void set_default_styles();
	public:
		chaptersubtitle_control();
		chaptersubtitle_control(container_control_base* _parent, int _id);
		virtual ~chaptersubtitle_control();
	};

	class paragraph_control : public text_display_control, public cloneable<paragraph_control>
	{
		void set_default_styles();
	public:
		paragraph_control();
		paragraph_control(container_control_base* _parent, int _id);
		virtual ~paragraph_control();
	};

	class code_control : public text_display_control, public cloneable<code_control>
	{
		void set_default_styles();
	public:
		code_control();
		code_control(container_control_base* _parent, int _id);
		virtual ~code_control();
	};

	class label_control : public text_display_control, public cloneable<label_control>
	{
		void set_default_styles();
	public:
		label_control();
		label_control(container_control_base* _parent, int _id);
		virtual ~label_control();
	};

	class placeholder_control : public text_display_control, public cloneable<placeholder_control>
	{
		void set_default_styles();
	public:
		placeholder_control();
		placeholder_control(container_control_base* _parent, int _id);
		virtual ~placeholder_control();
	};

	text_display_control::text_display_control()
	{
		init();
	}

	text_display_control::text_display_control(container_control_base* _parent, int _id)
		: draw_control(_parent, _id)
	{
		init();
		if (_parent)
		{
			set_nchittest(_parent->get_nchittest());
		}
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
					auto draw_bounds = inner_bounds;

					draw_bounds.x = 0;
					draw_bounds.y = 0;

					std::string test_text = std::format("{0}, {1}, {2}", text, draw_bounds.x, draw_bounds.y, (long)this);

					pwindow->getContext().drawText(text.c_str(), &draw_bounds, this->text_style.name, this->text_fill_brush.name);
					//	pwindow->getContext().drawRectangle(&draw_bounds, this->text_fill_brush.name, 4, nullptr);
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
		text_fill_brush.brushColor = toColor(_color);
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

	void title_control::set_default_styles()
	{

		auto st = styles.get_style();

		background_brush.name = "title_fill";
		background_brush.brushColor = toColor(st->TitleBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "title_text_fill";
		text_fill_brush.brushColor = toColor(st->TitleTextColor);

		text_style = {};
		text_style.name = "title_text_style";
		text_style.fontName = st->PrimaryFont;
		text_style.fontSize = 24;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = st->PrevailingAlignment;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
	}

	title_control::title_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	title_control::title_control()
	{
		set_default_styles();
	}

	title_control::~title_control()
	{
	}

	void subtitle_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush.name = "subtitle_fill";
		background_brush.brushColor = toColor(st->SubtitleBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "subtitle_text_fill";
		text_fill_brush.brushColor = toColor(st->SubtitleTextColor);

		text_style = {};
		text_style.name = "subtitle_text_style";
		text_style.fontName = st->PrimaryFont;
		text_style.fontSize = 18;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = true;
		text_style.strike_through = false;
		text_style.horizontal_align = st->PrevailingAlignment;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
	}

	subtitle_control::subtitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	subtitle_control::subtitle_control()
	{
		set_default_styles();
	}

	subtitle_control::~subtitle_control()
	{
	}

	void chaptertitle_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush.name = "chaptertitle_fill";
		background_brush.brushColor = toColor(st->ChapterTitleBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "chaptertitle_text_fill";
		text_fill_brush.brushColor = toColor(st->ChapterTitleTextColor);

		text_style = {};
		text_style.name = "chaptertitle_text_style";
		text_style.fontName = st->PrimaryFont;
		text_style.fontSize = 16;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = st->PrevailingAlignment;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
	}

	chaptertitle_control::chaptertitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	chaptertitle_control::chaptertitle_control()
	{
		set_default_styles();
	}

	chaptertitle_control::~chaptertitle_control()
	{
	}

	void chaptersubtitle_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush.name = "chaptersubtitle_fill";
		background_brush.brushColor = toColor(st->SubchapterTitleBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "chaptersubtitle_text_fill";
		text_fill_brush.brushColor = toColor(st->SubchapterTitleTextColor);

		text_style = {};
		text_style.name = "chaptersubtitle_text_style";
		text_style.fontName = st->PrimaryFont;
		text_style.fontSize = 14;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = st->PrevailingAlignment;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
	}


	chaptersubtitle_control::chaptersubtitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	chaptersubtitle_control::chaptersubtitle_control()
	{
		set_default_styles();
	}

	chaptersubtitle_control::~chaptersubtitle_control()
	{
	}

	void paragraph_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush.name = "paragraph_fill";
		background_brush.brushColor = toColor(st->ParagraphBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "paragraph_text_fill";
		text_fill_brush.brushColor = toColor(st->ParagraphTextColor);

		text_style = {};
		text_style.name = "paragraph_text_style";
		text_style.fontName = styles.get_style()->PrimaryFont;
		text_style.fontSize = 12;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = visual_alignment::align_near;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
	}

	paragraph_control::paragraph_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	paragraph_control::paragraph_control()
	{
		set_default_styles();
	}

	paragraph_control::~paragraph_control()
	{
	}

	void code_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush.name = "code_fill";
		background_brush.brushColor = toColor(st->CodeBackgroundColor);
		background_brush.active = true;

		text_fill_brush.name = "code_text_fill";
		text_fill_brush.brushColor = toColor(st->CodeTextColor);

		text_style = {};
		text_style.name = "code_text_style";
		text_style.fontName = "Cascadia Mono,Courier New";
		text_style.fontSize = 14;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = visual_alignment::align_near;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = false;
	}

	code_control::code_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	code_control::code_control()
	{
		set_default_styles();
	}

	code_control::~code_control()
	{
	}

	void label_control::set_default_styles()
	{
		text_fill_brush.name = "label_text_fill";
		text_fill_brush.brushColor = toColor(styles.get_style()->TextColor);

		text_style = {};
		text_style.name = "label_text_style";
		text_style.fontName = styles.get_style()->PrimaryFont;
		text_style.fontSize = 12;
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

	label_control::label_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		set_default_styles();
	}

	label_control::~label_control()
	{

	}

	void placeholder_control::set_default_styles()
	{
		text_fill_brush.name = "placeholder_text_fill";
		text_fill_brush.brushColor = toColor(styles.get_style()->TextColor);

		text_style = {};
		text_style.name = "placeholder_text_style";
		text_style.fontName = styles.get_style()->PrimaryFont;
		text_style.fontSize = 14;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = visual_alignment::align_center;
		text_style.vertical_align = visual_alignment::align_center;
		text_style.wrap_text = false;
	}

	placeholder_control::placeholder_control()
	{
		text = "Placeholder";
	}

	placeholder_control::placeholder_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		text = "Placeholder";
	}

	placeholder_control::~placeholder_control()
	{

	}
}

#endif
