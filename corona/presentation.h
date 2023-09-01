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

		class control_base : public std::enable_shared_from_this<control_base>
		{
		protected:
			point get_size(rectangle _ctx, point _remaining);
			point get_position(rectangle _ctx);
			double get_margin(measure _margin);
			virtual point get_remaining(point _ctx);
			virtual void on_resize();
			void arrange_children(rectangle _bounds, int zorder, 
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
			measure					margin;

			rectangle				bounds;
			point					margin_amount;

			win32::directApplicationWin32* app;
			container_control *parent;

			std::vector<std::shared_ptr<control_base>> children;

			control_base() :
				id(-1),
				margin(),
				parent(nullptr),
				margin_amount({ 0.0, 0.0 }),
				alignment(visual_alignment::align_near)
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

			virtual void arrange(rectangle _ctx, int zorder);
			bool contains(point pt);

			control_base* find(int _id);
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

			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host);
			virtual void destroy();
			virtual void draw();

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

		};

		class draw_control : public control_base
		{
			void init();
		protected:

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
			virtual void on_resize();
		};

		class container_control : public draw_control
		{

		public:

			layout_rect				item_box = {};
			visual_alignment		item_alignment = visual_alignment::align_none;
			measure					item_margin = {};

			container_control();
			container_control(container_control* _parent, int _id);
			virtual ~container_control() { ; }

			virtual void apply(control_base& _ref);

			template <typename control_type> control_type& create(int _id)
			{
				id_counter::check(_id);
				std::shared_ptr<control_type> temp = std::make_shared<control_type>(this, _id);
				children.push_back(temp);
				std::string indent(debug_indent, ' ');
				std::cout << indent << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
				return *temp.get();
			}

			container_control& set_item_align(visual_alignment _new_alignment);
			container_control& set_item_origin(measure _x, measure _y);
			container_control& set_item_size(measure _width, measure _height);
			container_control& set_item_position(layout_rect _new_layout);
			container_control& set_item_margin(measure _item_margin);

			container_control& set_align(visual_alignment _new_alignment);
			container_control& set_origin(measure _x, measure _y);
			container_control& set_size(measure _width, measure _height);
			container_control& set_position(layout_rect _new_layout);
			container_control& set_margin(measure _space);

			container_control& set_background_color(solidBrushRequest _brushFill);
			container_control& set_background_color(std::string _color);

			row_layout& row_begin(int id = id_counter::next());
			column_layout& column_begin(int id = id_counter::next());
			absolute_layout& absolute_begin(int id = id_counter::next());
			container_control& end();

			container_control& title(std::string _text, int id = id_counter::next());
			container_control& subtitle(std::string _text, int id = id_counter::next());
			container_control& chaptertitle(std::string _text, int id = id_counter::next());
			container_control& chaptersubtitle(std::string _text, int id = id_counter::next());
			container_control& paragraph(std::string _text, int id = id_counter::next());
			container_control& code(std::string _text, int id = id_counter::next());

			container_control& title(int id = id_counter::next());
			container_control& subtitle(int id = id_counter::next());
			container_control& chaptertitle(int id = id_counter::next());
			container_control& chaptersubtitle(int id = id_counter::next());
			container_control& paragraph(int id = id_counter::next());
			container_control& code(int id = id_counter::next());
			container_control& label(int id = id_counter::next());
			container_control& label(std::string _text, int id = id_counter::next());

			container_control& image(int id = id_counter::next());

			container_control& push_button(int id);
			container_control& radio_button(int id);
			container_control& checkbox(int id);
			container_control& push_button(std::string _text, int id);
			container_control& radio_button(std::string _text, int id);
			container_control& checkbox(std::string _text, int id);
			container_control& listbox(int id);
			container_control& combobox(int id);
			container_control& edit(int id);
			container_control& scrollbar(int id);

			container_control& listview(int id);
			container_control& treeview(int id);
			container_control& header(int id);
			container_control& toolbar(int id);
			container_control& statusbar(int id);
			container_control& hotkey(int id);
			container_control& animate(int id);
			container_control& richedit(int id);
			container_control& draglistbox(int id);
			container_control& rebar(int id);
			container_control& comboboxex(int id);
			container_control& datetimepicker(int id);
			container_control& monthcalendar(int id);


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

		class title_control : public text_display_control
		{
			void set_default_styles();
		public:
			title_control();
			title_control(container_control* _parent, int _id);
		};

		class subtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			subtitle_control();
			subtitle_control(container_control* _parent, int _id);
		};

		class chaptertitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptertitle_control();
			chaptertitle_control(container_control* _parent, int _id);
		};

		class chaptersubtitle_control : public text_display_control
		{
			void set_default_styles();
		public:
			chaptersubtitle_control();
			chaptersubtitle_control(container_control* _parent, int _id);
		};

		class paragraph_control : public text_display_control
		{
			void set_default_styles();
		public:
			paragraph_control();
			paragraph_control(container_control* _parent, int _id);
		};

		class code_control : public text_display_control
		{
			void set_default_styles();
		public:
			code_control();
			code_control(container_control* _parent, int _id);
		};

		class label_control: public text_display_control
		{
			void set_default_styles();
		public:
			label_control();
			label_control(container_control* _parent, int _id);
		};

		class image_control : 
			public draw_control
		{
		public:
			image_control();
			image_control(container_control* _parent, int _id) : draw_control(_parent, _id) { ; }
		};

		class absolute_layout : 
			public container_control
		{
		public:
			absolute_layout() { ; }
			absolute_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~absolute_layout() { ; }

			virtual void arrange(rectangle _ctx, int zorder);
		};

		class column_layout : 
			public container_control
		{
			layout_rect item_size;
		public:
			column_layout() { ; }
			column_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
			virtual ~column_layout() { ; }

			virtual void arrange(rectangle _ctx, int zorder);
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

			virtual void arrange(rectangle _ctx, int zorder);
			virtual point get_remaining(point _ctx);
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

			virtual void arrange(rectangle _ctx, int zorder);
			virtual point get_remaining(point _ctx);
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class windows_control : public control_base
		{

			void set_default_styles()
			{
				text_style = {};
				text_style.name = "windows_control_style";
				text_style.fontName = styles.get_style().PrimaryFont;
				text_style.fontSize = 20;
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

			virtual void create(std::weak_ptr<win32::directApplicationWin32> _host)
			{
				window_host = _host;

				if (auto phost = window_host.lock()) {
					HWND parent = phost->getMainWindow();
					HFONT font = phost->createFont(text_style.fontName, text_style.fontSize, text_style.bold, text_style.italics);
					text_font.Attach(font);

					auto boundsPixels = phost->toPixelsFromDips(bounds);

					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + boundsPixels.w;
					r.bottom = boundsPixels.y + boundsPixels.h;

					if (((HWND)window) == nullptr) {
						window.Create(parent, r, NULL, dwStyle, dwExStyle, id, NULL);
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
					auto boundsPixels = phost->toPixelsFromDips(control_base::bounds);

					RECT r;
					r.left = boundsPixels.x;
					r.top = boundsPixels.y;
					r.right = boundsPixels.x + boundsPixels.w;
					r.bottom = boundsPixels.y + windows_control<WtlWindowClass, dwStyle, dwExStyle>::text_style.fontSize * 8;
					windows_control<WtlWindowClass, dwStyle, dwExStyle>::window.MoveWindow(&r);
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
		const int EditWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD;
		const int RichEditWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
		const int ComboWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | CBS_SORT;
		const int ComboExWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | CBS_SORT;
		const int PushButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT;
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

		class richedit_control : public text_control_base<WTL::CRichEditCtrl, EditWindowStyles>
		{
		public:
			void set_html(const std::string& _text);
			std::string get_html();

			richedit_control(container_control* _parent, int _id) : text_control_base<WTL::CRichEditCtrl, EditWindowStyles>(_parent, _id) {
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

			virtual void create(std::weak_ptr<corona::win32::directApplicationWin32> _host);
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
						auto str = std::format("Object is not {0} ", typeid(*citem).name());
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
