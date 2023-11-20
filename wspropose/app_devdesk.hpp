
#ifndef APP_DEVDESK_H
#define APP_DEVDESK_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{

	/*
	
	using json_method = std::function<sync<int>(json _params, data_set *_set)>;
	using gui_method = std::function<sync<int>(json _params, data_set* _set)>;

	*/

	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{
		application app;

		EnableGuiStdOuts();

		std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
		factory->refresh();

		std::shared_ptr<directApplicationWin32> application = std::make_shared<directApplicationWin32>(factory);
		std::shared_ptr<calico_client> calico_svc = std::make_shared<calico_client>();
		std::shared_ptr<data_plane> app_data = std::make_shared<data_plane>();
		std::shared_ptr<presentation> application_presentation = std::make_shared<presentation>(application);

		corona::menu_item app_menu;

		const int IDC_COMPANY_NAME = 1001;
		const int IDC_COMPANY_LOGO = 1002;
		const int IDC_TITLE_BAR = 1003;
		const int IDC_SYSTEM_MENU = 1004;

		const int IDM_VIEW_MENU = 2001;
		const int IDM_HOME = 2002;
		const int IDM_PRODUCTS = 2004;
		const int IDM_TEAM = 2006;
		const int IDM_OBSTACLES = 2007;

		app_menu.destination(IDM_HOME, "Ho&me", "home")
			.destination(IDM_PRODUCTS, "Pro&ducts", "products")
			.destination(IDM_TEAM, "T&eam", "team")
			.destination(IDM_OBSTACLES, "Se&ttings", "settings");

		auto st = styles.get_style();

		app_data->put_data_source("calico", "calico service", "assets\\images\\calico.png");

		app_data->put_data_set("calico", "login", 
			[calico_svc, application](json _params, data_set* _set) -> sync<int> 
				{
					int temp = co_await calico_svc->login("devdesk", application->getUserName(), _set->data);
					co_return temp;
				}, 
				[calico_svc, application, application_presentation](json _params, data_set* _set) {
					// when logged in, do something;
					if (_set->data.has_member("jwtToken")) {
						application_presentation->select_page("home");
					}
					return 1;
				},
				5
			);

		app_data->put_data_set("calico", "fields",
			[calico_svc, application, app_data](json _params, data_set* _set) -> sync<int>
			{
				json credentials = app_data->get("login");
				int temp = co_await calico_svc->get_fields(credentials, _set->data);
				co_return temp;
			},
			[calico_svc, application](json _params, data_set* _set) -> int {
				// when logged in, do something;				
				return 0;
			},
			10);

		app_data->put_data_set("calico", "classes",
			[calico_svc, application, app_data](json _params, data_set* _set) -> sync<int>
			{
				json credentials = app_data->get("login");
				int temp = co_await calico_svc->get_classes(credentials, _set->data);
				co_return temp;
			},
			[calico_svc, application](json _params, data_set* _set) -> int {
				// when logged in, do something;				
				return 0;
			},
			10);

		app_data->put_data_set("calico", "actoroptions",
			[calico_svc, application, app_data](json _params, data_set* _set) -> sync<int>
			{
				json credentials = app_data->get("login");
				int temp = co_await calico_svc->get_actor_options(credentials, _set->data);
				co_return temp;
			},
			[calico_svc, application, application_presentation](json _params, data_set* _set) -> int {
				// when logged in, do something;				
				if (_set->data.has_member("jwtToken")) {
					if (set->data.has_member("")) {
						application_presentation->select_page("home");
					}
				}
				return 0;
			},
			0);

		bool forceWindowed = false;

		if (strstr(lpszCmdParam, "-window")) {
			forceWindowed = true;
		}

#if _DEBUG
		forceWindowed = true;
#endif

		std::function<void(pushbutton_control& _set_defaults)> push_button_defaults = [](pushbutton_control& ctrl) {
			ctrl.set_size(150.0_px, 50.0_px);
			};

		application_presentation->create_page("home", [calico_svc, application](page& _pg)
			{
				_pg.on_load([calico_svc, application](page_load_event _evt)
					{

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

			application_presentation->create_page("login", [calico_svc, application](page& _pg)
						{
							_pg.on_load([calico_svc, application](page_load_event _evt)
								{

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
									_cb.subtitle_name = "Login";
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
