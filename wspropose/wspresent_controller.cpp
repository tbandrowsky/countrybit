
#pragma once


#include "corona.h"
#include "resource.h"
#include "wspresent_controller.h"

namespace proposal
{

	using namespace corona::database;

	wspresent_controller::wspresent_controller() : corona_controller()
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

		/*
		Now that we have our fields, we can make our classes.
		*/

		/*
		We have a class object to represent our application states and objects with respect to this model.  We supply a class name, description, a list
		of member fields, and then, a primary key.  We call put_class to upsert the class, and the class id is returned for easy reference.
		The schema can also be searched by the class_name to find the class.
		*/

		/* This is the application home class.  It is just a place holder now */

		jmodel jm;


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

		set_style_sheet();

		create_object_request cor;

		cor.actor_id = actor_id;
//		cor.class_id = idc_home;
		cor.collection_id = user_collection.get_collection_id();
		cor.item_id = -1;
		cor.select_on_create = true;
		cor.template_item_id = -1;

		// create our initial object, and get our state.
		state = user_collection.create_object(cor, "create home");

	}

	wspresent_controller::~wspresent_controller()
	{
		;
	}

	void wspresent_controller::render_navigation_frame(std::function<void(page_item* navigation_contents, page_item* _frame)> _contents)
	{
		clear();

	}

	void wspresent_controller::render_form(page_item* _navigation, page_item* _frame, const char* _form_title)
	{
		_frame->windowsRegion = true;
		add_update_fields(_frame, field_layout::label_on_left, _form_title);
		space(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wspresent_controller::render_search_page(page_item* _navigation, page_item* _contents, relative_ptr_type _class_id, const char* _form_title, int count_fields, relative_ptr_type* _field_ids)
	{
		auto form_search = row(_contents, null_row, { 0.0_px, 0.0_px, 100.0_pct, 25.0_px });
		add_update_fields(form_search, field_layout::label_on_left, _form_title);
		auto form_table = canvas2d_column(id_canvas_form_table_a, _contents, schema.idf_view_background_style);
		search_table(form_table, _class_id, _field_ids, count_fields);
		text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wspresent_controller::render(const rectangle& newSize)
	{
		auto style_sheet = getStyleSheet();
		arrange(newSize.w, newSize.h, style_sheet, 16);
		canvasWindowsId = host->renderPage(pg, &schema, state, user_collection);
		host->redraw();
	}

	void wspresent_controller::set_style_sheet()
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

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_navigation_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#BFBFBFFF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_navigation_selected_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#002060FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

	}

}
