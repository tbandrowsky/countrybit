
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

	const int IDC_COMPANY_NAME = 1001;
	const int IDC_COMPANY_LOGO = 1002;
	const int IDC_TITLE_BAR = 1003;
	const int IDC_SYSTEM_MENU = 1004;

	const int IDM_VIEW_MENU = 2001;
	const int IDM_HOME = 2002;
	const int IDM_LOGIN = 2003;

	void create_devdesk_page(
		json actor_options,
		page& _page,
		std::shared_ptr<directApplicationWin32> application,
		std::shared_ptr<calico_client> calico_svc,
		std::shared_ptr<data_plane> app_data,
		std::shared_ptr<presentation> application_presentation,
		std::shared_ptr<menu_item> app_menu,
		presentation_style *st
		)
	{
		// First check to make sure we have all the things

		if (!actor_options.has_member("ActorOptions"))
			return;

		actor_options = actor_options["ActorOptions"];

		// we clear the page of all of its children controls, and start building our form
		// clearing the page should also clear the event handlers.
		// we hope.

		_page.clear();

		auto contents_root = _page.column_begin();

		// first we put a caption bar in our standard page

		contents_root.caption_bar(id_counter::next(), st, app_menu.get(), [](caption_bar_control& _cb)
			{
				_cb.menu_button_id = IDC_SYSTEM_MENU;
				_cb.image_control_id = IDC_COMPANY_LOGO;
				_cb.image_file = "assets\\small_logo.png";
				_cb.corporate_name = "WOODRUFF SAWYER";
				_cb.title_name = "PROPERTY AND CASUALTY";
			}
		)
		.end();

		contents_root.apply_controls(_page.get_root());

		

		return;

		// then, below the caption bar, an overall contents pane
		// which has a navigation column on the left, and, the tab view on the right

		auto contents = contents_root.row_begin(
			app_data->get_control_id("main_row", []() { return id_counter::next(); }), 
			[](row_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_remaining);
			});

		// then, we get the objects the user has selected.  this can be used to build a breadcrumb trail and show the user where they are at, navigationally.
		json selected_objects = actor_options["SelectedObjects"];

		// then, we get a list of classes of objects that we are allowed to create.  this gets visualized as buttons, to allow creating of a new object
		json create_options = actor_options["CreateOptions"];

		// now, select options are, all the objects that we may select. These are objects of classes that match the hiearachy of selected objects
		json select_options = actor_options["SelectOptions"];

		/*  ---------------------------------------------------------------------------------------------------
			Navigating back up through previous selections
		*/

		// show time.  First we build out where our high level stuff is.  So first we have a row that has a bunch of buttons on it, and that is our breadcrumb trail.
		// these selected objects are, well, things that we have selected in this path to get where we are now.
		int command_container_id = app_data->get_control_id("command_container", []() { return id_counter::next(); });

		// note that, we are putting the breadcrumbs on a nav pane to the left.
		auto command_container = contents.column_begin(command_container_id, [](column_layout& rl) {
			rl.set_size(300.0_px, 1.0_container);
			});

		command_container.chaptertitle("Your location");

		// and now we go through our selected objects....
		for (int i = 0; i < selected_objects.size(); i++)
		{
			// pull our current object out of this json array
			auto selected_object = selected_objects[i];

			std::string class_name = selected_object["ClassName"].get_string();
			int64_t class_id = selected_object["ClassId"].get_double();
			int64_t object_id = selected_object["ObjectId"].get_double();
			std::string object_id_string = selected_object["ObjectId"];

			// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
			// we also use this, to tie to our data, which actually drives the application
			std::string button_name = "select_object." + object_id_string;

			int select_button_id = app_data->get_control_id(button_name, []() { return id_counter::next(); });

			// and, now, create our button.  here, the class name is used as a label
			// in the future we can create data set aware buttons but for now this is really all we need, because in calico this sort of 
			// sequence does everything.
			command_container.push_button(select_button_id, class_name);

			// so, we have to bind our button to our data.. first, we describe what the button does with data...
			// whenever we get a data set with this key, this stuff gets invoked.
			// and this all happens on background threads.
			app_data->put_data_set("calico", button_name,
				[calico_svc, application, app_data, class_name, object_id](json _params, data_set* _set) -> int
				{
					// get our login credentials from the data set.
					json credentials = app_data->get("login");

					// create a new object
					json_parser jp;
					json object_select_request = jp.create_object();
					object_select_request.put_member("ObjectId", object_id);

					// call our application
					int temp = calico_svc->select_object(object_select_request, credentials, _set->data);

					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				[app_data](json _params, data_set* _set) -> int {
					// when back on the ui thread, kick off our refresh, which is to just call actor options again
					json options = app_data->get("actoroptions");
					return 0;
				},
				0);
		}

		/*  ---------------------------------------------------------------------------------------------------
			Creating New Objects 
		*/

		command_container.chaptertitle("Create New");

		for (int i = 0; i < create_options.size(); i++)
		{
			auto co = create_options[i];

			// then, fish out the stuff we need
			std::string class_name = co["CreateClass"].get_string();

			// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
			// we also use this, to tie to our data, which actually drives the application
			std::string button_name = "create_class." + class_name;

			int button_id = app_data->get_control_id(button_name, []() { return id_counter::next(); });

			// and, create our button and add it to our container
			command_container.push_button(button_id, class_name);

			// and now, we associate creating the object with the application data...
			// whenever we get a data set with this key, this stuff gets invoked.
			// and this all happens on background threads.

			app_data->put_data_set("calico", button_name,
				[calico_svc, application, app_data, class_name](json _params, data_set* _set) -> int
				{
					// get our login credentials from the data set.
					json credentials = app_data->get("login");

					// create a new object
					json_parser jp;
					json new_object_request = jp.create_object();
					new_object_request.put_member("ClassName", class_name);

					// call our application
					int temp = calico_svc->create_object(new_object_request, credentials, _set->data);

					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				[app_data](json _params, data_set* _set) -> int {
					// when back on the ui thread, kick off our refresh
					json options = app_data->get("actoroptions");
					return 0;
				},
				0);

			// and, now add an event handler, to select the object on the back end, when this is pressed.
			// the [capture,..](param,...) notation is how C++ does lambda expressions.
			_page.on_command(button_id, [button_name, app_data, class_name, calico_svc](command_event ce) {	
					json options = app_data->get("button_name");
				});

		}

		/*  ---------------------------------------------------------------------------------------------------
			Viewing and manipulating new objects
			these we present in a basic form, with the edit fields of the base selected object, and then, the details (children), of each, in tabs.
		*/

		// define our tabs
		std::vector<tab_pane> tabs;

		// now, we have to build up our tabs
		for (int i = 0; i < select_options.size(); i++)
		{
			// get our element out of this json array
			auto selected_rule = select_options[i];

			// get our data out of this element
			// the rule name and description are the model rules by which this object was selected.
			// these will be needed to do further object manipulations.

			std::string rule_name = selected_rule["RuleName"];
			std::string rule_description = selected_rule["RuleDescription"];
			std::string class_name = selected_rule["ClassName"];
			std::string class_id = selected_rule["ClassId"];
			std::string view_name = selected_rule["ViewName"];
			auto objects_in_rule = selected_rule["Items"];

			std::string bind_name = "rule." + rule_name;

			// now we specify our tab.
			// for the tab, we're going to specify what our control is, and then put items in the control
			// because there could be a lot of items, we will define some data sources that do the transformation
			// the column view is associated with the tab and it can do selection and the other things
			tab_pane new_tab;
			new_tab.id = app_data->get_control_id(bind_name, []() { return id_counter::next(); });
			new_tab.name = class_name;
			auto tab_controls = std::make_shared<column_view_layout>();
			new_tab.tab_controls = tab_controls;

			// group scans an array, then creates a new object whose each member corresponds to the key 
			// used to group by.
			auto objects_by_class = objects_in_rule.group([](json& _item) {
				return _item["ClassName"].get_string();
				});

			array_data_source ads;
			json_parser jp;

			ads.data = jp.create_array();
			ads.data_to_control = [app_data](control_base* _parent, json& _array, int _index) {
				auto json_object = _array[_index];
				auto class_name = json_object["ClassName"];
				auto factory = app_data->get_class_control_factory(class_name);
				return factory(_parent, _array, _index);
				};

			// so grouped, we go through the members
			// note that, here is our chance to change the visualization based upon the item type

			auto members = objects_by_class.get_members();

			for (auto member : members)
			{
				// convert this to a member array, and...
				json member_array(member.second);

				// add a header for this class name
				json header_obj = jp.create_object();
				header_obj.put_member("ClassName", ".section");
				header_obj.put_member("Name", member.first);
				ads.data.put_element(-1, header_obj);

				// and, within the group, we go through the array
				for (int k = 0; k < member_array.size(); k++)
				{
					json member_obj = member_array[i];
					json item_obj = jp.create_object();
					item_obj.copy_member("ClassName", member_obj);
					item_obj.copy_member("Name", member_obj);
					item_obj.put_member("Source", member_obj);
					ads.data.put_element(-1, item_obj);
				}
			}

			tab_controls->set_item_source(ads);
		}

		auto selected_objects_container = contents.tab_view(id_counter::next(), [tabs](tab_view_control& tv) {
			tv.set_size(1.0_remaining, 1.0_container);
			tv.set_tabs(tabs);
			});

	}

	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{
		application app;

		EnableGuiStdOuts();

		std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
		factory->refresh();

		std::shared_ptr<directApplicationWin32> application = std::make_shared<directApplicationWin32>(factory);
		std::shared_ptr<calico_client> calico_svc = std::make_shared<calico_client>("localhost", 7277);
		std::shared_ptr<data_plane> app_data = std::make_shared<data_plane>();
		std::shared_ptr<presentation> application_presentation = std::make_shared<presentation>(application);
		std::shared_ptr<menu_item> app_menu = std::make_shared<menu_item>();

		app_menu->destination(IDM_HOME, "&Home", "home")
			.destination(IDM_LOGIN, "&Login", "login");

		auto st = styles.get_style();

		app_data->put_data_source("calico", "calico service", "assets\\images\\calico.png");

		app_data->put_data_set("calico", "actoroptions",
			[calico_svc, application, app_data](json _params, data_set* _set) -> int
			{
				json credentials = app_data->get("login");
				int temp = calico_svc->get_actor_options(credentials, _set->data);
				return temp;
			},
			[calico_svc, application, application_presentation, app_data, app_menu, st](json _params, data_set* _set) -> int {
				// when logged in, do something;		
				// 
				// 		
				return 0;
			},
			0);

		app_data->put_data_set("calico", "login", 
			[calico_svc, application, app_data](json _params, data_set* _set) -> int
				{
					json classes_json;
					json fields_json;
					int temp = calico_svc->login("Property", application->getUserName(), _set->data);

					if (temp) {
						temp = calico_svc->get_classes(_set->data, classes_json);
						temp = calico_svc->get_fields(_set->data, fields_json);
						app_data->put_data_set("calico", "classes", classes_json);
						app_data->put_data_set("calico", "fields", fields_json);
						app_data->get("actoroptions");
					}
					return temp;
				}, 
				[calico_svc, app_menu, app_data, application, application_presentation, st](json _params, data_set* _set) {
					// when logged in, do something;
					auto err = _set->get_error();
					if (err.error) 
					{
						application_presentation->pages["login"]->changed("login");
					}
					else 
					{
						auto& new_page = application_presentation->create_page("home", [_set, application, calico_svc, app_data, application_presentation, app_menu, st](page& new_page) {
							auto ao = app_data->get_data_set("actoroptions")->data;
							create_devdesk_page(ao, new_page, application, calico_svc, app_data, application_presentation, app_menu, st);
							});
					}

					return 1;
				},
				5
			);

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

		const int IDC_STATUS_MESSAGE = 5001;
		const int IDC_STATUS_DETAIL = 5002;

		auto& home_page = application_presentation->create_page("home", [calico_svc, app_data, application, st, app_menu](page& _pg)
			{
				_pg.column_begin()
					.caption_bar(id_counter::next(), st, app_menu.get(), [](caption_bar_control& _cb)
						{
							_cb.menu_button_id = IDC_SYSTEM_MENU;
							_cb.image_control_id = IDC_COMPANY_LOGO;
							_cb.image_file = "assets\\small_logo.png";
							_cb.corporate_name = "WOODRUFF SAWYER";
							_cb.title_name = "PROPERTY and CASUALTY";
							_cb.code_detail_id = IDC_STATUS_DETAIL;
							_cb.code_status_id = IDC_STATUS_MESSAGE;
						}
					)
					.end();

					_pg.on_select([calico_svc, application, app_data, st, app_menu](page_select_event _evt)
					{
						auto& tc_message = _evt.pg->root->find<code_control>(IDC_STATUS_MESSAGE);
						auto& tc_detail = _evt.pg->root->find<code_control>(IDC_STATUS_DETAIL);
						tc_message.text = "Connected";
						tc_detail.text = "Getting Options";

						threadomatic::run([app_data]() { app_data->get("actoroptions"); });
					}
				);
			});

		auto &login_page = application_presentation->create_page("login", [calico_svc, application, app_data, st, app_menu](page& _pg)
			{
				int title_column_id = id_counter::next();
				_pg.column_begin()
					.caption_bar(id_counter::next(), st, app_menu.get(), [title_column_id](caption_bar_control& _cb)
						{
							_cb.menu_button_id = IDC_SYSTEM_MENU;
							_cb.image_control_id = IDC_COMPANY_LOGO;
							_cb.image_file = "assets\\small_logo.png";
							_cb.corporate_name = "WOODRUFF SAWYER";
							_cb.title_name = "PROPERTY and CASUALTY";
							_cb.code_detail_id = IDC_STATUS_DETAIL;
							_cb.code_status_id = IDC_STATUS_MESSAGE;
						}
					)
					.column_begin(title_column_id, [](column_layout& rl) {
							rl.set_size(0.5_container, 1.0_remaining);
							rl.set_item_size(1.0_container, 150.0_px);
						})
						.end()
				.end();

				_pg.on_select([calico_svc, application, app_data, st, app_menu](page_select_event _evt)
					{
						auto& tc_message = _evt.pg->root->find<code_control>(IDC_STATUS_MESSAGE);
						auto& tc_detail = _evt.pg->root->find<code_control>(IDC_STATUS_DETAIL);
						tc_message.text = "Connecting";
						tc_detail.text = "";
						threadomatic::run([app_data]() { app_data->get("login"); });
					}
				);

				_pg.on_changed([calico_svc, application, app_data, st, app_menu](page& _pg, std::string _set_name)
					{
						auto& tc_message = _pg.root->find<code_control>(IDC_STATUS_MESSAGE);
						auto& tc_detail = _pg.root->find<code_control>(IDC_STATUS_DETAIL);
						auto err = app_data->get_data_set(_set_name)->get_error();
						tc_message.text = _set_name;
						tc_detail.text = err.message;
					}
				);
			});

		if (forceWindowed)
		{
			application->runDialog(hInstance, "Property and Casualty", IDI_WSPROPOSE, false, application_presentation);
		}
		else
		{
			application->runDialog(hInstance, "Property and Casualty", IDI_WSPROPOSE, true, application_presentation);
		}
	}

}

#endif
