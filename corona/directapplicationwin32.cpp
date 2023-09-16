
#pragma once

#include "corona.h"

#ifdef WINDESKTOP_GUI


namespace corona
{
	namespace win32
	{

		// -------------------------------------------------------

		directApplicationWin32* directApplicationWin32::current;

		directApplicationWin32::directApplicationWin32(std::weak_ptr<adapterSet> _factory) : factory(_factory), colorCapture(false)
		{
			current = this;
			currentController = NULL;
			controlFont = nullptr;
			labelFont = nullptr,
			titleFont = nullptr;
			dpiScale = 1.0;
			disableChangeProcessing = false;

			backgroundColor.a = 1.0;
			backgroundColor.r = 1.0;
			backgroundColor.g = 1.0;
			backgroundColor.b = 1.0;
		}

		directApplicationWin32::~directApplicationWin32()
		{

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
				auto pfactory = factory.lock();
				if (!pfactory) return;

				auto winroot = pfactory->getWindow(hwndRoot).lock();

				if (winroot == nullptr)
					return;

				winroot->beginDraw(failedDevice);						

				if (!failedDevice) 
				{
					auto wins = winroot->getChildren();
					auto dc = winroot->getContext().getDeviceContext();

					dc->Clear(&backgroundColor);

					// here, we tell the children to draw on their own surfaces...
					// and then, draw on this one.
					currentController->drawFrame(dc);

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

					D2D1_RECT_F dest;
					dest.left = pos;
					dest.top = (wbounds.h - boxh) / 2;
					dest.right = pos + boxw;
					dest.bottom = dest.top + boxh;

					CComPtr<ID2D1SolidColorBrush> brush;
					D2D1_COLOR_F brushColor = {};
					brushColor.a = 1.0;
					brushColor.b = 1.0;

					// dc->CreateSolidColorBrush(brushColor, &brush);
					// dc->DrawRectangle(&dest, brush, 4, nullptr);

					winroot->endDraw(failedDevice);
				}

			failed_check:

				if (failedDevice) {
					pfactory->clearWindows();
					pfactory->refresh();
				}
			}
		}

		void directApplicationWin32::loadStyleSheet()
		{

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

		HFONT directApplicationWin32::createFontDips(HWND target, const char *_fontName, double fontSize, bool bold, bool italic )
		{
			HFONT hfont = nullptr;

			double targetScale = 96.0 / ::GetDpiForWindow(target);
			double ifontSize = fontSize * dpiScale;
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

		HFONT directApplicationWin32::createFontPixels(const char* _fontName, double fontSizePixels, bool bold, bool italic)
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

		HFONT directApplicationWin32::createFontIndirect(LOGFONT srcFont, const char* _fontName, double fontSizePixels, bool bold, bool italic)
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
			auto pfactory = factory.lock();
			if (!pfactory) return cx;

			auto win = pfactory->getWindow(hwndRoot).lock();
			auto child = win->createChild(control_id, bounds.x, bounds.y, bounds.w, bounds.h);
			return child;
		}

		LRESULT CALLBACK directApplicationWin32::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			return current->windowProcHandler(hwnd, message, wParam, lParam);
		}

		LRESULT directApplicationWin32::windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			bool found = false;
			point point;
			static HBRUSH hbrBkgnd2 = NULL;
			char className[256];
			database::point ptz;
			HRESULT hr;

			auto pfactory = factory.lock();

			std::weak_ptr<direct2dWindow> current_window;
			std::shared_ptr< direct2dWindow> pcurrent_window = nullptr;

			if (pfactory) {
				current_window = pfactory->getWindow(hwnd);
				pcurrent_window = current_window.lock();
			}

