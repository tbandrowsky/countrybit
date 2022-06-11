
#pragma once


#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

const int IDC_DIRECT2D = -1;

namespace proposal
{

	using namespace corona::database;

	wsproposal_controller::wsproposal_controller() : corona_controller()
	{
		box.init(1 << 22);
		schema = jschema::create_schema(&box, 50, true, schema_id);

		put_integer_field_request ifr;
		ifr.name.name = "home_id";
		ifr.name.description = "Home Id";
		ifr.name.type_id = jtype::type_int64;
		idhome = schema.put_integer_field(ifr);

		ifr.name.name = "client_root_id";
		ifr.name.description = "Client Root Id";
		ifr.name.type_id = jtype::type_int64;
		idclient_root = schema.put_integer_field(ifr);

		ifr.name.name = "carrier_root_id";
		ifr.name.description = "Carrier Root Id";
		ifr.name.type_id = jtype::type_int64;
		idcarrier_root = schema.put_integer_field(ifr);

		ifr.name.name = "coverage_root_id";
		ifr.name.description = "Coverage Root Id";
		ifr.name.type_id = jtype::type_int64;
		idcoverage_root = schema.put_integer_field(ifr);

		ifr.name.name = "client_id";
		ifr.name.description = "Client Id";
		ifr.name.type_id = jtype::type_int64;
		idprogram = schema.put_integer_field(ifr);

		ifr.name.name = "program_id";
		ifr.name.description = "Program Id";
		ifr.name.type_id = jtype::type_int64;
		idprogram = schema.put_integer_field(ifr);

		ifr.name.name = "policy_id";
		ifr.name.description = "Policy Id";
		idpolicy = schema.put_integer_field(ifr);

		ifr.name.name = "coverage_id";
		ifr.name.description = "Coverage Id";
		idcoverage = schema.put_integer_field(ifr);

		put_double_field_request dfr;
		dfr.name.name = "limit";
		dfr.name.description = "Policy Limit";
		dfr.name.type_id = jtype::type_float64;
		idlimit = schema.put_double_field(dfr);

		dfr.name.name = "attachment";
		dfr.name.description = "Attachment Point";
		idattachment = schema.put_double_field(dfr);

		dfr.name.name = "deductible";
		dfr.name.description = "Deductible";
		iddeductible = schema.put_double_field(dfr);

		dfr.name.name = "share";
		dfr.name.description = "Share";
		idshare = schema.put_double_field(dfr);

		put_string_field_request sfr;
		sfr.name.name = "comment";
		sfr.name.description = "Comment";
		sfr.options.full_text_editor = true;
		sfr.options.length = 512;
		idcomment = schema.put_string_field(sfr);

		sfr.name.name = "client_name";
		sfr.name.description = "Client name";
		sfr.options.length = 200;
		idclient_name = schema.put_string_field(sfr);

		sfr.name.name = "program_name";
		sfr.name.description = "Program name";
		idprogram_name = schema.put_string_field(sfr);

		sfr.name.name = "program_description";
		sfr.name.description = "Program Description";
		idprogram_description = schema.put_string_field(sfr);

		sfr.name.name = "policy_name";
		sfr.name.description = "Policy name";
		idpolicy_name = schema.put_string_field(sfr);

		sfr.name.name = "coverage_name";
		sfr.name.description = "Coverage Name";
		idcoverage_name = schema.put_string_field(sfr);

		sfr.name.name = "carrier_name";
		sfr.name.description = "Carrier Name";
		idcarrier_name = schema.put_string_field(sfr);

		sfr.name.name = "property_sov";
		sfr.name.description = "Placeholder Buildings SOV";
		idproperty_list = schema.put_string_field(sfr);

		sfr.name.name = "wc_sov";
		sfr.name.description = "Placeholder Aircraft SOV";
		idwc_list = schema.put_string_field(sfr);

		sfr.name.name = "vehicle_sov";
		sfr.name.description = "Placeholder Vehicle SOV";
		idvehicle_list = schema.put_string_field(sfr);

		sfr.name.name = "aircraft_sov";
		sfr.name.description = "Placeholder Aircraft SOV";
		idaircraft_list = schema.put_string_field(sfr);

		sfr.name.name = "slide_heading1";
		sfr.name.description = "Slide Heading";
		idslide_heading1 = schema.put_string_field(sfr);

		sfr.name.name = "slide_heading2";
		sfr.name.description = "Slide SubHeading";
		idslide_heading2 = schema.put_string_field(sfr);

		put_class_request pcr;
		pcr.class_name = "home";
		pcr.class_description = "Home";
		pcr.member_fields = { idhome, schema.idlong_name };
		pcr.field_id_primary_key = idhome;
		idhome_class = schema.put_class(pcr);

		pcr.class_name = "clients";
		pcr.class_description = "Clients";
		pcr.member_fields = { idhome, idclient_root, schema.idlong_name };
		pcr.field_id_primary_key = idclient_root;
		idclient_root_class = schema.put_class(pcr);

		pcr.class_name = "carriers";
		pcr.class_description = "Carriers";
		pcr.member_fields = { idhome, idcarrier_root, schema.idlong_name };
		pcr.field_id_primary_key = idcarrier_root;
		idcarrier_root_class = schema.put_class(pcr);

		pcr.class_name = "coverages";
		pcr.class_description = "Coverages";
		pcr.member_fields = { idhome, idcoverage_root, schema.idlong_name };
		pcr.field_id_primary_key = idcarrier_root;
		idcoverage_root_class = schema.put_class(pcr);


		pcr.class_name = "client";
		pcr.class_description = "Client";
		pcr.member_fields = { idclient_root, idclient, idclient_name, schema.idfirst_name, schema.idlast_name, schema.idstreet, schema.idcity, schema.idstate, schema.idpostal  };
		pcr.field_id_primary_key = idclient;
		idclient_class = schema.put_class(pcr);

		pcr.class_name = "carrier";
		pcr.class_description = "Carrier";
		pcr.member_fields = { idcarrier_root, idcarrier, idcarrier_name, schema.idfirst_name, schema.idlast_name, schema.idstreet, schema.idcity, schema.idstate, schema.idpostal, schema.idrectangle, schema.idlayout_rect };
		pcr.field_id_primary_key = idcarrier;
		idcarrier_class = schema.put_class(pcr);

		pcr.class_name = "coverage";
		pcr.class_description = "Coverage";
		pcr.member_fields = { idcoverage_root, idcoverage, idcoverage_name, schema.idrectangle, schema.idlayout_rect };
		pcr.field_id_primary_key = idcoverage;
		idprogram_class = schema.put_class(pcr);

		pcr.class_name = "program";
		pcr.class_description = "Program";
		pcr.member_fields = { idclient, idprogram, idprogram_name, idprogram_description, idprogram_view, idproperty_list, idwc_list, idaircraft_list, idvehicle_list, schema.idrectangle, schema.idlayout_rect };
		pcr.field_id_primary_key = idprogram;
		idprogram_class = schema.put_class(pcr);

		pcr.class_name = "buildings_policy";
		pcr.class_description = "Buildings";
		pcr.member_fields = { idprogram, idclient, idcarrier, idpolicy, idlimit, idattachment, idshare, schema.idcolor, schema.idrectangle, schema.idlayout_rect };
		idpolicy_property_class = schema.put_class(pcr);

		pcr.class_name = "wc_policy";
		pcr.class_description = "Workers Comp";
		pcr.member_fields = { idprogram, idclient, idcarrier, idpolicy, idlimit, idattachment, idshare, schema.idcolor, schema.idrectangle, schema.idlayout_rect };
		idpolicy_wc_class = schema.put_class(pcr);

		pcr.class_name = "vehicles_policy";
		pcr.class_description = "Vehicles";
		pcr.member_fields = { idprogram, idclient, idcarrier, idpolicy, idlimit, idattachment, idshare, schema.idcolor, schema.idrectangle, schema.idlayout_rect };
		idpolicy_vehicles_class = schema.put_class(pcr);

		pcr.class_name = "aircraft_policy";
		pcr.class_description = "Aircraft";
		pcr.member_fields = { idprogram, idclient, idcarrier, idpolicy, idlimit, idattachment, idshare, schema.idcolor, schema.idrectangle, schema.idlayout_rect };
		idpolicy_aircraft_class = schema.put_class(pcr);

		pcr.class_name = "umbrella_policy";
		pcr.class_description = "Umbrella";
		pcr.member_fields = { idprogram, idclient, idcarrier, idpolicy, idlimit, idattachment, idshare, schema.idcolor, schema.idrectangle, schema.idlayout_rect };
		idpolicy_umbrella_class = schema.put_class(pcr);

		pcr.class_name = "slide_title";
		pcr.class_description = "Title Slide";
		pcr.member_fields = { idprogram, idclient, idslide_heading1, idslide_heading2, schema.idrectangle, schema.idlayout_rect };
		idslide_title_class = schema.put_class(pcr);

		pcr.class_name = "slide_chart";
		pcr.class_description = "Chart Slide";
		pcr.member_fields = { idprogram, idclient, idslide_heading1, idslide_heading2, schema.idrectangle, schema.idlayout_rect };
		idslide_program_chart_class = schema.put_class(pcr);

		pcr.class_name = "slide_chart";
		pcr.class_description = "Comparison Slide?";
		pcr.member_fields = { idprogram, idclient, idslide_heading1, idslide_heading2, schema.idrectangle, schema.idlayout_rect };
		idslide_demo_cart_class = schema.put_class(pcr);

		jmodel jm;

		jm.name = "program_chart";

		model_creatable_class* mcr;
		model_selectable_class* msr;
		model_updatable_class* mur;
		selector_rule* sr;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Client";
		mcr->selectors.when(idclient_root_class);
		mcr->create_class_id = idclient_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Carrier";
		mcr->selectors.when(idcarrier_root_class);
		mcr->create_class_id = idcarrier_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr->rule_name = "Add Coverage";
		mcr->selectors.when(idcoverage_root_class);
		mcr->create_class_id = idcoverage_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Buildings Policy";
		mcr->selectors.when(idcarrier_class, idprogram);
		mcr->create_class_id = idpolicy_property_class;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add WC Policy";
		mcr->selectors.when(idcarrier_class, idprogram);
		mcr->create_class_id = idpolicy_wc_class;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Aircraft Policy";
		mcr->selectors.when(idcarrier_class, idprogram);
		mcr->create_class_id = idpolicy_aircraft_class;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Aircraft Policy";
		mcr->selectors.when(idcarrier_class, idprogram);
		mcr->create_class_id = idpolicy_aircraft_class;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Program";
		mcr->selectors.when(idclient_class);
		mcr->create_class_id = idprogram_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Title Slide";
		mcr->selectors.when(idprogram_class);
		mcr->create_class_id = idslide_title_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Program Slide";
		mcr->selectors.when(idprogram_class);
		mcr->create_class_id = idslide_program_chart_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "Add Comparison Slide";
		mcr->selectors.when(idprogram_class);
		mcr->create_class_id = idslide_demo_cart_class;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		msr = jm.select_options.append();
		msr->rule_name = "select client";
		msr->select_class_id = idclient_class;

		msr = jm.select_options.append();
		msr->rule_name = "select coverage";
		msr->select_class_id = idcoverage_class;

		msr = jm.select_options.append();
		msr->rule_name = "select carrier";
		msr->select_class_id = idcarrier_class;

		msr = jm.select_options.append();
		msr->rule_name = "select program";
		msr->select_class_id = idprogram_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy1";
		msr->select_class_id = idpolicy_property_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy2";
		msr->select_class_id = idpolicy_wc_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy3";
		msr->select_class_id = idpolicy_vehicles_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy4";
		msr->select_class_id = idpolicy_aircraft_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy5";
		msr->select_class_id = idpolicy_umbrella_class;

		msr = jm.select_options.append();
		msr->rule_name = "select policy cov";
		msr->select_class_id = idpolicy_coverage_class;

		mur = jm.update_options.append();
		mur->rule_name = "update client";
		mur->update_class_id = idclient_class;

		mur = jm.update_options.append();
		mur->rule_name = "update coverage";
		mur->update_class_id = idcoverage_class;

		mur = jm.update_options.append();
		mur->rule_name = "update carrier";
		mur->update_class_id = idcarrier_class;

		mur = jm.update_options.append();
		mur->rule_name = "update program";
		mur->update_class_id = idprogram_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy1";
		mur->update_class_id = idpolicy_property_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy1";
		mur->update_class_id = idpolicy_wc_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy2";
		mur->update_class_id = idpolicy_vehicles_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy3";
		mur->update_class_id = idpolicy_aircraft_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy4";
		mur->update_class_id = idpolicy_umbrella_class;

		mur = jm.update_options.append();
		mur->rule_name = "update policy";
		mur->update_class_id = idpolicy_coverage_class;

		jm.selection_hierarchy.push_back({ 0, idhome_class });
		jm.selection_hierarchy.push_back({ 1, idclient_root_class });
		jm.selection_hierarchy.push_back({ 1, idcoverage_root_class });
		jm.selection_hierarchy.push_back({ 1, idcarrier_root_class });
		jm.selection_hierarchy.push_back({ 2, idclient_class });
		jm.selection_hierarchy.push_back({ 2, idcoverage_class });
		jm.selection_hierarchy.push_back({ 2, idcarrier_class });
		jm.selection_hierarchy.push_back({ 3, idprogram_class });
		jm.selection_hierarchy.push_back({ 4, idpolicy_property_class });
		jm.selection_hierarchy.push_back({ 4, idpolicy_wc_class });
		jm.selection_hierarchy.push_back({ 4, idpolicy_vehicles_class });
		jm.selection_hierarchy.push_back({ 4, idpolicy_aircraft_class });
		jm.selection_hierarchy.push_back({ 4, idpolicy_umbrella_class });
		jm.selection_hierarchy.push_back({ 5, idpolicy_coverage_class });
		jm.selection_hierarchy.push_back({ 6, idslide_title_class });
		jm.selection_hierarchy.push_back({ 6, idslide_demo_cart_class });
		jm.selection_hierarchy.push_back({ 6, idslide_program_chart_class });

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
		sample_actor.current_view_class_id = idhome_class;
		sample_actor = program_chart.create_actor(sample_actor);

		relative_ptr_type style_sheet_id = null_row;
		auto style_sheet = program_chart.create_object(0, null_row, schema.id_style_sheet, style_sheet_id);
		style_sheet.set(
			{ schema.id_view_title },
			{
				{ schema.idname, "view_background" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 10.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "#ffffffFF" },
				{ schema.idbox_border_thickness, "4" },
				{ schema.idbox_border_color, "#ffffffff" }
			}
			);

		style_sheet.set(
			{ schema.id_view_title },
			{
				{ schema.idname, "view_title" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_view_subtitle },
			{
				{ schema.idname, "view_subtitle" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 24.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_view_section },
			{
				{ schema.idname, "view_section" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 16.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_view },
			{
				{ schema.idname, "view" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 16.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_disclaimer },
			{
				{ schema.idname, "disclaimer" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 12.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_copyright },
			{
				{ schema.idname, "copyright" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 12.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_h1 },
			{
				{ schema.idname, "h1" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_h2 },
			{
				{ schema.idname, "h2" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 24.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_h3 },
			{
				{ schema.idname, "h3" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 20.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_column_number_head },
			{
				{ schema.idname, "column_number_head" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 14.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_column_text_head },
			{
				{ schema.idname, "column_text_head" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 14.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);


		style_sheet.set(
			{ schema.id_column_data },
			{
				{ schema.idname, "column_data" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);


		style_sheet.set(
			{ schema.id_label },
			{
				{ schema.idname, "label" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_chart_axis },
			{
				{ schema.idname, "chart_axis" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_chart_legend },
			{
				{ schema.idname, "chart_legend" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_chart_block },
			{
				{ schema.idname, "chart_block" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_tooltip },
			{
				{ schema.idname, "tooltip" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 30.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#000000FF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		style_sheet.set(
			{ schema.id_breadcrumb },
			{
				{ schema.idname, "breadcrumb" },
				{ schema.idfont_name, "Arial" },
				{ schema.idfont_size, 14.0 },
				{ schema.idbold, false },
				{ schema.iditalic, false },
				{ schema.idunderline, false },
				{ schema.idstrike_through, false },
				{ schema.idline_spacing, 0.0 },
				{ schema.idhorizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idvertical_alignment, (int)visual_alignment::align_near },
				{ schema.idshape_fill_color, "#0000CCFF" },
				{ schema.idshape_border_thickness, "" },
				{ schema.idshape_border_color, "" },
				{ schema.idbox_fill_color, "" },
				{ schema.idbox_border_thickness, "" },
				{ schema.idbox_border_color, "" }
			}
			);

		relative_ptr_type existing_client_id = null_row;
		auto client = program_chart.create_object(0, sample_actor.actor_id, idclient_class, existing_client_id);

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
		breadcrumbs(navigation_bar, [](jslice& _item) {
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

		//const database::actor_view_object& avo, database::jslice& slice
		for_each(program_class_id, [ptitle_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
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

		for_each(plimit_fields, [this, pmax_amount, pmin_amount, pcount, plimit_fields](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
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

		for_each(coverage_class_id, [this, chartHeight, scaley, pcomparison_fields, pcoverage_box, coverage_width](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			corona::database::rectangle policy_box, * ppolicy_box = &policy_box;
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *pcoverage_box;
			policy_box.w = pcoverage_box->w;
			policy_box.x = pcoverage_box->x;
			policy_box.h = 0;
			policy_box.y = 0;
			pcoverage_box->x += coverage_width;
			std::cout << std::format("coverage x:{} w:{}", pcoverage_box->x, pcoverage_box->w) << std::endl;

			for_each(slice, pcomparison_fields, [this, chartHeight, scaley, ppolicy_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {

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
