#ifndef CORONA_PRESENTATION_CONTROLS_DX_H
#define CORONA_PRESENTATION_CONTROLS_DX_H

#include "corona-presentation-controls-base.hpp"

namespace corona
{

	class draw_control : public control_base
	{
	public:

		HBRUSH background_brush_win32;
		solidBrushRequest	background_brush;

		std::weak_ptr<applicationBase> host;
		std::weak_ptr<direct2dChildWindow> window;
		std::function<void(draw_control*)> on_draw;
		std::function<void(draw_control*)> on_create;

		draw_control()
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			parent = nullptr;
			id = id_counter::next();
		}

		draw_control(const draw_control& _src) : control_base( _src )
		{
			background_brush_win32 = nullptr;
			background_brush = _src.background_brush;
			on_draw = _src.on_draw;
			on_create = _src.on_create;
		}

		draw_control(container_control_base *_parent, int _id) 
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			parent = _parent;
			id = _id;
		}

		virtual ~draw_control()
		{
			if (background_brush_win32) 
			{
				::DeleteObject(background_brush_win32);
			}
		}

		virtual void create(std::weak_ptr<applicationBase> _host)
		{
			host = _host;
			if (auto phost = _host.lock()) {
				if (inner_bounds.x < 0 || inner_bounds.y < 0 || inner_bounds.w < 0 || inner_bounds.h < 0) {
					throw std::logic_error("inner bounds not initialized");
				}
				window = phost->createDirect2Window(id, inner_bounds);
			}
			if (on_create) {
				on_create(this);
			}
			for (auto child : children) {
				child->create(_host);
			}
		}

		void destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void on_resize()
		{
			auto ti = typeid(*this).name();

			if (auto pwindow = window.lock())
			{
				pwindow->moveWindow(inner_bounds.x, inner_bounds.y, inner_bounds.w, inner_bounds.h);
			}
		}

		void draw()
		{
			bool adapter_blown_away = false;

			if (auto pwindow = window.lock())
			{
				pwindow->beginDraw(adapter_blown_away);
				if (!adapter_blown_away)
				{
					auto& context = pwindow->getContext();

					auto& bc = background_brush.brushColor;

					if (background_brush.active)
					{
						if (background_brush_win32)
							DeleteBrush(background_brush_win32);

						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor(bc);
						background_brush_win32 = ::CreateSolidBrush(RGB(color.a * color.r * 255.0, color.a * color.g * 255.0, color.a * color.b * 255.0));
						dc->Clear(color);
					}
					else
					{
						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor("00000000");
						dc->Clear(color);
					}

					if (on_draw)
					{
						on_draw(this);
					}
					else
					{

					}
				}
				pwindow->endDraw(adapter_blown_away);
			}
			for (auto& child : children) {
				child->draw();
			}
		}

		void render(ID2D1DeviceContext* _dest)
		{
			if (auto pwindow = window.lock())
			{
				auto bm = pwindow->getBitmap();
				D2D1_RECT_F dest;
				dest.left = inner_bounds.x;
				dest.top = inner_bounds.y;
				dest.right = inner_bounds.w + inner_bounds.x;
				dest.bottom = inner_bounds.h + inner_bounds.y;

				auto size = bm->GetPixelSize();
				D2D1_RECT_F source;
				source.left = 0;
				source.top = 0;
				source.bottom = inner_bounds.h;
				source.right = inner_bounds.w;
				_dest->DrawBitmap(bm, &dest, 1.0, D2D1_INTERPOLATION_MODE::D2D1_INTERPOLATION_MODE_LINEAR, &source);
			}
			for (auto &child : children)
			{
				child->render(_dest);
			}
		}

	};


	class camera_control : public draw_control
	{
		void init();
		solidBrushRequest	border_brush;
	public:

		camera_control(const camera_control& _src) : draw_control(_src)
		{
			border_brush = _src.border_brush;
		}

		camera_control(container_control_base* _parent, int _id);
		virtual ~camera_control();

	};

	class grid_control : public draw_control
	{
		void init();
		solidBrushRequest	border_brush;
	public:
		grid_control(const grid_control& _src) : draw_control(_src)
		{
			border_brush = _src.border_brush;
		}
		grid_control(container_control_base* _parent, int _id);
		virtual ~grid_control();

	};

	const int IDC_CHART_BASE = 1000000;
	const int IDC_CHART_PROGRAM = 1 + IDC_CHART_BASE;
	const int IDC_CHART_BAR = 2 + IDC_CHART_BASE;
	const int IDC_CHART_LINE = 3 + IDC_CHART_BASE;
	const int IDC_CHART_PIE = 4 + IDC_CHART_BASE;
	const int IDC_CHART_SCATTER = 5 + IDC_CHART_BASE;
	const int IDC_CHART_BUBBLE = 6 + IDC_CHART_BASE;
	const int IDC_CHART_TREE_BOX = 7 + IDC_CHART_BASE;
	const int IDC_CHART_TREE_MAP = 8 + IDC_CHART_BASE;

	class chart_control : public draw_control
	{
		void init();
		solidBrushRequest	border_brush;

		/*

		The big thing for the design here is to just be the mainframe sleazes when it comes to handling loads of options.
		For the type of chart, you can make in the menu into it.

		*/

	public:

		int chart_type;
		table_data  data;
		std::string color_series;
		std::string sseries1;
		std::string sseries2;
		std::string sseries3;
		std::string sseries4;

		chart_control(const chart_control& _src) : draw_control(_src)
		{
			border_brush = _src.border_brush;
			chart_type = _src.chart_type;
			data = _src.data;
			color_series = _src.color_series;
			sseries1 = _src.sseries1;
			sseries2 = _src.sseries2;
			sseries3 = _src.sseries3;
			sseries4 = _src.sseries4;
		}

		chart_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
		{
			;
		}

		virtual ~chart_control()
		{
			;
		}

	};


	class slide_control : public draw_control
	{
		void init();
		solidBrushRequest	border_brush;

	public:

		int source_object_id;

		slide_control(container_control_base* _parent, int _id) : draw_control(_parent, _id) { ; }
		virtual ~slide_control() { ; }

	};

	class gradient_button_control : public draw_control
	{
	public:

		linearGradientBrushRequest buttonFaceNormal;
		linearGradientBrushRequest buttonFaceDown;
		linearGradientBrushRequest buttonFaceOver;
		radialGradientBrushRequest buttonBackLight;

		solidBrushRequest foregroundNormal;
		solidBrushRequest foregroundOver;
		solidBrushRequest foregroundDown;

		gradient_button_control()
		{
			;
		}

		gradient_button_control(const gradient_button_control& _src) : draw_control(_src)
		{
			buttonFaceNormal = _src.buttonFaceNormal;
			buttonFaceDown = _src.buttonFaceDown;
			buttonFaceOver = _src.buttonFaceOver;
			buttonBackLight = _src.buttonBackLight;

			foregroundNormal = _src.foregroundNormal;
			foregroundOver = _src.foregroundOver;
			foregroundDown = _src.foregroundDown;
		}

		gradient_button_control(container_control_base* _parent, int _id, std::string _base_name) : draw_control(_parent, _id)
		{
			buttonFaceNormal.name = _base_name + "_face_normal";
			buttonFaceOver.name = _base_name + "_face_over";
			buttonFaceDown.name = _base_name + "_face_down";

			foregroundNormal.name = _base_name + "_fore_normal";
			foregroundOver.name = _base_name + "_fore_over";
			foregroundDown.name = _base_name + "_fore_down";

			buttonFaceNormal.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.8 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonFaceOver.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.8 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonFaceDown.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.9 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonBackLight.gradientStops = {
				{ toColor("#00000000"), 0.0 },
				{ toColor("#20202020"), 0.9 },
				{ toColor("#E0F0E0FF"), 1.0 }
			};

			foregroundNormal.active = true;
			foregroundNormal.brushColor = toColor("#808080");
			foregroundOver.active = true;
			foregroundOver.brushColor = toColor("#70A070");
			foregroundDown.active = true;
			foregroundDown.brushColor = toColor("#00FF00");

		}

		virtual void arrange(rectangle _ctx)
		{
			draw_control::arrange(_ctx);

			if (auto pwindow = this->window.lock())
			{
				buttonFaceNormal.start.x = inner_bounds.w / 2;
				buttonFaceNormal.start.y = 0;
				buttonFaceNormal.stop.y = inner_bounds.h;
				buttonFaceNormal.stop.x = inner_bounds.w / 2;

				buttonFaceDown.start.x = inner_bounds.w / 2;
				buttonFaceDown.start.y = 0;
				buttonFaceDown.stop.y = inner_bounds.h;
				buttonFaceDown.stop.x = inner_bounds.w / 2;

				buttonFaceOver.start.x = inner_bounds.w / 2;
				buttonFaceOver.start.y = 0;
				buttonFaceOver.stop.y = inner_bounds.h;
				buttonFaceOver.stop.x = inner_bounds.w / 2;

				buttonBackLight.center = rectangle_math::center(_ctx);
				buttonBackLight.offset = {};
				buttonBackLight.radiusX = inner_bounds.w / 2.0;
				buttonBackLight.radiusY = inner_bounds.h / 2.0;

				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceNormal);
				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceDown);
				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceOver);
				pwindow->getContext().setRadialGradientBrush(&this->buttonBackLight);
				pwindow->getContext().setSolidColorBrush(&this->foregroundNormal);
				pwindow->getContext().setSolidColorBrush(&this->foregroundDown);
				pwindow->getContext().setSolidColorBrush(&this->foregroundOver);
			}
		}

		virtual ~gradient_button_control()
		{
			;
		}

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

	class minimize_button_control : public gradient_button_control
	{
	public:

		minimize_button_control() { ; }
		minimize_button_control(const minimize_button_control& _src) : gradient_button_control(_src) { ; }
		minimize_button_control(container_control_base* _parent, int _id);

		virtual ~minimize_button_control();
		virtual LRESULT get_nchittest() {
			return HTCLIENT; // we lie here 
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

	};

	class maximize_button_control : public gradient_button_control
	{
	public:

		maximize_button_control() { ; }
		maximize_button_control(const maximize_button_control& _src) : gradient_button_control(_src) { ; }
		maximize_button_control(container_control_base* _parent, int _id);

		virtual ~maximize_button_control();
		virtual LRESULT get_nchittest() {
			return HTCLIENT;// we lie here 
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

	};

	class close_button_control : public gradient_button_control
	{
	public:

		close_button_control() { ; }
		close_button_control(const close_button_control& _src) : gradient_button_control(_src) { ; }

		close_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "close")
		{
			auto ctrl = this;

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
								point start, stop;
								start.x = _bounds->x;
								start.y = _bounds->y;
								stop.x = _bounds->right();
								stop.y = _bounds->bottom();
								pcontext->drawLine(&start, &stop, _foreground->name, 4);
								start.x = _bounds->right();
								start.y = _bounds->y;
								stop.x = _bounds->x;
								stop.y = _bounds->bottom();
								pcontext->drawLine(&start, &stop, _foreground->name, 4);
								};

							draw_button(draw_shape);
						}
					}
				};
		}

		virtual LRESULT get_nchittest() {
			return HTCLIENT;// we lie here 
		}


		void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
				{
					_presentation->close_window();
				});
		}

		virtual ~close_button_control()
		{
			;
		}

	};

	class menu_button_control : public gradient_button_control
	{
	public:

		menu_item menu;

		menu_button_control()
		{
			;
		}
		menu_button_control(const menu_button_control& _src) : gradient_button_control(_src) {
			menu = _src.menu; 
		}
		menu_button_control(container_control_base* _parent, int _id);
		virtual ~menu_button_control() { ; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);
	};

	class tab_button_control : public gradient_button_control
	{
	public:

		std::string			text;
		solidBrushRequest	text_fill_brush;
		textStyleRequest	text_style;
		double				icon_width;
		int*				active_id;
		std::function<void(tab_button_control& _tb)> tab_selected;

		tab_button_control() : active_id(nullptr)
		{
			init();
		}

		tab_button_control(const tab_button_control& _src) : gradient_button_control(_src) {
			init();
			text = _src.text;
			text_fill_brush = _src.text_fill_brush;
			text_style = _src.text_style;
			icon_width = _src.icon_width;
			active_id = _src.active_id;
		}
		tab_button_control(container_control_base* _parent, int _id);
		virtual ~tab_button_control() { ; }

		void init();
		virtual double get_font_size() { return text_style.fontSize; }
		tab_button_control& set_text(std::string _text);
		tab_button_control& set_text_fill(solidBrushRequest _brushFill);
		tab_button_control& set_text_fill(std::string _color);
		tab_button_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
		tab_button_control& set_text_style(textStyleRequest request);

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

					if (mouse_left_down.value() || *active_id == id)
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


	class image_control :
		public draw_control
	{

		enum image_modes {
			no_image,
			use_file_name,
			use_control_id,
			use_resource_id
		};

		bitmapInstanceDto instance;

		image_modes		image_mode;

		std::string		image_file_name;
		int				image_control_id;
		DWORD			image_resource_id;
		std::string		image_name;

		void init();

	public:
		image_control();
		image_control(const image_control& _src) = default;
		image_control(container_control_base* _parent, int _id);
		image_control(container_control_base* _parent, int _id, std::string _file_name);
		image_control(container_control_base* _parent, int _id, int _source_control_id);
		virtual ~image_control();

		void load_from_file(std::string _name);
		void load_from_resource(DWORD _resource_id);
		void load_from_control(int _control_id);
	};


	camera_control::camera_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
	{
		init();
	}

	void camera_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(1.0_container, 1.2_fontgr);

		on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					solidBrushRequest	border_brush;
					pwindow->getContext().setSolidColorBrush(&this->background_brush);
					pwindow->getContext().setSolidColorBrush(&this->border_brush);
				}
			};

		on_draw = [this](draw_control* _src) {
			if (auto pwindow = this->window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					draw_bounds.x = 0;
					draw_bounds.y = 0;

					pwindow->getContext().drawRectangle(&draw_bounds, this->border_brush.name, 8, this->background_brush.name);
				}
			}
			};
	}

	camera_control::~camera_control()
	{
		;
	}

	image_control::image_control()
	{
		init();
	}

	image_control::image_control(container_control_base* _parent, int _id)
		: draw_control(_parent, _id)
	{
		init();
	}

	image_control::image_control(container_control_base* _parent, int _id, std::string _file_name) : draw_control(_parent, _id)
	{
		init();
		load_from_file(_file_name);
	}

	image_control::image_control(container_control_base* _parent, int _id, int _source_control_id) : draw_control(_parent, _id)
	{
		init();
		load_from_control(_source_control_id);
	}

	void image_control::load_from_file(std::string _name)
	{
		image_mode = image_modes::use_file_name;
		image_file_name = _name;
		instance.bitmapName = std::format("bitmap_file_{0}", id);
	}

	void image_control::load_from_resource(DWORD _resource_id)
	{
		image_mode = image_modes::use_resource_id;
		image_resource_id = _resource_id;
		instance.bitmapName = std::format("bitmap_resource_{0}_{1}", id, _resource_id);
	}

	void image_control::load_from_control(int _control_id)
	{
		image_mode = image_modes::use_control_id;
		image_control_id = _control_id;
		instance.bitmapName = std::format("bitmap_control_{0}_{1}", id, _control_id);
	}

	void image_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(50.0_px, 50.0_px);

		on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					auto& context = pwindow->getContext();

					solidBrushRequest sbr;
					sbr.active = true;
					sbr.brushColor = toColor("FFFF00");
					sbr.name = "image_control_test";
					context.setSolidColorBrush(&sbr);

					switch (image_mode) {
					case image_modes::use_control_id:
						break;
					case image_modes::use_resource_id:
					{
						bitmapRequest request = {};
						request.resource_id = image_resource_id;
						request.name = instance.bitmapName;
						request.cropEnabled = false;
						point pt = { inner_bounds.w, inner_bounds.h };
						request.sizes.push_back(pt);
						context.setBitmap(&request);
						break;
					}
					break;
					case image_modes::use_file_name:
					{
						if (image_file_name.size() == 0)
							throw std::logic_error("Missing file name for image");
						bitmapRequest request = {};
						request.file_name = image_file_name;
						request.name = instance.bitmapName;
						request.cropEnabled = false;
						point pt = { inner_bounds.w, inner_bounds.h };
						request.sizes.push_back(pt);
						context.setBitmap(&request);
						auto szfound = std::begin(request.sizes);
						if (szfound != std::end(request.sizes)) {
							instance.width = request.sizes.begin()->x;
							instance.height = request.sizes.begin()->y;
						}
						else
						{
							instance.width = 0;
							instance.height = 0;
						}
						break;
					}
					}
				}
			};

		on_draw = [this](draw_control* _src) {
			if (auto pwindow = this->window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					draw_bounds.x = 0;
					draw_bounds.y = 0;

					instance.copyId = 0;
					instance.selected = false;
					instance.x = draw_bounds.x;
					instance.y = draw_bounds.y;
					instance.width = draw_bounds.w;
					instance.height = draw_bounds.h;
					instance.alpha = 1.0;

					auto& context = pwindow->getContext();

					context.drawBitmap(&instance);
				}
			}
			};
	}

	image_control::~image_control()
	{
		;
	}


	menu_button_control::menu_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
	{
		auto ctrl = this;

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

							point start;
							point stop;

							start.x = _bounds->x;
							start.y = _bounds->y + _bounds->h / 2.0;
							stop.x = _bounds->right();
							stop.y = _bounds->y + _bounds->h / 2.0;

							pcontext->drawLine(&start, &stop, _foreground->name, 4);

							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->bottom());
							pid.path.addLineTo(_bounds->x, _bounds->bottom());
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
							};

						draw_button(draw_shape);

					}
				}
			};
	}

	void menu_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		menu.subscribe(_presentation, _page);
		_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
			{
				_presentation->open_menu(this, this->menu);
			});
	}

	minimize_button_control::minimize_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "minimize")
	{
		auto ctrl = this;

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
							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->bottom());
							pid.path.addLineTo(_bounds->right(), _bounds->y);
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
							};

						draw_button(draw_shape);

					}
				}
			};
	}

	void minimize_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
			_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
				{
					_presentation->minimize_window();
				});
	}

	minimize_button_control::~minimize_button_control()
	{
		;
	}

	maximize_button_control::maximize_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "maximize")
	{
		auto ctrl = this;

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
							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->bottom());
							pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->bottom());
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
							};

						draw_button(draw_shape);

					}
				}
			};
	}

	void maximize_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
			{
				_presentation->restore_window();
			});
	}

	maximize_button_control::~maximize_button_control()
	{
		;
	}


	tab_button_control::tab_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
	{
		init();
	}

	void tab_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		_page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
			{
				if (active_id) {
					*active_id = id;
				}
				if (tab_selected)
				{
					tab_selected(*this);
				}
			});
	}

	void tab_button_control::init()
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

							if (active_id && *active_id == id) {
								point start;
								point stop;
								start.x = icon_bounds.x + icon_width / 2;
								start.y = icon_bounds.y;
								stop.x = start.x;
								stop.y = icon_bounds.y + icon_bounds.h;
								pcontext->drawLine(&start, &stop, _foreground->name, 4);
								_bounds->y -= 8;
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

	tab_button_control& tab_button_control::set_text(std::string _text)
	{
		text = _text;
		return *this;
	}

	tab_button_control& tab_button_control::set_text_fill(solidBrushRequest _brushFill)
	{
		text_fill_brush = _brushFill;
		return *this;
	}

	tab_button_control& tab_button_control::set_text_fill(std::string _color)
	{
		text_fill_brush.name = typeid(*this).name();
		text_fill_brush.brushColor = toColor(_color);
		return *this;
	}

	tab_button_control& tab_button_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
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

	tab_button_control& tab_button_control::set_text_style(textStyleRequest request)
	{
		text_style = request;
		return *this;
	}


}

#endif

