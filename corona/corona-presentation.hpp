#ifndef CORONA_PRESENTATION_H
#define CORONA_PRESENTATION_H

namespace corona {

	enum class field_layout
	{
		label_on_left = 1,
		label_on_top = 2
	};

	class presentation : public controller, public presentation_base
	{
	protected:

		std::weak_ptr<page> current_page;
		rectangle current_size;
		lockable control_lock;
		std::map<std::string, int> control_ids;
		std::string home_page;

	public:
		int default_focus_id;
		int default_push_button_id;

		std::map<std::string, std::shared_ptr<page>> pages;
		std::weak_ptr<applicationBase> window_host;
		std::shared_ptr<data_lake> data;

		presentation()
		{
			data = std::make_shared<data_lake>();
			default_push_button_id = 0;
			default_focus_id = 0;
		}

		presentation(std::weak_ptr<applicationBase> _window_host) : window_host(_window_host)
		{
			data = std::make_shared<data_lake>();
			default_push_button_id = 0;
			default_focus_id = 0;
		}

		int get_control_id(std::string _name, std::function<int()> _id)
		{
			int temp = 0;
			scope_lock lockit(control_lock);

			if (!control_ids.contains(_name)) {
				control_ids.insert_or_assign(_name, _id());
			}
			return control_ids[_name];
		}

		virtual ~presentation()
		{
			;
		}

		virtual int getDefaultButtonId(void)
		{
			return default_push_button_id;
		}

		virtual page& create_page(std::string _name, std::function<void(page& pg)> _settings = nullptr);
		virtual void select_page(const std::string& _page_name);
		virtual void select_page(const std::string& _page_name, std::function<void(page& pg)> _settings);
		virtual void set_home_page(const std::string& _page_name) 
		{
			home_page = _page_name;
		}

		template <typename control_type> control_type& find(int _id)
		{
			if (auto cp = current_page.lock()) {
				control_base* temp = cp->root->find(_id);
				if (temp == nullptr)
				{
					auto str = std::format("Control {0} not found ", _id);
					throw std::invalid_argument(str);
				}
				control_type* citem = dynamic_cast<control_type*>(temp);
				if (citem == nullptr)
				{
					auto str = std::format("Object is not {0} ", typeid(control_type).name());
					throw std::invalid_argument(str);
				}
				return *citem;
			}
			throw std::exception("could not lock current page");
		}

		template <typename control_type> control_type *find_ptr(int _id)
		{
			if (auto cp = current_page.lock()) {
				control_base* temp = cp->root->find(_id);
				if (temp == nullptr)
				{
					auto str = std::format("Control {0} not found ", _id);
					return nullptr;
				}
				control_type* citem = dynamic_cast<control_type*>(temp);
				return citem;
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
				control_type* citem = dynamic_cast<control_type*>(temp);
				if (citem == nullptr)
				{
					auto str = std::format("Object is not {0} ", typeid(*citem).name());
					throw std::invalid_argument(str);
				}
				return *citem;
			}
			throw std::exception("could not lock current page");
		}

		virtual bool drawFrame(direct2dContext& _ctx);
		virtual bool update(double _elapsedSeconds, double _totalSeconds);

		virtual void keyPress(int _ctrl_id, short _key);
		virtual void keyDown(int _ctrl_id, short _key);
		virtual void keyUp(int _ctrl_id, short _key);
		virtual void mouseMove(point* _point);
		virtual void mouseLeftDown(point* _point);
		virtual void mouseLeftUp(point* _point);
		virtual void mouseRightDown(point* _point);
		virtual void mouseRightUp(point* _point);
		virtual void pointSelected(point* _point, ccolor* _color);
		virtual LRESULT ncHitTest(point* _point);
		virtual void setFocus(int ddlControlId);
		virtual void killFocus(int ddlControlId);
		virtual bool navigationKey(int _key);

		std::function<void(presentation*)> create_pages;