			switch (message)
			{
			case WM_CREATE:
				{
					hwndRoot = hwnd;
					RECT rcClient;
					GetWindowRect(hwnd, &rcClient);
					SetWindowPos(hwnd,NULL,rcClient.left, rcClient.top, abs(rcClient.right - rcClient.left), abs(rcClient.bottom - rcClient.top),SWP_FRAMECHANGED);
					if (currentController) {
						pfactory->createD2dWindow(hwnd, backgroundColor);
						dpiScale = 96.0 / GetDpiForWindow(hwnd);
						loadStyleSheet();
						currentController->onCreated();
					}
				}
				break;
			case WM_NCCALCSIZE:
				return 0;
/*			case WM_NCHITTEST:
				if (false && currentController)
				{
					point.x = GET_X_LPARAM(lParam);
					point.y = GET_Y_LPARAM(lParam);

					point.x = point.x * 96.0 / GetDpiForWindow(hwnd);
					point.y = point.y * 96.0 / GetDpiForWindow(hwnd);

					LRESULT ret = currentController->ncHitTest(pcurrent_window, &point);
					return ret;
				}
				break;
	*/		case WM_NCPAINT:
				{
					HDC hdc;
					hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
					// Paint into this DC 
					ReleaseDC(hwnd, hdc);
				}
				return 0;
			case WM_DESTROY:
				pfactory->closeWindow(hwnd);
				PostQuitMessage(0);
				return 0;
			case WM_COMMAND:
				if (currentController && !disableChangeProcessing)
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
					case CBN_SELCHANGE:
						{
							char window_class[500];
							if (::RealGetWindowClassA(controlWindow, window_class, sizeof(window_class) - 1)) {
								if (strcmp(WC_COMBOBOX, window_class) || 
									strcmp(WC_COMBOBOXEX, window_class) == 0) {
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
					break;
				}
				break;
			case WM_DPICHANGED:
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
				if (currentController && !disableChangeProcessing)
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
						database::control_base pi;
						if (lpmnlv->uNewState & LVIS_SELECTED)
							currentController->onListViewChanged(lpnm->idFrom);
					}
					break;
					case NM_CLICK:
					{

						::GetClassNameA(lpnm->hwndFrom, className, sizeof(className) - 1);
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
						if (draw_control* pdraw = pcontroller->get_parent_control<draw_control>(id)) {

							hbrBkgnd = pdraw->background_brush_win32;
							auto cv = pdraw->background_brush.brushColor;
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

			case WM_LBUTTONDOWN:
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
							color pickedColor;
							pickedColor.r = GetRValue(cr) / 255.0;
							pickedColor.g = GetGValue(cr) / 255.0;
							pickedColor.b = GetBValue(cr) / 255.0;
							ptz.x = p.x;
							ptz.y = p.y;
							if (currentController) {
								currentController->pointSelected(pcurrent_window, &ptz, &pickedColor);
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
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						if (pcurrent_window) {
							currentController->mouseLeftDown(pcurrent_window, &ptxo);
						}
					}
				}
				break;
			case WM_LBUTTONUP:
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						if (pcurrent_window) {
							currentController->mouseLeftUp(pcurrent_window, &ptxo);
						}
					}
				}
				break;
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
							database::point ptxo;
							ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
							ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
							if (pcurrent_window) {
								currentController->mouseRightDown(pcurrent_window, &ptxo);
							}
						}
					}
				}
				break;
			case WM_RBUTTONUP:
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						if (pcurrent_window) {
							currentController->mouseRightUp(pcurrent_window, &ptxo);
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
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						currentController->mouseMove(pcurrent_window, &ptxo);
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
				auto wwin = pfactory->getWindow(hwnd);
				if (auto win = wwin.lock()) {
					win->resize(rect.w, rect.h);
					if (currentController) {
						dpiScale = 96.0 / GetDpiForWindow(hwnd);
#if TRACE_SIZE
						std::cout << " w " << rect.w << "h " << rect.h << std::endl;

#endif
						rect.w *= dpiScale;
						rect.h *= dpiScale;
						currentController->onResize(rect, dpiScale);
					}
				}
			}
			break;
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		std::weak_ptr<direct2dChildWindow> directApplicationWin32::getDirect2dWindow(relative_ptr_type i)
		{
			std::weak_ptr<direct2dChildWindow> wp;

			auto pfactory = factory.lock();
			if (!pfactory) return wp;

			wp = pfactory->findChild(i);
			return wp;
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
			_firstController->setHost(ptr);

			WNDCLASS wcMain;
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
				::MessageBoxA(NULL, "Could not start because the  class could not be registered", "Couldn't Start", MB_ICONERROR);
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
				dwStyle | WS_CLIPSIBLINGS ,
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
					if (!::IsDialogMessage(hwndRoot, &msg)) {
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
			_firstController->setHost(ptr);

			WNDCLASS wcMain;
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
			wcMain.hbrBackground =	NULL;
			wcMain.lpszMenuName = NULL;
			wcMain.lpszClassName = "Corona2dBase";
			if (!RegisterClass(&wcMain)) {
				::MessageBoxA(NULL, "Could not start because the main window class could not be registered", "Couldn't Start", MB_ICONERROR);
				return 0;
			}

			hwndRoot = NULL;

			if (_fullScreen) {
				dwStyle = WS_POPUP | WS_MAXIMIZE;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
				ShowCursor(FALSE);
			}
			else {
				dwStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			}

			setController(_firstController);

			hwndRoot = CreateWindowEx(dwExStyle,
				wcMain.lpszClassName, _title,
				dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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
					if (!::IsDialogMessage(hwndRoot, &msg)) {
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

		void directApplicationWin32::setEditText(int textControlId, const std::string& _string)
		{
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			::SetWindowTextA(control, _string.c_str());
		}

		void directApplicationWin32::setEditText(int textControlId, const char* _string)
		{
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			::SetWindowTextA(control, _string);
		}

		std::string directApplicationWin32::getEditText(int textControlId)
		{
			std::string value = "";
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			int length = ::GetWindowTextLengthA(control) + 1;
			char* buffer = new char[length];
			if (buffer) {
				::GetWindowTextA(control, buffer, length);
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
			::SendMessageA(control, CB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setComboSelectedText(int ddlControlId, std::string& _text)
		{
			setComboSelectedText(ddlControlId, _text.c_str());
		}

		void directApplicationWin32::setComboSelectedText(int ddlControlId, const char* _text)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int index = ::SendMessageA(control, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)_text);
			::SendMessageA(control, CB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setComboSelectedValue(int ddlControlId, int value)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int count = ::SendMessageA(control, CB_GETCOUNT, NULL, NULL);
			for (int i = 0; i < count; i++) {
				int data = (int)::SendMessageA(control, CB_GETITEMDATA, i, 0);
				if (data == value) {
					::SendMessageA(control, CB_SETCURSEL, i, NULL);
					break;
				}
			}
		}

		void directApplicationWin32::clearComboItems(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessageA(control, CB_RESETCONTENT, NULL, NULL);
		}

		void directApplicationWin32::addComboItem(int ddlControlId, std::string& _text, int _data)
		{
			addComboItem(ddlControlId, _text.c_str(), _data);
		}

		void directApplicationWin32::addComboItem(int ddlControlId, const char* _text, int _data)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newItemIndex = (int)::SendMessageA(control, CB_ADDSTRING, NULL, (LPARAM)_text);
			if (newItemIndex != CB_ERR) {
				int err = ::SendMessageA(control, CB_SETITEMDATA, newItemIndex, (LPARAM)_data);
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
			::SendMessageA(control, LB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setListSelectedText(int ddlControlId, std::string& _text)
		{
			setListSelectedText(ddlControlId, _text.c_str());
		}

		void directApplicationWin32::setListSelectedText(int ddlControlId, const char* _text)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int index = ::SendMessageA(control, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)_text);
			::SendMessageA(control, LB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setListSelectedValue(int ddlControlId, int value)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int count = ::SendMessageA(control, LB_GETCOUNT, NULL, NULL);
			for (int i = 0; i < count; i++) {
				int data = (int)::SendMessageA(control, LB_GETITEMDATA, i, 0);
				if (data == value) {
					::SendMessageA(control, LB_SETCURSEL, i, NULL);
					break;
				}
			}
		}

		void directApplicationWin32::clearListItems(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessageA(control, LB_RESETCONTENT, NULL, NULL);
		}

		void directApplicationWin32::addListItem(int ddlControlId, std::string& _text, int _data)
		{
			addListItem(ddlControlId, _text.c_str(), _data);
		}

		void directApplicationWin32::addListItem(int ddlControlId, const char* _text, int _data)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newItemIndex = (int)::SendMessageA(control, LB_ADDSTRING, NULL, (LPARAM)_text);
			if (newItemIndex != LB_ERR) {
				int err = ::SendMessageA(control, LB_SETITEMDATA, newItemIndex, (LPARAM)_data);
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

			WIN32_FIND_DATA findData;
			ZeroMemory(&findData, sizeof(findData));

			HANDLE hfind = INVALID_HANDLE_VALUE;

			hfind = ::FindFirstFileA(searchPath, &findData);
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
						::PathAddBackslashA(recurseBuff);
						::PathAppendA(recurseBuff, findData.cFileName);
						addComboItem(ddlControlId, recurseBuff, 0);
						addFoldersToCombo(ddlControlId, recurseBuff);
					}
				} while (FindNextFile(hfind, &findData) != 0);
			}

		}

		void directApplicationWin32::addPicturesFoldersToList(int ddlControlId)
		{
			char picturesPath[MAX_PATH * 2];
			::SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, picturesPath);
			addFoldersToCombo(ddlControlId, picturesPath);
		}

		void directApplicationWin32::setFocus(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetFocus(control);
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

			WIN32_FIND_DATA findData;
			ZeroMemory(&findData, sizeof(findData));

			HANDLE hfind = INVALID_HANDLE_VALUE;

			hfind = ::FindFirstFileA(searchPath, &findData);
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
						::PathAddBackslashA(recurseBuff);
						::PathAppendA(recurseBuff, findData.cFileName);
						addComboItem(ddlControlId, recurseBuff, 0);
						addFoldersToCombo(ddlControlId, recurseBuff);
					}
				} while (FindNextFile(hfind, &findData) != 0);
			}
		}

		void directApplicationWin32::addPicturesFoldersToCombo(int ddlControlId)
		{
			char picturesPath[MAX_PATH * 2];
			::SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, picturesPath);
			addFoldersToCombo(ddlControlId, picturesPath);
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
			ListView_InsertItem(control, &lvitem);
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
			ListView_InsertColumn(control, column_id, &lvitem);
		}

		void directApplicationWin32::addListViewRow(int ddlControlId, LPARAM _data, const std::vector<char*>& _items)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.mask = LVIF_TEXT | LVIF_PARAM;
			lvitem.iItem = ListView_GetItemCount(control);
			lvitem.iSubItem = 0;
			lvitem.pszText = (LPSTR)_items[0];
			lvitem.cchTextMax = 0;
			lvitem.lParam = _data;
			bool success = ListView_InsertItem(control, &lvitem);
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
				LVITEM lvitem;
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
				LVITEM lvitem;
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
				LVITEM lvitem;
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

			LVFINDINFO lvfi;

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

			dpiScale = 96.0 / GetDpiForWindow(hwndRoot);

			rd.x *= dpiScale;
			rd.y *= dpiScale;
			rd.w *= dpiScale;
			rd.h *= dpiScale;

			return rd;
		}

		double directApplicationWin32::toPixelsFromDips(double r)
		{
			return r * GetDpiForWindow(hwndRoot) / 96.0;
		}

		double directApplicationWin32::toDipsFromPixels(double r)
		{
			return r * 96.0 / GetDpiForWindow(hwndRoot);
		}

		rectangle directApplicationWin32::toPixelsFromDips(const rectangle& r)
		{
			dpiScale = GetDpiForWindow(hwndRoot) / 96.0;
			rectangle rx;

			rx.x = r.x * dpiScale;
			rx.y = r.y * dpiScale;
			rx.w = r.w * dpiScale;
			rx.h = r.h * dpiScale;
			return rx;
		}

		rectangle directApplicationWin32::toDipsFromPixels(const rectangle& r)
		{
			dpiScale = 96.0 / GetDpiForWindow(hwndRoot);
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

			rd = toPixelsFromDips(rd);

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

		void directApplicationWin32::setSysLinkText(int ddlControlId, const char* _text)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetWindowTextA(control, _text);
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

			OPENFILENAME ofn;
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
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#endif
