/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is the main window file for a gui direct 2d corona application.
It has the window main, and handles the interaction betweenn presentations
and their pages.
It has the message loop, for example.

Notes
co_await implementation instead of the janky queue 
mechanism might be better.  but, it works.

For Future Consideration
*/


#ifndef CORONA_DIRECTAPPLICATION_H
#define CORONA_DIRECTAPPLICATION_H

namespace corona
{



	class directApplicationWin32 : public applicationBase, public std::enable_shared_from_this<directApplicationWin32>
	{
	protected:

		static directApplicationWin32* current;
		static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK controlWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		virtual LRESULT windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		virtual LRESULT controlWindowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		HINSTANCE hinstance;
		HWND hwndRoot;
		std::list<int> pressedKeys;

		__int64 performanceFrequency;
		__int64 startCounter, lastCounter;
		point minimumWindowSize;

		bool colorCapture;
		int iconResourceId;

		HFONT	controlFont,
			labelFont,
			titleFont;

		double dpiScale;

		std::shared_ptr<directXAdapter> factory;
		HWND tooltip;

	public:

		comm_bus_interface* bus;

		directApplicationWin32(comm_bus_interface* _bus, std::shared_ptr<directXAdapter>  _factory);
		virtual ~directApplicationWin32();

		ccolor backgroundColor;

		HWND getMainWindow() { return hwndRoot; }
		HWND getTooltipWindow() { return tooltip; }
		HWND createWindow(DWORD window_id, LPCTSTR		lpClassName, LPCTSTR		lpWindowName, DWORD       dwStyle, rectangle bounds, LPVOID		lpParam, HFONT		font);
		void destroyWindow(HWND hwnd);

		std::weak_ptr<direct2dChildWindow> createDirect2Window(DWORD control_id, rectangle bounds);

