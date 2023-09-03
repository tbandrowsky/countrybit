
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

	int IDC_TEST_LABEL1 = 1001;
	int IDC_TEST_EDIT1 = 1002;
	int IDC_TEST_LABEL2 = 1003;
	int IDC_TEST_EDIT2 = 1004;
	int IDC_TEST_LABEL3 = 1005;
	int IDC_TEST_COMBO1 = 1006;
	int IDC_TEST_LABEL4 = 1007;
	int IDC_TEST_LISTVIEW = 1008;
	int IDC_RICH_EDIT1 = 1009;
	int IDC_COMBO_BOX1 = 1010;
	int IDC_COMBO_BOXEX1 = 1011;
	int IDC_TEST_LABEL5 = 1012;
	int IDC_TEST_LABEL6 = 1013;
	int IDC_BUTTON1 = 1014;
	int IDC_PUSHBUTTON1 = 1015;
	int IDC_RADIOBUTTON1 = 1016;
	int IDC_RADIOBUTTON2 = 1017;
	int IDC_TEST_LABEL7 = 1018;
	int IDC_TEST_LABEL8 = 1019;
	int IDC_TEST_LABEL9 = 1020;
	int IDC_APP_TITLE = 1021;
	int IDC_APP_SUBTITLE = 1022;

	auto& st = styles.get_style();

	test_app->create_page("home")
			.column_begin()
				.row_begin()
				.set_size(1.0_container, 250.0_px)
				.set_background_color(st.HeaderBackgroundColor)
				.set_content_align(visual_alignment::align_near)
				.set_content_cross_align(visual_alignment::align_near)
					.column_begin()
					.set_size(400.0_px, 200.0_px)
					.set_content_align(visual_alignment::align_near)
						.title( "Woodruff Sawyer", IDC_APP_TITLE )
						.subtitle("Technology Test", IDC_APP_SUBTITLE)
					.end()
				.end()
				.row_begin()
				.set_item_margin(23.0_px)
				.set_size(1.0_container, 1.0_remaining)
					.column_begin()
					.set_background_color(st.Section1BackgroundColor)
					.set_size(.3_container, 1.0_container)
						.chaptertitle("List Controls")
						.chaptersubtitle("For viewing lists")
						.label("List View", IDC_TEST_LABEL1)
						.set_item_size(1.0_container, .5_remaining)
						.listview(IDC_TEST_LISTVIEW)
						.label("List Box", IDC_TEST_LABEL2)
					.end()
					.column_begin()
					.set_background_color(st.Section2BackgroundColor)
					.set_size(.2_container, 1.0_container)
					.set_item_margin(4.0_px)
						.chaptertitle("Edit Controls")
						.chaptersubtitle("Test Panel For Edits")
						.label("Edit 1", IDC_TEST_LABEL3)
						.edit(IDC_TEST_EDIT1)
						.label("Edit 2", IDC_TEST_LABEL4)
						.edit(IDC_TEST_EDIT2)
						.label("Combo ", IDC_TEST_LABEL5)
						.combobox(IDC_COMBO_BOX1)
						.label("Combo EX", IDC_TEST_LABEL6)
						.comboboxex(IDC_COMBO_BOXEX1)
						.label("Checkbox", IDC_TEST_LABEL7)
						.checkbox("Check", IDC_BUTTON1)
						.label("Radio", IDC_TEST_LABEL8)
						.radio_button("Radio 1", IDC_RADIOBUTTON1)
						.radio_button("Radio 2", IDC_RADIOBUTTON2)
						.set_item_size(1.0_container, 2.0_fontgr)
						.push_button("Ok", IDC_PUSHBUTTON1)
					.end()
					.column_begin()
					.set_background_color(st.Section3BackgroundColor)
					.set_size(1.0_remaining, 1.0_container)
					.set_item_size(.95_remaining, 0.0_px)
					.set_item_margin(4.0_px)
						.chaptertitle("Content Controls")
						.chaptersubtitle("Windows Text Editor")
						.label("Text Editor", IDC_TEST_LABEL9)
						.set_item_size(1.0_container, .5_remaining)
						.richedit(IDC_RICH_EDIT1)
						.chaptersubtitle("Direct 2d Controls")
					.end()
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

	auto& cb = test_app->find<combobox_control>(IDC_COMBO_BOX1);

	list_data ldata;
	ldata.id_field = "Id";
	ldata.text_field = "Name";
	ldata.items = R"([
{ "Id":0, "Name":"Bob" },
{ "Id":1, "Name":"Ted" },
{ "Id":2, "Name":"Isiah" },
{ "Id":3, "Name":"April" },
{ "Id":4, "Name":"Moesha" },
{ "Id":5, "Name":"Joe" },
{ "Id":6, "Name":"Raghu" },
{ "Id":7, "Name":"Harmeet" }
])"_json;
	cb.set_list(ldata);

	auto& cbex = test_app->find<comboboxex_control>(IDC_COMBO_BOXEX1);

	list_data ldata2;
	ldata2.id_field = "Id";
	ldata2.text_field = "Name";
	ldata2.items = R"([
{ "Id":0, "Name":"Akron" },
{ "Id":1, "Name":"Cleveland" },
{ "Id":2, "Name":"Barberton" },
{ "Id":3, "Name":"Youngstown" },
{ "Id":4, "Name":"Breezewood" },
{ "Id":5, "Name":"Leesville" },
{ "Id":6, "Name":"Phillipsburg" },
{ "Id":7, "Name":"Marlton" },
{ "Id":8, "Name":"Elkton" },
{ "Id":9, "Name":"Philadelphia" }
])"_json;
	cbex.set_list(ldata2);

	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

}
