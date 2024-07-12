/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is for controls using direct2d to draw their own contexts and are containers

Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H
#define CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H

namespace corona
{

	template <typename DATA> class contents_generator
	{
	public:
		DATA data;
		std::function<void(DATA& data, control_base* generator)> generator;

		void operator() ( control_base* _this )
		{
			generator(data, _this);
		}
	};

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

		virtual void set_contents(std::function<void(control_base* _page)> _contents)
		{
			_contents(this);

			for (auto child : children) {
				child->parent = this;
			}

			arrange(bounds);
			create(host);
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


		container_control& set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		container_control& set_margin(measure _item_space)
		{
			margin = _item_space;
			calculate_margins();
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

		virtual void get_json(json& _dest)
		{
			draw_control::get_json(_dest);

			corona::get_json(_dest, "content_alignment", content_alignment);
			corona::get_json(_dest, "content_cross_alignment", content_cross_alignment);
		}

		virtual void put_json(json& _src)
		{
			draw_control::put_json(_src);

			corona::put_json(content_alignment, _src, "content_alignment" );
			corona::put_json(content_cross_alignment, _src, "content_cross_alignment");
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

		measure item_start_space;
		measure item_next_space;
		measure column_width;
		int		column_count;

	public:
		column_layout() : column_count(0), column_width(300.0_px) { ; }
		column_layout(const column_layout& _src) = default;
		column_layout(container_control_base* _parent, int _id) : container_control(_parent, _id), column_count(0), column_width(300.0_px) { ; }

		virtual ~column_layout() { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<column_layout>(*this);
			return tv;
		}


		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);

		void set_column_width(measure _column_width)
		{
			column_width = _column_width;
		}
		measure get_column_width()
		{
			return column_width;
		}
	};

	class row_layout :
		public container_control
	{
	protected:
	public:

		measure item_start_space;
		measure item_next_space;

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

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<frame_layout>(*this);
			return tv;
		}

		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);


	};

	class grid_view_row
	{
	public:
		int item_id;
		int page_index;
		rectangle bounds;
	};

	class grid_view : public draw_control
	{
		array_data_source items_source;
		std::vector<grid_view_row> rows;
		std::map<int,int> page_to_item_index;
		solidBrushRequest selection_border;

		// we keep the set of controls here on the back end, because they are small as they are not dragging around any 
		// back end bitmaps or windows.  (arranging doesn't create the assets on a control, create does)
		rectangle view_port;

		int selected_page_index;
		int selected_item_index;

		void check_scroll()
		{
			if (selected_item_index >= rows.size() - 1)
			{
				selected_item_index = rows.size() - 1;
			}
			if (selected_item_index < 0)
			{
				selected_item_index = 0;
			}
			selected_page_index = rows[selected_item_index].page_index;
			int selected_item_page_index = page_to_item_index[selected_page_index];
			view_port.y = rows[selected_item_page_index].bounds.y;
			std::cout << selected_page_index << " " << selected_item_index << " " << selected_item_page_index << " " << view_port.y << std::endl;
		}

		void set_selection_border(solidBrushRequest _brushFill)
		{
			selection_border = _brushFill;
			selection_border.name = typeid(*this).name();
			selection_border.name +="_selection";
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&selection_border);
			}
		}

		void set_selection_border(std::string _color)
		{
			selection_border.brushColor = toColor(_color.c_str());
			selection_border.name = typeid(*this).name();
			selection_border.name += "_selection";
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&selection_border);
			}
		}

		void init()
		{
			on_create = [this](draw_control *_src)
				{
					if (items_source.assets)
					{
						items_source.assets(this, bounds);
					}

					if (auto pwindow = _src->window.lock())
					{
						pwindow->getContext().setSolidColorBrush(&selection_border);
					}
				};

			on_draw = [this](control_base* _item)
				{
					if (auto pwindow = window.lock())
					{
						if (auto phost = host.lock()) {

							auto draw_bounds = inner_bounds;

							draw_bounds.x = inner_bounds.x - bounds.x;
							draw_bounds.y = inner_bounds.y - bounds.y;

							point offset = { view_port.x, view_port.y };

							auto& context = pwindow->getContext();

							if (!page_to_item_index.contains(selected_page_index )) {
								std::cout << "selected_page_index not found." << std::endl;
								return;
							}

							int idx = page_to_item_index[ selected_page_index ];

							while (idx < rows.size())
							{
								auto& row = rows[idx];

								auto rect_bounds = row.bounds;
								rect_bounds.x -= offset.x;
								rect_bounds.y -= offset.y;

								if (rect_bounds.y < bounds.bottom()) 
								{
									items_source.draw_item(this, idx, items_source.data, rect_bounds);
									if (selected_item_index == idx) 
									{
										context.drawRectangle(&rect_bounds, selection_border.name, 4, nullptr);
									}
									context.drawText("Test", &rect_bounds, "" ,selection_border.name);
								}
								else 
								{
									break;
								}

								idx++;
							}
						}
					}
				};

		}

	public:

		grid_view()
		{
			view_port = {};
			selected_item_index = 0;
			selected_page_index = 0;
			set_selection_border("#000000");
			init();
		}

		grid_view(container_control_base* _parent, int _id) : draw_control(_parent, _id)
		{
			view_port = {};
			selected_item_index = 0;
			selected_page_index = 0;
			set_selection_border("#000000");
			init();
		}

		grid_view(const grid_view& _src) = default;

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<grid_view>(*this);
			return tv;
		}

		virtual ~grid_view()
		{
			;
		}

		virtual void arrange(rectangle _bounds)
		{
			draw_control::arrange(_bounds);

			view_port.w = _bounds.w;
			view_port.h = _bounds.h;

			if (_bounds.h == 0 || _bounds.w == 0) 
			{
				return;
			}

			double h = _bounds.h;
			double y = 0;

			int page_index;
			page_to_item_index.clear();

			int sz = rows.size();
			int i;
			for (i = 0; i < sz; i++)
			{
				json temp;
				auto isz = items_source.size_item(this, i, temp, _bounds);
				auto& r = rows[i];
				r.bounds.x = 0;
				r.bounds.y = y;
				r.bounds.w = _bounds.w;
				r.bounds.h = isz.y;
				page_index = (r.bounds.bottom() / h);
				r.page_index = page_index;
				if (!page_to_item_index.contains(page_index))
				{
					page_to_item_index[page_index] = i;
				}
				y += isz.y;
			}
		}

		void set_item_source(array_data_source _item_source)
		{
			items_source = _item_source;
			rows.clear();

			int i;
			rectangle item_bounds;
			item_bounds.x = 0;
			item_bounds.y = 0;
			item_bounds.w = 0;
			item_bounds.h = 0;

			for (i = 0; i < items_source.data.size(); i++)
			{
				grid_view_row gvr;
				gvr.page_index = 0;
				gvr.bounds = item_bounds;
				gvr.item_id = i;
				rows.push_back(gvr);
			}

			if (auto pwindow = window.lock()) {
				arrange(bounds);

				if (rows.size() <= selected_item_index)
				{
					selected_item_index = 0;
					check_scroll();
				}
			}

		}

		virtual void key_down(int _key)
		{
			switch (_key) {
			case VK_BACK:
				break;
			case VK_UP:
				line_up();
				break;
			case VK_DOWN:
				line_down();
				break;
			case VK_PRIOR:
				page_up();
				break;
			case VK_NEXT:
				page_down();
				break;
			case VK_HOME:
				home();
				break;
			case VK_END:
				end();
				break;
			case VK_DELETE:
				delete_selected();
				break;
			case VK_RETURN:
				navigate_selected();
				break;
			}
		}

		void line_down()
		{
			selected_item_index++;
			check_scroll();
		}

		void line_up()
		{
			selected_item_index--;
			check_scroll();
		}

		void page_up()
		{
			selected_page_index--;
			if (selected_page_index < 0)
				selected_page_index = 0;
			selected_item_index = page_to_item_index[selected_page_index];
			check_scroll();
		}

		void page_down()
		{
			selected_page_index++;
			if (selected_page_index >= page_to_item_index.size())
				selected_page_index = page_to_item_index.size() - 1;
			selected_item_index = page_to_item_index[selected_page_index];
			check_scroll();
		}

		void home()
		{
			selected_item_index = 0;
			check_scroll();
		}

		void end()
		{
			selected_item_index = items_source.data.size() - 1;
			check_scroll();
		}

		void navigate_selected()
		{
			;
		}

		void delete_selected()
		{
			;
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			_page->on_key_down(id, [](key_down_event evt)
				{
					grid_view* gv = dynamic_cast<grid_view *>(evt.control);
					if (gv) {
						gv->key_down(evt.key);
					}
				});
			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
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
			if (child->box.width.units != measure_units::percent_remaining)
			{
				auto sz = child->get_size(bounds, { 0.0, 0.0 });
				pt.x += sz.x;
			}
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
			[this](point _remaining, const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			},
			[this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			},
			[this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
				point temp = { _bounds->x, _bounds->y };
				return temp;
			}
		);
	}

	void row_layout::arrange(rectangle _bounds)
	{
		point origin = { 0, 0, 0 };
		set_bounds(_bounds);

		double item_start_space_px = to_pixels_x(item_start_space);
		double item_next_space_px = to_pixels_x(item_next_space);

		std::function<point(point _remaining, point* _origin, const rectangle* _bounds, control_base* _item)> align_item = [this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) -> point
		{
			point temp = { 0, 0, 0 };

			auto sz = _item->get_size(inner_bounds, { _bounds->w, _bounds->h });

			if (this->content_cross_alignment == visual_alignment::align_near)
			{
				temp.x = _origin->x;
				temp.y = _bounds->y;
			}
			else if (this->content_cross_alignment == visual_alignment::align_center)
			{
				temp.x = _origin->x;
				temp.y = _bounds->y + (_bounds->h - sz.y) / 2.0;
			}
			else if (this->content_cross_alignment == visual_alignment::align_far)
			{
				temp.x = _origin->x;
				temp.y = _bounds->y + (_bounds->h - sz.y);
			}

			return temp;
		};

		if (content_alignment == visual_alignment::align_near)
		{
			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {
					point temp = { _bounds->x, _bounds->y };
					temp.x += _item->get_margin_amount().x;
					temp.y += _item->get_margin_amount().y;
					temp.x += item_start_space_px;
					return temp;
				},
				align_item,
				[this, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x + item_next_space_px;
					temp.x += _item->get_margin_amount().x;
					return temp;
				}
			);
		}
		else if (content_alignment == visual_alignment::align_far)
		{
			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {

					double w = 0;

					w = _item->margin.amount;
					for (auto child : children)
					{
						child->calculate_margins();
						auto sz = child->get_size(*_bounds, _remaining);
						w += sz.x;
						w += child->get_margin_amount().x;
					}

					point temp = { 0, 0, 0 };
					temp.x = (_bounds->right() - w);
					temp.x += item_start_space_px;
					return temp;
				},
				align_item,
				[this, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x;
					temp.x += _item->get_margin_amount().x;
					temp.x += item_next_space_px;
					return temp;
				}
			);
		}
		else //  DOESN'T MATTER AND ALLOWS FOR A CORRUPTION if (content_alignment == visual_alignment::align_center)
		{
			content_alignment = visual_alignment::align_center;
			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {

					double w = 0.0;
					point origin = { 0, 0, 0 };
					point remaining = { 0, 0, 0 };
					remaining.x = _bounds->w;
					remaining.y = _bounds->h;
					remaining = this->get_remaining(remaining);

					w = _item->margin.amount;
					for (auto child : children)
					{
						auto sz = child->get_size(*_bounds, remaining);
						w += sz.x;
						w += child->get_margin_amount().x;
					}

					origin.x = (bounds.x + bounds.w - w) / 2;
					origin.y = bounds.y;
					origin.x += item_start_space_px;

					return origin;
				},
				align_item,
				[this, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.x += sz.x;
					temp.x += get_margin_amount().x;
					temp.x += item_next_space_px;
					return temp;
				}
			);
		} 
	}

	void column_layout::arrange(rectangle _bounds)
	{
		point origin = { 0, 0, 0 };

		set_bounds(_bounds);

		double item_start_space_px = to_pixels_y(item_start_space);
		double item_next_space_px = to_pixels_y(item_next_space);

		std::function<point(point _remaining, point* _origin, const rectangle* _bounds, control_base* _item)> align_item = [this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) -> point
		{
			point temp = { 0, 0, 0 };

			auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });

			if (this->content_cross_alignment == visual_alignment::align_near)
			{
				temp.x = _bounds->x + _item->get_margin_amount().x;
				temp.y = _origin->y;
			}
			else if (this->content_cross_alignment == visual_alignment::align_center)
			{
				temp.x = _bounds->x + (_bounds->w + _item->get_margin_amount().x * 2 - sz.x) / 2.0;
				temp.y = _origin->y;
			}
			else if (this->content_cross_alignment == visual_alignment::align_far)
			{
				temp.x = _bounds->x + (_bounds->w - _item->get_margin_amount().x * 2 - sz.x);
				temp.y = _origin->y;
			}

			return temp;
		};

		if (content_alignment == visual_alignment::align_near)
		{
			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {
					point temp = { 0, 0, 0 };
					temp.x = _bounds->x + _item->get_margin_amount().x;
					temp.y = _bounds->y + _item->get_margin_amount().y + item_start_space_px;
					return temp;
				},
				align_item,
				[this, item_start_space_px, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.y += sz.y;
					temp.y += _item->get_margin_amount().y + item_next_space_px;
					auto width_pixels = to_pixels_x(column_width);
					if (temp.y > _bounds->bottom()) {
						column_count++;
						temp.x = _bounds->x + column_count * width_pixels;
						temp.y = _bounds->y + _item->get_margin_amount().y + item_start_space_px;
					}
					return temp;
				}
			);

		}
		else if (content_alignment == visual_alignment::align_far)
		{
			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {
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
						h += child->get_margin_amount().y;
					}

					temp.x = _bounds->x;
					temp.y = _bounds->y + _bounds->h - h + item_start_space_px;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });

					return temp;
				},
				align_item,
				[this, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto size = _item->get_size(bounds, { bounds.w, bounds.h });
					temp.y += (size.y);
					temp.y += _item->get_margin_amount().y;
					temp.y += item_next_space_px;

					return temp;
				}
			);
		}
		else if (content_alignment == visual_alignment::align_center)
		{

			arrange_children(inner_bounds,
				[this, item_start_space_px](point _remaining, const rectangle* _bounds, control_base* _item) {

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
						h += child->get_margin_amount().y;
					}

					origin.x = bounds.x;
					origin.y = (bounds.y + bounds.h - h) / 2;
					origin.y += item_start_space_px;
					return origin;
				},
				align_item,
				[this, item_next_space_px](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.y += sz.y;
					temp.y += item_next_space_px;
					return temp;
				}
			);
		}
		else
		{
			throw std::exception("column content_alignment not set");
		}
	}

	void frame_layout::arrange(rectangle _bounds)
	{
		set_bounds(_bounds);

		point origin = { 0, 0, 0 };
		point remaining = { _bounds.w, _bounds.h, 0.0 };

		arrange_children(inner_bounds,
			[this](point _remaining, const rectangle* _bounds, control_base* _item) {
				point temp = { 0, 0, 0 };
				temp.x = _bounds->x;
				temp.y = _bounds->y;
				return temp;
			},
			[this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
				point temp = *_origin;
				auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
				temp.x = _bounds->x;
				return temp;
			},
			[this](point _remaining, point* _origin, const rectangle* _bounds, control_base* _item) {
				point temp = *_origin;
				auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
				temp.y += sz.y;
				temp.x = _bounds->x;
				return temp;
			}
		);
	}

	point frame_layout::get_remaining(point _ctx)
	{
		return _ctx;
	}

}

#endif
