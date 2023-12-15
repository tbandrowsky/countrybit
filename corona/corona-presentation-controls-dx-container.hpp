#ifndef CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H
#define CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H

#include "corona-presentation-base.hpp"
#include "corona-presentation-controls-base.hpp"
#include "corona-presentation-controls-dx.hpp"
#include "corona-presentation-controls-dx-text.hpp"
#include "corona-presentation-controls-win32.hpp"

namespace corona
{


	class container_control : public draw_control
	{

	public:

		layout_rect				item_box = {};
		measure					item_margin = {};

		visual_alignment		content_alignment = visual_alignment::align_near;
		visual_alignment		content_cross_alignment = visual_alignment::align_near;

		container_control()
		{
			parent = nullptr;
			id = id_counter::next();
		}

		container_control(const container_control& _src) = default;

		container_control(container_control_base *_parent, int _id)
		{
			parent = _parent;
			id = _id;
			if (parent && get_nchittest() == HTCLIENT) {
				set_nchittest(parent->get_nchittest());
			}
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<container_control>(*this);
			return tv;
		}

		virtual ~container_control()
		{
			;
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}

		container_control& set_content_align(visual_alignment _new_alignment)
		{
			content_alignment = _new_alignment;
			return *this;
		}

		container_control& set_content_cross_align(visual_alignment _new_alignment)
		{
			content_cross_alignment = _new_alignment;
			return *this;
		}

		container_control& set_item_origin(measure _x, measure _y)
		{
			item_box.x = _x;
			item_box.y = _y;
			return *this;
		}

		container_control& set_item_size(measure _width, measure _height)
		{
			if (_width.amount > 0)
				item_box.width = _width;
			if (_height.amount > 0)
				item_box.height = _height;
			return *this;
		}

		container_control& set_item_position(layout_rect _new_layout)
		{
			item_box = _new_layout;
			return *this;
		}

		container_control& set_item_margin(measure _item_margin)
		{
			item_margin = _item_margin;
			return *this;
		}


		container_control& set_origin(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return *this;
		}

		container_control& set_size(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return *this;
		}

