#pragma once

namespace corona
{
	namespace database
	{
		using namespace win32;

		class id_counter
		{
		public:
			static int id;
			static int next();
			static int check(int _id);
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

		class row_layout;
		class column_layout;
		class absolute_layout;

		class title_control;
		class subtitle_control;
		class chaptertitle_control;
		class chaptersubtitle_control;
		class paragraph_control;
		class code_control;
		class image_control;

		class static_control;
		class button_control;
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

		class control_base : public std::enable_shared_from_this<control_base>
		{
		protected:
			point get_size(rectangle _ctx, point _remaining);
			point get_position(rectangle _ctx);
			double get_item_space(rectangle _ctx);
			virtual point get_remaining(point _ctx);
			virtual void on_resize();
			void arrange_children(rectangle _bounds,
				std::function<point(rectangle *_bounds, control_base*)> _initial_origin,
				std::function<point(point* _origin, rectangle *_bounds, control_base*)> _next_origin);

		public:

			friend class absolute_layout;
			friend class row_layout;
			friend class column_layout;
			friend class draw_control;

			static int				debug_indent;

			int						id;

			visual_alignment		alignment;
			layout_rect				box;
			measure					item_space;

			rectangle				bounds;
			point					item_space_amount;

			win32::directApplicationWin32* app;
			control_base *parent;

			std::vector<std::shared_ptr<control_base>> children;

			control_base() :
				id(-1),
				item_space(),
				parent(nullptr),
				item_space_amount({ 0.0, 0.0 }),
				alignment(visual_alignment::align_near)
			{
				id = id_counter::next();
			}

			control_base(control_base* _parent, int _id) : control_base()
			{
				parent = _parent;
				if (_id < 0) {
					id = id_counter::next();
				}
				else {
					id = _id;
				}
			}

			virtual void arrange(rectangle _ctx);
			bool contains(point pt);

			control_base* find(int _id);
			control_base* get(control_base* _root, int _id);
			void foreach(std::function<void(control_base* _root)> _item);

			virtual void create(std::weak_ptr<win32::win32ControllerHost> _host);
			virtual void destroy();
			virtual void draw();

			template <typename control_type> control_type& create(int _id)
			{
				id_counter::check(_id);
				std::shared_ptr<control_type> temp = std::make_shared<control_type>(this, _id);
				children.push_back(temp);
				std::string indent(debug_indent, ' ');
				std::cout << indent << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
				return *temp.get();
			}

			control_base* set_align_base(visual_alignment _new_alignment);
			control_base* set_origin_base(measure _x, measure _y);
			control_base* set_size_base(measure _width, measure _height);
			control_base* set_position_base(layout_rect _new_layout);
			control_base* set_spacing_base(measure _spacing);

			control_base& set_align(visual_alignment _new_alignment)
			{
				alignment = _new_alignment;
				return *this;
			}

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

			template <typename control_class> control_class& set_spacing(measure _spacing)
			{
				control_class* r = dynamic_cast<control_class>(this);
				r->item_space = _spacing;
				return *this;
			}

			row_layout& row_begin(int id = id_counter::next());
			column_layout& column_begin(int id = id_counter::next());
			absolute_layout& absolute_begin(int id = id_counter::next());
			control_base& end();

			control_base& title(std::string _text, int id = id_counter::next());
			control_base& subtitle(std::string _text, int id = id_counter::next());
			control_base& chaptertitle(std::string _text, int id = id_counter::next());
			control_base& chaptersubtitle(std::string _text, int id = id_counter::next());
			control_base& paragraph(std::string _text, int id = id_counter::next());
			control_base& code(std::string _text, int id = id_counter::next());

			control_base& title(int id = id_counter::next());
			control_base& subtitle(int id = id_counter::next());
			control_base& chaptertitle(int id = id_counter::next());
			control_base& chaptersubtitle(int id = id_counter::next());
			control_base& paragraph(int id = id_counter::next());
			control_base& code(int id = id_counter::next());

			control_base& image(int id = id_counter::next());

			control_base& label(int id = id_counter::next());
			control_base& button(int id);
			control_base& listbox(int id);
			control_base& combobox(int id);
			control_base& edit(int id);
			control_base& scrollbar(int id);

			control_base& listview(int id);
			control_base& treeview(int id);
			control_base& header(int id);
			control_base& toolbar(int id);
			control_base& statusbar(int id);
			control_base& hotkey(int id);
			control_base& animate(int id);
			control_base& richedit(int id);
			control_base& draglistbox(int id);
			control_base& rebar(int id);
			control_base& comboboxex(int id);
			control_base& datetimepicker(int id);
			control_base& monthcalendar(int id);

		};

