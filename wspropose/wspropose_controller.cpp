
#pragma once


#include "corona.h"
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
		dtoFactory factory;
		auto dhost = host->getDrawable(0);

		factory
			.colorMake(0, 0, 0, 1)
			->solidBrushMake(dhost, TitleFill, false)
			->colorMake(0, 0, 0, 1)
			->solidBrushMake(dhost, SubtitleFill, false)
			->colorMake(.8, .8, .8, 1)
			->solidBrushMake(dhost, DisclaimerFill, false)
			->colorMake(0, 0, 0, 1)
			->solidBrushMake(dhost, PolicyFill, false)
			->colorMake(0, 0, 1, 1)
			->solidBrushMake(dhost, LegendFill, false);

		factory
			.textStyleMake(dhost, TitleText, "Arial", 30, false, false, 0, visual_alignment::align_center, visual_alignment::align_center, true)
			->textStyleMake(dhost, SubtitleText, "Arial", 20, false, false, 0, visual_alignment::align_center, visual_alignment::align_center, true)
			->textStyleMake(dhost, DisclaimerText, "Arial", 12, false, false, 0, visual_alignment::align_center, visual_alignment::align_center, true)
			->textStyleMake(dhost, PolicyText, "Arial", 14, false, false, 0, visual_alignment::align_center, visual_alignment::align_center, true)
			->textStyleMake(dhost, LegendText, "Arial", 14, false, false, 0, visual_alignment::align_center, visual_alignment::align_center, true);

	}

	void wsproposal_controller::onInit()
	{
		enableEditMessages = false;

		auto pos = host->getWindowPos(0);
		sizeIntDto curSize(pos.w, pos.h);
		host->setMinimumWindowSize(curSize);

		box.init(1 << 22);
		schema = jschema::create_schema(&box, 50, true, schema_id);

		put_double_field_request dfr;
		dfr.name.name = "limit";
		dfr.name.description = "Policy Limit";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		relative_ptr_type limit_field_id = schema.put_double_field(dfr);

		dfr.name.name = "attachment";
		dfr.name.description = "Attachment Point";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		attachment_field_id = schema.put_double_field(dfr);

		dfr.name.name = "deductible";
		dfr.name.description = "Deductible";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		deductible_field_id = schema.put_double_field(dfr);

		put_string_field_request sfr;
		sfr.name.name = "description";
		sfr.name.description = "Description";
		sfr.options.full_text_editor = true;
		sfr.options.length = 512;
		comment_field_id = schema.put_string_field(sfr);

		sfr.name.name = "program_name";
		sfr.name.description = "Program name";
		sfr.options.length = 200;
		program_name_field_id = schema.put_string_field(sfr);

		sfr.name.name = "coverage_name";
		sfr.name.description = "Coverage Name";
		sfr.options.length = 200;
		coverage_name_id = schema.put_string_field(sfr);

		sfr.name.name = "carrier_name";
		sfr.name.description = "Carrier Name";
		sfr.options.length = 200;
		carrier_name_id = schema.put_string_field(sfr);

		put_class_request pcr;

		pcr.class_name = "program";
		pcr.class_description = "Program summary";
		pcr.member_fields = { "program_name", "description", "rectangle", "layout_rect" };
		program_class_id = schema.put_class(pcr);

		if (program_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage";
		pcr.class_description = "coverage frame";
		pcr.member_fields = { "coverage_name", "description", "rectangle", "layout_rect" };
		coverage_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage_spacer";
		pcr.class_description = "spacer frame";
		pcr.member_fields = { "coverage_name", "rectangle", "layout_rect" };
		coverage_spacer_id = schema.put_class(pcr);

		if (coverage_spacer_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "carrier";
		pcr.class_description = "carrier frame";
		pcr.member_fields = { "carrier_name", "description", "rectangle", "layout_rect", "color" };
		carrier_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy";
		pcr.class_description = "policy block";
		pcr.member_fields = { "coverage_name", "carrier_name", "description", "rectangle", "layout_rect", "color", "limit", "attachment" };
		policy_class_id = schema.put_class(pcr);

		if (policy_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_deductible";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "coverage_name", "description", "rectangle", "layout_rect", "color", "deductible" };
		policy_deductible_class_id = schema.put_class(pcr);

		if (policy_deductible_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_umbrella";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "coverage_name", "description", "rectangle", "layout_rect", "color", "limit", "attachment" };
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
		msr->rule_name = "select program";
		msr->select_class_id = program_class_id;

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
		mur->rule_name = "update program";
		mur->update_class_id = program_class_id;

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

		relative_ptr_type program_summary_id = null_row;
		program_chart.create_object(0, sample_actor.actor_id, program_class_id, program_summary_id);

		enableEditMessages = true;
	}

	void wsproposal_controller::stateChanged(const rectDto& newSize)
	{
		pg.clear();

		auto mainr = pg.row(nullptr);
		auto controlcolumn = pg.column(mainr, { 0.0_px,0.0_px,25.0_pct,100.0_pct });

		auto d2dcolumn = pg.column(mainr, { 0.0_px,0.0_px,75.0_pct,100.0_pct });
		auto d2dwin = pg.canvas2d(d2dcolumn, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		corona::database::rectangle title_box, *ptitle_box = &title_box;
		corona::database::rectangle canvasRect = host->getDrawable(canvasWindowsId)->getCanvasSize();

		rectDto newDim = newSize;
		newDim.w -= newDim.x;
		newDim.x = 0;
		newDim.y -= newDim.y;
		newDim.y = 0;

		std::cout << std::format("canvas w:{} h:{}", canvasRect.w, canvasRect.h) << std::endl;
		std::cout << std::format("size w:{} h:{}", newDim.w, newDim.h) << std::endl;

		auto left_margin = 20.0_px;
		auto chart_top = 10.0_px;

		title_box.w = 500.0;
		title_box.h = 100.0;
		title_box.x = left_margin.amount;
		title_box.y = chart_top.amount;

		//const database::actor_view_object& avo, database::jslice& slice
		for_each(program_class_id, [ptitle_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *ptitle_box;
			return true;
			});

		corona::database::rectangle legend_box, *plegend_box = &legend_box;

		legend_box.w = 300;
		legend_box.h = 20;
		legend_box.x = canvasRect.w - 300;
		legend_box.y = chart_top.amount;

		for_each(carrier_class_id, [plegend_box](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
			auto rbx = slice.get_rectangle("rectangle");
			rbx = *plegend_box;
			plegend_box->y += 20;
			std::cout << std::format("carrier y:{} h:{}", plegend_box->y, plegend_box->h) << std::endl;
			return true;
			});

		// get dimensions of the chart, the width

		int coverage_count = state.view_objects.count_if([this](auto& avokp) { return avokp.second.class_id == this->coverage_class_id; });
		if (coverage_count < 3)
			coverage_count = 3;
		coverage_count += 2;
		int coverage_width = canvasRect.w / coverage_count;

		std::cout << std::format("coverages {} w:{}", coverage_count, coverage_width) << std::endl;

		// and the height

		corona::database::relative_ptr_type limit_fields[3], *plimit_fields = &limit_fields[0];
		limit_fields[0] = limit_field_id;
		limit_fields[1] = attachment_field_id;
		limit_fields[2] = null_row;

		double max_amount = 0.0, min_amount = 0.0, count = 0.0;
		double *pmax_amount = &max_amount, *pmin_amount = &min_amount, *pcount = &count;

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
				*pcount+= 1.0;
				pfield++;
			}
			return true;
			});

		corona::database::rectangle coverage_box, *pcoverage_box = &coverage_box;

		double chartMargin = -60;
		double chartHeight = newSize.h + chartMargin;
		double scaley = *pmax_amount / chartHeight;

		coverage_box.w = coverage_width;
		coverage_box.h = 30;
		coverage_box.x = coverage_width;
		coverage_box.y = canvasRect.h + chartMargin;

		corona::database::relative_ptr_type comparison_fields[2], *pcomparison_fields = &comparison_fields[0];
		comparison_fields[0] = coverage_name_id;
		comparison_fields[1] = null_row;

		double policyMax = 0.0;
		double policyMin = 0.0;

		for_each(coverage_class_id, [this, chartHeight, scaley, pcomparison_fields,pcoverage_box,coverage_width](const corona::database::actor_view_object& avo, corona::database::jslice& slice) {
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

		pg.actor_update_fields(controlcolumn, &state, &schema, &program_chart);
		pg.actor_create_buttons(controlcolumn, &state, &schema, &program_chart);
		pg.actor_select_items(d2dwin, &state, &schema, &program_chart);

		pg.arrange(newSize.w, newSize.h);

		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsproposal_controller::drawFrame()
	{
		corona::win32::dtoFactory factory;

		factory.colorMake(1, 1, 1, 1);
		host->getDrawable(0)->clear(&factory.color);

		auto drawables = pg.where([](const auto& pgi) { return pgi.item.is_drawable(); });

		for (auto pgi : drawables)
		{
			host->getDrawable(0)->drawRectangle(&pgi.item.bounds, viewStyle::BlackFill, 3, viewStyle::GreenFill);
			host->getDrawable(0)->drawText(pgi.item.caption ? pgi.item.caption : "empty", &pgi.item.bounds, viewStyle::H3Text, viewStyle::H3Fill);
		}
	}
}