		container_control& set_background_color(solidBrushRequest _brushFill)
		{
			background_brush = _brushFill;
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& set_background_color(std::string _color)
		{
			background_brush.brushColor = toColor(_color.c_str());
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		container_control& set_margin(measure _item_space)
		{
			margin = _item_space;
			return *this;
		}

		// default implementation for composed controls
		virtual void arrange(rectangle _ctx)
		{
			set_bounds(_ctx);
			for (auto child : children) {
				child->arrange(_ctx);
			}
		}
	};

	class absolute_layout :
		public container_control
	{
	public:
		absolute_layout() { ; }
		absolute_layout(const absolute_layout& _src) = default;
		absolute_layout(container_control_base *_parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~absolute_layout() { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<absolute_layout>(*this);
			return tv;
		}

		virtual void arrange(rectangle _ctx);
	};

	class column_layout :
		public container_control
	{
		layout_rect item_size;
	public:
		column_layout() { ; }
		column_layout(const column_layout& _src) = default;
		column_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~column_layout() { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<column_layout>(*this);
			return tv;
		}


		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
	};

	class row_layout :
		public container_control
	{
	protected:
	public:
		row_layout() { ; }
		row_layout(const row_layout& _src) = default;
		row_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~row_layout() { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<row_layout>(*this);
			return tv;
		}

		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
	};

	class frame_layout :
		public container_control
	{
	protected:
	public:
		frame_layout() { ; }
		frame_layout(const frame_layout& _src) = default;
		frame_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~frame_layout() { ; }

		void set_contents(control_base* _page)
		{
			children.clear();
			auto temp = _page->clone();
			temp->parent = this;
			apply_item_sizes(*temp);
			children.push_back(temp);
			arrange(bounds);
			create(host);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<frame_layout>(*this);
			return tv;
		}


		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
	};


	class column_view_layout :
		public column_layout
	{
		array_data_source item_source;
		std::vector<std::shared_ptr<control_base>> items;
		std::vector<int> page_to_item_index;
		std::vector<int> item_to_page_index;
		// we keep the set of controls here on the back end, because they are small as they are not dragging around any 
		// back end bitmaps or windows.  (arranging doesn't create the assets on a control, create does)
		rectangle view_port;
		rectangle child_area;

		int selected_page_index;
		int selected_item_index;

		void position_children()
		{
			child_area = bounds;
			view_port.w = bounds.w;
			view_port.h = bounds.h;
			children.clear();

			for (auto item : items)
			{
				child_area = rectangle_math::join(child_area, item->get_bounds());
				if (rectangle_math::contains(view_port, item->get_bounds().x, item->get_bounds().y) ||
					rectangle_math::contains(view_port, item->get_bounds().right(), item->get_bounds().y) ||
					rectangle_math::contains(view_port, item->get_bounds().right(), item->get_bounds().bottom()) ||
					rectangle_math::contains(view_port, item->get_bounds().x, item->get_bounds().bottom()))
				{
					children.push_back(item);
				}
			}
		}

		void check_scroll()
		{
			if (selected_item_index > item_source.data.size())
			{
				selected_item_index = item_source.data.size() - 1;
				selected_page_index = item_to_page_index[selected_item_index];
			}
			if (selected_item_index < 0)
			{
				selected_item_index = 0;
				selected_page_index = 0;
			}
		}

	public:

		column_view_layout()
		{
			view_port = {};
			child_area = {};
			selected_item_index = 0;
		}

		column_view_layout(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			view_port = {};
			child_area = {};
			selected_item_index = 0;
		}

		column_view_layout(const column_view_layout& _src) = default;

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<column_view_layout>(*this);
			return tv;
		}


		virtual ~column_view_layout()
		{
			;
		}

		void set_item_source(array_data_source _item_source)
		{
			item_source = _item_source;
			selected_item_index = 0;
		}

		virtual void arrange(rectangle _ctx)
		{
			children.clear();
			items.clear();

			int i;
			for (i = 0; i < item_source.data.size(); i++)
			{
				auto cb = item_source.data_to_control(this, item_source.data, i);
				if (auto sp = cb.lock()) {
					children.push_back(sp);
					items.push_back(sp);
				}
			}

			column_layout::arrange(_ctx);

			page_to_item_index.clear();
			item_to_page_index.clear();
			view_port.w = _ctx.w;
			view_port.h = _ctx.h;
			double h = 0.0;
			int current_page = -1;
			int index = 0;
			for (auto item : items)
			{
				h += item->get_bounds().h;
				if (h > view_port.h || current_page < 0) {
					current_page++;
					page_to_item_index.push_back(index);
				}
				item_to_page_index.push_back(current_page);
				index++;
			}
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			_page->on_key_down(id, [](key_down_event evt) {
				});
			_page->on_key_up(id, [](key_up_event evt) {
					column_view_layout* cvt = (column_view_layout*)evt.control;
					switch (evt.key) {
					case VK_BACK:
						break;
					case VK_UP:
						cvt->line_up();
						break;
					case VK_DOWN:
						cvt->line_down();
						break;
					case VK_PRIOR:
						cvt->page_up();
						break;
					case VK_NEXT:
						cvt->page_down();
						break;
					case VK_DELETE:
						cvt->delete_selected();
						break;
					case VK_RETURN:
						cvt->navigate_selected();
						break;
					}
				});

			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}

		void line_down()
		{
			selected_item_index--;
			check_scroll();

			auto& temp = items[selected_item_index];
			view_port.y -= temp->get_bounds().h;
			position_children();
		}

		void line_up()
		{
			selected_item_index++;
			check_scroll();
			auto& temp = items[selected_item_index];
			view_port.y += temp->get_bounds().h;
			position_children();
		}

		void page_up()
		{
			selected_page_index--;
			if (selected_page_index < 0)
				selected_page_index = 0;
			selected_item_index = page_to_item_index[selected_page_index];
			view_port.y = items[selected_item_index].get()->get_bounds().y;
			view_port.x = items[selected_item_index].get()->get_bounds().x;
			position_children();
		}

		void page_down()
		{
			selected_page_index++;
			if (selected_page_index >= page_to_item_index.size())
				selected_page_index = page_to_item_index.size() - 1;
			selected_item_index = page_to_item_index[selected_page_index];
			view_port.y = items[selected_item_index].get()->get_bounds().y;
			view_port.x = items[selected_item_index].get()->get_bounds().x;
			position_children();
		}

		void home()
		{
			selected_item_index = 0;
			view_port.y = 0;
			position_children();
		}

		void end()
		{
			selected_item_index = item_source.data.size() - 1;
			auto& temp = items[selected_item_index];
			view_port.y = temp->get_bounds().y;
			position_children();
		}

		void navigate_selected()
		{
			;
		}

		void delete_selected()
		{
			;
		}

	};

	class row_view_layout :
		public row_layout
	{
	protected:
	public:
		row_view_layout() { ; }
		row_view_layout(const row_view_layout& _src) = default;
		row_view_layout(container_control_base* _parent, int _id) : row_layout(_parent, _id) { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<row_view_layout>(*this);
			return tv;
		}

		virtual ~row_view_layout() { ; }
	};

	class absolute_view_layout :
		public absolute_layout
	{
	protected:
	public:
		absolute_view_layout() { ; }
		absolute_view_layout(const absolute_view_layout& _src) = default;
		absolute_view_layout(container_control_base* _parent, int _id) : absolute_layout(_parent, _id) { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<absolute_view_layout>(*this);
			return tv;
		}

		virtual ~absolute_view_layout() { ; }
	};


	point row_layout::get_remaining(point _ctx)
	{
		point pt = { 0.0, 0.0, 0.0 };

		for (auto child : children)
		{
			auto sz = child->get_size(bounds, { 0.0, 0.0 });
			pt.x += sz.x;
		}

		pt = _ctx - pt;
		return pt;
	}

	point column_layout::get_remaining(point _ctx)
	{
		point pt = { 0.0, 0.0, 0.0 };

		for (auto child : children)
		{
			if (child->box.height.units != measure_units::percent_remaining)
			{
				pt.y += child->bounds.h;
			}
		}

		pt = _ctx - pt;
		return pt;
	}

	void absolute_layout::arrange(rectangle _bounds)
	{
		set_bounds(_bounds);

		point origin = { _bounds.x, _bounds.y, 0.0 };
		point remaining = { _bounds.w, _bounds.h, 0.0 };

		arrange_children(bounds,
			[this](const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			},
			[this](point* _origin, const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			}
		);
	}

	void row_layout::arrange(rectangle _bounds)
	{
		point origin = { 0, 0, 0 };
		set_bounds(_bounds);

		if (content_alignment == visual_alignment::align_near)
		{
			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {
					point temp = { 0, 0, 0 };
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
						temp.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x;
						temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x;
						temp.y = _bounds->y + (_bounds->h - sz.y);
					}
					return temp;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x;
					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y);
					}
					return temp;
				}
			);

		}
		else if (content_alignment == visual_alignment::align_far)
		{
			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {

					double w = 0;
					point remaining = { 0, 0, 0 };
					remaining.x = _bounds->w;
					remaining.y = _bounds->h;
					remaining = this->get_remaining(remaining);

					for (auto child : children)
					{
						auto sz = child->get_size(*_bounds, remaining);
						w += sz.x;
					}

					auto sz = _item->get_size(bounds, remaining);

					point temp = { 0, 0, 0 };
					temp.x = _bounds->right() - w;

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y);
					}

					return temp;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x;
					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y);
					}

					return temp;
				}
			);
		}
		else if (content_alignment == visual_alignment::align_center)
		{

			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {

					double w = 0.0;
					point origin = { 0, 0, 0 };
					point remaining = { 0, 0, 0 };
					remaining.x = _bounds->w;
					remaining.y = _bounds->h;
					remaining = this->get_remaining(remaining);

					for (auto child : children)
					{
						auto sz = child->get_size(*_bounds, remaining);
						w += sz.x;
					}

					origin.x = (bounds.x + bounds.w - w) / 2;
					origin.y = bounds.y;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						origin.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						origin.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						origin.y = _bounds->y + (_bounds->h - sz.y);
					}

					return origin;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x;

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.y = _bounds->y;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.y = _bounds->y + (_bounds->h - sz.y);
					}
					return temp;
				}
			);
		}
	}

	void column_layout::arrange(rectangle _bounds)
	{
		point origin = { 0, 0, 0 };

		set_bounds(_bounds);

		if (content_alignment == visual_alignment::align_near)
		{
			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {
					point temp = { 0, 0, 0 };
					temp.x = _bounds->x;
					temp.y = _bounds->y;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x);
					}

					return temp;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.y += sz.y;

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x);
					}

					return temp;
				}
			);

		}
		else if (content_alignment == visual_alignment::align_far)
		{
			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {
					point temp = { 0, 0, 0 };

					double h = 0;
					point remaining = { };
					remaining.x = _bounds->w;
					remaining.y = _bounds->h;
					remaining = this->get_remaining(remaining);

					for (auto child : children)
					{
						auto sz = child->get_size(*_bounds, remaining);
						h += sz.y;
					}

					temp.x = _bounds->x;
					temp.y = _bounds->y + _bounds->h - h;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x);
					}

					return temp;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto size = _item->get_size(bounds, { bounds.w, bounds.h });
					temp.y += (size.y);

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x + (_bounds->w - size.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x + (_bounds->w - size.x);
					}

					return temp;
				}
			);
		}
		else if (content_alignment == visual_alignment::align_center)
		{

			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {

					double h = 0.0;
					point origin = { 0, 0, 0 };
					point remaining = { 0, 0, 0 };
					remaining.x = _bounds->w;
					remaining.y = _bounds->h;
					remaining = this->get_remaining(remaining);

					for (auto child : children)
					{
						auto sz = child->get_size(*_bounds, remaining);
						h += sz.y;
					}

					origin.x = bounds.x;
					origin.y = (bounds.y + bounds.h - h) / 2;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						origin.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						origin.x = _bounds->x + (_bounds->w - sz.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						origin.x = _bounds->x + (_bounds->w - sz.x);
					}

					return origin;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.y += sz.y;

					if (this->content_cross_alignment == visual_alignment::align_near)
					{
						temp.x = _bounds->x;
					}
					else if (this->content_cross_alignment == visual_alignment::align_center)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x) / 2.0;
					}
					else if (this->content_cross_alignment == visual_alignment::align_far)
					{
						temp.x = _bounds->x + (_bounds->w - sz.x);
					}
					return temp;
				}
			);
		}
	}

	void frame_layout::arrange(rectangle _bounds)
	{
		set_bounds(_bounds);

		point origin = { _bounds.x, _bounds.y, 0.0 };
		point remaining = { _bounds.w, _bounds.h, 0.0 };

		arrange_children(bounds,
			[this](const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			},
			[this](point* _origin, const rectangle* _bounds, control_base* _item) {
				return *_origin;
			}
		);
	}

	point frame_layout::get_remaining(point _ctx)
	{
		return _ctx;
	}

}

#endif
