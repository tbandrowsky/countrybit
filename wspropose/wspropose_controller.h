#pragma once

namespace proposal
{
	using namespace corona::win32;

	class wsproposal_controller : public corona_controller 
	{
	public:

		corona::database::relative_ptr_type idc_home;
		corona::database::relative_ptr_type idf_home;

		corona::database::relative_ptr_type idc_client_root;
		corona::database::relative_ptr_type idf_client_root;

		corona::database::relative_ptr_type idc_client;
		corona::database::relative_ptr_type idf_client;

		corona::database::relative_ptr_type idc_carrier_root;
		corona::database::relative_ptr_type idf_carrier_root;

		corona::database::relative_ptr_type idc_carrier;
		corona::database::relative_ptr_type idf_carrier;

		corona::database::relative_ptr_type idc_coverage_root;
		corona::database::relative_ptr_type idf_coverage_root;

		corona::database::relative_ptr_type idc_coverage;
		corona::database::relative_ptr_type idf_coverage;

		corona::database::relative_ptr_type idc_system_root;
		corona::database::relative_ptr_type idf_system_root;

		corona::database::relative_ptr_type idc_program_template_root;
		corona::database::relative_ptr_type idf_program_template_root;
		corona::database::relative_ptr_type idf_program_template;
		corona::database::relative_ptr_type idc_program_template;

		corona::database::relative_ptr_type idc_program;
		corona::database::relative_ptr_type idf_program;
		corona::database::relative_ptr_type idf_program_view;
		corona::database::relative_ptr_type idf_product;
		corona::database::relative_ptr_type idc_product;

		corona::database::relative_ptr_type idf_product_item_base;
		corona::database::relative_ptr_type idc_product_item_base;

		corona::database::relative_ptr_type idf_inception;
		corona::database::relative_ptr_type idf_expiration;
		corona::database::relative_ptr_type idf_status;
		corona::database::relative_ptr_type idf_attachment;
		corona::database::relative_ptr_type idf_limit;
		corona::database::relative_ptr_type idf_deductible;
		corona::database::relative_ptr_type idf_share;
		corona::database::relative_ptr_type idf_comment;
		corona::database::relative_ptr_type idf_program_style;

		corona::database::relative_ptr_type idf_pi_inception;
		corona::database::relative_ptr_type idf_pi_expiration;
		corona::database::relative_ptr_type idf_pi_status;
		corona::database::relative_ptr_type idf_pi_attachment;
		corona::database::relative_ptr_type idf_pi_limit;
		corona::database::relative_ptr_type idf_pi_deductible;
		corona::database::relative_ptr_type idf_pi_share;
		corona::database::relative_ptr_type idf_pi_coverage;
		corona::database::relative_ptr_type idf_pi_comment;

		corona::database::relative_ptr_type idc_pi_inception;
		corona::database::relative_ptr_type idc_pi_expiration;
		corona::database::relative_ptr_type idc_pi_status;
		corona::database::relative_ptr_type idc_pi_attachment;
		corona::database::relative_ptr_type idc_pi_limit;
		corona::database::relative_ptr_type idc_pi_deductible;
		corona::database::relative_ptr_type idc_pi_share;
		corona::database::relative_ptr_type idc_pi_coverage;
		corona::database::relative_ptr_type idc_pi_comment;

		corona::database::relative_ptr_type idf_program_title;
		corona::database::relative_ptr_type idf_program_subtitle;
		
		corona::database::relative_ptr_type idf_slide_title;

		corona::database::relative_ptr_type idc_program_chart_slide;
		corona::database::relative_ptr_type idf_program_chart_slide;
		corona::database::relative_ptr_type idc_program_chart_slide_product;
		corona::database::relative_ptr_type idf_program_chart_slide_product;

		corona::database::relative_ptr_type idc_program_generic_slide;
		corona::database::relative_ptr_type idf_program_generic_slide;

		corona::database::relative_ptr_type idf_comment;

		wsproposal_controller();
		virtual ~wsproposal_controller();

		void create_style_sheet();

		void render_home(page_item *_frame);
		void render_client(page_item* _frame);
		void render_product_template(page_item* _frame);
		void render_carrier(page_item* _frame);
		void render_program(page_item* _frame);

		virtual void render(const rectangle& newSize);
	};

}
