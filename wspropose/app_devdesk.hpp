
#ifndef APP_DEVDESK_H
#define APP_DEVDESK_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{

	const int IDC_COMPANY_NAME = 1001;
	const int IDC_COMPANY_LOGO = 1002;
	const int IDC_TITLE_BAR = 1003;
	const int IDC_SYSTEM_MENU = 1004;

	const int IDM_VIEW_MENU = 2001;
	const int IDM_HOME = 2002;
	const int IDM_LOGIN = 2003;

	const int IDC_STATUS_MESSAGE = 5001;
	const int IDC_STATUS_DETAIL = 5002;

	void create_home_page(
		page& _page,
		std::shared_ptr<directApplicationWin32> application,
		std::shared_ptr<calico_client> calico_svc,
		std::shared_ptr<presentation> app_show,
		std::shared_ptr<menu_item> app_menu,
		presentation_style *st
		)
	{

		// we clear the page of all of its children controls, and start building our form
		// clearing the page should also clear the event handlers.
		// we hope. and... we know.

		_page.clear();

		// First check to make sure we have all the things

		control_builder contents_root;

		// first we put a caption bar in our standard page

		contents_root.caption_bar(id_counter::next(), st, app_menu.get(), [](caption_bar_control& _cb)
			{
				_cb.menu_button_id = IDC_SYSTEM_MENU;
				_cb.image_control_id = IDC_COMPANY_LOGO;
				_cb.image_file = "assets\\small_logo.png";
				_cb.corporate_name = "WOODRUFF SAWYER";
				_cb.title_name = "PROPERTY AND CASUALTY";
				_cb.code_detail_id = IDC_STATUS_DETAIL;
				_cb.code_status_id = IDC_STATUS_MESSAGE;
			}
		);

		auto& tc_message = contents_root.get_root()->find<code_control>(IDC_STATUS_MESSAGE);
		auto& tc_detail = contents_root.get_root()->find<code_control>(IDC_STATUS_DETAIL);

		json actor_options;

		auto base_ds = app_show->get_data_set("current_state");
		actor_options = base_ds->data;

		auto err = base_ds->get_error();
		if (err.error) {
			tc_message.text = "Error";
			tc_detail.text = err.message;
			contents_root.apply_controls(_page.root.get());
			return;
		} else if (!actor_options.has_member("ActorOptions")) {
			tc_message.text = "No data yet";
			tc_detail.text = "";
			auto control_to_apply_to = _page.root.get();
			contents_root.apply_controls(control_to_apply_to);
			return;
		}

		actor_options = actor_options["ActorOptions"];

		tc_message.text = "Connected";
		tc_detail.text = "";

		// then, below the caption bar, an overall contents pane
		// which has a navigation column on the left, and, the tab view on the right

		auto contents = contents_root.row_begin(
			app_show->get_control_id("main_row", []() { return id_counter::next(); }),
			[](row_layout& _settings) {
				_settings.set_margin(10.0_px);
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
		int command_container_id = app_show->get_control_id("command_container", []() { return id_counter::next(); });

		// note that, we are putting the breadcrumbs on a nav pane to the left.
		auto command_container = contents.column_begin(command_container_id, [](column_layout& rl) {
			rl.set_size(300.0_px, 1.0_container);
			});

		command_container.chaptertitle("Your location", [](chaptertitle_control& ct) {
			ct.set_size(.95_container, 30.0_px);
			});

		// and now we go through our selected objects....
		for (int i = 0; i < selected_objects.size(); i++)
		{
			// pull our current object out of this json array
			auto selected_object = selected_objects[i];

			std::string class_name = selected_object["ClassName"].get_string();
			int64_t class_id = selected_object["ClassId"].get_double();
			int64_t object_id = selected_object["ObjectId"].get_double();
			std::string object_id_string = selected_object["ObjectId"];

			std::cout << "selected object:" << class_name << " " << object_id_string << std::endl;

			// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
			// we also use this, to tie to our data, which actually drives the application
			std::string button_name = "select_object." + object_id_string;

			int select_button_id = app_show->get_control_id(button_name, []() { return id_counter::next(); });

			// and, now, create our button.  here, the class name is used as a label
			// in the future we can create data set aware buttons but for now this is really all we need, because in calico this sort of 
			// sequence does everything.
			command_container.push_button(select_button_id, class_name, [](pushbutton_control& pc) {
				pc.set_size(.95_container, 30.0_px );
				});

			// so, we have to bind our button to our data.. first, we describe what the button does with data...
			// whenever we get a data set with this key, this stuff gets invoked.
			// and this all happens on background threads.
		}

		/*  ---------------------------------------------------------------------------------------------------
			Creating New Objects 
		*/

		command_container.chaptertitle("Create New", [](chaptertitle_control& ct) {
			ct.set_size(.95_container, 30.0_px);
			});

		for (int i = 0; i < create_options.size(); i++)
		{
			auto co = create_options[i];

			// then, fish out the stuff we need
			std::string class_name = co["CreateClassName"].get_string();
			std::string class_description = co["CreateClassDescription"].get_string();

			// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
			// we also use this, to tie to our data, which actually drives the application
			std::string button_name = "create_class." + class_name;

			int button_id = app_show->get_control_id(button_name, []() { return id_counter::next(); });

			// and, create our button and add it to our container
/*			command_container.push_button(button_id, class_name, [](pushbutton_control& pc) {
				pc.set_size(.95_container, 30.0_px);
				});
				*/
			command_container.push_button(button_id, class_description, [](pushbutton_control& pc) {
				pc.set_size(.95_container, 30.0_px);
				});

			// and now, we associate creating the object with the application data...
			// whenever we get a data set with this key, this stuff gets invoked.
			// and this all happens on background threads.

			app_show->put_data_set("calico", button_name,
				[calico_svc, application, app_show, class_name](json _params, data_function* _set) -> int
				{
					// get our login credentials from the data set.
					json credentials = app_show->get_data("login");

					// create a new object
					json_parser jp;
					json new_object_request = jp.create_object();
					new_object_request.put_member("ClassName", class_name);

					// call our application
					int temp = calico_svc->create_object(new_object_request, credentials, _set->data);
					app_show->update_data_set("current_state");

					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				[_page, app_show](json _params, data_function* _set) -> int {
					// when back on the ui thread, kick off our refresh					
					auto& tc_message = _page.root->find<code_control>(IDC_STATUS_MESSAGE);
					auto& tc_detail = _page.root->find<code_control>(IDC_STATUS_DETAIL);
					auto loginstate = app_show->get_data_set("login");
					auto login_error = loginstate->get_error();
					if (login_error.error) {
						tc_message.set_text( "Error With Credentials" );
						tc_detail.set_text(login_error.message);
					}
					else 
					{
						auto stateds = app_show->get_data_set("current_state");
						auto stateds_error = stateds->get_error();
						if (stateds_error.error) {
							tc_message.set_text("Error With Credentials");
							tc_detail.set_text(stateds_error.message);
						}
					}
					return 0;
				},
				0);

			// and, now add an event handler, to select the object on the back end, when this is pressed.
			// the [capture,..](param,...) notation is how C++ does lambda expressions.
			_page.on_command(button_id, [_page, button_name, app_show, class_name, calico_svc](command_event ce) {
					auto& tc_message = _page.root->find<code_control>(IDC_STATUS_MESSAGE);
					auto& tc_detail = _page.root->find<code_control>(IDC_STATUS_DETAIL);
					tc_message.text = "Creating";
					tc_message.text = "Creating Object";

					app_show->update_data_set(button_name);
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
			std::string class_description = selected_rule["ClassDescription"];
			std::string class_id = selected_rule["ClassId"];
			std::string view_name = selected_rule["ViewName"];
			auto objects_in_rule = selected_rule["Items"];

			std::string bind_name = "rule." + rule_name;

			// now we specify our tab.
			// for the tab, we're going to specify what our control is, and then put items in the control
			// because there could be a lot of items, we will define some data sources that do the transformation
			// the column view is associated with the tab and it can do selection and the other things
			tab_pane new_tab;
			new_tab.id = app_show->get_control_id(bind_name, []() { return id_counter::next(); });
			new_tab.name = rule_description;

			// group scans an array, then creates a new object whose each member corresponds to the key 
			// used to group by.
			auto objects_by_class = objects_in_rule.group([](json& _item) {
				return _item["ClassName"].get_string();
				});

			array_data_source ads;
			json_parser jp;

			ads.data = jp.create_array();
			ads.data_to_control = [app_show](control_base* _parent, json& _array, int _index) {
				auto json_object = _array[_index];
				auto class_name = json_object["ClassName"];
				auto factory = app_show->get_class_control_factory(class_name);
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
					json member_obj = member_array[k];
					json item_obj = jp.create_object();
					item_obj.copy_member("ClassName", member_obj);
					item_obj.copy_member("Name", member_obj);
					item_obj.put_member("Source", member_obj);
					ads.data.put_element(-1, item_obj);
				}
			}

			new_tab.create_tab_controls = [rule_description,ads](tab_pane& _pane, control_base* _cont) {
				control_builder cb;
				cb.column_view_begin(id_counter::next(), [ads](column_view_layout& _layout) {
					_layout.set_item_source(ads);
					_layout.set_size(1.0_container, 1.0_container);
					});
				cb.apply_controls(_cont);
				};
			 
			tabs.push_back(new_tab);
		}

		int tab_view_id = app_show->get_control_id("actor_options_tab_view", []() { return id_counter::next(); });

		contents.tab_view(tab_view_id, [tabs](tab_view_control& tv) {
			tv.set_size(1.0_remaining, 1.0_container);
			tv.set_tabs(tabs);
			});

		contents_root.apply_controls(_page.root.get());
	}

	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{
		application app;

		EnableGuiStdOuts();

		// get the direct adapters and initialize them
		std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
		factory->refresh();

		// create a win32 set of windows that can use this factory
		// to make windows with a d2d / d3d image
		std::shared_ptr<directApplicationWin32> application = std::make_shared<directApplicationWin32>(factory);

		// create the data store and bind the calico client to it.
		std::shared_ptr<data_lake> app_data = std::make_shared<data_lake>();
		std::shared_ptr<calico_client> calico_svc = std::make_shared<calico_client>("localhost", 7277);
		calico_svc->bind(app_data);

		// create the presentation - this holds the data of what is on screen, for various pages.
		std::shared_ptr<presentation> application_presentation = std::make_shared<presentation>(application);
		application_presentation->data = app_data;

		std::shared_ptr<menu_item> app_menu = std::make_shared<menu_item>();

		app_menu->destination(IDM_HOME, "&Home", "home")
				 .destination(IDM_LOGIN, "&Login", "login");

		auto st = styles.get_style();

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

		auto& home_page = application_presentation->create_page("home", [application_presentation, calico_svc, app_data, application, st, app_menu](page& _pg)
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

				_pg.on_select([calico_svc, application, application_presentation, app_data, st, app_menu](page_select_event _evt)
					{
						std::cout << "home: selected" << std::endl;
						create_home_page( *(_evt.pg), application, calico_svc,  application_presentation, app_menu, st);
					});

				_pg.on_changed([calico_svc, application, application_presentation, st, app_menu](page& _pg, std::string _set_name)
					{
						std::cout << "home:  changed (" << _set_name << ")" << std::endl;
//						application_presentation->select_page("home");
						create_home_page( _pg, application, calico_svc, application_presentation, app_menu, st);
					}
				);

			});

		auto &login_page = application_presentation->create_page("login", [calico_svc, application_presentation, application, app_data, st, app_menu](page& _pg)
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

				_pg.on_select([calico_svc, application, application_presentation, app_data, st, app_menu](page_select_event _evt)
					{
						app_data->update_data_set("login");
					});

				_pg.on_changed([calico_svc, application, application_presentation, app_data, st, app_menu](page& _pg, std::string _set_name)
					{
						auto& tc_message = _pg.root->find<code_control>(IDC_STATUS_MESSAGE);
						auto& tc_detail = _pg.root->find<code_control>(IDC_STATUS_DETAIL);

						auto err_set = app_data->get_data_set("login");
						auto err = err_set->get_error();
						if (err.error)
						{
							tc_message.text = err.error;
							tc_detail.text = err.message;
						}
					});
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
