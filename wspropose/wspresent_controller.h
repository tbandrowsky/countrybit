#pragma once

namespace proposal
{
	using namespace corona::win32;

	class wspresent_controller : public corona_controller
	{
	public:

		corona::database::relative_ptr_type idc_presentations_root;
		corona::database::relative_ptr_type idf_presentations_root;

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

		view_query vq_presentations;
	
		wspresent_controller();
		virtual ~wspresent_controller();

		void set_style_sheet();

		void render_navigation_frame(std::function<void(page_item* _navigation, page_item* _contents)> _contents);
		void render_form(page_item* _navigation, page_item* _frame, const char* _form_title);
		void render_search_page(page_item* _navigation, page_item* _frame, relative_ptr_type _class_id, const char* _form_title, int count_fields, relative_ptr_type* _field_ids);

		virtual void render(const rectangle& newSize);
	};

}
