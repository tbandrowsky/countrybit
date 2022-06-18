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

		corona::database::relative_ptr_type idc_product_template_root;
		corona::database::relative_ptr_type idf_product_template_root;
		corona::database::relative_ptr_type idc_product_template;
		corona::database::relative_ptr_type idf_product_template;
		corona::database::relative_ptr_type idf_product_template_name;
		corona::database::relative_ptr_type idf_product_template_code;
		corona::database::relative_ptr_type idf_product_template_status;
		corona::database::relative_ptr_type idf_product_template_edition_start;
		corona::database::relative_ptr_type idf_product_template_edition_stop;
		corona::database::relative_ptr_type idf_product_template_type;
		corona::database::relative_ptr_type idf_product_template_line_of_business;
		corona::database::relative_ptr_type idf_product_template_carrier;
		corona::database::relative_ptr_type idf_product_template_updated_by;
		corona::database::relative_ptr_type idf_product_template_status_updated_by;

		corona::database::relative_ptr_type idf_product_template_product_header;
		corona::database::relative_ptr_type idf_product_template_product_structure;
		corona::database::relative_ptr_type idf_product_template_coverage_header;
		corona::database::relative_ptr_type idf_product_template_coverage_structure;

		corona::database::relative_ptr_type idc_product_template_product_header;
		corona::database::relative_ptr_type idc_product_template_product_structure;
		corona::database::relative_ptr_type idc_product_template_coverage_header;
		corona::database::relative_ptr_type idc_product_template_coverage_structure;			

		corona::database::relative_ptr_type idf_inception;
		corona::database::relative_ptr_type idf_expiration;
		corona::database::relative_ptr_type idf_status;
		corona::database::relative_ptr_type idf_attachment;
		corona::database::relative_ptr_type idf_limit;
		corona::database::relative_ptr_type idf_deductible;
		corona::database::relative_ptr_type idf_share;

		corona::database::relative_ptr_type idf_program_title;
		corona::database::relative_ptr_type idf_program_subtitle;
		corona::database::relative_ptr_type idc_program;
		corona::database::relative_ptr_type idf_program;
		corona::database::relative_ptr_type idf_program_view;

		corona::database::relative_ptr_type idc_program_product;
		corona::database::relative_ptr_type idf_program_product;
		corona::database::relative_ptr_type idc_product_instance;
		corona::database::relative_ptr_type idf_product_instance;
		
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

		void render_client_view(const rectangle& newSize);
		void render_carrier_view(const rectangle& newSize);
		void render_coverage_view(const rectangle& newSize);
		void render_program_view(const rectangle& newSize);

		virtual void render(const rectangle& newSize);
	};

}
