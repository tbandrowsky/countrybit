#pragma once

namespace corona
{
	namespace database
	{
		using namespace win32;

		extern presentation_style_factory styles;

		class id_counter
		{
		public:
			static int id;
			static int next();
			static int check(int _id);
			static int status_bar_id;
			static int status_text_title_id;
			static int status_text_subtitle_id;
		};

		class layout_context
		{
		public:
			point flow_origin;
			point container_origin;
			point container_size;
			point remaining_size;
			point space_amount;
		};

		class container_control;
		class row_layout;
		class column_layout;
		class absolute_layout;
		class frame_layout;

		class presentation;
		class page;

		class title_control;
		class subtitle_control;
		class chaptertitle_control;
		class chaptersubtitle_control;
		class paragraph_control;
		class code_control;
		class label_control;
		class image_control;

		class static_control;
		class pushbutton_control;
		class pressbutton_control;
		class checkbox_control;
		class radiobutton_control;
		class linkbutton_control;
		class listbox_control;
		class combobox_control;
		class edit_control;
		class scrollbar_control;
		class listview_control;
		class treeview_control;
		class header_control;
		class toolbar_control;
		class statusbar_control;
		class hotkey_control;
		class animate_control;
		class richedit_control;
		class draglistbox_control;
		class rebar_control;
		class comboboxex_control;
		class datetimepicker_control;
		class monthcalendar_control;
		class control_base;

		class minimize_button_control;
		class maximize_button_control;
		class close_button_control;
		class menu_button_control;

		using menu_click_handler = std::function<void(presentation *_presentation, std::weak_ptr<page> _page)>;

		class menu_item_navigate 
		{
		public:
			int control_id;
			std::string target_page;
			menu_click_handler handler;

			menu_item_navigate();
			menu_item_navigate(int _source_control_id, std::string _target_page);
			menu_item_navigate(const menu_item_navigate& _src);
			menu_item_navigate operator =(const menu_item_navigate& _src);
			menu_item_navigate(menu_item_navigate&& _src);
			menu_item_navigate& operator =(menu_item_navigate&& _src);
			operator menu_click_handler ();
		};

		class menu_item : public std::enable_shared_from_this<menu_item>
		{
			menu_item* parent;
			HMENU to_menu_children(HMENU hmenu, int idx = 0);
			HMENU created_menu;

		public:

			int	 id;
			bool is_separator;
			std::string name;
			std::vector<std::shared_ptr<menu_item>> children;
			menu_click_handler handler;	

			menu_item();
			menu_item(int _id, std::string _name = "Empty", std::function<void(menu_item& _item)> _settings = nullptr);
			virtual ~menu_item();

			menu_item& item(int _id, std::string _name, std::function<void(menu_item& _item)>  _settings = nullptr);
			menu_item& destination(int _id, std::string _name, std::string _destination_name, std::function<void(menu_item& _item)>  _settings = nullptr);
			menu_item& separator(int _id, std::function<void(menu_item& _item)>  _settings = nullptr);

			menu_item& begin_submenu(int _id, std::string _name, std::function<void(menu_item& _item)>  _settings = nullptr);
			menu_item& end();

			void subscribe(presentation* _presentation, std::weak_ptr<page> _page);

			HMENU to_menu();
		};

		enum control_push_property 
		{
			cp_none = 0,
			cp_left_bounds = 1,
			cp_top_bounds = 2,
			cp_right_bounds = 4,
			cp_bottom_bounds = 8
		};

		class control_push_request 
		{
		public:
			int dest_control_id;
			int properties_to_push;
		};

		template <typename T> class change_monitored_property
		{
			T last_value;
			T current_value;
			bool is_changed;

		public:

			change_monitored_property()
			{
				current_value = {};
				last_value = {};
				is_changed = false;
			}

			change_monitored_property(T _default_value)
			{
				current_value = _default_value;
				last_value = _default_value;
				is_changed = false;
			}

			virtual ~change_monitored_property()
			{
				;
			}

			operator T () 
			{
				return current_value;
			}

			T operator = (T _new_value)
			{
				is_changed = (_new_value != current_value);
				last_value = current_value;
				current_value = _new_value;
				return current_value;
			}

			void reset(T _value)
			{
				is_changed = false;
				last_value = current_value;
				current_value = _value;
				return current_value;
			}

			T& value()
			{
				return current_value;
			}

			bool changed_from(T _from_value)
			{
				bool really_changed = (is_changed &&
					_from_value == current_value);
				return really_changed;
			}

			bool changed_to(T _to_value)
			{
				bool really_changed = (is_changed &&					
					_to_value == current_value);
				return really_changed;
			}

			bool changed( T _from_value, T _to_value )
			{
				bool really_changed = (is_changed &&
					_from_value == last_value &&
					_to_value == current_value);
				return really_changed;
			}
		};

		class control_base : public std::enable_shared_from_this<control_base>
		{
		protected:

			point get_size(rectangle _ctx, point _remaining);
			point get_position(rectangle _ctx);
			double to_pixels(measure _margin);
			virtual point get_remaining(point _ctx);
			virtual void on_resize();
			void arrange_children(rectangle _bounds,  
				std::function<point(const rectangle *_bounds, control_base*)> _initial_origin,
				std::function<point(point* _origin, const rectangle *_bounds, control_base*)> _next_origin);

			rectangle				bounds;
			rectangle				inner_bounds;
			point					margin_amount;
			point					padding_amount;

			change_monitored_property<bool> mouse_over;
			change_monitored_property<bool> mouse_left_down;
			change_monitored_property<bool> mouse_right_down;
			point		 mouse_relative_position;

		public:

			friend class absolute_layout;
			friend class row_layout;
			friend class column_layout;
			friend class draw_control;

			int						id;

			layout_rect				box;
			measure					margin;
			measure					padding;

			win32::directApplicationWin32* app;
			container_control *parent;

			std::vector<std::shared_ptr<control_base>> children;

			LRESULT hittest = HTCLIENT;

