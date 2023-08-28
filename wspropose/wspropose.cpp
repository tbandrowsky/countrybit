
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
	int IDC_TEST_LABEL3 = 1003;
	int IDC_TEST_COMBO1 = 1004;
	int IDC_TEST_LABEL4 = 1003;
	int IDC_TEST_LISTVIEW = 1004;

	test_app->create_page("home")
		.row_begin()
		.set_size(1.0_container, 1.0_container)
			.column_begin()
			.set_item_size( 1.0_container, 50.0_px)
			.set_size(.5_container, 1.0_container)
				.title("Title")
				.subtitle("Subtitle")
				.label("List View", IDC_TEST_LABEL)
				.listview(IDC_TEST_LISTVIEW)
			.end()
			.column_begin()
			.set_size(.5_container, 1.0_container)
				.row_begin()
				.set_size(1.0_container, 1.0_container)
					.column_begin()
					.set_size(0.5_container, 1.0_container)
					.set_align(visual_alignment::align_center)
						.chaptertitle("Chapter Title 1")
						.chaptersubtitle("Chapter Subtitle 1")
						.label("Test Edit", IDC_TEST_LABEL)
						.edit(IDC_TEST_EDIT)
					.end()
					.column_begin()
					.set_size(0.5_container, 1.0_container)
					.set_align(visual_alignment::align_far)
						.chaptertitle("Chapter Title 2")
						.chaptersubtitle("Chapter Subtitle 2")
						.label("Test Edit 2", IDC_TEST_LABEL2)
						.edit(IDC_TEST_EDIT2)
					.end()
				.end()
			.end()
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
