
#include "corona.h"

#define TRACE_LAYOUT 01

namespace corona
{
	namespace database
	{

		const control_base* control_base::find(int _id) const
		{
			const control_base* result = nullptr;
			if (this->id == _id) {
				result = this;
			}
			else
			{
				for (auto child : children)
				{
					result = child->find(_id);
					if (result != nullptr) {
						break;
					}
				}
			}
			return result;
		}

		std::weak_ptr<control_base> control_base::get(std::shared_ptr<control_base>& _root, int _id)
		{
			std::weak_ptr<control_base> result;
			if (_root->id == _id) {
				return _root;
			}
			else
			{
				for (auto child : _root->children)
				{
					result = get(child, _id);
					if (!result.expired()) {
						return result;
					}
				}
			}
			return result;
		}

		void control_base::foreach(std::function<void(control_base * _root)> _item)
		{
			_item(this);
			for (auto child : children) {
				child->foreach(_item);
			}
		}

		void control_base::create(win32::win32ControllerHost* _host)
		{
			for (auto child : children) {
				child->create(_host);
			}
		}

		void control_base::destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void control_base::draw()
		{
			for (auto child : children) {
				child->draw();
			}
		}

		row_layout& control_base::row_layout_new(int id)
		{
			return create<row_layout>(id);
		}

		column_layout& control_base::column_layout_new(int id)
		{
			return create<column_layout>(id);
		}

		absolute_layout& control_base::absolute_layout_new(int id)
		{
			return create<absolute_layout>(id);
		}


		static_control& control_base::static_control_new(int id)
		{
			return create<static_control>(id);
		}

		button_control& control_base::button_control_new(int id)
		{
			return create<button_control>(id);
		}

		listbox_control& control_base::listbox_control_new(int id)
		{
			return create<listbox_control>(id);
		}

		combobox_control& control_base::combobox_control_new(int id)
		{
			return create<combobox_control>(id);
		}

		edit_control& control_base::edit_control_new(int id)
		{
			return create<edit_control>(id);
		}

		scrollbar_control& control_base::scrollbar_control_new(int id)
		{
			return create<scrollbar_control>(id);
		}

		listview_control& control_base::listview_control_new(int id)
		{
			return create<listview_control>(id);
		}

		treeview_control& control_base::treeview_control_new(int id)
		{
			return create<treeview_control>(id);
		}

		header_control& control_base::header_control_new(int id)
		{
			return create<header_control>(id);
		}

		toolbar_control& control_base::toolbar_control_new(int id)
		{
			return create<toolbar_control>(id);
		}

		statusbar_control& control_base::statusbar_control_new(int id)
		{
			return create<statusbar_control>(id);
		}

		hotkey_control& control_base::hotkey_control_new(int id)
		{
			return create<hotkey_control>(id);
		}

		animate_control& control_base::animate_control_new(int id)
		{
			return create<animate_control>(id);
		}

		richedit_control& control_base::richedit_control_new(int id)
		{
			return create<richedit_control>(id);
		}

		draglistbox_control& control_base::draglistbox_control_new(int id)
		{
			return create<draglistbox_control>(id);
		}

		rebar_control& control_base::rebar_control_new(int id)
		{
			return create<rebar_control>(id);
		}

		comboboxex_control& control_base::comboboxex_control_new(int id)
		{
			return create<comboboxex_control>(id);
		}

		datetimepicker_control& control_base::datetimepicker_control_new(int id)
		{
			return create<datetimepicker_control>(id);
		}

		monthcalendar_control& control_base::monthcalendar_control_new(int id)
		{
			return create<monthcalendar_control>(id);
		}

		void control_base::size_constant(layout_context _ctx)
		{

			control_base& pi = *this;
			pi.bounds.w = 0;
			pi.bounds.h = 0;

			if (pi.box.width.units == measure_units::pixels)
			{
				pi.bounds.w = pi.box.width.amount;
			}
			else if (pi.box.width.units == measure_units::percent_container)
			{
				pi.bounds.w = pi.box.width.amount * _ctx.container_size.x;
			}
			else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				pi.bounds.w = font_height * pi.box.width.amount;
				if (pi.box.width.units == measure_units::font_golden_ratio)
				{
					pi.bounds.w /= 1.618;
				}
			}