		class draw_control : public control_base
		{
			void init();
		protected:
			std::weak_ptr<win32::win32ControllerHost> host;
			std::weak_ptr<win32::direct2dChildWindow> window;
			std::function<void(draw_control*)> on_draw;
			std::function<void(draw_control*)> on_create;

			draw_control();
			draw_control(control_base* _parent, int _id);
			virtual void create(std::weak_ptr<win32::win32ControllerHost> _host);
			virtual void destroy();
			virtual void draw();
			virtual void on_resize();
		};

		class container_control : public draw_control
		{
		public:
			container_control();
			container_control(control_base * _parent, int _id);
			virtual ~container_control() { ; }
		};

		class text_display_control : public draw_control
		{
		public:
			std::string			text;
			solidBrushRequest	text_fill_brush;
			textStyleRequest	text_style;

			text_display_control();
			text_display_control(control_base * _parent, int _id); 

			void init();
			text_display_control& set_text(std::string _text);
			text_display_control& set_text_fill(solidBrushRequest _brushFill);
			text_display_control& set_text_fill(std::string _color);
			text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
			text_display_control& set_text_style(textStyleRequest request);

		};

		class title_control : public text_display_control
		{
			void set_default_styles();
		public:
			title_control();
			title_control(control_base* _parent, int _id);
		};

		class subtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			subtitle_control();
			subtitle_control(control_base* _parent, int _id);
		};

		class chaptertitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptertitle_control();
			chaptertitle_control(control_base* _parent, int _id);
		};

