#pragma once

namespace bridges
{
	using namespace corona::win32;


	/*
	* 
	* Bridges is an application that allows for people to participate in local 
	* politics with an eye towards inculcating candidates for state and other 
	* local representatives, and to reminding people of the intersectionality of 
	* freedom.
	* 
	* It shows a selection of states,
	* then, districts in each state.
	* Within each district,
	* people and places and stories may be banked
	* and all users can communicate
	* districts have links to local news, local attractions, businesses, and resources
	* along with property information, etc.

	*/

	class bridges_controller : public corona_controller
	{
	public:

		corona::database::relative_ptr_type idc_login;
		corona::database::relative_ptr_type idf_login;

		corona::database::relative_ptr_type idc_registration;
		corona::database::relative_ptr_type idf_registration;

		corona::database::relative_ptr_type idc_password_reminder;
		corona::database::relative_ptr_type idc_password_reminder;

		corona::database::relative_ptr_type idc_home;
		corona::database::relative_ptr_type idf_home;

		corona::database::relative_ptr_type idc_inventory;
		corona::database::relative_ptr_type idf_inventory;

		corona::database::relative_ptr_type idc_world;
		corona::database::relative_ptr_type idf_world;

		const char* application_title = "Flowers and Beautiful Things";

		bridges_controller();
		virtual ~bridges_controller();

		void set_style_sheet();

		void render_header(page_item* _frame, const char* _title, const char* _subtitle, bool _left_pad);
		void render_form(std::function<void(page_item* _frame)> _contents);
		void render_2d(std::function<void(page_item* _frame)> _contents);
		void render_mixed(std::function<void(page_item* _frame)> _contents);

		void render_login();
		void render_registration();
		void render_password_reminder();
		void render_home();
		void render_inventory();
		void render_world();

		void render_login_contents(page_item* _frame);
		void render_registration_contents(page_item* _frame);
		void render_password_reminder_contents(page_item* _frame);
		void render_inventory_contents(page_item* _frame);
		void render_world(page_item* _frame);

		virtual void render(const rectangle& newSize);
	};

}
