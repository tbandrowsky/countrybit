
#include "corona.h"
#include "resource.h"
#include "corona.h"

/*

Use CComPtr for COM objects and CAdapt for collections

*/

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{
	
	auto dpiResult = ::SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	CoInitialize(NULL);

	corona::win32::EnableGuiStdOuts();

	std::shared_ptr<corona::win32::adapterSet> factory = std::make_shared<corona::win32::adapterSet>();
	factory->refresh();

	std::shared_ptr<corona::win32::directApplicationWin32> wsPropose = std::make_shared<corona::win32::directApplicationWin32>(factory);

	bool forceWindowed = false;

	if (strstr(lpszCmdParam, "-window")) {
		forceWindowed = true;
	}

#if _DEBUG
	forceWindowed = true;
#endif

	std::shared_ptr<corona::win32::presentation> test_app = std::make_shared<corona::win32::presentation>();

	const int IDC_HOME_COLUMN = 1;

	test_app->create_page("home")
		.column_begin(IDC_HOME_COLUMN);

	if (forceWindowed) 
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else 
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

	CoUninitialize();
}

