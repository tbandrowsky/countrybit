
#ifndef CORONA_PRESENTATION_CONTROLS_BASE_H
#define CORONA_PRESENTATION_CONTROLS_BASE_H

#include "corona-presentation-base.hpp"
#include "corona-presentation-events.hpp"
#include "corona-presentation-menu.hpp"

namespace corona
{
	class row_layout;
	class column_layout;
	class absolute_layout;
	class frame_layout;
	class row_view_layout;
	class column_view_layout;
	class absolute_view_layout;
	class single_column_form;
	class double_column_form;

	class title_control;
	class subtitle_control;
	class chaptertitle_control;
	class chaptersubtitle_control;
	class paragraph_control;
	class code_control;
	class label_control;
	class image_control;

	class static_control;
	class pushbutton_control;
	class pressbutton_control;
	class checkbox_control;
	class radiobutton_control;
	class linkbutton_control;
	class listbox_control;
	class combobox_control;
	class edit_control;
	class scrollbar_control;
	class listview_control;
	class treeview_control;
	class header_control;
	class toolbar_control;
	class statusbar_control;
	class hotkey_control;
	class animate_control;
	class richedit_control;
	class draglistbox_control;
	class comboboxex_control;
	class datetimepicker_control;
	class monthcalendar_control;
	class control_base;

	class minimize_button_control;
	class maximize_button_control;
	class close_button_control;
	class menu_button_control;

	class caption_bar_control;
	class status_bar_control;
	class form_double_column_control;
	class form_single_column_control;

	class camera_control;

	enum control_push_property
	{
		cp_none = 0,
		cp_left_bounds = 1,
		cp_top_bounds = 2,
		cp_right_bounds = 4,
		cp_bottom_bounds = 8
	};

	class control_push_request
	{
	public:
		int dest_control_id;
		int properties_to_push;
	};

	template <typename T> class change_monitored_property
	{
		T last_value;
		T current_value;
		bool is_changed;

	public:

		change_monitored_property()
		{
			current_value = {};
			last_value = {};
			is_changed = false;
		}

		change_monitored_property(T _default_value)
		{
			current_value = _default_value;
			last_value = _default_value;
			is_changed = false;
		}

		virtual ~change_monitored_property()
		{
			;
		}

		operator T ()
		{
			return current_value;
		}

		T operator = (T _new_value)
		{
			is_changed = (_new_value != current_value);
			last_value = current_value;
			current_value = _new_value;
			return current_value;
		}

		void reset(T _value)
		{
			is_changed = false;
			last_value = current_value;
			current_value = _value;
			return current_value;
		}

		T& value()
		{
			return current_value;
		}

		bool changed_from(T _from_value)
		{
			bool really_changed = (is_changed &&
				_from_value == current_value);
			return really_changed;
		}

		bool changed_to(T _to_value)
		{
			bool really_changed = (is_changed &&
				_to_value == current_value);
			return really_changed;
		}

		bool changed(T _from_value, T _to_value)
		{
			bool really_changed = (is_changed &&
				_from_value == last_value &&
				_to_value == current_value);
			return really_changed;
		}
	};

	class container_control_base
	{
	public:
		container_control_base()
		{
			;
		}

		virtual ~container_control_base()
		{
			;
		}

		virtual control_base* get_control() = 0;
		virtual LRESULT get_nchittest() = 0;
	};

	template <typename T> class cloneable {
	public:
		virtual std::shared_ptr<control_base> clone(container_control_base* _parent)
		{
			T* base = dynamic_cast<T*>(this);
			std::shared_ptr<control_base> cb = std::make_shared<T>(*base);
			cb->parent = _parent;
			return cb;
		}
	};

	class control_base : public container_control_base, public cloneable<control_base>
	{
	protected:

		point get_size(rectangle _ctx, point _remaining);
		point get_position(rectangle _ctx);
		double to_pixels(measure _margin);
		virtual point get_remaining(point _ctx);
		virtual void on_resize();
		void arrange_children(rectangle _bounds,
			std::function<point(const rectangle* _bounds, control_base*)> _initial_origin,
			std::function<point(point* _origin, const rectangle* _bounds, control_base*)> _next_origin);

		rectangle				bounds;
		rectangle				inner_bounds;
		point					margin_amount;
		point					padding_amount;

		change_monitored_property<bool> mouse_over;
		change_monitored_property<bool> mouse_left_down;
		change_monitored_property<bool> mouse_right_down;
		point		 mouse_relative_position;

