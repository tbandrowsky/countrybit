#define APPLICATION_DEVDESK 1
#define APPLICATION_PROPOSE 2
#define APPLICATION_ID APPLICATION_DEVDESK

#include "windows.h"
#include "windowsx.h"
#include <functional>


#if APPLICATION_ID == APPLICATION_DEVDESK
#include "app_devdesk.hpp"
#define application_function corona::run_developer_application;
#endif

#if APPLICATION_ID == APPLICATION_PROPOSE
#include "app_wspropose.hpp"
#define application_function corona::run_proposal_application;
#endif

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	auto dpiResult = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	CoInitialize(NULL);
	INITCOMMONCONTROLSEX ice;
	ZeroMemory(&ice, sizeof(ice));
	ice.dwSize = sizeof(ice);
	ice.dwICC = ICC_LISTVIEW_CLASSES |
		ICC_TREEVIEW_CLASSES |
		ICC_BAR_CLASSES |
		ICC_TAB_CLASSES |
		ICC_UPDOWN_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_HOTKEY_CLASS |
		ICC_ANIMATE_CLASS |
		ICC_WIN95_CLASSES |
		ICC_DATE_CLASSES |
		ICC_USEREX_CLASSES |
		ICC_COOL_CLASSES |
		ICC_INTERNET_CLASSES |
		ICC_NATIVEFNTCTL_CLASS |
		ICC_LINK_CLASS;

	BOOL result = ::InitCommonControlsEx(&ice);

	corona::run_developer_application(hInstance, lpszCmdParam);

	CoUninitialize();
}

