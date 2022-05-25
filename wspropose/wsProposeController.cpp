
#pragma once


#include "pch.h"
#include "resource.h"
#include "wsProposeController.h"

const int IDC_DIRECT2D = -1;

namespace proposal
{

	using namespace corona::database;

	wsProposalController::wsProposalController(viewStyle* _vs) :
		controller(_vs),
		showUpdate(false),
		currentWindowView(currentWindowViews::viewEmpty),
		previewMode(false),
		magnification(100)
	{
		;
	}

	wsProposalController::~wsProposalController()
	{
		;
	}

	void wsProposalController::loadController()
	{
		;
	}

	void wsProposalController::onInit()
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
		relative_ptr_type attachment_field_id = schema.put_double_field(dfr);

		dfr.name.name = "deductible";
		dfr.name.description = "Point at which policy begins paying";
		dfr.name.type_id = jtype::type_float64;
		dfr.options.minimum_double = 0.0;
		dfr.options.maximum_double = 1E10;
		relative_ptr_type deductible_field_id = schema.put_double_field(dfr);

		put_string_field_request sfr;
		sfr.name.name = "comment";
		sfr.name.description = "Descriptive text";
		sfr.options.length = 512;
		relative_ptr_type comment_field_id = schema.put_string_field(sfr);

		sfr.name.name = "program_name";
		sfr.name.description = "name of a program";
		sfr.options.length = 200;
		relative_ptr_type program_name_field_id = schema.put_string_field(sfr);

		sfr.name.name = "program_description";
		sfr.name.description = "name of a program";
		sfr.options.length = 512;
		relative_ptr_type program_description_field_id = schema.put_string_field(sfr);

		sfr.name.name = "coverage_name";
		sfr.name.description = "name of a coverage";
		sfr.options.length = 200;
		relative_ptr_type coverage_name_id = schema.put_string_field(sfr);

		sfr.name.name = "carrier_name";
		sfr.name.description = "name of a carrier";
		sfr.options.length = 200;
		relative_ptr_type carrier_name_id = schema.put_string_field(sfr);

		put_class_request pcr;

		pcr.class_name = "program";
		pcr.class_description = "program summary";
		pcr.member_fields = { "program_name", "program_description" };
		relative_ptr_type program_class_id = schema.put_class(pcr);

