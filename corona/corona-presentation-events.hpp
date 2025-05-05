/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
Defines events 

Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_EVENTS_H
#define CORONA_PRESENTATION_EVENTS_H

namespace corona {

	class control_base;

	class page;

	class page_event
	{
	public:
		std::shared_ptr<page> pg;
	};

	class page_select_event : public page_event
	{
	public:

	};

	class page_load_event : public page_event
	{
	public:

	};

	class page_unload_event : public page_event
	{
	public:

	};

	class control_event
	{
	public:
		int control_id;
		control_base* control;
		comm_bus_app_interface* bus;
	};

	class command_event : public control_event
	{
	public:
	};

	class mouse_event : public control_event
	{
	public:
		point absolute_point;
		point relative_point;
	};

	class mouse_move_event : public mouse_event
	{

	};

	class mouse_click_event : public mouse_event
	{

	};

	class mouse_left_click_event : public mouse_event
	{

	};

	class mouse_right_click_event : public mouse_event
	{

	};

	class key_event : public control_event
	{
	public:
		short key;
	};

	class key_down_event : public key_event
	{
	public:

	};

	class key_up_event : public key_event
	{
	public:

	};

	class key_press_event : public key_event
	{
	public:

	};

	class draw_event : public control_event
	{
	public:
		int state;
	};

	class item_changed_event : public control_event
	{
	public:
		std::string text_value;
	};

	class list_changed_event : public control_event
	{
	public:
		int state;

		std::string selected_text;
		int selected_index;
		relative_ptr_type selected_value;
	};

	class key_up_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(key_up_event) > on_key_up;
	};

	class key_down_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(key_down_event) > on_key_down;
	};
	 
	class key_press_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(key_press_event) > on_key_press;
	};

	class mouse_move_event_binding
	{
	public:
		int subscribed_item_id;
		control_base *control;
		std::function< void(mouse_move_event) > on_mouse_move;
	};

	class mouse_click_event_binding
	{
	public:
		int subscribed_item_id;
		control_base* control;
		std::function< void(mouse_click_event) > on_mouse_click;
	};

	class mouse_left_click_event_binding
	{
	public:
		int subscribed_item_id;
		control_base* control;
		std::function< void(mouse_left_click_event) > on_mouse_left_click;
	};

	class mouse_right_click_event_binding
	{
	public:
		int subscribed_item_id;
		control_base* control;
		std::function< void(mouse_right_click_event) > on_mouse_right_click;
	};

	class draw_event_binding
	{
	public:
		int subscribed_item_id;
		control_base* control;
		std::function< void(draw_event) > on_draw;
	};

	class item_changed_event_binding
	{
	public:
		int subscribed_item_id;
		control_base* control;
		std::function< void(item_changed_event) > on_change;
	};

	class command_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(command_event) > on_command;
	};

	class list_changed_event_binding
	{
	public:
		int subscribed_item_id;
		std::weak_ptr<control_base> control;
		std::function< void(list_changed_event) > on_change;
	};

	class page_select_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(page_select_event) > on_select;
	};

	class page_load_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(page_load_event) > on_load;
	};

	class page_unload_event_binding
	{
	public:
		int subscribed_item_id;
		std::function< void(page_unload_event) > on_unload;
	};

	class page_base {
	public:

		std::shared_ptr<control_base> root;
		std::string name;

		virtual void on_key_up(int _control_id, std::function< void(key_up_event) >) = 0;
		virtual void on_key_down(int _control_id, std::function< void(key_down_event) >) = 0;
		virtual void on_key_press(int _control_id, std::function< void(key_press_event) >) = 0;
		virtual void on_mouse_move(control_base *_base, std::function< void(mouse_move_event) >) = 0;
		virtual void on_mouse_click(control_base* _base, std::function< void(mouse_click_event) >) = 0;
		virtual void on_mouse_left_click(control_base* _base, std::function< void(mouse_left_click_event) >) = 0;
		virtual void on_mouse_right_click(control_base* _base, std::function< void(mouse_right_click_event) >) = 0;
		virtual void on_item_changed(int _control_id, std::function< void(item_changed_event) >) = 0;
		virtual void on_list_changed(int _control_id, std::function< void(list_changed_event) >) = 0;
		virtual void on_command(int _item_id, std::function< void(command_event) >) = 0;
		virtual void on_select(std::function< void(page_select_event) >) = 0;
		virtual void on_load(std::function< void(page_load_event) >) = 0;
		virtual void on_unload(std::function< void(page_unload_event) >) = 0;
		virtual void clear_events(int _item_id) = 0;

	};
}

#endif
