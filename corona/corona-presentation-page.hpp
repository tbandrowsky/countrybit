#ifndef CORONA_PRESENTATION_PAGE_H
#define CORONA_PRESENTATION_PAGE_H

namespace corona
{

	using update_function = std::function< void(page* _page, double _elapsedSeconds, double _totalSeconds) >;

	class page : public page_base
	{

		std::map<int, std::shared_ptr<key_up_event_binding> > key_up_bindings;
		std::map<int, std::shared_ptr<key_down_event_binding> > key_down_bindings;
		std::map<int, std::shared_ptr<key_press_event_binding> > key_press_bindings;
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
		std::list<std::shared_ptr<page_data_event_binding>> page_data_bindings;
		std::vector<std::shared_ptr<page_refresh_data_binding>> page_refresh_bindings;
		std::list<std::shared_ptr<page_logged_event_binding>> page_logged_bindings;
		update_function update_event;

		lockable binding_lock;

	public:

		std::shared_ptr<menu_item> menu;
		presentation* parent;

		page(const char* _name)
		{
			parent = nullptr;
			name = _name != nullptr ? _name : "Test";
			root = std::make_shared<column_layout>();
		}

		page(std::string _name)
		{
			parent = nullptr;
			name = _name;
			root = std::make_shared<column_layout>();
		}

		page()
		{
			parent = nullptr;
			name = "Test";
			root = std::make_shared<column_layout>();
		}

		virtual ~page()
		{
			destroy();
		}

		void clear()
		{
			scope_lock locker(binding_lock);
			key_press_bindings.clear();
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
			page_data_bindings.clear();
			page_refresh_bindings.clear();
			page_logged_bindings.clear();
			destroy();
			root = std::make_shared<column_layout>();
		}

		void clear_events(int _control_id)
		{
			scope_lock locker(binding_lock);
			key_press_bindings.erase(_control_id);
			key_up_bindings.erase(_control_id);
			key_down_bindings.erase(_control_id);
			mouse_move_bindings.erase(_control_id);
			mouse_click_bindings.erase(_control_id);
			mouse_left_click_bindings.erase(_control_id);
			mouse_right_click_bindings.erase(_control_id);
			item_changed_bindings.erase(_control_id);
			list_changed_events.erase(_control_id);
			command_bindings.erase(_control_id);
			page_data_bindings.remove_if([_control_id](std::shared_ptr<page_data_event_binding>& x) { return x->subscribed_item_id == _control_id; });
			page_logged_bindings.remove_if([_control_id](std::shared_ptr<page_logged_event_binding>& x) { return x->subscribed_item_id == _control_id; });
		}

		auto get_root_container() 
		{
			return std::dynamic_pointer_cast<column_layout>(root);
		}

		inline control_base* get_root() {
			return root.get();
		}

		template <typename T> inline T* find(int _id) {
			return dynamic_cast<T *>(get_root()->find(_id));
		}

		std::shared_ptr<container_control> find_child(int _id) {
			return get_root()->find_by_id<container_control>(_id);
		}

		inline control_base* find(int _id) {
			return get_root()->find(_id);
		}

		template <typename T> inline T* find_container(int _id) {
			return dynamic_cast<T*>(get_root()->find(_id));
		}


		control_base* operator[](int _id)
		{
			return get_root()->find(_id);
		}

