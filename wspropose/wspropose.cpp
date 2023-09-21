
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

	const int IDM_VIEW = 5001;
	const int IDM_VIEW_FULL_LOGIN = 5003;
	const int IDM_VIEW_CREATE_ACCOUNT = 5004;
	const int IDM_VIEW_CONFIRM_ACCOUNT = 5005;
	const int IDM_VIEW_ACCESS_DENIED = 5006;
	const int IDM_VIEW_CUSTOMER_HOME = 5007;
	const int IDM_VIEW_TEST = 5008;
	const int IDM_ACCOUNT = 5101;
	const int IDM_ACCOUNT_LOGOUT = 5102;
	const int IDM_ACCOUNT_LOGIN = 5103;

	const int IDM_SETTINGS = 5201;

	const int IDM_COMPANY = 5301;
	const int IDM_COMPANY_ABOUT = 5302;
	const int IDM_COMPANY_HOME = 5303;
	const int IDM_COMPANY_WIKIPEDIA = 5304;

	menu_item app_menu;

	app_menu.begin_submenu(IDM_COMPANY, "Woodruff &Sawyer")
		.item(IDM_COMPANY_ABOUT, "A&bout")
		.item(IDM_COMPANY_HOME, "H&ome")
		.item(IDM_COMPANY_WIKIPEDIA, "Wiki&pedia")
	.end()
	.begin_submenu(IDM_VIEW, "&View")
		.destination(IDM_VIEW_FULL_LOGIN, "&Login", "full_login")
		.destination(IDM_VIEW_CREATE_ACCOUNT, "&Create Account", "create_account")
		.destination(IDM_VIEW_CONFIRM_ACCOUNT, "C&onfirm Account", "confirm_account")
		.destination(IDM_VIEW_ACCESS_DENIED, "Access &Denied", "access_denied")
		.destination(IDM_VIEW_CUSTOMER_HOME, "C&ustomer Home", "customer_home")
		.destination(IDM_VIEW_TEST, "Control &Test", "control_test1")
	.end()
	.begin_submenu(IDM_ACCOUNT, "&Account")
		.item(IDM_ACCOUNT_LOGOUT, "L&ogout")
		.item(IDM_ACCOUNT_LOGIN, "Log&in")
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

	const int IDC_LOGIN_MANUAL = 1201;
	const int IDC_LOGIN_USERNAME = 1202;
	const int IDC_LOGIN_PASSWORD = 1203;
	const int IDC_LOGIN_FORGOT = 1204;
	const int IDC_LOGIN_CANCEL = 1205;
	const int IDC_CREATE_NEW_ACCOUNT = 1206;
	const int IDC_LOGIN_FIRST_NAME = 1207;
	const int IDC_LOGIN_LAST_NAME = 1208;
	const int IDC_LOGIN_ID_NUMBER = 1209;
	const int IDC_LOGIN_STATE = 1210;

	test_app->create_page("full_login")
		.column_begin(IDC_HOME)
			.caption_bar(
				st,
				IDC_TITLE_BAR,
				IDC_SYSTEM_MENU,
				app_menu,
				IDC_IMAGE_LOGO,
				"assets\\small_logo.png",
				"WOODRUFF SAWYER",
				IDC_PLATFORM_TEST,
				"CONNECT PROPERTY",
				"Full Login"
			)
		.form_double_column(IDC_PLATFORM_TEST, "Full Login", [push_button_defaults](container_control& c) {
				c.chaptertitle("Identification");
				c.label("Scan RealID");
				c.subtitle("Placeholder for Camera");
				c.label("First Name");
				c.edit(IDC_LOGIN_FIRST_NAME);
				c.label("Last Name");
				c.edit(IDC_LOGIN_LAST_NAME);
				c.label("State");
				c.edit(IDC_LOGIN_STATE);
				c.label("ID Number");
				c.edit(IDC_LOGIN_ID_NUMBER);
			},
		[push_button_defaults](container_control& c2) {
				c2.chaptertitle("Authentication");
				c2.label("Enter Password");
				c2.label("Enter Password");
			}
			
			)
	.end();

	const int IDC_FIRST_NAME = 1301;
	const int IDC_LAST_NAME = 1302;
	const int IDC_EMAIL = 1303;
	const int IDC_PHONE = 1304;
	const int IDC_NEW_PASSWORD1 = 1305;
	const int IDC_NEW_PASSWORD2 = 1306;
	const int IDC_CREATE_ACCOUNT = 1307;
	const int IDC_CANCEL_ACCOUNT = 1308;

	test_app->create_page("create_account")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Create New Account"
		)
		.form_single_column(IDC_PLATFORM_TEST, "Create New Account", [push_button_defaults](container_control& c) {
			c.chaptertitle("Create New Account");
			c.chaptersubtitle("Identification");
			c.label("Scan your RealID");
			c.subtitle("Placeholder for Camera");
			c.chaptersubtitle("About you");
			c.label("First Name");
			c.edit(IDC_FIRST_NAME);
			c.label("Last Name");
			c.edit(IDC_LAST_NAME);
			c.label("Email");
			c.edit(IDC_EMAIL);
			c.label("Phone");
			c.edit(IDC_PHONE);
			c.chaptertitle("Set password");
			c.label("Password");
			c.edit(IDC_NEW_PASSWORD1);
			c.label("Confirm Password");
			c.edit(IDC_NEW_PASSWORD2);
			c.push_button(IDC_CREATE_ACCOUNT, "Create Account");
			c.push_button(IDC_CANCEL_ACCOUNT, "Cancel");
			c.paragraph("To ensure all members of this community are real, legal Americans, this application uses your RealID.");
		})
		.end()
	.end();

	const int IDC_SEND_ACCOUNT_CONFIRM = 1400;
	const int IDC_EDIT_ACCOUNT_CONFIRM = 1401;
	const int IDC_ACCOUNT_CONFIRM = 1402;
	const int IDC_CANCEL_ACCOUNT_CONFIRM = 1403;

	test_app->create_page("confirm_account")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Confirm Account"
		)
		.form_single_column(IDC_PLATFORM_TEST, "Full Login", [push_button_defaults](container_control& c) {
			c.chaptertitle("Confirm Account");
			c.chaptersubtitle("Confirmation Code.");
			c.push_button(IDC_SEND_ACCOUNT_CONFIRM, "Send Code");
			c.label("Enter Code");
			c.edit(IDC_EDIT_ACCOUNT_CONFIRM);
			c.push_button(IDC_ACCOUNT_CONFIRM, "Confirm");
			c.push_button(IDC_CANCEL_ACCOUNT_CONFIRM, "Confirm");
			c.end();
			})
		.end();

	const int IDC_LOGIN_AGAIN = 1500;
	const int IDC_LOGIN_FORGOT_PASSWORD = 1501;
	const int IDC_PASSWORD = 1502;
	const int IDC_DO_LOGIN = 1503;

	test_app->create_page("access_denied")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Access Denied"
		)
		.form_single_column(IDC_PLATFORM_TEST, "Full Login", [push_button_defaults](container_control& c) {
			c.chaptertitle("Access Denied");
			c.chaptersubtitle("Access is denied.");
			c.push_button(IDC_LOGIN_AGAIN, "Try again");
			c.chaptersubtitle("You can try to reset your password here.");
			c.push_button(IDC_LOGIN_FORGOT, "Forgot password");
		})
	.end();

	const int IDC_SEND_PASSWORD_CHANGE_CODE = 1600;
	const int IDC_EDIT_PASSWORD_CHANGE_CODE = 1601;
	const int IDC_NEW_RESET_PASSWORD1 = 1602;
	const int IDC_NEW_RESET_PASSWORD2 = 1603;
	const int IDC_SET_NEW_PASSWORD = 1604;
	const int IDC_CANCEL_PASSWORD = 1606;

	test_app->create_page("reset_password")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Reset Password"
		)
		.form_single_column(IDC_PLATFORM_TEST, "Full Login", [push_button_defaults](container_control& c) {
			c.chaptertitle("Reset Password");
			c.chaptersubtitle("Confirmation Code.");
			c.push_button(IDC_SEND_PASSWORD_CHANGE_CODE, "Send Code");
			c.label("Enter Code");
			c.edit(IDC_EDIT_PASSWORD_CHANGE_CODE);
			c.chaptersubtitle("Create New Password.");
			c.label("New Password");
			c.edit(IDC_NEW_RESET_PASSWORD1);
			c.label("Confirm New Password");
			c.edit(IDC_NEW_RESET_PASSWORD2);
			c.push_button(IDC_SET_NEW_PASSWORD, "Set password");
			c.push_button(IDC_CANCEL_PASSWORD, "Cancel");
			})
		.end();


	const int IDC_TO_LOGIN = 1701;

	test_app->create_page("password_changed")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Password Changed"
		)
		.row_begin([](row_layout& r)
			{
				r.set_size(1.0_container, 1.0_container);
				r.set_content_align(visual_alignment::align_center);
			})
		.column_begin([](column_layout& r)
			{
				r.set_size(.50_container, 0.5_container);
			})
				.chaptertitle("Password Changed")
		.chaptersubtitle("Password changed successfully")
		.paragraph("Your password was reset just fine, and now you can login.")
		.push_button(IDC_TO_LOGIN, "Login")
		.end()
		.end()
		.end()
		.end();

	const int IDC_USER_NAVIGATION = 1801;
	const int IDC_USER_FRAME = 1802;

	test_app->create_page("customer_home")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_IMAGE_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			IDC_PLATFORM_TEST,
			"CONNECT PROPERTY",
			"Home"
		)
		.row_begin([](row_layout& r)
			{
				r.set_size(1.0_container, 1.0_container);
				r.set_content_align(visual_alignment::align_center);
			})
				.column_begin(IDC_USER_NAVIGATION, [st](column_layout& col) {
			col.set_size(.5_container, 1.0_container);
				})
			.chaptertitle("Projects")
			.chaptersubtitle("News")
			.chaptersubtitle("Current")
			.chaptersubtitle("Closed")
			.chaptertitle("Products")
			.chaptersubtitle("Home Page")
			.chaptersubtitle("Services")
			.chaptertitle("Property")
			.chaptersubtitle("Buildings")
			.chaptersubtitle("Vehicles")
			.chaptertitle("Find")
			.chaptersubtitle("Electricians")
			.chaptersubtitle("Lawyers")
			.chaptersubtitle("Plumbers")
			.chaptersubtitle("Landscapers")
			.chaptersubtitle("Contractor")
			.chaptersubtitle("Insurance")
			.chaptertitle("News")
			.chaptersubtitle("Social")
			.chaptersubtitle("Insurance")
			.end()
			.column_begin(IDC_USER_FRAME, [st](column_layout& col) {
					col.set_size(.5_container, 1.0_container);
				})
			.frame_begin()
				.paragraph( "Frame Contents")
			.end()
			.end()
			.end()
			.end()
			.end()
	.end();

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
	const int IDC_FONT_PICK = 1029;
	const int IDC_BOLD = 1030;
	const int IDC_ITALIC = 1031;
	const int IDC_UNDERLINE = 1032;
	const int IDC_STRIKETHROUGH = 1033;
	const int IDC_SUPERSCRIPT = 1034;
	const int IDC_SUBSCRIPT = 1035;
	const int IDC_FONT_SIZE = 1036;
	const int IDC_FONT_NAME = 1037;


	test_app->create_page("control_test1")
		.column_begin(IDC_HOME)
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
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
							.listview(IDC_TEST_LISTVIEW, [st](listview_control& lv) {
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
							})
						.set_item_size(1.0_container, 1.2_fontgr)
						.label("List Box")
						.set_item_size(1.0_container, 10.0_fontgr)
								.listbox(IDC_TEST_LISTBOX, [st](listbox_control& lb) {
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

							})
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
							.combobox(IDC_COMBO_BOX1, [st](combobox_control& cb) {
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
							})
						.label("Combo EX")
								.comboboxex(IDC_COMBO_BOXEX1, [st](comboboxex_control& cbex) {
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

							})
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




	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, false, test_app);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Technology Demonstrator", IDI_WSPROPOSE, true, test_app);
	}

}

