#ifndef CORONA_PRESENTATION_PAGE_H
#define CORONA_PRESENTATION_PAGE_H

namespace corona
{

	using update_function = std::function< void(page* _page, double _elapsedSeconds, double _totalSeconds) >;
	using change_function = std::function< void(page& _page, std::string _changed_set_name) >;

	class page : public page_base
	{

		std::map<int, std::shared_ptr<key_up_event_binding> > key_up_bindings;
		std::map<int, std::shared_ptr<key_down_event_binding> > key_down_bindings;
		std::map<int, std::shared_ptr<mouse_move_event_binding> > mouse_move_bindings;
		std::map<int, std::shared_ptr<mouse_click_event_binding> > mouse_click_bindings;
		std::map<int, std::shared_ptr<mouse_left_click_event_binding> > mouse_left_click_bindings;
		std::map<int, std::shared_ptr<mouse_right_click_event_binding> > mouse_right_click_bindings;
		std::map<int, std::shared_ptr<item_changed_event_binding> > item_changed_bindings;
		std::map<int, std::shared_ptr<list_changed_event_binding> > list_changed_events;
		std::map<int, std::shared_ptr<command_event_binding> > command_bindings;
		std::vector<std::shared_ptr<page_unload_event_binding>> unload_bindings;
		std::vector<std::shared_ptr<page_load_event_binding>> load_bindings;
		std::vector<std::shared_ptr<page_select_event_binding>> select_bindings;
		update_function update_event;
		change_function change_event;

	public:

		std::shared_ptr<menu_item> menu;

		page(const char* _name)
		{
			name = _name != nullptr ? _name : "Test";
			root = std::make_shared<column_layout>();
		}

		page(std::string _name)
		{
			name = _name;
			root = std::make_shared<column_layout>();
		}

		page()
		{
			name = "Test";
			root = std::make_shared<column_layout>();
		}

		virtual ~page()
		{
			destroy();
		}

		void clear()
		{
			key_up_bindings.clear();
			key_down_bindings.clear();
			mouse_move_bindings.clear();
			mouse_click_bindings.clear();
			mouse_left_click_bindings.clear();
			mouse_right_click_bindings.clear();
			item_changed_bindings.clear();
			list_changed_events.clear();
			command_bindings.clear();
			unload_bindings.clear();
			load_bindings.clear();
			select_bindings.clear();
			update_event = nullptr;
			change_event = nullptr;
			destroy();
			root = std::make_shared<column_layout>();
		}

		auto get_root_container() 
		{
			return std::dynamic_pointer_cast<column_layout>(root);
		}

		inline control_base* get_root() {
			return root.get();
		}

		control_base* operator[](int _id)
		{
			return get_root()->find(_id);
		}

		void create(std::weak_ptr<applicationBase> _host)
		{
			if (auto whost = _host.lock()) {
				auto pos = whost->getWindowClientPos();
				pos = whost->toDipsFromPixels(pos);
				arrange(pos.w, pos.h);
				if (root.get())
				{
					root->create(_host);
				}
			}
		}

		void destroy()
		{
			if (root.get())
			{
				root->destroy();
			}
		}

		void draw()
		{
			if (root.get())
			{
				root->draw();
			}
		}

		void render(ID2D1DeviceContext* _context)
		{
			if (root.get())
			{
				root->render(_context);
			}
		}

		void update(double _elapsedSeconds, double _totalSeconds)
		{
			if (update_event) 
			{
				update_event(this, _elapsedSeconds, _totalSeconds);
			}
		}
		
		void changed(std::string _set_name)
		{
			if (change_event) 
			{
				change_event(*this, _set_name);
			}
		}

		void subscribe(presentation_base* _presentation)
		{
			root->on_subscribe(_presentation, this);
		}

		control_builder row_begin(int id = id_counter::next())
		{

			//			std::cout << "create: row"<< std::endl;
			auto new_row = std::make_shared<row_layout>((container_control_base*)nullptr, id);
			root = new_row;
			std::shared_ptr<container_control> build_root = std::dynamic_pointer_cast<container_control>(root);
			control_builder builder(build_root);
			return builder;
		}

