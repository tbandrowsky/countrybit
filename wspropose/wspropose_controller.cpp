
#pragma once


#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

namespace proposal
{

	using namespace corona::database;

	wsproposal_controller::wsproposal_controller() : corona_controller()
	{
		box.init(1 << 22);
		schema = jschema::create_schema(&box, 50, true, schema_id);

		put_class_request pcr;
		put_object_field_request porf;

		idf_home = schema.put_integer_field({ { null_row,  jtype::type_int64, "home_id", "Home Id" }, { 0, INT64_MAX } });
		idf_client_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "client_root", "Clients" }, { 0, INT64_MAX } });
		idf_client = schema.put_integer_field({ { null_row,  jtype::type_int64, "client_id", "Client Id" }, { 0, INT64_MAX } });
		idf_carrier_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "carrier_root", "Carriers" }, { 0, INT64_MAX } });
		idf_carrier = schema.put_integer_field({ { null_row,  jtype::type_int64, "carrier_id", "Carrier Id" }, { 0, INT64_MAX } });
		idf_program_template_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "home", "Home id" }, { 0, INT64_MAX } });
		idf_program_template = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_template", "Product Template id" }, { 0, INT64_MAX } });
		idf_coverage_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "coverage_root", "Coverages" }, { 0, INT64_MAX } });
		idf_coverage = schema.put_integer_field({ { null_row,  jtype::type_int64, "coverage_id", "Coverage Id" }, { 0, INT64_MAX } });
		idf_system_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "system_root", "System Settings" }, { 0, INT64_MAX } });

		idf_program = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_id", "Program Id" }, { 0, INT64_MAX } });
		idf_program_view = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_view", "Program View" }, { 0, INT64_MAX } });
		idf_program_title = schema.put_string_field({ { null_row,  jtype::type_string, "program_title", "Program Title" }, { 100, "", "" } });
		idf_program_subtitle = schema.put_string_field({ { null_row,  jtype::type_string, "program_title", "Program Subtitle" }, { 100, "", "" } });
		idf_product = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_id", "Product Id" }, { 0, INT64_MAX } });
		idf_product_item_base = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_item_id", "Product Item Id" }, { 0, INT64_MAX } });
		idf_program_chart_slide = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_chart_slide_id", "Program Chart Slide Id" }, { 0, INT64_MAX } });
		idf_program_chart_slide_product = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_chart_slide_product_id", "Program Chart Slide Item Id" }, { 0, INT64_MAX } });
		idf_program_generic_slide = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_generic_slide", "Generic Slide Id" }, { 0, INT64_MAX } });

		idf_inception = schema.put_time_field({ { null_row,  jtype::type_datetime, "inception", "Inception" }, { 0, INT64_MAX } });
		idf_expiration = schema.put_time_field({ { null_row,  jtype::type_datetime, "expiration", "Expiration" }, { 0, INT64_MAX } });
		idf_status = schema.put_integer_field({ { null_row,  jtype::type_int32, "status", "Status" }, { 0, INT64_MAX } });
		idf_attachment = schema.put_double_field({ { null_row,  jtype::type_float32, "attachment", "Attachment" }, { 0.0, 1E10 } });
		idf_limit = schema.put_double_field({ { null_row,  jtype::type_float32, "limit", "Limit" }, { 0.0, 1E10 } });
		idf_deductible = schema.put_double_field({ { null_row,  jtype::type_float32, "deductible", "Deductible" }, { 0.0, 1E10 } });
		idf_share = schema.put_double_field({ { null_row,  jtype::type_float32, "share", "Share %" }, { 0.0, 100 } });
		idf_comment = schema.put_string_field({ { null_row,  jtype::type_string, "comment", "Comment" }, { 500, "", "", } });
		idf_program_style = schema.put_string_field({ { null_row,  jtype::type_color, "style", "Style" }, {  } });

		idf_pi_inception = schema.put_integer_field({ { null_row,  jtype::type_int64, "inception_id", "Inception Id" }, { 0, INT64_MAX } });
		idf_pi_expiration = schema.put_integer_field({ { null_row,  jtype::type_int64, "expiration_id", "Expiration Id" }, { 0, INT64_MAX } });
		idf_pi_status = schema.put_integer_field({ { null_row,  jtype::type_int64, "status_id", "Status Id" }, { 0, INT64_MAX } });
		idf_pi_attachment = schema.put_integer_field({ { null_row,  jtype::type_int64, "attachment_id", "Attachment Id" }, { 0, INT64_MAX } });
		idf_pi_limit = schema.put_integer_field({ { null_row,  jtype::type_int64, "limit_id", "Limit Id" }, { 0, INT64_MAX } });
		idf_pi_deductible = schema.put_integer_field({ { null_row,  jtype::type_int64, "deductible_id", "Deductible Id" }, { 0, INT64_MAX } });
		idf_pi_share = schema.put_integer_field({ { null_row,  jtype::type_int64, "share_id", "Share % Id" }, { 0, INT64_MAX } });
		idf_pi_comment = schema.put_integer_field({ { null_row,  jtype::type_int64, "comment_id", "Comment Id" }, { 0, INT64_MAX } });

		pcr.class_name = "home";
		pcr.class_description = "Application Home";
		pcr.field_id_primary_key = idf_home;
		pcr.member_fields = { idf_home };
		idc_home = schema.put_class(pcr);

		pcr.class_name = "client_root";
		pcr.class_description = "Clients";
		pcr.field_id_primary_key = idf_client_root;
		pcr.member_fields = { idf_client_root, idf_home, schema.idf_layout_rect, schema.idf_search_string };
		idc_home = schema.put_class(pcr);

		pcr.class_name = "client";
		pcr.class_description = "Client";
		pcr.field_id_primary_key = idf_client;
		pcr.member_fields = { idf_client, idf_client_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_client = schema.put_class(pcr);

		pcr.class_name = "carrier_root";
		pcr.class_description = "Carriers";
		pcr.field_id_primary_key = idf_carrier_root;
		pcr.member_fields = { idf_carrier_root, idf_home, schema.idf_layout_rect, schema.idf_search_string };
		idc_carrier_root = schema.put_class(pcr);

		pcr.class_name = "carrier";
		pcr.class_description = "Carrier";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_carrier, idf_carrier_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_carrier = schema.put_class(pcr);

		pcr.class_name = "coverage_root";
		pcr.class_description = "Coverages";
		pcr.field_id_primary_key = idf_coverage_root;
		pcr.member_fields = { idf_coverage_root, idf_home, schema.idf_layout_rect, schema.idf_search_string };
		idc_coverage_root = schema.put_class(pcr);

		pcr.class_name = "coverage";
		pcr.class_description = "Coverage";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_coverage, idf_coverage_root, schema.idf_name };
		idc_coverage = schema.put_class(pcr);

		pcr.class_name = "system_root";
		pcr.class_description = "System Settings";
		pcr.field_id_primary_key = idf_system_root;
		pcr.member_fields = { idf_system_root, idf_home, schema.idf_layout_rect, schema.idf_search_string };
		idc_system_root = schema.put_class(pcr);

		pcr.class_name = "program_templates";
		pcr.class_description = "Program Templates";
		pcr.field_id_primary_key = idf_program_template_root;
		pcr.member_fields = { idf_program_template_root, idf_home, schema.idf_layout_rect, schema.idf_search_string };
		idc_program_template_root = schema.put_class(pcr);

		pcr.class_name = "program_template";
		pcr.class_description = "Program Template";
		pcr.field_id_primary_key = idf_program_template;
		pcr.member_fields = { idf_program_template_root, idf_program_template, idf_home, schema.idf_layout_rect };
		idc_program_template_root = schema.put_class(pcr);

		pcr.class_name = "program";
		pcr.class_description = "Program";
		pcr.field_id_primary_key = idf_program;
		pcr.member_fields = { idf_program, idf_client, idf_program_view, schema.idf_name, idf_program_title, idf_program_subtitle };
		idc_coverage = schema.put_class(pcr);

		pcr.class_name = "product";
		pcr.class_description = "Product";
		pcr.field_id_primary_key = idf_product;
		pcr.member_fields = { idf_product, idf_program, idf_client, idf_program_style, schema.idf_layout_rect };
		idc_product = schema.put_class(pcr);

		pcr.class_name = "product_item_base";
		pcr.class_description = "Product Item Base";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.member_fields = { idf_product, idf_product_item_base };
		idc_product_item_base = schema.put_class(pcr);

		pcr.class_name = "pi_inception";
		pcr.class_description = "Inception";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_inception };
		idc_pi_inception = schema.put_class(pcr);

		pcr.class_name = "pi_expiration";
		pcr.class_description = "Expiration";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_expiration };
		idc_pi_expiration = schema.put_class(pcr);

		pcr.class_name = "pi_status";
		pcr.class_description = "Status";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_status };
		idc_pi_status = schema.put_class(pcr);

		pcr.class_name = "pi_limit";
		pcr.class_description = "Limit";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_limit };
		idc_pi_limit = schema.put_class(pcr);

		pcr.class_name = "pi_deductible";
		pcr.class_description = "Deductible";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_deductible };
		idc_pi_deductible = schema.put_class(pcr);

		pcr.class_name = "pi_share";
		pcr.class_description = "Share";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_deductible };
		idc_pi_share = schema.put_class(pcr);

		pcr.class_name = "pi_comment";
		pcr.class_description = "Comment";
		pcr.field_id_primary_key = idf_product_item_base;
		pcr.base_class_id = idc_product_item_base;
		pcr.member_fields = { idf_comment };
		idc_pi_comment = schema.put_class(pcr);

		pcr.class_name = "program_chart_slide";
		pcr.class_description = "Program Chart Slide";
		pcr.field_id_primary_key = idf_program_chart_slide;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_slide_title };
		idc_program_chart_slide = schema.put_class(pcr);

		pcr.class_name = "program_chart_slide_product";
		pcr.class_description = "Program Chart Slide Product";
		pcr.field_id_primary_key = idf_program_chart_slide_product;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_product };
		idc_program_chart_slide_product = schema.put_class(pcr);

		pcr.class_name = "program_generic_slide";
		pcr.class_description = "Program Generic Slide";
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
		jm.update_always(&schema, idc_product);
		jm.update_always(&schema, idc_product_item_base);
		jm.update_always(&schema, idc_program_chart_slide);
		jm.update_always(&schema, idc_program_generic_slide);

		jm.select_always(&schema, idc_home);
		jm.select_always(&schema, idc_carrier_root);
		jm.select_always(&schema, idc_carrier);
		jm.select_always(&schema, idc_program_template_root);
		jm.select_always(&schema, idc_program_template);
		jm.select_always(&schema, idc_coverage_root);
		jm.select_always(&schema, idc_coverage);
		jm.select_always(&schema, idc_client_root);
		jm.select_always(&schema, idc_client);
		jm.select_always(&schema, idc_program);
		jm.select_always(&schema, idc_product);
		jm.select_always(&schema, idc_program_chart_slide);
		jm.select_always(&schema, idc_program_generic_slide);
		jm.select_always(&schema, idc_product_item_base);

		jm.delete_always(&schema, idc_carrier);
		jm.delete_always(&schema, idc_program_template);
		jm.delete_always(&schema, idc_coverage);
		jm.delete_always(&schema, idc_client);
		jm.delete_always(&schema, idc_program);
		jm.delete_always(&schema, idc_product);
		jm.delete_always(&schema, idc_program_chart_slide);
		jm.delete_always(&schema, idc_program_generic_slide);
		jm.delete_always(&schema, idc_product_item_base);

		jm.create_when(&schema, idc_carrier_root, idc_carrier, null_row, true, false);
		jm.create_when(&schema, idc_program_template_root, idc_program_template, null_row, true, false);
		jm.create_when(&schema, idc_coverage_root, idc_coverage, null_row, true, false);
		jm.create_when(&schema, idc_client_root, idc_client, null_row, true, false);
		jm.create_when(&schema, idc_client, idc_program, null_row, true, false);
		jm.create_when(&schema, idc_program, idc_product, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_inception, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_expiration, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_status, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_attachment, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_limit, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_deductible, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_share, null_row, true, false);
		jm.create_when(&schema, idc_product, idc_pi_comment, null_row, true, false);
		jm.create_when(&schema, idc_program, idc_program_chart_slide, idc_program, true, false);
		jm.create_when(&schema, idc_program_chart_slide, idc_program_chart_slide_product, idc_program, true, false);
		jm.create_when(&schema, idc_program, idc_program_generic_slide, idc_program, true, false);

		jm.navigation({
						{ idc_home, 0},
						{ idc_carrier_root, 1 },
						{ idc_coverage_root, 1 },
						{ idc_program_template_root, 1 },
						{ idc_client_root, 1 },
						{ idc_client, 2 },
						{ idc_carrier, 2 },
						{ idc_coverage, 2 },
						{ idc_program, 3 },
						{ idc_product, 4 },
						{ idc_program_chart_slide, 4 },
						{ idc_program_generic_slide, 4 } }
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

		relative_ptr_type id_home = null_row, id_carrier_root = null_row, id_coverage_root = null_row, id_client_root = null_row, id_product_template_root = null_row, id_system_root = null_row;

		program_chart.create_object(null_row, sample_actor.actor_id, idc_home, id_home);
		program_chart.create_object(null_row, sample_actor.actor_id, idc_carrier_root, id_carrier_root, { { idf_home , id_home }});
		program_chart.create_object(null_row, sample_actor.actor_id, idc_coverage_root, id_coverage_root, { { idf_home, id_home } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_client_root, id_client_root, { { idf_home, id_home } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_program_template_root, id_product_template_root, { { idf_home, id_home } });
		program_chart.create_object(null_row, sample_actor.actor_id, idc_system_root, id_system_root, { { idf_home, id_home } });

		relative_ptr_type style_sheet_id = null_row;

		relative_ptr_type homes_id = null_row;
		relative_ptr_type clients_id = null_row;
		relative_ptr_type carriers_id = null_row;
		relative_ptr_type programs_id = null_row;

		program_chart.create_object(0, sample_actor.actor_id, idc_home, homes_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_client_root, clients_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, carriers_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, programs_id);

		create_style_sheet();

		map_style(idc_home, schema.idf_home_style);
		map_style(idc_client_root, schema.idf_client_style);
		map_style(idc_carrier_root, schema.idf_carrier_style);
		map_style(idc_coverage_root, schema.idf_coverage_style);
		map_style(idc_program_template_root, schema.idf_product_style);
		map_style(idc_system_root, schema.idf_system_style);

	}

	wsproposal_controller::~wsproposal_controller()
	{
		;
	}

	void wsproposal_controller::render_header(page_item* _frame, const char* _title, const char* _subtitle, bool _left_pad)
	{
		auto title_block = row(_frame, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 150.0_px });

		page_item* header_area;

		if (_left_pad) 
		{
			space(_frame, schema.idf_view_background_style, { 0.0_px, 0.0_px, 25.0_pct, 100.0_pct });
			header_area = column(title_block, schema.idf_view_background_style, { 0.0_px, 0.0_px, 75.0_pct, 100.0_pct });
		}
		else 
		{
			header_area = column(title_block, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		}

		auto title_bar = row(header_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 50.0_px });
		auto subtitle_bar = row(header_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 50.0_px });
		auto breadcrumb_bar = row(header_area, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 50.0_px });

		text(title_bar, schema.idf_view_title_style, _title, { 0.0_px, 0.0_px, 150.0_px, 100.0_pct });
		text(subtitle_bar, schema.idf_view_subtitle_style, _subtitle, { 0.0_px, 0.0_px, 150.0_px, 100.0_pct });
		breadcrumbs(breadcrumb_bar, [](jobject& _item) {
			return nullptr;
			}, { 0.0_px, 0.0_px, 100.0_px, 100.0_pct });
	}

	void wsproposal_controller::render_form(std::function<void(page_item* _frame)> _contents)
	{
		clear();

		auto mainr = row(nullptr, null_row);

		auto d2drow = column(mainr, null_row, { 0.0_px,0.0_px,100.0_pct,100.0_pct });
		auto controlrow = row(mainr, null_row, { 0.0_px,0.0_px,100.0_pct,100.0_pct });
		auto controlcolumn = column(mainr, null_row, { 20.0_pct,0.0_px,60.0_pct,100.0_pct });

		auto d2dwin = canvas2d(d2drow, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		auto d2dwin_area = column(d2dwin, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		render_header(d2dwin_area, application_title, object_title, true);

		// editable controls on the left
		add_update_fields(controlcolumn);
		add_create_buttons(controlcolumn);

		_contents(d2dwin_area);
	}


	void wsproposal_controller::render_search(std::function<void(page_item* _frame)> _contents)
	{
		clear();

		auto mainr = row(nullptr, null_row);
		auto d2drow = column(mainr, null_row, { 0.0_px,0.0_px,100.0_pct,25.0_pct });
		auto controlrow = column(mainr, null_row, { 0.0_px,0.0_px,100.0_pct,75.0_pct });
		auto d2dwin = canvas2d(d2drow, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		auto d2dwin_area = column(d2dwin, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		render_header(d2dwin, application_title, object_title, false);

		auto client_area = row(d2dwin, schema.idf_view_background_style, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		_contents(client_area);

		// editable controls on the left
		add_update_fields(controlrow);
		add_create_buttons(controlrow);

	}

	void wsproposal_controller::render_visual(std::function<void(page_item* _frame)> _contents)
	{
		clear();

		auto mainr = row(nullptr, null_row);
		auto controlcolumn = column(mainr, null_row, { 0.0_px,0.0_px,25.0_pct,100.0_pct });
		auto d2dcolumn = column(mainr, null_row, { 0.0_px,0.0_px,75.0_pct,100.0_pct });
		auto d2dwin = canvas2d(d2dcolumn, schema.idf_view_background_style, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		object_description object_title;
		object_title = schema.get_class(state.actor.current_view_class_id).item().description;

		render_header(d2dwin, application_title, object_title, false);

		// editable controls on the left
		add_update_fields(controlcolumn);
		add_create_buttons(controlcolumn);

		_contents(d2dwin);
	}

	void wsproposal_controller::render_home()
	{
		render_visual([this](page_item* _frame) { render_home_contents(_frame);  });
	}

	void wsproposal_controller::render_client_root()
	{
		render_visual([this](page_item* _frame) { render_client_root_contents(_frame);  });
	}

	void wsproposal_controller::render_client()
	{
		render_form([this](page_item* _frame) { render_client_contents(_frame);  });
	}

	void wsproposal_controller::render_coverage_root()
	{
		render_visual([this](page_item* _frame) { render_coverage_root_contents(_frame);  });
	}

	void wsproposal_controller::render_coverage()
	{
		render_form([this](page_item* _frame) { render_coverage_contents(_frame);  });
	}

	void wsproposal_controller::render_product_template_root()
	{
		render_visual([this](page_item* _frame) { render_product_template_root_contents(_frame);  });
	}

	void wsproposal_controller::render_product_template()
	{
		render_visual([this](page_item* _frame) { render_product_template_contents(_frame);  });
	}

	void wsproposal_controller::render_carrier_root()
	{
		render_visual([this](page_item* _frame) { render_carrier_root_contents(_frame);  });
	}

	void wsproposal_controller::render_carrier()
	{
		render_form([this](page_item* _frame) { render_carrier_contents(_frame);  });
	}

	void wsproposal_controller::render_program()
	{
		render_form([this](page_item* _frame) { render_program_contents(_frame);  });
	}

	void wsproposal_controller::render_home_contents(page_item* _frame)
	{
		relative_ptr_type class_ids1[3] = { idc_client_root, idc_carrier_root, idc_coverage_root };
		relative_ptr_type class_ids2[2] = { idc_program_template_root, idc_system_root };

		column_class(_frame, { 0.0_px, 0.0_px, 50.0_pct, 100.0_pct }, class_ids1, 3);
		column_class(_frame, { 0.0_px, 0.0_px, 50.0_pct, 100.0_pct }, class_ids2, 2);
	}

	void wsproposal_controller::render_client_root_contents(page_item* _frame)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		search_table(_frame, idc_client, field_ids, 5);
	}

	void wsproposal_controller::render_client_contents(page_item* _frame)
	{
		;
	}

	void wsproposal_controller::render_coverage_root_contents(page_item* _frame)
	{
		relative_ptr_type field_ids[1] = { schema.idf_name };
		search_table(_frame, idc_client, field_ids, 1);
	}

	void wsproposal_controller::render_coverage_contents(page_item* _frame)
	{

	}

	void wsproposal_controller::render_product_template_root_contents(page_item* _frame)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		search_table(_frame, idc_carrier, field_ids, 5);
	}

	void wsproposal_controller::render_product_template_contents(page_item* _frame)
	{
		;
	}

	void wsproposal_controller::render_carrier_root_contents(page_item* _frame)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		search_table(_frame, idc_carrier, field_ids, 5);
	}

	void wsproposal_controller::render_carrier_contents(page_item* _frame)
	{
		;
	}

	void wsproposal_controller::render_program_contents(page_item* _frame)
	{
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
		);
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
		else if (state.actor.current_view_class_id == idc_client)
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
		else if (state.actor.current_view_class_id == idc_carrier_root)
		{
			render_carrier_root();
		}
		else if (state.actor.current_view_class_id == idc_carrier)
		{
			render_carrier();
		}
		else if (state.actor.current_view_class_id == idc_program)
		{
			render_program();
		}

		arrange(newSize.w, newSize.h);
		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsproposal_controller::create_style_sheet()
	{
		auto style_sheet = getStyleSheet();

		const char* fontName = "Open Sans,Arial";

		style_sheet.set(
			{ schema.idf_view_style },
			{
				{ schema.idf_name, "view_background" },
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
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#ffffffff" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_title_style },
			{
				{ schema.idf_name, "view_title" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_subtitle_style },
			{
				{ schema.idf_name, "view_subtitle" },
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 24.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_section_style },
			{
				{ schema.idf_name, "view_section" },
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_style },
			{
				{ schema.idf_name, "view" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 16.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_disclaimer_style },
			{
				{ schema.idf_name, "disclaimer" },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_copyright_style },
			{
				{ schema.idf_name, "copyright" },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h1_style },
			{
				{ schema.idf_name, "h1" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h2_style },
			{
				{ schema.idf_name, "h2" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h3_style },
			{
				{ schema.idf_name, "h3" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 20.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_number_head_style },
			{
				{ schema.idf_name, "column_number_head" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, true },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#efefefFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_text_head_style },
			{
				{ schema.idf_name, "column_text_head" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, true },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#efefefFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_number_style },
			{
				{ schema.idf_name, "column_number" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
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
			{ schema.idf_column_text_style },
			{
				{ schema.idf_name, "column_text" },
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
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_data_style },
			{
				{ schema.idf_name, "column_data" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
			{ schema.idf_label_style },
			{
				{ schema.idf_name, "label" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_axis_style },
			{
				{ schema.idf_name, "chart_axis" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_legend_style },
			{
				{ schema.idf_name, "chart_legend" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_block_style },
			{
				{ schema.idf_name, "chart_block" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
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
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_tooltip_style },
			{
				{ schema.idf_name, "tooltip" },
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
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_error_style },
			{
				{ schema.idf_name, "error" },
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_client_style },
			{
				{ schema.idf_name, "client" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_carrier_style },
			{
				{ schema.idf_name, "carrier" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_coverage_style },
			{
				{ schema.idf_name, "coverage" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_product_style },
			{
				{ schema.idf_name, "product" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_system_style },
			{
				{ schema.idf_name, "system" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_home_style },
			{
				{ schema.idf_name, "home" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_login_style },
			{
				{ schema.idf_name, "login" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_a1_style },
			{
				{ schema.idf_name, "company_a1" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_a2_style },
			{
				{ schema.idf_name, "company_a2" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_a3_style },
			{
				{ schema.idf_name, "company_a3" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_b1_style },
			{
				{ schema.idf_name, "company_b1" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_b2_style },
			{
				{ schema.idf_name, "company_b2" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_b3_style },
			{
				{ schema.idf_name, "company_b3" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_c1_style },
			{
				{ schema.idf_name, "company_c1" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_c2_style },
			{
				{ schema.idf_name, "company_c2" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_c3_style },
			{
				{ schema.idf_name, "company_c3" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_d1_style },
			{
				{ schema.idf_name, "company_d1" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_d2_style },
			{
				{ schema.idf_name, "company_d2" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_d3_style },
			{
				{ schema.idf_name, "company_d3" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_company_deductible_style },
			{
				{ schema.idf_name, "deductible" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

	}

}
