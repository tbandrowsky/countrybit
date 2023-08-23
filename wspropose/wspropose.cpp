
#include "corona.h"
#include "resource.h"

using namespace corona::database;
using namespace corona::win32;

/*

Use CComPtr for COM objects and CAdapt for collections

*/

void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam);

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{
	
	auto dpiResult = ::SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	CoInitialize(NULL);

	run_application(hInstance, lpszCmdParam);

	CoUninitialize();
}

void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
{
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

	int IDC_TEST_LABEL = 1001;
	int IDC_TEST_EDIT = 1002;
	int IDC_TEST_LABEL2 = 1003;
	int IDC_TEST_EDIT2 = 1004;

	test_app->create_page("home")
		.column_begin()
		.set_size(100.0_container, 100.0_container)
		.set_align(visual_alignment::align_center)
		.title("Title")
		.subtitle("Subtitle")
		.chaptertitle("Chapter Title")
		.chaptersubtitle("Chapter Subtitle")
		.paragraph("Paragraph")
		.chaptersubtitle("Chapter Subtitle")
		.label(IDC_TEST_LABEL)
		.edit(IDC_TEST_EDIT)
		.label(IDC_TEST_LABEL2)
		.edit(IDC_TEST_EDIT2)
		.end();

	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

}