		virtual bool isDialogMessage(HWND hwnd, LPMSG msg);
		virtual bool runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController);
		virtual bool runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController);
		virtual bool checkBackgroundComplete(MSG *_msg);

		virtual void setController(std::shared_ptr<controller> _newCurrentController);

		HFONT createFontDips(HWND target, std::string  _fontName, double fontSize, bool bold, bool italic);
		HFONT createFontPixels(std::string _fontName, double fontSize, bool bold, bool italic);
		HFONT createFontIndirect(LOGFONTA _font, std::string _fontName, double fontSize, bool bold, bool italic);

		// general
		virtual void redraw();
		virtual void setVisible(int controlId, bool visible);
		virtual void setEnable(int controlId, bool enabled);
		virtual void setFocus(int ddlControlId);
		virtual void killFocus(int ddlControlId);

		virtual rectangle getWindowClientPos();
		virtual rectangle getWindowPos(int ddlControlId);
		virtual void setWindowPos(int ddlControlId, rectangle rect);
		virtual void setMinimumWindowSize(point size);

		double toDipsFromPixels(double r);
		virtual double toPixelsFromDips(double r);
		virtual rectangle toPixelsFromDips(const rectangle& r);
		virtual rectangle toDipsFromPixels(const rectangle& r);

		// icon control
		virtual void setPictureIcon(int controlId, dtoIconId iconId);
		virtual void setButtonIcon(int controlId, dtoIconId iconId);

		//text
		virtual void setEditText(int textControlId, const std::string& _string);
		virtual void setEditText(int textControlId, const char* _string);
		virtual std::string getEditText(int textControlId);

		// drop downs
		virtual void clearComboItems(int ddlControlId);
		virtual void addComboItem(int ddlControlId, std::string& _src, int _data);
		virtual void addComboItem(int ddlControlId, const char* _text, int _data);
		virtual std::string getComboSelectedText(int ddlControlId);
		virtual int getComboSelectedIndex(int ddlControlId);
		virtual int getComboSelectedValue(int ddlControlId);
		virtual void setComboSelectedIndex(int ddlControlId, int index);
		virtual void setComboSelectedText(int ddlControlId, std::string& _text);
		virtual void setComboSelectedText(int ddlControlId, const char* _text);
		virtual void setComboSelectedValue(int ddlControlId, int value);
		virtual void addFoldersToCombo(int ddlControlId, const char* _path);
		virtual void addPicturesFoldersToCombo(int ddlControlId);

		// ListBox
		virtual std::string getListSelectedText(int ddlControlId);
		virtual int getListSelectedValue(int ddlControlId);
		virtual int getListSelectedIndex(int ddlControlId);
		virtual void setListSelectedIndex(int ddlControlId, int index);
		virtual void setListSelectedText(int ddlControlId, std::string& _text);
		virtual void setListSelectedText(int ddlControlId, const char* _text);
		virtual void setListSelectedValue(int ddlControlId, int value);
		virtual void clearListItems(int ddlControlId);
		virtual void addListItem(int ddlControlId, std::string& _text, int _data);
		virtual void addListItem(int ddlControlId, const char* _text, int _data);
		virtual void addFoldersToList(int ddlControlId, const char* _path);
		virtual void addPicturesFoldersToList(int ddlControlId);

		// buttons
		virtual void setButtonChecked(int controlId, bool enabled);
		virtual bool getButtonChecked(int controlId);

		// list view
		virtual void clearListView(int ddlControlId);
		virtual void addListViewItem(int ddlControlId, std::string& _text, LPARAM _data);
		virtual void addListViewItem(int ddlControlId, const char* _text, LPARAM _data);
		virtual void addListViewColumn(int ddlControlId, int column_id, char* _text, int _width, visual_alignment _alignment);
		virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<char*>& _items);

		virtual std::string getListViewSelectedText(int ddlControlId);
		virtual int getListViewSelectedIndex(int ddlControlId);
		virtual LPARAM getListViewSelectedValue(int ddlControlId);
		virtual std::list<std::string> getListViewSelectedTexts(int ddlControlId);
		virtual std::list<int> getListViewSelectedIndexes(int ddlControlId);
		virtual std::list<LPARAM> getListViewSelectedValues(int ddlControlId);

		virtual void setListViewSelectedIndex(int ddlControlId, int indexId);
		virtual void setListViewSelectedText(int ddlControlId, std::string& _text);
		virtual void setListViewSelectedText(int ddlControlId, const char* _text);
		virtual void setListViewSelectedValue(int ddlControlId, LPARAM value);
		virtual void clearListViewSelection(int ddlControlId);

		// scroller
		virtual void setScrollHeight(int ddlControlId, int height);
		virtual void setScrollWidth(int ddlControlId, int width);
		virtual point getScrollPos(int ddlControlId);
		virtual point getScrollTrackPos(int ddlControlId);
		virtual point getScrollRange(int ddlControlId);
		virtual void setScrollPos(int ddlControlId, point pt);

		// spinner
		virtual void setSpinRange(int ddlControlId, int lo, int high);
		virtual void setSpinPos(int ddlControlId, int pos);

		// Handy
		virtual void setRedraw(int ddlControlId, bool pos);
		virtual void redraw(int ddlControlId, bool pos);

		// Simple
		virtual void setSysLinkText(int ddlControlId, const char* _text);
		std::vector<std::string> readInternet(const char* _domain, const char* _path);

		// utility
		char* getLastChar(char* _str);
		bool getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension);

		virtual void setColorCapture(int _iconResourceId);

		std::string getUserName();

	};

	void EnableGuiStdOuts();

	directApplicationWin32* directApplicationWin32::current;

	directApplicationWin32::directApplicationWin32(comm_bus_interface* _bus, std::shared_ptr<directXAdapter> _factory) : 
		factory(_factory), 
		colorCapture(false), 
		bus(_bus)
	{
		current = this;
		controlFont = nullptr;
		labelFont = nullptr,
		titleFont = nullptr;
		dpiScale = 1.0;

		backgroundColor.a = 1.0;
		backgroundColor.r = 1.0;
		backgroundColor.g = 1.0;
		backgroundColor.b = 1.0;

		hinstance = nullptr;
		hwndRoot = nullptr;

		performanceFrequency = 0;
		startCounter = lastCounter = 0;
		minimumWindowSize = { 500, 300 };

		colorCapture = false;
		iconResourceId = 0;

		dpiScale = 0;

		tooltip = nullptr;

	}

	directApplicationWin32::~directApplicationWin32()
	{
		global_job_queue->shutDown();
	}

	void directApplicationWin32::redraw()
	{
		static int counter = 0;
		static int sign = 1;
		static int pos = 0;

		if (currentController)
		{


			bool failedDevice = false;

			// and here, we compose those surfaces onto our screen.

			auto winroot = factory->getWindow(hwndRoot).lock();

			if (!winroot)
				return;

			winroot->beginDraw(failedDevice);

			if (!failedDevice)
			{
				auto wins = winroot->getChildren();
				auto& ctx = winroot->getContext();
				auto dc = ctx.getDeviceContext();

				// here, we tell the children to draw on their own surfaces...
				// and then, draw on this one.
				currentController->drawFrame(ctx);

				auto wbounds = winroot->getBoundsDips();

				pos += sign;
				double boxw = wbounds.w / 4;
				double boxh = wbounds.h - 150;

				if (pos < 0)
				{
					sign = 1;
				}
				else if (pos > (wbounds.w - boxw))
				{
					sign = -1;
				}

/*				D2D1_RECT_F dest;
				dest.left = pos;
				dest.top = (wbounds.h - boxh) / 2;
				dest.right = pos + boxw;
				dest.bottom = dest.top + boxh;

				ID2D1SolidColorBrush* brush = nullptr;
				D2D1_COLOR_F brushColor = {};
				brushColor.a = 1.0;
				brushColor.b = 1.0;

				dc->CreateSolidColorBrush(brushColor, &brush);
				if (brush) {
					dc->DrawRectangle(&dest, brush, 4, nullptr);
					brush->Release();
				}
				*/
				winroot->endDraw(failedDevice);
			}

		failed_check:

			if (failedDevice) {
				factory->clearWindows();
				factory->refresh();
			}
		}
	}

	HWND directApplicationWin32::createWindow(
		DWORD window_id,
		LPCTSTR		lpClassName,
		LPCTSTR		lpWindowName,
		DWORD       dwStyle,
		rectangle bounds,
		LPVOID		lpParam,
		HFONT		font
	)
	{

		HWND hwnd = nullptr;
		bool created_something = false;

		bounds.x /= dpiScale;
		bounds.y /= dpiScale;
		bounds.w /= dpiScale;
		bounds.h /= dpiScale;

		dwStyle |= WS_CLIPSIBLINGS;

		hwnd = CreateWindow(lpClassName, lpWindowName, dwStyle, bounds.x, bounds.y, bounds.w, bounds.h, directApplicationWin32::hwndRoot, (HMENU)window_id, hinstance, lpParam);
		if (font)
		{
			SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
		}

		return hwnd;
	}

	void directApplicationWin32::destroyWindow(HWND hwnd)
	{
		::DestroyWindow(hwnd);
	}

	HFONT directApplicationWin32::createFontDips(HWND target, std::string _fontName, double fontSize, bool bold, bool italic)
	{
		HFONT hfont = nullptr;

		double ifontSize = fontSize / dpiScale;
		istring<2048> fontList = _fontName;

		int state = 0;
		char* fontExtractedName = fontList.next_token(',', state);

		while (fontExtractedName && !hfont)
		{
			hfont = CreateFont(-ifontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontExtractedName);
			fontExtractedName = fontList.next_token(',', state);
		}
		return hfont;
	}

	HFONT directApplicationWin32::createFontPixels(std::string  _fontName, double fontSizePixels, bool bold, bool italic)
	{
		HFONT hfont = nullptr;

		double ifontSize = fontSizePixels;
		istring<2048> fontList = _fontName;

		int state = 0;
		char* fontExtractedName = fontList.next_token(',', state);

		while (fontExtractedName && !hfont)
		{
			hfont = CreateFont(ifontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontExtractedName);
			fontExtractedName = fontList.next_token(',', state);
		}
		return hfont;
	}

	HFONT directApplicationWin32::createFontIndirect(LOGFONTA srcFont, std::string  _fontName, double fontSizePixels, bool bold, bool italic)
	{
		HFONT hfont = nullptr;

		double ifontSize = fontSizePixels;
		istring<2048> fontList = _fontName;

		int state = 0;
		char* fontExtractedName = fontList.next_token(',', state);

		while (fontExtractedName && !hfont)
		{
			strcpy_s(srcFont.lfFaceName, fontExtractedName);
			srcFont.lfWeight = bold ? FW_BOLD : FW_NORMAL;
			srcFont.lfItalic = italic;
			fontExtractedName = fontList.next_token(',', state);

			hfont = ::CreateFontIndirect(&srcFont);
		}
		return hfont;
	}

	std::weak_ptr<direct2dChildWindow> directApplicationWin32::createDirect2Window(DWORD control_id, rectangle bounds)
	{
		std::weak_ptr<direct2dChildWindow> cx;
		if (bounds.w < 1 || bounds.h < 1)
			return cx;

		auto dpi = GetDpiForWindow(hwndRoot);

		auto win = factory->getWindow(hwndRoot).lock();

		if (win) 
		{
			auto child = win->createChild(control_id, bounds.x, bounds.y, bounds.w, bounds.h);
			return child;
		}
		else 
		{
			std::cout << "could not create direct2d window because root window not found" << std::endl;
			std::weak_ptr<direct2dChildWindow> child;
			return child;
		}
	}

	LRESULT CALLBACK directApplicationWin32::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return current->windowProcHandler(hwnd, message, wParam, lParam);
	}

	LRESULT CALLBACK directApplicationWin32::controlWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return current->controlWindowProcHandler(hwnd, message, wParam, lParam);
	}

	bool directApplicationWin32::isDialogMessage(HWND hwnd, LPMSG msg)
	{
		bool navigationKey = false;

		if (currentController) {
			navigationKey = currentController->navigationKey(msg->wParam );
			if (navigationKey)
				return !navigationKey;
		}

		navigationKey = ::IsDialogMessage(hwnd, msg);

		return navigationKey;
	}

	LRESULT directApplicationWin32::controlWindowProcHandler(HWND hwndchild, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto &pfactory = factory;

		std::weak_ptr<direct2dWindow> current_window;
		std::shared_ptr< direct2dWindow> pcurrent_window = nullptr;

		HWND hwnd = ::GetParent(hwndchild);
		int ctrlId = ::GetDlgCtrlID(hwndchild);

		if (pfactory) {
			current_window = pfactory->getWindow(hwnd);
			pcurrent_window = current_window.lock();
		}

		switch (message)
		{
		case WM_CREATE:
			{
			}
			break;
		case WM_PAINT:
			{
				ValidateRect(hwnd, nullptr);
				redraw();
				return 0;
			}
		case WM_ERASEBKGND:
			{
				RECT rect, rect2;
				HDC eraseDc = (HDC)wParam;
				HBRUSH hbrBkgnd = (HBRUSH)::GetStockObject(WHITE_BRUSH);
				::GetClientRect(hwndchild, &rect);
				::FillRect((HDC)wParam, &rect, hbrBkgnd);
				return 0;
			}
			break;
		case WM_SETFOCUS:
			if (currentController)
			{
				currentController->setFocus(ctrlId);
			}
			break;
		case WM_KILLFOCUS:
			if (currentController)
			{
				currentController->killFocus(ctrlId);
			}
			break;

		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_MOUSEMOVE:
			if (currentController)
			{
				POINT p;
				bool lbutton = true;
				if (GetCursorPos(&p))
				{
					ScreenToClient(hwnd, &p);
					point ptxo;
					ptxo.x = p.x * dpiScale;
					ptxo.y = p.y * dpiScale;

					if (pcurrent_window) {
						switch (message) {
						case WM_LBUTTONUP:
							setFocus(ctrlId);
							currentController->mouseLeftUp(&ptxo);
							break;
						case WM_LBUTTONDOWN:
							currentController->mouseLeftDown(&ptxo);
							break;
						case WM_RBUTTONUP:
							currentController->mouseRightUp(&ptxo);
							break;
						case WM_RBUTTONDOWN:
							currentController->mouseRightDown(&ptxo);
							break;
						case WM_MOUSEMOVE:
							currentController->mouseMove(&ptxo);
							break;
						}
					}
				}
			}
			break;
		case WM_CHAR:
			if (currentController)
			{
				if (pcurrent_window) {
					currentController->keyPress(ctrlId, wParam);
					return 0;
				}
			}
			break;
		case WM_KEYDOWN:
			if (currentController)
			{
				if (pcurrent_window) {
					currentController->keyDown(ctrlId, wParam);
					return 0;
				}
			}
			break;
		case WM_KEYUP:
			if (currentController)
			{
				if (pcurrent_window) {
					currentController->keyUp(ctrlId, wParam);
					return 0;
				}
			}
			break;
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	LRESULT directApplicationWin32::windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		bool found = false;
		point temp_pt;
		static HBRUSH hbrBkgnd2 = NULL;
		char className[256];
		HRESULT hr;
		const int BORDERWIDTH = 8;

		auto &pfactory = factory;

		std::weak_ptr<direct2dWindow> current_window;
		std::shared_ptr< direct2dWindow> pcurrent_window = nullptr;

		if (pfactory) {
			current_window = pfactory->getWindow(hwnd);
			pcurrent_window = current_window.lock();
		}

		try
		{

			switch (message)
			{
			case WM_CREATE:
			{
				hwndRoot = hwnd;

				RECT rcClient;
				GetWindowRect(hwnd, &rcClient);
				SetWindowPos(hwnd, NULL, rcClient.left, rcClient.top, abs(rcClient.right - rcClient.left), abs(rcClient.bottom - rcClient.top), SWP_FRAMECHANGED);
				SetTimer(hwnd,1,500,nullptr);

				dpiScale = 96.0 / GetDpiForWindow(hwnd);
				if (currentController) {
					pfactory->createD2dWindow(hwnd, backgroundColor);

					tooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
						WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
						CW_USEDEFAULT, CW_USEDEFAULT,
						CW_USEDEFAULT, CW_USEDEFAULT,
						hwndRoot, NULL,
						hinstance, NULL);

					currentController->onHostCreated();
				}
			}
			break;
			case WM_TIMER:
				if (bus) {
					bus->poll();
				}
				break;
			case WM_NCCALCSIZE:
			{
				if (wParam)
				{
					NCCALCSIZE_PARAMS* Params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
					Params->rgrc[0].bottom += BORDERWIDTH; // rgrc[0] is what makes this work, don't know what others (rgrc[1], rgrc[2]) do, but why not change them all?
					Params->rgrc[0].right += BORDERWIDTH;
					Params->rgrc[1].bottom += BORDERWIDTH;
					Params->rgrc[1].right += BORDERWIDTH;
					Params->rgrc[2].bottom += BORDERWIDTH;
					Params->rgrc[2].right += BORDERWIDTH;
					return 0;
				}
				return DefWindowProc(hwnd, message, wParam, lParam);
			}
			case WM_NCHITTEST:
			{
				RECT WindowRect;
				int x, y;

				GetWindowRect(hwnd, &WindowRect);
				x = GET_X_LPARAM(lParam) - WindowRect.left;
				y = GET_Y_LPARAM(lParam) - WindowRect.top;
				LRESULT test_hit = HTCLIENT;

				if (x < BORDERWIDTH && y < BORDERWIDTH)
					test_hit = HTTOPLEFT;
				else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH && y < BORDERWIDTH)
					test_hit = HTTOPRIGHT;
				else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
					test_hit = HTBOTTOMRIGHT;
				else if (x < BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
					test_hit = HTBOTTOMLEFT;
				else if (x < BORDERWIDTH)
					test_hit = HTLEFT;
				else if (y < BORDERWIDTH)
					test_hit = HTTOP;
				else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH)
					test_hit = HTRIGHT;
				else if (y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
					test_hit = HTBOTTOM;
				else if (currentController) {
					point hitpoint;
					hitpoint = { x, y };
					hitpoint.x = x * dpiScale;
					hitpoint.y = y * dpiScale;
					test_hit = currentController->ncHitTest(&hitpoint);
				}
				else
					test_hit = HTCLIENT;

				//std::cout << x << " " << y << " " << test_hit << std::endl;

				return test_hit;
			}
			break;
			case WM_DESTROY:
				pfactory->closeWindow(hwnd);
				PostQuitMessage(0);
				return 0;
			case WM_COMMAND:
				if (currentController)
				{
					UINT controlId = LOWORD(wParam);
					UINT notificationCode = HIWORD(wParam);
					HWND controlWindow = (HWND)lParam;
					switch (notificationCode) {
						case BN_CLICKED: // button or menu
							currentController->onCommand(controlId);
							break;
						case EN_UPDATE:
							currentController->onTextChanged(controlId);
							break;
						case LBN_SELCHANGE:
						{
							char window_class[500];
							if (::RealGetWindowClass(controlWindow, window_class, sizeof(window_class) - 1)) {
								if (
									(strcmp(WC_COMBOBOX, window_class) == 0) ||
									(strcmp(WC_COMBOBOXEX, window_class) == 0)
									) {
									currentController->onDropDownChanged(controlId);
								}
								else
								{
									currentController->onListBoxChanged(controlId);
								}
							}
							break;
						}
					}
				}
				break;
			case WM_DPICHANGED:
				dpiScale = 96.0 / GetDpiForWindow(hwnd);
				if (currentController)
				{
					RECT* const prcNewWindow = (RECT*)lParam;
					SetWindowPos(hwnd,
						NULL,
						prcNewWindow->left,
						prcNewWindow->top,
						prcNewWindow->right - prcNewWindow->left,
						prcNewWindow->bottom - prcNewWindow->top,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
				break;
			case WM_NOTIFY:
				if (currentController)
				{
					//LVN_ITEMACTIVATE
					LPNMHDR lpnm = (LPNMHDR)lParam;
					switch (lpnm->code) {
					case UDN_DELTAPOS:
					{
						auto lpnmud = (LPNMUPDOWN)lParam;
						currentController->onSpin(lpnm->idFrom, lpnmud->iPos + lpnmud->iDelta);
						return 0;
					}
					break;
					case LVN_ITEMCHANGED:
					{
						auto lpmnlv = (LPNMLISTVIEW)lParam;
						if (lpmnlv->uNewState & LVIS_SELECTED)
							currentController->onListViewChanged(lpnm->idFrom);
					}
					break;
					case EN_CHANGE:
						currentController->onTextChanged(lpnm->idFrom);
						break;
					case NM_CLICK:
					{

						::GetClassName(lpnm->hwndFrom, className, sizeof(className) - 1);
						if (strcmp(className, "SysLink") == 0) {
							auto plink = (PNMLINK)lParam;
							auto r = ::ShellExecuteW(NULL, L"open", plink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
						}
					}
					break;

					/*
								case NM_CUSTOMDRAW:
								{
									LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
									switch (lplvcd->nmcd.dwDrawStage) {
									case CDDS_PREPAINT:
										return CDRF_NOTIFYITEMDRAW;
									case CDDS_ITEMPREPAINT:
										LVITEM lvitem;
										char buff[16384];
										ZeroMemory(&lvitem, sizeof(lvitem));
										lvitem.iItem = lplvcd->nmcd.dwItemSpec;
										lvitem.stateMask = LVIS_SELECTED;
										lvitem.mask = LVIF_STATE | LVIF_TEXT;
										lvitem.cchTextMax = sizeof(buff) - 1;
										lvitem.pszText = buff;
										HWND control = ::GetDlgItem(hwndRoot, lplvcd->nmcd.hdr.idFrom);
										ListView_GetItem(control, &lvitem);

										RECT area = lplvcd->nmcd.rc;

										if (lvitem.state & LVIS_SELECTED) {
											::FillRect(lplvcd->nmcd.hdc, &area, GetSysColorBrush(COLOR_HIGHLIGHT));
											::SetTextColor(lplvcd->nmcd.hdc, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
											::DrawTextA(lplvcd->nmcd.hdc, lvitem.pszText, strlen(lvitem.pszText), &area, DT_CENTER);
										}
										else {
											::FillRect(lplvcd->nmcd.hdc, &area, GetSysColorBrush(COLOR_WINDOW));
											::SetTextColor(lplvcd->nmcd.hdc, ::GetSysColor(COLOR_WINDOWTEXT));
											::DrawTextA(lplvcd->nmcd.hdc, lvitem.pszText, strlen(lvitem.pszText), &area, DT_CENTER);
										}
										return CDRF_SKIPDEFAULT;
									}

					*/
					}
				}
				break;
			case WM_GETMINMAXINFO:
				if (minimumWindowSize.x > 0) {
					auto minmaxinfo = (LPMINMAXINFO)lParam;
					minmaxinfo->ptMinTrackSize.x = minimumWindowSize.x;
					minmaxinfo->ptMinTrackSize.y = minimumWindowSize.y;
					return 0;
				}
				break;
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLOREDIT:
			{
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				HBRUSH hbrBkgnd = (HBRUSH)::GetStockObject(WHITE_BRUSH);
				return (INT_PTR)hbrBkgnd;
			}
			break;
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORSTATIC:
			{
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				HBRUSH hbrBkgnd = (HBRUSH)::GetStockObject(WHITE_BRUSH);

				if (currentController)
				{
					DWORD id = ::GetDlgCtrlID((HWND)lParam);
					if (auto pcontroller = dynamic_cast<presentation*>(currentController.get()))
					{
						if (draw_control* pdraw = pcontroller->get_parent_for_control_by_id<draw_control>(id)) {

							auto cv = pdraw->view_style->box_fill_brush.getColor();
							SetBkColor(hdcStatic, RGB(int(cv.r * 255), int(255 * cv.g), int(255 * cv.b)));
						}
					}
				}

				return (INT_PTR)hbrBkgnd;
			}
			break;
			case WM_ERASEBKGND:
			{
				RECT rect, rect2;
				HDC eraseDc = (HDC)wParam;
				HBRUSH hbrBkgnd = (HBRUSH)::GetStockObject(WHITE_BRUSH);
				::GetClientRect(hwnd, &rect);
				::FillRect((HDC)wParam, &rect, hbrBkgnd);
				return 0;
			}
			break;

			case WM_CHAR:
				break;
			case WM_CANCELMODE:
				if (colorCapture) {
					colorCapture = false;
					::ReleaseCapture();
					::SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
				break;

			case WM_CAPTURECHANGED:
				colorCapture = false;
				break;

			case WM_KEYDOWN:
				if (currentController)
				{
					if (pcurrent_window) {
						int ctrlId = ::GetDlgCtrlID(hwnd);
						currentController->keyDown(ctrlId, wParam);
					}
				}
				break;
			case WM_KEYUP:
				if (currentController)
				{
					if (pcurrent_window) {
						int ctrlId = ::GetDlgCtrlID(hwnd);
						currentController->keyUp(ctrlId, wParam);
					}
				}
				break;
			case WM_NCLBUTTONDOWN:
			case WM_LBUTTONDOWN:
				std::cout << "left down." << std::endl;
				if (colorCapture) {
					colorCapture = false;
					::ReleaseCapture();
					::SetCursor(LoadCursor(NULL, IDC_ARROW));
					POINT p;
					if (GetCursorPos(&p))
					{
						HDC hdc = ::GetDC(NULL);
						if (hdc) {
							COLORREF cr = ::GetPixel(hdc, p.x, p.y);
							ccolor pickedColor;
							pickedColor.r = GetRValue(cr) / 255.0;
							pickedColor.g = GetGValue(cr) / 255.0;
							pickedColor.b = GetBValue(cr) / 255.0;
							temp_pt.x = p.x;
							temp_pt.y = p.y;
							if (currentController) {
								currentController->pointSelected(&temp_pt, &pickedColor);
							}
						}
					}
				}
				else if (currentController)
				{
					POINT p;
					bool lbutton = true;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						::SetFocus(hwnd);
						point ptxo;
						ptxo.x = p.x * dpiScale;
						ptxo.y = p.y * dpiScale;
						if (pcurrent_window) {
							currentController->mouseLeftDown(&ptxo);
						}
					}
				}
				break;

			case WM_NCLBUTTONUP:
			case WM_LBUTTONUP:
				std::cout << "left up." << std::endl;
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						point ptxo;
						ptxo.x = p.x * dpiScale;
						ptxo.y = p.y * dpiScale;
						if (pcurrent_window) {
							currentController->mouseLeftUp(&ptxo);
						}
					}
				}
				break;
			case WM_NCRBUTTONDOWN:
			case WM_RBUTTONDOWN:
			{
				if (colorCapture) {
					colorCapture = false;
					::ReleaseCapture();
					::SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						point ptxo;
						ptxo.x = p.x * dpiScale;
						ptxo.y = p.y * dpiScale;
						if (pcurrent_window) {
							currentController->mouseRightDown(&ptxo);
						}
					}
				}
			}
			break;
			case WM_NCRBUTTONUP:
			case WM_RBUTTONUP:
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						point ptxo;
						ptxo.x = p.x * dpiScale;
						ptxo.y = p.y * dpiScale;
						if (pcurrent_window) {
							currentController->mouseRightUp(&ptxo);
						}
					}
				}
				break;
			case WM_MOUSEMOVE:
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						point ptxo;
						ptxo.x = p.x * dpiScale;
						ptxo.y = p.y * dpiScale;
						currentController->mouseMove(&ptxo);
					}
				}
				break;
			case WM_PAINT:
			{
				ValidateRect(hwnd, nullptr);
				/*				PAINTSTRUCT ps;
								BeginPaint(hwnd, &ps);
								EndPaint(hwnd, &ps); */
				redraw();
				return 0;
			}
			case WM_SIZE:
			{
				rectangle rect;
				rect.x = 0;
				rect.y = 0;
				rect.w = LOWORD(lParam);
				rect.h = HIWORD(lParam);
				dpiScale = 96.0 / GetDpiForWindow(hwnd);
				if (pfactory) {
					auto wwin = pfactory->getWindow(hwnd);
					if (auto win = wwin.lock()) {
						win->resize(rect.w, rect.h);
						if (currentController) {
#if TRACE_SIZE
							std::cout << " w " << rect.w << "h " << rect.h << std::endl;

#endif
							rect.w *= dpiScale;
							rect.h *= dpiScale;
							currentController->onResize(rect, dpiScale);
						}
					}
				}
			}
			break;
			case WM_DEVICECHANGE:
				if (currentController) {
					currentController->hardwareChanged();
				}
				break;
			case WM_CORONA_RESET:
			{
				RECT rect_size;
				::GetWindowRect(hwnd, &rect_size);
				rectangle rect;
				rect.x = 0;
				rect.y = 0;
				rect.w = std::abs(rect_size.right - rect_size.left);
				rect.h = std::abs(rect_size.bottom - rect_size.top);
				dpiScale = 96.0 / GetDpiForWindow(hwnd);
				if (pfactory) {
					auto wwin = pfactory->getWindow(hwnd);
					if (auto win = wwin.lock()) {
						win->resize(rect.w, rect.h);
						if (currentController) {
#if TRACE_SIZE
							std::cout << " w " << rect.w << "h " << rect.h << std::endl;

#endif
							rect.w *= dpiScale;
							rect.h *= dpiScale;
							currentController->onResize(rect, dpiScale);
						}
					}
				}
			}
			break;
			case DM_GETDEFID:
			{
				if (currentController)
					return MAKELONG(currentController->getDefaultButtonId(), DC_HASDEFID);

			}
			break;

			}
		}
		catch (std::exception exc)
		{
			std::cout << exc.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "general exception" << std::endl;
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void directApplicationWin32::setController(std::shared_ptr<controller> _newCurrentController)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
		pressedKeys.clear();
		currentController = _newCurrentController;
		::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);
	}

	bool directApplicationWin32::runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController)
	{
		if (!_firstController)
			return false;

		auto ptr = weak_from_this();

		WNDCLASSA wcMain, wcControl;
		MSG msg;
		DWORD dwStyle, dwExStyle;

		hinstance = _hinstance;

		// register the control for the direct2d WINDOW - THIS is the main window this time.

		wcMain.style = CS_OWNDC;
		wcMain.lpfnWndProc = &directApplicationWin32::windowProc;
		wcMain.cbClsExtra = 0;
		wcMain.cbWndExtra = DLGWINDOWEXTRA;
		wcMain.hInstance = hinstance;
		wcMain.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(_iconId));
		wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcMain.hbrBackground = NULL;
		wcMain.lpszMenuName = NULL;
		wcMain.lpszClassName = "Corona2dBase";
		if (!RegisterClass(&wcMain)) {
			::MessageBox(NULL, "Could not start because the  class could not be registered", "Couldn't Start", MB_ICONERROR);
			return 0;
		}

		wcControl.style = CS_HREDRAW | CS_VREDRAW;
		wcControl.lpfnWndProc = &directApplicationWin32::controlWindowProc;
		wcControl.cbClsExtra = 0;
		wcControl.cbWndExtra = DLGWINDOWEXTRA;
		wcControl.hInstance = hinstance;
		wcControl.hIcon = NULL;
		wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcControl.hbrBackground = NULL;
		wcControl.lpszMenuName = NULL;
		wcControl.lpszClassName = "Corona2dControl";
		if (!RegisterClass(&wcControl)) {
			::MessageBox(NULL, "Could not start because the  class could not be registered", "Couldn't Start", MB_ICONERROR);
			return 0;
		}

		if (_fullScreen) {
			dwStyle = WS_POPUP | WS_MAXIMIZE;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
			ShowCursor(FALSE);
		}
		else {
			dwStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		}

		hwndRoot = NULL;

		setController(_firstController);

		hwndRoot = CreateWindowEx(dwExStyle,
			wcMain.lpszClassName, _title,
			dwStyle | WS_CLIPSIBLINGS | WS_TABSTOP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hinstance, NULL);

		if (!hwndRoot) {
			MessageBox(NULL, "Could not start because of a problem creating the main window.", _title, MB_OK);
			return FALSE;
		}

		::ShowWindow(hwndRoot, SW_SHOWNORMAL);
		::UpdateWindow(hwndRoot);

		::QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
		::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);

		while (true) {
			if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (!::GetMessage(&msg, NULL, 0, 0))
					break;
				else if (checkBackgroundComplete(&msg))
				{
					;
				}
				else if (!isDialogMessage(hwndRoot, &msg)) {
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
			else {
				__int64 counter;
				::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
				double elapsedSeconds = (double)(counter - lastCounter) / (double)performanceFrequency;
				double totalSeconds = (double)(counter - startCounter) / (double)performanceFrequency;
				lastCounter = counter;
				if (currentController->update(elapsedSeconds, totalSeconds)) {
					redraw();
				}
			}
		}

		return true;
	}

	bool directApplicationWin32::runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController)
	{
		if (!_firstController)
			return false;

		auto ptr = weak_from_this();

		WNDCLASSA wcMain, wcControl;
		MSG msg;
		DWORD dwStyle, dwExStyle;

		hinstance = _hinstance;

		// register the class for the main application window

		wcMain.style = CS_HREDRAW | CS_VREDRAW;
		wcMain.lpfnWndProc = &directApplicationWin32::windowProc;
		wcMain.cbClsExtra = 0;
		wcMain.cbWndExtra = DLGWINDOWEXTRA;
		wcMain.hInstance = hinstance;
		wcMain.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(_iconId));
		wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcMain.hbrBackground = NULL;
		wcMain.lpszMenuName = NULL;
		wcMain.lpszClassName = "Corona2dBase";
		if (!RegisterClass(&wcMain)) {
			::MessageBox(NULL, "Could not start because the main window class could not be registered", "Couldn't Start", MB_ICONERROR);
			return 0;
		}

		wcControl.style = CS_HREDRAW | CS_VREDRAW;
		wcControl.lpfnWndProc = &directApplicationWin32::controlWindowProc;
		wcControl.cbClsExtra = 0;
		wcControl.cbWndExtra = 0;
		wcControl.hInstance = hinstance;
		wcControl.hIcon = NULL;
		wcControl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcControl.hbrBackground = NULL;
		wcControl.lpszMenuName = NULL;
		wcControl.lpszClassName = "Corona2dControl";
		if (!RegisterClass(&wcControl)) {
			::MessageBox(NULL, "Could not start because the  class could not be registered", "Couldn't Start", MB_ICONERROR);
			return 0;
		}

		hwndRoot = NULL;

		RECT desktop_rect = {};
		::GetWindowRect(GetDesktopWindow(), &desktop_rect);
		int desktop_width, desktop_height;
		int window_width, window_height;
		int window_x, window_y;

		if (_fullScreen) {
			desktop_width = desktop_rect.right - desktop_rect.left;
			desktop_height = desktop_rect.bottom - desktop_rect.top;
			window_width = desktop_width;
			window_height = desktop_height;
			window_x = 0;
			window_y = 0;

			dwStyle = WS_POPUP | WS_MAXIMIZE;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		}
		else {
			int desktop_width, desktop_height;
			desktop_width = desktop_rect.right - desktop_rect.left;
			desktop_height = desktop_rect.bottom - desktop_rect.top;
			window_width = desktop_width * .75;
			window_height = desktop_height * .75;
			window_x = (desktop_width - window_width) / 2;
			window_y = (desktop_height - window_height) / 2;

			dwStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		}

		setController(_firstController);


		hwndRoot = CreateWindowEx(dwExStyle,
			wcMain.lpszClassName, _title,
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP,
			window_x, window_y, window_width, window_height,
			NULL, NULL, hinstance, NULL);

		if (!hwndRoot) {
			MessageBox(NULL, "Could not start because of a problem creating the main window.", _title, MB_OK);
			return FALSE;
		}

		::ShowWindow(hwndRoot, SW_SHOWNORMAL);
		::UpdateWindow(hwndRoot);

		::QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
		::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);

		while (true) {
			if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {								
				if (!::GetMessage(&msg, NULL, 0, 0))
					break;
				else if (checkBackgroundComplete(&msg))
				{
					;
				}
				else if (!isDialogMessage(hwndRoot, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
			else {
				__int64 counter;
				::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
				double elapsedSeconds = (double)(counter - lastCounter) / (double)performanceFrequency;
				double totalSeconds = (double)(counter - startCounter) / (double)performanceFrequency;
				lastCounter = counter;
				if (currentController->update(elapsedSeconds, totalSeconds)) {
					redraw();
				}
			}
		}

		return true;
	}

	void directApplicationWin32::setPictureIcon(int controlId, dtoIconId iconId)
	{
		SHSTOCKICONINFO iconInfo;
		ZeroMemory(&iconInfo, sizeof(iconInfo));
		iconInfo.cbSize = sizeof(iconInfo);
		HWND control = ::GetDlgItem(hwndRoot, controlId);
		HRESULT hresult = ::SHGetStockIconInfo((SHSTOCKICONID)iconId, SHGSI_ICON, &iconInfo);
		if (hresult == S_OK)
			::SendMessage(control, STM_SETICON, (WPARAM)iconInfo.hIcon, NULL);
	}

	void directApplicationWin32::setButtonIcon(int controlId, dtoIconId iconId)
	{
		SHSTOCKICONINFO iconInfo;
		ZeroMemory(&iconInfo, sizeof(iconInfo));
		iconInfo.cbSize = sizeof(iconInfo);

		HWND control = ::GetDlgItem(hwndRoot, controlId);
		HRESULT hresult = ::SHGetStockIconInfo((SHSTOCKICONID)iconId, SHGSI_ICON | SHGSI_SMALLICON, &iconInfo);
		if (hresult == S_OK)
			::SendMessage(control, BM_SETIMAGE, IMAGE_ICON, (WPARAM)iconInfo.hIcon);
	}

	void directApplicationWin32::setVisible(int controlId, bool visible)
	{
		HWND control = ::GetDlgItem(hwndRoot, controlId);
		::ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
	}

	void directApplicationWin32::setEnable(int controlId, bool enabled)
	{
		HWND control = ::GetDlgItem(hwndRoot, controlId);
		::EnableWindow(control, enabled);
	}

	void directApplicationWin32::setFocus(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SetFocus(control);
//		::PostMessage(hwndRoot, WM_NEXTDLGCTL, (WPARAM)control, TRUE);
	}

	void directApplicationWin32::killFocus(int ddlControlId)
	{
		;
	}

	void directApplicationWin32::setEditText(int textControlId, const std::string& _string)
	{
		HWND control = ::GetDlgItem(hwndRoot, textControlId);
		::SetWindowText(control, _string.c_str());
	}

	void directApplicationWin32::setEditText(int textControlId, const char* _string)
	{
		HWND control = ::GetDlgItem(hwndRoot, textControlId);
		::SetWindowText(control, _string);
	}

	std::string directApplicationWin32::getEditText(int textControlId)
	{
		std::string value = "";
		HWND control = ::GetDlgItem(hwndRoot, textControlId);
		int length = ::GetWindowTextLength(control) + 1;
		char* buffer = new char[length];
		if (buffer) {
			::GetWindowText(control, buffer, length);
			value = buffer;
			delete[] buffer;
		}
		return value;
	}

	std::string directApplicationWin32::getComboSelectedText(int ddlControlId)
	{
		std::string value = "";
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
		if (newSelection > -1) {
			int length = (int)::SendMessage(control, CB_GETLBTEXTLEN, newSelection, 0);
			char* buffer = new char[length + 16];
			if (buffer) {
				::SendMessage(control, CB_GETLBTEXT, newSelection, (LPARAM)buffer);
				value = buffer;
				delete[] buffer;
			}
		}
		return value;
	}

	int directApplicationWin32::getComboSelectedIndex(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
		return newSelection;
	}

	int directApplicationWin32::getComboSelectedValue(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
		int data = (int)::SendMessage(control, CB_GETITEMDATA, newSelection, 0);
		return data;
	}

	void directApplicationWin32::setComboSelectedIndex(int ddlControlId, int index)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, CB_SETCURSEL, index, NULL);
	}

	void directApplicationWin32::setComboSelectedText(int ddlControlId, std::string& _text)
	{
		setComboSelectedText(ddlControlId, _text.c_str());
	}

	void directApplicationWin32::setComboSelectedText(int ddlControlId, const char* _text)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int index = ::SendMessage(control, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)_text);
		::SendMessage(control, CB_SETCURSEL, index, NULL);
	}

	void directApplicationWin32::setComboSelectedValue(int ddlControlId, int value)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int count = ::SendMessage(control, CB_GETCOUNT, NULL, NULL);
		for (int i = 0; i < count; i++) {
			int data = (int)::SendMessage(control, CB_GETITEMDATA, i, 0);
			if (data == value) {
				::SendMessage(control, CB_SETCURSEL, i, NULL);
				break;
			}
		}
	}

	void directApplicationWin32::clearComboItems(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, CB_RESETCONTENT, NULL, NULL);
	}

	void directApplicationWin32::addComboItem(int ddlControlId, std::string& _text, int _data)
	{
		addComboItem(ddlControlId, _text.c_str(), _data);
	}

	void directApplicationWin32::addComboItem(int ddlControlId, const char* _text, int _data)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newItemIndex = (int)::SendMessage(control, CB_ADDSTRING, NULL, (LPARAM)_text);
		if (newItemIndex != CB_ERR) {
			int err = ::SendMessage(control, CB_SETITEMDATA, newItemIndex, (LPARAM)_data);
		}
	}

	// listbox

	std::string directApplicationWin32::getListSelectedText(int ddlControlId)
	{
		std::string value = "";
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, LB_GETCURSEL, 0, 0);
		if (newSelection > -1) {
			int length = (int)::SendMessage(control, LB_GETTEXTLEN, newSelection, 0);
			char* buffer = new char[length + 16];
			if (buffer) {
				::SendMessage(control, LB_GETTEXT, newSelection, (LPARAM)buffer);
				value = buffer;
				delete[] buffer;
			}
		}
		return value;
	}

	int directApplicationWin32::getListSelectedIndex(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, LB_GETCURSEL, 0, 0);
		return newSelection;
	}

	int directApplicationWin32::getListSelectedValue(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newSelection = (int)::SendMessage(control, LB_GETCURSEL, 0, 0);
		int data = (int)::SendMessage(control, LB_GETITEMDATA, newSelection, 0);
		return data;
	}

	void directApplicationWin32::setListSelectedIndex(int ddlControlId, int index)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, LB_SETCURSEL, index, NULL);
	}

	void directApplicationWin32::setListSelectedText(int ddlControlId, std::string& _text)
	{
		setListSelectedText(ddlControlId, _text.c_str());
	}

	void directApplicationWin32::setListSelectedText(int ddlControlId, const char* _text)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int index = ::SendMessage(control, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)_text);
		::SendMessage(control, LB_SETCURSEL, index, NULL);
	}

	void directApplicationWin32::setListSelectedValue(int ddlControlId, int value)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int count = ::SendMessage(control, LB_GETCOUNT, NULL, NULL);
		for (int i = 0; i < count; i++) {
			int data = (int)::SendMessage(control, LB_GETITEMDATA, i, 0);
			if (data == value) {
				::SendMessage(control, LB_SETCURSEL, i, NULL);
				break;
			}
		}
	}

	void directApplicationWin32::clearListItems(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, LB_RESETCONTENT, NULL, NULL);
	}

	void directApplicationWin32::addListItem(int ddlControlId, std::string& _text, int _data)
	{
		addListItem(ddlControlId, _text.c_str(), _data);
	}

	void directApplicationWin32::addListItem(int ddlControlId, const char* _text, int _data)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int newItemIndex = (int)::SendMessage(control, LB_INSERTSTRING, (WPARAM)(-1), (LPARAM)_text);
		if (newItemIndex != LB_ERR) {
			int err = ::SendMessage(control, LB_SETITEMDATA, newItemIndex, (LPARAM)_data);
		}
	}

	void directApplicationWin32::addFoldersToList(int ddlControlId, const char* _path)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		char searchPath[MAX_PATH + 8];
		strncpy_s(searchPath, _path, MAX_PATH);
		searchPath[MAX_PATH] = 0;

		char* lastChar = getLastChar(searchPath);
		if (!lastChar)
			return;

		if (*lastChar == '\\') {
			lastChar++;
			*lastChar = '*';
			lastChar++;
			*lastChar = 0;
		}
		else if (*lastChar != '*') {
			lastChar++;
			*lastChar = '\\';
			lastChar++;
			*lastChar = '*';
			lastChar++;
			*lastChar = 0;
		}

		WIN32_FIND_DATAA findData;
		ZeroMemory(&findData, sizeof(findData));

		HANDLE hfind = INVALID_HANDLE_VALUE;

		hfind = ::FindFirstFile(searchPath, &findData);
		if (hfind != INVALID_HANDLE_VALUE) {
			do
			{
				// oh windows, . is really a directory, seriously!
				if (strcmp(findData.cFileName, ".") == 0)
					continue;

				if (strcmp(findData.cFileName, "..") == 0)
					continue;

				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char recurseBuff[MAX_PATH + 8];
					strncpy_s(recurseBuff, _path, MAX_PATH);
					recurseBuff[MAX_PATH] = 0;
					::PathAddBackslash(recurseBuff);
					::PathAppend(recurseBuff, findData.cFileName);
					addComboItem(ddlControlId, recurseBuff, 0);
					addFoldersToCombo(ddlControlId, recurseBuff);
				}
			} while (FindNextFile(hfind, &findData) != 0);
		}

	}

	void directApplicationWin32::addPicturesFoldersToList(int ddlControlId)
	{
		PWSTR picturePath = nullptr;
		iwstring<1 << 16> picturePathiw;
		istring<1 << 16> picturePathi;

		SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &picturePath);
		if (picturePath) {
			picturePathiw = picturePath;
			picturePathi.copy(picturePathiw.c_str_w(), 1 << 16);
			addFoldersToList(ddlControlId, picturePathi.c_str());
		}
	}

	void directApplicationWin32::addFoldersToCombo(int ddlControlId, const char* _path)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		char searchPath[MAX_PATH + 8];
		strncpy_s(searchPath, _path, MAX_PATH);
		searchPath[MAX_PATH] = 0;

		char* lastChar = getLastChar(searchPath);
		if (!lastChar)
			return;

		if (*lastChar == '\\') {
			lastChar++;
			*lastChar = '*';
			lastChar++;
			*lastChar = 0;
		}
		else if (*lastChar != '*') {
			lastChar++;
			*lastChar = '\\';
			lastChar++;
			*lastChar = '*';
			lastChar++;
			*lastChar = 0;
		}

		WIN32_FIND_DATAA findData;
		ZeroMemory(&findData, sizeof(findData));

		HANDLE hfind = INVALID_HANDLE_VALUE;

		hfind = ::FindFirstFile(searchPath, &findData);
		if (hfind != INVALID_HANDLE_VALUE) {
			do
			{
				// oh windows, . is really a directory, seriously!
				if (strcmp(findData.cFileName, ".") == 0)
					continue;

				if (strcmp(findData.cFileName, "..") == 0)
					continue;

				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char recurseBuff[MAX_PATH + 8];
					strncpy_s(recurseBuff, _path, MAX_PATH);
					recurseBuff[MAX_PATH] = 0;
					::PathAddBackslash(recurseBuff);
					::PathAppend(recurseBuff, findData.cFileName);
					addComboItem(ddlControlId, recurseBuff, 0);
					addFoldersToCombo(ddlControlId, recurseBuff);
				}
			} while (FindNextFile(hfind, &findData) != 0);
		}
	}

	void directApplicationWin32::addPicturesFoldersToCombo(int ddlControlId)
	{
		PWSTR picturePath = nullptr;
		iwstring<1 << 16> picturePathiw;
		istring<1 << 16> picturePathi;

		SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &picturePath);
		if (picturePath) {
			picturePathiw = picturePath;
			picturePathi.copy(picturePathiw.c_str_w(), 1 << 16);
			addFoldersToCombo(ddlControlId, picturePathi.c_str());
		}
	}

	void directApplicationWin32::setButtonChecked(int controlId, bool enabled)
	{
		::CheckDlgButton(hwndRoot, controlId, enabled ? BST_CHECKED : BST_UNCHECKED);
	}

	bool directApplicationWin32::getButtonChecked(int controlId)
	{
		return ::IsDlgButtonChecked(hwndRoot, controlId) == BST_CHECKED;
	}

	void directApplicationWin32::clearListView(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		ListView_DeleteAllItems(control);
		while (ListView_DeleteColumn(hwndRoot, 1));
	}

	void directApplicationWin32::addListViewItem(int ddlControlId, std::string& _text, LPARAM _data)
	{
		addListViewItem(ddlControlId, _text.c_str(), _data);
	}

	void directApplicationWin32::addListViewItem(int ddlControlId, const char* _text, LPARAM _data)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		LVITEM lvitem;
		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem = ListView_GetItemCount(control);
		lvitem.pszText = (LPSTR)_text;
		lvitem.lParam = _data;
		bool success = ::SendMessage(control, LVM_INSERTITEMA, 0, (LPARAM)&lvitem);
	}

	void directApplicationWin32::addListViewColumn(int ddlControlId,
		int column_id,
		char* _text,
		int _width,
		visual_alignment _alignment)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		LVCOLUMN lvitem;
		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvitem.iSubItem = column_id;
		lvitem.pszText = (LPSTR)_text;
		lvitem.cchTextMax = 0;
		lvitem.fmt = LVCFMT_LEFT;
		lvitem.cx = _width;
		bool success = ::SendMessage(control, LVM_INSERTCOLUMNA, column_id, (LPARAM)&lvitem);
	}

	void directApplicationWin32::addListViewRow(int ddlControlId, LPARAM _data, const std::vector<char*>& _items)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		LVITEM lvitem;
		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem = ListView_GetItemCount(control);
		lvitem.iSubItem = 0;
		lvitem.pszText = _items[0];
		lvitem.cchTextMax = 0;
		lvitem.lParam = _data;
		bool success =  ListView_InsertItem(control, &lvitem);
		int row = lvitem.iItem;

		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.mask = LVIF_TEXT;
		bool subitemSuccess = false;

		for (int i = 0; i < _items.size(); i++)
		{
			lvitem.pszText = _items[i];
			lvitem.iItem = row;
			lvitem.iSubItem = i;
			subitemSuccess = ListView_SetItem(control, &lvitem);
		}
	}

	int directApplicationWin32::getListViewSelectedIndex(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int iPos = ListView_GetNextItem(control, -1, LVNI_SELECTED);
		return iPos;
	}

	LPARAM directApplicationWin32::getListViewSelectedValue(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int pos = getListViewSelectedIndex(ddlControlId);
		if (pos > -1) {
			LVITEMA lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = pos;
			lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			ListView_GetItem(control, &lvitem);
			return lvitem.lParam;
		}
		return -1;
	}

	std::string directApplicationWin32::getListViewSelectedText(int ddlControlId)
	{
		char buffer[16384];
		std::string result;
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		int pos = getListViewSelectedIndex(ddlControlId);
		if (pos > -1) {
			LVITEMA lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = pos;
			lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			lvitem.pszText = buffer;
			lvitem.cchTextMax = sizeof(buffer) - 1;
			ListView_GetItem(control, &lvitem);
			result = lvitem.pszText;
		}
		return result;
	}

	std::list<std::string> directApplicationWin32::getListViewSelectedTexts(int ddlControlId)
	{
		char buffer[16384];
		std::list<std::string> ret;

		std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		for (auto ix = indeces.begin(); ix != indeces.end(); ix++) {
			LVITEMA lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = *ix;
			lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			lvitem.pszText = buffer;
			lvitem.cchTextMax = sizeof(buffer) - 1;
			ListView_GetItem(control, &lvitem);
			ret.push_back(lvitem.pszText);
		}

		return ret;
	}

	std::list<int> directApplicationWin32::getListViewSelectedIndexes(int ddlControlId)
	{
		std::list<int> ret;

		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		int iPos = ListView_GetNextItem(control, -1, LVNI_SELECTED);
		while (iPos > -1) {
			ret.push_back(iPos);
			iPos = ListView_GetNextItem(control, iPos, LVNI_SELECTED);
		}

		return ret;
	}

	std::list<LPARAM> directApplicationWin32::getListViewSelectedValues(int ddlControlId)
	{
		std::list<LPARAM> ret;

		std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		for (auto ix = indeces.begin(); ix != indeces.end(); ix++) {
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = *ix;
			lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			ListView_GetItem(control, &lvitem);
			ret.push_back(lvitem.lParam);
		}

		return ret;
	}


	void directApplicationWin32::clearListViewSelection(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		// deselect the items
		std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
		for (auto idx = indeces.begin(); idx != indeces.end(); idx++) {
			int pos = *idx;
			if (pos > -1) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = pos;
				lvitem.mask = LVIF_STATE;
				lvitem.state = 0;
				lvitem.stateMask = LVIS_SELECTED;
				ListView_SetItem(control, &lvitem);
			}
		}
	}

	void directApplicationWin32::setListViewSelectedIndex(int ddlControlId, int indexId)
	{
		clearListViewSelection(ddlControlId);

		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		LVITEM lvitem;
		ZeroMemory(&lvitem, sizeof(lvitem));
		lvitem.iItem = indexId;
		lvitem.mask = LVIF_STATE;
		lvitem.state = LVIS_SELECTED;
		lvitem.stateMask = LVIS_SELECTED;
		ListView_SetItem(control, &lvitem);
	}

	void directApplicationWin32::setListViewSelectedText(int ddlControlId, std::string& _text)
	{
		setListViewSelectedText(ddlControlId, _text.c_str());
	}

	void directApplicationWin32::setListViewSelectedText(int ddlControlId, const char* _text)
	{
		clearListViewSelection(ddlControlId);

		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		LVFINDINFOA lvfi;

		ZeroMemory(&lvfi, sizeof(lvfi));

		lvfi.flags = LVFI_STRING;
		lvfi.psz = _text;

		int pos = ListView_FindItem(control, -1, &lvfi);

		if (pos > -1) {
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = pos;
			lvitem.mask = LVIF_STATE;
			lvitem.state = LVIS_SELECTED;
			lvitem.stateMask = LVIS_SELECTED;
			ListView_SetItem(control, &lvitem);
			ListView_EnsureVisible(control, pos, false);
		}

	}

	void directApplicationWin32::setListViewSelectedValue(int ddlControlId, LPARAM value)
	{
		clearListViewSelection(ddlControlId);

		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		LVFINDINFO lvfi;

		ZeroMemory(&lvfi, sizeof(lvfi));

		lvfi.flags = LVIF_PARAM;
		lvfi.lParam = value;

		int pos = ListView_FindItem(control, -1, &lvfi);

		if (pos > -1) {
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = pos;
			lvitem.mask = LVIF_STATE;
			lvitem.state = LVIS_SELECTED;
			lvitem.stateMask = LVIS_SELECTED;
			ListView_SetItem(control, &lvitem);
		}
	}

	void directApplicationWin32::setScrollHeight(int ddlControlId, int height)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SetScrollRange(control, SB_VERT, 0, height, TRUE);
	}

	void directApplicationWin32::setScrollWidth(int ddlControlId, int width)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SetScrollRange(control, SB_HORZ, 0, width, TRUE);
	}

	point directApplicationWin32::getScrollPos(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		point pt;
		pt.x = ::GetScrollPos(control, SB_HORZ);
		pt.y = ::GetScrollPos(control, SB_VERT);
		return pt;
	}

	point directApplicationWin32::getScrollTrackPos(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		point pt;

		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS | SIF_POS;
		::GetScrollInfo(control, SB_HORZ, &info);
		pt.x = info.nTrackPos;
		::GetScrollInfo(control, SB_VERT, &info);
		pt.y = info.nTrackPos;

		return pt;
	}

	void directApplicationWin32::setScrollPos(int ddlControlId, point pt)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		pt.x = ::SetScrollPos(control, SB_HORZ, pt.x, TRUE);
		pt.y = ::SetScrollPos(control, SB_VERT, pt.y, TRUE);
	}

	point directApplicationWin32::getScrollRange(int ddlControlId)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		point sz;
		int dummy, dx, dy;
		::GetScrollRange(control, SB_HORZ, &dummy, &dx);
		::GetScrollRange(control, SB_VERT, &dummy, &dy);
		sz.x = dx;
		sz.y = dy;
		return sz;
	}

	rectangle directApplicationWin32::getWindowPos(int ddlControlId)
	{
		HWND control = ddlControlId > 0 ? ::GetDlgItem(hwndRoot, ddlControlId) : hwndRoot;

		RECT r;
		rectangle rd;

		::GetWindowRect(control, &r);

		if (ddlControlId >= 0)
		{
			::ScreenToClient(hwndRoot, (LPPOINT)&r.left);
			::ScreenToClient(hwndRoot, (LPPOINT)&r.right);
		}

		rd.x = r.left;
		rd.y = r.top;
		rd.w = r.right - r.left;
		rd.h = r.bottom - r.top;

		rd.x *= dpiScale;
		rd.y *= dpiScale;
		rd.w *= dpiScale;
		rd.h *= dpiScale;

		return rd;
	}

	double directApplicationWin32::toPixelsFromDips(double r)
	{
		return r / dpiScale;
	}

	double directApplicationWin32::toDipsFromPixels(double r)
	{
		return r * dpiScale;
	}

	rectangle directApplicationWin32::toPixelsFromDips(const rectangle& r)
	{
		rectangle rx;

		rx.x = r.x / dpiScale;
		rx.y = r.y / dpiScale;
		rx.w = r.w / dpiScale;
		rx.h = r.h / dpiScale;
		return rx;
	}

	rectangle directApplicationWin32::toDipsFromPixels(const rectangle& r)
	{
		rectangle rx;

		rx.x = r.x * dpiScale;
		rx.y = r.y * dpiScale;
		rx.w = r.w * dpiScale;
		rx.h = r.h * dpiScale;
		return rx;
	}

	rectangle directApplicationWin32::getWindowClientPos()
	{
		RECT r;
		rectangle rd;

		::GetClientRect(hwndRoot, &r);

		rd.x = r.left;
		rd.y = r.top;
		rd.w = r.right - r.left;
		rd.h = r.bottom - r.top;

		rd = toDipsFromPixels(rd);

		return rd;
	}


	void directApplicationWin32::setWindowPos(int ddlControlId, rectangle rect)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

		::MoveWindow(control, rect.x, rect.y, rect.w, rect.h, true);
	}

	void directApplicationWin32::setMinimumWindowSize(point size)
	{
		this->minimumWindowSize = size;
	}

	void directApplicationWin32::setSpinRange(int ddlControlId, int lo, int high)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, UDM_SETRANGE32, lo, high);
	}

	void directApplicationWin32::setSpinPos(int ddlControlId, int pos)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, UDM_SETPOS32, 0, pos);
	}

	void directApplicationWin32::setRedraw(int ddlControlId, bool pos)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SendMessage(control, WM_SETREDRAW, pos, 0);
	}

	void directApplicationWin32::redraw(int ddlControlId, bool pos)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		RedrawWindow(control, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}

	void directApplicationWin32::setSysLinkText(int ddlControlId, const char* _text)
	{
		HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
		::SetWindowText(control, _text);
	}

	class WinHttpSession {
	private:

		HINTERNET	hSession,
			hConnect,
			hRequest;
	public:

		WinHttpSession() : hSession(NULL), hConnect(NULL), hRequest(NULL)
		{
			// Use WinHttpOpen to obtain a session handle.
			hSession = ::WinHttpOpen(L"Corona Win64-Direct2d/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0);

			throwOnNull(hSession, "Could not create WinHttpSession");
		}

		virtual std::string Get(const char* _domain, const char* _path)
		{
			wchar_t domain[2048], path[2048];

			std::string result = "";

			::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _domain, strlen(_domain) + 1, domain, sizeof(domain) / sizeof(wchar_t));

			if (_path)
				::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _path, strlen(_path) + 1, path, sizeof(path) / sizeof(wchar_t));
			else
				path[0] = 0;

			if (hSession) {
				hConnect = ::WinHttpConnect(hSession, domain, INTERNET_DEFAULT_HTTP_PORT, 0);
				throwOnNull(hConnect, "Could not connect to domain");

				hRequest = ::WinHttpOpenRequest(hConnect, L"GET", _path ? path : NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
				throwOnNull(hRequest, "Could not open http request");

				bool success = ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
				if (success) {
					DWORD dwSize = 0;
					DWORD dwDownloaded = 0;
					LPSTR pszOutBuffer;
					BOOL  bResults = FALSE;

					if (!WinHttpReceiveResponse(hRequest, NULL)) {
						throwOnNull(hRequest, "Could not open receive http response");
					}
					do
					{
						// Check for available data.
						dwSize = 0;
						if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
						{
							throwOnNull(NULL, "Could not query data available");
						}

						// Allocate space for the buffer.
						pszOutBuffer = new char[dwSize + 1];
						throwOnNull(pszOutBuffer, "Out of Memory");
						// Read the data.
						ZeroMemory(pszOutBuffer, dwSize + 1);

						if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
							delete[] pszOutBuffer;
							throwOnNull(NULL, "Could not read");
						}

						result += pszOutBuffer;

						delete[] pszOutBuffer;

					} while (dwSize > 0);
				}
			}

			return result;
		}

		virtual ~WinHttpSession()
		{
			// Close any open handles.
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
		}
	};

	std::vector<std::string> directApplicationWin32::readInternet(const char* _domain, const char* _path)
	{
		WinHttpSession session;

		std::string resultString = session.Get(_domain, _path);

		return split(resultString, '\n');
	}

	// utility
	char* directApplicationWin32::getLastChar(char* _str)
	{
		char* pc = NULL;
		while (*_str) {
			pc = _str;
			_str++;
		}
		return pc;
	}

	bool directApplicationWin32::getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension)
	{
		char szFileName[MAX_PATH + 1] = "";
		bool retval;

		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndRoot;
		ofn.lpstrFilter = _pathExtensions;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = _defaultExtension;

		retval = GetSaveFileName(&ofn);
		if (retval)
			_saveFileName = szFileName;

		return retval;
	}

	void directApplicationWin32::setColorCapture(int _iconResourceId)
	{
		::SetCapture(hwndRoot);
		colorCapture = true;
		SetCursor(LoadCursor(hinstance, MAKEINTRESOURCE(_iconResourceId)));
	}

	bool directApplicationWin32::checkBackgroundComplete(MSG* _msg)
	{
		bool handled = false;
		ui_task_result* result = nullptr;
		http_task_result* http_result = nullptr;

		if (_msg->message == WM_CORONA_JOB_COMPLETE)
		{
			result = (ui_task_result *)_msg->lParam;
			if (result) {
				if (result->on_gui) {
					result->on_gui();
				}
				delete result;
			}
			handled = true;
		}
		else if (_msg->message == WM_CORONA_TASK_COMPLETE)
		{
			result = (ui_task_result*)_msg->lParam;
			if (result) {
				if (result->on_gui) {
					result->on_gui();
				}
				delete result;
			}
			handled = true;
		}
		else if (_msg->message == WM_CORONA_HTTP_TASK_COMPLETE)
		{
			http_result = (http_task_result*)_msg->lParam;
			if (http_result) {
				if (http_result->on_gui) {
					http_result->on_gui( http_result->status);
				}
				delete http_result;
			}
			handled = true;
		}
		else if (_msg->message == WM_CORONA_HTTP_JOB_COMPLETE)
		{
			http_result = (http_task_result*)_msg->lParam;
			if (http_result) {
				if (http_result->on_gui) {
					http_result->on_gui(http_result->status);
				}
				delete http_result;
			}
			handled = true;
		}

		return handled;
	}


	// this needs to get moved to application
	std::string directApplicationWin32::getUserName()
	{
		std::string result;
		char buffer[UNLEN+1] = {};
		DWORD max_length = sizeof(buffer) / sizeof(char);
		if (GetUserNameEx(NameUserPrincipal, buffer, &max_length)) {
			result = buffer;
		}
		return result;
	}

	// This was taken from a stack overflow article

	void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
	{
		// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
		// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
		// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
		// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
		// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
		// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
		// using the "_dup2" function.
		if (bindStdIn)
		{
			FILE* dummyFile;
			freopen_s(&dummyFile, "nul", "r", stdin);
		}
		if (bindStdOut)
		{
			FILE* dummyFile;
			freopen_s(&dummyFile, "nul", "w", stdout);
		}
		if (bindStdErr)
		{
			FILE* dummyFile;
			freopen_s(&dummyFile, "nul", "w", stderr);
		}

		// Redirect unbuffered stdin from the current standard input handle
		if (bindStdIn)
		{
			HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
			if (stdHandle != INVALID_HANDLE_VALUE)
			{
				int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
				if (fileDescriptor != -1)
				{
					FILE* file = _fdopen(fileDescriptor, "r");
					if (file != NULL)
					{
						int dup2Result = _dup2(_fileno(file), _fileno(stdin));
						if (dup2Result == 0)
						{
							setvbuf(stdin, NULL, _IONBF, 0);
						}
					}
				}
			}
		}

		// Redirect unbuffered stdout to the current standard output handle
		if (bindStdOut)
		{
			HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			if (stdHandle != INVALID_HANDLE_VALUE)
			{
				int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
				if (fileDescriptor != -1)
				{
					FILE* file = _fdopen(fileDescriptor, "w");
					if (file != NULL)
					{
						int dup2Result = _dup2(_fileno(file), _fileno(stdout));
						if (dup2Result == 0)
						{
							setvbuf(stdout, NULL, _IONBF, 0);
						}
					}
				}
			}
		}

		// Redirect unbuffered stderr to the current standard error handle
		if (bindStdErr)
		{
			HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
			if (stdHandle != INVALID_HANDLE_VALUE)
			{
				int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
				if (fileDescriptor != -1)
				{
					FILE* file = _fdopen(fileDescriptor, "w");
					if (file != NULL)
					{
						int dup2Result = _dup2(_fileno(file), _fileno(stderr));
						if (dup2Result == 0)
						{
							setvbuf(stderr, NULL, _IONBF, 0);
						}
					}
				}
			}
		}

		// Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
		// standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
		// versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
		// has been read from or written to the targets or not.
		if (bindStdIn)
		{
			std::wcin.clear();
			std::cin.clear();
		}
		if (bindStdOut)
		{
			std::wcout.clear();
			std::cout.clear();
		}
		if (bindStdErr)
		{
			std::wcerr.clear();
			std::cerr.clear();
		}
	}

	void EnableGuiStdOuts()
	{
		AllocConsole();
		BindCrtHandlesToStdHandles(true, true, true);
	}

}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#endif
