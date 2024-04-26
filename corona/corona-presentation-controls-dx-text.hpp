#ifndef CORONA_PRESENTATION_CONTROLS_DX_TEXT_H
#define CORONA_PRESENTATION_CONTROLS_DX_TEXT_H

namespace corona
{
	class text_display_control : public draw_control
	{
	public:
		std::string			text;
		std::shared_ptr<generalBrushRequest>	text_fill_brush;
		std::shared_ptr<textStyleRequest>		text_style;

		text_display_control(const text_display_control& _src) : draw_control(_src)
		{
			text = _src.text;
			text_fill_brush = _src.text_fill_brush;
			text_style = _src.text_style;
		}

		text_display_control();
		text_display_control(container_control_base* _parent, int _id);
		virtual ~text_display_control() { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<text_display_control>(*this);
			return tv;
		}

		void init();
		virtual double get_font_size() { return text_style ? text_style->fontSize : 14; }
		text_display_control& set_text(std::string _text);
		text_display_control& set_text_fill(generalBrushRequest _brushFill);
		text_display_control& set_text_fill(std::string _color);
		text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
		text_display_control& set_text_style(textStyleRequest request);

	};

	class title_control : public text_display_control
	{
		void set_default_styles();
	public:
		title_control();
		title_control(const title_control& _src) : text_display_control(_src) 
		{
			;
		}
		title_control(container_control_base* _parent, int _id);

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<title_control>(*this);
			return tv;
		}

		virtual ~title_control();
	};

	class subtitle_control : public text_display_control
	{
		void set_default_styles();
	public:
		subtitle_control();
		subtitle_control(const subtitle_control& _src) : text_display_control(_src)
		{
			;
		}
		subtitle_control(container_control_base* _parent, int _id);

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<subtitle_control>(*this);
			return tv;
		}

		virtual ~subtitle_control();
	};

	class chaptertitle_control : public text_display_control
	{
		void set_default_styles();
	public:
		chaptertitle_control();
		chaptertitle_control(const chaptertitle_control& _src) : text_display_control(_src)
		{
			;
		}
		chaptertitle_control(container_control_base* _parent, int _id);

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<chaptertitle_control>(*this);
			return tv;
		}

		virtual ~chaptertitle_control();
	};

	class chaptersubtitle_control : public text_display_control
	{
		void set_default_styles();
	public:
		chaptersubtitle_control();
		chaptersubtitle_control(const chaptersubtitle_control& _src) : text_display_control(_src)
		{
			;
		}
		chaptersubtitle_control(container_control_base* _parent, int _id);

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<chaptersubtitle_control>(*this);
			return tv;
		}

		virtual ~chaptersubtitle_control();
	};

	class paragraph_control : public text_display_control
	{
		void set_default_styles();
	public:
		paragraph_control();
		paragraph_control(container_control_base* _parent, int _id);
		paragraph_control(const paragraph_control& _src) : text_display_control(_src)
		{
			;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<paragraph_control>(*this);
			return tv;
		}


		virtual ~paragraph_control();
	};

	class code_control : public text_display_control
	{
		void set_default_styles();
	public:
		code_control();
		code_control(container_control_base* _parent, int _id);
		code_control(const code_control& _src) : text_display_control(_src)
		{
			;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<code_control>(*this);
			return tv;
		}

		virtual ~code_control();
	};

	class label_control : public text_display_control
	{
		void set_default_styles();
	public:
		label_control();
		label_control(container_control_base* _parent, int _id);
		label_control(const label_control& _src) : text_display_control(_src)
		{
			;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<label_control>(*this);
			return tv;
		}

		virtual ~label_control();
	};

	class placeholder_control : public text_display_control
	{
		void set_default_styles();
	public:
		placeholder_control();
		placeholder_control(container_control_base* _parent, int _id);
		placeholder_control(const placeholder_control& _src) : text_display_control(_src)
		{
			;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<placeholder_control>(*this);
			return tv;
		}

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

		on_create = [](draw_control* _src)
			{
				std::cout << typeid(*_src).name() << " on_create" << std::endl;
				text_display_control *t = dynamic_cast<text_display_control*>(_src);
				if (auto pwindow = _src->window.lock())
				{
					std::cout << typeid(*_src).name() << " on_create created" << std::endl;
					if (t->text_fill_brush) {
						pwindow->getContext().setBrush(t->text_fill_brush.get());
					}
					if (t->text_style) {
						pwindow->getContext().setTextStyle(t->text_style.get());
					}
				}
			};

		on_draw = [this](draw_control* _src) {
			if (auto pwindow = this->window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					if (!text.size()) text = "";

					std::string test_text = std::format("{0}, {1}, {2}", text, draw_bounds.x, draw_bounds.y, (long)this);

					draw_bounds.x -= bounds.x;
					draw_bounds.y -= bounds.y;

					if (text_style && text_fill_brush) {
						pwindow->getContext().drawText(text.c_str(), &draw_bounds, text_style->name, text_fill_brush->get_name());
					}
				}
			}
		};
	}

	text_display_control& text_display_control::set_text(std::string _text)
	{
		text = _text;
		return *this;
	}

	text_display_control& text_display_control::set_text_fill(generalBrushRequest _brushFill)
	{
		text_fill_brush = std::make_shared<generalBrushRequest>(_brushFill);
		return *this;
	}

	text_display_control& text_display_control::set_text_fill(std::string _color)
	{
		text_fill_brush = std::make_shared<generalBrushRequest>();
		text_fill_brush->setColor(_color);
		text_fill_brush->set_name("text_fill");
		return *this;
	}

	text_display_control& text_display_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
	{
		text_style = std::make_shared<textStyleRequest>();
		text_style->name = typeid(*this).name();
		text_style->fontName = _font_name;
		text_style->fontSize = _font_size;
		text_style->bold = _bold;
		text_style->underline = _underline;
		text_style->italics = _italic;
		text_style->strike_through = _strike_through;
		return *this;
	}

	text_display_control& text_display_control::set_text_style(textStyleRequest request)
	{
		text_style = std::make_shared<textStyleRequest>(request);
		return *this;
	}

	void title_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush = st->TitleBackgroundBrush;
		text_fill_brush = st->TitleTextBrush;
		text_style = st->TitleFont;
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

		background_brush = st->SubtitleBackgroundBrush;
		text_fill_brush = st->SubtitleTextBrush;
		text_style = st->SubtitleFont;

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

		background_brush = st->ChapterTitleBackgroundBrush;
		text_fill_brush = st->ChapterTitleTextBrush;
		text_style = st->ChatperTitleFont;
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

		background_brush = st->ChapterSubTitleBackgroundBrush;
		text_fill_brush = st->ChapterSubTitleTextBrush;
		text_style = st->ChapterSubTitleFont;
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

		background_brush = st->ParagraphBackgroundBrush;
		text_fill_brush = st->ParagraphTextBrush;
		text_style = st->ParagraphFont;
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

		background_brush = st->CodeBackgroundBrush;
		text_fill_brush = st->CodeTextBrush;
		text_style = st->CodeFont;

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

		auto st = styles.get_style();

		background_brush = st->LabelBackgroundBrush;
		text_fill_brush = st->LabelTextBrush;
		text_style = st->LabelFont;

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
		auto st = styles.get_style();

		background_brush = st->PlaceholderBackgroundBrush;
		text_fill_brush = st->PlaceholderTextBrush;
		text_style = st->PlaceholderFont;

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