		class chaptersubtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptersubtitle_control();
			chaptersubtitle_control(control_base* _parent, int _id);
		};

		class paragraph_control : public text_display_control
		{
			void set_default_styles();
		public:
			paragraph_control();
			paragraph_control(control_base* _parent, int _id);
		};

		class code_control : public text_display_control
		{
			void set_default_styles();
		public:
			code_control();
			code_control(control_base* _parent, int _id);
		};

		class image_control : 
			public draw_control
		{
		public:
			image_control();
			image_control(control_base * _parent, int _id) : draw_control(_parent, _id) { ; }
		};

		class absolute_layout : 
			public container_control
		{
		public:
			absolute_layout() { ; }
			absolute_layout(control_base * _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~absolute_layout() { ; }

			virtual void arrange(rectangle _ctx);
		};

		class column_layout : 
			public container_control
		{
		public:
			column_layout() { ; }
			column_layout(control_base * _parent, int _id) : container_control(_parent, _id) { ; }
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
			row_layout(control_base * _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~row_layout() { ; }

			virtual void arrange(rectangle _ctx);
			virtual point get_remaining(point _ctx);
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class windows_control : public control_base
		{
		public:

			using control_base::id;

			WtlWindowClass window;
			std::weak_ptr<win32::win32ControllerHost> window_host;

			windows_control()
			{
				set_origin_base(0.0_px, 0.0_px);
				set_size_base(150.0_px, 2.0_fontgr);
			}

			windows_control(control_base * _parent, int _id) : control_base(_parent, _id)
			{
				set_origin_base(0.0_px, 0.0_px);
				set_size_base(150.0_px, 2.0_fontgr);
			}

			virtual void on_resize()
			{
				if (auto phost = window_host.lock()) {
					HWND parent = phost->getMainWindow();

					auto boundsPixels = phost->toPixelsFromDips(bounds);
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

			virtual void create(std::weak_ptr<win32::win32ControllerHost> _host)
			{
				window_host = _host;

				if (auto phost = window_host.lock()) {
					HWND parent = phost->getMainWindow();

					auto boundsPixels = phost->toPixelsFromDips(bounds);

					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + bounds.w;
					r.bottom = boundsPixels.y + bounds.h;

					if (((HWND)window) == nullptr) {
						window.Create(parent, r, NULL, dwStyle, dwExStyle, id, NULL);
					}
				}
			}

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
		public:

			using control_base::id;
			using windows_control<WtlWindowClass,dwStyle,dwExStyle>::window_host;

			text_control_base()
			{
				;
			}

			text_control_base(control_base * _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				;
			}

			void set_text(const std::string& _text)
			{
				window_host->setEditText(id, _text);
			}

			std::string get_text()
			{
				return window_host->getEditText(id);
			}
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class table_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
		public:
			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;

			table_control_base()
			{
				;
			}

			table_control_base(control_base * _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				;
			}

			void set_table(table_data& choices)
			{
				window_host->clearListView();
				int idx = 1;
				std::map<std::string, int> column_map;
				for (auto col : choices.columns) {
					//virtual void addListViewColumn(int ddlControlId, int column_id, const char* _text, int _width, visual_alignment _alignment);
					window_host->addListViewColumn(id, idx, col.display_name, col.width, col.alignment);
					column_map[col.json_field] = idx;
				}
				int row_idx = 0;
				for (auto item : choices.items)
				{
					std::vector<std::string> data;
					for (auto col : choices.columns) 
					{
						bool has_field = item.contains(col.json_field);
						if (has_field) {
							std::string contents = item[col.json_field].get<std::string>();
							data.push_back(contents);
						}
					}
					//virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<std::string>&_items) = 0;
					window_host->addListViewRow(id, row_idx, data);
					row_idx++;
				}
			}
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class list_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
		public:
			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;

			list_control_base()
			{
				;
			}

			list_control_base(control_base * _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				;
			}

			void set_list(list_data& choices)
			{
				window_host->clearListItems();
				for (auto element : choices.items.items()) 
				{
					auto c = element.value();
					int id = c[choices.id_field].template get<int>();
					std::string description = c[choices.text_field].template get<std::string>();
					window_host->addListItem(id, description, id);
				}
			}
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class dropdown_control_base : public windows_control<WtlWindowClass, dwStyle, dwExStyle>
		{
		public:

			using control_base::id;
			using windows_control<WtlWindowClass, dwStyle, dwExStyle>::window_host;

			dropdown_control_base()
			{
				;
			}

			dropdown_control_base(control_base * _parent, int _id) : windows_control<WtlWindowClass, dwStyle, dwExStyle>(_parent, _id)
			{
				;
			}

			void set_list(list_data& choices)
			{
				window_host->clearComboItems();
				for (auto element : choices.items.items())
				{
					auto c = element.value();
					int id = c[choices.id_field].template get<int>();
					std::string description = c[choices.text_field].template get<std::string>();
					window_host->addComboItem(id, description, id);
				}
			}
		};

		const int DefaultWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP;

		class static_control : public text_control_base<WTL::CStatic, DefaultWindowStyles>
		{
		public:
			static_control(control_base * _parent, int _id) : text_control_base<WTL::CStatic, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~static_control() { ; }
		};

		class button_control : public text_control_base<WTL::CButton, DefaultWindowStyles>
		{
		public:
			button_control(control_base * _parent, int _id) : text_control_base<WTL::CButton, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~button_control() { ; }
		};

		class edit_control : public text_control_base<WTL::CEdit, DefaultWindowStyles>
		{
		public:
			edit_control(control_base * _parent, int _id) : text_control_base<WTL::CEdit, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~edit_control() { ; }
		};

		class listbox_control : public list_control_base<WTL::CListBox, DefaultWindowStyles>
		{
		public:
			listbox_control(control_base * _parent, int _id) : list_control_base<WTL::CListBox, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~listbox_control() { ; }
		};

		class combobox_control : public list_control_base<WTL::CComboBox, DefaultWindowStyles>
		{
		public:
			combobox_control(control_base * _parent, int _id) : list_control_base<WTL::CComboBox, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~combobox_control() { ; }
		};

		class comboboxex_control : public list_control_base<WTL::CComboBoxEx, DefaultWindowStyles>
		{
		public:
			comboboxex_control(control_base * _parent, int _id) : list_control_base<WTL::CComboBoxEx, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~comboboxex_control() { ; }
		};

		class listview_control : public table_control_base<WTL::CListViewCtrl, DefaultWindowStyles>
		{
		public:
			listview_control(control_base * _parent, int _id) : table_control_base<WTL::CListViewCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~listview_control() { ; }
		};

		class scrollbar_control : public windows_control<WTL::CScrollBar, DefaultWindowStyles>
		{
		public:
			scrollbar_control(control_base * _parent, int _id) : windows_control<WTL::CScrollBar, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~scrollbar_control() { ; }
		};

		class richedit_control : public text_control_base<WTL::CRichEditCtrl, DefaultWindowStyles>
		{
		public:
			void set_html(const std::string& _text);
			std::string get_html();

			richedit_control(control_base * _parent, int _id) : text_control_base<WTL::CRichEditCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~richedit_control() { ; }
		};

		class datetimepicker_control : public windows_control<CDateTimePickerCtrl, DefaultWindowStyles>
		{
		public:
			void set_text(const std::string& _text);
			std::string get_text();

			datetimepicker_control(control_base * _parent, int _id) : windows_control<CDateTimePickerCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~datetimepicker_control() { ; }
		};

		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, DefaultWindowStyles>
		{
		public:
			monthcalendar_control(control_base * _parent, int _id) : windows_control<CMonthCalendarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~monthcalendar_control() { ; }
		};

		class animate_control : public windows_control<WTL::CAnimateCtrl, DefaultWindowStyles>
		{
		public:
			animate_control(control_base * _parent, int _id) : windows_control<WTL::CAnimateCtrl, DefaultWindowStyles>(_parent, _id) { ; }
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
			treeview_control(control_base * _parent, int _id) : windows_control<WTL::CTreeViewCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~treeview_control() { ; }
		};

		class header_control : public windows_control<WTL::CHeaderCtrl, DefaultWindowStyles>
		{
		public:
			header_control(control_base * _parent, int _id) : windows_control<WTL::CHeaderCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~header_control() { ; }
		};

		class toolbar_control : public windows_control<WTL::CToolBarCtrl, DefaultWindowStyles>
		{
		public:
			toolbar_control(control_base * _parent, int _id) : windows_control<WTL::CToolBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~toolbar_control() { ; }
		};

		class statusbar_control : public windows_control<WTL::CStatusBarCtrl, DefaultWindowStyles>
		{
		public:
			statusbar_control(control_base * _parent, int _id) : windows_control<WTL::CStatusBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~statusbar_control() { ; }

		};

		class hotkey_control : public windows_control<WTL::CHotKeyCtrl, DefaultWindowStyles>
		{
		public:
			hotkey_control(control_base * _parent, int _id) : windows_control<WTL::CHotKeyCtrl, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~hotkey_control() { ; }
		};

		class draglistbox_control : public windows_control<WTL::CDragListBox, DefaultWindowStyles>
		{
		public:
			draglistbox_control(control_base * _parent, int _id) : windows_control<WTL::CDragListBox, DefaultWindowStyles>(_parent, _id) { ; }
			virtual ~draglistbox_control() { ; }
		};

		class rebar_control : public windows_control<WTL::CReBarCtrl, DefaultWindowStyles>
		{
		public:
			rebar_control(control_base * _parent, int _id) : windows_control<WTL::CReBarCtrl, DefaultWindowStyles>(_parent, _id) { ; }
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

		class draw_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(mouse_click_event) > on_mouse_click;
		};

		class item_changed_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(item_changed_event) > on_change;
		};

		class list_changed_event_binding
		{
		public:
			int subscribed_item_id;
			std::weak_ptr<control_base> control;
			std::function< void(list_changed_event) > on_change;
		};

		class page;

		using update_function = std::function< void(page* _page, double _elapsedSeconds, double _totalSeconds) >;

		class page
		{

			rectangle layout(control_base* _item, layout_context _ctx);
			std::map<int, std::shared_ptr<key_up_event_binding> > key_up_events;
			std::map<int, std::shared_ptr<key_down_event_binding> > key_down_events;
			std::map<int, std::shared_ptr<mouse_move_event_binding> > mouse_move_events;
			std::map<int, std::shared_ptr<mouse_click_event_binding> > mouse_click_events;
			std::map<int, std::shared_ptr<item_changed_event_binding> > item_changed_events;
			std::map<int, std::shared_ptr<list_changed_event_binding> > list_changed_events;
			update_function update_event;

		protected:

			void handle_key_up(int _control_id, key_up_event evt);
			void handle_key_down(int _control_id, key_down_event evt);
			void handle_mouse_move(int _control_id, mouse_move_event evt);
			void handle_mouse_click(int _control_id, mouse_click_event evt);
			void handle_item_changed(int _control_id, item_changed_event evt);
			void handle_list_changed(int _control_id, list_changed_event evt);

			void arrange(double _width, double _height, double _padding = 0.0);

			virtual void create(std::weak_ptr<corona::win32::win32ControllerHost> _host);
			virtual void destroy();
			virtual void draw();
			virtual void update(double _elapsedSeconds, double _totalSeconds);

		public:

			std::shared_ptr<control_base> root;
			std::string name;

			page(const char* _name = nullptr);
			virtual ~page();

			void clear();

			void on_key_up( int _control_id, std::function< void(key_up_event) > );
			void on_key_down( int _control_id, std::function< void(key_down_event) > );
			void on_mouse_move( int _control_id, std::function< void(mouse_move_event) > );
			void on_mouse_click( int _control_id, std::function< void(mouse_click_event) > );
			void on_item_changed( int _control_id, std::function< void(item_changed_event) > );
			void on_list_changed( int _control_id, std::function< void(list_changed_event) > );
			void on_update(update_function fnc);

			row_layout& row_begin(int id = id_counter::next());
			column_layout& column_begin(int id = id_counter::next());
			absolute_layout& absolute_begin(int id = id_counter::next());
			control_base& end();		

			control_base* get_root();

			const control_base *operator[](int _id)
			{
				return get_root()->find(_id);
			}

			friend class presentation;
		};

		class presentation : public win32::controller
		{

			std::weak_ptr<page> current_page;

		public:

			std::map<std::string, std::shared_ptr<page>> pages;

			presentation();
			virtual ~presentation();

			virtual page& create_page(std::string _name);
			virtual void select_page(const std::string& _page_name);

			virtual bool drawFrame();
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key);
			virtual void keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key);
			virtual void mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void mouseClick(std::shared_ptr<win32::direct2dWindow>& win, point* _point);
			virtual void pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color);

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

		};
	}
}
