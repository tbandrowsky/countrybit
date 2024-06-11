
#ifndef CORONA_PRESENTATION_CONTROLS_WIN32_H
#define CORONA_PRESENTATION_CONTROLS_WIN32_H

namespace corona
{

	class windows_control : public control_base
	{
	protected:

		void set_default_styles()
		{
			text_style = {};
			text_style.name = "windows_control_style";
			text_style.fontName = styles.get_style()->PrimaryFont;
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_near;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
			text_style.font_stretch = DWRITE_FONT_STRETCH_NORMAL;
		}

		HFONT text_font;
		HWND window;

	public:


		using control_base::id;

		std::weak_ptr<applicationBase> window_host;
		textStyleRequest	text_style;

		bool is_default_focus;
		bool is_default_button;

		windows_control() :
			window(nullptr),
			text_font(nullptr)
		{
			set_origin(0.0_px, 0.0_px);
			set_size(1.0_container, 1.5_fontgr);
			set_default_styles();
			is_default_focus = false;
			is_default_button = false;
		}

		windows_control(const windows_control& _src) : control_base(_src),
			window(nullptr),
			text_font(nullptr)
		{
			text_style = _src.text_style;
			is_default_focus = false;
			is_default_button = false;
		}

		windows_control(container_control_base *_parent, int _id)
			: control_base(_parent, _id),
			window(nullptr),
			text_font(nullptr)
		{
			set_origin(0.0_px, 0.0_px);
			set_size(1.0_container, 1.5_fontgr);
			set_default_styles();
			is_default_focus = false;
			is_default_button = false;
		}

		virtual bool gets_real_focus() { return true; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<windows_control>(*this);
			return tv;
		}

		virtual const char* get_window_class()
		{
			return nullptr;
		}

		virtual const wchar_t* get_window_class_w() 
		{ 
			return nullptr; 
		}

		virtual DWORD get_window_style()
		{
			return 0;
		}

		virtual DWORD get_window_ex_style()
		{
			return 0;
		}

		virtual double get_font_size() { return text_style.fontSize; }

		virtual bool kill_focus()
		{
			is_focused = false;
			return true;
		}

		virtual bool set_focus()
		{
			is_focused = true;
			return true;
		}

		virtual LRESULT send_message(UINT msg, WPARAM  wParam, LPARAM  lParam)
		{
			return SendMessageA(window, msg, wParam, lParam);
		}

		void enable()
		{
			::EnableWindow(window, true);
		}

		void disable()
		{
			::EnableWindow(window, false);
		}

		virtual void on_resize()
		{
			if (auto phost = window_host.lock()) {
				HWND parent = phost->getMainWindow();

				auto boundsPixels = phost->toPixelsFromDips(inner_bounds);

				if (window != nullptr) {
					MoveWindow(window, boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, TRUE);
				}
			}
		}

		virtual void create(std::weak_ptr<applicationBase> _host)
		{
			window_host = _host;

			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(inner_bounds);

				if (window == nullptr) {
					HWND parent = phost->getMainWindow();
					if (auto wclassname = get_window_class_w())
						window = CreateWindowExW(get_window_ex_style(), get_window_class_w(), L"", get_window_style(), boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, parent, (HMENU)id, NULL, NULL);
					else
						window = CreateWindowEx(get_window_ex_style(), get_window_class(), "", get_window_style(), boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, parent, (HMENU)id, NULL, NULL);

					HFONT old_font = text_font;

					text_font = phost->createFontDips(window, text_style.fontName, text_style.fontSize, text_style.bold, text_style.italics);
					SendMessage(window, WM_SETFONT, (WPARAM)text_font, 0);

					if (old_font) {
						::DeleteObject(old_font);
						old_font = nullptr;
					}

					HWND tooltip = phost->getTooltipWindow();
					if (tooltip && tooltip_text.size() > 0) {
						TOOLINFOA toolInfo = { 0 };
						toolInfo.cbSize = sizeof(toolInfo);
						toolInfo.hwnd = parent;
						toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
						toolInfo.uId = (UINT_PTR)window;
						toolInfo.lpszText = (LPSTR)tooltip_text.c_str();
						SendMessageA(tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
					}
					on_create();
				}
			}
		}

		virtual void on_create() { ; }

		virtual void destroy()
		{
			if (::IsWindow(window)) {
				DestroyWindow(window);
				window = nullptr;
			}
		}

