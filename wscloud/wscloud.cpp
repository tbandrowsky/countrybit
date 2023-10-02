
#include "corona_platform.h"
#include "resource.h"

#include <functional>

import corona.database;

/*

Use CComPtr for COM objects and CAdapt for collections

*/


void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam);

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	auto dpiResult = ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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

	run_application(hInstance, lpszCmdParam);

	CoUninitialize();
}

void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
{
	EnableGuiStdOuts();

	std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
	factory->refresh();

	std::shared_ptr<directApplicationWin32> wsPropose = std::make_shared<directApplicationWin32>(factory);

	bool forceWindowed = false;

	if (strstr(lpszCmdParam, "-window")) {
		forceWindowed = true;
	}

#if _DEBUG
	forceWindowed = true;
#endif

	std::shared_ptr<presentation> test_app = std::make_shared<presentation>();

	const int IDM_VIEW = 5000;
	const int IDM_VIEW_ENVIRONMENTS = 5001;
	const int IDM_VIEW_NETWORKS = 5003;

	const int IDM_SETTINGS = 5201;

	const int IDM_COMPANY = 5301;
	const int IDM_COMPANY_ABOUT = 5302;
	const int IDM_COMPANY_HOME = 5303;

	menu_item app_menu;

	app_menu.begin_submenu(IDM_COMPANY, "Woodruff &Sawyer")
		.item(IDM_COMPANY_ABOUT, "A&bout")
		.item(IDM_COMPANY_HOME, "H&ome")
		.end()
		.begin_submenu(IDM_VIEW, "&View")
		.destination(IDM_VIEW_ENVIRONMENTS, "&Networks", "networks")
		.destination(IDM_VIEW_NETWORKS, "&Environments", "environments")
		.end();

	const int IDC_IMAGE_LOGO = 1024;
	const int IDC_COMPANY_NAME = 1025;
	const int IDC_PLATFORM_TEST = 1026;

	const int IDC_HOME = 1027;
	const int IDC_TITLE_BAR = 1028;

	const int IDC_REGISTER_NEW_ACCOUNT = 1204;

	auto& st = styles.get_style();

	const int IDC_LOGIN = 1101;
	const int IDC_LOGIN_USERS = 1102;
	const int IDC_LOGIN_HELLO = 1103;
	const int IDC_LOGIN_REGISTER = 1104;

	const int IDC_SYSTEM_MENU = 5001;

	std::function<void(pushbutton_control& _set_defaults)> push_button_defaults = [](pushbutton_control& ctrl) {
		ctrl.set_size(.50_container, 50.0_px);
	};

	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Cloud", IDI_WSCLOUD, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Cloud", IDI_WSCLOUD, true, test_app);
	}

}