		if (program_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage";
		pcr.class_description = "coverage frame";
		pcr.member_fields = { "coverage_name", "comment", "rectangle" };
		relative_ptr_type coverage_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "coverage_spacer";
		pcr.class_description = "spacer frame";
		pcr.member_fields = { "rectangle" };
		relative_ptr_type coverage_spacer_id = schema.put_class(pcr);

		if (coverage_spacer_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "carrier";
		pcr.class_description = "carrier frame";
		pcr.member_fields = { "carrier_name", "comment", "rectangle", "color" };
		relative_ptr_type carrier_class_id = schema.put_class(pcr);

		if (coverage_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy";
		pcr.class_description = "policy block";
		pcr.member_fields = { "coverage_name", "carrier_name", "comment", "rectangle", "color", "limit", "attachment" };
		relative_ptr_type policy_class_id = schema.put_class(pcr);

		if (policy_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_deductible";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "coverage_name", "comment", "rectangle", "color", "deductible" };
		relative_ptr_type policy_deductible_class_id = schema.put_class(pcr);

		if (policy_deductible_class_id == null_row) {
			std::cout << __LINE__ << ":class create failed failed" << std::endl;
			return;
		}

		pcr.class_name = "policy_umbrella";
		pcr.class_description = "deductible block";
		pcr.member_fields = { "comment", "rectangle", "color", "limit", "attachment" };
		relative_ptr_type policy_umbrella_class_id = schema.put_class(pcr);

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

	void wsProposalController::updateState(corona::database::actor_state& state, const rectDto& newSize)
	{
		corona::database::page pg;

		auto mainr = pg.row(nullptr);
		auto controlcolumn = pg.column(mainr, { 0.0px,0.0px,25.0pct,100.0pct });



		auto d2dcolumn = pg.column(mainr, { 0.0px,0.0px,75.0pct,100.0pct });
		auto d2dwin = pg.canvas2d(d2dcolumn, { 0.0px,0.0px,100.0pct,100.0pct });

		if (state.modified_object_id != null_row) 
		{
			pg.slice(controlcolumn, state.modified_object_id, state.modified_object);
		}
		pg.arrange(newSize.width, newSize.height);

		canvasWindowsId = host->renderPage(pg, &schema, state, program_chart);
		host->redraw();
	}

	void wsProposalController::onCreated(const rectDto& newSize)
	{
		auto state = this->program_chart.get_actor_state(this->sample_actor.actor_id);
		updateState(state, newSize);
	}

	void wsProposalController::randomAdvertisement()
	{
	}

	void wsProposalController::keyDown(short _key)
	{
		;
	}

	void wsProposalController::keyUp(short _key)
	{
		;
	}

	void wsProposalController::mouseMove(pointDto* _point)
	{
		;
	}

	int wsProposalController::onHScroll(int controlId, scrollTypes scrollType)
	{
		if (controlId != canvasWindowsId)
			return 0;

		rectDto r = host->getWindowPos(controlId);
		int pos = host->getScrollPos(controlId).x;
		int max = host->getScrollRange(controlId).width;

		switch (scrollType) {
		case ScrollPageUp:
			pos -= r.width;
			if (pos < 0) pos = 0;
			break;
		case ScrollPageDown:
			pos += r.width;
			if (pos > max) pos = max;
			break;
		case ScrollLineUp:
			pos -= 16;
			if (pos < 0) pos = 0;
			break;
		case ScrollLineDown:
			pos += 16;
			if (pos > max) pos = max;
			break;
		case ThumbTrack:
			pos = host->getScrollTrackPos(controlId).x;
			break;
		}

		currentScroll.x = pos;
		host->redraw();

		return pos;
	}

	int wsProposalController::onVScroll(int controlId, scrollTypes scrollType)
	{
		if (controlId != canvasWindowsId)
			return 0;

		rectDto r = host->getWindowPos(controlId);
		int pos = host->getScrollPos(controlId).y;
		int max = host->getScrollRange(controlId).height;

		switch (scrollType) {
		case ScrollPageUp:
			pos -= r.height;
			if (pos < 0) pos = 0;
			break;
		case ScrollPageDown:
			pos += r.height;
			if (pos > max) pos = max;
			break;
		case ScrollLineUp:
			pos -= 16;
			if (pos < 0) pos = 0;
			break;
		case ScrollLineDown:
			pos += 16;
			if (pos > max) pos = max;
			break;
		case ThumbTrack:
			pos = host->getScrollTrackPos(controlId).y;
			break;
		}

		currentScroll.y = pos;
		host->redraw();

		return pos;
	}

	int wsProposalController::onSpin(int controlId, int newPosition)
	{
		double newPositionF = newPosition / 100.0;
		return 0;
	}

	int wsProposalController::onResize(const rectDto& newSize)
	{
		rectDto r = host->getWindowPos(canvasWindowsId);

		r.width = newSize.width - (r.left + 32);
		r.height = newSize.height - (r.top + 32);

		host->setWindowPos(canvasWindowsId, r);

		setScrollBars();

		auto result = program_chart.get_actor_state(sample_actor.actor_id, null_row, "state");
		updateState(result, newSize);

		return 0;
	}

	void wsProposalController::setScrollBars()
	{
	}

	void wsProposalController::mouseClick(pointDto* _point)
	{
	}

	void wsProposalController::drawFrame()
	{
		try
		{

			auto frameLayout = host->getWindowPos(canvasWindowsId);
			frameLayout.left = 0.0;
			frameLayout.top = 0.0;
			float f = 192.0 / 255.0;
			float paperMargin = 8.0;
			float shadowSize = 4.0;
			dtoFactory fact;

			switch (currentWindowView) {
				case currentWindowViews::viewEmpty:
				{
					int mx = 100, my = 80;
					fact.colorMake(f, f, f, 1.0);
					host->getDrawable(0)->clear(&fact.color);

					pathImmediateDto pim;
					fact.rectangleMake(&pim, (frameLayout.width - mx) / 2.0 + shadowSize, (frameLayout.height - my) / 2.0 + shadowSize, mx, my, "back", style->getBlackWashBrushName(), NULL, 0.0);
					host->getDrawable(0)->drawPath(&pim);
					fact.rectangleMake(&pim, (frameLayout.width - mx) / 2.0, (frameLayout.height - my) / 2.0, mx, my, "border", style->getWhiteBrushName(), style->getGreyBrushName(), 1.0);
					host->getDrawable(0)->drawPath(&pim);

					textInstance2dDto txt;
					getH1Styles(&txt);
					txt.position.x = 100;
					txt.position.y = 100;
					txt.rotation = 0.0;
					txt.text = "Carrier 1.";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);

					getH2Styles(&txt);
					txt.position.x = 100;
					txt.position.y = 200;
					txt.rotation = 0.0;
					txt.text = "Carrier 2.";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);

					getH3Styles(&txt);
					txt.position.x = 100;
					txt.position.y = 300;
					txt.rotation = 0.0;
					txt.text = "Carrier 3.";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);

					this->getLabelStyles(&txt);
					txt.position.x = 100;
					txt.position.y = 400;
					txt.rotation = 0.0;
					txt.text = "Add Carrier";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);

					
				}
				break;
			}

		}
		catch (std::exception exc)
		{
			;
		}
	}

	void wsProposalController::exportBitmap(const char* _filenameImage)
	{
		dtoFactory fact;

		sizeIntDto dt = { 100, 100 };
		auto bm = host->getDrawable(0)->createBitmap(dt);

		colorDto color;
		color.alpha = color.red = color.green = color.blue = 0.0;

		bm->beginDraw();
		bm->clear(&color);

		bm->endDraw();

		bm->save(_filenameImage);

		delete bm;
	}

	void wsProposalController::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
	{
		;
	}

	bool wsProposalController::update(double _elapsedSeconds, double _totalSeconds)
	{
		bool oldShowUpdate = showUpdate;
		showUpdate = false;
		return oldShowUpdate;
	}

	void wsProposalController::onCommand(int buttonId)
	{

		switch (buttonId) {
		}
		host->redraw();

	}

	void wsProposalController::onTextChanged(int textControlId)
	{
	}

	void wsProposalController::onDropDownChanged(int dropDownId)
	{
	}

	void wsProposalController::onListViewChanged(int listViewId)
	{
	}

	void wsProposalController::fromImage()
	{
		std::string temp;
		bool tempB;
		int tempI;

		enableEditMessages = false;

		enableEditMessages = true;
	}

	void wsProposalController::pointSelected(pointDto* _point, colorDto* _color)
	{

	}

	void wsProposalController::clearErrors(errorDto* _error)
	{
		;
	}

	void wsProposalController::addError(errorDto* _error)
	{
		;
	}
}

