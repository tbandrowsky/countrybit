
#ifndef APP_AURA_H
#define APP_AURA_H

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

	class aura_application
	{
	public:
		presentation_style* st;

		json				data;

		// the adapter, created first.  It is the graphics card on the machine
		std::shared_ptr<directXAdapter> factory;

		// This creates the application window (and children), handling the mapping from the window and the events
		// back through the presentation.
		std::shared_ptr<directApplicationWin32> appwin32;

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

		int id_camera_controls;
		int id_camera_select;
		int id_camera_detection_threshold;
		int id_camera_activation_area_percentage;
		int id_camera_activation_response;
		int id_camera_activation_cooldown;

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

		aura_application()
		{

			json_parser jp;
			data = jp.create_object();
			data.put_member_object("cameras");

			application::get_application()->application_folder_name = "countryvideogames";
			application::get_application()->application_name = "colorparty";

			st = presentation_style_factory::get_current()->get_style();

			std::cout << "Color Party Startup at " << std::filesystem::current_path() << std::endl;

			samples::test();

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
					id_camera_controls = _presentation->get_control_id("camera_controls", []() { return id_counter::next(); });
					id_camera_select = _presentation->get_control_id("camera_select", []() { return id_counter::next(); });
					id_camera_detection_threshold = _presentation->get_control_id("camera_activation_threshold", []() { return id_counter::next(); });
					id_camera_activation_area_percentage = _presentation->get_control_id("camera_activation_area", []() { return id_counter::next(); });
					id_camera_activation_response = _presentation->get_control_id("camera_activation_response", []() { return id_counter::next(); });
					id_camera_activation_cooldown = _presentation->get_control_id("camera_activation_cooldown", []() { return id_counter::next(); });

					_presentation->create_page("user_aura", [this](page& _page) {
						create_user_license_page(_page);
						});

					_presentation->set_home_page("user_aura");

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
					auto result =  f.read(0, b.get_ptr(), fsize);
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
								return true;
							}
						}
					}
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}
			return false;
		}

		/*

		This is the run method.  It reads the configuration for the application and then launches the application window.
		Run dialog is a bit of a misnomer, in that, this is just a standard window that uses the dialogproc to handle
		keyboard navigation.

		*/

		void run(HINSTANCE hInstance, bool fullScreen)
		{
			application->runDialog(hInstance, "COLOR PARTY BY TODD BANDROWSKY", IDI_REVOLUTION, fullScreen, presentation_layer);
		}

		using content_function = std::function<void(control_builder& _contents)>;

		void create_page_frame(page& _page, content_function _fn, bool _enable_command_container)
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
					_settings.set_style(st->PageStyle);
				});

			int title_id = 0;
			int* ptitle_id = &title_id;

			contents.caption_bar(id_caption_bar, [this, ptitle_id](caption_bar_control& _cb)
				{
					_cb.set_size(1.0_container, 100.0_px);
					_cb.menu_button_id = IDC_SYSTEM_MENU;
					_cb.image_control_id = IDC_COMPANY_LOGO;
					_cb.image_file = "small_logo.png";
					_cb.subtitle_name = "    BANDROWSKY";
					_cb.title_name = "  COLOR PARTY";
					_cb.set_style(st->CaptionStyle);
					_cb.menu = application_menu.get();
					*ptitle_id = _cb.title_id;
				}
			);

			auto document_row = contents.row_begin(id_document_container, [this](row_layout& rl) {
				rl.set_size(1.0_container, 1.0_remaining);
				rl.item_next_space = 8.0_px;
				});

			if (_enable_command_container) {
				command_container = document_row.column_begin(id_document_command_container, [this](column_layout& rl) {
					rl.set_size(150.0_px, 1.0_container);
					});
			}

			document_body = document_row.column_begin(id_document_body_container, [title_id](column_layout& rl) {
				rl.set_size(1.0_remaining, 1.0_container);
				rl.push(title_id, true, false, false, false);
				});

			if (_enable_command_container) {
				right_command_container = document_row.column_begin(id_document_right_command_container, [this](column_layout& rl) {
					rl.set_size(150.0_px, 1.0_container);
					});
			}

			control_builder doc_details = document_body.column_begin(id_document_body_details, [title_id](column_layout& rl) {
				rl.set_size(1.0_container, 1.0_remaining);
				rl.set_margin(0.0_px);
				rl.push(title_id, true, false, false, false);
				});
			_fn(doc_details);

			contents_root.apply_controls(_page.root.get());
		}

		void create_user_license_page(
			page& _page
		)
		{
			create_page_frame(_page, [this](control_builder& cb)
				{
					control_builder root_row = cb.row_begin(id_counter::next(), [](row_layout& _settings) {
						_settings.set_size(1.0_container, 1.0_container);
						_settings.item_next_space = 0.0_px;
						});

					control_builder camera_column = root_row.column_begin(id_counter::next(), [](column_layout& _settings) {
						_settings.set_size(.8_container, 1.0_container);
						});

					camera_column.camera(id_camera, [this](camera_control& _settings) {
						_settings.set_padding(8.0_px);
						_settings.set_size(1.0_container, 1.0_container);
						_settings.camera_changed = [this](camera_control* _src) {
							std::vector<std::string> camera_list = _src->get_cameras();
							listbox_control& cbo = _src->find<listbox_control>(id_camera_select);
							json_parser jp;
							list_data ld;
							ld.id_field = "id";
							ld.text_field = "camera";
							ld.items = jp.create_array();
							std::cout << "Camera changed" << std::endl;
							json camera_settings;
							camera_settings = data["cameras"];

							for (int i = 0; i < camera_list.size(); i++) 
							{
								std::string& camera_name = camera_list[i];
								json jo = jp.create_object();
								jo.put_member("id", i);
								jo.put_member("camera", camera_name);
								std::cout << camera_name << std::endl;

								if (!camera_settings.has_member(camera_name))
								{
									json new_settings = jp.create_object();

									new_settings.put_member("camera_name", camera_name);
									new_settings.put_member("detection_threshold", _src->get_detection_threshold() );
									new_settings.put_member("activation_area", _src->get_activation_area_percentage());
									new_settings.put_member("detection_pulse", _src->get_detection_pulse());
									new_settings.put_member("detection_cooldown", _src->get_detection_cooldown());

									camera_settings.put_member(camera_list[i], new_settings);
								}

								ld.items.append_element(jo);
							}
							cbo.set_list(ld);
						};
					});

					control_builder preview_column = root_row.column_begin(id_counter::next(), [](column_layout& _settings) {
						_settings.set_size(.2_container, 1.0_container);
						_settings.set_content_align(visual_alignment::align_far);
						});

					control_builder camera_controls = preview_column.form(id_camera_controls, [this](form_control& _settings) {
						_settings.set_padding(8.0_px);
						_settings.set_size(0.92_container, 500.0_px);
						_settings.set_column_width(0.5_container);
						_settings.on_changed = [this](int _control_id, form_control* _fv) {
							camera_control& cc = _fv->find<camera_control>(id_camera);
							json form_data = _fv->get_data();
							std::string cam = form_data["camera_name"];

							if (_control_id == id_camera_select) {
								cc.select_camera(cam);
								json new_cam = data["cameras"][cam];
								_fv->set_data(new_cam);
							}
							else 
							{
								double temp = form_data["detection_threshold"];
								cc.set_detection_threshold(temp);
								temp = form_data["activation_area"];
								cc.set_activation_area_percentage(temp);
								temp = form_data["detection_pulse"];
								cc.set_detection_pulse(temp);
								temp = form_data["detection_cooldown"];
								cc.set_detection_cooldown(temp);
								data["cameras"].put_member(cam, form_data);
							}
						};

						form_model ids;

						form_field new_field;

						new_field.field_id = id_camera_select;
						new_field.source_list.id_field = "Id";
						new_field.source_list.text_field = "Name";
						new_field.field_type = "listbox";
						new_field.label_text = "Camera Select";
						new_field.json_field_name = "camera_name";
						ids.fields.push_back(new_field);

						new_field.field_id = id_camera_detection_threshold;
						new_field.field_type = "number";
						new_field.label_text = "Detection";
						new_field.json_field_name = "detection_threshold";
						ids.fields.push_back(new_field);

						new_field.field_id = id_camera_activation_area_percentage;
						new_field.field_type = "number";
						new_field.label_text = "Area";
						new_field.json_field_name = "activation_area";
						ids.fields.push_back(new_field);

						new_field.field_id = id_camera_activation_response;
						new_field.field_type = "number";
						new_field.label_text = "Response";
						new_field.json_field_name = "detection_pulse";
						ids.fields.push_back(new_field);

						new_field.field_id = id_camera_activation_cooldown;
						new_field.field_type = "number";
						new_field.label_text = "Cooldown";
						new_field.json_field_name = "detection_cooldown";
						ids.fields.push_back(new_field);

						_settings.set_model(ids);
						});

					preview_column.label("  Detection", [](label_control& _lc) {
						_lc.set_margin(8.0_px);
						_lc.set_size(1.0_container, 45.0_px);
					});

					preview_column.camera_view([this](camera_view_control& _settings) {
						_settings.set_padding(8.0_px);
						_settings.set_size(1.0_container, 1.0_remaining);
						_settings.camera_control_id = id_camera;
						});

					preview_column.authorscredit("By Todd Bandrowsky\nCountry Video Games\nBowling Green KY, USA", 
						[this](authorscredit_control& _settings) {
						_settings.set_padding(8.0_px);
						_settings.set_size(1.0_container,100.0_px);

						});
				}, 
				false);
		}

		void set_corona_handlers()
		{
			corona_api->on_post_request = [this](int _windows_id, std::string _function_name, json& _credentials, json& _payload)
				{
				};

			corona_api->on_post_response = [this](int _windows_id, call_status _status, std::string _function_name, json& _credentials, json& _payload)
				{
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

		if (strstr(lpszCmdParam, "-console")) {
			EnableGuiStdOuts();
			std::cout << "ColorParty Console Started" << std::endl;
		}

		aura_application app;

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

		DisableGuiStdOuts();
	}

}

#endif
