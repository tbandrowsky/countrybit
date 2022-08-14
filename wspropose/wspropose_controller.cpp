
#pragma once


#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

namespace proposal
{

	using namespace corona::database;

	wsproposal_controller::wsproposal_controller() : corona_controller()
	{
		/*

		In this example we create our schema and data store locally.
		In a remote application, we will wind up fetching and binding but that will be for a different example.

		*/

		/* first we initialize our box.  Our box can grow, but we start with a size of 16MB, which is more than enough
		* for demo purposes.
		*/

		box.init(1 << 23);

		/* with our box, we can then create a schema.  We'll reserve space to create roughly 75 classes,
		* and also include standard fields and classes.
		*/

		schema = jschema::create_schema(&box, 500, true, schema_id);

		/*
		Now, we can begin to flesh out schema for this application.  In Corona, a schema is a collection of fields,
		that are composed into classes.  There are many different types of fields, and only some are used below.
		Some C++ syntactical sugar makes the process less painful.  Note that the id of each field returned goes
		into a variable with the idf naming convention.
		*/

		put_class_request pcr;
		put_object_field_request porf;

		/*
		These fields are all custom integer fields that we will use to represent primary keys.
		Primary keys in Corona are like a cross between sequences and auto number.  The field itself
		is just an integer, but it will be assigned as a PK as part of its class membership.
		*/

		/* This is the application home class.  It is just a place holder now */
		pcr.class_name = "home";
		pcr.class_description = "Home";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.idf_name, schema.idf_style_id };
		idc_home = schema.put_class(pcr);

