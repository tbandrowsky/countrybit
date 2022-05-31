
#pragma once


#include "pch.h"
#include "resource.h"
#include "wspropose_controller.h"

const int IDC_DIRECT2D = -1;

namespace proposal
{

	using namespace corona::database;

	wsproposal_controller::wsproposal_controller(viewStyle* _vs) : corona_controller(_vs)
	{
		;
	}

	wsproposal_controller::~wsproposal_controller()
	{
		;
	}

	void wsproposal_controller::loadController()
	{
		;
	}

	void wsproposal_controller::onInit()
	{
		enableEditMessages = false;

		auto pos = host->getWindowPos(0);
		sizeIntDto curSize(pos.width, pos.height);
		host->setMinimumWindowSize(curSize);

		box.init(1 << 22);
		schema = jschema::create_schema(&box, 50, true, schema_id);

		put_double_field_request dfr;
		dfr.name.name = "limit";
		dfr.name.description = "Maximum amount paid by policy";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		relative_ptr_type limit_field_id = schema.put_double_field(dfr);

		dfr.name.name = "attachment";
		dfr.name.description = "Point at which policy begins coverage";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		attachment_field_id = schema.put_double_field(dfr);

		dfr.name.name = "deductible";
		dfr.name.description = "Point at which policy begins paying";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		deductible_field_id = schema.put_double_field(dfr);

		put_string_field_request sfr;
		sfr.name.name = "comment";
		sfr.name.description = "Descriptive text";
		sfr.options.length = 512;
		comment_field_id = schema.put_string_field(sfr);

		sfr.name.name = "program_name";
		sfr.name.description = "name of a program";
		sfr.options.length = 200;
		program_name_field_id = schema.put_string_field(sfr);

		sfr.name.name = "program_description";
		sfr.name.description = "name of a program";
		sfr.options.length = 512;
		program_description_field_id = schema.put_string_field(sfr);

		sfr.name.name = "coverage_name";
		sfr.name.description = "name of a coverage";
		sfr.options.length = 200;
		coverage_name_id = schema.put_string_field(sfr);

		sfr.name.name = "carrier_name";
		sfr.name.description = "name of a carrier";
		sfr.options.length = 200;
		carrier_name_id = schema.put_string_field(sfr);

		put_class_request pcr;

		pcr.class_name = "program";
		pcr.class_description = "program summary";
		pcr.member_fields = { "program_name", "program_description", "rectangle" };
		program_class_id = schema.put_class(pcr);

		if (program_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage";
		pcr.class_description = "coverage frame";
		pcr.member_fields = { "coverage_name", "comment", "rectangle" };
		coverage_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage_spacer";
		pcr.class_description = "spacer frame";
		pcr.member_fields = { "rectangle" };
		coverage_spacer_id = schema.put_class(pcr);

		if (coverage_spacer_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "carrier";
		pcr.class_description = "carrier frame";
		pcr.member_fields = { "carrier_name", "comment", "rectangle", "color" };
		carrier_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy";
		pcr.class_description = "policy block";
		pcr.member_fields = { "coverage_name", "carrier_name", "comment", "rectangle", "color", "limit", "attachment" };
		policy_class_id = schema.put_class(pcr);

		if (policy_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_deductible";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "coverage_name", "comment", "rectangle", "color", "deductible" };
		policy_deductible_class_id = schema.put_class(pcr);

		if (policy_deductible_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_umbrella";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "comment", "rectangle", "color", "limit", "attachment" };
		policy_umbrella_class_id = schema.put_class(pcr);

		if (policy_deductible_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		jmodel jm;

		jm.name = "program_chart";

		model_creatable_class* mcr;
		model_selectable_class* msr;
		model_updatable_class* mur;
		selector_rule* sr;

		mcr = jm.create_options.append();
		mcr->rule_name = "add coverage";
		mcr->selectors.always();
		mcr->create_class_id = coverage_class_id;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "add carrier";
		mcr->selectors.always();
		mcr->create_class_id = carrier_class_id;
		mcr->replace_selected = false;
		mcr->select_on_create = true;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "add coverage spacer";
		mcr->selectors.when(coverage_class_id);
		mcr->create_class_id = coverage_spacer_id;
		mcr->select_on_create = false;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "add policy";
		mcr->selectors.when(coverage_class_id, carrier_class_id);
		mcr->create_class_id = policy_class_id;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "add deductible";
		mcr->selectors.when(coverage_class_id);
		mcr->create_class_id = policy_deductible_class_id;
		mcr->select_on_create = true;
		mcr->replace_selected = false;
		mcr->item_id_class = null_row;

		mcr = jm.create_options.append();
		mcr->rule_name = "add umbrella";
		mcr->selectors.when(policy_class_id);
		mcr->create_class_id = policy_umbrella_class_id;
		mcr->select_on_create = true;
		mcr->replace_selected = true;
		mcr->item_id_class = null_row;

		msr = jm.select_options.append();
		msr->rule_name = "select coverage";
		msr->select_class_id = coverage_class_id;

		msr = jm.select_options.append();
		msr->rule_name = "select carrier";
		msr->select_class_id = carrier_class_id;

		msr = jm.select_options.append();
		msr->rule_name = "select coverage spacer";
		msr->select_class_id = coverage_spacer_id;

		msr = jm.select_options.append();
		msr->rule_name = "select policy";
		msr->select_class_id = policy_class_id;

		msr = jm.select_options.append();
		msr->rule_name = "select deductible";
		msr->select_class_id = policy_deductible_class_id;

		msr = jm.select_options.append();
		msr->rule_name = "select umbrella";
		msr->select_class_id = policy_umbrella_class_id;

		mur = jm.update_options.append();
		mur->rule_name = "update coverage";
		mur->update_class_id = coverage_class_id;

		mur = jm.update_options.append();
		mur->rule_name = "update carrier";
		mur->update_class_id = carrier_class_id;

		mur = jm.update_options.append();
		mur->rule_name = "update coverage spacer";
		mur->update_class_id = coverage_spacer_id;

		mur = jm.update_options.append();
		mur->rule_name = "update policy";
		mur->update_class_id = policy_class_id;

		mur = jm.update_options.append();
		mur->rule_name = "update deductible";
		mur->update_class_id = policy_deductible_class_id;

		mur = jm.update_options.append();
		mur->rule_name = "update umbrella";
		mur->update_class_id = policy_umbrella_class_id;

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

		sample_actor.actor_name = "sample actor";
		sample_actor.actor_id = null_row;
		sample_actor = program_chart.create_actor(sample_actor);
		enableEditMessages = true;
	}

	void wsproposal_controller::updateState(corona::database::actor_state& state, const rectDto& newSize)
	{
		pg.clear();

		auto mainr = pg.row(nullptr);
		auto controlcolumn = pg.column(mainr, { 0.0_px,0.0_px,25.0_pct,100.0_pct });

		auto d2dcolumn = pg.column(mainr, { 0.0_px,0.0_px,75.0_pct,100.0_pct });
		auto d2dwin = pg.canvas2d(d2dcolumn, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		corona::database::rectangle title_box, *ptitle_box = &title_box;

		auto left_margin = 20.0_px;
		auto chart_top = 10.0_px;

		title_box.w = 500.0;
		title_box.h = 100.0;
		title_box.x = left_margin.amount;
		title_box.y = chart_top.amount;

		//const database::actor_view_object& avo, database::jslice& slice
		for_each(state, program_class_id, [ptitle_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *ptitle_box;
			return true;
			});

		corona::database::rectangle legend_box, *plegend_box = &legend_box;

		legend_box.w = 300;
		legend_box.h = 20;
		legend_box.x = newSize.width - 300;
		legend_box.y = chart_top.amount;

		for_each(state, carrier_class_id, [plegend_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *plegend_box;
			plegend_box->y += 20;
			return true;
			});

		int coverage_count = state.view_objects.count_if([this](auto& avokp) { return avokp.second.class_id == this->coverage_class_id; });
		if (coverage_count < 3)
			coverage_count = 3;
		coverage_count += 2;
		int coverage_width = newSize.width / coverage_count;

		corona::database::rectangle coverage_box, *pcoverage_box = &coverage_box;

		coverage_box.w = coverage_width;
		coverage_box.h = 30;
		coverage_box.x = coverage_width;
		coverage_box.y = newSize.height - 30;

		for_each(state, coverage_class_id, [pcoverage_box, coverage_width](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *pcoverage_box;
			pcoverage_box->x += coverage_width;
			return true;
			});

		pg.actor_update_fields(controlcolumn, &state, &schema, &program_chart);
		pg.actor_create_buttons(controlcolumn, &state, &schema, &program_chart);
		pg.actor_select_items(d2dwin, &state, &schema, &program_chart);

		pg.arrange(newSize.width, newSize.height);

		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsproposal_controller::drawFrame()
	{
		;
	}

}

