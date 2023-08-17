#pragma once

namespace corona
{
	namespace database
	{

		class id_counter
		{
		public:
			static int id;
			static int next() { return id++; }
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

		class control_base;

		class row_layout;
		class column_layout;
		class absolute_layout;

		class title_control;
		class subtitle_control;
		class chapter_control;
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

			virtual void size_constant(layout_context _ctx);
			virtual void size_constants(layout_context _ctx);

			virtual void size_aspect(layout_context _ctx);
			virtual void size_aspect_widths(layout_context _ctx, int safety);
			virtual void size_aspect_heights(layout_context _ctx, int safety);
			virtual void size_aspects(layout_context _ctx);
			virtual void size_children(layout_context _ctx);

			virtual void size_remaining(layout_context _ctx);
			virtual void size_remainings(layout_context _ctx);
			virtual layout_context get_remaining(layout_context _ctx);

			virtual void size_items(layout_context _ctx);
			virtual void positions(layout_context _ctx);

			control_base* align_base(visual_alignment _new_alignment);
			control_base* position_base(layout_rect _new_layout);
			control_base* spacing_base(measure _spacing);

		public:

			int						id;

			visual_alignment		alignment;
			layout_rect				box;
			measure					item_space;

			rectangle				bounds;
			point					item_space_amount;

			win32::directApplicationWin32* app;
			std::weak_ptr<control_base> parent;

			std::vector<std::shared_ptr<control_base>> children;

			control_base() :
				id(-1),
				item_space(),
				item_space_amount({ 0.0, 0.0 } )
			{
			}

			control_base(const std::shared_ptr<control_base>& _parent, int _id) :
				parent(_parent),
				id(_id),
				item_space(),
				item_space_amount({ 0.0, 0.0 })
			{
			}

			virtual void size_item(layout_context _ctx);
			virtual rectangle layout(layout_context _ctx);
			virtual void position(layout_context _ctx);
			virtual bool contains(point pt);

			const control_base* find(int _id) const;
			static std::weak_ptr<control_base> get(std::shared_ptr<control_base>& _root, int _id);
			void foreach(std::function<void (control_base *_root)> _item);

			virtual void create(std::weak_ptr<win32::win32ControllerHost> _host);
			virtual void destroy();
			virtual void draw();

			template <typename control_type> control_type& create( int _id )
			{
				std::shared_ptr<control_type> temp = std::make_shared<control_type>();
				temp->id = id;
				int idx = children.size();
				children.push_back(temp);
				return *temp.get();
			}
			
			row_layout &row_begin(int id = id_counter::next());
			column_layout &column_begin(int id = id_counter::next());
			absolute_layout &absolute_begin(int id = id_counter::next());
			control_base& end();

			title_control& title(int id = id_counter::next());
			subtitle_control& subtitle(int id = id_counter::next());
			chapter_control& chapter(int id = id_counter::next());
			paragraph_control& paragraph(int id = id_counter::next());
			code_control& code(int id = id_counter::next());
			image_control& image(int id = id_counter::next());

			static_control& label(int id = id_counter::next());
			button_control& button(int id);
			listbox_control& listbox(int id);
			combobox_control& combobox(int id);
			edit_control& edit(int id);
			scrollbar_control& scrollbar(int id);

			listview_control &listview(int id);
			treeview_control &treeview(int id);
			header_control &header(int id);
			toolbar_control &toolbar(int id);
			statusbar_control &statusbar(int id);
			hotkey_control &hotkey(int id);
			animate_control &animate(int id);
			richedit_control &richedit(int id);
			draglistbox_control &draglistbox(int id);
			rebar_control &rebar(int id);
			comboboxex_control &comboboxex(int id);
			datetimepicker_control &datetimepicker(int id);
			monthcalendar_control &monthcalendar(int id);

		};

		class draw_control : public control_base
		{
		protected:
			std::weak_ptr<win32::win32ControllerHost> host;
			std::weak_ptr<win32::direct2dChildWindow> window;
			std::function<void(draw_control*)> on_draw;
			std::function<void(draw_control*)> on_create;

			virtual void create(std::weak_ptr<win32::win32ControllerHost> _host);
			virtual void destroy();
			virtual void draw();
		};

		class container_control : public draw_control
		{
		public:
		};

		class text_display_control : public draw_control
		{
		protected:
			textStyleRequest	text_style;
			std::string			text_color;
			std::string			style_name;

		public:
			std::string text;

			text_display_control();
		};

		class title_control : public text_display_control
		{
		public:
			title_control();
		};

		class subtitle_control : public text_display_control
		{
		public:
			subtitle_control();
		};

		class chapter_control : public text_display_control
		{
		public:
			chapter_control();
		};

		class paragraph_control : public text_display_control
		{
		public:
			paragraph_control();
		};

		class code_control : public text_display_control
		{
		public:
			code_control();
		};

		chapter_control& chapter(int id);
		paragraph_control& paragraph(int id);
		code_control& code(int id);

