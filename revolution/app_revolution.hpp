
#ifndef APP_REVOLUTION_H
#define APP_REVOLUTION_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{


	/*
	
	This is a boilerplate entry point for a corona based Windows client application.
	It creates an application object, and illustrates how all the pre-requisites are tied together.
	
	*/

	/*
	
	These are general constants, and they are used to identify specific controls.
	The presentation system lets you use win32 controls and your own dx based controls interchangably.

	*/

	// important general
	const int IDC_COMPANY_NAME = 1001;
	const int IDC_COMPANY_LOGO = 1002;
	const int IDC_TITLE_BAR = 1003;
	const int IDC_SYSTEM_MENU = 1004;

	// user controls
	const int IDC_USER_NAME = 2001;
	const int IDC_USER_PASSWORD = 2002;
	const int IDC_USER_PASSWORD1 = 2003;
	const int IDC_USER_PASSWORD2 = 2004;
	const int IDC_USER_CONFIRMATION = 2005;


	const int IDC_USER_LASTNAME = 2021;
	const int IDC_USER_FIRSTNAME = 2022;
	const int IDC_USER_STREET1 = 2023;
	const int IDC_USER_STREET2 = 2024;
	const int IDC_USER_CITY = 2025;
	const int IDC_USER_STATE = 2026;
	const int IDC_USER_ZIP = 2027;
	const int IDC_USER_CITIZEN = 2028;
	const int IDC_USER_RACE = 2029;
	const int IDC_USER_SEX = 2030;
	const int IDC_USER_EYES = 2031;
	const int IDC_USER_HAIR = 2032;
	const int IDC_USER_HEIGHT = 2033;
	const int IDC_USER_WEIGHT = 2034;
	const int IDC_USER_IDENTITY = 2035;
	const int IDC_USER_VETERAN = 2036;

	const int IDC_BTN_CANCEL = 2101;
	const int IDC_BTN_LOGIN = 2102;
	const int IDC_BTN_LOGIN_START = 2103;
	const int IDC_BTN_LOGIN_SENDCODE = 2104;
	const int IDC_BTN_LOGIN_CONFIRM = 2105;
	const int IDC_BTN_OBJECTS_EDIT_SAVE = 2106;
	const int IDC_BTN_REVERT = 2107;
	const int IDC_BTN_NEXT = 2108;

	// menu options
	const int IDM_VIEW_MENU = 3001;
	const int IDM_HOME = 3002;
	const int IDM_LOGIN = 3003;

	// forms
	const int IDC_FORM_VIEW = 4001;

	// bits of status
	const int IDC_STATUS_MESSAGE = 5001;
	const int IDC_STATUS_DETAIL = 5002;

	/* And now, this is the application */

	class revolution_application
	{
	public:
		presentation_style* st;

		// the adapter, created first.  It is the graphics card on the machine
		std::shared_ptr<directXAdapter> factory;

		// This creates the application window (and children), handling the mapping from the window and the events
		// back through the presentation.
		std::shared_ptr<directApplicationWin32> application;

		// create a win32 set of windows that can use this factory
		// to make windows with a d2d / d3d image
		std::shared_ptr<menu_item> application_menu;

		// create the data store and bind the calico client to it.
		std::shared_ptr<corona_client> corona_api;

		// create the presentation - this holds the data of what is on screen, for various pages.
		std::shared_ptr<presentation> presentation_layer;


		std::string current_page;

		int id_caption_bar;
		int id_main_row;
		int id_document_container;
		int id_document_command_container;
		int id_document_right_command_container;
		int id_document_body_container;
		int id_document_body_details;
		int id_status;

		int id_location_title;
		int id_create_title;
		int id_form_view;
		int id_tab_view;
		int image_control_id;
		int id_camera;

		std::map<std::string, call_status> responses;
		json credentials;

		call_status				 status_recent;
		std::vector<call_status> status_history;

		revolution_application()
		{

			application::get_application()->application_folder_name = "countryvideogames";
			application::get_application()->application_name = "revolution";

			std::cout << "Revolution Client Startup at " << std::filesystem::current_path()  << std::endl;

			status_recent = {};

			MFStartup(MF_VERSION);

			factory = std::make_shared<directXAdapter>();

			// build the factory initially.  we may have occasion to call this 
			// in the future in the event of a system setting change or something.
			// otherwise.... I hate doing big things in constructors.
			factory->refresh();
			// create a win32 set of windows that can use this factory
			// to make windows with a d2d / d3d image
			application = std::make_shared<directApplicationWin32>(factory);

			// create the data store and bind the calico client to it.
			corona_api = std::make_shared<corona_client>("localhost", 7277);

			// create the presentation - this holds the data of what is on screen, for various pages.
			presentation_layer = std::make_shared<presentation>(application);

			application_menu = std::make_shared<menu_item>();

			application_menu->destination(IDM_HOME, "&Home", "home")
								.destination(IDM_LOGIN, "&Login", "login");


			file_transaction<relative_ptr_type> ftr = read_config("client.json");
			ftr.wait();

			/*
			
			create pages is like, a handler to initialize the system at the time the main window has finished creating.
			(WM_CREATE handler, basically).  In this way we can use this sort of like an init dialog.
			
			*/

			presentation_layer->create_pages = [this](presentation* _presentation)
			{
				id_caption_bar = _presentation->get_control_id("caption_bar", []() { return id_counter::next(); });
				id_main_row = _presentation->get_control_id("main_row", []() { return id_counter::next(); });
				id_document_container = _presentation->get_control_id("document_container", []() { return id_counter::next(); });
				id_document_command_container = _presentation->get_control_id("document_command_container", []() { return id_counter::next(); });
				id_document_right_command_container = _presentation->get_control_id("document_right_command_container", []() { return id_counter::next(); });
				id_document_body_container = _presentation->get_control_id("document_body_container", []() { return id_counter::next(); });
				id_location_title = _presentation->get_control_id("location_title", []() { return id_counter::next(); });
				id_create_title = _presentation->get_control_id("create_title", []() { return id_counter::next(); });
				id_form_view = _presentation->get_control_id("get_ui_form_view", []() { return id_counter::next(); });
				id_tab_view = _presentation->get_control_id("get_ui_tab_view", []() { return id_counter::next(); });
				image_control_id = _presentation->get_control_id("get_ui_logo", []() { return id_counter::next(); });
				id_document_body_details = _presentation->get_control_id("document_details", []() { return id_counter::next(); });
				id_status = _presentation->get_control_id("status", []() { return id_counter::next(); });
				id_camera = _presentation->get_control_id("camera", []() { return id_counter::next(); });

				_presentation->create_page("home", [](page& _page) {
					control_builder cb(_page.get_root_container());
					cb.title("Test");
					});

				_presentation->create_page("login_start", [this](page& _page) {
					create_login_start_page(_page);
					});

				_presentation->create_page("login_sendcode", [this](page& _page) {
					create_login_sendcode_page(_page);
					});

				_presentation->create_page("login_confirmcode", [this](page& _page) {
					create_login_confirmcode_page(_page);
					});

				_presentation->create_page("login_passwordset", [this](page& _page) {
					create_login_passwordset_page(_page);
					});

				_presentation->create_page("edit_object", [this](page& _page) {
					create_object_edit_page(_page);
					});

				_presentation->create_page("user_license", [this](page& _page) {
					create_user_license_page(_page);
					});

				_presentation->create_page("user_account", [this](page& _page) {
					create_user_account_page(_page);
					});

				_presentation->set_home_page("user_license");

				set_corona_handlers();
			};
		}

		/*
		
		This is a helper method to read a configuration file for the UI and point it.
		In this way, users can connect to multiple corona servers.
		
		*/

		file_transaction<relative_ptr_type> read_config(std::string _config_file_name)
		{
			json_parser jp;
			try {
				file f = application->open_file(_config_file_name, file_open_types::open_existing);
				if (f.success()) {
					auto fsize = f.size();
					buffer b(fsize + 1);
					auto result = co_await f.read(0, b.get_ptr(), fsize);
					if (result.success) {
						crypto crypter;
						if (b.is_safe_string()) {
							std::string s_contents = b.get_ptr();
							json temp_contents = jp.parse_object(s_contents);
							if (!jp.parse_errors.size()) {
								json corona_config = temp_contents["CoronaServer"];
								corona_api->host = corona_config["Host"];
								corona_api->path = corona_config["Path"];
								corona_api->port = corona_config["Port"];
								co_return true;
							}
						}
					}
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}
			co_return false;
		}

		/*
		
		This is the run method.  It reads the configuration for the application and then launches the application window.
		Run dialog is a bit of a misnomer, in that, this is just a standard window that uses the dialogproc to handle 
		keyboard navigation.

		*/

		void run(HINSTANCE hInstance, bool fullScreen)
		{
			application->runDialog(hInstance, "COUNTRYBIT REVOLUTION", IDI_REVOLUTION, fullScreen, presentation_layer);
		}

		using content_function = std::function<void(control_builder& _contents)> ;

		void create_page_frame(page& _page, content_function _fn)
		{
			control_builder command_container;
			control_builder right_command_container;
			control_builder document_body;
			caption_bar_control* caption_container;
			tab_view_control* tab_container;
			form_control* form_view;

			control_builder contents_root(_page.get_root_container());

			auto contents = contents_root.column_begin(
				id_main_row,
				[this](column_layout& _settings) {
					_settings.set_size(1.0_container, 1.0_container);
					_settings.background_brush = st->PageBackgroundBrush;
					_settings.border_brush = st->PageBorderBrush;
				});

			int title_id = 0;
			int *ptitle_id = &title_id;

			contents.caption_bar(id_caption_bar, st, application_menu.get(), [this, ptitle_id](caption_bar_control& _cb)
				{
					_cb.set_size(1.0_container, 100.0_px);
					_cb.menu_button_id = IDC_SYSTEM_MENU;
					_cb.image_control_id = IDC_COMPANY_LOGO;
					_cb.image_file = "small_logo.png";
					_cb.corporate_name = "COUNTRY VIDEO GAMES";
					_cb.title_name = "Revolution";
					_cb.background_brush = st->CaptionBackgroundBrush;
					_cb.border_brush = st->CaptionBorderBrush;
					*ptitle_id = _cb.title_id;
				}
			);

			auto document_row = contents.row_begin(id_document_container, [this](row_layout& rl) {
				rl.set_size(1.0_container, 1.0_remaining);
				rl.item_next_space = 8.0_px;
				});

			command_container = document_row.column_begin(id_document_command_container, [this](column_layout& rl) {
				rl.set_size(150.0_px, 1.0_container);
				});

			document_body = document_row.column_begin(id_document_body_container, [title_id](column_layout& rl) {
				rl.set_size(1.0_remaining, 1.0_container);
				rl.push(title_id, true, false, false, false);
				}); 

			right_command_container = document_row.column_begin(id_document_right_command_container, [this](column_layout& rl) {
				rl.set_size(150.0_px, 1.0_container);
				});

			// note that, we are putting the breadcrumbs on a nav pane to the left.
			control_builder doc_message = document_body.status(status_recent, [](status_control& _sc) {
				measure sc_height = 10.0_px;
				if (_sc.status_set) sc_height = 100.0_px;
				_sc.set_size(1.0_container, sc_height);
				_sc.set_margin(4.0_px);
				}, id_status);

			int status_id = id_status;

			control_builder doc_details = document_body.column_begin(id_document_body_details, [title_id, status_id](column_layout& rl) {
				rl.set_size(1.0_container, 1.0_remaining);
				rl.set_margin(4.0_px);
				rl.push(title_id, true, false, false, false);
				rl.push(status_id, true, false, false, false);
				});
			_fn(doc_details);

			contents_root.apply_controls(_page.root.get());
		}

		void create_login_start_page (
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder cb) 
			{
					cb.chaptertitle("Login");
					cb.chaptersubtitle("Enter your username and password.");					
					cb.form_view(IDC_FORM_VIEW, [this](form_control& _fv)
						{
							form_model ids;

							form_field iff;
							iff.field_id = IDC_USER_NAME;
							iff.label_text = "User name:";
							iff.field_type = "string";
							iff.json_member_name = "Name";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_PASSWORD;
							iff.label_text = "Password";
							iff.field_type = "string";
							iff.json_member_name = "Name";
							ids.fields.push_back(iff);
							ids.fn_buttons = [this](control_builder& cb)
								{
									cb.calico_button(IDC_BTN_LOGIN_START, [this](calico_button_control& cbc)
										{
											json_parser jp;
											cbc.options = {};
											cbc.options.function_data = jp.create_object();
											cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
											cbc.options.corona_client = corona_api.get();
											cbc.options.function_name = "/login/start/";
											cbc.options.credentials = jp.create_object();
											cbc.is_default_button = true;
											cbc.set_text("Login");
										});
									cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
										{
											cbc.set_text("&Cancel");
										});

								};

						_fv.fields_per_column = 3;
						_fv.set_size(1.0_container, 200.0_px);
						_fv.set_data(ids);

					});
			});

			return;
		}

		void create_login_sendcode_page(
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder cb)
				{
					cb.chaptertitle("Send Login Code");
					cb.chaptersubtitle("Bring up your email, and click send code.  You'll receive a code that you will enter to login.  This extra step helps protect you.");
					cb.calico_button(IDC_BTN_LOGIN_SENDCODE, [this](calico_button_control& cbc)
						{
							json_parser jp;
							cbc.options = {};
							cbc.options.corona_client = corona_api.get();
							cbc.options.function_name = "/login/sendcode/";
							cbc.options.credentials = credentials;
						});
					cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
						{
							cbc.set_text("&Cancel");
						});
				});

		}

		void create_login_confirmcode_page (
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.chaptertitle("Enter Confirmation Code");
					cb.chaptersubtitle("An email was sent to you.  Please enter the code you received.");
					cb.form(IDC_FORM_VIEW, [this](form_control& _fv)
						{
							form_model ids;
							form_field iff;
							iff.field_id = IDC_USER_CONFIRMATION;
							iff.label_text = "Confirmation Code:";
							iff.field_type = "string";
							iff.json_member_name = "ConfirmationCode";
							ids.fields.push_back(iff);
							ids.fn_buttons = [this](control_builder& cb) {
								cb.calico_button(IDC_BTN_LOGIN_CONFIRM, [this](calico_button_control& cbc)
									{
										json_parser jp;
										cbc.options = {};
										cbc.options.corona_client = corona_api.get();
										cbc.options.function_name = "/login/confirmcode/";
										cbc.options.credentials = credentials;
										cbc.options.function_data = jp.create_object();
										cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
									});
								cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
									{
										cbc.set_text("&Cancel");
									});
								};

							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, .15_container);
							_fv.set_data(ids);
						});
				});
		}

		void create_user_license_page(
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.chaptertitle("Enlistment");
					cb.chaptersubtitle("Please present your US RealID driver's license");

					control_builder root_row = cb.row_begin(id_counter::next(), [](row_layout& _settings) {
						_settings.set_size(1.0_container, 1.0_container);
						_settings.item_next_space = 8.0_px;
						});

					control_builder camera_column = root_row.column_begin(id_counter::next(), [](column_layout& _settings) {
						_settings.set_size(.5_container, 1.0_container);
						});

					camera_column.camera(id_camera, [](camera_control& _settings) {
						_settings.set_size(1.0_container, 1.0_aspect);
						});

					control_builder preview_column = root_row.column_begin(id_counter::next(), [](column_layout& _settings) {
						_settings.set_size(.5_container, 1.0_container);
						});

					preview_column.camera_view([this](camera_view_control& _settings) {
						_settings.set_size(1.0_container, 1.0_aspect);
						_settings.camera_control_id = id_camera;
						});

				});

				_page.on_command(IDC_BTN_NEXT, [](command_event evt) {

					});

		}

		void create_user_account_page(
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.chaptertitle("Account Settings");

					control_builder root_row = cb.row_begin(id_counter::next(), [](row_layout& _settings) {
						_settings.set_size(1.0_container, 1.0_container);
						_settings.item_next_space = 8.0_px;
						});

					control_builder form_column = root_row.form(IDC_FORM_VIEW, [this](form_control& _fv)
						{
							_fv.set_size( 1.0_container, 1.0_container);

							form_model ids;
							form_field iff;

							iff.field_id = id_counter::next();
							iff.label_text = "Name";
							iff.field_type = "section";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_LASTNAME;
							iff.label_text = "Last Name";
							iff.field_type = "string";
							iff.json_member_name = "LastName";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_FIRSTNAME;
							iff.label_text = "First Name:";
							iff.field_type = "string";
							iff.json_member_name = "FirstName";
							ids.fields.push_back(iff);

							iff.field_id = id_counter::next();
							iff.label_text = "Address";
							iff.field_type = "section";

							iff.field_id = IDC_USER_STREET1;
							iff.label_text = "Street 1:";
							iff.field_type = "string";
							iff.json_member_name = "Street1";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_STREET2;
							iff.label_text = "Street 2:";
							iff.field_type = "string";
							iff.json_member_name = "Street2";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_CITY;
							iff.label_text = "City:";
							iff.field_type = "string";
							iff.json_member_name = "City";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_STATE;
							iff.label_text = "State:";
							iff.field_type = "State";
							iff.json_member_name = "State";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_ZIP;
							iff.label_text = "Zip:";
							iff.field_type = "string";
							iff.json_member_name = "Zip";
							ids.fields.push_back(iff);

							iff.field_id = id_counter::next();
							iff.label_text = "Service";
							iff.field_type = "section";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_VETERAN;
							iff.label_text = "Veteran:";
							iff.field_type = "string";
							iff.json_member_name = "Veteran";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_CITIZEN;
							iff.label_text = "Citizenship:";
							iff.field_type = "string";
							iff.json_member_name = "Citizenship";
							ids.fields.push_back(iff);

							iff.field_id = id_counter::next();
							iff.label_text = "Description";
							iff.field_type = "section";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_RACE;
							iff.label_text = "Race:";
							iff.field_type = "string";
							iff.json_member_name = "Race";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_SEX;
							iff.label_text = "Sex:";
							iff.field_type = "string";
							iff.json_member_name = "Sex";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_EYES;
							iff.label_text = "Eyes:";
							iff.field_type = "string";
							iff.json_member_name = "Eyes";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_HAIR;
							iff.label_text = "Hair:";
							iff.field_type = "string";
							iff.json_member_name = "Hair";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_HEIGHT;
							iff.label_text = "Height:";
							iff.field_type = "string";
							iff.json_member_name = "Password2";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_WEIGHT;
							iff.label_text = "Weight:";
							iff.field_type = "string";
							iff.json_member_name = "Password2";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_IDENTITY;
							iff.label_text = "Identity:";
							iff.field_type = "string";
							iff.json_member_name = "Identity";
							ids.fields.push_back(iff);

							ids.fn_buttons = [this](control_builder& cb) {
								cb.push_button(IDC_BTN_NEXT, "&Next", [this](pushbutton_control& cbc)
									{
										json_parser jp;
									});
								cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
									{
										cbc.set_text("&Cancel");
									});
								};

							_fv.fields_per_column = 8;
							_fv.column_start_space = 0.0_px;
							_fv.column_next_space = 8.0_px;
							_fv.set_size(1.0_container, 1.0_container);
							_fv.set_data(ids);
						});
				});

			_page.on_command(IDC_BTN_NEXT, [](command_event evt) {

				});
		}

		void create_newuser_user_page(
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.chaptertitle("Enter New Password");
					cb.chaptersubtitle("Please enter the code that you were emailed.");

					cb.form_view(IDC_FORM_VIEW, [this](form_control& _fv)
						{
							form_model ids;
							form_field iff;

							iff.field_id = IDC_USER_PASSWORD1;
							iff.label_text = "Password 1:";
							iff.field_type = "string";
							iff.json_member_name = "Password1";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_PASSWORD2;
							iff.label_text = "Password 2:";
							iff.field_type = "string";
							iff.json_member_name = "Password2";
							ids.fields.push_back(iff);

							ids.fn_buttons = [this](control_builder& cb) {
								cb.calico_button(IDC_BTN_LOGIN, [this](calico_button_control& cbc)
									{
										json_parser jp;
										cbc.options = {};
										cbc.options.corona_client = corona_api.get();
										cbc.options.function_name = "/login/passwordset/";
										cbc.options.credentials = credentials;
										cbc.options.function_data = jp.create_object();
										cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
									});
								cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
									{
										cbc.set_text("&Cancel");
									});
								};

							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, 400.0_px);
							_fv.set_data(ids);
						});
				});

		}

		void create_login_passwordset_page (
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.chaptertitle("Enter New Password");
					cb.chaptersubtitle("Please enter the code that you were emailed.");

					cb.form_view(IDC_FORM_VIEW, [this](form_control& _fv)
						{
							form_model ids;
							form_field iff;

							iff.field_id = IDC_USER_PASSWORD1;
							iff.label_text = "Password 1:";
							iff.field_type = "string";
							iff.json_member_name = "Password1";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_PASSWORD2;
							iff.label_text = "Password 2:";
							iff.field_type = "string";
							iff.json_member_name = "Password2";
							ids.fields.push_back(iff);

							ids.fn_buttons = [this](control_builder& cb) {
								cb.calico_button(IDC_BTN_LOGIN, [this](calico_button_control& cbc)
									{
										json_parser jp;
										cbc.options = {};
										cbc.options.corona_client = corona_api.get();
										cbc.options.function_name = "/login/passwordset/";
										cbc.options.credentials = credentials;
										cbc.options.function_data = jp.create_object();
										cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
									});
								cb.calico_button(IDC_BTN_CANCEL, [this](calico_button_control& cbc)
									{
										cbc.set_text("&Cancel");
									});
								};

							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, 400.0_px);
							_fv.set_data(ids);
						});
				});

		}

		void create_object_edit_page(
			page& _page
		) 
		{
			json_parser jp;
			control_builder command_container;
			caption_bar_control* caption_container;
			tab_view_control* tab_container;
			form_control* form_view;
			std::vector<tab_pane> tabs;

			// then we must be a new page

			create_page_frame(_page, [this](control_builder& cb)
				{
					cb.form_view(IDC_FORM_VIEW, [](form_control& _fv)
						{
							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, .15_container);							
						});
				});

			if (!this->responses.contains("/objects/edit/")) {
				return;
			}
			auto response_data = responses["/objects/edit/"];
			json response_json = jp.parse_object(response_data.response);

			if (!response_json["Success"]) {
				return;
			}

			command_container = _page.edit(id_document_command_container);
			caption_container = _page.find_container<caption_bar_control>(id_caption_bar);
			tab_container = _page.find_container<tab_view_control>(id_tab_view);

			command_container.chaptertitle("Create New", [](chaptertitle_control& ct) {
				ct.set_size(.95_container, 50.0_px);
				}, id_create_title);

			// first we put a caption bar in our standard page and our stock form.

			json edited_data;
			json edited_class;
			json edited_fields;
			json edited_build;
			std::string class_name;


			json token = response_json["Token"];
			edited_data = response_json["Data"];
			edited_class = edited_data["ClassDefinition"];
			class_name = edited_class["ClassName"];
			edited_fields = edited_class["Fields"];
			edited_build = edited_class["Edit"];

			form_model ids;
			ids.name = edited_class["Description"];
			ids.data = edited_data;
			ids.fn_buttons = [this](control_builder& cb) {
				cb.calico_button(IDC_BTN_OBJECTS_EDIT_SAVE, [this](calico_button_control& cbc)
					{
						json_parser jp;
						cbc.options = {};
						cbc.options.corona_client = corona_api.get();
						cbc.options.function_name = "/objects/put/";
						cbc.options.credentials = credentials;
						cbc.options.function_data = jp.create_object();
						cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
					});
				cb.calico_button(IDC_BTN_REVERT, [this](calico_button_control& cbc)
					{
						json_parser jp;
						cbc.options = {};
						cbc.options.corona_client = corona_api.get();
						cbc.options.function_name = "/objects/edit/";
						cbc.options.credentials = credentials;
						cbc.options.function_data = jp.create_object();
						cbc.options.function_data.put_member_i64("SourceControlId", IDC_FORM_VIEW);
					});
				};

			auto members = edited_fields.get_members();
			for (auto field : members)
			{
				std::string field_name;
				std::string field_type;
				json field_choices;
				json field_options;
				form_field new_field;

				field_name = field.first;

				new_field.json_member_name = field_name;
				new_field.read_only = false;
				std::string control_name = "fv_" + class_name + "_" + field_name;
				new_field.field_id = presentation_layer->get_control_id(control_name, []() { return id_counter::next(); });

				if (field.second.is_object())
				{
					field_options = field.second;
					field_type = field_options["FieldType"];
					field_choices = field_options["Choices"];

					new_field.label_text = field_options["Description"];
					new_field.tooltip_text = field_options["ToolTip"];
					new_field.choice_options.id_field = field_choices["ValueField"];
					new_field.choice_options.text_field = field_choices["NameField"];
					new_field.choice_options.items = field_choices["Items"];
				}
				else
				{
					field_type = field.second;
					new_field.label_text = field_name;
					new_field.tooltip_text = "";
				}

				if (field_choices.is_object())
				{
					new_field.field_type = "combobox";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "currency") {
					//edit_currency
					new_field.field_type = "currency";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "datetime") {
					//datetimepicker_field
					new_field.field_type = "datetime";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "double") {
					//edit_double
					new_field.field_type = "double";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "int64") {
					new_field.field_type = "integer";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "string") {
					new_field.field_type = "string";
					ids.fields.push_back(new_field);
				}
				else if (field_type == "object")
				{
					tab_pane object_tab;
					std::string control_name = "tb_" + class_name + "_" + field_name;

					object_tab.id = presentation_layer->get_control_id(control_name, []() { return id_counter::next(); });
					object_tab.name = class_name;

				}
				else if (field_type == "array")
				{
					tab_pane array_tab;
					std::string control_name = "tb_" + class_name + "_" + field_name;

					array_tab.id = presentation_layer->get_control_id(control_name, []() { return id_counter::next(); });
					array_tab.name = class_name;

					array_data_source ads;
					json_parser jp;

					ads.data = edited_data[field_name];
					if (ads.data.is_empty()) {
						ads.data = jp.create_array();
					}

					json child_create_options;

					if (edited_build.has_member(field_name))
					{
						child_create_options = edited_build[field_name];
						if (child_create_options.is_array())
						{
							for (int i = 0; i < child_create_options.size(); i++)
							{
								json option_item = child_create_options.get_element(i);
								std::string function_name = option_item["Function"];
								if (function_name == "/objects/create/")
								{
									array_tab.create_objects.push_back(option_item["Data"]);
								}
								else if (function_name == "/classes/create/")
								{
									array_tab.create_classes.push_back(option_item["Data"]);
								}
							}
						}
					}

					ads.assets = [this](draw_control* _parent, rectangle _bounds) -> void
						{
							if (auto win = _parent->window.lock()) {
								auto& ctxt = win->getContext();

								if (st->ParagraphFont) {
									ctxt.setTextStyle(st->ParagraphFont.get());
								}

								solidBrushRequest sbr;
								sbr.brushColor = toColor("#000000");
								sbr.name = "item_foreground";
								ctxt.setSolidColorBrush(&sbr);

								sbr.brushColor = toColor("#FFFFFF");
								sbr.name = "item_background";
								ctxt.setSolidColorBrush(&sbr);

								sbr.brushColor = toColor("#C0C0C0");
								sbr.name = "item_border";
								ctxt.setSolidColorBrush(&sbr);
							}
							return;
						};

					ads.draw_item = [ads, this](draw_control* _parent, int _index, rectangle _bounds) -> void {
						auto json_object = ads.data.get_element(_index);
						if (json_object.has_member("Source")) {
							json_object = json_object["Source"];
						}
						auto object_members = json_object.get_members();
						double x = _bounds.x;
						double w = 0.0;
						if (auto win = _parent->window.lock()) {
							for (auto om : object_members) {
								w = 100;
								auto& ctxt = win->getContext();
								ctxt.drawRectangle(&_bounds, "item_border", 1, nullptr);
								std::string text = om.second->to_string();
								auto field_bounds = _bounds;
								field_bounds.x = x;
								ctxt.drawText(text, &field_bounds, "item_paragraph", "item_foreground");
								x += w;
							}
						}
						};

					ads.size_item = [](draw_control* _parent, int _index, rectangle _bounds) -> point {
						point p(_bounds.w, 40.0, 0.0);
						return p;
						};


					array_tab.apply_data = [ads](tab_pane& _pane, control_base* _cont) {
						grid_view* gv = dynamic_cast<grid_view*>(_cont);
						if (gv) {
							gv->set_item_source(ads);
						}
						};

					corona_client* corapi = corona_api.get();

					array_tab.create_tab_controls = [ads, corapi, token](tab_pane& _pane, control_base* _cont) {
						control_builder cb;
						cb.row_begin(id_counter::next(), [_pane, corapi, token](row_layout& _rl) {
							for (auto& co : _pane.create_objects)
							{
								if (co.is_object()) {
									control_builder rb;
									rb.calico_button(id_counter::next(), [co, corapi, token](calico_button_control& _cb) {
										_cb.options.corona_client = corapi;
										_cb.options.credentials = token;
										_cb.options.function_name = "/objects/create/";
										_cb.options.function_data = co["Data"];
										});
								}
							}

							for (auto& cc : _pane.create_classes)
							{
								if (cc.is_object()) {
									control_builder rb;
									rb.calico_button(id_counter::next(), [cc, corapi, token](calico_button_control& _cb) {
										std::string option_name = "Subclass " + cc["Data"]["BaseClassName"];
										_cb.set_text(option_name);
										_cb.options.corona_client = corapi;
										_cb.options.credentials = token;
										_cb.options.function_name = "/classes/create/";
										_cb.options.function_data = cc["Data"];
										});
								}
							}
							});
						cb.end();
						cb.grid_view_begin(id_counter::next(), [ads](grid_view& _layout) {
							_layout.set_item_source(ads);
							_layout.set_size(1.0_container, 1.0_remaining);
							});
						cb.apply_controls(_cont);
						};


					tabs.push_back(array_tab);
				}
			}

			auto fv = presentation_layer->get_control<form_control>(id_form_view);
			if (fv) {
				fv->set_data(ids);
			}
			tab_container->set_tabs(tabs);
		}

		void set_corona_handlers()
		{
			corona_api->on_post_request = [this](int _windows_id, std::string _function_name, json& _credentials, json& _payload)
				{
					auto ctrl = presentation_layer->find_ptr<windows_control>(_windows_id);
					if (ctrl) {
						ctrl->disable();
					}
				};

			corona_api->on_post_response = [this](int _windows_id, call_status _status, std::string _function_name, json& _credentials, json& _payload)
				{
					json response_data;

					responses.insert_or_assign(_function_name, _status);
					status_recent = _status;

					status_control *status_ctrl = presentation_layer->find_ptr<status_control>(id_status);
					if (status_ctrl) {
						status_ctrl->set_status(status_recent);
					}

					if (_status.success) {
						json_parser jp;
						response_data = jp.parse_object(_status.response);
						if (response_data.has_member("Token")) {
							credentials = response_data["Token"];
						}
					}

					auto ctrl = presentation_layer->find_ptr<windows_control>(_windows_id);
					if (ctrl) {
						ctrl->enable();
					}

					if (_function_name == "/login/start/") 
					{
						if (_status.success)
						{
							select_page("login_sendcode");
						}
						else
						{
							select_page("login_start");
						}
					}
					else if (_function_name == "/login/sendcode/")
					{
						if (_status.success)
						{
							select_page( "login_confirmcode" );
						}
						else
						{
							select_page("login_sendcode");
						}
					}
					else if (_function_name == "/login/confirmcode/") 
					{
						if (_status.success)
						{
							db_object_id_type object_id = {};
							json response;
							call_status status = corona_api->edit_object(credentials, object_id, response);
							select_page("edit_object");
						}
						else
						{
							select_page("login_confirmcode");
						}
					}
					else if (_function_name == "/login/passwordset/") 
					{
						if (_status.success)
						{
							select_page("login_start");
						}
						else
						{
							select_page("login_passwordset");
						}
					}
					else if (_function_name == "/objects/edit/")
					{
						select_page("object_edit");
					}
				};
		}

		void select_page(std::string _new_page)
		{
			if (current_page == _new_page) return;

			current_page = _new_page;

			threadomatic::run_complete(nullptr, [this]() -> void {
					presentation_layer->select_page(current_page);
				});
		}

	};

	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{

		EnableGuiStdOuts();

		revolution_application app;

		// and now wire the data to the presentation 
		// the presentation can invoke the data
		// the data invokes whatever client
		// json for analytics
		// off to C++ structures for heavy duty

		bool fullScreen = false;

		if (strstr(lpszCmdParam, "-fullscreen")) {
			fullScreen = true;
		}
		else if (strstr(lpszCmdParam, "-window")) {
			fullScreen = false;
		}

#if _DEBUG
		fullScreen = false;
#endif
		app.run(hInstance, fullScreen);
	}

}

#endif
