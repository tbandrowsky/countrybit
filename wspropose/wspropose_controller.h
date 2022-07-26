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

		corona::database::relative_ptr_type idc_product_root;
		corona::database::relative_ptr_type idf_product_root;
		corona::database::relative_ptr_type idf_product;
		corona::database::relative_ptr_type idc_product;

		corona::database::relative_ptr_type idf_product_name;
		corona::database::relative_ptr_type idf_product_code;
		corona::database::relative_ptr_type idf_product_status;
		corona::database::relative_ptr_type idf_product_edition;
		corona::database::relative_ptr_type idf_product_template_type;
		corona::database::relative_ptr_type idf_product_line_of_business;
		corona::database::relative_ptr_type idf_product_carrier;
		corona::database::relative_ptr_type idf_product_updated_by;
		corona::database::relative_ptr_type idf_product_updated_date;
		corona::database::relative_ptr_type idf_product_view;

		corona::database::relative_ptr_type idf_prompt;

		corona::database::relative_ptr_type idc_product_program_header;
		corona::database::relative_ptr_type idf_product_program_header;
		corona::database::relative_ptr_type idc_product_phi_base;
		corona::database::relative_ptr_type idf_product_phi_base;

		corona::database::relative_ptr_type idc_product_phi_fill;
		corona::database::relative_ptr_type idf_product_phi_fill;
		corona::database::relative_ptr_type idc_product_phi_carrier;
		corona::database::relative_ptr_type idf_product_phi_carrier;

		corona::database::relative_ptr_type idc_product_program_structure;
		corona::database::relative_ptr_type idf_product_program_structure;
		corona::database::relative_ptr_type idc_product_psi_base;
		corona::database::relative_ptr_type idf_product_psi_base;

		corona::database::relative_ptr_type idc_product_coverage_header;
		corona::database::relative_ptr_type idf_product_coverage_header;
		corona::database::relative_ptr_type idc_product_chi_base;
		corona::database::relative_ptr_type idf_product_chi_base;

		corona::database::relative_ptr_type idc_product_coverage_structure;
		corona::database::relative_ptr_type idf_product_coverage_structure;
		corona::database::relative_ptr_type idc_product_csi_base;
		corona::database::relative_ptr_type idf_product_csi_base;

		corona::database::relative_ptr_type idf_inception;
		corona::database::relative_ptr_type idf_expiration;
		corona::database::relative_ptr_type idf_status;
		corona::database::relative_ptr_type idf_attachment;
		corona::database::relative_ptr_type idf_limit;
		corona::database::relative_ptr_type idf_deductible;
		corona::database::relative_ptr_type idf_share;
		corona::database::relative_ptr_type idf_comment;
		corona::database::relative_ptr_type idf_carrier_name;
		corona::database::relative_ptr_type idf_coverage_name;

		relative_ptr_type	id_canvas_header = 1;
		relative_ptr_type	id_canvas_navigation = 2;
		relative_ptr_type	id_canvas_footer = 3;
		relative_ptr_type	id_canvas_form_table_a = 4;
		relative_ptr_type	id_canvas_products_a = 5;
		relative_ptr_type	id_canvas_products_b = 6;

		const char* application_title = "PROOFS OF CONCEPT";
		const char* application_author = "WOODRUFF SAWYER";

		wsproposal_controller();
		virtual ~wsproposal_controller();

		void set_style_sheet();

		void render_navigation_frame(std::function<void(page_item* _navigation, page_item* _contents)> _contents);
		void render_form(page_item* _navigation, page_item* _frame, const char *_form_title);
		void render_search_page(page_item* _navigation, page_item* _frame, relative_ptr_type _class_id, const char* _form_title, int count_fields, relative_ptr_type *_field_ids);

		void render_home();
		void render_client_root();
		void render_client();
		void render_coverage_root();
		void render_coverage();
		void render_product_root();
		void render_product();
		void render_carrier_root();
		void render_system_root();
		void render_carrier();
		void render_program();

		void render_home_contents(page_item *_navigation, page_item* _contents);
		void render_client_root_contents(page_item* _navigation, page_item* _contents);
		void render_client_contents(page_item* _navigation, page_item* _contents);
		void render_coverage_root_contents(page_item* _navigation, page_item* _contents);
		void render_system_root_contents(page_item* _navigation, page_item* _contents);
		void render_coverage_contents(page_item* _navigation, page_item* _contents);
		void render_product_root_contents(page_item* _navigation, page_item* _contents);
		void render_product_contents(page_item* _navigation, page_item* _contents);
		void render_carrier_root_contents(page_item* _navigation, page_item* _contents);
		void render_carrier_contents(page_item* _navigation, page_item* _contents);

		virtual void render(const rectangle& newSize);
	};

}