		control_builder column_begin(int id = id_counter::next())
		{
			//		std::cout << "create: column" << std::endl;
			auto new_row = std::make_shared<column_layout>((container_control_base*)nullptr, id);
			root = new_row;
			std::shared_ptr<container_control> build_root = std::dynamic_pointer_cast<container_control>(root);
			control_builder builder(build_root);
			return builder;
		}

		control_builder absolute_begin(int id = id_counter::next())
		{
			//			std::cout << "create: begin" << std::endl;
			auto new_row = std::make_shared<absolute_layout>((container_control_base*)nullptr, id);
			root = new_row;
			std::shared_ptr<container_control> build_root = std::dynamic_pointer_cast<container_control>(root);
			control_builder builder(build_root);
			return builder;
		}

		void arrange(double width, double height, double _padding = 0)
		{

			double pd = _padding * 2.0;
			rectangle bounds;
			bounds.x = 0;
			bounds.y = 0;
			bounds.w = width;
			bounds.h = height;

			//			std::cout << "page arrange: " << bounds.w << " " << bounds.h << std::endl;

			root->arrange(bounds);
			//			std::cout << std::endl;
		}

		void on_key_up(int _control_id, std::function< void(key_up_event) > handler)
		{
			auto evt = std::make_shared<key_up_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_up = handler;
			key_up_bindings[_control_id] = evt;
		}

		void on_key_down(int _control_id, std::function< void(key_down_event) >  handler)
		{
			auto evt = std::make_shared<key_down_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_down = handler;
			key_down_bindings[_control_id] = evt;
		}

