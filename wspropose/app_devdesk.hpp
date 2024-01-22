
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
		presentation_style* st
	)
	{
		int id_caption_bar = app_show->get_control_id("caption_bar", []() { return id_counter::next(); });
		int id_main_row = app_show->get_control_id("main_row", []() { return id_counter::next(); });
		int id_command_container = app_show->get_control_id("command_container", []() { return id_counter::next(); });
		int id_current_container = app_show->get_control_id("current_container", []() { return id_counter::next(); });
		int id_location_title = app_show->get_control_id("location_title", []() { return id_counter::next(); });
		int id_create_title = app_show->get_control_id("create_title", []() { return id_counter::next(); });
		int id_form_view = app_show->get_control_id("actor_options_form_view", []() { return id_counter::next(); });
		int id_tab_view = app_show->get_control_id("actor_options_tab_view", []() { return id_counter::next(); });

		control_builder command_container;
		caption_bar_control* caption_container;
		tab_view_control* tab_container;
		form_view_control* form_view;
		// then we must be a new page

		if (_page.get_root()->children.size() == 0) 
		{
			// First check to make sure we have all the things

			control_builder contents_root(_page.get_root_container());

			contents_root.caption_bar(id_caption_bar, st, app_menu.get(), [](caption_bar_control& _cb)
				{
					_cb.menu_button_id = IDC_SYSTEM_MENU;
					_cb.image_control_id = IDC_COMPANY_LOGO;
					_cb.image_file = "assets\\small_logo.png";
					_cb.corporate_name = "COUNTRYBIT";
					_cb.title_name = "REVOLUTION";
					_cb.code_detail_id = IDC_STATUS_DETAIL;
					_cb.code_status_id = IDC_STATUS_MESSAGE;
				}
			);

			auto contents = contents_root.row_begin(
				id_main_row,
				[](row_layout& _settings) {
					_settings.set_margin(10.0_px);
					_settings.set_size(1.0_container, 1.0_remaining);
				});

			// note that, we are putting the breadcrumbs on a nav pane to the left.
			auto command_container = contents.column_begin(id_command_container, [](column_layout& rl) {
				rl.set_size(300.0_px, 1.0_container);
				});

			auto current_container = contents.column_begin(id_current_container, [](column_layout& rl) {
				rl.set_size(1.0_remaining, 1.0_container);
				});

			current_container.form_view(id_form_view, [](form_view_control& _fv)
				{
					item_data_source ids;
					_fv.fields_per_column = 3;
					_fv.set_size(1.0_container, .15_container);
					_fv.set_data(ids);
				});

			current_container.tab_view(id_tab_view, [](tab_view_control& tv) {
				tv.set_size(1.0_remaining, 1.0_container);
				});

			_page.schedule_refresh(10, "calico", "get_ui");

			_page.on_select([app_show, calico_svc, application, st, app_menu](page_select_event psevt) {
				page& pg = *psevt.pg;
				create_home_page(pg, application, calico_svc, app_show, app_menu, st);
				});

			_page.on_changed(0, "calico", "get_ui", [app_show, calico_svc, application, st, app_menu](page_data_event pde) {
				app_show->select_page("home");
				});

			contents_root.apply_controls(_page.root.get());
		}

		command_container = _page.edit(id_command_container);
		caption_container = _page.find_container<caption_bar_control>(id_caption_bar);
		tab_container = _page.find_container<tab_view_control>(id_tab_view);
	
		// first we put a caption bar in our standard page

		json general_options;

		json actor_options;

		auto base_ds = app_show->data->get_function("calico", "get_ui");
		general_options = base_ds->data;

		auto err = base_ds->status;

		// here, we tell the page to subscribe to data changes
		// and then we schedule our data change

		if (!err.success) 
		{
			return;
		} 
		else if (!general_options.has_member("current_state")) {
			return;
		}

		actor_options = general_options["current_state"];

		actor_options = actor_options["ActorOptions"];

		// then, below the caption bar, an overall contents pane
		// which has a navigation column on the left, and, the tab view on the right

		// then, we get the objects the user has selected.  this can be used to build a breadcrumb trail and show the user where they are at, navigationally.
		json selected_objects = actor_options["SelectedObjects"];

		// then, we get a list of classes of objects that we are allowed to create.  this gets visualized as buttons, to allow creating of a new object
		json create_options = actor_options["CreateOptions"];

		// now, select options are, all the objects that we may select. These are objects of classes that match the hiearachy of selected objects
		json select_options = actor_options["SelectOptions"];

		/*  ---------------------------------------------------------------------------------------------------
			Navigating back up through previous selections
		*/

		command_container.chaptertitle("Your location", [](chaptertitle_control& ct) {
			ct.set_size(.95_container, 30.0_px);
			}, id_location_title);

		int64_t current_edited_object_id = -1;
		std::string current_edited_class_name;
		std::string current_edited_class_description;

		// and now we go through our selected objects....
		for (int i = 0; i < selected_objects.size(); i++)
		{
			// pull our current object out of this json array
			auto selected_object = selected_objects[i];

			std::string class_name = selected_object["ClassName"].get_string();
			std::string class_description = selected_object["ClassDescription"].get_string();
			int64_t class_id = selected_object["ClassId"].get_double();
			int64_t object_id = selected_object["ObjectId"].get_double();
			std::string object_id_string = selected_object["ObjectId"];

			if (current_edited_object_id < object_id)
			{
				current_edited_object_id = object_id;
				current_edited_class_name = class_name;
				current_edited_class_description = class_description;
			}

			std::cout << "selected object:" << class_name << " " << object_id_string << std::endl;

			// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
			// we also use this, to tie to our data, which actually drives the application
			std::string button_name = "select_object." + object_id_string;

			int select_button_id = app_show->get_control_id(button_name, []() { return id_counter::next(); });

			command_container.calico_button(select_button_id,[object_id, class_description, app_show](calico_button_control& pc) {
				pc.set_size(.95_container, 30.0_px);
				pc.text = class_description;
				pc.lake = app_show->data;

				json_parser jp;
				json object_request = jp.create_object();
				object_request.put_member("ObjectId", object_id);

				pc.source_name = "calico";
				pc.function_name = "select_object";
				pc.function_parameters = object_request;

				});

			// so, we have to bind our button to our data.. first, we describe what the button does with data...
			// whenever we get a data set with this key, this stuff gets invoked.
			// and this all happens on background threads.
		}

		/* 
		and now build our form view
		*/

		json current_class = general_options["current_class"];
		json current_object = general_options["current_object"];
		json current_fields = general_options["current_fields"];

		// extract our class data
		auto class_obj = current_class["CalicoClass"];
		std::string classDescription = class_obj["ClassDescription"];
		std::string classFullName = class_obj["ClassFullName"];
		json relatedClassList = class_obj["RelatedClassList"];
		json classFieldList = class_obj["ClassFieldList"];
		std::string primaryKeyField = class_obj["PrimaryKeyField"];
		std::string sqlTableName = class_obj["SqlTableName"];
		std::string sqlViewName = class_obj["SqlViewName"];
		int64_t primary_key_field_id = class_obj["PrimaryKeyField"];

		// and load it into our form editor...
		json_parser jp;

		item_data_source ids;
		ids.name = classDescription;
		ids.data = current_object;

		for (int i = 0; i < current_fields.size(); i++)
		{
			auto fieldDefinition = current_fields.get_element(i);

			std::string fieldName = fieldDefinition["FieldName"];
			std::string fieldDescription = fieldDefinition["FieldDescription"];
			std::string fieldType = fieldDefinition["FieldType"];
			std::string columnDataType = fieldDefinition["ColumnDataType"];
			std::string dotNetFormat = fieldDefinition["DotNetFormat"];
			std::string excelFormat = fieldDefinition["ExcelFormat"];
			std::string javaScriptFormat = fieldDefinition["JavaScriptFormat"];
			std::string basicValidationMessage = fieldDefinition["BasicValidationMessage"];
			std::string minimumValue = fieldDefinition["MinimumValue"];
			std::string maximumValue = fieldDefinition["MaximumValue"];
			std::string gridColumnWidth = fieldDefinition["GridColumnWidth"];
			int64_t field_id = fieldDefinition["FieldId"];

			item_field new_field;

			new_field.field_label = fieldDescription;
			new_field.json_member_name = fieldName;
			new_field.field_tooltip = classFullName + "." + fieldName + " " + columnDataType;
			new_field.read_only = false;
			new_field.field_id = app_show->get_control_id("form_view_" + fieldName, []() { return id_counter::next(); });

			if (fieldType == "Currency") {
				//edit_currency
				new_field.field_type = "edit_currency";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "DateTime") {
				//datetimepicker_field
				new_field.field_type = "datetimepicker_field";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "Double") {
				//edit_double
				new_field.field_type = "edit_double";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "Integer") {
				new_field.field_type = "edit_integer";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "String") {
				new_field.field_type = "edit_string";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "EnumerationDataSource") {
				//comboboxex_field
				new_field.field_type = "comboboxex_field";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "EnumerationList") {
				//comboboxex_field
				new_field.field_type = "comboboxex_field";
				ids.fields.push_back(new_field);
			}
			else if (fieldType == "Key") {
				//edit_integer

				if (primary_key_field_id == primary_key_field_id) 
				{
					new_field.read_only = true;
					new_field.field_type = "edit_integer";
					ids.fields.push_back(new_field);
				}
				else 
				{
					new_field.read_only = true;
					new_field.field_type = "combobox_field";
					ids.fields.push_back(new_field);
				}
			}
			else if (fieldType == "Property") 
			{
				//edit_string
				new_field.read_only = true;
				new_field.field_type = "edit_string";
				ids.fields.push_back(new_field);
			}
		}

		auto fv = app_show->get_control<form_view_control>(id_form_view);
		if (fv) {
			fv->set_data(ids);
		}

		/*  ---------------------------------------------------------------------------------------------------
			Creating New Objects 
		*/

		command_container.chaptertitle("Create New", [](chaptertitle_control& ct) {
			ct.set_size(.95_container, 30.0_px);
			}, id_create_title);

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
			command_container.calico_button(button_id, [class_description, class_name, app_show](calico_button_control& pc) {
				pc.set_size(.95_container, 30.0_px);
				pc.text = class_description;

				json_parser jp;
				json new_object_request = jp.create_object();
				new_object_request.put_member("ClassName", class_name);

				pc.source_name = "calico";
				pc.function_name = "create_object";
				pc.function_parameters = new_object_request;
				pc.lake = app_show->data;
				});
		}

		/*  ---------------------------------------------------------------------------------------------------
			Viewing and manipulating new objects
			these we present in a basic form, with the edit fields of the base selected object, and then, the details (children), of each, in tabs.
		*/

		// first the edit area of the current object.

		/*
				int64_t current_edited_object_id = -1;
		std::string current_edited_class_name;
		std::string current_edited_class_description;

		the form view will be handled in parallel.

		*/

		// now the tab area

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

			ads.assets = [st, app_show](draw_control* _parent, rectangle _bounds) 
			{
				if (auto win = _parent->window.lock()) {
					auto& ctxt = win->getContext();

					textStyleRequest tsr = {};
					tsr.fontName = st->ParagraphTextFont;
					tsr.fontSize = 14;
					tsr.name = "item_paragraph";
					ctxt.setTextStyle(&tsr);

					solidBrushRequest sbr;
					sbr.active = true;
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
			};

			ads.draw_item = [app_show, ads](draw_control* _parent, int _index, rectangle _bounds) {
				auto json_object = ads.data[_index];
				if (json_object.has_member("Source")) {
					json_object = json_object["Source"];
				}
				auto object_members = json_object.get_members();
				double x = _bounds.x;
				double w = 0.0;
				if (auto win = _parent->window.lock()) {
					auto fieldResponse = app_show->data->get_result("calico", "fields");
					auto fieldsByName = fieldResponse["FieldsByName"];
					for (auto om : object_members) {
						auto field = fieldsByName[om.first];
						if (field.is_double()) {
							w = field["GridColumnWidth"];
						}
						else 
						{
							w = 100;
						}
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

			ads.size_item = [app_show](draw_control* _parent, int _index, rectangle _bounds) -> point {
				point p(_bounds.w, 40.0, 0.0);
				return p;
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

			new_tab.apply_data = [rule_description, ads](tab_pane& _pane, control_base* _cont) {
				grid_view* gv = dynamic_cast<grid_view *>(_cont);
				if (gv) {
					gv->set_item_source(ads);
				}
			};

			new_tab.create_tab_controls = [rule_description,ads](tab_pane& _pane, control_base* _cont) {
				control_builder cb;
				cb.grid_view_begin(id_counter::next(), [ads](grid_view& _layout) {
					_layout.set_item_source(ads);
					_layout.set_size(1.0_container, 1.0_container);
					});
				cb.apply_controls(_cont);
				};
			 
			tabs.push_back(new_tab);
		}

		tab_container->set_tabs(tabs);

		app_show->layout();

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
		std::shared_ptr<calico_client> calico_svc = std::make_shared<calico_client>(app_data, "localhost", 7277);

		// create the presentation - this holds the data of what is on screen, for various pages.
		std::shared_ptr<presentation> application_presentation = std::make_shared<presentation>(application);
		application_presentation->data = app_data;

		// and now we want the application presentation 
		app_data->on_changed("calico", "*", [application_presentation](json _params, data_lake* _lake, data_function* _set) -> int {
			application_presentation->onDataChanged(_params, _lake, _set );
			return 0;
		});

		auto logged_handler = [application_presentation](json _params, data_lake* _lake, data_function* _set)
		{
			application_presentation->onLogged(_lake);
			return 1;
		};
		app_data->on_logged(logged_handler);

		// and now wire the data to the presentation 
		// the presentation can invoke the data
		// the data invokes whatever client
		// json for analytics
		// off to C++ structures for heavy duty

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
				create_home_page(_pg, application, calico_svc, application_presentation, app_menu, st);
			});

		auto &login_page = application_presentation->create_page("login", [calico_svc, application_presentation, application, app_data, st, app_menu](page& _pg)
			{

				_pg.column_begin()
					.caption_bar(id_counter::next(), st, app_menu.get(), [](caption_bar_control& _cb)
						{
							_cb.menu_button_id = IDC_SYSTEM_MENU;
							_cb.image_control_id = IDC_COMPANY_LOGO;
							_cb.image_file = "assets\\small_logo.png";
							_cb.corporate_name = "COUNTRYBIT";
							_cb.title_name = "REVOLUTION";
							_cb.code_detail_id = IDC_STATUS_DETAIL;
							_cb.code_status_id = IDC_STATUS_MESSAGE;
						}
					)
					.end();

				_pg.on_select([calico_svc, application, application_presentation, app_data, st, app_menu](page_select_event _evt)
					{
						json_parser jp;
						auto params = jp.create_object();
						std::string user_name = application->getUserName();
						std::string model_name = "Security";
						params.put_member("UserName", user_name);
						params.put_member("ModelName", model_name);
						app_data->call_function("calico", "credentials", params);
					});

				_pg.on_changed(0, "calico", "fields", [application_presentation](page_data_event pde) {
					application_presentation->select_page("home");
					});

			});

		if (forceWindowed)
		{
			application->runDialog(hInstance, "COUNTRYBIT REVOLUTION", IDI_WSPROPOSE, false, application_presentation);
		}
		else
		{
			application->runDialog(hInstance, "COUNTRYBIT REVOLUTION", IDI_WSPROPOSE, true, application_presentation);
		}
	}

}

#endif
