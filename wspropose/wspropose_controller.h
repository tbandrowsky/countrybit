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

		inline static const  char* TitleText = "wspTitleText";
		inline static const  char* SubtitleText = "wspSubtitleText";
		inline static const  char* DisclaimerText = "wspDisclaimerText";

		inline static const  char* TitleFill = "wspTitleFill";
		inline static const  char* SubtitleFill = "wspSubtitleFill";
		inline static const  char* DisclaimerFill = "wspDisclaimerFill";

		inline static const  char* PolicyText = "wspPolicyText";
		inline static const  char* PolicyFill = "wspPolicyFill";

		inline static const  char* CoverageText = "wspCoverageText";
		inline static const  char* CoverageFill = "wspCoverageFill";

		inline static const  char* LegendText = "wspLegendText";
		inline static const  char* LegendFill = "wspLegendFill";

		wsproposal_controller(viewStyle* _vs);
		virtual ~wsproposal_controller();

		virtual void stateChanged(const rectDto& newSize);
		virtual void loadController();
		virtual void onInit();
		virtual void drawFrame();
	};

}
