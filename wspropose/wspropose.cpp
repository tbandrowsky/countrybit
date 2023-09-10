
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

	const int IDC_TEST_EDIT1 = 1002;
	const int IDC_TEST_EDIT2 = 1004;
	const int IDC_TEST_COMBO1 = 1006;
	const int IDC_TEST_LISTVIEW = 1008;
	const int IDC_RICH_EDIT1 = 1009;
	const int IDC_COMBO_BOX1 = 1010;
	const int IDC_COMBO_BOXEX1 = 1011;
	const int IDC_BUTTON1 = 1014;
	const int IDC_PUSHBUTTON1 = 1015;
	const int IDC_RADIOBUTTON1 = 1016;
	const int IDC_RADIOBUTTON2 = 1017;
	const int IDC_TEST_LISTBOX = 1023;
	const int IDC_IMAGE_LOGO = 1024;
	const int IDC_COMPANY_NAME = 1025;
	const int IDC_PLATFORM_TEST = 1026;

	const int IDC_HOME = 1027;
	const int IDC_TITLE_BAR = 1028;

	const int IDC_FONT_PICK = 1029;
	const int IDC_BOLD = 1030;
	const int IDC_ITALIC = 1031;
	const int IDC_UNDERLINE = 1032;
	const int IDC_STRIKETHROUGH = 1033;
	const int IDC_SUPERSCRIPT = 1034;
	const int IDC_SUBSCRIPT = 1035;
	const int IDC_FONT_SIZE = 1036;
	const int IDC_FONT_NAME = 1037;

	auto& st = styles.get_style();

	test_app->create_page("test")
		.column_begin(IDC_HOME)
		.corporate_logo_bar(
			st,
			IDC_TITLE_BAR,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"TECHNOLOGY TEST",
			"Corona UI"
		)
		.row_begin()
			.column_begin()
				.label()
				.input()
				.input()
				.label()
				.button()
				.end()
			.end()
	.end();

	test_app->create_page("test")
		.column_begin(IDC_HOME)
		.corporate_logo_bar(
			st,
			IDC_TITLE_BAR,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"TECHNOLOGY TEST",
			"Corona UI"
		)
		.row_begin()
				.set_item_margin(10.0_px)
				.set_size(1.0_container, 1.0_remaining)
					.column_begin([st, IDC_COMPANY_NAME](column_layout& target) 
						{
							target.set_background_color(st.Section1BackgroundColor);
							target.set_size(.3_container, 1.0_container);
							target.set_item_margin(8.0_px);
							target.push(IDC_COMPANY_NAME, true, false, false, false );
						})
						.chaptertitle("List Controls")
						.chaptersubtitle("For viewing lists")
						.label("List View" )
						.set_item_size(1.0_container, 10.0_fontgr)
						.listview(IDC_TEST_LISTVIEW)
						.set_item_size(1.0_container, 1.2_fontgr)
						.label("List Box")
						.set_item_size(1.0_container, 10.0_fontgr)
						.listbox(IDC_TEST_LISTBOX)
					.end()
					.column_begin([st, IDC_PLATFORM_TEST ](column_layout& target)
						{
							target.set_background_color(st.Section2BackgroundColor);
							target.set_size(.2_container, 1.0_container);
							target.set_item_margin(4.0_px);
							target.push(IDC_PLATFORM_TEST, true, false, false, false);
						})
						.chaptertitle("Edit Controls")
						.chaptersubtitle("Test Panel For Edits")
						.label("Edit 1")
						.edit(IDC_TEST_EDIT1)
						.label("Edit 2")
						.edit(IDC_TEST_EDIT2)
						.label("Combo ")
						.combobox(IDC_COMBO_BOX1)
						.label("Combo EX")
						.comboboxex(IDC_COMBO_BOXEX1)
						.label("Checkbox")
						.checkbox(IDC_BUTTON1, "Check" )
						.label("Radio")
						.radio_button( IDC_RADIOBUTTON1, "Radio 1")
						.radio_button( IDC_RADIOBUTTON2, "Radio 2")
						.set_item_size(1.0_container, 2.0_fontgr)
						.push_button(IDC_PUSHBUTTON1, "Ok" )
					.end()
					.column_begin([st](column_layout& control) {
						control.set_background_color(st.Section3BackgroundColor);
						control.set_size(1.0_remaining, 1.0_container);
						control.set_item_margin(4.0_px);
					})
						.chaptertitle("Content Controls")
						.chaptersubtitle("Windows Text Editor")
						.row_begin([st](row_layout& _layout) {
							_layout.set_content_align(visual_alignment::align_near);
							_layout.set_size(1.0_container, 1.0_fontgr);
							_layout.set_item_size(8.0_fontgr, 1.0_fontgr);
							_layout.set_background_color(st.Section3BackgroundColor);
							})
							.label("Text Editor", [](label_control& _control) {
							_control.set_size(15.0_fontgr, 50.0_px);
								})
								.press_button(IDC_BOLD, "B", [](pressbutton_control& _control) { _control.text_style.bold = true;  })
							.press_button( IDC_ITALIC, "I", [](pressbutton_control& _control) { _control.text_style.italics = true;  })
							.press_button(IDC_UNDERLINE, "U", [](pressbutton_control& _control) { _control.text_style.underline = true;  })
							.press_button(IDC_STRIKETHROUGH, "S", [](pressbutton_control& _control) { _control.text_style.strike_through = true;  })
						.end()
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

	auto& lb = test_app->find<listbox_control>(IDC_TEST_LISTBOX);

		list_data ldata3;
	ldata3.id_field = "Id";
	ldata3.text_field = "Name";
	ldata3.items = R"([
{ "Id":0, "Name":"Bentley" },
{ "Id":1, "Name":"Cadillac" },
{ "Id":2, "Name":"Scout" },
{ "Id":3, "Name":"Chevy SS" },
{ "Id":4, "Name":"GMC" }
])"_json;
	lb.set_list(ldata3);



	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

}
