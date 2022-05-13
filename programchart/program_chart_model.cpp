
#include "pch.h"


namespace app_model
{


	program_chart_model::program_chart_model()
	{

		try
		{
			box.init(1 << 22);
			cdb::relative_ptr_type schema_id;

			schema = cdb::jschema::create_schema(&box, 50, true, schema_id);

			cdb::put_double_field_request dfr;
			dfr.name.name = "limit";
			dfr.name.description = "Maximum amount paid by policy";
			dfr.name.type_id = cdb::jtype::type_float64;
			dfr.options.minimum_double = 0.0;
			dfr.options.maximum_double = 1E10;
			cdb::relative_ptr_type limit_field_id = schema.put_double_field(dfr);

			dfr.name.name = "attachment";
			dfr.name.description = "Point at which policy begins coverage";
			dfr.name.type_id = cdb::jtype::type_float64;
			dfr.options.minimum_double = 0.0;
			dfr.options.maximum_double = 1E10;
			cdb::relative_ptr_type attachment_field_id = schema.put_double_field(dfr);

			dfr.name.name = "deductible";
			dfr.name.description = "Point at which policy begins paying";
			dfr.name.type_id = cdb::jtype::type_float64;
			dfr.options.minimum_double = 0.0;
			dfr.options.maximum_double = 1E10;
			cdb::relative_ptr_type deductible_field_id = schema.put_double_field(dfr);

			cdb::put_string_field_request sfr;
			sfr.name.name = "comment";
			sfr.name.description = "Descriptive text";
			sfr.options.length = 512;
			cdb::relative_ptr_type comment_field_id = schema.put_string_field(sfr);

			sfr.name.name = "program_name";
			sfr.name.description = "name of a program";
			sfr.options.length = 200;
			cdb::relative_ptr_type program_name_field_id = schema.put_string_field(sfr);

			sfr.name.name = "program_description";
			sfr.name.description = "name of a program";
			sfr.options.length = 512;
			cdb::relative_ptr_type program_description_field_id = schema.put_string_field(sfr);

			sfr.name.name = "coverage_name";
			sfr.name.description = "name of a coverage";
			sfr.options.length = 200;
			cdb::relative_ptr_type coverage_name_id = schema.put_string_field(sfr);

			sfr.name.name = "carrier_name";
			sfr.name.description = "name of a carrier";
			sfr.options.length = 200;
			cdb::relative_ptr_type carrier_name_id = schema.put_string_field(sfr);

			cdb::put_class_request pcr;

			pcr.class_name = "program";
			pcr.class_description = "program summary";
			pcr.member_fields = { "program_name", "program_description" };
			cdb::relative_ptr_type program_class_id = schema.put_class(pcr);

			if (program_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "coverage";
			pcr.class_description = "coverage frame";
			pcr.member_fields = { "coverage_name", "comment", "rectangle" };
			cdb::relative_ptr_type coverage_class_id = schema.put_class(pcr);

			if (coverage_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "coverage_spacer";
			pcr.class_description = "spacer frame";
			pcr.member_fields = { "rectangle" };
			cdb::relative_ptr_type coverage_spacer_id = schema.put_class(pcr);

			if (coverage_spacer_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "carrier";
			pcr.class_description = "carrier frame";
			pcr.member_fields = { "carrier_name", "comment", "rectangle", "color" };
			cdb::relative_ptr_type carrier_class_id = schema.put_class(pcr);

			if (coverage_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "policy";
			pcr.class_description = "policy block";
			pcr.member_fields = { "coverage_name", "carrier_name", "comment", "rectangle", "color", "limit", "attachment" };
			cdb::relative_ptr_type policy_class_id = schema.put_class(pcr);

			if (policy_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "policy_deductible";
			pcr.class_description = "deductible block";
			pcr.member_fields = { "coverage_name", "comment", "rectangle", "color", "deductible" };
			cdb::relative_ptr_type policy_deductible_class_id = schema.put_class(pcr);

			if (policy_deductible_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			pcr.class_name = "policy_umbrella";
			pcr.class_description = "deductible block";
			pcr.member_fields = { "comment", "rectangle", "color", "limit", "attachment" };
			cdb::relative_ptr_type policy_umbrella_class_id = schema.put_class(pcr);

			if (policy_deductible_class_id == cdb::null_row) {
				set_status("class create failed", false);
				return;
			}

			model.name = "program_chart";

			cdb::model_creatable_class* mcr;
			cdb::model_selectable_class* msr;
			cdb::model_updatable_class* mur;
			cdb::selector_rule* sr;

			mcr = model.create_options.append();
			mcr->rule_name = "add coverage";
			mcr->selectors.always();
			mcr->create_class_id = coverage_class_id;
			mcr->replace_selected = false;
			mcr->select_on_create = true;
			mcr->item_id_class = cdb::null_row;

			mcr = model.create_options.append();
			mcr->rule_name = "add carrier";
			mcr->selectors.always();
			mcr->create_class_id = carrier_class_id;
			mcr->replace_selected = false;
			mcr->select_on_create = true;
			mcr->item_id_class = cdb::null_row;

			mcr = model.create_options.append();
			mcr->rule_name = "add coverage spacer";
			mcr->selectors.when(coverage_class_id);
			mcr->create_class_id = coverage_spacer_id;
			mcr->select_on_create = false;
			mcr->replace_selected = false;
			mcr->item_id_class = cdb::null_row;

			mcr = model.create_options.append();
			mcr->rule_name = "add policy";
			mcr->selectors.when(coverage_class_id, carrier_class_id);
			mcr->create_class_id = policy_class_id;
			mcr->select_on_create = true;
			mcr->replace_selected = false;
			mcr->item_id_class = cdb::null_row;

			mcr = model.create_options.append();
			mcr->rule_name = "add deductible";
			mcr->selectors.when(coverage_class_id);
			mcr->create_class_id = policy_deductible_class_id;
			mcr->select_on_create = true;
			mcr->replace_selected = false;
			mcr->item_id_class = cdb::null_row;

			mcr = model.create_options.append();
			mcr->rule_name = "add umbrella";
			mcr->selectors.when(policy_class_id);
			mcr->create_class_id = policy_umbrella_class_id;
			mcr->select_on_create = true;
			mcr->replace_selected = true;
			mcr->item_id_class = cdb::null_row;

			msr = model.select_options.append();
			msr->rule_name = "select coverage";
			msr->select_class_id = coverage_class_id;

			msr = model.select_options.append();
			msr->rule_name = "select carrier";
			msr->select_class_id = carrier_class_id;

			msr = model.select_options.append();
			msr->rule_name = "select coverage spacer";
			msr->select_class_id = coverage_spacer_id;

			msr = model.select_options.append();
			msr->rule_name = "select policy";
			msr->select_class_id = policy_class_id;

			msr = model.select_options.append();
			msr->rule_name = "select deductible";
			msr->select_class_id = policy_deductible_class_id;

			msr = model.select_options.append();
			msr->rule_name = "select umbrella";
			msr->select_class_id = policy_umbrella_class_id;

			mur = model.update_options.append();
			mur->rule_name = "update coverage";
			mur->update_class_id = coverage_class_id;

			mur = model.update_options.append();
			mur->rule_name = "update carrier";
			mur->update_class_id = carrier_class_id;

			mur = model.update_options.append();
			mur->rule_name = "update coverage spacer";
			mur->update_class_id = coverage_spacer_id;

			mur = model.update_options.append();
			mur->rule_name = "update policy";
			mur->update_class_id = policy_class_id;

			mur = model.update_options.append();
			mur->rule_name = "update deductible";
			mur->update_class_id = policy_deductible_class_id;

			mur = model.update_options.append();
			mur->rule_name = "update umbrella";
			mur->update_class_id = policy_umbrella_class_id;

			schema.put_model(model);

			cdb::jcollection_ref ref;
			ref.data = &box;
			ref.model_name = model.name;
			ref.max_actors = 2;
			ref.max_objects = 1000;
			ref.collection_size_bytes = 1 << 21;

			if (!init_collection_id(ref.collection_id))
			{				
				set_status( "collection id failed", false );
			}

			program_chart = schema.create_collection(&ref);

			set_status("ready", true);

		}
		catch (std::exception& exc)
		{
			set_status(exc.what(), false);
		}
	}

	void program_chart_model::set_status(const char* _status, bool _ready)
	{
		status = _status;
		ready = _ready;
	}

	std::string program_chart_model::get_status()
	{
		return status;
	}

	bool program_chart_model::get_ready()
	{
		return ready;
	}

}
