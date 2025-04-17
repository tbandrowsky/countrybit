/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This is for the direct 2d controls such as title, subtitle, etc.

Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_CONTROLS_DX_TEXT_H
#define CORONA_PRESENTATION_CONTROLS_DX_TEXT_H

namespace corona
{
	class text_display_control : public draw_control
	{
	public:
		std::string			text;

		text_display_control(const text_display_control& _src) = default;
		text_display_control();
		text_display_control(container_control_base* _parent, int _id);
		virtual ~text_display_control() { ; }

		virtual void get_json(json& _dest)
		{
			draw_control::get_json(_dest);
			_dest.put_member("text", text);
		}
		virtual void put_json(json& _src)
		{
			draw_control::put_json(_src);
			text = _src["text"];
		}

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

		virtual double get_font_size() { return view_style ? view_style->text_style.fontSize : 14; }
		text_display_control& set_text(std::string _text);
		text_display_control& set_style(viewStyleRequest request);



	};

	class title_control : public text_display_control
	{
	public:
		title_control();
		title_control(const title_control& _src) = default;
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
		subtitle_control(const subtitle_control& _src) = default;
		subtitle_control(container_control_base* _parent, int _id);

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<subtitle_control>(*this);
			return tv;
		}

		virtual ~subtitle_control();
	};

	class authorscredit_control : public text_display_control
	{
	public:
		authorscredit_control();
		authorscredit_control(const authorscredit_control& _src) = default;
		authorscredit_control(container_control_base* _parent, int _id);

		virtual void set_default_styles();

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<authorscredit_control>(*this);
			return tv;
		}

		virtual ~authorscredit_control();
	};

	class chaptertitle_control : public text_display_control
	{
	public:
		chaptertitle_control();
		chaptertitle_control(const chaptertitle_control& _src) = default;
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
		chaptersubtitle_control(const chaptersubtitle_control& _src) = default;
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
		paragraph_control(const paragraph_control& _src) = default;
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
		code_control(const code_control& _src) = default;
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
		label_control(const label_control& _src) = default;
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
		placeholder_control(const placeholder_control& _src) = default;
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
		error_control(const error_control& _src) = default;
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
		success_control(const success_control& _src) = default;
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
		bool status_set;

		status_control();
		status_control(const status_control& _src) = default;
		status_control(container_control_base* _parent, int _id);

		bool set_status(call_status _status)
		{
			children.clear();
			status_set = false;
			if (_status.message.size()) {
				status_set = true;
				if (_status.success) {
					std::shared_ptr<success_control> sc = std::make_shared<success_control>(this, id_counter::next());
					sc->set_status(_status);
					sc->set_padding(8.0_px);
					sc->set_size(1.0_container, 1.0_container);
					sc->arrange(inner_bounds);
					children.push_back(sc);
				}
				else
				{
					std::shared_ptr<error_control> sc = std::make_shared<error_control>(this, id_counter::next());
					sc->set_status(_status);
					sc->set_padding(8.0_px);
					sc->set_size(1.0_container, 1.0_container);
					sc->arrange(inner_bounds);
					children.push_back(sc);
				}
			}
			return status_set;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<status_control>(*this);
			return tv;
		}

		virtual void arrange(rectangle _bounds)
		{
			frame_layout::arrange(_bounds);
			//std::cout << "status layout" << std::endl;
			//std::cout << bounds.x << " " << bounds.y << std::endl;
			//std::cout << inner_bounds.x << " " << inner_bounds.y << std::endl;
			//for (auto child : children) {
				//std::cout << "child layout" << std::endl;
				//std::cout << child->get_bounds().x << " " << child->get_bounds().y << std::endl;
				//std::cout << child->get_inner_bounds().x << " " << child->get_inner_bounds().y << std::endl;
			//}
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

		on_create = [](std::shared_ptr<direct2dContext>& _context, draw_control* _src)
			{
				text_display_control *t = dynamic_cast<text_display_control*>(_src);
				if (t) {
					t->set_default_styles();
				}
			};

		on_draw = [](std::shared_ptr<direct2dContext>& _context, draw_control* _src) {
			text_display_control* t = dynamic_cast<text_display_control*>(_src);

			auto draw_bounds = t->inner_bounds;

			if (not t->text.size()) t->text = "";

			std::string test_text = std::format("{0}, {1}, {2}", t->text, draw_bounds.x, draw_bounds.y, (long)t);
			//std::cout << test_text << std::endl;

			if (t->view_style) {
				_context->drawText(t->text.c_str(), &draw_bounds, t->view_style->text_style.name, t->view_style->shape_fill_brush.get_name());
			}
		};
	}

	text_display_control& text_display_control::set_text(std::string _text)
	{
		text = _text;
		return *this;
	}

	text_display_control& text_display_control::set_style(viewStyleRequest request)
	{
		view_style = std::make_shared<viewStyleRequest>(request);

		return *this;
	}

	void title_control::set_default_styles()
	{
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->TitleStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->SubtitleStyle;

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

	void authorscredit_control::set_default_styles()
	{
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->AuthorsCreditStyle;

	}

	authorscredit_control::authorscredit_control(container_control_base* _parent, int _id) : text_display_control(_parent, _id)
	{

	}

	authorscredit_control::authorscredit_control()
	{

	}

	authorscredit_control::~authorscredit_control()
	{
	}

	void chaptertitle_control::set_default_styles()
	{
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->ChapterTitleStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->ChapterSubTitleStyle;

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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->ParagraphStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->CodeStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->LabelStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->PlaceholderStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->ErrorStyle;
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
		auto st = presentation_style_factory::get_current();

		view_style = st->get_style()->SuccessStyle;
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
		status_set = false;
		set_padding(0.0_px);
	}

	status_control::status_control()
	{
		status_set = false;
		set_padding(0.0_px);
	}

	status_control::~status_control()
	{
	}

}

#endif
