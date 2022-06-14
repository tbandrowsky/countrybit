
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
		pcr.class_name = "home";
		pcr.class_description = "Application Home";
		pcr.field_id_primary_key = idf_home;
		pcr.member_fields = { idf_home };
		idc_home = schema.put_class(pcr);

		idf_client_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "client_root", "Clients" }, { 0, INT64_MAX } });
		pcr.class_name = "client_root";
		pcr.class_description = "Clients";
		pcr.field_id_primary_key = idf_client_root;
		pcr.member_fields = { idf_client_root, idf_home };
		idc_home = schema.put_class(pcr);

		idf_client = schema.put_integer_field({ { null_row,  jtype::type_int64, "client_id", "Client Id" }, { 0, INT64_MAX } });
		pcr.class_name = "client";
		pcr.class_description = "Client";
		pcr.field_id_primary_key = idf_client;
		pcr.member_fields = { idf_client, idf_client_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_client = schema.put_class(pcr);

		idf_carrier_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "carrier_root", "Carriers" }, { 0, INT64_MAX } });
		pcr.class_name = "carrier_root";
		pcr.class_description = "Carriers";
		pcr.field_id_primary_key = idf_carrier_root;
		pcr.member_fields = { idf_carrier_root, idf_home };
		idc_carrier_root = schema.put_class(pcr);

		idf_carrier = schema.put_integer_field({ { null_row,  jtype::type_int64, "carrier_id", "Carrier Id" }, { 0, INT64_MAX } });
		pcr.class_name = "carrier";
		pcr.class_description = "Carrier";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_carrier, idf_carrier_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_carrier = schema.put_class(pcr);

		idf_coverage_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "coverage_root", "Coverages" }, { 0, INT64_MAX } });
		pcr.class_name = "coverage_root";
		pcr.class_description = "Carriers";
		pcr.field_id_primary_key = idf_coverage_root;
		pcr.member_fields = { idf_coverage_root, idf_home };
		idc_coverage_root = schema.put_class(pcr);

		idf_coverage = schema.put_integer_field({ { null_row,  jtype::type_int64, "coverage_id", "Coverage Id" }, { 0, INT64_MAX } });
		pcr.class_name = "coverage";
		pcr.class_description = "Coverage";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_coverage, idf_coverage_root, schema.idf_name };
		idc_coverage = schema.put_class(pcr);

		idf_inception = schema.put_time_field({ { null_row,  jtype::type_datetime, "home", "Home id" }, { 0, INT64_MAX } });
		idf_expiration = schema.put_time_field({ { null_row,  jtype::type_datetime, "home", "Home id" }, { 0, INT64_MAX } });
		idf_status = schema.put_integer_field({ { null_row,  jtype::type_int32, "home", "Home id" }, { 0, INT64_MAX } });
		idf_attachment = schema.put_double_field({ { null_row,  jtype::type_float32, "home", "Home id" }, { 0.0, 1E10 } });
		idf_limit = schema.put_double_field({ { null_row,  jtype::type_float32, "home", "Home id" }, { 0.0, 1E10 } });
		idf_attachment = schema.put_double_field({ { null_row,  jtype::type_float32, "home", "Home id" }, { 0.0, 1E10 } });
		idf_deductible = schema.put_double_field({ { null_row,  jtype::type_float32, "home", "Home id" }, { 0.0, 1E10 } });
		idf_share = schema.put_double_field({ { null_row,  jtype::type_float32, "home", "Home id" }, { 0.0, 1E10 } });

		idf_product_template_root = schema.put_integer_field({ { null_row,  jtype::type_int64, "home", "Home id" }, { 0, INT64_MAX } });
		pcr.class_name = "product_templates";
		pcr.class_description = "Product Templates";
		pcr.field_id_primary_key = idf_product_template_root;
		pcr.member_fields = { idf_product_template_root, idf_home };
		idc_product_template_root = schema.put_class(pcr);

		idf_product_template = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_template", "Product Template id" }, { 0, INT64_MAX } });
		idf_product_template_name = schema.put_string_field({ { null_row,  jtype::type_string, "product_template_name", "Name" }, { 100, "", "" } });
		idf_product_template_code = schema.put_string_field({ { null_row,  jtype::type_string, "product_template_code", "Code" }, { 32, "", "" }});
		idf_product_template_status = schema.put_string_field({ { null_row,  jtype::type_string, "product_template_status", "Status" }, { 32, "", "" } });
		idf_product_template_edition_start = schema.put_time_field({ { null_row,  jtype::type_datetime, "product_template_edition_start", "Edition Start" }, { 0, INT64_MAX } });
		idf_product_template_edition_stop = schema.put_time_field({ { null_row,  jtype::type_datetime, "product_template_edition_stop", "Edition Stop" }, { 0, INT64_MAX } });
		idf_product_template_type = schema.put_string_field({ { null_row,  jtype::type_string, "product_template_type", "Type" }, { 32, "", "" } });
		idf_product_template_line_of_business = schema.put_string_field({ { null_row,  jtype::type_string, "product_template_line_of_business", "Line of Business" }, { 32, "", "" } });
		idf_product_template_carrier = schema.put_string_field({ { null_row,  jtype::type_string, "home", "Home id" }, { 32, "", "" } });
		idf_product_template_updated_by = schema.put_string_field({ { null_row,  jtype::type_string, "home", "Home id" }, { 32, "", "" } });
		
		porf = { { null_row, jtype::type_list, "product_template_product_header", "Product Header" }, { {1,1,1}, schema.idc_user_class } };
		idf_product_template_product_header = schema.put_object_field(porf);

		porf = { { null_row, jtype::type_list, "product_template_product_structure", "Product Structure" }, { {32,1,1}, schema.idc_user_class } };
		idf_product_template_product_structure = schema.put_object_field(porf);

		porf = { { null_row, jtype::type_list, "product_template_coverage_header", "Coverage Header" }, { {1,1,1}, schema.idc_user_class } };
		idf_product_template_coverage_header = schema.put_object_field(porf);

		porf = { { null_row, jtype::type_list, "product_template_coverage_structure", "Coverage Structure" }, { {32,1,1}, schema.idc_user_class } };
		idf_product_template_coverage_structure = schema.put_object_field(porf);

		pcr.class_name = "product_template";
		pcr.class_description = "Product Template";
		pcr.field_id_primary_key = idf_product_template;
		pcr.member_fields = { 
			idf_product_template, 
			idf_product_template_root,
			idf_product_template_name, 
			idf_product_template_code, 
			idf_product_template_status, 
			idf_product_template_edition_start,
			idf_product_template_edition_stop,
			idf_product_template_type,
			idf_product_template_line_of_business,
			idf_product_template_carrier,
			idf_product_template_updated_by
		};

		idf_program = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_id", "Program" }, { 0, INT64_MAX } });
		idf_program_view = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_view", "Program View" }, { 0, INT64_MAX } });
		idf_program_title = schema.put_string_field({ { null_row,  jtype::type_string, "program_title", "Program Title" }, { 100, "", "" }});
		idf_program_subtitle = schema.put_string_field({ { null_row,  jtype::type_string, "program_title", "Program Subtitle" }, { 100, "", "" } });
		pcr.class_name = "program";
		pcr.class_description = "Program";
		pcr.field_id_primary_key = idf_program;
		pcr.member_fields = { idf_program, idf_client, idf_program_view, schema.idf_name, idf_program_title, idf_program_subtitle };
		idc_coverage = schema.put_class(pcr);

		idf_program_product = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_product_id", "Product Instance for Program" }, { 0, INT64_MAX } });
		idc_product_instance = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_instance_class_id", "Product Instance Class" }, { 0, INT64_MAX } });
		idf_product_instance = schema.put_integer_field({ { null_row,  jtype::type_int64, "product_instance_object_id", "Product Instance Object" }, { 0, INT64_MAX } });
		pcr.class_name = "program_product";
		pcr.class_description = "Program Product";
		pcr.field_id_primary_key = idf_program_product;
		pcr.member_fields = { idf_program_product, idf_program, idf_product_template, idc_product_instance, idf_product_instance };
		idc_program_product = schema.put_class(pcr);

		idf_program_chart_slide = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_chart_slide_id", "Program Chart Slide" }, { 0, INT64_MAX } });
		pcr.class_name = "program_chart_slide";
		pcr.class_description = "Program Chart Slide";
		pcr.field_id_primary_key = idf_program_chart_slide;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_slide_title };
		idc_program_chart_slide = schema.put_class(pcr);

		idf_program_chart_slide_product = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_chart_slide_product_id", "Product for Program Chart Slide" }, { 0, INT64_MAX } });
		pcr.class_name = "program_chart_slide_product";
		pcr.class_description = "Program Chart Slide Product";
		pcr.field_id_primary_key = idf_program_chart_slide_product;
		pcr.member_fields = { idf_program_chart_slide, idf_client, idf_program, idf_slide_title };
		idc_program_chart_slide_product = schema.put_class(pcr);

		idf_program_generic_slide = schema.put_integer_field({ { null_row,  jtype::type_int64, "program_generic_slide", "Generic Slide" }, { 0, INT64_MAX } });
		pcr.class_name = "program_generic_slide";
		pcr.class_description = "Program Generic Slide";
		pcr.field_id_primary_key = idf_program_chart_slide;
		pcr.member_fields = { idf_program_generic_slide, idf_client, idf_program, idf_slide_title };
		idc_program_generic_slide = schema.put_class(pcr);

		jmodel jm;

		schema.put_model(jm);

		jcollection_ref ref;
		ref.data = &box;
		ref.model_name = jm.name;
		ref.max_actors = 2;
		ref.max_objects = 100;
		ref.collection_size_bytes = 1 << 19;

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

		relative_ptr_type style_sheet_id = null_row;
		auto style_sheet = program_chart.create_object(0, null_row, schema.idc_style_sheet, style_sheet_id);
		style_sheet.set(
			{ schema.idf_view_style },
			{
				{ schema.idf_name, "view_background" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 10.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, "4" },
				{ schema.idf_box_border_color, "#ffffffff" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_title_style },
			{
				{ schema.idf_name, "view_title" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_subtitle_style },
			{
				{ schema.idf_name, "view_subtitle" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_section_style },
			{
				{ schema.idf_name, "view_section" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_view_style },
			{
				{ schema.idf_name, "view" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_disclaimer_style },
			{
				{ schema.idf_name, "disclaimer" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_copyright_style },
			{
				{ schema.idf_name, "copyright" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h1_style },
			{
				{ schema.idf_name, "h1" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h2_style },
			{
				{ schema.idf_name, "h2" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_h3_style },
			{
				{ schema.idf_name, "h3" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 20.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_number_head_style },
			{
				{ schema.idf_name, "column_number_head" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_column_text_head_style },
			{
				{ schema.idf_name, "column_text_head" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);


		style_sheet.set(
			{ schema.idf_column_data_style },
			{
				{ schema.idf_name, "column_data" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);


		style_sheet.set(
			{ schema.idf_label_style },
			{
				{ schema.idf_name, "label" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_axis_style },
			{
				{ schema.idf_name, "chart_axis" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_legend_style },
			{
				{ schema.idf_name, "chart_legend" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_chart_block_style },
			{
				{ schema.idf_name, "chart_block" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_tooltip_style },
			{
				{ schema.idf_name, "tooltip" },
				{ schema.idf_font_name, "Arial" },
				{ schema.idf_font_size, 30.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.idf_breadcrumb_style },
			{
				{ schema.idf_name, "breadcrumb" },
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
				{ schema.idf_shape_border_thickness, "" },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "" },
				{ schema.idf_box_border_thickness, "" },
				{ schema.idf_box_border_color, "" }
			}
			);

		relative_ptr_type homes_id = null_row;
		relative_ptr_type clients_id = null_row;
		relative_ptr_type carriers_id = null_row;
		relative_ptr_type programs_id = null_row;

		program_chart.create_object(0, sample_actor.actor_id, idc_home, homes_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_client_root, clients_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, carriers_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, programs_id);

		//{ idname, idfont_name, idfont_size, idbold, iditalic, idline_spacing, idhorizontal_alignment, idvertical_alignment,
	//idshape_fill_color, idshape_border_thickness, idshape_border_color, idbox_fill_color, idbox_border_thickness, idbox_border_color };

		//			pcr.member_fields = { idname, id_view_title, id_view_subtitle, id_view_section, id_view, id_disclaimer, id_copyright,
		//id_h1, id_h2, id_h3, id_column_number_head, id_column_text_head, id_column_data, id_label, id_control, id_chart_axis, id_chart_legend, id_chart_block, id_tooltip };

	}

	wsproposal_controller::~wsproposal_controller()
	{
		;
	}

	void wsproposal_controller::render_client_view(const rectangle& newSize)
	{
		;
	}

	void wsproposal_controller::render_carrier_view(const rectangle& newSize)
	{
		;
	}

	void wsproposal_controller::render_coverage_view(const rectangle& newSize)
	{
		;
	}

	void wsproposal_controller::render_program_view(const rectangle& newSize)
	{
		corona::database::layout_rect title_box, * ptitle_box = &title_box;

		auto left_margin = 20.0_px;
		auto chart_top = 10.0_px;

		auto mainr = row(nullptr, null_row);
		auto controlcolumn = column(mainr, null_row, { 0.0_px,0.0_px,25.0_pct,100.0_pct });
		auto d2dcolumn = column(mainr, null_row, { 0.0_px,0.0_px,75.0_pct,100.0_pct });
		auto d2dwin = canvas2d(d2dcolumn, schema.id_view_background, { 0.0_px,0.0_px,100.0_pct,100.0_pct });
		auto d2dwin_area = column(d2dwin, schema.id_view_background, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		auto navigation_bar = row(d2dwin, schema.id_view_background, { 0.0_px, 0.0_px, 100.0_pct, -50.0_px });
		breadcrumbs(navigation_bar, [](jobject& _item) {
			return nullptr;
			}, {0.0_px, 0.0_px, 100.0_px, 20.0_px});

		// editable controls on the left
		add_update_fields(controlcolumn);
		add_create_buttons(controlcolumn);

		if (state.actor.current_view_class_id == idhome_class) 
		{
			
		} 
		else if (state.actor.current_view_class_id == idhome_class)
		{
			;
		}
		else if (state.actor.current_view_class_id == idhome_class)
		{
			;
		}
		else if (state.actor.current_view_class_id == idhome_class)
		{
			;
		}
		else if (state.actor.current_view_class_id == idhome_class)
		{
			;
		}

		// draw the slides at the top
		relative_ptr_type slide_fields[2] = { idslide_heading1, null_row };
		auto slide_area = row_common(d2dwin, { 0.0_px, 0.0_px, 100.0_pct, 200.0_px }, slide_fields);

		// 


		// and now draw the selected slide
		relative_ptr_type title_classes[2] = { idslide_title_class, null_row };

		title_box.width = 500.0_px;
		title_box.height = 100.0_px;
		title_box.x = left_margin;
		title_box.y = chart_top;
		auto title_area = column(d2dwin, schema.id_view_title, title_box);
		column(title_area, { 5.0_px, 0.0_px, 500.0_px, 75.0_px }, slide_title_class_id);

		//const database::actor_view_object& avo, database::jobject& slice
		for_each(program_class_id, [ptitle_box](const corona::database::actor_view_object& avo, corona::database::jobject& slice) {
			auto rbx = slice.get_layout_rect("layout_rect");
			rbx = *ptitle_box;
			return true;
			});

		corona::database::layout_rect legend_box, * plegend_box = &legend_box;
		int carrier_count = state.view_objects.count_if([this](const auto& avokp) { return avokp.second.class_id == carrier_class_id; });

		legend_box.width = 300.0_px;
		legend_box.height = 20.0_px;
		legend_box.height.amount;
		legend_box.x = -1.1_psz;
		legend_box.y = chart_top;

		auto legend_area = column(d2dwin, schema.id_chart_legend, legend_box);
		column(legend_area, { 5.0_px, 0.0_px, 250.0_px, 30.0_px }, carrier_class_id);

		// get dimensions of the chart, the width

		int coverage_count = state.view_objects.count_if([this](auto& avokp) { return avokp.second.class_id == this->coverage_class_id; });
		if (coverage_count < 3)
			coverage_count = 3;
		coverage_count += 2;
		int coverage_width = 1.0 / coverage_count * 100;

		std::cout << std::format("coverages {} w:{}", coverage_count, coverage_width) << std::endl;

		// and the height

		corona::database::relative_ptr_type limit_fields[3], * plimit_fields = &limit_fields[0];
		limit_fields[0] = limit_field_id;
		limit_fields[1] = attachment_field_id;
		limit_fields[2] = null_row;

		double max_amount = 0.0, min_amount = 0.0, count = 0.0;
		double* pmax_amount = &max_amount, * pmin_amount = &min_amount, * pcount = &count;

		for_each(plimit_fields, [this, pmax_amount, pmin_amount, pcount, plimit_fields](const corona::database::actor_view_object& avo, corona::database::jobject& slice) {
			corona::database::relative_ptr_type* pfield = plimit_fields;
			std::cout << std::format("{} min:{} max:{}", *pmin_amount, *pmax_amount, *pcount) << std::endl;
			while (*pfield != null_row) {
				auto t = slice.get_double(*pfield, true);
				if (t < *pmin_amount) {
					*pmin_amount = t;
				}
				if (t > *pmax_amount) {
					*pmax_amount = t;
				}
				*pcount += 1.0;
				pfield++;
			}
			return true;
			});

		corona::database::rectangle coverage_box, * pcoverage_box = &coverage_box;

		double chartMargin = -60;
		double chartHeight = newSize.h + chartMargin;
		double scaley = *pmax_amount / chartHeight;

		coverage_box.w = coverage_width;
		coverage_box.h = 30;
		coverage_box.x = coverage_width;
		coverage_box.y = chartHeight;

		corona::database::relative_ptr_type comparison_fields[2], * pcomparison_fields = &comparison_fields[0];
		comparison_fields[0] = coverage_name_id;
		comparison_fields[1] = null_row;

		double policyMax = 0.0;
		double policyMin = 0.0;

		for_each(coverage_class_id, [this, chartHeight, scaley, pcomparison_fields, pcoverage_box, coverage_width](const corona::database::actor_view_object& avo, corona::database::jobject& slice) {
			corona::database::rectangle policy_box, * ppolicy_box = &policy_box;
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *pcoverage_box;
			policy_box.w = pcoverage_box->w;
			policy_box.x = pcoverage_box->x;
			policy_box.h = 0;
			policy_box.y = 0;
			pcoverage_box->x += coverage_width;
			std::cout << std::format("coverage x:{} w:{}", pcoverage_box->x, pcoverage_box->w) << std::endl;

			for_each(slice, pcomparison_fields, [this, chartHeight, scaley, ppolicy_box](const corona::database::actor_view_object& avo, corona::database::jobject& slice) {

				if (slice.has_field(limit_field_id) && slice.has_field(attachment_field_id))
				{
					auto pbx = slice.get_rectangle("rectangle");

					double limit, attach;
					limit = slice.get_double(limit_field_id, true);
					attach = slice.get_double(attachment_field_id, true);

					ppolicy_box->h = chartHeight - limit * scaley;
					ppolicy_box->y = chartHeight - attach * scaley;

					pbx = *ppolicy_box;
				}

				return true;
				});

			return true;
			});

		pg.arrange(newSize.w, newSize.h);

		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsproposal_controller::render(const rectangle& newSize)
	{
		clear();

		if (state.modified_object_id == null_row || state.modified_object.get_class_id() == idclient_class)
		{
			render_client_view(newSize);
		}
		else
		{
			render_program_view(newSize);
		}

	}

}
