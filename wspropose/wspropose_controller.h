#pragma once

namespace proposal
{
	using namespace corona::win32;

	class wsproposal_controller : public corona_controller 
	{
	public:

		corona::database::relative_ptr_type idc_home;
		corona::database::relative_ptr_type idc_client_root;
		corona::database::relative_ptr_type idc_client;
		corona::database::relative_ptr_type idc_program;
		corona::database::relative_ptr_type idc_program_item;
		corona::database::relative_ptr_type idc_program_insurance;
		corona::database::relative_ptr_type idc_program_insurance_coverage;

		corona::database::relative_ptr_type idc_carrier_root;
		corona::database::relative_ptr_type idc_carrier;
		corona::database::relative_ptr_type idc_coverage_root;
		corona::database::relative_ptr_type idc_coverage;
		corona::database::relative_ptr_type idc_system_root;

		relative_ptr_type	id_canvas_header = 1;
		relative_ptr_type	id_canvas_navigation = 2;
		relative_ptr_type	id_canvas_footer = 3;
		relative_ptr_type	id_canvas_table = 4;

		view_query vq_navigation;
		view_query vq_carriers;
		view_query vq_clients;
		view_query vq_coverages;
		view_query vq_client;
		view_query vq_program;

		const char* application_title = "PROOFS OF CONCEPT";
		const char* application_author = "WOODRUFF SAWYER";

		wsproposal_controller();
		virtual ~wsproposal_controller();

		void set_style_sheets();
		void set_style_sheet(int _index);

		void render_navigation_frame(std::function<void(page_item* _navigation, page_item* _contents)> _contents);

		void render_home();
		void render_client_root();
		void render_client();
		void render_coverage_root();
		void render_coverage();
		void render_carrier_root();
		void render_system_root();
		void render_carrier();
		void render_program();

		void render_home_contents(page_item *_navigation, page_item* _contents);
		void render_client_root_contents(page_item* _navigation, page_item* _contents);
		void render_client_contents(page_item* _navigation, page_item* _contents);
		void render_program_contents(page_item* _navigation, page_item* _contents);
		void render_coverage_root_contents(page_item* _navigation, page_item* _contents);
		void render_coverage_contents(page_item* _navigation, page_item* _contents);
		void render_system_root_contents(page_item* _navigation, page_item* _contents);
		void render_carrier_root_contents(page_item* _navigation, page_item* _contents);
		void render_carrier_contents(page_item* _navigation, page_item* _contents);

		virtual void render(const rectangle& newSize);
	};

}