		void copy(const control_base& _src)
		{
			id = _src.id;
			bounds = _src.bounds;
			inner_bounds = _src.inner_bounds;
			margin_amount = _src.margin_amount;
			padding_amount = _src.padding_amount;

			mouse_over = _src.mouse_over;
			mouse_left_down = _src.mouse_left_down;
			mouse_right_down = _src.mouse_right_down;
			mouse_relative_position = _src.mouse_relative_position;

			box = _src.box;
			margin = _src.margin;
			padding = _src.padding;
			tooltip_text = _src.tooltip_text;

			parent = _src.parent;
			push_requests = _src.push_requests;

			children.clear();
			for (auto child : _src.children) {
				auto new_child = child->clone(this);
				children.push_back(new_child);
			}
		}

	public:

		friend class draw_control;
		friend class row_layout;
		friend class column_layout;
		friend class absolute_layout;

		int						id;

		layout_rect				box;
		measure					margin;
		measure					padding;
		std::string				tooltip_text;

		container_control_base* parent;
		std::vector<control_push_request> push_requests;

		std::vector<std::shared_ptr<control_base>> children;

		LRESULT hittest = HTCLIENT;

		virtual LRESULT get_nchittest() { return hittest; }
		virtual void set_nchittest(LRESULT _hittest) { hittest = _hittest; }

		control_base() :
			id(-1),
			margin(),
			parent(nullptr),
			margin_amount({ 0.0, 0.0 })
		{
			id = id_counter::next();
		}

		control_base(container_control_base *_parent, int _id) : control_base()
		{
			parent = _parent;

			if (_id < 0) {
				id = id_counter::next();
			}
			else {
				id = _id;
			}
		}

		virtual control_base* get_control()
		{
			return this;
		}

		control_base( const control_base& _src ) 
		{
			copy(_src);
		}

		virtual ~control_base()
		{
			;
		}

		rectangle& get_bounds() { return bounds; }
		rectangle& get_inner_bounds() { return inner_bounds; }

		rectangle& set_bounds(rectangle& _bounds);

		virtual void arrange(rectangle _ctx);
		bool contains(point pt);

		void push(int _destination_control_id, bool _push_left, bool _push_top, bool _push_right, bool _push_bottom)
		{
			control_push_request cpr = {};

			cpr.dest_control_id = _destination_control_id;
			if (_push_left) {
				cpr.properties_to_push |= cp_left_bounds;
			}
			if (_push_top) {
				cpr.properties_to_push |= cp_top_bounds;
			}
			if (_push_right) {
				cpr.properties_to_push |= cp_right_bounds;
			}
			if (_push_bottom) {
				cpr.properties_to_push |= cp_bottom_bounds;
			}

			push_requests.push_back(cpr);
		}

		control_base* find(int _id);
		control_base* find(point p);
		control_base* get(control_base* _root, int _id);

		virtual double get_font_size() { return 12; }

		template <typename control_type> control_type& find(int _id)
		{
			control_base* temp = find(_id);
			if (!temp) {
				throw std::invalid_argument(std::format("Control id {0} not found", _id));
			}
			control_type* citem = dynamic_cast<control_type *>(temp);
			if (!citem) {
				throw std::invalid_argument(std::format("Control id {0} is a {1}.", _id, typeid(*temp).name()));
			}
			return *citem;
		}

		template <typename control_type> control_type& get(control_base* _root, int _id)
		{
			control_base* temp = get(_root, _id);
			control_type* citem = dynamic_cast<control_type *>(temp);
			return *citem;
		}

		void foreach(std::function<void(control_base* _root)> _item);
		control_base* find_if(std::function<bool(control_base* _root)> _item);

		virtual void create(std::weak_ptr<applicationBase> _host);
		virtual void destroy();
		virtual void draw();
		virtual void render(ID2D1DeviceContext* _dest);

		control_base& set_origin(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return *this;
		}

		control_base& set_size(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return *this;
		}

		control_base& set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		control_base& set_margin(measure _space)
		{
			margin = _space;
			return *this;
		}

		template <typename control_class> control_class& set_spacing(measure _spacing)
		{
			control_class* r = dynamic_cast<control_class>(this);
			r->margin = _spacing;
			return *this;
		}

