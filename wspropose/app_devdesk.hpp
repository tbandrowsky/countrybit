
#ifndef APP_DEVDESK_H
#define APP_DEVDESK_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{

	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{
		application app;

		EnableGuiStdOuts();

		std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
		factory->refresh();

		std::shared_ptr<directApplicationWin32> application = std::make_shared<directApplicationWin32>(factory);
		std::shared_ptr<calico_client> calico_application = std::make_shared<calico_client>();
		std::shared_ptr<data_plane> app_data;

		bool forceWindowed = false;

		if (strstr(lpszCmdParam, "-window")) {
			forceWindowed = true;
		}

#if _DEBUG
		forceWindowed = true;
#endif
		auto application_presentation = std::make_shared<presentation>(application);


		corona::menu_item app_menu;

		const int IDC_COMPANY_NAME = 1001;
		const int IDC_COMPANY_LOGO = 1002;
		const int IDC_TITLE_BAR = 1003;
		const int IDC_SYSTEM_MENU = 1004;

		const int IDM_VIEW_MENU = 2001;
		const int IDM_HOME = 2002;
		const int IDM_PRESENTATIONS = 2003;
		const int IDM_PRODUCTS = 2004;
		const int IDM_PROJECTS = 2005;
		const int IDM_TEAM = 2006;
		const int IDM_OBSTACLES = 2007;

		app_menu.destination(IDM_HOME, "Ho&me", "home")
			.destination(IDM_PRESENTATIONS, "Pre&sentations", "presentations")
			.destination(IDM_PRODUCTS, "Pro&ducts", "products")
			.destination(IDM_PROJECTS, "Proj&ect", "projects")
			.destination(IDM_TEAM, "T&eam", "team")
			.destination(IDM_OBSTACLES, "Se&ttings", "settings");

		auto st = styles.get_style();

		std::function<void(pushbutton_control& _set_defaults)> push_button_defaults = [](pushbutton_control& ctrl) {
			ctrl.set_size(150.0_px, 50.0_px);
			};

		application_presentation->create_page("home", [calico_application, application](page& _pg)
			{
				_pg.on_load([calico_application, application](page_load_event _evt)
					{
						DWORD thread_id_before = ::GetCurrentThreadId();
						std::string model_name = "Zoos";
						std::string user_name = application->getUserName();
						calico_application->login_async(model_name, user_name);
						DWORD thread_id_after = ::GetCurrentThreadId();
					}
				);
			})
			.column_begin()
			.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
					{
						_cb.title_bar_id = IDC_TITLE_BAR;
						_cb.menu_button_id = IDC_SYSTEM_MENU;
						_cb.image_control_id = IDC_COMPANY_LOGO;
						_cb.image_file = "assets\\small_logo.png";
						_cb.corporate_name = "WOODRUFF SAWYER";
						_cb.id_title_column_id = 0;
						_cb.title_name = "DEVELOPER STATION";
						_cb.subtitle_name = "Home";
					}
				)
				.end();

			application_presentation->create_page("presentations")
				.column_begin()
				.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
					{
						_cb.title_bar_id = IDC_TITLE_BAR;
						_cb.menu_button_id = IDC_SYSTEM_MENU;
						_cb.image_control_id = IDC_COMPANY_LOGO;
						_cb.image_file = "assets\\small_logo.png";
						_cb.corporate_name = "WOODRUFF SAWYER";
						_cb.id_title_column_id = 0;
						_cb.title_name = "DEVELOPER STATION";
						_cb.subtitle_name = "Presentations";
					}
				)
				.end();

			application_presentation->create_page("presentation")
				.column_begin()
				.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
					{
						_cb.title_bar_id = IDC_TITLE_BAR;
						_cb.menu_button_id = IDC_SYSTEM_MENU;
						_cb.image_control_id = IDC_COMPANY_LOGO;
						_cb.image_file = "assets\\small_logo.png";
						_cb.corporate_name = "WOODRUFF SAWYER";
						_cb.id_title_column_id = 0;
						_cb.title_name = "DEVELOPER STATION";
						_cb.subtitle_name = "Presentation Detail";
					}
				)
				.end();

			application_presentation->create_page("products")
				.column_begin()
				.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
					{
						_cb.title_bar_id = IDC_TITLE_BAR;
						_cb.menu_button_id = IDC_SYSTEM_MENU;
						_cb.image_control_id = IDC_COMPANY_LOGO;
						_cb.image_file = "assets\\small_logo.png";
						_cb.corporate_name = "WOODRUFF SAWYER";
						_cb.id_title_column_id = 0;
						_cb.title_name = "DEVELOPER STATION";
						_cb.subtitle_name = "Products";
					}
				)
				.end();

				application_presentation->create_page("product")
					.column_begin()
					.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
						{
							_cb.title_bar_id = IDC_TITLE_BAR;
							_cb.menu_button_id = IDC_SYSTEM_MENU;
							_cb.image_control_id = IDC_COMPANY_LOGO;
							_cb.image_file = "assets\\small_logo.png";
							_cb.corporate_name = "WOODRUFF SAWYER";
							_cb.id_title_column_id = 0;
							_cb.title_name = "DEVELOPER STATION";
							_cb.subtitle_name = "Product Details";
						}
					)
					.end();

				application_presentation->create_page("settings")
					.column_begin()
					.caption_bar(id_counter::next(), st, &app_menu, [](caption_bar_control& _cb)
						{
							_cb.title_bar_id = IDC_TITLE_BAR;
							_cb.menu_button_id = IDC_SYSTEM_MENU;
							_cb.image_control_id = IDC_COMPANY_LOGO;
							_cb.image_file = "assets\\small_logo.png";
							_cb.corporate_name = "WOODRUFF SAWYER";
							_cb.id_title_column_id = 0;
							_cb.title_name = "DEVELOPER STATION";
							_cb.subtitle_name = "Settings";
						}
					)
					.end();

				application_presentation->select_page("home");

				if (forceWindowed)
				{
					application->runDialog(hInstance, "Developer Station", IDI_WSPROPOSE, false, application_presentation);
				}
				else
				{
					application->runDialog(hInstance, "Developer Station", IDI_WSPROPOSE, true, application_presentation);
				}
	}

}

#endif