		pcr.class_name = "carrier_root";
		pcr.class_description = "Carriers";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_home), schema.idf_name, schema.idf_search_string, schema.idf_style_id };
		idc_carrier_root = schema.put_class(pcr);

		pcr.class_name = "carrier";
		pcr.class_description = "Carrier";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_carrier_root), schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_carrier = schema.put_class(pcr);

		pcr.class_name = "coverage_root";
		pcr.class_description = "Coverages";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_home), schema.idf_name, schema.idf_search_string, schema.idf_style_id };
		idc_coverage_root = schema.put_class(pcr);

		pcr.class_name = "coverage";
		pcr.class_description = "Coverage";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_coverage_root), schema.idf_name };
		idc_coverage = schema.put_class(pcr);

		/* This is the client root class. Objects of the class have a layout and accept the client home id as a relation.  There is also
		a standard search string field, for searching for clients and keeping the search state. */
		pcr.class_name = "client_root";
		pcr.class_description = "Clients";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_home), schema.idf_name, schema.idf_search_string, schema.idf_style_id };
		idc_client_root = schema.put_class(pcr);
		
		pcr.class_name = "client";
		pcr.class_description = "Client";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_client_root), schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_client = schema.put_class(pcr);

		pcr.class_name = "program";
		pcr.class_description = "Program";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_client), schema.idf_name };
		idc_program = schema.put_class(pcr);

		pcr.class_name = "program_item";
		pcr.class_description = "Program Item";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_program), schema.idf_name };
		idc_program_item = schema.put_class(pcr);

		relative_ptr_type idf_insurance_carrier = schema.put_string_field({ {  jtype::type_string, "insurance_carrier", "Carrier" }, { 500, "", "", } });
		relative_ptr_type idf_insurance_inception = schema.put_time_field({ {  jtype::type_datetime, "insurance_inception", "Inception Dt" }, { 0, INT64_MAX } });
		relative_ptr_type idf_insurance_expiration = schema.put_time_field({ {  jtype::type_datetime, "insurance_expiration", "Expiration Dt" }, { 0, INT64_MAX } });
		relative_ptr_type idf_insurance_attachment = schema.put_double_field({ {  jtype::type_float32, "insurance_attachment", "Attachment $" }, { 0.0, 1E10 } });
		relative_ptr_type idf_insurance_limit = schema.put_double_field({ {  jtype::type_float32, "insurance_limit", "Limit $" }, { 0.0, 1E10 } });
		relative_ptr_type idf_insurance_premium = schema.put_double_field({ {  jtype::type_float32, "insurance_premium", "Premium $" }, { 0.0, 1E10 } });
		relative_ptr_type idf_insurance_deductible = schema.put_double_field({ {  jtype::type_float32, "insurance_deductible", "Deductible $" }, { 0.0, 1E10 } });
		relative_ptr_type idf_insurance_share = schema.put_double_field({ {  jtype::type_float32, "insurance_share", "Share %" }, { 0.0, 100 } });
		relative_ptr_type idf_insurance_comment = schema.put_string_field({ {  jtype::type_string, "insurance_comment", "Comment" }, { 500, "", "", } });

		pcr.class_name = "insurance_item";
		pcr.class_description = "Insurance Item";
		pcr.auto_primary_key = false;
		pcr.base_class_id = idc_program_item;
		pcr.member_fields = { idf_insurance_carrier, idf_insurance_inception, idf_insurance_expiration, idf_insurance_comment, idf_insurance_premium };
		idc_program_insurance = schema.put_class(pcr);

		relative_ptr_type idf_insurance_coverage_name = schema.put_string_field({ {  jtype::type_string, "insurance_coverage_name", "Coverage", false }, { 50 } });
		relative_ptr_type idf_insurance_coverage_comment = schema.put_string_field({ {  jtype::type_string, "insurance_coverage_comment", "Comment", false }, { 250 } });
		relative_ptr_type idf_insurance_coverage_color = schema.put_color_field({ {  jtype::type_string, "insurance_coverage_color", "Color", false } });
		pcr.class_name = "insurance_item_coverage";
		pcr.class_description = "Item Coverage";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_program_item), schema.get_primary_key(idc_coverage), 
								idf_insurance_coverage_name, idf_insurance_coverage_color, idf_insurance_coverage_comment, idf_insurance_attachment, idf_insurance_limit, idf_insurance_deductible, idf_insurance_share
							};
		idc_program_insurance_coverage = schema.put_class(pcr);

		pcr.class_name = "system_root";
		pcr.class_description = "System";
		pcr.auto_primary_key = true;
		pcr.member_fields = { schema.get_primary_key(idc_home), schema.idf_search_string, schema.idf_style_id };
		idc_system_root = schema.put_class(pcr);

		jmodel jm;

		jm.update_when(&schema, {}, idc_home, {});
		jm.update_when(&schema, {}, idc_carrier_root, {});
		jm.update_when(&schema, {}, idc_carrier, {});
		jm.update_when(&schema, {}, idc_coverage_root, {});
		jm.update_when(&schema, {}, idc_program, {});
		jm.update_when(&schema, {}, idc_program_item, {});
		jm.update_when(&schema, {}, idc_program_insurance_coverage, {});
		jm.update_when(&schema, {}, idc_coverage, {});
		jm.update_when(&schema, {}, idc_client_root, {});
		jm.update_when(&schema, {}, idc_client, {});
		jm.update_when(&schema, {}, idc_system_root, {});

		vq_navigation.classes = { idc_home, idc_carrier_root, idc_coverage_root, idc_client_root, idc_system_root };
		vq_navigation.fields = { schema.idf_name };
		vq_navigation.query_name = "navigation";

		vq_carriers.classes = { idc_carrier };
		vq_carriers.query_name = "carriers";
		auto comp = vq_carriers.parameters.append();
		comp->comparison = comparisons::eq;
		comp->field_id_source = schema.get_primary_key(idc_carrier);
		comp->field_id_target = schema.get_primary_key(idc_carrier);

		vq_clients.classes = { idc_client };
		vq_clients.query_name = "clients";
		comp = vq_client.parameters.append();
		comp->comparison = comparisons::eq;
		comp->field_id_source = schema.get_primary_key(idc_client);
		comp->field_id_target = schema.get_primary_key(idc_client);

		vq_coverages.classes = { idc_coverage };
		vq_coverages.query_name = "coverages";
		comp = vq_coverages.parameters.append();
		comp->comparison = comparisons::eq;
		comp->field_id_source = schema.get_primary_key(idc_coverage);
		comp->field_id_target = schema.get_primary_key(idc_coverage);

		vq_client.classes = { idc_client, idc_program };
		comp = vq_client.parameters.append();
		comp->comparison = comparisons::eq;
		comp->field_id_source = schema.get_primary_key(idc_client);
		comp->field_id_target = schema.get_primary_key(idc_client);
		vq_client.query_name = "client";

		vq_program.classes = { idc_program, idc_program_item, idc_program_insurance_coverage };
		vq_program.query_name = "program";
		comp = vq_program.parameters.append();
		comp->comparison = comparisons::eq;
		comp->field_id_source = schema.get_primary_key(idc_program);
		comp->field_id_target = schema.get_primary_key(idc_program);

		view_options vo_home;
		vo_home.use_view = true;
		vo_home.view_class_id = idc_home;
		vo_home.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_home, {}, {}, vo_home);

		view_options vo_carrier_root;
		vo_carrier_root.use_view = true;
		vo_carrier_root.view_class_id = idc_carrier_root;
		vo_carrier_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_home }, idc_carrier_root, {}, {}, vo_carrier_root);

		view_options vo_carrier;
		vo_carrier.use_view = true;
		vo_carrier.view_class_id = idc_carrier;
		vo_carrier.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_carrier_root }, idc_carrier, {}, {}, vo_carrier);

		view_options vo_coverage_root;
		vo_coverage_root.use_view = true;
		vo_coverage_root.view_class_id = idc_coverage_root;
		vo_coverage_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_home }, idc_coverage_root, {}, {}, vo_coverage_root);

		view_options vo_coverage;
		vo_coverage.use_view = true;
		vo_coverage.view_class_id = idc_coverage;
		vo_coverage.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_coverage_root }, idc_coverage, {}, {}, vo_coverage);

		view_options vo_client_root;
		vo_client_root.use_view = true;
		vo_client_root.view_class_id = idc_client_root;
		vo_client_root.view_queries.push_back(vq_navigation);
		vo_client_root.view_queries.push_back(vq_client);
		jm.select_when(&schema, { idc_home }, idc_client_root, {}, {}, vo_client_root);

		view_options vo_client;
		vo_client.use_view = true;
		vo_client.view_class_id = idc_client;
		vo_client.view_queries.push_back(vq_navigation);
		vo_client.view_queries.push_back(vq_client);
		jm.select_when(&schema, { idc_client_root }, idc_client, {}, {}, vo_client);

		view_options vo_program;
		vo_client.use_view = true;
		vo_client.view_class_id = idc_program;
		vo_client.view_queries.push_back(vq_navigation);
		vo_client.view_queries.push_back(vq_program);
		jm.select_when(&schema, { idc_client }, idc_program, {}, {}, vo_program);

		view_options vo_system_root;
		vo_system_root.use_view = true;
		vo_system_root.view_class_id = idc_system_root;
		vo_system_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_system_root, {}, {}, {});

		jm.delete_when(&schema, {}, idc_carrier, {});
		jm.delete_when(&schema, {}, idc_coverage, {});
		jm.delete_when(&schema, {}, idc_client, {});

		jm.create_when(&schema, { }, idc_home, null_row, true, false, 1, { idc_carrier_root, idc_coverage_root, idc_client_root, idc_system_root });
		jm.create_when(&schema, { idc_home }, idc_carrier_root, null_row, true, false, 1, {});
		jm.create_when(&schema, { idc_home }, idc_coverage_root, null_row, true, false, 1, {});
		jm.create_when(&schema, { idc_home }, idc_client_root, null_row, true, false, 1, {});
		jm.create_when(&schema, { idc_home }, idc_system_root, null_row, true, false, 1, { });
		jm.create_when(&schema, { idc_carrier_root }, idc_carrier, null_row, true, false, 100, {});
		jm.create_when(&schema, { idc_coverage_root }, idc_coverage, null_row, true, false, 10, {});
		jm.create_when(&schema, { idc_client_root }, idc_client, null_row, true, false, 100, {});
		jm.create_when(&schema, { idc_client }, idc_program, null_row, false, false, 20, {});
		jm.create_when(&schema, { idc_program }, idc_program_item, null_row, false, false, 50, {});
		jm.create_when(&schema, { idc_program_insurance }, idc_program_insurance_coverage, null_row, false, false, 50, {});

		schema.put_model(jm);

		jcollection_ref ref;
		ref.data = &box;
		ref.model_name = jm.name;
		ref.max_actors = 2;
		ref.max_objects = 100;
		ref.collection_size_bytes = 1 << 19;
		ref.create_style_sheet = true;

		if (!init_collection_id(ref.collection_id))
		{
			std::cout << __LINE__ << "collection id failed" << std::endl;
		}

		user_collection = schema.create_collection(&ref);

		jactor sample_actor;
		sample_actor.actor_name = "sample actor";
		sample_actor.actor_id = null_row;
		sample_actor = user_collection.create_actor(sample_actor);

		actor_id = sample_actor.actor_id;

		set_style_sheets();

		create_object_request cor;

		cor.actor_id = actor_id;
		cor.class_id = idc_home;
		cor.collection_id = user_collection.get_collection_id();
		cor.item_id = -1;
		cor.select_on_create = true;
		cor.template_item_id = -1;

		// create our initial object, and get our state.
		state = user_collection.create_object(cor, "create home");

		user_collection.update(vq_navigation, &sample_actor, [this](jobject& src) { 
			src.set({ { schema.idf_name, src.get_class().item().description }});
			return true;
		});

	}

	wsproposal_controller::~wsproposal_controller()
	{
		;
	}

	void wsproposal_controller::render_navigation_frame(std::function<void(page_item* navigation_contents, page_item* _frame)> _contents)
	{
		clear();

		const char* _title = application_title;
		const char* _subtitle = application_author;

		auto page_column = column(nullptr, null_row, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		auto title_bar = canvas2d_row(id_canvas_header, page_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 45.0_px });
		text(title_bar, schema.idf_album_title_style, _title);

		auto main_row = row(page_column, null_row, { 0.0_px, 15.0_px, 100.0_pct, 100.0_pct });
		auto navigation_contents = canvas2d_column(id_canvas_navigation, main_row, schema.idf_panel_style, { 0.0_px, 0.0_px, 200.0_px, 100.0_pct });

		selectable_items(navigation_contents, vq_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 1.2_fntgr });

		auto form_contents = column(main_row, null_row, { 16.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		form_contents->caption = pg.copy("contents form");
		_contents(navigation_contents, form_contents);

		auto footer_bar = canvas2d_row(id_canvas_footer, page_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
		text(footer_bar, schema.idf_artist_title_style, _subtitle);
	}


	void wsproposal_controller::render_home()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_home_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_client_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_client_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_client()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_client_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_coverage_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_coverage_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_coverage()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_coverage_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_carrier_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_carrier_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_system_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_system_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_carrier()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_carrier_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_program()
	{
		;
	}

	void wsproposal_controller::render_home_contents(page_item* _navigation, page_item* _contents)
	{
	}

	void wsproposal_controller::render_client_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };		

		table_options options;

		options.alternating_row = true;
		options.data = &vq_clients;
		options.header_height = 1.2_fntgr;
		options.row_height = 1.2_fntgr;
		options.columns = { 
			{ "Name", "", 100.0_px, schema.idf_name },
			{ "Street", "", 200.0_px, schema.idf_name },
			{ "City", "", 100.0_px, schema.idf_name },
			{ "State", "", 100.0_px, schema.idf_name },
			{ "Postal", "", 100.0_px, schema.idf_name }		
		};

		search_form(_navigation, _contents, id_canvas_table, idc_client_root, options, "Clients", { schema.idf_search_string });
	}

	void wsproposal_controller::render_client_contents(page_item* _navigation, page_item* _contents)
	{
		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.view.view_class_id).item().description;

		auto edit_body = row(_contents, null_row, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		auto control = column(edit_body, schema.idf_view_background_style, { 0.0_pct, 0.0_px, 30.0_pct, 100.0_pct });
		auto client_id = state.get_selected(idc_client);
		edit_fields(control, client_id, field_layout::label_on_top, "Client Details", { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal });
		space(control, schema.idf_button_style, { 0.0_px, 0.0_px, 1.0_fntgr, 1.0_fntgr });
		create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });

		auto children = canvas2d_column(id_canvas_table, edit_body, schema.idf_view_background_style, { 0.0_px, 0.0_px, 65.0_pct, 100.0_pct });

		table_options options;
		options.alternating_row = true;
		options.data = &vq_client;
		options.header_height = 1.2_fntgr;
		options.row_height = 1.2_fntgr;
		options.columns = {
			{ "Name", "", 100.0_px, schema.idf_name },
			{ "Street", "", 200.0_px, schema.idf_street },
			{ "City", "", 100.0_px, schema.idf_city },
			{ "State", "", 100.0_px, schema.idf_state },
			{ "Postal", "", 100.0_px, schema.idf_postal }
		};

		table(children, options);
	}

	void wsproposal_controller::render_coverage_root_contents(page_item* _navigation, page_item* _contents)
	{
		table_options options;
		options.alternating_row = true;
		options.data = &vq_coverages;
		options.header_height = 1.2_fntgr;
		options.row_height = 1.2_fntgr;
		options.columns = {
			{ "Name", "", 100.0_px, schema.idf_name },
			{ "Street", "", 200.0_px, schema.idf_street },
			{ "City", "", 100.0_px, schema.idf_city },
			{ "State", "", 100.0_px, schema.idf_state },
			{ "Postal", "", 100.0_px, schema.idf_postal }
		};

		search_form(_navigation, _contents, id_canvas_table, idc_coverage_root, options, "Coverages", { schema.idf_search_string });
	}

	void wsproposal_controller::render_program_contents(page_item* _navigation, page_item* _contents)
	{
		text(_contents, schema.idf_album_about_style, "Program Details");
	}

	void wsproposal_controller::render_system_root_contents(page_item* _navigation, page_item* _contents)
	{
		text(_contents, schema.idf_album_about_style, "System Root");
	}

	void wsproposal_controller::render_coverage_contents(page_item* _navigation, page_item* _contents)
	{
		auto sel = state.get_selected(idc_coverage);
		edit_form(_navigation, _contents, sel, "Coverage", { schema.idf_name });
	}

	void wsproposal_controller::render_carrier_root_contents(page_item* _navigation, page_item* _contents)
	{
		table_options options;
		options.alternating_row = true;
		options.data = &vq_carriers;
		options.header_height = 1.2_fntgr;
		options.row_height = 1.2_fntgr;
		options.columns = {
			{ "Name", "", 100.0_px, schema.idf_name },
			{ "Street", "", 200.0_px, schema.idf_street },
			{ "City", "", 100.0_px, schema.idf_city },
			{ "State", "", 100.0_px, schema.idf_state },
			{ "Postal", "", 100.0_px, schema.idf_postal }
		};

		search_form(_navigation, _contents, id_canvas_table, idc_carrier_root, options, "Carriers", { schema.idf_search_string });
	}

	void wsproposal_controller::render_carrier_contents(page_item* _navigation, page_item* _contents)
	{
		auto sel = state.get_selected(idc_carrier);
		edit_form(_navigation, _contents, sel, "Carrier", { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal});
	}

	void wsproposal_controller::render(const rectangle& newSize)
	{
		if (state.actor.view.view_class_id == idc_home) 
		{
			render_home();
		} 
		else if (state.actor.view.view_class_id == idc_client_root)
		{
			render_client_root();
		}
		else if (state.actor.view.view_class_id == idc_client)
		{
			render_client();
		}
		else if (state.actor.view.view_class_id == idc_coverage_root)
		{
			render_coverage_root();
		}
		else if (state.actor.view.view_class_id == idc_coverage)
		{
			render_coverage();
		}
		else if (state.actor.view.view_class_id == idc_carrier_root)
		{
			render_carrier_root();
		}
		else if (state.actor.view.view_class_id == idc_carrier)
		{
			render_carrier();
		}
		else if (state.actor.view.view_class_id == idc_system_root)
		{
			render_system_root();
		}

		auto style_sheet = get_style_sheet(0);
		arrange(newSize.w, newSize.h, style_sheet, 16);
		canvasWindowsId = host->renderPage(pg, &schema, state, user_collection);
		host->redraw();
	}

	void wsproposal_controller::set_style_sheets()
	{
		for (int i = 0; i < 4; i++)
		{
			set_style_sheet(i);
		}
	}

	void wsproposal_controller::set_style_sheet(int _index)
	{
		auto style_sheet = get_style_sheet(_index);

		const char* fontName = "Open Sans,Arial";

		style_sheet.set(
			{ schema.idf_view_background_style },
			{
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "#000000FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_background_style,
			{ schema.idf_view_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_panel_style },
			{
				{ schema.idf_box_fill_color, "#EFEFEFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#CFCFCFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_view_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_view_subtitle_style },
			{
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_bold, false },
			}
			);

		auto color_check_src = style_sheet.get_object(schema.idf_view_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);
		auto color_check_dest = style_sheet.get_object(schema.idf_view_subtitle_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_view_section_style },
			{
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_disclaimer_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_copyright_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h1_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h2_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h3_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 20.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_number_head_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#1F2A44FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#1F2A44FF" }
			}
			);

		style_sheet.set(
			schema.idf_column_number_head_style,
			{ schema.idf_column_text_head_style },
			{
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
			}
			);

		style_sheet.set(
			schema.idf_column_number_head_style,
			{ schema.idf_column_number_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_number_style },
			{
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_data_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_label_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_control_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, true },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_axis_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_legend_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_block_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_tooltip_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_error_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_deductible_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_header_area_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "#ffffffFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_title_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_shape_fill_color, "#319B42FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_subtitle_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_breadcrumb_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_breadcrumb_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#5284A3FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#DEE1E1FF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		if (_index == style_normal)
		{
			style_sheet.set(
				schema.idf_view_style,
				{ schema.idf_button_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#CCCCCCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#319B42FF" },
				{ schema.idf_box_border_thickness, 2 },
				{ schema.idf_box_border_color, "#319B42FF" }
			}
			);
		}
		else if (_index == style_over)
		{
			style_sheet.set(
				schema.idf_view_style,
				{ schema.idf_button_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#319B42FF" },
				{ schema.idf_box_border_thickness, 2 },
				{ schema.idf_box_border_color, "#319B42FF" }
			}
			);

		}
		else if (_index == style_selected)
		{
			style_sheet.set(
				schema.idf_view_style,
				{ schema.idf_button_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, true },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#319B42FF" },
				{ schema.idf_box_border_thickness, 2 },
				{ schema.idf_box_border_color, "#319B42FF" }
			}
			);
		}

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 40.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title1_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title2_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title3_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title4_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title5_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title6_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_navigation_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
			}
			);

	}

}