		virtual void apply_item_sizes(control_base& _ref);
		virtual bool set_mouse(point _position,
			bool* _left_down,
			bool* _right_down,
			std::function<void(control_base* _item)> _left_click,
			std::function<void(control_base* _item)> _right_click
		);

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			;
		}

	};

	control_base* control_base::find(point p)
	{
		control_base* result = nullptr;

		if (rectangle_math::contains(inner_bounds, p.x, p.y))
		{
			result = this;
			for (auto child : children)
			{
				auto temp = child->find(p);
				if (temp) {
					return temp;
				}
			}
		}

		return result;
	}

	control_base* control_base::find(int _id)
	{
		control_base* root = (control_base *)this;
		while (root->parent) 
		{
			root = (control_base*)root->parent;
		}
		control_base* result = control_base::get(root, _id);
		return result;
	}

	control_base* control_base::get(control_base* _root, int _id)
	{
		control_base* result = _root->find_if([_id](control_base* c) { return c->id == _id; });
		return result;
	}

	control_base* control_base::find_if(std::function<bool(control_base* _root)> _item)
	{
		if (_item(this))
			return this;
		for (auto child : children) {
			auto result = child->find_if(_item);
			if (result)
				return result;
		}
		return nullptr;
	}

	void control_base::foreach(std::function<void(control_base* _root)> _item)
	{
		_item(this);
		for (auto child : children) {
			child->foreach(_item);
		}
	}

	void control_base::create(std::weak_ptr<applicationBase> _host)
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

	void control_base::render(ID2D1DeviceContext* _dest)
	{
		for (auto child : children) {
			child->render(_dest);
		}
	}

	void control_base::apply_item_sizes(control_base& _ref)
	{

	}

	bool control_base::set_mouse(point _position,
		bool* _left_down,
		bool* _right_down,
		std::function<void(control_base* _item)> _left_click,
		std::function<void(control_base* _item)> _right_click
	)
	{
		mouse_relative_position = {};
		mouse_over = rectangle_math::contains(bounds, _position.x, _position.y);

		if (mouse_over)
		{
			mouse_relative_position = _position;
			mouse_relative_position.x -= bounds.x;
			mouse_relative_position.y -= bounds.y;
			if (_left_down)
			{
				mouse_left_down = *_left_down;
			}
			if (_right_down)
			{
				mouse_right_down = *_right_down;
			}
		}
		else
		{
			mouse_left_down = false;
			mouse_right_down = false;
		}

		if (mouse_left_down.changed_to(false) && _left_click) {
			_left_click(this);
		}

		if (mouse_right_down.changed_to(false) && _right_click) {
			_right_click(this);
		}

		for (auto child : children)
		{
			child->set_mouse(_position, _left_down, _right_down, _left_click, _right_click);
		}
		return mouse_over;
	}
	double control_base::to_pixels(measure length)
	{
		double sz = 0.0;

		control_base& pi = *this;

		switch (length.units) {
		case measure_units::pixels:
			sz = length.amount;
			break;
		case measure_units::percent_container:
			sz = length.amount * bounds.w;
			break;
		case measure_units::percent_remaining:
			sz = length.amount * bounds.w;
			break;
		case measure_units::font:
		case measure_units::font_golden_ratio:
			double font_height = 12.0;
			sz = font_height * pi.box.width.amount;
			if (pi.box.width.units == measure_units::font_golden_ratio)
			{
				sz /= 1.618;
			}
			break;
		}
		return sz;
	}

	point control_base::get_size(rectangle _ctx, point _remaining)
	{
		point sz;

		control_base& pi = *this;
		auto mm = to_pixels(margin);

		if (pi.box.width.units == measure_units::pixels)
		{
			sz.x = pi.box.width.amount;
			sz.x += mm * 2.0;
		}
		else if (box.width.units == measure_units::percent_remaining)
		{
			sz.x = box.width.amount * _remaining.x;
		}
		else if (pi.box.width.units == measure_units::percent_container)
		{
			sz.x = pi.box.width.amount * _ctx.w;
		}
		else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
		{
			double font_height = get_font_size();
			sz.x = font_height * pi.box.width.amount;
			if (pi.box.width.units == measure_units::font_golden_ratio)
			{
				sz.x /= 1.618;
			}
			sz.x += mm * 2.0;
		}

		if (pi.box.height.units == measure_units::pixels)
		{
			sz.y = pi.box.height.amount;
			sz.y += mm * 2.0;
		}
		else if (box.height.units == measure_units::percent_remaining)
		{
			sz.y = pi.box.height.amount * _remaining.y;
		}
		else if (pi.box.height.units == measure_units::percent_container)
		{
			sz.y = pi.box.height.amount * _ctx.h;
		}
		else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
		{
			double font_height = get_font_size();
			sz.y = font_height * pi.box.height.amount;
			if (pi.box.height.units == measure_units::font_golden_ratio)
			{
				sz.y *= 1.618;
			}
			sz.y += mm * 2.0;
		}

		if (box.width.units == measure_units::percent_aspect)
		{
			sz.x = box.width.amount * bounds.h;
		}

		if (box.height.units == measure_units::percent_aspect)
		{
			sz.y = box.height.amount * bounds.w;
		}

		if (sz.x < 0)
			sz.x = 0;
		if (sz.y < 0)
			sz.y = 0;

		return sz;
	}

	point control_base::get_position(rectangle _ctx)
	{
		point pos;

		switch (box.x.units)
		{
		case measure_units::percent_container:
		case measure_units::percent_remaining:
			pos.x = box.x.amount * _ctx.w;
			break;
		case measure_units::pixels:
			pos.x = box.x.amount;
			break;
		case measure_units::font:
		case measure_units::font_golden_ratio:
		case measure_units::percent_aspect:
		case measure_units::percent_child:
			throw std::logic_error("font, aspect and child units cannot be used for position");
			break;
		default:
			pos.x = 0;
			break;
		}

		switch (box.y.units)
		{
		case measure_units::percent_container:
		case measure_units::percent_remaining:
			pos.y = box.y.amount * _ctx.h;
			break;
		case measure_units::pixels:
			pos.y = box.y.amount;
			break;
		case measure_units::font:
		case measure_units::font_golden_ratio:
		case measure_units::percent_aspect:
		case measure_units::percent_child:
			throw std::logic_error("font, aspect and child units cannot be used for position");
			break;
		default:
			pos.y = 0;
			break;
		}

		return pos;
	}

	point control_base::get_remaining(point _ctx)
	{
		point pt = { 0.0, 0.0, 0.0 };
		pt = _ctx - pt;
		return pt;
	}
	bool control_base::contains(point pt)
	{
		return rectangle_math::contains(bounds, pt.x, pt.y);
	}

	rectangle& control_base::set_bounds(rectangle& _bounds)
	{
		bounds = _bounds;

		margin_amount.x = margin_amount.y = to_pixels(margin);
		padding_amount.x = padding_amount.y = to_pixels(padding);

		if (parent)
		{
			auto pparent = (control_base *)(parent);
			if (pparent) {
				auto pbounds = pparent->get_inner_bounds();
				if (bounds.x < pbounds.x)
					bounds.x = pbounds.x;
				if (bounds.y < pbounds.y)
					bounds.y = pbounds.y;
				if (bounds.right() > pbounds.right())
				{
					bounds.w -= (bounds.right() - pbounds.right());
				}
				if (bounds.bottom() > pbounds.bottom())
				{
					bounds.h -= (bounds.bottom() - pbounds.bottom());
				}
				if (bounds.w < 0) bounds.w = 0;
				if (bounds.h < 0) bounds.h = 0;
			}
			else
			{

			}
		}

		inner_bounds = bounds;

		inner_bounds.x += margin_amount.x;
		inner_bounds.y += margin_amount.y;
		inner_bounds.w -= (margin_amount.x * 2);
		inner_bounds.h -= (margin_amount.y * 2);

		if (inner_bounds.w < 0) inner_bounds.w = 0;
		if (inner_bounds.h < 0) inner_bounds.h = 0;

		for (auto pr : push_requests) {
			auto target = find(pr.dest_control_id);

			if (target) {
				auto temp_bounds = target->bounds;
				if (pr.properties_to_push & control_push_property::cp_left_bounds)
				{
					temp_bounds.x = bounds.x;
				}
				if (pr.properties_to_push & control_push_property::cp_top_bounds)
				{
					temp_bounds.y = bounds.y;
				}
				if (pr.properties_to_push & control_push_property::cp_right_bounds)
				{
					temp_bounds.x += bounds.right() - temp_bounds.right();
				}
				if (pr.properties_to_push & control_push_property::cp_bottom_bounds)
				{
					temp_bounds.x += bounds.bottom() - temp_bounds.bottom();
				}
				target->arrange(temp_bounds);
			}
		}

		on_resize();
		return bounds;
	}

	void control_base::arrange(rectangle _bounds)
	{
		set_bounds(_bounds);
	}

	void control_base::arrange_children(rectangle _bounds,
		std::function<point(const rectangle* _bounds, control_base*)> _initial_origin,
		std::function<point(point* _origin, const rectangle* _bounds, control_base*)> _next_origin)
	{
		point origin = { _bounds.x, _bounds.y, 0.0 };
		point remaining = { _bounds.w, _bounds.h, 0.0 };

		if (children.size()) {

			auto sichild = std::begin(children);

			origin = _initial_origin(&_bounds, sichild->get());

			remaining = get_remaining(remaining);

			while (sichild != std::end(children))
			{
				auto child = *sichild;

				auto sz = child->get_size(_bounds, remaining);
				auto pos = child->get_position(_bounds);

				rectangle new_bounds;
				new_bounds.x = origin.x + pos.x;
				new_bounds.y = origin.y + pos.y;
				new_bounds.w = sz.x;
				new_bounds.h = sz.y;

				child->arrange(new_bounds);

				origin = _next_origin(&origin, &bounds, child.get());

				sichild++;
			}

		}
	}

	void control_base::on_resize()
	{
		auto ti = typeid(*this).name();
		//std::cout << "resize control_base:" << ti << " " << bounds.x << "," << bounds.y << " x " << bounds.w << " " << bounds.h << std::endl;
	}

	using control_json_mapper = std::function<std::weak_ptr<control_base>(control_base *_parent, json& _array, int _index)>;

	class array_data_source
	{
	public:
		json		data;
		control_json_mapper data_to_control;
	};

}

#endif
