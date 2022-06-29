#pragma once

namespace flowers
{
	using namespace corona::win32;


	/*
	
	Flowers and Beautiful Things is a Co-Op Building Game
	It's a bit like Minecraft, except, people all play in one world.

	There is only one world, and everyone plays in it.

	In the world, each has a garden spot. 
	The garden spot can be as big as you want,
	and it doesn't take from anyone else,
	but, you can only make it as big as you build on it.

	There's some interplay between the kind of flower and thing you place, 
	and the music that it makes together.

	You can't hurt anything anyone else builds.

	Everyone's Avatar is a soul,
	and their souls are beautiful lights.

	People can send signs to each other,
	and be supportive, and give things to each other,
	but there's no chat per se.

	The thing is to have this shared national experience, where everyone is just together,
	doing happy things, making beautiful things for themselves,
	and for each other.

	Pot is recommended, but not required.
	
	*/

	class flowers_controller : public corona_controller
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

		flowers_controller();
		virtual ~flowers_controller();

		void set_style_sheet();

		void render_header(page_item* _frame, const char* _title, const char* _subtitle, bool _left_pad);
		void render_form(std::function<void(page_item* _frame)> _contents);
		void render_search(std::function<void(page_item* _frame)> _contents);
		void render_visual(std::function<void(page_item* _frame)> _contents);

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
