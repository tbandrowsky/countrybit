#pragma once

namespace proposal
{
	using namespace corona::win32;

	class wsproposal_controller : public corona_controller 
	{
	public:

		corona::database::relative_ptr_type limit_field_id;
		corona::database::relative_ptr_type attachment_field_id;
		corona::database::relative_ptr_type deductible_field_id;
		corona::database::relative_ptr_type comment_field_id;
		corona::database::relative_ptr_type program_name_field_id;
		corona::database::relative_ptr_type program_description_field_id;
		corona::database::relative_ptr_type coverage_name_id;
		corona::database::relative_ptr_type carrier_name_id;

		corona::database::relative_ptr_type program_class_id;
		corona::database::relative_ptr_type coverage_class_id;
		corona::database::relative_ptr_type coverage_spacer_id;
		corona::database::relative_ptr_type carrier_class_id;
		corona::database::relative_ptr_type policy_class_id;
		corona::database::relative_ptr_type policy_deductible_class_id;
		corona::database::relative_ptr_type policy_umbrella_class_id;

		wsproposal_controller(viewStyle* _vs);
		virtual ~wsproposal_controller();

		virtual void stateChanged(const rectDto& newSize);
		virtual void loadController();
		virtual void onInit();
		virtual void drawFrame();
	};

}
