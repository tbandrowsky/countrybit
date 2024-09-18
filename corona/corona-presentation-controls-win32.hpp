/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This is a wrepper for the Win32 controls...
Notes

For Future Consideration
*/

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
			text_style.fontName = presentation_style_factory::get_current()->get_style()->PrimaryFont;
			if (text_style.fontName.empty())
				text_style.fontName = "Verdana";
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
			text_font(nullptr),
			is_group(false)
		{
			set_default_styles();
			is_default_focus = false;
			is_default_button = false;
		}

		windows_control(const windows_control& _src) : control_base(_src),
			window(nullptr),
			text_font(nullptr),
			is_group(false)
		{
			text_style = _src.text_style;
			is_default_focus = false;
			is_default_button = false;
		}

		windows_control(container_control_base *_parent, int _id)
			: control_base(_parent, _id),
			window(nullptr),
			text_font(nullptr),
			is_group(false)
		{
			set_default_styles();
			is_default_focus = false;
			is_default_button = false;
		}

		virtual void set_window_size()
		{
			calculate_margins();
			if (auto phost = window_host.lock()) {
				auto cbounds = control_base::get_inner_bounds();
				auto boundsPixels = phost->toPixelsFromDips(cbounds);

				if (windows_control::window != nullptr) {
					::MoveWindow(windows_control::window, boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, TRUE);
				}
			}
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

		virtual void put_json(json& _src)
		{
			control_base::put_json(_src);
			set_window_size();
		}

		void enable()
		{
			::EnableWindow(window, true);
		}

		void disable()
		{
			::EnableWindow(window, false);
		}

		void hide()
		{
			::ShowWindow(window, SW_HIDE);
		}

		void show()
		{
			::ShowWindow(window, SW_SHOW);
		}

		bool is_group;

		virtual void on_resize()
		{
			set_window_size();
		}

		virtual void create(std::weak_ptr<applicationBase> _host)
		{
			window_host = _host;

			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(inner_bounds);

				if (window == nullptr) {
					HWND parent = phost->getMainWindow();

					DWORD group_style = 0;

					if (is_group) {
						group_style = WS_GROUP | WS_TABSTOP;
					}

					if (auto wclassname = get_window_class_w())
						window = CreateWindowExW(get_window_ex_style(), get_window_class_w(), L"", get_window_style() | group_style, boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, parent, (HMENU)id, NULL, NULL);
					else
						window = CreateWindowEx(get_window_ex_style(), get_window_class(), "", get_window_style() | group_style, boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, parent, (HMENU)id, NULL, NULL);

					HFONT old_font = text_font;

					text_font = phost->createFontDips(window, text_style.fontName, text_style.fontSize, text_style.bold, text_style.italics);
					SendMessage(window, WM_SETFONT, (WPARAM)text_font, 0);

					if (old_font) {
						::DeleteObject(old_font);
						old_font = nullptr;
					}

					HWND tooltip = phost->getTooltipWindow();
					if (tooltip and tooltip_text.size() > 0) {
						TOOLINFOA toolInfo = { 0 };
						toolInfo.cbSize = sizeof(toolInfo);
						toolInfo.hwnd = parent;
						toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
						toolInfo.uId = (UINT_PTR)window;
						toolInfo.lpszText = (LPSTR)tooltip_text.c_str();
						SendMessageA(tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
					}

					set_window_size();
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
		std::string format;

	public:

		using control_base::id;
		using windows_control::window_host;

		std::shared_ptr<corona_bus_command> change_command;

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
			change_command = _src.change_command;
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

		void set_format(const std::string& _text)
		{
			format = _text;
		}

		std::string get_format()
		{
			return format;
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
			if (not json_field_name.empty()) {
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

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			windows_control::get_json(_dest);
			if (change_command) {
				json jcommand = jp.create_object();
				corona::get_json(jcommand, change_command);
				_dest.put_member("change_command", jcommand);
			}

			_dest.put_member("text", text);
			_dest.put_member("format", format);
		}

		virtual void put_json(json& _src)
		{
			windows_control::put_json(_src);

			json jcommand = _src["change_command"];
			corona::put_json(change_command, jcommand);

			std::string temp = _src["text"];
			set_text(temp);

			format = _src["format"];
			set_format(temp);
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			if (change_command) {
				_page->on_item_changed(id, [this](item_changed_event lce) {
					lce.bus->run_command(change_command);
					});
			}
		}

	};


	class table_control_base : public windows_control
	{
		char blank[256] = { 0 };

		void data_changed()
		{
			std::vector<std::string> data_row;
			strcpy_s(blank, "N/A");
			if (auto phost = window_host.lock())
			{
				phost->clearListView(id);
				int row_size = choices.columns.size();
				int num_rows = choices.items.size() * 2;
				std::map<std::string, int> column_map;
				int row_index = 0;
				int col_index = 0;
				for (auto col : choices.columns) {
					// damn it I know what I'm doing!
					// or... do I?
					char* t = (char *)col.display_name.c_str();
					phost->addListViewColumn(id, col_index, t, col.width, col.alignment);
					column_map[col.json_field_name] = col_index;
					col_index++;
				}
				data_row.resize(choices.columns.size());
				row_index++;

				if (choices.items.array())
				{
					for (int i = 0; i < choices.items.size(); i++)
					{
						auto item = choices.items.get_element(i);
						col_index = 0;
						for (auto col : choices.columns)
						{
							data_row[col_index] = blank;
							bool has_field = item.has_member(col.json_field_name);
							if (has_field) {
								std::string item_value = item[col.json_field_name].format_string(col.format);
								char* value = (char*)item_value.c_str();
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
		}

	public:

		using control_base::id;
		using windows_control::window_host;
		table_data choices;
		std::shared_ptr<corona_bus_command> select_command;

		table_control_base()
		{
		}

		table_control_base(const table_control_base& _src) : windows_control(_src)
		{
			choices = _src.choices;
			select_command = _src.select_command;
		}

		table_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<table_control_base>(*this);
			return tv;
		}


		virtual ~table_control_base() { ; }

		virtual void on_create()
		{
			ListView_SetExtendedListViewStyle(window, LVS_EX_FULLROWSELECT );
			data_changed();
		}

		void set_table(table_data& _choices)
		{
			choices = _choices;
			data_changed();
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			windows_control::get_json(_dest);
			json jtable_data = jp.create_object();
			choices.get_json(jtable_data);
			if (select_command) {
				json jcommand = jp.create_object();
				corona::get_json(jcommand, select_command);
				_dest.put_member("select_command", jcommand);
			}

			_dest.put_member("table", jtable_data);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			windows_control::put_json(_src);
			json jtable_data = _src["table"];
			choices.put_json(jtable_data);
			json command = _src["select_command"];

			if (command.empty()) {
				system_monitoring_interface::global_mon->log_information("table control missing select_command", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(_src);
			}

			corona::put_json(select_command, command);

			data_changed();

		}

		virtual bool set_items(json _data)
		{
			choices.items = _data;
			data_changed();
			return true;
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			_page->on_list_changed(id, [this](list_changed_event lce) {
				table_control_base* tcb = dynamic_cast<table_control_base*>(lce.control);
				if (tcb and tcb->select_command) {
					lce.bus->run_command(tcb->select_command);
				}
			});
		}

	};

	class list_control_base : public windows_control
	{
	public:
		using control_base::id;
		using windows_control::window_host;
		list_data choices;
		std::shared_ptr<corona_bus_command> select_command;

		list_control_base()
		{
		}

		list_control_base(const list_control_base& _src) : windows_control(_src)
		{
			choices = _src.choices;
			select_command = _src.select_command;
		}

		list_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
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
				std::string selection = phost->getListSelectedText(id);
				phost->clearListItems(id);
				if (choices.items.array()) {
					for (int i = 0; i < choices.items.size(); i++)
					{
						auto c = choices.items.get_element(i);
						int lid = c[choices.id_field];
						std::string description = c[choices.text_field];
						phost->addListItem(id, description, lid);
					}
					if (selection.size())
					{
						phost->setListSelectedText(id, selection.c_str());
					}
				}
			}
		}

		virtual json get_data()
		{
			json result;
			if (not json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					std::string new_text = ptr->getListSelectedText(id);
					int index_lists = ptr->getListSelectedIndex(id);
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
					std::string existing = ptr->getListSelectedText(id);
					if (existing != text) {
						ptr->setListSelectedText(id, text.c_str());
					}
				}
			}
			return _data;
		}

		virtual json get_selected_object()
		{
			json j;
			if (auto ptr = window_host.lock()) {
				int index_lists = ptr->getListSelectedIndex(id);
				if (index_lists >= 0) {
					j = choices.items.get_element(index_lists);
				}
			}
			return j;
		}

		void set_list(list_data& _choices)
		{
			choices = _choices;
			data_changed();
		}

		virtual bool set_items(json _data)
		{
			choices.items = _data;
			data_changed();
			return true;
		}

		virtual void on_create()
		{
			data_changed();
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			control_base::get_json(_dest);
			json jlist_data = jp.create_object();
			choices.get_json(jlist_data);
			_dest.put_member("list", jlist_data);
			if (select_command) {
				json jcommand = jp.create_object();
				corona::get_json(jcommand, select_command);
				_dest.put_member("select_command", jcommand);
			}

		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			control_base::put_json(_src);
			json jlist_data = _src["list"];
			choices.put_json(jlist_data);
			json command = _src["select_command"];
			if (command.empty()) {
				system_monitoring_interface::global_mon->log_information("list control missing select_command", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(_src);
			}

			corona::put_json(select_command, command);

			data_changed();
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			if (select_command) {
				_page->on_list_changed(id, [this](list_changed_event lce) {
						lce.bus->run_command(select_command);
				});
			}
		}
	};

	class dropdown_control_base : public windows_control
	{
	public:

		using control_base::id;
		using windows_control::window_host;
		list_data choices;
		std::shared_ptr<corona_bus_command> select_command;

		dropdown_control_base()
		{
		}

		dropdown_control_base(const dropdown_control_base& _src) : windows_control(_src)
		{
			choices = _src.choices;
			select_command = _src.select_command;
		}

		dropdown_control_base(container_control_base* _parent, int _id) : windows_control(_parent, _id)
		{
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
				if (choices.items.array()) 
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

		virtual json get_selected_object()
		{
			json j;
			if (auto ptr = window_host.lock()) {
				int index_lists = ptr->getComboSelectedIndex(id);
				if (index_lists >= 0) {
					j = choices.items.get_element(index_lists);
				}
			}
			return j;
		}

		void set_list(list_data& _choices)
		{
			choices = _choices;
			data_changed();
		}

		virtual void set_window_size()
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
			if (not json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					std::string new_text = ptr->getComboSelectedText(id);
					int index_lists = ptr->getComboSelectedIndex(id);
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
					std::string existing = ptr->getComboSelectedText(id);
					if (existing != text) {
						ptr->setComboSelectedText(id, text.c_str());
					}
				}
			}
			return _data;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			control_base::get_json(_dest);
			json jlist_data = jp.create_object();
			choices.get_json(jlist_data);
			_dest.put_member("list", jlist_data);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			control_base::put_json(_src);
			json jlist_data = _src["list"];
			choices.put_json(jlist_data);
			json command = _src["select_command"];
			if (command.empty()) {
				system_monitoring_interface::global_mon->log_information("dropdown control missing select_command", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(_src);
			}

			corona::put_json(select_command, command);

			data_changed();
		}

		virtual bool set_items(json _data)
		{
			choices.items = _data;
			data_changed();
			return true;
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			if (select_command) {
				_page->on_list_changed(id, [this](list_changed_event lce) {
					lce.bus->run_command(select_command);
					});
			}
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
	const int ListViewWindowsStyles = DefaultWindowStyles | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER | WS_VSCROLL;
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
	protected:
		using control_base::id;
		using windows_control::window_host;
		std::string caption_text;
		long caption_icon_id;
		HICON caption_icon;
	public:
		std::shared_ptr<corona_bus_command> click_command;

		button_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) { ; }
		button_control(const button_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<button_control<ButtonWindowStyles>>(*this);
			return tv;
		}

		virtual ~button_control() { ; }
		virtual const char* get_window_class() { return WC_BUTTON; }
		virtual DWORD get_window_style() { return ButtonWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			text_control_base::get_json(_dest);
			if (click_command) {
				json jcommand = jp.create_object();
				corona::get_json(jcommand, click_command);
				_dest.put_member("click_command", jcommand);
			}
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			text_control_base::put_json(_src);
			json jcommand = _src["click_command"];
			if (jcommand.empty()) {
				system_monitoring_interface::global_mon->log_information("button control missing click_command", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(_src);
			}

			corona::put_json(click_command, jcommand);
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			text_control_base::on_subscribe(_presentation, _page);

			if (click_command) {
				_page->on_command(id, [this](command_event lce) {
					lce.bus->run_command(click_command);
					});
			}
		}

	};

	class pushbutton_control : public button_control<PushButtonWindowStyles>
	{
	public:

		using windows_control::is_default_button;

		pushbutton_control(container_control_base* _parent, int _id) : button_control<PushButtonWindowStyles>(_parent, _id) { ; }
		pushbutton_control(const pushbutton_control& _src) = default;

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
		pressbutton_control(const pressbutton_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<pressbutton_control>(*this);
			return tv;
		}

		virtual ~pressbutton_control() { ; }
	};

	class radiobutton_control : public button_control<RadioButtonWindowStyles>
	{
		using button_control<RadioButtonWindowStyles>::window_host;
	public:

		radiobutton_control(container_control_base* _parent, int _id) : button_control<RadioButtonWindowStyles>(_parent, _id) { ; }
		radiobutton_control(const radiobutton_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<radiobutton_control>(*this);
			return tv;
		}

		virtual json get_data()
		{
			json result;
			if (not json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					bool test = ptr->getButtonChecked(id);
					result.put_member(json_field_name, test);
				}
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (not json_field_name.empty()) {
				json_parser jp;

				if (auto ptr = window_host.lock()) {
					int checked = _data.get_member(json_field_name);
					ptr->setButtonChecked(id, checked);
				}
			}
			return _data;
		}

		virtual ~radiobutton_control() { ; }
	};

	class checkbox_control : public button_control<CheckboxWindowStyles>
	{
	public:

		checkbox_control(container_control_base* _parent, int _id) : button_control<CheckboxWindowStyles>(_parent, _id) { ; }
		checkbox_control(const checkbox_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<checkbox_control>(*this);
			return tv;
		}

		virtual json get_data()
		{
			json result;
			if (not json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();

				if (auto ptr = window_host.lock()) {
					bool test = ptr->getButtonChecked(id);
					result.put_member(json_field_name, test);
				}
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (not json_field_name.empty()) {
				json_parser jp;

				if (auto ptr = window_host.lock()) {
					int checked = _data.get_member(json_field_name);
					ptr->setButtonChecked(id, checked);
				}
			}
			return _data;
		}


		virtual ~checkbox_control() { ; }
	};

	class linkbutton_control : public button_control<LinkButtonWindowStyles>
	{
	public:

		linkbutton_control(container_control_base* _parent, int _id) : button_control<LinkButtonWindowStyles>(_parent, _id) { ; }
		linkbutton_control(const linkbutton_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<linkbutton_control>(*this);
			return tv;
		}

		virtual ~linkbutton_control() { ; }
	};

	class edit_control : public text_control_base
	{

		using windows_control::window;

	public:

		std::string format;

		edit_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) { ; }
		virtual ~edit_control() { ; }
		edit_control(const edit_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<edit_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_EDIT; }
		virtual DWORD get_window_style() { return EditWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		static LRESULT myEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			LONG_PTR ptr = ::GetWindowLongPtrA(hWnd, GWLP_USERDATA);
			if (ptr) {
				auto pedit = (edit_control*)ptr;
				if (msg == WM_PAINT)
				{
					auto szFormat = pedit->format.c_str();
					auto ccFormat = pedit->format.size();
					PAINTSTRUCT ps = {};
					BeginPaint(hWnd, &ps);
					SetTextColor(ps.hdc, RGB(128, 128, 128));
					DrawText(ps.hdc, szFormat, ccFormat, &ps.rcPaint, DT_EDITCONTROL);
					EndPaint(hWnd, &ps);
				}
			}
			return CallWindowProcW(DefWindowProcW, hWnd, msg, wParam, lParam);
		}

		virtual void on_create() 
		{ 
			if (::IsWindow(window)) {
				::SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
			}
		}
	};

	class number_control : public edit_control
	{
	public:

		number_control(container_control_base* _parent, int _id) : edit_control(_parent, _id) { ; }
		virtual ~number_control() { ; }
		number_control(const number_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<number_control>(*this);
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

		listbox_control(const listbox_control& _src) = default;
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

		combobox_control(const combobox_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<combobox_control>(*this);
			return tv;
		}
		virtual void set_window_size();
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
		std::shared_ptr<corona_bus_command> select_command;

		comboboxex_control();
		comboboxex_control(container_control_base* _parent, int _id);

		comboboxex_control(const comboboxex_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<comboboxex_control>(*this);
			return tv;
		}
		virtual void set_window_size();

		virtual ~comboboxex_control() { ; }
		void data_changed();
		void set_list(list_data& _choices);
		virtual void on_create();

		virtual const char* get_window_class() { return WC_COMBOBOXEX; }
		virtual DWORD get_window_style() { return ComboWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual json get_selected_object()
		{
			json j;
			if (auto ptr = window_host.lock()) {
				int index_lists = ptr->getComboSelectedIndex(id);
				if (index_lists >= 0) {
					j = choices.items.get_element(index_lists);
				}
			}
			return j;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			control_base::get_json(_dest);
			json jlist_data = jp.create_object();
			choices.get_json(jlist_data);
			_dest.put_member("choices", jlist_data);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			control_base::put_json(_src);
			json jlist_data = _src["list"];
			choices.put_json(jlist_data);
			json command = _src["select_command"];
			if (command.empty()) {
				comm_bus_app_interface::global_bus->log_information("comboboxex control missing select_command", __FILE__, __LINE__);
				comm_bus_app_interface::global_bus->log_json(_src);
			}

			corona::put_json(select_command, command);

			data_changed();
		}

	};

	class listview_control : public table_control_base
	{
	public:

		listview_control(container_control_base* _parent, int _id) : table_control_base(_parent, _id) { ; }
		virtual ~listview_control() { ; }

		listview_control(const listview_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<listview_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_LISTVIEW; }
		virtual DWORD get_window_style() { return ListViewWindowsStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual json get_selected_object()
		{
			json j;
			if (auto ptr = window_host.lock()) {
				int index_lists = ptr->getListViewSelectedIndex(id);
				if (index_lists >= 0) {
					j = choices.items.get_element(index_lists);
				}
			}
			return j;
		}

	};

	class scrollbar_control : public windows_control
	{
	public:

		SCROLLINFO sbi;
		double scaleMin;
		double scaleMax;
		double scale;

		std::shared_ptr<corona_bus_command> change_command;

		scrollbar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) 
		{ 
			sbi = {};
			sbi.cbSize = sizeof(SCROLLINFO);
			sbi.fMask = SIF_PAGE | SIF_RANGE | SIF_POS | SIF_TRACKPOS;
			scaleMin = scaleMax = scale = 0;
		}
		virtual ~scrollbar_control() { ; }

		scrollbar_control(const scrollbar_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<scrollbar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_SCROLLBAR; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			if (change_command) {
				_page->on_item_changed(id, [this](item_changed_event lce) {
						lce.bus->run_command(change_command);
					});
			}
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			
			windows_control::get_json(_dest);

			if (window) {
				::GetScrollInfo(window, SB_CTL, &sbi);
			}

			_dest.put_member("min", sbi.nMin);
			_dest.put_member("max", sbi.nMax);
			_dest.put_member("page", sbi.nPage);
			_dest.put_member("pos", sbi.nPos);
			_dest.put_member("track", sbi.nTrackPos);
			_dest.put_member("scale_min", scaleMin);
			_dest.put_member("scale_max", scaleMax);

		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			windows_control::put_json(_src);
			sbi.nMin = (double)_src["min"];
			sbi.nMax = (double)_src["max"];
			sbi.nPage = (double)_src["page"];
			sbi.nPos = (double)_src["pos"];
			scaleMin = (double)_src["scale_min"];
			scaleMax = (double)_src["scale_max"];
			scale = (double)(sbi.nMax - sbi.nMin) / (scaleMax - scaleMin);
			if (window) {
				::SetScrollInfo(window, SB_CTL, &sbi, true);
			}

		}

		virtual void on_create()
		{
			if (window) {
				::SetScrollInfo(window, SB_CTL, &sbi, true);
			}
		}

		virtual json get_data()
		{
			json result;
			if (not json_field_name.empty()) {
				json_parser jp;
				if (window) {
					::GetScrollInfo(window, SB_CTL, &sbi);
				}
				result = jp.create_object();
				double v = (sbi.nPos - sbi.nMin) * scale + scaleMin;
				result.put_member(json_field_name, v);
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				double pos = _data[json_field_name];
				sbi.nPos = (pos - scaleMin) / scale + sbi.nMin;
			}
			return _data;
		}

	};

	class richedit_control : public text_control_base
	{
	public:
		std::shared_ptr<corona_bus_command> changed_command;

		void set_html(const std::string& _text);
		std::string get_html();

		richedit_control(container_control_base* _parent, int _id) : text_control_base(_parent, _id) {
			LoadLibrary(TEXT("Msftedit.dll"));
		}

		richedit_control(const richedit_control& _src) = default;
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

		date_time current_date;
		date_time min_date;
		date_time max_date;
		std::shared_ptr<corona_bus_command> change_command;

		void set_text(const std::string& _text);
		std::string get_text();

		datetimepicker_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }

		datetimepicker_control(const datetimepicker_control& _src) = default;

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<datetimepicker_control>(*this);
			return tv;
		}

		virtual ~datetimepicker_control() { ; }

		virtual const char* get_window_class() { return DATETIMEPICK_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles | DTS_SHORTDATECENTURYFORMAT; 		}		
		virtual DWORD get_window_ex_style() { return 0; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			if (change_command) {
				_page->on_item_changed(id, [this](item_changed_event lce) {
					lce.bus->run_command(change_command);
					});
			}
		}


		virtual void get_json(json& _dest)
		{
			json_parser jp;

			windows_control::get_json(_dest);

			if (window) {
				SYSTEMTIME st[3];
				DateTime_GetRange(window, &st[1]);
				min_date = st[1];
				max_date = st[2];
			}

			_dest.put_member("min_date", min_date);
			_dest.put_member("max_date", max_date);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			windows_control::put_json(_src);

			min_date = (date_time)_src["min_date"];
			max_date = (date_time)_src["max_date"];

			if (window) {
				SYSTEMTIME st[3];
				DateTime_SetRange(window, GDTR_MIN | GDTR_MAX, &st[1]);
				st[1] = min_date;
				st[2] = max_date;
			}
		}

		virtual void on_create()
		{
			if (window) {
				SYSTEMTIME st[3];
				st[0] = current_date;
				st[1] = min_date;
				st[2] = max_date;
				DateTime_SetRange(window, GDTR_MIN | GDTR_MAX, &st[1]);
				DateTime_SetSystemtime(window, GDT_VALID, &st[0]);
			}
		}

		virtual json get_data()
		{
			json result;
			if (not json_field_name.empty()) {
				if (window) {
					SYSTEMTIME st;
					DateTime_GetSystemtime(window, &st);
					current_date = st;
				}
				result.put_member(json_field_name, current_date);
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				current_date = (date_time)_data[json_field_name];
				if (window) {
					SYSTEMTIME st = current_date;
					DateTime_SetSystemtime(window, GDT_VALID, &st);
				}
			}
			return _data;
		}
	};

	class monthcalendar_control : public windows_control
	{
	public:
		date_time current_date;
		date_time min_date;
		date_time max_date;
		std::shared_ptr<corona_bus_command> change_command;

		monthcalendar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~monthcalendar_control() { ; }

		monthcalendar_control(const monthcalendar_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<monthcalendar_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return MONTHCAL_CLASS; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }


		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			if (change_command) {
				_page->on_item_changed(id, [this](item_changed_event lce) {
					lce.bus->run_command(change_command);
					});
			}
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			windows_control::get_json(_dest);

			if (window) {
				SYSTEMTIME st[3];
				MonthCal_GetRange(window, &st[1]);
				min_date = st[1];
				max_date = st[2];
			}

			_dest.put_member("min_date", min_date);
			_dest.put_member("max_date", max_date);

			if (change_command) {
				json jcommand = jp.create_object();
				corona::get_json(jcommand, change_command);
				_dest.put_member("change_command", jcommand);
			}

		}

		virtual void put_json(json& _src)
		{
			json_parser jp;

			windows_control::put_json(_src);

			json jcommand = _src["change_command"];
			if (jcommand.empty()) {
				system_monitoring_interface::global_mon->log_information("month calendar control missing change_command", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(_src);
			}

			corona::put_json(change_command, jcommand);

			min_date = (date_time)_src["min_date"];
			max_date = (date_time)_src["max_date"];

			if (window) {
				SYSTEMTIME st[3];
				MonthCal_SetRange(window, GDTR_MIN | GDTR_MAX, &st[1]);
				st[1] = min_date;
				st[2] = max_date;
				
			}
		}

		virtual void on_create()
		{
			if (window) {
				SYSTEMTIME st[3];
				st[0] = current_date;
				st[1] = min_date;
				st[2] = max_date;
				MonthCal_SetRange(window, GDTR_MIN | GDTR_MAX, &st[1]);
				MonthCal_SetCurSel(window, GDT_VALID, &st[0]);
				/*
				RECT rc = {};
				MonthCal_GetMinReqRect(window, &rc);

				// Resize the control now that the size values have been obtained.
				SetWindowPos(window, NULL, inner_bounds.x, inner_bounds.y, rc.right, rc.bottom, SWP_NOZORDER);
				*/
			}
		}

		virtual json get_data()
		{
			json_parser jp;
			json result = jp.create_object();
			if (not json_field_name.empty()) {
				if (window) {
					SYSTEMTIME st;
					MonthCal_GetCurSel(window, &st);
					current_date = st;
				}
				result.put_member(json_field_name, current_date);
			}
			return result;
		}

		virtual json set_data(json _data)
		{
			if (_data.has_member(json_field_name)) {
				current_date = (date_time)_data[json_field_name];
				if (window) {
					SYSTEMTIME st = current_date;
					MonthCal_SetCurSel(window, GDT_VALID, &st);
				}
			}
			return _data;
		}
	};

	class animate_control : public windows_control
	{

	public:
		animate_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~animate_control() { ; }

		std::shared_ptr<corona_bus_command> change_command;

		animate_control(const animate_control& _src) = default;
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

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			if (change_command) {
				_page->on_item_changed(id, [this](item_changed_event lce) {
						lce.bus->run_command(change_command);
					});
			}
		}

	};

	class treeview_control : public windows_control
	{
	public:
		treeview_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~treeview_control() { ; }

		std::shared_ptr<corona_bus_command> select_command;

		treeview_control(const treeview_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<treeview_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_TREEVIEW; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			if (select_command) {
				_page->on_list_changed(id, [this](list_changed_event lce) {
					lce.bus->run_command(select_command);
				});
			}
		}

	};

	class header_control : public windows_control
	{
		std::shared_ptr<corona_bus_command> select_command;

	public:
		header_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~header_control() { ; }

		header_control(const header_control& _src) = default;
		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<header_control>(*this);
			return tv;
		}

		virtual const char* get_window_class() { return WC_HEADER; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return 0; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			windows_control::on_subscribe(_presentation, _page);

			if (select_command) {
				_page->on_list_changed(id, [this](list_changed_event lce) {
					lce.bus->run_command(select_command);
				});
			}
		}

	};

	class toolbar_control : public windows_control
	{
	public:

		toolbar_control(container_control_base* _parent, int _id) : windows_control(_parent, _id) { ; }
		virtual ~toolbar_control() { ; }

		toolbar_control(const toolbar_control& _src) = default;
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

		statusbar_control(const statusbar_control& _src) = default;
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
		hotkey_control(const hotkey_control& _src) = default;
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
		draglistbox_control(const draglistbox_control& _src) = default;
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

	void combobox_control::set_window_size()
	{
		if (auto phost = window_host.lock()) {
			auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

			if (windows_control::window) {
				MoveWindow(window, boundsPixels.x, boundsPixels.y, boundsPixels.w, windows_control::text_style.fontSize * 8, TRUE);
			}
		}
	}

	comboboxex_control::comboboxex_control()
	{
	}

	comboboxex_control::comboboxex_control(container_control_base* _parent, int _id) : windows_control(_parent, _id)
	{
	}

	void comboboxex_control::data_changed()
	{
		if (IsWindow(window)) {
			if (auto phost = window_host.lock()) {
				phost->clearComboItems(id);
				for (int i = 0; i < choices.items.size(); i++)
				{
					auto c = choices.items.get_element(i);
					if (c.has_member(choices.id_field) and c.has_member(choices.text_field)) {
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
		set_window_size();
		data_changed();
	}

	void comboboxex_control::set_window_size()
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