			if (pi.box.height.units == measure_units::pixels)
			{
				pi.bounds.h = pi.box.height.amount;
			}
			else if (pi.box.height.units == measure_units::percent_container)
			{
				pi.bounds.h = pi.box.height.amount * _ctx.container_size.y;
			}
			else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = 12.0;
				pi.bounds.h = font_height * pi.box.height.amount;
				if (pi.box.height.units == measure_units::font_golden_ratio)
				{
					pi.bounds.h *= 1.618;
				}
			}
		}

		void control_base::size_constants(layout_context _ctx)
		{
			for (auto child : children)
			{
				child->size_constant(_ctx);
			}
		}

		void control_base::size_aspect_widths(layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (box.width.units == measure_units::percent_aspect)
			{
				size_aspect_heights(_ctx, safety + 1);
				bounds.w = box.width.amount * bounds.h;
			}
		}

		void control_base::size_aspect_heights(layout_context _ctx, int safety)
		{
			if (safety > 2)
				return;

			if (box.height.units == measure_units::percent_aspect)
			{
				size_aspect_widths(_ctx, safety + 1);
				bounds.h = box.height.amount * bounds.w;
			}
		}

		void control_base::size_aspect(layout_context _ctx)
		{
			size_aspect_heights(_ctx, 0);
			size_aspect_widths(_ctx, 0);
		}

		void control_base::size_aspects(layout_context _ctx)
		{
			for (auto child : children)
			{
				child->size_aspect(_ctx);
			}
		}

		layout_context control_base::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };
			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}



		layout_context row_layout::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.width.units != measure_units::percent_remaining)
				{
					pt.x += child->bounds.w;
				}
			}

			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}

		layout_context column_layout::get_remaining(layout_context _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };

			for (auto child : children)
			{
				if (child->box.height.units != measure_units::percent_remaining)
				{
					pt.y += child->bounds.h;
				}
			}

			_ctx.remaining_size = _ctx.container_size - pt;
			return _ctx;
		}


		void control_base::size_remaining(layout_context _ctx)
		{
			if (box.width.units == measure_units::percent_remaining)
			{
				bounds.w = box.width.amount * _ctx.remaining_size.x;
			}
			if (box.height.units == measure_units::percent_remaining)
			{
				bounds.h = box.height.amount * _ctx.remaining_size.y;
			}
		}

		void control_base::size_remainings(layout_context _ctx)
		{
			_ctx = get_remaining(_ctx);
			for (auto child : children)
			{
				child->size_remaining(_ctx);
			}
		}

		void control_base::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto ew = child->bounds.w + child->bounds.x - bounds.x;
					if (ew > sizes.x)
					{
						sizes.x = ew;
					}
					auto eh = child->bounds.h + child->bounds.y - bounds.y;
					if (child->bounds.h > sizes.y)
					{
						sizes.y = child->bounds.h;
					}
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		void column_layout::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto ew = child->bounds.w + child->bounds.x - bounds.x;
					if (ew > sizes.x)
					{
						sizes.x = ew;
					}
					sizes.y += child->bounds.h;
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		void row_layout::size_children(layout_context _ctx)
		{
			bool sheight = box.height.units == measure_units::percent_child;
			bool swidth = box.width.units == measure_units::percent_child;
			if (sheight || swidth)
			{
				point sizes = { 0.0, 0.0 };
				for (auto child : children)
				{
					auto eh = child->bounds.h + child->bounds.y - bounds.y;
					if (child->bounds.h > sizes.y)
					{
						sizes.y = child->bounds.h;
					}
					sizes.x += child->bounds.w;
				}
				if (sheight)
				{
					bounds.h = sizes.y * box.height.amount;
				}
				if (swidth)
				{
					bounds.w = sizes.x * box.width.amount;
				}
			}
			size_remainings(_ctx);
		}

		bool control_base::contains(point pt)
		{
			return rectangle_math::contains(bounds, pt.x, pt.y);
		}

		void control_base::size_item(layout_context _ctx)
		{
			size_constant(_ctx);
			size_aspect(_ctx);
			size_remaining(_ctx);
		}

		void control_base::size_items(layout_context _ctx)
		{
			size_constants(_ctx);
			size_aspects(_ctx);
			size_remainings(_ctx);
		}

		void control_base::position(layout_context _ctx)
		{
			switch (box.x.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				bounds.x = box.x.amount * _ctx.container_size.x + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::pixels:
				bounds.x = box.x.amount + _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				bounds.x = _ctx.flow_origin.x + _ctx.container_origin.x;
				break;
			}

			switch (box.y.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				bounds.y = box.y.amount * _ctx.container_size.y + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::pixels:
				bounds.y = box.y.amount + _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				bounds.y = _ctx.flow_origin.y + _ctx.container_origin.y;
				break;
			}


			switch (item_space.units)
			{
			case measure_units::font:
				item_space_amount.x = item_space.amount * 16.0;
				item_space_amount.y = item_space.amount * 16.0;
				break;
			case measure_units::font_golden_ratio:
				item_space_amount.x = item_space.amount * 16.0 / 1.618;
				item_space_amount.y = item_space.amount * 16.0 / 1.618;
				break;
			case measure_units::percent_aspect:
				item_space_amount.x = item_space.amount * _ctx.container_size.y;
				item_space_amount.y = item_space.amount * _ctx.container_size.x;
				break;
			case measure_units::percent_remaining:
				item_space_amount.x = item_space.amount * _ctx.remaining_size.x;
				item_space_amount.y = item_space.amount * _ctx.remaining_size.y;
				break;
			case measure_units::pixels:
				item_space_amount.x = item_space.amount;
				item_space_amount.y = item_space.amount;
				break;
			}
		}

		void control_base::positions(layout_context _ctx)
		{
			_ctx.flow_origin.x = 0;
			_ctx.flow_origin.y = 0;

			for (auto child : children)
			{
				child->position(_ctx);
				child->layout(_ctx);
			}

			size_children(_ctx);
		}

		void row_layout::positions(layout_context _ctx)
		{
			if (alignment == visual_alignment::align_near)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x += (child->bounds.w);
					_ctx.flow_origin.x += _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_far)
			{
				_ctx.flow_origin.x = _ctx.container_size.x;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					_ctx.flow_origin.x -= (child->bounds.w);
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x -= _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_center)
			{
				double w = 0.0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					w += child->bounds.w;
				}

				_ctx.flow_origin.x = (_ctx.container_size.x - w) / 2;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.x += (child->bounds.w);
					_ctx.flow_origin.x += _ctx.space_amount.x;
				}
			}

			size_children(_ctx);
		}

		void column_layout::positions(layout_context _ctx)
		{
			if (alignment == visual_alignment::align_near)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y += (child->bounds.h);
					_ctx.flow_origin.y += _ctx.space_amount.y;
				}
			}
			else if (alignment == visual_alignment::align_far)
			{
				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = _ctx.container_size.y;

				for (auto child : children)
				{
					_ctx.flow_origin.y -= (child->bounds.h);
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y -= _ctx.space_amount.x;
				}
			}
			else if (alignment == visual_alignment::align_center)
			{
				double h = 0;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					h += child->bounds.h;
				}

				_ctx.flow_origin.x = 0;
				_ctx.flow_origin.y = (_ctx.container_size.y - h) / 2;

				for (auto child : children)
				{
					child->position(_ctx);
					child->layout(_ctx);
					_ctx.flow_origin.y += (child->bounds.h);
					_ctx.flow_origin.y += _ctx.space_amount.y;
				}
			}

			size_children(_ctx);
		}

		rectangle control_base::layout(layout_context _ctx)
		{
			_ctx.container_origin.x = bounds.x;
			_ctx.container_origin.y = bounds.y;
			_ctx.container_size.x = bounds.w;
			_ctx.container_size.y = bounds.h;
			_ctx.remaining_size = _ctx.container_size;
			_ctx.space_amount = { (double)item_space.amount, (double)item_space.amount };
			_ctx.flow_origin.x = 0;
			_ctx.flow_origin.y = 0;

			size_items(_ctx);
			positions(_ctx);

			return bounds;

		}

		void container_control::create(win32::win32ControllerHost* _host)
		{
			host = _host;
			window = _host->createDirect2Window(id, bounds);
			for (auto child : children) {
				child->create(_host);
			}
		}

		void container_control::destroy()
		{
			host = nullptr;
			window = nullptr;
			for (auto child : children) {
				child->destroy();
			}
		}

		void container_control::draw()
		{
			bool adapter_blown_away = false;
			window->beginDraw(adapter_blown_away);
			if (!adapter_blown_away)
			{
				if (this->on_draw != nullptr) {
					on_draw(this);
				}
				else
					window->drawRectangle(&bounds, "container", 2, "containerfill");
			}
			window->endDraw(adapter_blown_away);

			for (auto child : children) {
				child->draw();
			}
		}

		page::page(const char* _name = nullptr) : name(_name)
		{
			root = std::make_shared<column_layout>();
		}

		void page::clear()
		{
			destroy();
			root = std::make_shared<column_layout>();
		}

		void page::create(win32::win32ControllerHost* _host)
		{
			if (root.get()) 
			{
				root->create(_host);
			}
		}

		void page::destroy()
		{
			if (root.get())
			{
				root->destroy();
			}
		}

		void page::draw()
		{
			if (root.get())
			{
				root->draw();
			}
		}

		void page::update(double _elapsedSeconds, double _totalSeconds)
		{
			if (update_event) {
				update_event(this, _elapsedSeconds, _totalSeconds);
			}
		}

		bool presentation::drawFrame()
		{
			return false;
		}

		bool presentation::update(double _elapsedSeconds, double _totalSeconds)
		{
			;
		}

		void page::arrange(double width, double height, double _padding)
		{

			double pd = _padding * 2.0;
			layout_context ctx;
			ctx.space_amount = { 0.0, 0.0 };
			ctx.container_size = { width - pd, height - pd };
			ctx.container_origin = { _padding, _padding };
			ctx.flow_origin = { 0, 0 };
			ctx.remaining_size = ctx.container_size;

			if (root->box.height.units == measure_units::percent_child ||
				root->box.width.units == measure_units::percent_child)
				throw std::logic_error("Cannot use child based sizing on a root element");

			root->size_item(ctx);
			root->position(ctx);
			root->layout(ctx);

		}

		void page::on_key_up(int _control_id, std::function< void(key_up_event) > handler)
		{
			auto evt = std::make_shared<key_up_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_up = handler;
			key_up_events[_control_id] = evt;
		}

		void page::on_key_down(int _control_id, std::function< void(key_down_event) >  handler)
		{
			auto evt = std::make_shared<key_down_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_down = handler;
			key_down_events[_control_id] = evt;
		}

		void page::on_mouse_move(int _control_id, std::function< void(mouse_move_event) > handler)
		{
			auto evt = std::make_shared<mouse_move_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_move = handler;
			mouse_move_events[_control_id] = evt;
		}

		void page::on_mouse_click(int _control_id, std::function< void(mouse_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_click_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_click = handler;
			mouse_click_events[_control_id] = evt;
		}

		void page::on_item_changed(int _control_id, std::function< void(item_changed_event) > handler)
		{
			auto evt = std::make_shared<item_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			item_changed_events[_control_id] = evt;
		}

		void page::on_list_changed(int _control_id, std::function< void(list_changed_event) > handler)
		{
			auto evt = std::make_shared<list_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			list_changed_events[_control_id] = evt;
		}

		void page::on_update(update_function fnc)
		{
			update_event = fnc;
		}

		void page::handle_key_up(int _control_id, key_up_event evt)
		{
			if (key_up_events.contains(_control_id)) {
				key_up_events[_control_id]->on_key_up(evt);
			}
		}

		void page::handle_key_down(int _control_id, key_down_event evt)
		{
			if (key_down_events.contains(_control_id)) {
				key_down_events[_control_id]->on_key_down(evt);
			}
		}

		void page::handle_mouse_move(int _control_id, mouse_move_event evt)
		{
			mouse_move_event* pevt = &evt;

			if (mouse_move_events.contains(_control_id)) {
				auto& ptrx = mouse_move_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->bounds.x;
					evt.relative_point.y = evt.absolute_point.y - temp->bounds.y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_move(evt);
				}
			}

			if (!_control_id) 
			{
				for (auto evh : mouse_move_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->bounds.x;
						evt.relative_point.y = evt.absolute_point.y - lck->bounds.y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_move(evt);
					}
				}
			}

		}

		void page::handle_mouse_click(int _control_id, mouse_click_event evt)
		{
			mouse_click_event* pevt = &evt;

			if (mouse_click_events.contains(_control_id)) {
				auto& ptrx = mouse_click_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->bounds.x;
					evt.relative_point.y = evt.absolute_point.y - temp->bounds.y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_click(evt);
				}
			}

			if (!_control_id)
			{
				for (auto evh : mouse_click_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->bounds.x;
						evt.relative_point.y = evt.absolute_point.y - lck->bounds.y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_click(evt);
					}
				}
			}
		}

		void presentation::select_page(const std::string& _page_name)
		{
			auto lock_ptr = current_page.lock();
			if (lock_ptr) {
				lock_ptr->destroy();
			}
			if (pages.contains(_page_name)) {
				current_page = pages[_page_name];
			}
		}

		bool presentation::drawFrame()
		{
			;
		}

		bool presentation::update(double _elapsedSeconds, double _totalSeconds)
		{
			;
		}

		void presentation::keyDown(win32::direct2dWindow* win, short _key)
		{
			auto cp = current_page.lock();
			if (cp) {
				auto children = win->getChildren();
				key_down_event kde;
				kde.control_id = 0;
				kde.key = _key;
				cp->handle_key_down(0, kde);
			}
		}

		void presentation::keyUp(win32::direct2dWindow* win, short _key)
		{
			auto cp = current_page.lock();
			if (cp) {
				key_up_event kde;
				kde.control_id = 0;
				kde.key = _key;
				cp->handle_key_up(0, kde);
			}
		}

		void presentation::mouseMove(win32::direct2dWindow* win, point* _point)
		{
			auto cp = current_page.lock();
			if (cp) {
				mouse_move_event kde;
				kde.control_id = 0;
				kde.absolute_point = *_point;
				cp->handle_mouse_move(0, kde);
			}
		}

		void presentation::mouseClick(win32::direct2dWindow* win, point* _point)
		{
			auto cp = current_page.lock();
			if (cp) {
				mouse_click_event kde;
				kde.control_id = 0;
				kde.absolute_point = *_point;
				cp->handle_mouse_click(0, kde);
			}
		}

		void presentation::pointSelected(win32::direct2dWindow* win, point* _point, color* _color)
		{
			;
		}

		void presentation::onCreated()
		{
			;
		}

		void presentation::onCommand(int buttonId)
		{
			;
		}

		void presentation::onTextChanged(int textControlId)
		{
			std::string newText = getHost()->getEditText(textControlId);
		}

		void presentation::onDropDownChanged(int dropDownId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string newText = ptr->getComboSelectedText(dropDownId);
				int index = ptr->getComboSelectedIndex(dropDownId);
				int value = ptr->getComboSelectedValue(dropDownId);
			}
		}

		void presentation::onListViewChanged(int listViewId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string newText = ptr->getListViewSelectedText(listViewId);
				int index = ptr->getListViewSelectedIndex(listViewId);
				int value = ptr->getListViewSelectedValue(listViewId);
			}
		}

		int presentation::onHScroll(int controlId, win32::scrollTypes scrollType)
		{

		}

		int presentation::onVScroll(int controlId, win32::scrollTypes scrollType)
		{

		}

		int presentation::onResize(const rectangle& newSize, double d2dScale)
		{
			auto pg = current_page.lock();
			if (pg) {
				pg->arrange(newSize.w, newSize.h);
			}
		}

		int presentation::onSpin(int controlId, int newPosition)
		{
			int value = newPosition;
		}

	}
}