		void on_mouse_left_click(control_base* _base, std::function< void(mouse_left_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_left_click_event_binding>();
			if (auto pbase = _base)
			{
				if (_base->id == 0) {
					throw std::logic_error("You need to have an id on a control to subscribe to events");
				}
				evt->control = pbase;
				evt->subscribed_item_id = pbase->id;
				evt->on_mouse_left_click = handler;
				mouse_left_click_bindings[pbase->id] = evt;
			}
		}

		void on_mouse_right_click(control_base* _base, std::function< void(mouse_right_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_right_click_event_binding>();
			if (auto pbase = _base)
			{
				if (_base->id == 0) {
					throw std::logic_error("You need to have an id on a control to subscribe to events");
				}
				evt->control = pbase;
				evt->subscribed_item_id = pbase->id;
				evt->on_mouse_right_click = handler;
				mouse_right_click_bindings[pbase->id] = evt;
			}
		}

		void on_mouse_move(control_base* _base, std::function< void(mouse_move_event) > handler)
		{
			auto evt = std::make_shared<mouse_move_event_binding>();
			if (auto pbase = _base)
			{
				if (_base->id == 0) {
					throw std::logic_error("You need to have an id on a control to subscribe to events");
				}
				evt->control = pbase;
				evt->subscribed_item_id = pbase->id;
				evt->on_mouse_move = handler;
				mouse_move_bindings[pbase->id] = evt;
			}
		}

		void on_mouse_click(control_base* _base, std::function< void(mouse_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_click_event_binding>();
			if (auto pbase = _base)
			{
				if (_base->id == 0) {
					throw std::logic_error("You need to have an id on a control to subscribe to events");
				}
				evt->control = pbase;
				evt->subscribed_item_id = pbase->id;
				evt->on_mouse_click = handler;
				mouse_click_bindings[pbase->id] = evt;
			}
		}

		void on_item_changed(int _control_id, std::function< void(item_changed_event) > handler)
		{
			auto evt = std::make_shared<item_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			item_changed_bindings[_control_id] = evt;
		}

		void on_list_changed(int _control_id, std::function< void(list_changed_event) > handler)
		{
			auto evt = std::make_shared<list_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			list_changed_events[_control_id] = evt;
		}

		void on_command(int _control_id, std::function< void(command_event) > handler)
		{
			auto evt = std::make_shared<command_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_command = handler;
			command_bindings[_control_id] = evt;
		}

		void on_changed(change_function fnc)
		{
			change_event = fnc;
		}

		void on_update(update_function fnc)
		{
			update_event = fnc;
		}

		void on_select(std::function< void(page_select_event) > fnc)
		{
			auto plet = std::make_shared<page_select_event_binding>();
			plet->on_select = fnc;
			select_bindings.push_back(plet);
		}

		void on_load(std::function< void(page_load_event) > fnc)
		{
			auto plet = std::make_shared<page_load_event_binding>();
			plet->on_load = fnc;
			load_bindings.push_back(plet);
		}

		void on_unload(std::function< void(page_unload_event) > fnc)
		{
			auto plet = std::make_shared<page_unload_event_binding>();
			plet->on_unload = fnc;
			unload_bindings.push_back(plet);
		}

		bool handle_key_up(int _control_id, key_up_event evt)
		{
			bool handled = false;
			if (key_up_bindings.contains(_control_id)) {
				key_up_bindings[_control_id]->on_key_up(evt);
				handled = true;
			}
			return handled;
		}

		void handle_key_down(int _control_id, key_down_event evt)
		{
			if (key_down_bindings.contains(_control_id)) {
				key_down_bindings[_control_id]->on_key_down(evt);
			}
		}

		void handle_mouse_move(int _control_id, mouse_move_event evt)
		{
			if (mouse_move_bindings.contains(_control_id)) {
				auto& ptrx = mouse_move_bindings[_control_id];
				if (auto temp = ptrx.get()->control) {
					evt.relative_point.x = evt.absolute_point.x - temp->get_bounds().x;
					evt.relative_point.y = evt.absolute_point.y - temp->get_bounds().y;
					evt.control = temp;
					evt.control_id = temp->id;
					ptrx->on_mouse_move(evt);
				}
			}
		}

		void handle_mouse_click(int _control_id, mouse_click_event evt)
		{
			if (mouse_click_bindings.contains(_control_id)) {
				auto& ptrx = mouse_click_bindings[_control_id];
				evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
				evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
				evt.control_id = evt.control->id;
				evt.control = ptrx->control;
				ptrx->on_mouse_click(evt);
			}
		}

		void handle_mouse_left_click(int _control_id, mouse_left_click_event evt)
		{
			if (mouse_left_click_bindings.contains(_control_id)) {
				auto& ptrx = mouse_left_click_bindings[_control_id];
				evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
				evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
				evt.control = ptrx->control;
				ptrx->on_mouse_left_click(evt);
			}
		}

		void handle_mouse_right_click(int _control_id, mouse_right_click_event evt)
		{
			if (mouse_right_click_bindings.contains(_control_id)) {
				auto& ptrx = mouse_right_click_bindings[_control_id];
				evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
				evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
				evt.control_id = evt.control->id;
				evt.control = ptrx->control;
				ptrx->on_mouse_right_click(evt);
			}
		}

		void handle_item_changed(int _control_id, item_changed_event evt)
		{
			if (item_changed_bindings.contains(_control_id)) {
				auto& ptrx = item_changed_bindings[_control_id];
				if (auto temp = ptrx.get()->control) {
					evt.control_id = temp->id;
					evt.control = ptrx->control;
					ptrx->on_change(evt);
				}
			}
		}

		void handle_list_changed(int _control_id, list_changed_event evt)
		{
			if (list_changed_events.contains(_control_id)) {
				auto& ptrx = list_changed_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_change(evt);
				}
			}
		}

		void handle_command(int _control_id, command_event evt)
		{
			if (command_bindings.contains(_control_id)) {
				auto& ptrx = command_bindings[_control_id];
				ptrx->on_command(evt);
			}
		}

		void handle_onselect(std::shared_ptr<page> _pg)
		{
			for (auto evt : select_bindings) {
				page_select_event ple = {};
				ple.pg = _pg;
				evt->on_select(ple);
			}
		}

		void handle_onload(std::shared_ptr<page> _pg)
		{
			for (auto evt : load_bindings) {
				page_load_event ple = {};
				ple.pg = _pg;
				evt->on_load(ple);
			}
		}

		void handle_unload(std::shared_ptr<page> _pg)
		{
			for (auto evt : unload_bindings) {
				page_unload_event ple = {};
				ple.pg = _pg;
				evt->on_unload(ple);
			}
		}

		friend class presentation;
	};
}

#endif
