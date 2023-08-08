#pragma once

namespace corona
{
	namespace database
	{
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

		class control_base
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

		public:

			int						id;
			std::string				control_type;
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

			virtual void create(win32::win32ControllerHost* _host);
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

			row_layout &row_layout_new(int id);
			column_layout &column_layout_new(int id);
			absolute_layout &absolute_layout_new(int id);

			static_control &static_control_new(int id);
			button_control &button_control_new(int id);
			listbox_control &listbox_control_new(int id);
			combobox_control &combobox_control_new(int id);
			edit_control &edit_control_new(int id);
			scrollbar_control &scrollbar_control_new(int id);

			listview_control &listview_control_new(int id);
			treeview_control &treeview_control_new(int id);
			header_control &header_control_new(int id);
			toolbar_control &toolbar_control_new(int id);
			statusbar_control &statusbar_control_new(int id);
			hotkey_control &hotkey_control_new(int id);
			animate_control &animate_control_new(int id);
			richedit_control &richedit_control_new(int id);
			draglistbox_control &draglistbox_control_new(int id);
			rebar_control &rebar_control_new(int id);
			comboboxex_control &comboboxex_control_new(int id);
			datetimepicker_control &datetimepicker_control_new(int id);
			monthcalendar_control &monthcalendar_control_new(int id);

		};

		class container_control : public control_base
		{
		public:
			win32::win32ControllerHost* host;
			win32::direct2dChildWindow* window;
			std::function<void(container_control*)> on_draw;

			virtual void create(win32::win32ControllerHost* _host);
			virtual void destroy();
			virtual void draw();

		};

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

			WtlWindowClass window;

			virtual void create(win32::win32ControllerHost* _host)
			{
				HWND parent = _host->getMainWindow();

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
				window.DestroyWindow();
			}

			virtual ~windows_control()
			{
				destroy();
			}
		};

		class static_control : public windows_control<WTL::CStatic, WS_VISIBLE>
		{
		public:

		};

		class button_control : public windows_control<WTL::CButton, 0>
		{
		public:

		};

		class listbox_control : public windows_control<WTL::CListBox, 0>
		{
		public:
		};

		class combobox_control : public windows_control<WTL::CComboBox, 0>
		{
		public:
			
		};

		class edit_control : public windows_control<WTL::CEdit, 0>
		{
		public:
		};

		class scrollbar_control : public windows_control<WTL::CScrollBar, 0>
		{
		public:
		};

		class listview_control : public windows_control<WTL::CListViewCtrl, 0>
		{
		public:
		};

		class treeview_control : public windows_control<WTL::CTreeViewCtrl, 0>
		{
		public:
		};

		class header_control : public windows_control<WTL::CHeaderCtrl, 0>
		{
		public:
		};

		class toolbar_control : public windows_control<WTL::CToolBarCtrl, 0>
		{
		public:
		};

		class statusbar_control : public windows_control<WTL::CStatusBarCtrl, 0>
		{
		public:
		};

		class hotkey_control : public windows_control<WTL::CHotKeyCtrl, 0>
		{
		public:
		};

		class animate_control : public windows_control<WTL::CAnimateCtrl, 0>
		{
		public:
		};

		class richedit_control : public windows_control<WTL::CRichEditCtrl, 0>
		{
		public:
		};

		class draglistbox_control : public windows_control<WTL::CDragListBox, 0>
		{
		public:
		};

		class rebar_control : public windows_control<WTL::CReBarCtrl, 0>
		{
		public:
		};

		class comboboxex_control : public windows_control<WTL::CComboBoxEx, 0>
		{
		public:
		};

		class datetimepicker_control : public windows_control<CDateTimePickerCtrl, 0>
		{
		public:
		};

		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, 0>
		{
		public:
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
			int select_index;
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

		public:

			std::shared_ptr<control_base> root;
			std::string name;

			page(const char *_name = nullptr);
			virtual ~page();

			void clear();

			void on_key_up( int _control_id, std::function< void(key_up_event) >);
			void on_key_down( int _control_id, std::function< void(key_down_event) >);
			void on_mouse_move( int _control_id, std::function< void(mouse_move_event) >);
			void on_mouse_click( int _control_id, std::function< void(mouse_click_event) >);
			void on_item_changed(int _control_id, std::function< void(item_changed_event) >);
			void on_list_changed(int _control_id, std::function< void(list_changed_event) >);
			void on_update(update_function fnc);

			void handle_key_up(int _control_id, key_up_event evt);
			void handle_key_down(int _control_id, key_down_event evt);
			void handle_mouse_move(int _control_id, mouse_move_event evt);
			void handle_mouse_click(int _control_id, mouse_click_event evt);
			void handle_item_changed(int _control_id, item_changed_event evt);
			void handle_list_changed(int _control_id, list_changed_event evt);

			void arrange(double _width, double _height, double _padding = 0.0);

			virtual void create(win32::win32ControllerHost *_host);
			virtual void destroy();
			virtual void draw();
			virtual void update(double _elapsedSeconds, double _totalSeconds);
			virtual void item_changed(int _control_id, std::string _text_value);
			virtual void list_changed(int _control_id, std::string _text_value, int _index, relative_ptr_type _value);

			control_base* get_root();

			const control_base *operator[](int _id)
			{
				return get_root()->find(_id);
			}
		};


		class presentation : public win32::controller
		{

			std::weak_ptr<page> current_page;

		public:

			std::map<std::string, std::shared_ptr<page>> pages;

			virtual void select_page(const std::string& _page_name);

			virtual bool drawFrame();
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void keyDown(win32::direct2dWindow* win, short _key);
			virtual void keyUp(win32::direct2dWindow* win, short _key);
			virtual void mouseMove(win32::direct2dWindow* win, point* _point);
			virtual void mouseClick(win32::direct2dWindow* win, point* _point);
			virtual void pointSelected(win32::direct2dWindow* win, point* _point, color* _color);

			virtual void onCreated();
			virtual void onCommand(int buttonId);
			virtual void onTextChanged(int textControlId);
			virtual void onDropDownChanged(int dropDownId);
			virtual void onListViewChanged(int listViewId);
			virtual int onHScroll(int controlId, win32::scrollTypes scrollType);
			virtual int onVScroll(int controlId, win32::scrollTypes scrollType);
			virtual int onResize(const rectangle& newSize, double d2dScale);
			virtual int onSpin(int controlId, int newPosition);

		};
	}
}