			virtual LRESULT get_nchittest() { return hittest; }
			virtual void set_nchittest( LRESULT _hittest ) { hittest = _hittest; }

			control_base() :
				id(-1),
				margin(),
				parent(nullptr),
				margin_amount({ 0.0, 0.0 })
			{
				id = id_counter::next();
			}

			control_base(container_control* _parent, int _id) : control_base()
			{
				parent = _parent;

				if (_id < 0) {
					id = id_counter::next();
				}
				else {
					id = _id;
				}
			}

			std::vector<control_push_request> push_requests;

			rectangle& get_bounds() { return bounds;  }
			rectangle& get_inner_bounds() { return inner_bounds; }

			rectangle& set_bounds(rectangle& _bounds);

			virtual void arrange(rectangle _ctx);
			bool contains(point pt);

			void push(int _destination_control_id, bool _push_left, bool _push_top, bool _push_right, bool _push_bottom);

			control_base* find(int _id);
			control_base* find(point p);
			control_base* get(control_base* _root, int _id);

			virtual double get_font_size() { return 12; }

			template <typename control_type> control_type& find(int _id)
			{
				control_base* temp = find(_id);
				control_type* citem = dynamic_cast<control_type>(temp);
				return *citem;
			}

			template <typename control_type> control_type& get(control_base* _root, int _id)
			{
				control_base* temp = get(_root, _id);
				control_type* citem = dynamic_cast<control_type>(temp);
				return *citem;
			}

			void foreach(std::function<void(control_base* _root)> _item);
			control_base *find_if(std::function<bool(control_base* _root)> _item);

			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host);
			virtual void destroy();
			virtual void draw();
			virtual void render(CComPtr<ID2D1DeviceContext>& _dest);

			control_base& set_origin(measure _x, measure _y)
			{
				box.x = _x;
				box.y = _y;
				return *this;
			}

			control_base& set_size(measure _width, measure _height)
			{
				box.width = _width;
				box.height = _height;
				return *this;
			}

			control_base& set_position(layout_rect _new_layout)
			{
				box = _new_layout;
				return *this;
			}

			control_base& set_margin(measure _space)
			{
				margin = _space;
				return *this;
			}

			template <typename control_class> control_class& set_spacing(measure _spacing)
			{
				control_class* r = dynamic_cast<control_class>(this);
				r->margin = _spacing;
				return *this;
			}

			virtual void apply(control_base& _ref);
			virtual bool set_mouse(point _position,
				bool *_left_down,
				bool *_right_down,
				std::function<void(control_base* _item)> _left_click,
				std::function<void(control_base* _item)> _right_click
			);

			virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
		};

		class draw_control : public control_base
		{
			void init();

		public:

			HBRUSH background_brush_win32;
			solidBrushRequest	background_brush;

			std::weak_ptr<win32::directApplicationWin32> host;
			std::weak_ptr<win32::direct2dChildWindow> window;
			std::function<void(draw_control*)> on_draw;
			std::function<void(draw_control*)> on_create;