		virtual void onHostCreated();
		virtual void onCreated();
		virtual void onCommand(int buttonId);
		virtual void onTextChanged(int textControlId);
		virtual void onDropDownChanged(int dropDownId);
		virtual void onListBoxChanged(int listBoxId);
		virtual void onListViewChanged(int listViewId);
		virtual int onHScroll(int controlId, scrollTypes scrollType);
		virtual int onVScroll(int controlId, scrollTypes scrollType);
		virtual int onResize(const rectangle& newSize, double d2dScale);
		virtual int onSpin(int controlId, int newPosition);
		virtual void onJobComplete(bool _success, int _id);
		virtual void onTaskComplete(bool _success, ui_task_result_base* _result);
		virtual void onDataChanged(json _params, data_lake* _api, data_function* _set);
		virtual void onLogged(data_lake* _api);

		virtual int layout();

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

		template <typename control_type> control_type* get_parent_for_control_by_id(int _id)
		{
			control_type* r = nullptr;
			if (auto ppage = current_page.lock())
			{
				auto& rpage = *ppage;
				control_base* cb = rpage[_id];
				if (cb) {
					r = dynamic_cast<control_type*>(cb);
					if (r && r->parent) 
					{
						r = dynamic_cast<control_type*>(r->parent->get_control());
					}
					else 
					{
						r = nullptr;
					}
				}
			}
			return r;
		}

