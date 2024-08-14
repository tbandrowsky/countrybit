/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
A controller / view for a corona application.
a presentation can have mulitple pages and it handles 
updates of all the little controllers.

Notes

For Future Consideration
*/


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

		corona::lockable presentation_lock;
		std::weak_ptr<page> current_page;
		rectangle current_size;
		lockable control_lock;
		std::map<std::string, int> control_ids;
		std::string home_page;
		point last_mouse_position;
		point last_mouse_click;
		json json_pages;

		comm_bus_interface *bus;

	public:
		int default_focus_id;
		int default_push_button_id;

		std::map<std::string, std::shared_ptr<page>> pages;
		std::weak_ptr<applicationBase> window_host;

		presentation(comm_bus_interface* _com_bus) : bus(_com_bus)
		{
			default_push_button_id = 0;
			default_focus_id = 0;
			last_mouse_position = {};
			last_mouse_click = {};
		}

		presentation(comm_bus_interface *_com_bus, std::weak_ptr<applicationBase> _window_host) : window_host(_window_host), bus(_com_bus)
		{
			default_push_button_id = 0;
			default_focus_id = 0;
			last_mouse_position = {};
			last_mouse_click = {};
		}

		page_base* get_current_page()
		{
			page_base* gcp = nullptr;
			if (auto cpg = current_page.lock()) {
				gcp = cpg.get();
			}
			return gcp;
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
		virtual bool is_current_page(const std::string& _page_name);
		virtual void select_page(const std::string& _page_name);
		virtual void select_page(const std::string& _page_name, std::function<void(page& pg)> _settings);
		virtual void set_home_page(const std::string& _page_name) 
		{
			home_page = _page_name;
		}
		virtual void load_page();

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

		template <typename control_type> control_type* find_ptr(std::string _name)
		{
			if (auto cp = current_page.lock()) {
				control_base* temp = cp->root->find(_name);
				if (temp == nullptr)
				{
					auto str = std::format("Control {0} not found ", _name);
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
		virtual void hardwareChanged();
		virtual std::string setPresentation(json pages);

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

		virtual int onResize()
		{
			return onResize(current_size, 1.0);
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
		if (!current_page.lock())
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

	bool presentation::is_current_page(const std::string& _page_name)
	{
		bool success = false;
		if (pages.contains(_page_name)) {		
			success = current_page.lock() == pages[_page_name];
		}
		return success;
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

	void presentation::load_page()
	{
		auto phost = window_host.lock();
		if (!phost) {
			throw std::logic_error("Cannot select a page without the window host being created.");
		}

		HWND hwndMainMenu = phost->getMainWindow();
		if (!hwndMainMenu) {
			throw std::logic_error("Cannot select a page without the window being created first.");
		}

		default_focus_id = 0;
		default_push_button_id = 0;

		if (auto ppage = current_page.lock()) {
			ppage->handle_unload(ppage);
			ppage->destroy();

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
			ppage->handle_onselect(ppage);
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
				auto pos = phost->getWindowClientPos();
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
			auto *style = presentation_style_factory::get_current()->get_style();			
			cp->draw();

			if (style->PageStyle) {
				viewStyleRequest vsr = *style->PageStyle;
				rectangle size_rect = current_size;
				point size_pt;
				size_pt.x = size_rect.w;
				size_pt.y = size_rect.h;
				vsr.apply_scale(size_pt);
				_ctx.setViewStyle(vsr);
				_ctx.drawRectangle(&size_rect, style->PageStyle->box_fill_brush.get_name(), style->PageStyle->box_border_thickness, style->PageStyle->box_fill_brush.get_name());
			}
			auto dc = _ctx.getDeviceContext();
			cp->render(dc);

			if (auto phost = window_host.lock()) {
				auto pos = phost->getWindowClientPos();

				// this draws the mouse
				if (false) {
					ID2D1SolidColorBrush* mouseBrush = nullptr;
					dc->CreateSolidColorBrush({ 0.0, 1.0, 0.0, 1.0 }, &mouseBrush);
					if (mouseBrush) {
						D2D1_ELLIPSE ellipse = {};
						ellipse.point.x = last_mouse_position.x;
						ellipse.point.y = last_mouse_position.y;
						ellipse.radiusX = 4;
						ellipse.radiusY = 4;
						dc->FillEllipse(ellipse, mouseBrush);

						ellipse.point.x = last_mouse_click.x;
						ellipse.point.y = last_mouse_click.y;
						ellipse.radiusX = 8;
						ellipse.radiusY = 8;
						dc->FillEllipse(ellipse, mouseBrush);
						mouseBrush->Release();
					}
				}

				// this draws the screen border and sizing bars

				double border_thickness = 6;

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
			cp->update(_elapsedSeconds, _totalSeconds);
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
		kde.bus = bus;
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
		kde.bus = bus;
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
		kde.bus = bus;
		if (cp) {
			cp->handle_key_up(_ctrl_id, kde);
		}
	}

	void presentation::mouseMove(point* _point)
	{
		last_mouse_position = *_point;
		auto cp = current_page.lock();
		mouse_move_event kde;
		kde.control_id = 0;
		kde.absolute_point = *_point;
		kde.bus = bus;
		if (cp) {
			cp->handle_mouse_move(0, kde);
			cp->root->set_mouse(*_point, nullptr, nullptr, nullptr, nullptr);
		}
	}

	void presentation::mouseLeftDown(point* _point)
	{
		bool leftMouse = true;
		last_mouse_click = *_point;
		auto cp = current_page.lock();

		if (cp) {
			presentation* p = this;
			SHORT keystate = ::GetKeyState(VK_CONTROL);

			if (keystate < 0) 
			{
				control_base* cb = cp->root->find(*_point);

				if (cb) 
				{
					system_monitoring_interface::global_mon->log_information("Control clicked");
					cb->dump();
				}
			}

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
				mcel.bus = p->bus;
				cp->handle_mouse_left_click(_item->id, mcel);

				mouse_click_event mce = {};
				mcel.bus = p->bus;
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
	}

	void presentation::mouseLeftUp(point* _point)
	{
		bool leftMouse = false;
		auto cp = current_page.lock();
		if (cp) {
			presentation* p = this;

			cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				mce.bus = p->bus;

				cp->handle_mouse_click(_item->id, mce);

				mouse_left_click_event mcel;
				mcel.control = _item;
				mcel.control_id = _item->id;
				mcel.absolute_point.x = _point->x;
				mcel.absolute_point.y = _point->y;
				mcel.absolute_point.z = 0;
				mcel.bus = p->bus;

				cp->handle_mouse_left_click(_item->id, mcel);
				}, nullptr);
		}
	}

	void presentation::mouseRightDown(point* _point)
	{
		bool rightMouse = true;
		auto cp = current_page.lock();
		if (cp) {
			presentation* p = this;
			cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr, [cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				mce.bus = p->bus;

				cp->handle_mouse_click(_item->id, mce);
				});
		}
	}

	void presentation::mouseRightUp(point* _point)
	{
		bool rightMouse = false;
		auto cp = current_page.lock();
		if (cp) {
			presentation* p = this;
			cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr, [cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				mce.bus = p->bus;
				cp->handle_mouse_click(_item->id, mce);
				mouse_right_click_event mcel;
				mcel.control = _item;
				mcel.control_id = _item->id;
				mcel.absolute_point.x = _point->x;
				mcel.absolute_point.y = _point->y;
				mcel.absolute_point.z = 0;
				mcel.bus = p->bus;

				cp->handle_mouse_right_click(_item->id, mcel);
				});
		}
	}

	void presentation::pointSelected(point* _point, ccolor* _color)
	{
		;
	}

	LRESULT presentation::ncHitTest(point* _point)
	{
		LRESULT result = HTCLIENT;

		mouseMove(_point);

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
		ce.bus = bus;
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
			lce.bus = bus;

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
			lce.bus = bus;
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

			ptr->setRedraw(dropDownId, false);
			list_changed_event lce;
			lce.control_id = dropDownId;
			lce.selected_text = new_text;
			lce.selected_value = value;
			lce.selected_index = index;
			lce.state = 0;
			lce.control = nullptr; // the page will assign this.
			lce.bus = bus;
			auto cp = current_page.lock();
			if (cp) {
				cp->handle_list_changed(dropDownId, lce);
			}
			ptr->setRedraw(dropDownId, true);
			ptr->redraw(dropDownId, 0);
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
			lce.bus = bus;
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


	void presentation::hardwareChanged()
	{
		if (auto pg = current_page.lock()) {
			pg->hardware_scan();
		}
	}
	
	std::string presentation::setPresentation(json _json_pages)
	{
		corona::scope_lock lock(presentation_lock);
		json_pages = _json_pages;
		std::string default_page_name;

		if (json_pages.error()) {
			pages.clear();
			create_page("errors", [this](page& _pg) {
				control_builder cb(_pg.get_root_container());
				cb.grid_view_begin(id_counter::next(), [this](grid_view& rvl) {
					json_parser jp;
					array_data_source ads;
					ads.data = json_pages["errors"];
/*					using cell_json_size = std::function<point(draw_control* _parent, int _index, rectangle _bounds)>;
					using cell_json_draw = std::function<void(draw_control* _parent, int _index, rectangle _bounds)>;
					using cell_json_assets = std::function<void(draw_control* _parent, rectangle _bounds)>;
					*/
					ads.draw_item = [this](draw_control* _parent, int _index, json _data, rectangle _bounds) {
						auto json_object = _data.get_element(_index);
						auto object_members = json_object.get_members();
						double x = _bounds.x;
						double w = 0.0;
						if (auto win = _parent->window.lock()) {
							auto& ctxt = win->getContext();
							ctxt.drawRectangle(&_bounds, "item_border", 1, nullptr);
							std::string eline = json_object.get_member("line");
							std::string echar = json_object.get_member("char");
							std::string etopic = json_object.get_member("topic");
							std::string eerror = json_object.get_member("error");

							std::string text = std::format("{0}: '{1}' {2} - {3}", eline, echar, etopic, eerror);
							auto field_bounds = _bounds;
							field_bounds.x = x;
							ctxt.drawText(text, &field_bounds, "item_paragraph", "item_foreground");
							x += w;
						}
						};
					ads.size_item = [this](draw_control* _parent, int _index, json _item, rectangle _bounds) -> point {
						point pt;
						pt.x = _bounds.w;
						pt.y = 50;
						return pt;
						};
					ads.assets = [this](draw_control* _parent, rectangle _bounds) {
						if (auto win = _parent->window.lock()) {
							auto& ctxt = win->getContext();

							textStyleRequest tsr = {};
							tsr.fontName = "Arial";
							tsr.fontSize = 14;
							tsr.name = "item_paragraph";
							ctxt.setTextStyle(&tsr);

							solidBrushRequest sbr;
							sbr.brushColor = toColor("#000000");
							sbr.name = "item_foreground";
							ctxt.setSolidColorBrush(&sbr);

							sbr.brushColor = toColor("#FFFFFF");
							sbr.name = "item_background";
							ctxt.setSolidColorBrush(&sbr);

							sbr.brushColor = toColor("#C0C0C0");
							sbr.name = "item_border";
							ctxt.setSolidColorBrush(&sbr);
						}
					};
					rvl.set_item_source(ads);
					});
				});
				select_page("errors");
			return default_page_name;
		}

		json jstyles = json_pages.get_member("styles");
		if (jstyles.array())
		{
			for (auto js : jstyles) {
				presentation_style_factory::get_current()->load_style_sheet(js);
			}
		}

		json jpages = json_pages.get_member("pages");
		
		if (jpages.array())
		{
			std::string current_page_name;
			if (auto cp = current_page.lock()) {
				current_page_name = cp->name;
			}
			pages.clear();
			for (auto pg : jpages)
			{
				if (pg.object()) {
					std::string class_name = pg["class_name"];
					if (class_name == "page") {
						bool is_default = (bool)pg["default"];
						std::string name = pg["page_name"];
						if (name.empty()) {
							system_monitoring_interface::global_mon->log_warning("page_name is empty for this page, skipping");
						}
						if (is_default || default_page_name.empty()) {
							default_page_name = name;
						}
						create_page(name, [pg, this](page& _settings)->void
							{
								json_parser jp;
								auto root = _settings.get_root_container();
								root->set_size(1.0_container, 1.0_container);
								root->children.clear();
								control_builder cb(root);
								cb.bus = bus;
								json jchildren = pg["children"];
								if (jchildren.array()) {	
									for (auto jchild : jchildren) 
									{
										auto child = cb.from_json(jchild);
									}
								}
							});
					}
					else 
					{
						std::stringstream ss;
						ss << "Unknown class_name: " << class_name << std::endl;
						system_monitoring_interface::global_mon->log_warning(ss.str());
					}
				}
			}
			if (!current_page_name.empty() && pages.contains(current_page_name)) {
				default_page_name = current_page_name;
				current_page = pages[current_page_name];
			}
		}

		load_page();
		return default_page_name;
	}
}

#endif