		virtual ~windows_control()
		{
			destroy();
		}
	};


	class text_control_base : public windows_control
	{
		std::string text;

	public:

		using control_base::id;
		using windows_control::window_host;

		text_control_base()
		{
			;
		}

		text_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
			;
		}

		text_control_base(const text_control_base& _src) : windows_control(_src)
		{
			text = _src.text;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<text_control_base>(*this);
			return tv;
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

		virtual void create(std::weak_ptr<applicationBase> _host)
		{
			windows_control::create(_host);
			if (auto phost = window_host.lock()) {
				phost->setEditText(id, text);
			}
		}

		virtual json get_data()
		{
			json result;
			if (json_field_name.size() > 0) {
				json_parser jp;
				result = jp.create_object();
				std::string text = get_text();
				result.put_member(json_field_name, text);
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				std::string text = _data[json_field_name];
				set_text(text);
			}
			return _data;
		}

	};


	class table_control_base : public windows_control
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
					char* t = mtable.set(col_index, row_index, col.display_name);
					phost->addListViewColumn(id, col_index, t, col.width, col.alignment);
					column_map[col.json_field] = col_index;
					col_index++;
				}
				std::vector<char*> data_row;
				data_row.resize(choices.columns.size());
				row_index++;

				for (int i = 0; i < choices.items.size(); i++)
				{
					auto item = choices.items.get_element(i);
					col_index = 0;
					for (auto col : choices.columns)
					{
						data_row[col_index] = blank;
						bool has_field = item.has_member(col.json_field);
						if (has_field) {
							std::string item_value = item[col.json_field];
							char* value = mtable.set(col_index, row_index, item_value);
							if (value) {
								data_row[col_index] = value;
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
		using windows_control::window_host;
		table_data choices;

		table_control_base()
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 10.0_fontgr);
		}

		table_control_base(const table_control_base& _src) : windows_control(_src)
		{
			mtable = _src.mtable;
			choices = _src.choices;
		}

		table_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 10.0_fontgr);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<table_control_base>(*this);
			return tv;
		}


		virtual ~table_control_base() { ; }

		virtual void on_create()
		{
			ListView_SetExtendedListViewStyle(window, LVS_EX_FULLROWSELECT);
			data_changed();
		}

		void set_table(table_data& _choices)
		{
			choices = _choices;
			data_changed();
		}
	};

	class list_control_base : public windows_control
	{
	public:
		using control_base::id;
		using windows_control::window_host;
		list_data choices;

		list_control_base()
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 200.0_px);
		}

		list_control_base(const list_control_base& _src) : windows_control(_src)
		{
			choices = _src.choices;
		}

		list_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 200.0_px);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<list_control_base>(*this);
			return tv;
		}


		virtual ~list_control_base() { ; }

		void data_changed()
		{
			if (auto phost = window_host.lock()) {
				phost->clearListItems(id);
				if (choices.items.is_array()) {
					for (int i = 0; i < choices.items.size(); i++)
					{
						auto c = choices.items.get_element(i);
						int lid = c[choices.id_field];
						std::string description = c[choices.text_field];
						phost->addListItem(id, description, lid);
					}
				}
			}
		}

		virtual json get_data()
		{
			json result;
			if (json_field_name.size() > 0) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					std::string new_text = ptr->getListSelectedText(id);
					int index = ptr->getListSelectedIndex(id);
					int value = ptr->getListSelectedValue(id);
					result.put_member(json_field_name, new_text);
				}
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				std::string text = _data[json_field_name];
				if (auto ptr = window_host.lock()) {
					ptr->setListSelectedText(id, text.c_str());
				}
			}
			return _data;
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

	class dropdown_control_base : public windows_control
	{
	public:

		using control_base::id;
		using windows_control::window_host;
		list_data choices;

		dropdown_control_base()
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		dropdown_control_base(const dropdown_control_base& _src) : windows_control(_src)
		{
			choices = _src.choices;
		}

		dropdown_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<dropdown_control_base>(*this);
			return tv;
		}

		virtual ~dropdown_control_base() { ; }

		void data_changed()
		{
			if (auto phost = window_host.lock()) {
				std::string selectedText = phost->getComboSelectedText(id);
				phost->clearComboItems(id);
				if (choices.items.is_array()) 
				{
					for (int i = 0; i < choices.items.size(); i++)
					{
						auto element = choices.items.get_element(i);
						int lid = element[choices.id_field];
						std::string description = element[choices.text_field];
						phost->addComboItem(id, description, lid);
					}
				}
				phost->setComboSelectedText(id, selectedText);
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

				if (windows_control::window != nullptr) {
					int h = windows_control::text_style.fontSize * 8;
					::MoveWindow(windows_control::window, boundsPixels.x, boundsPixels.y, boundsPixels.w, h, TRUE);
				}
			}
		}

		virtual void on_create()
		{
			on_resize();
			data_changed();
		}

		virtual json get_data()
		{
			json result;
			if (json_field_name.size() > 0) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					std::string new_text = ptr->getComboSelectedText(id);
					int index = ptr->getComboSelectedIndex(id);
					int value = ptr->getComboSelectedValue(id);
					result.put_member(json_field_name, new_text);
				}
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				std::string text = _data[json_field_name];
				if (auto ptr = window_host.lock()) {
					ptr->setComboSelectedText(id, text.c_str());
				}
			}
			return _data;
		}

	};

	const int DefaultWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	const int DisplayOnlyWindowStyles = WS_VISIBLE | WS_CHILD;
	const int EditWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	const int RichEditWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
	const int ComboWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
	const int ComboExWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
	const int PushButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT | BS_NOTIFY;
	const int PressButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT | BS_AUTOCHECKBOX | BS_PUSHLIKE | BS_NOTIFY;
	const int CheckboxWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | BS_FLAT | BS_NOTIFY;
	const int RadioButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_FLAT | BS_NOTIFY;
	const int LinkButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_COMMANDLINK | BS_FLAT | BS_NOTIFY;
	const int ListViewWindowsStyles = DefaultWindowStyles | LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL;
	const int ListBoxWindowsStyles = DefaultWindowStyles | WS_BORDER | WS_VSCROLL | LBS_NOTIFY;

	class static_control : public text_control_base
	{
	public:
		static_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) { ; }
		virtual ~static_control() { ; }


		static_control(const static_control& _src) : text_control_base(_src)
		{
			
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<static_control>(*this);
			return tv;
		}


		virtual const char* get_window_class() { return WC_STATIC; }
		virtual DWORD get_window_style() { return DisplayOnlyWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }
	};

	template <long ButtonWindowStyles> class button_control : public text_control_base
	{
		using control_base::id;
		using windows_control::window_host;
		std::string caption_text;
		long caption_icon_id;
		HICON caption_icon;
	public:
		button_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) { ; }
		button_control(const button_control& _src) : text_control_base(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<button_control<ButtonWindowStyles>>(*this);
			return tv;
		}

		virtual ~button_control() { ; }
		virtual const char* get_window_class() { return WC_BUTTON; }
		virtual DWORD get_window_style() { return ButtonWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }
	};

	class pushbutton_control : public button_control<PushButtonWindowStyles>
	{
	public:

		using windows_control::is_default_button;

		pushbutton_control(container_control_base* _parent, int _id) : button_control<PushButtonWindowStyles>(_parent, _id) { ; }
		pushbutton_control(const pushbutton_control& _src) : button_control<PushButtonWindowStyles>(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<pushbutton_control>(*this);
			return tv;
		}

		virtual DWORD get_window_style() { return PushButtonWindowStyles | (is_default_button ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON); }

		virtual ~pushbutton_control() { ; }
	};

	class pressbutton_control : public button_control<PressButtonWindowStyles>
	{
	public:
		pressbutton_control(container_control_base* _parent, int _id) : button_control<PressButtonWindowStyles>(_parent, _id) { ; }
		pressbutton_control(const pressbutton_control& _src) : button_control<PressButtonWindowStyles>(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<pressbutton_control>(*this);
			return tv;
		}

		virtual ~pressbutton_control() { ; }
	};

	class radiobutton_control : public button_control<RadioButtonWindowStyles>
	{
	public:
		radiobutton_control(container_control_base* _parent, int _id) : button_control<RadioButtonWindowStyles>(_parent, _id) { ; }
		radiobutton_control(const radiobutton_control& _src) : button_control<RadioButtonWindowStyles>(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<radiobutton_control>(*this);
			return tv;
		}

		virtual ~radiobutton_control() { ; }
	};

	class checkbox_control : public button_control<CheckboxWindowStyles>
	{
	public:
		checkbox_control(container_control_base* _parent, int _id) : button_control<CheckboxWindowStyles>(_parent, _id) { ; }
		checkbox_control(const checkbox_control& _src) : button_control<CheckboxWindowStyles>(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<checkbox_control>(*this);
			return tv;
		}

		virtual ~checkbox_control() { ; }
	};

	class linkbutton_control : public button_control<LinkButtonWindowStyles>
	{
	public:
		linkbutton_control(container_control_base* _parent, int _id) : button_control<LinkButtonWindowStyles>(_parent, _id) { ; }
		linkbutton_control(const linkbutton_control& _src) : button_control<LinkButtonWindowStyles>(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<linkbutton_control>(*this);
			return tv;
		}

		virtual ~linkbutton_control() { ; }
	};

	class edit_control : public text_control_base
	{
	public:
		edit_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) { ; }
		virtual ~edit_control() { ; }
		edit_control(const edit_control& _src) : text_control_base(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<edit_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_EDIT; }
		virtual DWORD get_window_style() { return EditWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class listbox_control : public list_control_base
	{
	public:
		listbox_control(container_control_base* _parent, int _id) : list_control_base(_parent, _id) { ; }
		virtual ~listbox_control() { ; }

		listbox_control(const listbox_control& _src) : list_control_base(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<listbox_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_LISTBOX; }
		virtual DWORD get_window_style() { return ListBoxWindowsStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class combobox_control : public dropdown_control_base
	{
	public:
		combobox_control(container_control_base* _parent, int _id) : dropdown_control_base(_parent, _id) { ; }
		virtual ~combobox_control() { ; }

		combobox_control(const combobox_control& _src) : dropdown_control_base(_src)
		{

		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<combobox_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_COMBOBOX; }
		virtual DWORD get_window_style() { return ComboWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class comboboxex_control : public windows_control
	{
	public:
		using control_base::id;
		using windows_control::window_host;
		list_data choices;

		comboboxex_control();
		comboboxex_control(container_control_base* _parent, int _id);

		comboboxex_control(const comboboxex_control& _src) : windows_control(_src)
		{
			choices = _src.choices;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<comboboxex_control>(*this);
			return tv;
		}

		virtual ~comboboxex_control() { ; }
		void data_changed();
		void set_list(list_data& _choices);
		virtual void on_create();
		virtual void on_resize();

		virtual const char* get_window_class() { return WC_COMBOBOXEX; }
		virtual DWORD get_window_style() { return ComboWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class listview_control : public table_control_base
	{
	public:
		listview_control(container_control_base* _parent, int _id) : table_control_base(_parent, _id) { ; }
		virtual ~listview_control() { ; }

		listview_control(const listview_control& _src) : table_control_base(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<listview_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_LISTVIEW; }
		virtual DWORD get_window_style() { return ListViewWindowsStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class scrollbar_control : public windows_control
	{
	public:
		scrollbar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~scrollbar_control() { ; }

		scrollbar_control(const scrollbar_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<scrollbar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_SCROLLBAR; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class richedit_control : public text_control_base
	{
	public:
		void set_html(const std::string& _text);
		std::string get_html();

		richedit_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) {
			LoadLibrary(TEXT("Msftedit.dll"));
		}

		richedit_control(const richedit_control& _src) : text_control_base(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<richedit_control>(*this);
			return tv;
		}

		virtual ~richedit_control() { ; }


		virtual const char* get_window_class() { return nullptr; }
		virtual const wchar_t* get_window_class_w() { return MSFTEDIT_CLASS; }
		virtual DWORD get_window_style() { return RichEditWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class datetimepicker_control : public windows_control
	{
	public:
		void set_text(const std::string& _text);
		std::string get_text();

		datetimepicker_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }

		datetimepicker_control(const datetimepicker_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<datetimepicker_control>(*this);
			return tv;
		}

		virtual ~datetimepicker_control() { ; }

		virtual const char* get_window_class() { return DATETIMEPICK_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class monthcalendar_control : public windows_control
	{
	public:
		monthcalendar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~monthcalendar_control() { ; }

		monthcalendar_control(const monthcalendar_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<monthcalendar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return MONTHCAL_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class animate_control : public windows_control
	{
	public:
		animate_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~animate_control() { ; }


		animate_control(const animate_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<animate_control>(*this);
			return tv;
		}

		bool open(const std::string& _name);
		bool open(DWORD resource_id);
		bool play(UINT from, UINT to, UINT rep);
		bool play();
		bool stop();

		virtual const char* get_window_class() { return ANIMATE_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }
	};

	class treeview_control : public windows_control
	{
	public:
		treeview_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~treeview_control() { ; }

		treeview_control(const treeview_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<treeview_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_TREEVIEW; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class header_control : public windows_control
	{
	public:
		header_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~header_control() { ; }

		header_control(const treeview_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<header_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_HEADER; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class toolbar_control : public windows_control
	{
	public:
		toolbar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~toolbar_control() { ; }

		toolbar_control(const toolbar_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<toolbar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return TOOLBARCLASSNAME; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class statusbar_control : public windows_control
	{
	public:
		statusbar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~statusbar_control() { ; }

		statusbar_control(const statusbar_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<statusbar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return STATUSCLASSNAME; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

	};

	class hotkey_control : public windows_control
	{
	public:
		hotkey_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		hotkey_control(const hotkey_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<hotkey_control>(*this);
			return tv;
		}

		virtual ~hotkey_control() { ; }

		virtual const char* get_window_class() { return TOOLBARCLASSNAME; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }
	};

	class draglistbox_control : public windows_control
	{
	public:
		draglistbox_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		draglistbox_control(const draglistbox_control& _src) : windows_control(_src)
		{
			;
		}
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<draglistbox_control>(*this);
			return tv;
		}

		virtual ~draglistbox_control() { ; }

		virtual const char* get_window_class() { return HOTKEY_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }
	};

	comboboxex_control::comboboxex_control()
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 2.0_fontgr);
	}

	comboboxex_control::comboboxex_control(container_control_base* _parent, int _id) : windows_control(_parent, _id)
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 2.0_fontgr);
	}

	void comboboxex_control::data_changed()
	{
		if (IsWindow(window)) {
			if (auto phost = window_host.lock()) {
				phost->clearComboItems(id);
				for (int i = 0; i < choices.items.size(); i++)
				{
					auto c = choices.items.get_element(i);
					if (c.has_member(choices.id_field) && c.has_member(choices.text_field)) {
						int lid = c[choices.id_field];
						std::string description = c[choices.text_field];

						COMBOBOXEXITEMA cbex = {};
						cbex.mask = CBEIF_TEXT | CBEIF_LPARAM;
						cbex.iItem = -1;
						cbex.pszText = (LPSTR)description.c_str();
						cbex.iImage = 0;
						cbex.iSelectedImage = 0;
						cbex.iIndent = 0;
						cbex.lParam = lid;
						SendMessage(windows_control::window, CBEM_INSERTITEM, 0, (LPARAM)&cbex);
					}
				}
			}
		}
	}

	void comboboxex_control::set_list(list_data& _choices)
	{
		choices = _choices;
		data_changed();
	}

	void comboboxex_control::on_create()
	{
		if (auto phost = window_host.lock()) {
			auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

			int h = text_style.fontSize * 8;
			MoveWindow(windows_control::window, boundsPixels.x, boundsPixels.y, boundsPixels.w, h, TRUE);
		}

		data_changed();
	}

	void comboboxex_control::on_resize()
	{
		if (auto phost = window_host.lock()) {
			auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

			if (windows_control::window) {
				MoveWindow(window, boundsPixels.x, boundsPixels.y, boundsPixels.w, windows_control::text_style.fontSize * 8, TRUE);
			}
		}
	}

	void richedit_control::set_html(const std::string& _text)
	{

	}

	std::string richedit_control::get_html()
	{
		return "";
	}

	void datetimepicker_control::set_text(const std::string& _text)
	{

	}

	std::string datetimepicker_control::get_text()
	{
		return "";
	}

	/*
	class monthcalendar_control : public windows_control<CMonthCalendarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD>
	{
	public:
	};
	*/

	bool animate_control::open(const std::string& _name)
	{
		return Animate_Open(windows_control::window, _name.c_str());
	}

	bool animate_control::open(DWORD resource_id)
	{
		return Animate_Open(windows_control::window, resource_id);
	}

	bool animate_control::play(UINT from, UINT to, UINT rep)
	{
		return Animate_Play(windows_control::window, from, to, rep);
	}

	bool animate_control::play()
	{
		return Animate_Play(windows_control::window, 0, -1, 1);
	}

	bool animate_control::stop()
	{
		return Animate_Stop(windows_control::window);
	}

}

#endif