		virtual void restore_window()
		{
			if (auto pw = window_host.lock()) {

				WINDOWPLACEMENT wp = {};
				wp.length = sizeof(wp);
				GetWindowPlacement(pw->getMainWindow(), &wp);
				if (wp.showCmd == SW_SHOWMAXIMIZED) {
					SendMessage(pw->getMainWindow(), WM_SYSCOMMAND, SC_RESTORE, 0);
				}
				else
				{
					SendMessage(pw->getMainWindow(), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				}
			}

		}

		virtual void minimize_window()
		{
			if (auto pw = window_host.lock()) {
				SendMessage(pw->getMainWindow(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
			}
		}

		virtual void close_window() 
		{
			if (auto pw = window_host.lock()) {
				SendMessage(pw->getMainWindow(), WM_SYSCOMMAND, SC_CLOSE, 0);
			}
		}

		void open_menu(control_base* _base, menu_item& _menu)
		{
			auto menu = _menu.to_menu();
			if (auto ptr = window_host.lock()) {
				HWND hwndMenu = ptr->getMainWindow();
				POINT tpstart;
				auto& bpos = _base->get_bounds();
				tpstart.x = bpos.right() * ::GetDpiForWindow(hwndMenu) / 96.0;
				tpstart.y = bpos.bottom() * ::GetDpiForWindow(hwndMenu) / 96.0;
				::ClientToScreen(hwndMenu, &tpstart);
				::TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN, tpstart.x, tpstart.y, hwndMenu, nullptr);
			}
		}

	};

	int id_counter::status_text_title_id = 20000;
	int id_counter::status_text_subtitle_id = 20001;
	int id_counter::status_bar_id = 20002;

	int id_counter::id = 10001;
	int id_counter::next()
	{
		id++;
		return id;
	}

	page& presentation::create_page(std::string _name, std::function<void(page& pg)> _settings)
	{
		auto new_page = std::make_shared<page>(_name);
		page& pg = *new_page.get();
		pg.parent = this;
		if (_settings) {
			_settings(pg);
		}
		pages[_name] = new_page;
		current_page = new_page;
		return pg;
	}

	void presentation::select_page(const std::string& _page_name, std::function<void(page& pg)> _settings)
	{
		auto phost = window_host.lock();
		if (!phost) {
			throw std::logic_error("Cannot select a page without the window host being created.");
		}

		HWND hwndMainMenu = phost->getMainWindow();
		if (!hwndMainMenu) {
			throw std::logic_error("Cannot select a page without the window being created first.");
		}

		if (pages.contains(_page_name)) {
			if (auto ppage = current_page.lock()) {
				ppage->handle_unload(ppage);
				ppage->destroy();
			}
			auto ptr = pages[_page_name];
			_settings(*ptr);
			current_page = pages[_page_name];
		}

		default_focus_id = 0;
		default_push_button_id = 0;

		if (auto ppage = current_page.lock()) {
			ppage->handle_onselect(ppage);
			auto root = ppage->get_root();
			root->foreach([this](control_base* _item) {
				pushbutton_control* pct = dynamic_cast<pushbutton_control*>(_item);
				if (pct && pct->is_default_button) {
					this->default_push_button_id = pct->get_id();
				}
				windows_control* wct = dynamic_cast<windows_control*>(_item);
				if (wct && wct->is_default_focus) {
					this->default_focus_id = wct->get_id();
				}
			});
		}

		onCreated();
		onResize(current_size, 1.0);
		if (auto ppage = current_page.lock()) {
			if (ppage->menu)
			{
				HMENU hmenu = ppage->menu->to_menu();
				::SetMenu(hwndMainMenu, hmenu);
				::DrawMenuBar(hwndMainMenu);
			}
			HWND hwnd_control = ::GetDlgItem(phost->getMainWindow(), default_focus_id);
			if (hwnd_control) {
				::SetFocus(hwnd_control);
			}

			ppage->handle_onload(ppage);
		}
	}

	void presentation::select_page(const std::string& _page_name)
	{

		auto phost = window_host.lock();
		if (!phost) {
			throw std::logic_error("Cannot select a page without the window host being created.");
		}

		HWND hwndMainMenu = phost->getMainWindow();
		if (!hwndMainMenu) {
			throw std::logic_error("Cannot select a page without the window being created first.");
		}

		if (pages.contains(_page_name)) {
			if (auto ppage = current_page.lock()) {
				ppage->handle_unload(ppage);
				ppage->destroy();
			}
			current_page = pages[_page_name];
		}

		default_focus_id = 0;
		default_push_button_id = 0;

		if (auto ppage = current_page.lock()) {
			ppage->handle_onselect(ppage);
			auto root = ppage->get_root();
			root->foreach([this](control_base* _item) {
				pushbutton_control* pct = dynamic_cast<pushbutton_control*>(_item);
				if (pct && pct->is_default_button) {
					this->default_push_button_id = pct->get_id();
				}
				windows_control* wct = dynamic_cast<windows_control*>(_item);
				if (wct && wct->is_default_focus) {
					this->default_focus_id = wct->get_id();
				}
			});
		}

		onCreated();
		onResize(current_size, 1.0);

		if (auto ppage = current_page.lock()) {
			if (ppage->menu)
			{
				HMENU hmenu = ppage->menu->to_menu();
				::SetMenu(hwndMainMenu, hmenu);
				::DrawMenuBar(hwndMainMenu);
			}

			HWND hwnd_control = ::GetDlgItem(phost->getMainWindow(), default_focus_id);
			if (hwnd_control) {
				::SetFocus(hwnd_control);
			}
			ppage->handle_onload(ppage);
			auto root = ppage->get_root();
		}

	}

	void presentation::onCreated()
	{
		auto cp = current_page.lock();
		if (cp) {
			cp->handle_onselect(cp);
			if (auto phost = window_host.lock()) {
				auto sheet = styles.get_style();
				auto pos = phost->getWindowClientPos();
				phost->toPixelsFromDips(pos);
				cp->arrange(pos.w, pos.h);
				cp->create(phost);
				cp->subscribe(this);
			}
		}
	}

	void presentation::onHostCreated()
	{
		if (create_pages) {
			create_pages(this);
		}

		if (home_page.size()) 
		{
			select_page(home_page);
		}
	}

	bool presentation::drawFrame(direct2dContext& _ctx)
	{
		auto cp = current_page.lock();
		if (cp) {
			cp->draw();

			auto dc = _ctx.getDeviceContext();
			cp->render(dc);

			if (auto phost = window_host.lock()) {
				auto pos = phost->getWindowClientPos();

				double border_thickness = 4;

				linearGradientBrushRequest lgbr;
				lgbr.start.x = pos.w;
				lgbr.start.y = pos.h;
				lgbr.stop.x = 0;
				lgbr.stop.y = 0;
				lgbr.name = "presentation_shade";
				lgbr.gradientStops = {
					{ toColor("#F0F0F0FF"), 0.0 },
					{ toColor("#303030FF"), 0.10 },
					{ toColor("#404040FF"), 0.90 },
					{ toColor("#202020FF"), 0.95 },
					{ toColor("#A0A0A0FF"), 1.0 },
				};
				_ctx.setLinearGradientBrush(&lgbr);

				double inner_right = pos.w - border_thickness * 2;
				double inner_bottom = pos.h - border_thickness * 2;
				double left_side = border_thickness - 1;

				pathImmediateDto pathx;
				pathx.fillBrushName = "presentation_shade";
				pathx.borderBrushName = "presentation_shade";
				pathx.strokeWidth = 2;
				pathx.path.addLineTo(0, 0);
				pathx.path.addLineTo(left_side, 0);
				pathx.path.addLineTo(left_side, inner_bottom);
				pathx.path.addLineTo(inner_right, inner_bottom);
				pathx.path.addLineTo(inner_right, 0);
				pathx.path.addLineTo(pos.w, 0);
				pathx.path.addLineTo(pos.w, pos.h);
				pathx.path.addLineTo(0, pos.h);
				pathx.path.addLineTo(0, 0);
				pathx.closed = true;

				_ctx.drawPath(&pathx);
			}

		}
		return false;
	}

	bool presentation::update(double _elapsedSeconds, double _totalSeconds)
	{
		auto cp = current_page.lock();
		if (cp) {
			cp->update(data, _elapsedSeconds, _totalSeconds);
		}
		return true;
	}

	void presentation::setFocus(int _ctrl_id)
	{
		auto ctrl = get_control<control_base>(_ctrl_id);
		if (ctrl)
			ctrl->set_focus();
	}

	void presentation::killFocus(int _ctrl_id)
	{
		auto ctrl = get_control<control_base>(_ctrl_id);
		if (ctrl)
			ctrl->kill_focus();
	}

	bool presentation::navigationKey(int _key)
	{
		bool r = false;
		auto cp = current_page.lock();

		HWND focusedWindow = ::GetFocus();
		while (focusedWindow || focusedWindow == ::GetDesktopWindow()) {
			int focusedWindowId = ::GetDlgCtrlID(focusedWindow);
			auto ctrl = get_control<control_base>(focusedWindowId);
			if (ctrl) {
				bool is_cm = ctrl->is_control_message(_key);
				if (is_cm)
					return is_cm;
			}
			focusedWindow = ::GetAncestor(focusedWindow, GA_PARENT);
		}
		return r;
	}

	void presentation::keyPress(int _ctrl_id, short _key)
	{
		auto cp = current_page.lock();
		key_press_event kde;
		kde.control_id = _ctrl_id;
		kde.key = _key;
		if (cp) {
			cp->handle_key_press(_ctrl_id, kde);
		}
	}

	void presentation::keyDown(int _ctrl_id, short _key)
	{
		auto cp = current_page.lock();
		key_down_event kde;
		kde.control_id = _ctrl_id;
		kde.key = _key;
		if (cp) {
			cp->handle_key_down(_ctrl_id, kde);
		}
	}

	void presentation::keyUp(int _ctrl_id, short _key)
	{
		auto cp = current_page.lock();
		key_up_event kde;
		kde.control_id = _ctrl_id;
		kde.key = _key;
		if (cp) {
			cp->handle_key_up(_ctrl_id, kde);
		}
	}

	void presentation::mouseMove(point* _point)
	{
		auto cp = current_page.lock();
		mouse_move_event kde;
		kde.control_id = 0;
		kde.absolute_point = *_point;
		if (cp) {
			cp->handle_mouse_move(0, kde);
		}
		cp->root->set_mouse(*_point, nullptr, nullptr, nullptr, nullptr);
	}

	void presentation::mouseLeftDown(point* _point)
	{
		bool leftMouse = true;
		auto cp = current_page.lock();
		presentation* p = this;
		cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p, _point](control_base* _item) {
			mouse_left_click_event mcel = {};
			mcel.control = _item;
			mcel.control_id = _item->id;
			mcel.absolute_point.x = _point->x;
			mcel.absolute_point.y = _point->y;
			mcel.absolute_point.z = 0;
			if (_item) {
				mcel.relative_point.x = _point->x - _item->get_bounds().x;
				mcel.relative_point.y = _point->y - _item->get_bounds().y;
				mcel.relative_point.z = 0;
			}
			cp->handle_mouse_left_click(_item->id, mcel);

			mouse_click_event mce = {};
			mce.control = _item;
			mce.control_id = _item->id;
			mce.absolute_point.x = _point->x;
			mce.absolute_point.y = _point->y;
			mce.absolute_point.z = 0;
			if (_item) {
				mcel.relative_point.x = _point->x - _item->get_bounds().x;
				mcel.relative_point.y = _point->y - _item->get_bounds().y;
				mcel.relative_point.z = 0;
			}
			cp->handle_mouse_click(_item->id, mce);
			}, nullptr);
	}

