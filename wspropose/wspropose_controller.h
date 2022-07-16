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
		corona::database::relative_ptr_type idf_program_item_base;
		corona::database::relative_ptr_type idc_program_item_base;

		corona::database::relative_ptr_type idf_program_feature1;
		corona::database::relative_ptr_type idf_program_feature2;
		corona::database::relative_ptr_type idc_program_feature1;
		corona::database::relative_ptr_type idc_program_feature2;

		corona::database::relative_ptr_type idf_product;
		corona::database::relative_ptr_type idc_product;
		corona::database::relative_ptr_type idf_product_item_base;
		corona::database::relative_ptr_type idc_product_item_base;

		corona::database::relative_ptr_type idf_product_feature1;
		corona::database::relative_ptr_type idf_product_feature2;
		corona::database::relative_ptr_type idc_product_feature1;
		corona::database::relative_ptr_type idc_product_feature2;

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
		corona::database::relative_ptr_type idf_feature1_text;
		corona::database::relative_ptr_type idf_feature2_text;

		corona::database::relative_ptr_type idf_program_title;
		corona::database::relative_ptr_type idf_program_subtitle;
		
		corona::database::relative_ptr_type idf_slide_title;

		corona::database::relative_ptr_type idc_program_chart_slide;
		corona::database::relative_ptr_type idf_program_chart_slide;
		corona::database::relative_ptr_type idc_program_chart_slide_product;
		corona::database::relative_ptr_type idf_program_chart_slide_product;
		corona::database::relative_ptr_type idc_program_generic_slide;
		corona::database::relative_ptr_type idf_program_generic_slide;

		relative_ptr_type	id_home,
							id_carrier_root, 
							id_coverage_root, 
							id_client_root, 
							id_product_template_root, 
							id_system_root;

		relative_ptr_type	id_canvas_header = 1;
		relative_ptr_type	id_canvas_navigation = 2;
		relative_ptr_type	id_canvas_footer = 3;
		relative_ptr_type	id_canvas_form_table_a = 4;

		const char* application_title = "PROOFS OF CONCEPT";
		const char* application_author = "WOODRUFF SAWYER";

		wsproposal_controller();
		virtual ~wsproposal_controller();

		void set_style_sheet();

		void render_navigation_frame(const char* _title, const char* _subtitle, bool _left_pad, std::function<void(page_item* _frame)> _contents);
		void render_form(page_item* _frame, const char *_form_title);

		void render_home();
		void render_client_root();
		void render_client();
		void render_coverage_root();
		void render_coverage();
		void render_product_template_root();
		void render_product_template();
		void render_carrier_root();
		void render_system_root();
		void render_carrier();
		void render_program();

		void render_home_contents(page_item* _frame);
		void render_client_root_contents(page_item* _frame);
		void render_client_contents(page_item* _frame);
		void render_coverage_root_contents(page_item* _frame);
		void render_system_root_contents(page_item* _frame);
		void render_coverage_contents(page_item* _frame);
		void render_product_template_root_contents(page_item* _frame);
		void render_product_template_contents(page_item* _frame);
		void render_carrier_root_contents(page_item* _frame);
		void render_carrier_contents(page_item* _frame);
		void render_program_contents(page_item* _frame);
		void render_program_slide(page_item* _frame, actor_view_object& _slide, layout_rect* _layout);
		void render_program_chart_slide(page_item* _frame, actor_view_object& _slide, layout_rect* _layout);
		void render_program_generic_slide(page_item* _frame, actor_view_object& _slide, layout_rect* _layout);

		virtual void render(const rectangle& newSize);
	};

}
