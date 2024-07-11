
#ifndef APP_REPARATIONS_H
#define APP_REPARATIONS_H

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

	class reparations_application
	{
	public:

		json				data;

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

		reparations_application()
		{

			json_parser jp;

			application::get_application()->application_folder_name = "countryvideogames";
			application::get_application()->application_name = "reparations";

			std::cout << "Reparations Startup at " << std::filesystem::current_path() << std::endl;

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
					_presentation->checkPresentationFile();

					_presentation->set_home_page("home");

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
			application->runDialog(hInstance, "REPARATIONS BY TODD BANDROWSKY", IDI_REVOLUTION, fullScreen, presentation_layer);
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

	void run_reparations_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{

		if (strstr(lpszCmdParam, "-console")) {
			EnableGuiStdOuts();
			std::cout << "Reparations Console Started" << std::endl;
		}

		reparations_application app;

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