	void presentation::mouseLeftUp(point* _point)
	{
		bool leftMouse = false;
		auto cp = current_page.lock();
		presentation* p = this;
		cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p, _point](control_base* _item) {
			mouse_click_event mce;
			mce.control = _item;
			mce.control_id = _item->id;
			mce.absolute_point.x = _point->x;
			mce.absolute_point.y = _point->y;
			mce.absolute_point.z = 0;
			cp->handle_mouse_click(_item->id, mce);

			mouse_left_click_event mcel;
			mcel.control = _item;
			mcel.control_id = _item->id;
			mcel.absolute_point.x = _point->x;
			mcel.absolute_point.y = _point->y;
			mcel.absolute_point.z = 0;
			cp->handle_mouse_left_click(_item->id, mcel);
			}, nullptr);
	}

	void presentation::mouseRightDown(point* _point)
	{
		bool rightMouse = true;
		auto cp = current_page.lock();
		presentation* p = this;
		cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr, [cp, p, _point](control_base* _item) {
			mouse_click_event mce;
			mce.control = _item;
			mce.control_id = _item->id;
			mce.absolute_point.x = _point->x;
			mce.absolute_point.y = _point->y;
			mce.absolute_point.z = 0;
			cp->handle_mouse_click(_item->id, mce);
			});
	}

	void presentation::mouseRightUp(point* _point)
	{
		bool rightMouse = false;
		auto cp = current_page.lock();
		presentation* p = this;
		cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr, [cp, p, _point](control_base* _item) {
			mouse_click_event mce;
			mce.control = _item;
			mce.control_id = _item->id;
			mce.absolute_point.x = _point->x;
			mce.absolute_point.y = _point->y;
			mce.absolute_point.z = 0;
			cp->handle_mouse_click(_item->id, mce);
			mouse_right_click_event mcel;
			mcel.control = _item;
			mcel.control_id = _item->id;
			mcel.absolute_point.x = _point->x;
			mcel.absolute_point.y = _point->y;
			mcel.absolute_point.z = 0;
			cp->handle_mouse_right_click(_item->id, mcel);
			});
	}

	void presentation::pointSelected(point* _point, ccolor* _color)
	{
		;
	}

	LRESULT presentation::ncHitTest(point* _point)
	{
		LRESULT result = HTCLIENT;

		if (auto ppage = current_page.lock())
		{
			auto hitBox = ppage->root->find(*_point);
			if (hitBox) {
				result = hitBox->get_nchittest();
			}
		}

		return result;
	}

	void presentation::onCommand(int buttonId)
	{
		auto cp = current_page.lock();
		command_event ce;
		ce.control_id = buttonId;
		if (cp) {
			cp->handle_command(buttonId, ce);
		}
	}

	void presentation::onTextChanged(int textControlId)
	{
		if (auto ptr = window_host.lock()) {
			std::string new_text = ptr->getEditText(textControlId);
			item_changed_event lce;
			lce.control_id = textControlId;
			lce.text_value = new_text;
			auto cp = current_page.lock();
			if (cp) {
				cp->handle_item_changed(textControlId, lce);
			}
		}
	}

	void presentation::onDropDownChanged(int dropDownId)
	{
		if (auto ptr = window_host.lock()) {
			std::string new_text = ptr->getComboSelectedText(dropDownId);
			int index = ptr->getComboSelectedIndex(dropDownId);
			int value = ptr->getComboSelectedValue(dropDownId);
			list_changed_event lce;
			lce.control_id = dropDownId;
			lce.selected_text = new_text;
			lce.selected_value = value;
			lce.selected_index = index;
			lce.state = 0;
			lce.control = nullptr; // the page will assign this.
			auto cp = current_page.lock();
			if (cp) {
				cp->handle_list_changed(dropDownId, lce);
			}
		}
	}

	void presentation::onListBoxChanged(int dropDownId)
	{
		if (auto ptr = window_host.lock()) {
			std::string new_text = ptr->getListSelectedText(dropDownId);
			int index = ptr->getListSelectedIndex(dropDownId);
			int value = ptr->getListSelectedValue(dropDownId);
			list_changed_event lce;
			lce.control_id = dropDownId;
			lce.selected_text = new_text;
			lce.selected_value = value;
			lce.selected_index = index;
			lce.state = 0;
			lce.control = nullptr; // the page will assign this.
			auto cp = current_page.lock();
			if (cp) {
				cp->handle_list_changed(dropDownId, lce);
			}
		}
	}

	void presentation::onListViewChanged(int listViewId)
	{
		if (auto ptr = window_host.lock()) {
			std::string new_text = ptr->getListViewSelectedText(listViewId);
			int index = ptr->getListViewSelectedIndex(listViewId);
			int value = ptr->getListViewSelectedValue(listViewId);
			list_changed_event lce;
			lce.control_id = listViewId;
			lce.selected_text = new_text;
			lce.selected_value = value;
			lce.selected_index = index;
			lce.state = 0;
			lce.control = nullptr; // the page will assign this.
			auto cp = current_page.lock();
			if (cp) {
				cp->handle_list_changed(listViewId, lce);
			}
		}
	}

	int presentation::onHScroll(int controlId, scrollTypes scrollType)
	{
		return 0;
	}

	int presentation::onVScroll(int controlId, scrollTypes scrollType)
	{
		return 0;
	}

	int presentation::onResize(const rectangle& newSize, double d2dScale)
	{
		current_size = newSize;
		auto pg = current_page.lock();
		if (pg) {
			pg->arrange(newSize.w, newSize.h);
		}
		onCreated();
		return 0;
	}

	int presentation::layout()
	{
		auto pg = current_page.lock();
		if (pg) {
			pg->arrange(current_size.w, current_size.h);
		}
		return 0;
	}

	int presentation::onSpin(int controlId, int newPosition)
	{
		int value = newPosition;
		return 0;
	}

	void presentation::onJobComplete(bool _success, int _id)
	{
		;
	}

	void presentation::onTaskComplete(bool _success, ui_task_result_base* _result)
	{
		;
	}

	void presentation::onDataChanged(json _params, data_lake* _api, data_function* _set)
	{
		if (auto pg = current_page.lock()) {
			pg->handle_changed(pg, _params, _api, _set);
		}
	}

	void presentation::onLogged(data_lake* _api)
	{
		if (auto pg = current_page.lock()) {
			pg->handle_logged(pg, _api);
		}
	}

}

#endif