			draw_control();
			draw_control(container_control* _parent, int _id);
			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host);
			virtual void destroy();
			virtual void draw();
			virtual void render(CComPtr<ID2D1DeviceContext>& _dest);
			virtual void on_resize();
		};

		class container_control : public draw_control
		{

		public:

			layout_rect				item_box = {};
			measure					item_margin = {};

			visual_alignment		content_alignment = visual_alignment::align_near;
			visual_alignment		content_cross_alignment = visual_alignment::align_near;

			container_control();
			container_control(container_control* _parent, int _id);
			virtual ~container_control() { ; }

			virtual void apply(control_base& _ref);

			template <typename control_type> control_type& create(int _id)
			{
				std::shared_ptr<control_type> temp = std::make_shared<control_type>(this, _id);
				children.push_back(temp);
				std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
				return *temp.get();
			}

			container_control& set_item_origin(measure _x, measure _y);
			container_control& set_item_size(measure _width, measure _height);
			container_control& set_item_position(layout_rect _new_layout);
			container_control& set_item_margin(measure _item_margin);

			container_control& set_content_align(visual_alignment _new_alignment);
			container_control& set_content_cross_align(visual_alignment _new_alignment);

			container_control& set_origin(measure _x, measure _y);
			container_control& set_size(measure _width, measure _height);
			container_control& set_position(layout_rect _new_layout);
			container_control& set_margin(measure _space);

			container_control& set_background_color(solidBrushRequest _brushFill);
			container_control& set_background_color(std::string _color);

			row_layout& row_begin(int id = id_counter::next(), std::function<void(row_layout&)> _settings = nullptr);
			column_layout& column_begin(int id = id_counter::next(), std::function<void(column_layout&)> _settings = nullptr);
			absolute_layout& absolute_begin(int id = id_counter::next(), std::function<void(absolute_layout&)> _settings = nullptr);
			frame_layout& frame_begin(int id = id_counter::next(), std::function<void(frame_layout&)> _settings = nullptr);

			row_layout& row_begin(std::function<void(row_layout&)> _settings) { return row_begin(id_counter::next(), _settings); }
			column_layout& column_begin(std::function<void(column_layout&)> _settings) { return column_begin(id_counter::next(), _settings); }
			absolute_layout& absolute_begin(std::function<void(absolute_layout&)> _settings) { return absolute_begin(id_counter::next(), _settings); }
			frame_layout& frame_begin(std::function<void(frame_layout&)> _settings) { return frame_begin(id_counter::next(), _settings); }

			container_control& end();

			container_control& title(std::string _text, std::function<void(title_control&)> _settings, int _id);
			container_control& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings, int _id);
			container_control& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings, int _id);
			container_control& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings, int _id);
			container_control& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings, int _id);
			container_control& code(std::string _text, std::function<void(code_control&)> _settings, int _id);
			container_control& label(std::string _text, std::function<void(label_control&)> _settings, int _id);

			inline container_control& title(std::string _text) { return title(_text, nullptr, id_counter::next()); }
			inline container_control& subtitle(std::string _text) { return subtitle(_text, nullptr, id_counter::next()); }
			inline container_control& chaptertitle(std::string _text) { return chaptertitle(_text, nullptr, id_counter::next()); }
			inline container_control& chaptersubtitle(std::string _text) { return chaptersubtitle(_text, nullptr, id_counter::next()); }
			inline container_control& paragraph(std::string _text) { return paragraph(_text, nullptr, id_counter::next()); }
			inline container_control& code(std::string _text) { return code(_text, nullptr, id_counter::next()); }
			inline container_control& label(std::string _text) { return label (_text, nullptr, id_counter::next()); }

			inline container_control& title(int _id, std::string _text) { return title(_text, nullptr, _id); }
			inline container_control& subtitle(int _id, std::string _text) { return subtitle(_text, nullptr, _id); }
			inline container_control& chaptertitle(int _id, std::string _text) { return chaptertitle(_text, nullptr, _id); }
			inline container_control& chaptersubtitle(int _id, std::string _text) { return chaptersubtitle(_text, nullptr, _id); }
			inline container_control& paragraph(int _id, std::string _text) { return paragraph(_text, nullptr, _id); }
			inline container_control& code(int _id, std::string _text) { return code(_text, nullptr, _id); }
			inline container_control& label(int _id, std::string _text) { return label(_text, nullptr, _id); }

			inline container_control& title(int _id, std::function<void(title_control&)> _settings) { return title("", _settings, _id); }
			inline container_control& subtitle(int _id, std::function<void(subtitle_control&)> _settings) { return subtitle("", _settings, _id); }
			inline container_control& chaptertitle(int _id, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle("", _settings, _id); }
			inline container_control& chaptersubtitle(int _id, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle("", _settings, _id); }
			inline container_control& paragraph(int _id, std::function<void(paragraph_control&)> _settings) { return paragraph("", _settings, _id); }
			inline container_control& code(int _id, std::function<void(code_control&)> _settings) { return code("", _settings, _id); }
			inline container_control& label(int _id, std::function<void(label_control&)> _settings) { return label("", _settings, _id); }

			inline container_control& title(std::string _text, std::function<void(title_control&)> _settings) { return title( _text, _settings, id_counter::next()); }
			inline container_control& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings) { return subtitle(_text, _settings, id_counter::next()); }
			inline container_control& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle(_text, _settings, id_counter::next()); }
			inline container_control& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle(_text, _settings, id_counter::next()); }
			inline container_control& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings) { return paragraph(_text, _settings, id_counter::next()); }
			inline container_control& code(std::string _text, std::function<void(code_control&)> _settings) { return code(_text, _settings, id_counter::next()); }
			inline container_control& label(std::string _text, std::function<void(label_control&)> _settings) { return label(_text, _settings, id_counter::next()); }

			container_control& image(int _id, int _control_id, std::function<void(image_control&)> _settings = nullptr);
			container_control& image(int _id, std::string _filename, std::function<void(image_control&)> _settings = nullptr);
			container_control& image(std::string _filename, std::function<void(image_control&)> _settings = nullptr);

			container_control& push_button(int _id, std::string text, std::function<void(pushbutton_control&)> _settings = nullptr);
			container_control& radio_button(int _id, std::string text, std::function<void(radiobutton_control&)> _settings = nullptr);
			container_control& checkbox(int _id, std::string text, std::function<void(checkbox_control&)> _settings = nullptr);
			container_control& press_button(int _id, std::string text, std::function<void(pressbutton_control&)> _settings = nullptr);

			container_control& listbox(int _id, std::function<void(listbox_control&)> _settings = nullptr);
			container_control& combobox(int _id, std::function<void(combobox_control&)> _settings = nullptr);
			container_control& edit(int _id, std::function<void(edit_control&)> _settings = nullptr);
			container_control& scrollbar(int _id, std::function<void(scrollbar_control&)> _settings = nullptr);

			container_control& listview(int _id, std::function<void(listview_control&)> _settings = nullptr);
			container_control& treeview(int _id, std::function<void(treeview_control&)> _settings = nullptr);
			container_control& header(int _id, std::function<void(header_control&)> _settings = nullptr);
			container_control& toolbar(int _id, std::function<void(toolbar_control&)> _settings = nullptr);
			container_control& statusbar(int _id, std::function<void(statusbar_control&)> _settings = nullptr);
			container_control& hotkey(int _id, std::function<void(hotkey_control&)> _settings = nullptr);
			container_control& animate(int _id, std::function<void(animate_control&)> _settings = nullptr);
			container_control& richedit(int _id, std::function<void(richedit_control&)> _settings = nullptr);
			container_control& draglistbox(int _id, std::function<void(draglistbox_control&)> _settings = nullptr);
			container_control& rebar(int _id, std::function<void(rebar_control&)> _settings = nullptr);
			container_control& comboboxex(int _id, std::function<void(comboboxex_control&)> _settings = nullptr);
			container_control& datetimepicker(int _id, std::function<void(datetimepicker_control&)> _settings = nullptr);
			container_control& monthcalendar(int _id, std::function<void(monthcalendar_control&)> _settings = nullptr);

			container_control& minimize_button(std::function<void(minimize_button_control&)> _settings = nullptr);
			container_control& maximize_button(std::function<void(maximize_button_control&)> _settings = nullptr);
			container_control& close_button(std::function<void(close_button_control&)> _settings = nullptr);
			container_control& menu_button(int _id,  std::function<void(menu_button_control&)> _settings = nullptr);

			container_control& caption_bar(
				presentation_style& st,
				int	title_bar_id,
				int menu_button_id,
				menu_item& menu,
				int image_control_id,
				std::string image_file,
				std::string corporate_name,
				int id_title_column_id,
				std::string title_name,
				std::string subtitle_name
			);

			container_control& form_single_column(int _align_id,
				std::string _form_name,
				std::function<void(container_control& _settings)> _add_controls
			);

			container_control& form_double_column(int _align_id,
				std::string _form_name,
				std::function<void(container_control& _settings)> _add_controls1,
				std::function<void(container_control& _settings)> _add_controls2
			);

			container_control& status_bar(presentation_style& st);

		};

		class text_display_control : public draw_control
		{
		public:
			std::string			text;
			solidBrushRequest	text_fill_brush;
			textStyleRequest	text_style;

			text_display_control();
			text_display_control(container_control* _parent, int _id);

			void init();
			virtual double get_font_size() { return text_style.fontSize; }
			text_display_control& set_text(std::string _text);
			text_display_control& set_text_fill(solidBrushRequest _brushFill);
			text_display_control& set_text_fill(std::string _color);
			text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
			text_display_control& set_text_style(textStyleRequest request);

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

			gradient_button_control(container_control* _parent, int _id, std::string _name);
			virtual ~gradient_button_control();

			virtual void arrange(rectangle _ctx);
			virtual void draw_button(std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape);

		};

		class minimize_button_control : public gradient_button_control
		{
		public:

			minimize_button_control(container_control* _parent, int _id);

			virtual ~minimize_button_control();
			virtual LRESULT get_nchittest() { 
				return HTCLIENT; // we lie here 
			}

			virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
		};

		class maximize_button_control : public gradient_button_control
		{
		public:

			maximize_button_control(container_control* _parent, int _id);

			virtual ~maximize_button_control();
			virtual LRESULT get_nchittest() { 
				return HTCLIENT;// we lie here 
			}

			virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
		};

		class close_button_control : public gradient_button_control
		{
		public:

			close_button_control(container_control* _parent, int _id);

			virtual ~close_button_control();
			virtual LRESULT get_nchittest() { 
				return HTCLIENT;// we lie here 
			}

			virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
		};

		class menu_button_control : public gradient_button_control
		{
		public:

			menu_item menu;

			menu_button_control(container_control* _parent, int _id);
			virtual ~menu_button_control() { ; }

			virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
		};

		class title_control : public text_display_control
		{
			void set_default_styles();
		public:
			title_control();
			title_control(container_control* _parent, int _id);
			virtual ~title_control();
		};

		class subtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			subtitle_control();
			subtitle_control(container_control* _parent, int _id);
			virtual ~subtitle_control();
		};

		class chaptertitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptertitle_control();
			chaptertitle_control(container_control* _parent, int _id);
			virtual ~chaptertitle_control();
		};

		class chaptersubtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptersubtitle_control();
			chaptersubtitle_control(container_control* _parent, int _id);
			virtual ~chaptersubtitle_control();
		};

		class paragraph_control : public text_display_control
		{
			void set_default_styles();
		public:
			paragraph_control();
			paragraph_control(container_control* _parent, int _id);
			virtual ~paragraph_control();
		};

		class code_control : public text_display_control
		{
			void set_default_styles();
		public:
			code_control();
			code_control(container_control* _parent, int _id);
			virtual ~code_control();
		};

		class label_control: public text_display_control
		{
			void set_default_styles();
		public:
			label_control();
			label_control(container_control* _parent, int _id);
			virtual ~label_control();
		};

		class placeholder_control: public text_display_control
		{
			void set_default_styles();
		public:
			placeholder_control();
			placeholder_control(container_control* _parent, int _id);
			virtual ~placeholder_control();
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
			image_control(container_control* _parent, int _id);
			image_control(container_control* _parent, int _id, std::string _file_name);
			image_control(container_control* _parent, int _id, int _source_control_id);
			virtual ~image_control();

			void load_from_file(std::string _name);
			void load_from_resource(DWORD _resource_id);
			void load_from_control(int _control_id);
		};

		class absolute_layout : 
			public container_control
		{
		public:
			absolute_layout() { ; }
			absolute_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~absolute_layout() { ; }

			virtual void arrange(rectangle _ctx);
		};

		class column_layout : 
			public container_control
		{
			layout_rect item_size;
		public:
			column_layout() { ; }
			column_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~column_layout() { ; }

			virtual void arrange(rectangle _ctx);
			virtual point get_remaining(point _ctx);
		};

		class row_layout : 
			public container_control
		{
		protected:
		public:
			row_layout() { ; }
			row_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~row_layout() { ; }

			virtual void arrange(rectangle _ctx);
			virtual point get_remaining(point _ctx);
		};

		class frame_layout :
			public container_control
		{
		protected:
			std::string selected_page_name;

		public:
			frame_layout() { ; }
			frame_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~frame_layout() { ; }

			virtual void set_page(page& _page);
		};

		class banner_control :
			public row_layout
		{
		protected:
			std::string logo_filename;
		public:
			banner_control() { ; }
			banner_control(container_control* _parent, int _id) : row_layout(_parent, _id) { ; }
			virtual ~banner_control() { ; }

			virtual void arrange(rectangle _ctx);
			virtual point get_remaining(point _ctx);
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class windows_control : public control_base
		{

			void set_default_styles()
			{
				text_style = {};
				text_style.name = "windows_control_style";
				text_style.fontName = styles.get_style().PrimaryFont;
				text_style.fontSize = 12;
				text_style.bold = false;
				text_style.italics = false;
				text_style.underline = false;
				text_style.strike_through = false;
				text_style.horizontal_align = visual_alignment::align_near;
				text_style.vertical_align = visual_alignment::align_near;
				text_style.wrap_text = true;
			}

			CFont text_font;

		public:

			using control_base::id;

			WtlWindowClass window;
			std::weak_ptr<win32::directApplicationWin32> window_host;
			textStyleRequest	text_style;

			windows_control()
			{
				set_origin(0.0_px, 0.0_px);
				set_size(1.0_container, 1.2_fontgr);
				set_default_styles();
			}

			windows_control(container_control* _parent, int _id) : control_base(_parent, _id)
			{
				set_origin(0.0_px, 0.0_px);
				set_size(1.0_container, 1.2_fontgr);
				set_default_styles();
			}

			virtual double get_font_size() { return text_style.fontSize; }

			virtual void on_resize()
			{
				if (auto phost = window_host.lock()) {
					HWND parent = phost->getMainWindow();

					auto boundsPixels = phost->toPixelsFromDips(inner_bounds);
					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + boundsPixels.w;
					r.bottom = boundsPixels.y + boundsPixels.h;

					if (((HWND)window) != nullptr) {
						window.MoveWindow(&r);
					}
				}
			}

			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host)
			{
				window_host = _host;

				if (auto phost = window_host.lock()) {
					auto boundsPixels = phost->toPixelsFromDips(inner_bounds);

					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + boundsPixels.w;
					r.bottom = boundsPixels.y + boundsPixels.h;

					if (((HWND)window) == nullptr) {
						HWND parent = phost->getMainWindow();
						window.Create(parent, r, NULL, dwStyle, dwExStyle, id, NULL);
						HFONT font = phost->createFontDips(window, text_style.fontName, text_style.fontSize, text_style.bold, text_style.italics);
						text_font.Attach(font);
						window.SetFont(text_font);
						on_create();
					}
				}
			}

			virtual void on_create() { ;  }

			virtual void destroy()
			{
				if (::IsWindow(window.m_hWnd)) {
					window.DestroyWindow();
				}
			}

			virtual ~windows_control()
			{
				destroy();
			}
		};

		class list_data
		{
		public:
			std::string id_field;
			std::string text_field;
			json items;
		};

		class table_column
		{
		public:
			std::string display_name;
			std::string json_field;
			int width;
			visual_alignment alignment;
		};

		class table_data
		{
		public:
			std::vector<table_column> columns;
			std::string id_field;
			json items;
		};


		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class text_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
			std::string text;

		public:

			using control_base::id;
			using windows_control<WtlWindowClass,dwStyle,dwExStyle>::window_host;

			text_control_base()
			{
				;
			}

			text_control_base(container_control* _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				;
			}

			virtual ~text_control_base() { ; }

			void set_text(const std::string& _text)
			{
				text = _text;
				if (auto phost = window_host.lock()) {
					phost->setEditText(id, _text);
				}
			}

			std::string get_text()
			{
				if (auto phost = window_host.lock()) {
					text = phost->getEditText(id);
				}
				return text;
			}

			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host)
			{
				windows_control<WtlWindowClass, dwStyle, dwExStyle>::create(_host);
				if (auto phost = window_host.lock()) {
					phost->setEditText(id, text);
				}
			}

		};

		class mini_table 
		{
			std::shared_ptr<dynamic_box> box;
			std::vector<relative_ptr_type> indeces;
			int row_size;
			int max_rows;
		public:

			mini_table(  )
			{
				row_size = 1;
				max_rows = 1;
				box = std::make_shared<dynamic_box>();
			}

			void init(int _row_size, int _max_rows)
			{
				max_rows = _max_rows;
				row_size = _row_size;
				indeces.clear();
				int c = (max_rows + 1) * row_size;
				if (indeces.size() < max_rows)
				{
					indeces.resize(c);
				}
				int b = c * 100;
				box->init(b);
			}

			char *set(int i, int j, std::string& src)
			{
				int idx = j * row_size + i;

				indeces[idx] = box->put_null_terminated(src.c_str(), 0);
				char *temp = box->get_object<char>(indeces[idx]);
				return temp;
			}
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class table_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
			mini_table mtable;
			char blank[256] = { 0 };

			void data_changed()
			{
				strcpy_s(blank, "N/A");
				if (auto phost = window_host.lock())
				{
					phost->clearListView(id);
					int row_size = choices.columns.size();
					int num_rows = choices.items.size() * 2;
					mtable.init(row_size, num_rows);
					std::map<std::string, int> column_map;
					int row_index = 0;
					int col_index = 0;
					for (auto col : choices.columns) {
						char *t = mtable.set(col_index, row_index, col.display_name);
						phost->addListViewColumn(id, col_index, t, col.width, col.alignment);
						column_map[col.json_field] = col_index;
						col_index++;
					}
					std::vector<char*> data_row;
					data_row.resize(choices.columns.size());
					row_index++;
					for (auto item : choices.items)
					{
						col_index = 0;
						for (auto col : choices.columns)
						{
							data_row[col_index] = blank;
							bool has_field = item.contains(col.json_field);
							if (has_field) {
								auto item_value = item[col.json_field];
								std::string contents;
								if (item_value.is_string())
									contents = item_value;
								else if (item_value.is_number()) {
									double temp = item_value;
									contents = std::to_string(temp);
								}
								char *value = mtable.set(col_index, row_index, contents);
								if (value) {
									data_row[ col_index ] = value;
								}
							}
							col_index++;
						}
						//virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<std::string>&_items) = 0;
						phost->addListViewRow(id, row_index, data_row);
						row_index++;
					}
				}
			}

		public:
			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;
			table_data choices;

			table_control_base()
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 10.0_fontgr);
			}

			table_control_base(container_control* _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 10.0_fontgr);
			}

			virtual ~table_control_base() { ; }

			virtual void on_create() 
			{ 
				windows_control<WtlWindowClass, dwStyle, dwExStyle>::window.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
				data_changed();
			}

			void set_table(table_data& _choices)
			{
				choices = _choices;
				data_changed();
			}
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class list_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
		public:
			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;
			list_data choices;

			list_control_base()
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 10.0_fontgr);
			}

			list_control_base(container_control* _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 10.0_fontgr);
			}

			virtual ~list_control_base() { ; }

			void data_changed()
			{
				if (auto phost = window_host.lock()) {
					phost->clearListItems(id);
					for (auto element : choices.items.items())
					{
						auto c = element.value();
						int lid = c[choices.id_field].template get<int>();
						std::string description = c[choices.text_field].template get<std::string>();
						phost->addListItem(id, description, lid);
					}
				}
			}

			void set_list(list_data& _choices)
			{
				choices = _choices;
				data_changed();
			}

			virtual void on_create()
			{
				data_changed();
			}

		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class dropdown_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
		public:

			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;
			list_data choices;

			dropdown_control_base()
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 2.0_fontgr);
			}

			dropdown_control_base(container_control * _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				control_base::set_origin(0.0_px, 0.0_px);
				control_base::set_size(1.0_container, 2.0_fontgr);
			}

			virtual ~dropdown_control_base() { ; }

			void data_changed()
			{
				if (auto phost = window_host.lock()) {
					phost->clearComboItems(id);
					for (auto element : choices.items.items())
					{
						auto c = element.value();
						int lid = c[choices.id_field].template get<int>();
						std::string description = c[choices.text_field].template get<std::string>();
						phost->addComboItem(id, description, lid);
					}
				}
			}

			void set_list(list_data& _choices)
			{
				choices = _choices;
				data_changed();
			}

			virtual void on_resize()
			{
				if (auto phost = window_host.lock()) {
					auto boundsPixels = phost->toPixelsFromDips(control_base::get_inner_bounds());

					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + boundsPixels.w;
					r.bottom = boundsPixels.y + windows_control<WtlWindowClass, dwStyle, dwExStyle>::text_style.fontSize * 8;
					if (windows_control<WtlWindowClass, dwStyle, dwExStyle>::window.m_hWnd) {
						windows_control<WtlWindowClass, dwStyle, dwExStyle>::window.MoveWindow(&r);
					}
				}
			}

			virtual void on_create()
			{
				on_resize();
				data_changed();
			}

		};

		const int DefaultWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
		const int DisplayOnlyWindowStyles = WS_VISIBLE | WS_CHILD;
		const int EditWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
		const int RichEditWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
		const int ComboWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
		const int ComboExWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
		const int PushButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT;
		const int PressButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT | BS_AUTOCHECKBOX | BS_PUSHLIKE;
		const int CheckboxWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | BS_FLAT;
		const int RadioButtonWindowStyles = WS_VISIBLE |WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_FLAT;
		const int LinkButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_COMMANDLINK | BS_FLAT;
		const int ListViewWindowsStyles = DefaultWindowStyles | LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL;
		const int ListBoxWindowsStyles = DefaultWindowStyles | WS_BORDER | WS_VSCROLL;

		class static_control : public text_control_base<WTL::CStatic, DisplayOnlyWindowStyles>
		{
		public:
			static_control(container_control* _parent, int _id) : text_control_base<WTL::CStatic, DisplayOnlyWindowStyles>(_parent, _id) { ; }
			virtual ~static_control() { ; }
		};

		template <long ButtonWindowStyles> class button_control : public text_control_base<WTL::CButton, ButtonWindowStyles>
		{
			using control_base::id;
			using windows_control<WTL::CButton, ButtonWindowStyles>::window_host;
			std::string caption_text;
			long caption_icon_id;
			CIcon caption_icon;
		public:
			button_control(container_control* _parent, int _id) : text_control_base<WTL::CButton, ButtonWindowStyles>(_parent, _id) { ; }
			virtual ~button_control() { ; }

		};

		class pushbutton_control : public button_control<PushButtonWindowStyles>
		{
		public:
			pushbutton_control(container_control* _parent, int _id) : button_control<PushButtonWindowStyles>(_parent, _id) { ; }
			virtual ~pushbutton_control() { ; }
		};

		class pressbutton_control : public button_control<PressButtonWindowStyles>
		{
		public:
			pressbutton_control(container_control* _parent, int _id) : button_control<PressButtonWindowStyles>(_parent, _id) { ; }
			virtual ~pressbutton_control() { ; }
		};

		class radiobutton_control : public button_control<RadioButtonWindowStyles>
		{
		public:
			radiobutton_control(container_control* _parent, int _id) : button_control<RadioButtonWindowStyles>(_parent, _id) { ; }
			virtual ~radiobutton_control() { ; }
		};

		class checkbox_control : public button_control<CheckboxWindowStyles>
		{
		public:
			checkbox_control(container_control* _parent, int _id) : button_control<CheckboxWindowStyles>(_parent, _id) { ; }
			virtual ~checkbox_control() { ; }
		};

		class linkbutton_control : public button_control<LinkButtonWindowStyles>
		{
		public:
			linkbutton_control(container_control* _parent, int _id) : button_control<LinkButtonWindowStyles>(_parent, _id) { ; }
			virtual ~linkbutton_control() { ; }
		};

		class edit_control : public text_control_base<WTL::CEdit, EditWindowStyles>
		{
		public:
			edit_control(container_control* _parent, int _id) : text_control_base<WTL::CEdit, EditWindowStyles>(_parent, _id) { ; }
			virtual ~edit_control() { ; }
		};

		class listbox_control : public list_control_base<WTL::CListBox, ListBoxWindowsStyles>
		{
		public:
			listbox_control(container_control* _parent, int _id) : list_control_base<WTL::CListBox, ListBoxWindowsStyles>(_parent, _id) { ; }
			virtual ~listbox_control() { ; }
		};

		class combobox_control : public dropdown_control_base<WTL::CComboBox, ComboWindowStyles>
		{
		public:
			combobox_control(container_control* _parent, int _id) : dropdown_control_base<WTL::CComboBox, ComboWindowStyles>(_parent, _id) { ; }
			virtual ~combobox_control() { ; }
		};

		class comboboxex_control : public windows_control<WTL::CComboBoxEx, ComboExWindowStyles>
		{
		public:
			using control_base::id;
			using windows_control<WTL::CComboBoxEx, ComboExWindowStyles>::window_host;
			list_data choices;

			comboboxex_control();
			comboboxex_control(container_control* _parent, int _id);
			virtual ~comboboxex_control() { ; }
			void data_changed();
			void set_list(list_data& _choices);
			virtual void on_create();
			virtual void on_resize();
		};

		class listview_control : public table_control_base<WTL::CListViewCtrl, ListViewWindowsStyles>
		{
		public:
			listview_control(container_control* _parent, int _id) : table_control_base<WTL::CListViewCtrl, ListViewWindowsStyles>(_parent, _id) { ; }
			virtual ~listview_control() { ; }
		};

		class scrollbar_control : public windows_control<WTL::CScrollBar, DefaultWindowStyles>
		{
		public:
			scrollbar_control(container_control* _parent, int _id) : windows_control<WTL::CScrollBar, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~scrollbar_control() { ; }
		};

		class richedit_control : public text_control_base<WTL::CRichEditCtrl,RichEditWindowStyles>
		{
		public:
			void set_html(const std::string& _text);
			std::string get_html();

			richedit_control(container_control* _parent, int _id) : text_control_base<WTL::CRichEditCtrl, RichEditWindowStyles>(_parent, _id) {
				LoadLibrary(TEXT("Msftedit.dll"));
			}
			virtual ~richedit_control() { ; }
		};

		class datetimepicker_control : public windows_control<CDateTimePickerCtrl, DefaultWindowStyles>
		{
		public:
			void set_text(const std::string& _text);
			std::string get_text();

			datetimepicker_control(container_control* _parent, int _id) : windows_control<CDateTimePickerCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~datetimepicker_control() { ; }
		};

		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, DefaultWindowStyles>
		{
		public:
			monthcalendar_control(container_control* _parent, int _id) : windows_control<CMonthCalendarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~monthcalendar_control() { ; }
		};

		class animate_control : public windows_control<WTL::CAnimateCtrl, DefaultWindowStyles>
		{
		public:
			animate_control(container_control* _parent, int _id) : windows_control<WTL::CAnimateCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~animate_control() { ; }

			bool open(const std::string& _name);
			bool open(DWORD resource_id);
			bool play(UINT from, UINT to, UINT rep);
			bool play();
			bool stop();
		};

		class treeview_control : public windows_control<WTL::CTreeViewCtrl, DefaultWindowStyles>
		{
		public:
			treeview_control(container_control* _parent, int _id) : windows_control<WTL::CTreeViewCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~treeview_control() { ; }
		};

		class header_control : public windows_control<WTL::CHeaderCtrl, DefaultWindowStyles>
		{
		public:
			header_control(container_control* _parent, int _id) : windows_control<WTL::CHeaderCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~header_control() { ; }
		};

		class toolbar_control : public windows_control<WTL::CToolBarCtrl, DefaultWindowStyles>
		{
		public:
			toolbar_control(container_control* _parent, int _id) : windows_control<WTL::CToolBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~toolbar_control() { ; }
		};

		class statusbar_control : public windows_control<WTL::CStatusBarCtrl, DefaultWindowStyles>
		{
		public:
			statusbar_control(container_control* _parent, int _id) : windows_control<WTL::CStatusBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~statusbar_control() { ; }

		};

		class hotkey_control : public windows_control<WTL::CHotKeyCtrl, DefaultWindowStyles>
		{
		public:
			hotkey_control(container_control* _parent, int _id) : windows_control<WTL::CHotKeyCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~hotkey_control() { ; }
		};

		class draglistbox_control : public windows_control<WTL::CDragListBox, DefaultWindowStyles>
		{
		public:
			draglistbox_control(container_control* _parent, int _id) : windows_control<WTL::CDragListBox, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~draglistbox_control() { ; }
		};

		class rebar_control : public windows_control<WTL::CReBarCtrl, DefaultWindowStyles>
		{
		public:
			rebar_control(container_control* _parent, int _id) : windows_control<WTL::CReBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~rebar_control() { ; }
		};

		enum class field_layout 
		{
			label_on_left = 1,
			label_on_top = 2
		};


		class control_event
		{
		public:
			int control_id;
			control_base *control;
		};

		class command_event : public control_event
		{
		public:
		};

		class mouse_event : public control_event
		{
		public:
			point absolute_point;
			point relative_point;
		};

		class mouse_move_event : public mouse_event
		{

		};

		class mouse_click_event : public mouse_event
		{

		};

		class mouse_left_click_event : public mouse_event
		{

		};

		class mouse_right_click_event : public mouse_event
		{

		};

		class key_event : public control_event
		{
		public:
			short key;
		};

		class key_down_event : public key_event
		{
		public:

		};

		class key_up_event : public key_event
		{
		public:

		};

		class draw_event : public control_event
		{
		public:
			int state;
		};

		class item_changed_event : public control_event
		{
		public:
			std::string text_value;
		};

		class list_changed_event : public control_event
		{
		public:
			int state;

			std::string selected_text;
			int selected_index;
			relative_ptr_type selected_value;
		};

		class key_up_event_binding
		{
		public:
			int subscribed_item_id;
			std::function< void(key_up_event) > on_key_up;
		};

		class key_down_event_binding
		{
		public:
			int subscribed_item_id;
			std::function< void(key_down_event) > on_key_down;
		};

		class mouse_move_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(mouse_move_event) > on_mouse_move;
		};

		class mouse_click_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(mouse_click_event) > on_mouse_click;
		};

		class mouse_left_click_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(mouse_left_click_event) > on_mouse_left_click;
		};

		class mouse_right_click_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(mouse_right_click_event) > on_mouse_right_click;
		};

		class draw_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(draw_event) > on_draw;
		};

		class item_changed_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(item_changed_event) > on_change;
		};

		class command_event_binding
		{
		public:
			int subscribed_item_id;
			std::function< void(command_event) > on_command;
		};

		class list_changed_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(list_changed_event) > on_change;
		};

		using update_function = std::function< void(page* _page, double _elapsedSeconds, double _totalSeconds) >;

		class page : public std::enable_shared_from_this<page>
		{

			rectangle layout(control_base* _item, layout_context _ctx);
			std::map<int, std::shared_ptr<key_up_event_binding> > key_up_events;
			std::map<int, std::shared_ptr<key_down_event_binding> > key_down_events;
			std::map<int, std::shared_ptr<mouse_move_event_binding> > mouse_move_events;
			std::map<int, std::shared_ptr<mouse_click_event_binding> > mouse_click_events;
			std::map<int, std::shared_ptr<mouse_left_click_event_binding> > mouse_left_click_events;
			std::map<int, std::shared_ptr<mouse_right_click_event_binding> > mouse_right_click_events;
			std::map<int, std::shared_ptr<item_changed_event_binding> > item_changed_events;
			std::map<int, std::shared_ptr<list_changed_event_binding> > list_changed_events;
			std::map<int, std::shared_ptr<command_event_binding> > command_events;
			update_function update_event;

		protected:

			void handle_key_up(int _control_id, key_up_event evt);
			void handle_key_down(int _control_id, key_down_event evt);
			void handle_mouse_move(int _control_id, mouse_move_event evt);
			void handle_mouse_click(int _control_id, mouse_click_event evt);
			void handle_mouse_left_click(int _control_id, mouse_left_click_event evt);
			void handle_mouse_right_click(int _control_id, mouse_right_click_event evt);
			void handle_item_changed(int _control_id, item_changed_event evt);
			void handle_list_changed(int _control_id, list_changed_event evt);
			void handle_command(int _command_id, command_event evt);

			void arrange(double _width, double _height, double _padding = 0.0);

			virtual void create(std::weak_ptr<corona::win32::directApplicationWin32> _host);
			virtual void destroy();
			virtual void draw();
			virtual void render(CComPtr<ID2D1DeviceContext>& _context);
			virtual void update(double _elapsedSeconds, double _totalSeconds);

			void subscribe(presentation* _presentation);

		public:

			std::shared_ptr<menu_item> menu;
			std::shared_ptr<control_base> root;
			std::string name;

			page(const char* _name = nullptr);
			virtual ~page();

			void clear();

			menu_item& create_menu();

			void on_key_up( int _control_id, std::function< void(key_up_event) > );
			void on_key_down( int _control_id, std::function< void(key_down_event) > );
			void on_mouse_move( int _control_id, std::function< void(mouse_move_event) > );
			void on_mouse_click( int _control_id, std::function< void(mouse_click_event) > );
			void on_mouse_left_click(int _control_id, std::function< void(mouse_left_click_event) >);
			void on_mouse_right_click(int _control_id, std::function< void(mouse_right_click_event) >);
			void on_item_changed( int _control_id, std::function< void(item_changed_event) > );
			void on_list_changed( int _control_id, std::function< void(list_changed_event) > );
			void on_command(int _item_id, std::function< void(command_event) >);
			void on_update(update_function fnc);

			row_layout& row_begin(int id = id_counter::next());
			column_layout& column_begin(int id = id_counter::next());
			absolute_layout& absolute_begin(int id = id_counter::next());
			control_base& end();		

			inline control_base* get_root() {
				return root.get();
			}

			control_base *operator[](int _id)
			{
				return get_root()->find(_id);
			}

			friend class presentation;
		};

		class presentation : public win32::controller
		{
		protected:

			std::weak_ptr<page> current_page;
			rectangle current_size;

		public:

			std::map<std::string, std::shared_ptr<page>> pages;

			presentation();
			virtual ~presentation();

			void open_menu(control_base* _base, menu_item& _menu);

			virtual page& create_page(std::string _name, std::function<void(page& pg)> _settings = nullptr);
			virtual void select_page(const std::string& _page_name);
			menu_item& create_menu();

			template <typename control_type> control_type& find(int _id)
			{
				if (auto cp = current_page.lock()) {
					control_base* temp = cp->root->find(_id);
					if (temp == nullptr)
					{
						auto str = std::format("Control {0} not found ", _id);
						throw std::invalid_argument(str);
					}
					control_type* citem = dynamic_cast<control_type *>(temp);
					if (citem == nullptr)
					{
						auto str = std::format("Object is not {0} ", typeid(control_type).name());
						throw std::invalid_argument(str);
					}
					return *citem;
				}
				throw std::exception("could not lock current page");
			}

			template <typename control_type> control_type& get(control_base* _root, int _id)
			{
				if (auto cp = current_page.lock()) {
					control_base* temp = cp->root->get(_root, _id);
					if (temp == nullptr)
					{
						auto str = std::format("Control {0} not found ", _id);
						throw std::invalid_argument(str);
					}
					control_type* citem = dynamic_cast<control_type *>(temp);
					if (citem == nullptr)
					{
						auto str = std::format("Object is not {0} ", typeid(*citem).name());
							throw std::invalid_argument(str);
					}
					return *citem;
				}
				throw std::exception("could not lock current page");
			}

			virtual bool drawFrame(win32::direct2dContext& _ctx);
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key);
			virtual void keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key);
			virtual void mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void mouseLeftDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void mouseLeftUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void mouseRightDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void mouseRightUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color);
			virtual LRESULT ncHitTest(std::shared_ptr<win32::direct2dWindow>& win, point* _point);

			virtual void onCreated();
			virtual void onCommand(int buttonId);
			virtual void onTextChanged(int textControlId);
			virtual void onDropDownChanged(int dropDownId);
			virtual void onListBoxChanged(int listBoxId);
			virtual void onListViewChanged(int listViewId);
			virtual int onHScroll(int controlId, win32::scrollTypes scrollType);
			virtual int onVScroll(int controlId, win32::scrollTypes scrollType);
			virtual int onResize(const rectangle& newSize, double d2dScale);
			virtual int onSpin(int controlId, int newPosition);

			template <typename control_type> control_type* get_control(int _id)
			{
				control_type* r = nullptr;
				if (auto ppage = current_page.lock()) 
				{
					auto& rpage = *ppage;
					control_base* cb = rpage[_id];
					r = dynamic_cast<control_type*>(cb);
				}
				return r;
			}

			template <typename control_type> control_type* get_parent_control(int _id)
			{
				control_type* r = nullptr;
				if (auto ppage = current_page.lock())
				{
					auto& rpage = *ppage;
					control_base* cb = rpage[_id];
					r = dynamic_cast<control_type*>(cb);
					while (!r && cb) {
						cb = cb->parent;
						if (cb) {
							r = dynamic_cast<control_type*>(cb);
						}
					}
				}
				return r;
			}

		};
	}
}