		void create(std::weak_ptr<applicationBase> _host)
		{
			if (auto whost = _host.lock()) {
				auto pos = whost->getWindowClientPos();
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

		virtual void draw()
		{
			if (root.get())
			{
				try {
					root->draw();
				}
				catch (std::exception exc)
				{
					std::cout << "Exception " << exc.what() << std::endl;
				}
			}
		}

		void render(ID2D1DeviceContext* _context)
		{
			if (root.get())
			{
				root->render(_context);
			}
		}

		double last_update_seconds = 0;
		const double refresh_pulse_seconds = .5;

		void update(std::shared_ptr<data_lake> _lake, double _elapsedSeconds, double _totalSeconds)
		{
			if (update_event) 
			{
				update_event(this, _elapsedSeconds, _totalSeconds);
			}
			last_update_seconds += _elapsedSeconds;
			if (last_update_seconds > refresh_pulse_seconds) {
				double previous_last_update_seconds = last_update_seconds;
				last_update_seconds = 0;
				scope_lock locker(binding_lock);
				for (auto sch : page_refresh_bindings) {
//					std::cout << "refresh check " << sch->source_name << " " << sch->function_name << " " << sch->seconds_since_update << std::endl;
					sch->seconds_since_update += previous_last_update_seconds;
					if (sch->seconds_since_update > sch->period_seconds)
					{
						sch->seconds_since_update = 0;
						_lake->call_function(sch->source_name, sch->function_name);
					}
				}
			}
		}
		
		void subscribe(presentation_base* _presentation)
		{
			root->on_subscribe(_presentation, this);
		}

		control_builder edit(int id)
		{
			std::shared_ptr<container_control> ct = root->find_by_id<container_control>(id);
			return control_builder(ct);
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
			if (root) {
				rectangle bounds;
				bounds.x = 0;
				bounds.y = 0;
				bounds.w = width;
				bounds.h = height;
				root->arrange(bounds);
			}
		}

		int get_keyboard_parent(int _control_id)
		{
			if (!root) 
				return -1;

			control_base* p = root->find(_control_id);
			while (p) {
				if (p->gets_real_focus()) {
					return p->id;
				}
				p = (control_base*)p->parent;
			}
			return -1;
		}

		virtual void on_key_press(int _control_id, std::function< void(key_press_event) > handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<key_press_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_press = handler;
			int event_capture_id = get_keyboard_parent(_control_id);
			if (event_capture_id > -1) {
				key_press_bindings[event_capture_id] = evt;
			}
		}

		virtual void on_key_up(int _control_id, std::function< void(key_up_event) > handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<key_up_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_up = handler;
			int event_capture_id = get_keyboard_parent(_control_id);
			if (event_capture_id > -1) {
				key_up_bindings[event_capture_id] = evt;
			}
		}

		virtual void on_key_down(int _control_id, std::function< void(key_down_event) >  handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<key_down_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_down = handler;
			key_down_bindings[_control_id] = evt;
			int event_capture_id = get_keyboard_parent(_control_id);
			if (event_capture_id > -1) {
				key_down_bindings[event_capture_id] = evt;
			}
		}

		virtual void on_mouse_left_click(control_base* _base, std::function< void(mouse_left_click_event) > handler)
		{
			scope_lock locker(binding_lock);
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

		virtual void on_mouse_right_click(control_base* _base, std::function< void(mouse_right_click_event) > handler)
		{
			scope_lock locker(binding_lock);
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

		virtual void on_mouse_move(control_base* _base, std::function< void(mouse_move_event) > handler)
		{
			scope_lock locker(binding_lock);
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

		virtual void on_mouse_click(control_base* _base, std::function< void(mouse_click_event) > handler)
		{
			scope_lock locker(binding_lock);
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

		virtual void on_item_changed(int _control_id, std::function< void(item_changed_event) > handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<item_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			item_changed_bindings[_control_id] = evt;
		}

		virtual void on_list_changed(int _control_id, std::function< void(list_changed_event) > handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<list_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			list_changed_events[_control_id] = evt;
		}

		virtual void on_command(int _control_id, std::function< void(command_event) > handler)
		{
			scope_lock locker(binding_lock);
			auto evt = std::make_shared<command_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_command = handler;
			command_bindings[_control_id] = evt;
		}

		virtual void on_update(update_function fnc)
		{
			scope_lock locker(binding_lock);
			update_event = fnc;
		}

		virtual void on_select(std::function< void(page_select_event) > fnc)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_select_event_binding>();
			plet->on_select = fnc;
			select_bindings.push_back(plet);
		}

		virtual void on_load(std::function< void(page_load_event) > fnc)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_load_event_binding>();
			plet->on_load = fnc;
			load_bindings.push_back(plet);
		}

		virtual void on_unload(std::function< void(page_unload_event) > fnc)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_unload_event_binding>();
			plet->on_unload = fnc;
			unload_bindings.push_back(plet);
		}

		virtual void on_changed(int _control_id, std::string _source_name, std::string _function_name, std::function< void(page_data_event) > evt)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_data_event_binding>();
			plet->source_name = _source_name;
			plet->function_name = _function_name;
			plet->subscribed_item_id = _control_id;
			plet->on_changed = evt;
			page_data_bindings.push_back(plet);
		}

		virtual void on_logged(int _control_id, std::function< void(page_logged_event) > evt)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_logged_event_binding>();
			plet->subscribed_item_id = _control_id;
			plet->on_changed = evt;
			page_logged_bindings.push_back(plet);
		}

		virtual void schedule_refresh(time_t _period_seconds, std::string _source_name, std::string _function_name)
		{
			scope_lock locker(binding_lock);
			auto plet = std::make_shared<page_refresh_data_binding>();
			plet->source_name = _source_name;
			plet->function_name = _function_name;
			plet->period_seconds = _period_seconds;
			plet->seconds_since_update = 0;
			page_refresh_bindings.push_back(plet);
		}

		bool handle_key_up(int _control_id, key_up_event evt)
		{
			scope_lock locker(binding_lock);
			bool handled = false;
			if (key_up_bindings.contains(_control_id)) {
				auto sc = key_up_bindings[_control_id]->subscribed_item_id;
				auto fnd = root->find_by_id<control_base>(sc);
				if (fnd) {
					evt.control = fnd.get();
				}
				else {
					evt.control = nullptr;
				}
				key_up_bindings[_control_id]->on_key_up(evt);
				handled = true;
			}
			return handled;
		}

		void handle_key_down(int _control_id, key_down_event evt)
		{
			scope_lock locker(binding_lock);
			if (key_down_bindings.contains(_control_id)) {
				auto sc = key_down_bindings[_control_id]->subscribed_item_id;
				auto fnd = root->find_by_id<control_base>(sc);
				if (fnd) {
					evt.control = fnd.get();
				}
				else {
					evt.control = nullptr;
				}
				key_down_bindings[_control_id]->on_key_down(evt);
			}
		}

		void handle_key_press(int _control_id, key_press_event evt)
		{
			scope_lock locker(binding_lock);
			if (key_press_bindings.contains(_control_id)) {
				auto sc = key_press_bindings[_control_id]->subscribed_item_id;
				auto fnd = root->find_by_id<control_base>(sc);
				if (fnd) {
					evt.control = fnd.get();
				}
				else {
					evt.control = nullptr;
				}
				key_press_bindings[_control_id]->on_key_press(evt);
			}
		}

		void handle_mouse_move(int _control_id, mouse_move_event evt)
		{
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
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
			scope_lock locker(binding_lock);
			if (command_bindings.contains(_control_id)) {
				auto& ptrx = command_bindings[_control_id];
				ptrx->on_command(evt);
			}
		}

		void handle_onselect(std::shared_ptr<page> _pg)
		{
			scope_lock locker(binding_lock);
			for (auto evt : select_bindings) {
				page_select_event ple = {};
				ple.pg = _pg;
				if (evt) {
					evt->on_select(ple);
				}
			}
		}

		void handle_onload(std::shared_ptr<page> _pg)
		{
			scope_lock locker(binding_lock);
			for (auto evt : load_bindings) {
				page_load_event ple = {};
				ple.pg = _pg;
				if (evt) {
					evt->on_load(ple);
				}
			}			
		}

		void handle_unload(std::shared_ptr<page> _pg)
		{
			scope_lock locker(binding_lock);
			for (auto evt : unload_bindings) {
				page_unload_event ple = {};
				ple.pg = _pg;
				if (evt) {
					evt->on_unload(ple);
				}
			}
		}

		void handle_changed(std::shared_ptr<page> _pg, json _params, data_lake* _api, data_function* _set)
		{
			scope_lock locker(binding_lock);
			for (auto pdb : page_data_bindings) {

				if (pdb->source_name == _set->api->name &&
					(pdb->function_name == _set->name || pdb->function_name == "*"))
				{
					threadomatic::run_complete(nullptr, [_pg, this, _params, _api, _set, pdb]() {
						page_data_event pde;
						pde.changed_fn = _set;
						pde.destination_control_id = pdb->subscribed_item_id;
						pde.params = _params;
						pde.lake = _api;
						pde.pg = _pg;
						pde.control = _pg->find(pde.destination_control_id);
						pdb->on_changed(pde);
						});
				}
			}
		}

		void handle_logged(std::shared_ptr<page> _pg, data_lake* _api)
		{
			scope_lock locker(binding_lock);
			for (auto pdb : page_logged_bindings) {
				threadomatic::run_complete(nullptr, [_pg, this, _api, pdb]() {
						page_logged_event pde;
						pde.destination_control_id = pdb->subscribed_item_id;
						pde.lake = _api;
						pde.pg = _pg;
						pde.control = _pg->find(pde.destination_control_id);
						pdb->on_changed(pde);
						});
			}
		}

		friend class presentation;
	};
}

#endif
