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

		virtual void set_default_styles()
		{
			;
		}

		virtual double get_font_size() { return text_style ? text_style->fontSize : 14; }
		text_display_control& set_text(std::string _text);
		text_display_control& set_text_fill(generalBrushRequest _brushFill);
		text_display_control& set_text_fill(std::string _color);
		text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
		text_display_control& set_text_style(textStyleRequest request);

	};

	class title_control : public text_display_control
	{
	public:
		title_control();
		title_control(const title_control& _src) : text_display_control(_src) 
		{
			;
		}
		title_control(container_control_base* _parent, int _id);

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<title_control>(*this);
			return tv;
		}

		virtual ~title_control();
	};

	class subtitle_control : public text_display_control
	{
	public:
		subtitle_control();
		subtitle_control(const subtitle_control& _src) : text_display_control(_src)
		{
			;
		}
		subtitle_control(container_control_base* _parent, int _id);

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<subtitle_control>(*this);
			return tv;
		}

		virtual ~subtitle_control();
	};

	class chaptertitle_control : public text_display_control
	{
	public:
		chaptertitle_control();
		chaptertitle_control(const chaptertitle_control& _src) : text_display_control(_src)
		{
			;
		}
		chaptertitle_control(container_control_base* _parent, int _id);
		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<chaptertitle_control>(*this);
			return tv;
		}

		virtual ~chaptertitle_control();
	};

	class chaptersubtitle_control : public text_display_control
	{
	public:
		chaptersubtitle_control();
		chaptersubtitle_control(const chaptersubtitle_control& _src) : text_display_control(_src)
		{
			;
		}
		chaptersubtitle_control(container_control_base* _parent, int _id);

		virtual void set_default_styles();
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<chaptersubtitle_control>(*this);
			return tv;
		}

		virtual ~chaptersubtitle_control();
	};

	class paragraph_control : public text_display_control
	{
	public:
		paragraph_control();
		paragraph_control(container_control_base* _parent, int _id);
		paragraph_control(const paragraph_control& _src) : text_display_control(_src)
		{
			;
		}
		virtual void set_default_styles();
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<paragraph_control>(*this);
			return tv;
		}


		virtual ~paragraph_control();
	};

	class code_control : public text_display_control
	{
	public:
		code_control();
		code_control(container_control_base* _parent, int _id);
		code_control(const code_control& _src) : text_display_control(_src)
		{
			;
		}
		virtual void set_default_styles();
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<code_control>(*this);
			return tv;
		}

		virtual ~code_control();
	};

	class label_control : public text_display_control
	{
	public:
		label_control();
		label_control(container_control_base* _parent, int _id);
		label_control(const label_control& _src) : text_display_control(_src)
		{
			;
		}
		virtual void set_default_styles();
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<label_control>(*this);
			return tv;
		}

		virtual ~label_control();
	};

	class placeholder_control : public text_display_control
	{
	public:
		placeholder_control();
		placeholder_control(container_control_base* _parent, int _id);
		placeholder_control(const placeholder_control& _src) : text_display_control(_src)
		{
			;
		}
		virtual void set_default_styles();
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<placeholder_control>(*this);
			return tv;
		}

		virtual ~placeholder_control();
	};

	class error_control : public text_display_control
	{
		call_status error_status;

	public:

		error_control();
		error_control(const error_control& _src) : text_display_control(_src)
		{
			;
		}
		error_control(container_control_base* _parent, int _id);

		void set_status(call_status _status)
		{
			error_status = _status;
			text = error_status.message;
		}

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<error_control>(*this);
			return tv;
		}

		virtual ~error_control();
	};

	class success_control : public text_display_control
	{
		call_status error_status;

	public:

		success_control();
		success_control(const success_control& _src) : text_display_control(_src)
		{
			;
		}
		success_control(container_control_base* _parent, int _id);

		void set_status(call_status _status)
		{
			error_status = _status;
			text = error_status.message;
		}

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<success_control>(*this);
			return tv;
		}

		virtual ~success_control();
	};

	class status_control : public frame_layout
	{
	public:

		using draw_control::host;

		status_control();
		status_control(const status_control& _src) : frame_layout(_src)
		{
			;
		}
		status_control(container_control_base* _parent, int _id);

		void set_status(call_status _status)
		{
			children.clear();
			if (_status.message.size()) {
				if (_status.success) {
					std::shared_ptr<success_control> sc = std::make_shared<success_control>(this, id_counter::next());
					sc->set_status(_status);
					sc->set_size(1.0_container, 1.0_container);
					sc->set_padding(8.0_px);
					sc->set_bounds(bounds);
					children.push_back(sc);
				}
				else
				{
					std::shared_ptr<error_control> sc = std::make_shared<error_control>(this, id_counter::next());
					sc->set_status(_status);
					sc->set_size(1.0_container, 1.0_container);
					sc->set_padding(8.0_px);
					sc->set_bounds(bounds);
					children.push_back(sc);
				}
			}
			create(host);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<status_control>(*this);
			return tv;
		}

		virtual ~status_control();
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
				text_display_control *t = dynamic_cast<text_display_control*>(_src);
				if (t) {
					t->set_default_styles();

					if (auto pwindow = this->window.lock())
					{					
						if (t->text_fill_brush) {
							pwindow->getContext().setBrush(t->text_fill_brush.get(), &inner_bounds);
						}
						if (t->text_style) {
							pwindow->getContext().setTextStyle(t->text_style.get());
						}
					}
				}
			};

		on_draw = [this](draw_control* _src) {
			if (auto pwindow = this->window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					text_display_control* t = dynamic_cast<text_display_control*>(_src);

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
		border_brush = st->TitleBorderBrush;
		border_width = st->TitleBorderWidth;
	}

	title_control::title_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	title_control::title_control()
	{
		
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
		border_brush = st->SubtitleBorderBrush;
		border_width = st->SubtitleBorderWidth;

	}

	subtitle_control::subtitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	subtitle_control::subtitle_control()
	{
		
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
		border_brush = st->ChapterTitleBorderBrush;
		border_width = st->ChatperTitleBorderWidth;
	}

	chaptertitle_control::chaptertitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	chaptertitle_control::chaptertitle_control()
	{
		
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
		border_brush = st->ChapterSubTitleBorderBrush;
		border_width = st->ChapterSubTitleBorderWidth;
	}

	chaptersubtitle_control::chaptersubtitle_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	chaptersubtitle_control::chaptersubtitle_control()
	{
		
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
		border_brush = st->ParagraphBorderBrush;
		border_width = st->ParagraphBorderWidth;
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
		border_brush = st->CodeBorderBrush;
		border_width = st->CodeBorderWidth;
	}

	code_control::code_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	code_control::code_control()
	{
		
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
		border_brush = st->LabelBorderBrush;
		border_width = st->LabelBorderWidth;
	}

	label_control::label_control()
	{
		
	}

	label_control::label_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
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
		border_brush = st->PlaceholderBorderBrush;
		border_width = st->PlaceholderBorderWidth;

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

	void error_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush = st->ErrorBackgroundBrush;
		text_fill_brush = st->ErrorTextBrush;
		text_style = st->ErrorFont;
		border_brush = st->ErrorBorderBrush;
		border_width = st->ErrorBorderWidth;
	}

	error_control::error_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{
		
	}

	error_control::error_control()
	{

	}

	error_control::~error_control()
	{
	}


	void success_control::set_default_styles()
	{
		auto st = styles.get_style();

		background_brush = st->SuccessBackgroundBrush;
		text_fill_brush = st->SuccessTextBrush;
		text_style = st->SuccessFont;
		border_brush = st->SuccessBorderBrush;
		border_width = st->SuccessBorderWidth;
	}

	success_control::success_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{

	}

	success_control::success_control()
	{

	}

	success_control::~success_control()
	{
	}

	status_control::status_control(container_control_base* _parent, int _id) : frame_layout(_parent, _id)
	{
		set_origin(0.0_px, 0.0_px);
		set_margin(0.0_px);
	}

	status_control::status_control()
	{
		set_origin(0.0_px, 0.0_px);
		set_margin(0.0_px);
	}

	status_control::~status_control()
	{
	}

}

#endif