		class absolute_layout : public container_control
		{
		protected:


		public:
		};

		class column_layout : public container_control
		{
		protected:
			virtual layout_context get_remaining(layout_context _ctx);
			virtual void size_children(layout_context _ctx);
			virtual void positions(layout_context _ctx);
		public:
		};

		class row_layout : public container_control
		{
		protected:
			virtual layout_context get_remaining(layout_context _ctx);
			virtual void size_children(layout_context _ctx);
			virtual void positions(layout_context _ctx);
		public:
		};

		template <typename WtlWindowClass, DWORD dwStyle, DWORD dwExStyle = 0> class windows_control : public control_base
		{
		public:

			using control_base::id;

			WtlWindowClass window;
			win32::win32ControllerHost* window_host;

			virtual void create(win32::win32ControllerHost* _host)
			{
				window_host = _host;

				HWND parent = window_host->getMainWindow();

				RECT r;
				r.left = bounds.x;
				r.top = bounds.y;
				r.right = bounds.x + bounds.w;
				r.bottom = bounds.y + bounds.h;

				if (((HWND)window) == nullptr) {
					window.Create(parent, r, NULL, dwStyle, dwExStyle, id, NULL);
				}
			}

			virtual void destroy()
			{
				window_host = nullptr;
				window.DestroyWindow();
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

		class static_control : public text_control_base<WTL::CStatic, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline static_control& align(visual_alignment _new_alignment)	{ align_base(_new_alignment);  return *this; }
			inline static_control& position(layout_rect _new_layout) {	position_base(_new_layout); return *this; }
			inline static_control& spacing(measure _spacing) { 	spacing_base(_spacing); return *this; }
		};

		class button_control : public text_control_base<WTL::CButton, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline button_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline button_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline button_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class edit_control : public text_control_base<WTL::CEdit, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline edit_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline edit_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline edit_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class listbox_control : public list_control_base<WTL::CListBox, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline listbox_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline listbox_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline listbox_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class combobox_control : public list_control_base<WTL::CComboBox, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline combobox_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline combobox_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline combobox_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class comboboxex_control : public list_control_base<WTL::CComboBoxEx, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline comboboxex_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline comboboxex_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline comboboxex_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class listview_control : public table_control_base<WTL::CListViewCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline listview_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline listview_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline listview_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class scrollbar_control : public windows_control<WTL::CScrollBar, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			inline scrollbar_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline scrollbar_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline scrollbar_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class richedit_control : public text_control_base<WTL::CRichEditCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			void set_html(const std::string& _text);
			std::string get_html();

			inline richedit_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline richedit_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline richedit_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class datetimepicker_control : public windows_control<CDateTimePickerCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			void set_text(const std::string& _text);
			std::string get_text();

			inline datetimepicker_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline datetimepicker_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline datetimepicker_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline monthcalendar_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline monthcalendar_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline monthcalendar_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class animate_control : public windows_control<WTL::CAnimateCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:
			bool open(const std::string& _name);
			bool open(DWORD resource_id);
			bool play(UINT from, UINT to, UINT rep);
			bool play();
			bool stop();

			inline animate_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline animate_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline animate_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class treeview_control : public windows_control<WTL::CTreeViewCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline treeview_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline treeview_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline treeview_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class header_control : public windows_control<WTL::CHeaderCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline header_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline header_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline header_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }

		};

		class toolbar_control : public windows_control<WTL::CToolBarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline toolbar_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline toolbar_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline toolbar_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};

		class statusbar_control : public windows_control<WTL::CStatusBarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline statusbar_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline statusbar_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline statusbar_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }

		};

		class hotkey_control : public windows_control<WTL::CHotKeyCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline hotkey_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline hotkey_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline hotkey_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }
		};


		class draglistbox_control : public windows_control<WTL::CDragListBox, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline draglistbox_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline draglistbox_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline draglistbox_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }

		};

		class rebar_control : public windows_control<WTL::CReBarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP>
		{
		public:

			inline rebar_control& align(visual_alignment _new_alignment) { align_base(_new_alignment);  return *this; }
			inline rebar_control& position(layout_rect _new_layout) { position_base(_new_layout); return *this; }
			inline rebar_control& spacing(measure _spacing) { spacing_base(_spacing); return *this; }

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

			page(const char *_name = nullptr);
			virtual ~page();

			void clear();

			void on_key_up( int _control_id, std::function< void(key_up_event) > );
			void on_key_down( int _control_id, std::function< void(key_down_event) > );
			void on_mouse_move( int _control_id, std::function< void(mouse_move_event) > );
			void on_mouse_click( int _control_id, std::function< void(mouse_click_event) > );
			void on_item_changed( int _control_id, std::function< void(item_changed_event) > );
			void on_list_changed( int _control_id, std::function< void(list_changed_event) > );
			void on_update(update_function fnc);

			row_layout& row_begin(int id);
			column_layout& column_begin(int id);
			absolute_layout& absolute_begin(int id);
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
