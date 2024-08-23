#define APPLICATION_REVOLUTION 1
#define APPLICATION_GUITAR 2
#define APPLICATION_AURA 3
#define APPLICATION_REPARATIONS 4
#define APPLICATION_DONATION_FINDER 5
#define APPLICATION_ID APPLICATION_DONATION_FINDER

#if APPLICATION_ID == APPLICATION_REVOLUTION
#include "app_revolution.hpp"
#define application_function(a,b) corona::run_aura_application(a,b);
#endif

#if APPLICATION_ID == APPLICATION_AURA
#include "app_aura.hpp"
#define application_function(a,b) corona::run_revolution_application(a,b);
#endif

#if APPLICATION_ID == APPLICATION_GUITAR
#include "app_guitar.hpp"
#define application_function(a,b) corona::run_proposal_application(a,b);
#endif

#if APPLICATION_ID == APPLICATION_REPARATIONS
#include "app_reparations.hpp"
#define application_function(a,b) corona::run_reparations_application(a,b);
#endif

#if APPLICATION_ID == APPLICATION_DONATION_FINDER
#include "app_donation_finder.hpp"
#define application_function(a,b) corona::run_donation_finder_application(a,b);
#endif

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	auto dpiResult = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
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

	application_function(hInstance, lpszCmdParam);

	CoUninitialize();
}

