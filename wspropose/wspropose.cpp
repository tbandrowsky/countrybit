
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
		.set_size(.3_container, 1.0_container)
		.title("Title")
		.subtitle("Subtitle")
		.label("List View", IDC_TEST_LABEL)
		.set_item_size(1.0_container, 1.0_remaining)
		.listview(IDC_TEST_LISTVIEW)
		.end()
		.column_begin()
		.set_size(.7_container, 1.0_container)
		.row_begin()
		.set_size(1.0_container, 1.0_container)
		.column_begin()
		.set_size(0.5_container, 1.0_container)
		.set_align(visual_alignment::align_center)
		.chaptertitle("Chapter Title 1")
		.chaptersubtitle("Chapter Subtitle 1")
		.label("Test Edit 1", IDC_TEST_LABEL2)
		.edit(IDC_TEST_EDIT)
		.end()
		.column_begin()
		.set_size(0.5_container, 1.0_container)
		.set_align(visual_alignment::align_far)
		.chaptertitle("Chapter Title 2")
		.chaptersubtitle("Chapter Subtitle 2")
		.label("Test Edit 2", IDC_TEST_LABEL3)
		.edit(IDC_TEST_EDIT2)
		.end()
		.end()
		.end()
		.end();

	// now load the data
	auto &lv = test_app->find<listview_control>(IDC_TEST_LISTVIEW);

	table_data tdata;
	table_column column;

	column.alignment = visual_alignment::align_near;
	column.display_name = "Id";
	column.json_field = "Id";
	column.width = 50;
	tdata.columns.push_back(column);

	column.alignment = visual_alignment::align_near;
	column.display_name = "First Name";
	column.json_field = "FirstName";
	column.width = 200;
	tdata.columns.push_back(column);

	column.alignment = visual_alignment::align_near;
	column.display_name = "Last Name";
	column.json_field = "LastName";
	column.width = 200;
	tdata.columns.push_back(column);

	column.alignment = visual_alignment::align_far;
	column.display_name = "Age";
	column.json_field = "Age";
	column.width = 200;
	tdata.columns.push_back(column);

	tdata.items = R"([
{ "Id":0, "FirstName":"Bob", "LastName":"Richards", "Age":42 },
{ "Id":1, "FirstName":"Ted", "LastName":"Baxter", "Age":42 },
{ "Id":2, "FirstName":"Carol", "LastName":"Sandcrete", "Age":42 },
{ "Id":3, "FirstName":"Alice", "LastName":"Laker", "Age":42 },
{ "Id":4, "FirstName":"Sandy", "LastName":"Smith", "Age":42 },
{ "Id":6, "FirstName":"Holly", "LastName":"Rogers", "Age":34 }
]
)"_json;

	lv.set_table(tdata);



	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

}
