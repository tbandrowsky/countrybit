
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

		idf_home = schema.put_integer_field({ {  jtype::type_int64, "home_id", "Home Id", false }, { 0, INT64_MAX } });
		idf_client_root = schema.put_integer_field({ {  jtype::type_int64, "client_root", "Clients", false }, { 0, INT64_MAX } });
		idf_client = schema.put_integer_field({ {  jtype::type_int64, "client_id", "Client Id", false }, { 0, INT64_MAX } });
		idf_carrier_root = schema.put_integer_field({ {  jtype::type_int64, "carrier_root", "Carriers", false }, { 0, INT64_MAX } });
		idf_carrier = schema.put_integer_field({ {  jtype::type_int64, "carrier_id", "Carrier Id", false }, { 0, INT64_MAX } });
		idf_program_template_root = schema.put_integer_field({ {  jtype::type_int64, "home", "Home id", false }, { 0, INT64_MAX } });
		idf_program_template = schema.put_integer_field({ {  jtype::type_int64, "product_template", "Product Template id", false }, { 0, INT64_MAX } });
		idf_coverage_root = schema.put_integer_field({ {  jtype::type_int64, "coverage_root", "Coverages", false }, { 0, INT64_MAX } });
		idf_coverage = schema.put_integer_field({ {  jtype::type_int64, "coverage_id", "Coverage Id", false }, { 0, INT64_MAX } });
		idf_system_root = schema.put_integer_field({ {  jtype::type_int64, "system_root", "System Settings", false }, { 0, INT64_MAX } });

		idf_program = schema.put_integer_field({ {  jtype::type_int64, "program_id", "Program Id", false }, { 0, INT64_MAX } });
		idf_program_item_base = schema.put_integer_field({ {  jtype::type_int64, "program_item_id", "Program Item Id", false }, { 0, INT64_MAX } });
		idf_program_view = schema.put_integer_field({ {  jtype::type_int64, "program_view", "Program View", false }, { 0, INT64_MAX } });
		idf_program_title = schema.put_string_field({ {  jtype::type_string, "program_title", "Program Title", true }, { 100, "", "" } });
		idf_program_status = schema.put_string_field({ {  jtype::type_string, "program_status", "Program Status", true }, { 100, "", "" } });
		idf_program_subtitle = schema.put_string_field({ {  jtype::type_string, "program_title", "Program Subtitle", true }, { 100, "", "" } });

		idf_carrier_name = schema.put_string_field({ {  jtype::type_string, "carrier_name", "Carrier Name", true }, { 100, "", "" } });
		idf_coverage_name = schema.put_string_field({ {  jtype::type_string, "carrier_name", "Coverage Name", true }, { 100, "", "" } });

		idf_product = schema.put_integer_field({ {  jtype::type_int64, "product_id", "Product Id", false }, { 0, INT64_MAX } });
		idf_product_item_base = schema.put_integer_field({ {  jtype::type_int64, "product_item_id", "Product Item Id", false }, { 0, INT64_MAX } });
		idf_program_chart_slide = schema.put_integer_field({ {  jtype::type_int64, "program_chart_slide_id", "Program Chart Slide Id", false }, { 0, INT64_MAX } });
		idf_program_chart_slide_product = schema.put_integer_field({ {  jtype::type_int64, "program_chart_slide_product_id", "Program Chart Slide Item Id", false}, { 0, INT64_MAX } });
		idf_program_generic_slide = schema.put_integer_field({ {  jtype::type_int64, "program_generic_slide", "Generic Slide Id", false }, { 0, INT64_MAX } });

		/*
		* Here we are going to specify a set of fields that may be used to create a program item.
		* A model in Corona is a way to tie together objects that are affiliated but necessarily members per se, although syntactical
		* sugar will be introduced to make it -seem- that way.
		*/

		idf_inception = schema.put_time_field({ {  jtype::type_datetime, "inception", "Inception" }, { 0, INT64_MAX } });
		idf_expiration = schema.put_time_field({ {  jtype::type_datetime, "expiration", "Expiration" }, { 0, INT64_MAX } });
		idf_status = schema.put_integer_field({ {  jtype::type_int32, "status", "Status" }, { 0, INT64_MAX } });
		idf_attachment = schema.put_double_field({ {  jtype::type_float32, "attachment", "Attachment" }, { 0.0, 1E10 } });
		idf_limit = schema.put_double_field({ {  jtype::type_float32, "limit", "Limit" }, { 0.0, 1E10 } });
		idf_deductible = schema.put_double_field({ {  jtype::type_float32, "deductible", "Deductible" }, { 0.0, 1E10 } });
		idf_share = schema.put_double_field({ {  jtype::type_float32, "share", "Share %" }, { 0.0, 100 } });
		idf_comment = schema.put_string_field({ {  jtype::type_string, "comment", "Comment" }, { 500, "", "", } });
		idf_feature1_text = schema.put_string_field({ {  jtype::type_string, "feature1_text", "Feature1 Text" }, { 500, "", "", } });
		idf_feature2_text = schema.put_string_field({ {  jtype::type_string, "feature2_text", "Feature2 Text" }, { 500, "", "", } });

		/*
		Now that we have our fields, we can make our classes.
		*/

		/*
		We have a class object to represent our application states and objects with respect to this model.  We supply a class name, description, a list
		of member fields, and then, a primary key.  We call put_class to upsert the class, and the class id is returned for easy reference.
		The schema can also be searched by the class_name to find the class.
		*/

		/* This is the application home class.  It is just a place holder now */
		pcr.class_name = "home";
		pcr.class_description = "HOME";
		pcr.field_id_primary_key = idf_home;
		pcr.member_fields = { idf_home, schema.idf_style_id };
		idc_home = schema.put_class(pcr);

		/* This is the client root class. Objects of the class have a layout and accept the client home id as a relation.  There is also
		a standard search string field, for searching for clients and keeping the search state. */
		pcr.class_name = "client_root";
		pcr.class_description = "CLIENTS";
		pcr.field_id_primary_key = idf_client_root;
		pcr.member_fields = { idf_client_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_client_root = schema.put_class(pcr);

		/* This is the client class. Objects of the class have client things like name and address.  The primary key is the
		field id idf_client, which is populated when objects of this class are constructed. . */

		pcr.class_name = "client";
		pcr.class_description = "CLIENT";
		pcr.field_id_primary_key = idf_client;
		pcr.member_fields = { idf_client, idf_client_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_client = schema.put_class(pcr);

		/* This is the carrier root class. Objects of the class have a layout and accept the idf_home as a relation and have a primary key
		that is auto-opulated at object load time.  There is also
			a standard search string field, for searching for clients and keeping the search state. */

		pcr.class_name = "carrier_root";
		pcr.class_description = "CARRIERS";
		pcr.field_id_primary_key = idf_carrier_root;
		pcr.member_fields = { idf_carrier_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_carrier_root = schema.put_class(pcr);

		/* This is the carrier class. Objects of the class have client things like name and address.  The primary key is the
field id idf_carrier, which is populated when objects of this class are constructed. . */

		pcr.class_name = "carrier";
		pcr.class_description = "CARRIER";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_carrier, idf_carrier_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_carrier = schema.put_class(pcr);

		pcr.class_name = "coverage_root";
		pcr.class_description = "COVERAGES";
		pcr.field_id_primary_key = idf_coverage_root;
		pcr.member_fields = { idf_coverage_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_coverage_root = schema.put_class(pcr);

		pcr.class_name = "coverage";
		pcr.class_description = "COVERAGE";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_coverage, idf_coverage_root, schema.idf_name };
		idc_coverage = schema.put_class(pcr);

		pcr.class_name = "system_root";
		pcr.class_description = "SYSTEM";
		pcr.field_id_primary_key = idf_system_root;
		pcr.member_fields = { idf_system_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_system_root = schema.put_class(pcr);

		pcr.class_name = "program_templates";
		pcr.class_description = "PRODUCTS";
		pcr.field_id_primary_key = idf_program_template_root;
		pcr.member_fields = { idf_program_template_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_program_template_root = schema.put_class(pcr);

		pcr.class_name = "program_template";
		pcr.class_description = "PRODUCT";
		pcr.field_id_primary_key = idf_program_template;
		pcr.member_fields = { idf_program_template_root, idf_program_template, idf_home };
		idc_program_template = schema.put_class(pcr);

		pcr.class_name = "program";
		pcr.class_description = "PROGRAM";
		pcr.field_id_primary_key = idf_program;
		pcr.member_fields = { idf_program,
			idf_client,
			idf_program_view,
			schema.idf_name,
			idf_program_status,
			idf_program_title,
			idf_program_subtitle,
			idf_inception,
			idf_expiration
		};
		idc_program = schema.put_class(pcr);

		pcr.class_name = "program_item_base";
		pcr.class_description = "PROGRAM ITEM";
		pcr.field_id_primary_key = idf_program_item_base;
		pcr.member_fields = {
			idf_program_item_base,
			idf_program,
			schema.idf_style_id,
		};
		idc_program_item_base = schema.put_class(pcr);

		pcr.class_name = "program_feature1";
		pcr.class_description = "PROGRAM FEATURE 1";
		pcr.field_id_primary_key = idf_program_item_base;
		pcr.base_class_id = idc_program_item_base;
		pcr.member_fields = {
			idf_program_item_base,
			idf_program,
			idf_feature1_text,
			schema.idf_style_id,
		};
		idc_program_feature1 = schema.put_class(pcr);

		pcr.class_name = "program_feature2";
		pcr.class_description = "PROGRAM FEATURE 2";
		pcr.field_id_primary_key = idf_program_item_base;
		pcr.base_class_id = idc_program_item_base;
		pcr.member_fields = {
			idf_program_item_base,
			idf_program,
			idf_feature2_text,
			schema.idf_style_id,
		};
		idc_program_feature2 = schema.put_class(pcr);

		pcr.class_name = "product";
		pcr.class_description = "PRODUCT";
		pcr.field_id_primary_key = idf_product;
		pcr.member_fields = {
			idf_product,
			idf_program,
			schema.idf_style_id,
			idf_attachment,
			idf_limit,
			idf_share,
			idf_deductible,
			idf_comment
		};
		idc_product = schema.put_class(pcr);

		pcr.class_name = "product_item_base";
		pcr.class_description = "PRODUCT ITEM";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.member_fields = {
			idf_product_item_base,
			idf_product,
		};
		idc_product_item_base = schema.put_class(pcr);

		pcr.class_name = "product_feature1";
		pcr.class_description = "PRODUCT FEATURE 1";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = {
			idf_program_item_base,
			idf_product,
			idf_feature1_text,
			schema.idf_style_id,
		};
		idc_product_feature1 = schema.put_class(pcr);

		pcr.class_name = "product_feature2";
		pcr.class_description = "PRODUCT FEATURE 2";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = {
			idf_program_item_base,
			idf_program,
			idf_feature2_text,
			schema.idf_style_id,
		};
		idc_product_feature2 = schema.put_class(pcr);

		pcr.class_name = "program_chart_slide";
		pcr.class_description = "PROGRAM SLIDE";
		pcr.field_id_primary_key = idf_program_chart_slide;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_slide_title };
		idc_program_chart_slide = schema.put_class(pcr);
		 
		pcr.class_name = "program_chart_slide_product";
		pcr.class_description = "SLIDE PRODUCT";
		pcr.field_id_primary_key = idf_program_chart_slide_product;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_product };
		idc_program_chart_slide_product = schema.put_class(pcr);

		pcr.class_name = "program_generic_slide";
		pcr.class_description = "GENERIC SLIDE";
		pcr.field_id_primary_key = idf_program_chart_slide;
		pcr.member_fields = { idf_program_generic_slide, idf_client, idf_program };
		idc_program_generic_slide = schema.put_class(pcr);

		jmodel jm;

		jm.update_always(&schema, idc_home);
		jm.update_always(&schema, idc_carrier_root);
		jm.update_always(&schema, idc_carrier);
		jm.update_always(&schema, idc_program_template_root);
		jm.update_always(&schema, idc_program_template);
		jm.update_always(&schema, idc_coverage_root);
		jm.update_always(&schema, idc_coverage);
		jm.update_always(&schema, idc_client_root);
		jm.update_always(&schema, idc_client);
		jm.update_always(&schema, idc_program);
		jm.update_always(&schema, idc_program_chart_slide);
		jm.update_always(&schema, idc_program_generic_slide);
		jm.update_always(&schema, idc_program_item_base);
		jm.update_always(&schema, idc_product);
		jm.update_always(&schema, idc_product_item_base);

		jm.select_always(&schema, idc_home);
		jm.select_always(&schema, idc_carrier_root);
		jm.select_always(&schema, idc_carrier);
		jm.select_always(&schema, idc_program_template_root);
		jm.select_always(&schema, idc_program_template);
		jm.select_always(&schema, idc_coverage_root);
		jm.select_always(&schema, idc_coverage);
		jm.select_always(&schema, idc_client_root);
		jm.select_always(&schema, idc_client);
		jm.select_always(&schema, idc_system_root);
		jm.update_always(&schema, idc_program);
		jm.update_always(&schema, idc_program_chart_slide);
		jm.update_always(&schema, idc_program_generic_slide);
		jm.update_always(&schema, idc_program_item_base);
		jm.update_always(&schema, idc_product);
		jm.update_always(&schema, idc_product_item_base);

		jm.delete_always(&schema, idc_carrier);
		jm.delete_always(&schema, idc_program_template);
		jm.delete_always(&schema, idc_coverage);
		jm.delete_always(&schema, idc_client);
		jm.update_always(&schema, idc_program);
		jm.update_always(&schema, idc_program_chart_slide);
		jm.update_always(&schema, idc_program_generic_slide);
		jm.update_always(&schema, idc_program_item_base);
		jm.update_always(&schema, idc_product);
		jm.update_always(&schema, idc_product_item_base);

		jm.create_when(&schema, idc_carrier_root, idc_carrier, null_row, true, false);
		jm.create_when(&schema, idc_program_template_root, idc_program_template, null_row, true, false);
		jm.create_when(&schema, idc_coverage_root, idc_coverage, null_row, true, false);
		jm.create_when(&schema, idc_client_root, idc_client, null_row, true, false);
		jm.create_when(&schema, idc_client, idc_program, null_row, true, false);
		jm.create_when(&schema, idc_program, idc_product, null_row, true, false);
		jm.create_when(&schema, idc_program, idc_program_chart_slide, idc_program, true, false);
		jm.create_when(&schema, idc_program_chart_slide, idc_product, idc_program_chart_slide_product, idc_product, true, false);
		jm.create_when(&schema, idc_program, idc_program_generic_slide, idc_program, true, false);
		jm.create_when(&schema, idc_program, idc_program_feature1, null_row, true, false);
		jm.create_when(&schema, idc_program, idc_program_feature2, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_product_feature1, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_product_feature2, null_row, true, false);

		jm.navigation({
						{ idc_home, 0},
						{ idc_carrier_root, 1 },
						{ idc_coverage_root, 1 },
						{ idc_program_template_root, 1 },
						{ idc_client_root, 1 },
						{ idc_system_root, 1 },
						{ idc_client, 2 },
						{ idc_carrier, 2 },
						{ idc_coverage, 2 },
						{ idc_program, 3 },
						{ idc_program_item_base, 4 },
						{ idc_program_chart_slide, 4 },
						{ idc_program_generic_slide, 4 },
						{ idc_product, 4 },
						{ idc_product_item_base, 5 },
						{ idc_product_item_base, 5 }
					}
			);

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

		program_chart = schema.create_collection(&ref);

		jactor sample_actor;
		sample_actor.actor_name = "sample actor";
		sample_actor.actor_id = null_row;
		sample_actor.current_view_class_id = idc_home;
		sample_actor = program_chart.create_actor(sample_actor);

		actor_id = sample_actor.actor_id;

		set_style_sheet();

		program_chart.create_object(null_row, sample_actor.actor_id, idc_home, id_home, { { schema.idf_style_id, schema.idf_home_style } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_carrier_root, id_carrier_root, { { idf_home , id_home }, { schema.idf_style_id, schema.idf_carrier_style } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_coverage_root, id_coverage_root, { { idf_home, id_home }, { schema.idf_style_id, schema.idf_coverage_style } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_client_root, id_client_root, { { idf_home, id_home }, { schema.idf_style_id, schema.idf_client_style } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_program_template_root, id_product_template_root, { { idf_home, id_home }, { schema.idf_style_id, schema.idf_product_style } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_system_root, id_system_root, { { idf_home, id_home }, { schema.idf_style_id, schema.idf_system_style} });
		
		// get our initial state
		state = program_chart.get_actor_state(sample_actor.actor_id);

		// then create our select request
		auto initial_selection = state.create_select_request(id_home, false);

		// now we can select our initial object!
		state = program_chart.select_object(initial_selection);

		// when the controller creates the window then, the state will be queried again and the application will be on the same
		// page

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
		auto navigation_contents = canvas2d_column(id_canvas_navigation, main_row, null_row, { 0.0_px, 0.0_px, 200.0_px, 100.0_pct });
		breadcrumbs(navigation_contents, [this](jobject& _item) {
			return _item.get_name(schema.idf_name);
			}, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });

		relative_ptr_type navigation_objects[5] = {idc_client_root, idc_program_template_root, idc_carrier_root, idc_coverage_root, idc_system_root};
		selects(navigation_contents, schema.idf_company_neutral1_style, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px }, schema.idf_name, navigation_objects, 5);

		auto form_contents = column(main_row, null_row, { 16.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		_contents(navigation_contents, form_contents);

		auto footer_bar = canvas2d_row(id_canvas_footer, page_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
		text(footer_bar, schema.idf_artist_title_style, _subtitle);
	}

	void wsproposal_controller::render_form(page_item* _navigation, page_item *_frame, const char *_form_title)
	{
		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		_frame->windowsRegion = true;
		add_update_fields(_frame, field_layout::label_on_left, _form_title);
		space(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		text(_navigation, schema.idf_label_style, "Create", {0.0_px, 0.0_px, 100.0_pct, 32.0_px});
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wsproposal_controller::render_search_page(page_item* _navigation, page_item* _contents, relative_ptr_type _class_id, const char* _form_title, int count_fields, relative_ptr_type *_field_ids)
	{
		auto form_search = row(_contents, null_row, { 0.0_px, 0.0_px, 100.0_pct, 25.0_px });
		add_update_fields(form_search, field_layout::label_on_left, _form_title);
		auto form_table = canvas2d_column(id_canvas_form_table_a, _contents, schema.idf_view_background_style);
		search_table(form_table, _class_id, _field_ids, count_fields);
		text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
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

	void wsproposal_controller::render_product_template_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_product_template_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_product_template()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_product_template_contents(_navigation, _contents);  });
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
	}

	void wsproposal_controller::render_home_contents(page_item* _navigation, page_item* _contents)
	{
	}

	void wsproposal_controller::render_client_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		render_search_page(_navigation, _contents, idc_client, "Clients", 5, field_ids);
	}

	void wsproposal_controller::render_client_contents(page_item* _navigation, page_item* _contents)
	{
		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		auto edit_body = row(_contents, null_row, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		auto control = column(edit_body, schema.idf_view_background_style, { 0.0_pct, 0.0_px, 30.0_pct, 100.0_pct });
		auto children = canvas2d_column(id_canvas_form_table_a, edit_body, schema.idf_view_background_style, { 0.0_px, 0.0_px, 65.0_pct, 100.0_pct });

		add_update_fields(control, field_layout::label_on_top, "Client Details");
		space(control, schema.idf_button_style, { 0.0_px, 0.0_px, 1.0_fntgr, 1.0_fntgr });

		text(children, schema.idf_view_subtitle_style, "Programs", { 0.0_px, 0.0_px, 100.0_pct, 1.4_fntgr });
		relative_ptr_type field_ids[1] = { idf_program_title };
		search_table(children, idc_program, field_ids, 1);

		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wsproposal_controller::render_coverage_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[1] = { schema.idf_name };
		render_search_page(_navigation, _contents, idc_coverage, "Coverages", 1, field_ids);
	}

	void wsproposal_controller::render_system_root_contents(page_item* _navigation, page_item* _contents)
	{

		}

	void wsproposal_controller::render_coverage_contents(page_item* _navigation, page_item* _contents)
	{
		render_form(_navigation, _contents, "Coverage");
	}

	void wsproposal_controller::render_product_template_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[1] = { schema.idf_name };
		render_search_page(_navigation, _contents, idc_program_template, "Products", 1, field_ids);
	}

	void wsproposal_controller::render_product_template_contents(page_item* _navigation, page_item* _contents)
	{
		render_form(_navigation, _contents, "Product");
	}

	void wsproposal_controller::render_carrier_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		render_search_page(_navigation, _contents, idc_carrier, "Carriers", 5, field_ids);
	}

	void wsproposal_controller::render_carrier_contents(page_item* _navigation, page_item* _contents)
	{
		render_form(_navigation, _contents, "Carrier");
	}

	void wsproposal_controller::render_program_contents(page_item* _navigation, page_item* _contents)
	{
		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		auto content_area = row(_contents, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		auto detail_area = column(content_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 40.0_pct });
		auto program_edit_area = row(detail_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 200.0_px });
		auto program_table_area = canvas2d_row(id_canvas_form_table_a, detail_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		// editable controls on the left
		add_update_fields(program_edit_area, field_layout::label_on_top, "Program Details");
		space(program_edit_area, schema.idf_button_style, { 0.0_px, 0.0_px, 1.0_fntgr, 1.0_fntgr });
		// and the add buttons
		add_create_buttons(program_edit_area, schema.idf_button_style);

		text(program_table_area, schema.idf_view_subtitle_style, "Program items", { 0.0_px, 0.0_px, 100.0_pct, 1.4_fntgr });
		relative_ptr_type field_ids[1] = { idf_program_title };
		search_table(program_table_area, idc_program, field_ids, 1);

/*

		page_item* contents = row( _frame, schema.idf_view_background_style);
		page_item* slide_area = column(contents, schema.idf_view_background_style, { 0.0_px, 0.0_px, 25.0_pct, 100.0_pct });
		page_item* selected_slide_area = column(contents, schema.idf_view_background_style, { 0.0_px, 0.0_px, 25.0_pct, 100.0_pct });

		layout_rect slide_layout = { 0.0_px, 0.0_px, 100.0_pct, 75.0_pcw }, *pslide_layout = &slide_layout;

		relative_ptr_type slide_types[2] = { idc_program_chart_slide, idc_program_generic_slide };
		for_class(slide_types, 2, [this, pslide_layout, slide_area, selected_slide_area](actor_view_object& slide_avo)
			{
				render_program_slide(slide_area, slide_avo, pslide_layout);
				if (slide_avo.selected) 
				{
					render_program_slide( selected_slide_area, slide_avo, pslide_layout );
				}
				return true;
			}
		); */
	}

	void wsproposal_controller::render_program_slide(page_item* _frame, actor_view_object& _slide, layout_rect *_layout)
	{
		if (_slide.class_id == idc_program_chart_slide)
		{
			render_program_chart_slide(_frame, _slide, _layout);
		}
		else if (_slide.class_id == idc_program_generic_slide)
		{
			render_program_generic_slide(_frame, _slide, _layout);
		}
	}

	void wsproposal_controller::render_program_chart_slide(page_item* _frame, actor_view_object& _slide, layout_rect* _layout)
	{
		text(_frame, schema.idc_text_style, "Program Chart", *_layout);
	}

	void wsproposal_controller::render_program_generic_slide(page_item* _frame, actor_view_object& _slide, layout_rect* _layout)
	{
		text(_frame, schema.idc_text_style, "Generic Slide", *_layout);
	}

	void wsproposal_controller::render(const rectangle& newSize)
	{
		if (state.actor.current_view_class_id == idc_home) 
		{
			render_home();
		} 
		else if (state.actor.current_view_class_id == idc_client_root)
		{
			render_client_root();
		}
		else if (state.actor.current_view_class_id == idc_client || state.actor.current_view_class_id == idc_program)
		{
			render_client();
		}
		else if (state.actor.current_view_class_id == idc_program_template_root)
		{
			render_product_template_root();
		}
		else if (state.actor.current_view_class_id == idc_program_template)
		{
			render_product_template();
		}
		else if (state.actor.current_view_class_id == idc_coverage_root)
		{
			render_coverage_root();
		}
		else if (state.actor.current_view_class_id == idc_coverage)
		{
			render_coverage();
		}
		else if (state.actor.current_view_class_id == idc_program_template)
		{
			render_product_template();
		}
		else if (state.actor.current_view_class_id == idc_program || 
			state.actor.current_view_class_id == idc_product || 
			program_chart.matches_class_id( state.actor.current_view_object_id, idc_program_item_base ) || 
			program_chart.matches_class_id(state.actor.current_view_object_id, idc_product_item_base) )
		{
			render_program();
		}
		else if (state.actor.current_view_class_id == idc_carrier_root)
		{
			render_carrier_root();
		}
		else if (state.actor.current_view_class_id == idc_carrier)
		{
			render_carrier();
		}
		else if (state.actor.current_view_class_id == idc_system_root)
		{
			render_system_root();
		}

		auto style_sheet = getStyleSheet();
		arrange(newSize.w, newSize.h, style_sheet, 16);
		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsproposal_controller::set_style_sheet()
	{
		auto style_sheet = getStyleSheet();

		const char* fontName = "Open Sans,Arial";

		style_sheet.set(
			{ schema.idf_view_background_style },
			{
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#ffffffff" }
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
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#ffffffff" }
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

		auto color_check_src = style_sheet.get_slice(schema.idf_view_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);
		auto color_check_dest = style_sheet.get_slice(schema.idf_view_subtitle_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);

		style_sheet.set(
			schema.idf_view_style, 
			{ schema.idf_view_section_style },
			{
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_box_fill_color, "#DEE1E1FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#C1C6C8FF" }
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
			schema.idf_view_style,
			{ schema.idf_column_text_head_style },
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
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#1F2A44FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#1F2A44FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_number_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
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
			{ schema.idf_column_text_style },
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
			{ schema.idf_client_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_carrier_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_coverage_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_product_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_system_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_home_style },
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
			{ schema.idf_login_style },
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
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
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
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
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
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
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

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 40.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 4 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
				{ schema.idf_shape_fill_color, "#001100FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title1_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title2_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title3_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title4_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title5_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title6_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

	}

}
