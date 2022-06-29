
#pragma once


#include "corona.h"
#include "resource.h"
#include "bridges_controller.h"

namespace bridges
{

	using namespace corona::database;

	bridges_controller::bridges_controller() : corona_controller()
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

		jmodel jm;

		jm.update_always(&schema, idc_home);

		jm.select_always(&schema, idc_home);

		jm.delete_always(&schema, idc_carrier);

		jm.create_when(&schema, idc_carrier_root, idc_carrier, null_row, true, false);

		jm.navigation({
						{ idc_home, 0},
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
		program_chart.create_object(null_row, sample_actor.actor_id, idc_carrier_root, id_carrier_root, { { idf_home , id_home } });
		relative_ptr_type style_sheet_id = null_row;

		relative_ptr_type homes_id = null_row;
		relative_ptr_type clients_id = null_row;
		relative_ptr_type carriers_id = null_row;
		relative_ptr_type programs_id = null_row;

		program_chart.create_object(0, sample_actor.actor_id, idc_home, homes_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, carriers_id);
		program_chart.create_object(0, sample_actor.actor_id, idc_carrier_root, programs_id);

		set_style_sheet();

		map_style(idc_home, schema.idf_home_style);
		map_style(idc_carrier_root, schema.idf_carrier_style);

		}

			bridges_controller::~bridges_controller()
		{
			;
		}

		void bridges_controller::render_header(page_item* _frame, const char* _title, const char* _subtitle, bool _left_pad)
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

		void bridges_controller::render_form(std::function<void(page_item* _frame)> _contents)
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


		void bridges_controller::render_search(std::function<void(page_item* _frame)> _contents)
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

		void bridges_controller::render_visual(std::function<void(page_item* _frame)> _contents)
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

		void bridges_controller::render_login()
		{
			;
		}

		void bridges_controller::render_registration()
		{
			;
		}

		void bridges_controller::render_password_reminder()
		{
			;
		}

		void bridges_controller::render_home()
		{
			;
		}

		void bridges_controller::render_inventory()
		{
			;
		}

		void bridges_controller::render_world()
		{
			;
		}

		void bridges_controller::render_login_contents(page_item* _frame)
		{
			;
		}

		void bridges_controller::render_registration_contents(page_item* _frame)
		{
			;
		}

		void bridges_controller::render_password_reminder_contents(page_item* _frame)
		{
			;
		}

		void bridges_controller::render_inventory_contents(page_item* _frame)
		{
			;
		}

		void bridges_controller::render_world(page_item* _frame)
		{
			;
		}

		void bridges_controller::render(const rectangle& newSize)
		{
			if (state.actor.current_view_class_id == idc_home)
			{
				render_home();
			}

			arrange(newSize.w, newSize.h);
			canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
			host->redraw();
		}

		void bridges_controller::set_style_sheet()
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
