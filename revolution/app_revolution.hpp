
#ifndef APP_REVOLUTION_H
#define APP_REVOLUTION_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{

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

	const int IDC_BTN_CANCEL = 2005;
	const int IDC_BTN_LOGIN = 2006;

	// menu options
	const int IDM_VIEW_MENU = 3001;
	const int IDM_HOME = 3002;
	const int IDM_LOGIN = 3003;

	// forms
	const int IDC_FORM_VIEW = 4001;

	// bits of status
	const int IDC_STATUS_MESSAGE = 5001;
	const int IDC_STATUS_DETAIL = 5002;

	class revolution_application
	{
	public:
		presentation_style* st;

		std::shared_ptr<directXAdapter> factory;

		// create a win32 set of windows that can use this factory
		// to make windows with a d2d / d3d image
		std::shared_ptr<directApplicationWin32> application;

		// create a win32 set of windows that can use this factory
		// to make windows with a d2d / d3d image
		std::shared_ptr<menu_item> application_menu;

		// create the data store and bind the calico client to it.
		std::shared_ptr<corona_client> corona_api;

		// create the presentation - this holds the data of what is on screen, for various pages.
		std::shared_ptr<presentation> presentation_layer;

		int id_caption_bar;
		int id_main_row;
		int id_command_container;
		int id_current_container;
		int id_location_title;
		int id_create_title;
		int id_form_view;
		int id_tab_view;
		int image_control_id;

		json login_object;
		json edited_object;
		json query_result;

		revolution_application()
		{
			std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();

			// create a win32 set of windows that can use this factory
			// to make windows with a d2d / d3d image
			application = std::make_shared<directApplicationWin32>(factory);

			// create the data store and bind the calico client to it.
			corona_api = std::make_shared<corona_client>("localhost", 7277, "CountrybitRevolution", application->getUserName());

			// create the presentation - this holds the data of what is on screen, for various pages.
			presentation_layer = std::make_shared<presentation>(application);

			application_menu = std::make_shared<menu_item>();

			application_menu->destination(IDM_HOME, "&Home", "home")
								.destination(IDM_LOGIN, "&Login", "login");

			st = styles.get_style();

			id_caption_bar = presentation_layer->get_control_id("caption_bar", []() { return id_counter::next(); });
			id_main_row = presentation_layer->get_control_id("main_row", []() { return id_counter::next(); });
			id_command_container = presentation_layer->get_control_id("command_container", []() { return id_counter::next(); });
			id_current_container = presentation_layer->get_control_id("current_container", []() { return id_counter::next(); });
			id_location_title = presentation_layer->get_control_id("location_title", []() { return id_counter::next(); });
			id_create_title = presentation_layer->get_control_id("create_title", []() { return id_counter::next(); });
			id_form_view = presentation_layer->get_control_id("get_ui_form_view", []() { return id_counter::next(); });
			id_tab_view = presentation_layer->get_control_id("get_ui_tab_view", []() { return id_counter::next(); });
			image_control_id = presentation_layer->get_control_id("get_ui_logo", []() { return id_counter::next(); });

			factory->refresh();

		}

		void run(HINSTANCE hInstance, bool forceWindowed)
		{
			if (forceWindowed)
			{
				application->runDialog(hInstance, "COUNTRYBIT REVOLUTION", IDI_REVOLUTION, false, presentation_layer);
			}
			else
			{
				application->runDialog(hInstance, "COUNTRYBIT REVOLUTION", IDI_REVOLUTION, true, presentation_layer);
			}
		}

		using content_function = std::function<void(control_builder& _contents)> ;

		void create_page_frame(page& _page, content_function _fn)
		{
			control_builder command_container;
			caption_bar_control* caption_container;
			tab_view_control* tab_container;
			form_view_control* form_view;
			// then we must be a new page

			if (_page.get_root()->children.size() == 0)
			{
				// First check to make sure we have all the things

				control_builder contents_root(_page.get_root_container());

				auto contents = contents_root.row_begin(
					id_main_row,
					[](row_layout& _settings) {
						_settings.set_size(1.0_container, 1.0_container);
					});

				// note that, we are putting the breadcrumbs on a nav pane to the left.
				auto command_container = contents.column_begin(id_command_container, [](column_layout& rl) {
					rl.set_size(300.0_px, 1.0_container);
					});

				command_container.image(image_control_id, "assets\\small_logo.png", [](image_control& control) {
					control.set_size(300.0_px, 300.0_px);
					});

				auto current_container = contents.column_begin(id_current_container, [](column_layout& rl) {
					rl.set_size(1.0_remaining, 1.0_container);
					});

				current_container.caption_bar(id_caption_bar, st, application_menu.get(), [](caption_bar_control& _cb)
					{
						_cb.menu_button_id = IDC_SYSTEM_MENU;
						_cb.image_control_id = IDC_COMPANY_LOGO;
						_cb.image_file = "assets\\small_logo.png";
						_cb.corporate_name = "COUNTRY VIDEO GAMES";
						_cb.title_name = "Revolution";
						_cb.code_detail_id = IDC_STATUS_DETAIL;
						_cb.code_status_id = IDC_STATUS_MESSAGE;
					}
				);

				_fn(current_container);


				contents_root.apply_controls(_page.root.get());
			}
		}

		void create_login_start_page (
			page& _page
		)
		{
			create_page_frame(_page, [](control_builder cb) 
			{
					cb.form_view(IDC_FORM_VIEW, [](form_view_control& _fv)
					{
						item_data_source ids;

						item_field iff;
						iff.field_id = IDC_USER_NAME;
						iff.field_label = "User name:";
						iff.field_type = "string";
						iff.json_member_name = "Name";
						ids.fields.push_back(iff);

						iff.field_id = IDC_USER_PASSWORD;
						iff.field_label = "Password";
						iff.field_type = "string";
						iff.json_member_name = "Name";
						ids.fields.push_back(iff);

						_fv.fields_per_column = 3;
						_fv.set_size(1.0_container, .15_container);
						_fv.set_data(ids);
					});
					cb.calico_button(IDC_BTN_LOGIN);
					cb.calico_button(IDC_BTN_CANCEL);
			});
		}

		void create_login_confirm_code_page (
			page& _page
		)
		{
			create_page_frame(_page, [](control_builder cb)
				{
					cb.form_view(IDC_FORM_VIEW, [](form_view_control& _fv)
						{
							item_data_source ids;
							item_field iff;
							iff.field_id = IDC_USER_CONFIRMATION;
							iff.field_label = "Confirmation Code:";
							iff.field_type = "string";
							iff.json_member_name = "ConfirmationCode";
							ids.fields.push_back(iff);

							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, .15_container);
							_fv.set_data(ids);
						});
					cb.calico_button(IDC_BTN_LOGIN);
					cb.calico_button(IDC_BTN_CANCEL);
				});
		}

		void create_login_password_page (
			page& _page
		)
		{
			create_page_frame(_page, [](control_builder cb)
				{
					cb.form_view(IDC_FORM_VIEW, [](form_view_control& _fv)
						{
							item_data_source ids;
							item_field iff;

							iff.field_id = IDC_USER_PASSWORD1;
							iff.field_label = "Password 1:";
							iff.field_type = "string";
							iff.json_member_name = "Password1";
							ids.fields.push_back(iff);

							iff.field_id = IDC_USER_PASSWORD2;
							iff.field_label = "Password 2:";
							iff.field_type = "string";
							iff.json_member_name = "Password2";
							ids.fields.push_back(iff);

							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, .15_container);
							_fv.set_data(ids);
						});
					cb.calico_button(IDC_BTN_LOGIN);
					cb.calico_button(IDC_BTN_CANCEL);
				});
		}

		void create_object_edit_page(
			page& _page
		)
		{
			control_builder command_container;
			caption_bar_control* caption_container;
			tab_view_control* tab_container;
			form_view_control* form_view;
			std::vector<tab_pane> tabs;

			// then we must be a new page

			create_page_frame(_page, [](control_builder cb)
				{
					cb.form_view(IDC_FORM_VIEW, [](form_view_control& _fv)
						{
							_fv.fields_per_column = 3;
							_fv.set_size(1.0_container, .15_container);
						});
					cb.calico_button(IDC_BTN_LOGIN);
					cb.calico_button(IDC_BTN_CANCEL);
				});

			command_container = _page.edit(id_command_container);
			caption_container = _page.find_container<caption_bar_control>(id_caption_bar);
			tab_container = _page.find_container<tab_view_control>(id_tab_view);

			command_container.chaptertitle("Create New", [](chaptertitle_control& ct) {
				ct.set_size(.95_container, 50.0_px);
				ct.text_style.horizontal_align = visual_alignment::align_center;
				ct.text_style.vertical_align = visual_alignment::align_center;
				}, id_create_title);

			// first we put a caption bar in our standard page and our stock form.

			json edited_data;
			json edited_class;
			json edited_fields;
			json edited_build;
			std::string class_name;

			json token = edited_object["Token"];
			edited_data = edited_object["Data"];
			edited_class = edited_data["ClassDefinition"];
			class_name = edited_data["ClassName"];
			edited_fields = edited_object["Fields"];
			edited_build = edited_object["Edit"];

			json_parser jp;
			item_data_source ids;
			ids.name = edited_class["Description"];
			ids.data = edited_data;

			auto members = edited_fields.get_members();
			for (auto field : members)
			{
				std::string field_name;
				std::string field_type;
				json field_choices;
				json field_options;
				item_field new_field;

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

					new_field.field_label = field_options["Description"];
					new_field.field_tooltip = field_options["ToolTip"];
					new_field.choice_options.id_field = field_choices["ValueField"];
					new_field.choice_options.text_field = field_choices["NameField"];
					new_field.choice_options.items = field_choices["Items"];
				}
				else 
				{
					field_type = field.second;
					new_field.field_label = field_name;
					new_field.field_tooltip = "";
				}

				if (field_choices.is_object())
				{
					new_field.field_type == "combobox";
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

			auto fv = presentation_layer->get_control<form_view_control>(id_form_view);
			if (fv) {
				fv->set_data(ids);
			}
			tab_container->set_tabs(tabs);


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
			std::string primary_key_field_name = class_obj["PrimaryKeyField"];

			// and load it into our form editor...


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
				new_field.field_id = presentation_layer->get_control_id("form_view_" + fieldName, []() { return id_counter::next(); });

			}

			/*  ---------------------------------------------------------------------------------------------------
				Creating New Objects
			*/


			for (int i = 0; i < create_options.size(); i++)
			{
				auto co = create_options.get_element(i);

				// then, fish out the stuff we need
				std::string class_name = co["CreateClassName"].get_string();
				std::string class_description = co["CreateClassDescription"].get_string();

				// then, come up with a canonical name for the button, so that, if we keep invoking this, we have the same ids.
				// we also use this, to tie to our data, which actually drives the application
				std::string button_name = "create_class." + class_name;

				int button_id = presentation_layer->get_control_id(button_name, []() { return id_counter::next(); });

				command_container.calico_button(button_id, [this, class_description, class_name](calico_button_control& pc) {
					pc.set_size(.95_container, 40.0_px);
					pc.text = class_description;

					json_parser jp;
					json new_object_request = jp.create_object();
					new_object_request.put_member("ClassName", class_name);

					pc.options.corona_client = this->corona;
					pc.options.credentials = jp.create_object();
					pc.options.function_data = jp.create_object();
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
				auto selected_rule = select_options.get_element(i);

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
						json member_obj = member_array.get_element(k);
						json item_obj = jp.create_object();
						item_obj.copy_member("ClassName", member_obj);
						item_obj.copy_member("Name", member_obj);
						item_obj.put_member("Source", member_obj);
						ads.data.put_element(-1, item_obj);
					}
				}
			

		}

	};

	revolution_application current_application;


	void run_developer_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
	{
		application app;

		EnableGuiStdOuts();

		// and now wire the data to the presentation 
		// the presentation can invoke the data
		// the data invokes whatever client
		// json for analytics
		// off to C++ structures for heavy duty

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

	}

}

#endif
