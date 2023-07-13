#pragma once

namespace corona
{
	namespace database
	{

		enum class layout_types
		{
			canvas2d_row = 0,
			canvas2d_column = 1,
			canvas2d_absolute = 2,
			canvas3d_row = 3,
			canvas3d_column = 4,
			canvas3d_absolute = 5,
			text_window = 6,
			dropdown_window = 7,
			spinner_window = 8,
			hscroll_window = 9,
			vscroll_window = 10,
			space = 11,
			row = 12,
			column = 13,
			absolute = 14,
			label = 15,
			create = 16,
			select = 17,
			table_cell = 18,
			navigate = 19,
			text = 20,
			set = 21,
			table_header = 22
		};

		inline constexpr const char* layout_type_names[] = {
			"canvas2d_row = 0",
			"canvas2d_column = 1",
			"canvas2d_absolute = 2",
			"canvas3d_row = 3",
			"canvas3d_column = 4",
			"canvas3d_absolute = 5",
			"text_window = 6",
			"dropdown_window = 7",
			"spinner_window = 8",
			"hscroll_window = 9",
			"vscroll_window = 10",
			"space = 11",
			"row = 12",
			"column = 13",
			"absolute = 14",
			"label = 15",
			"create = 16",
			"select = 17",
			"table_cell = 18",
			"navigate = 19",
			"text = 20",
			"set = 21",
			"table_header = 22"
		};

		struct page_item_identifier_x
		{
			relative_ptr_type		item_uid;
			relative_ptr_type		class_id;
			relative_ptr_type		object_id;
			relative_ptr_type		field_id;
			int						layout;
		};

		typedef int64_t page_item_identifier;

		class page_item
		{
		public:
			int						id;
			relative_ptr_type		item_uid;
			int						parent_id;
			int						canvas_id;

			layout_types			layout;
			relative_ptr_type		style_id;
			visual_alignment		alignment;

			relative_ptr_type		class_id;

			layout_rect				box;
			measure					item_space;

			rectangle				bounds;
			point					item_space_amount;

			const char*				caption;

			bool					mouse_over;
			bool					pressed;
			bool					selected;

			int						first_child;
			int						last_child;

			int						next_item;

			page_item() :
				id(-1),
				parent_id(-1),
				layout(layout_types::space),
				caption(nullptr),
				canvas_id(-1),
				style_id(null_row),
				item_space(),
				item_space_amount({ 0.0, 0.0 } ),
				item_uid(null_row),
				mouse_over(false),
				pressed(false),
				selected(false),
				first_child(-1),
				last_child(-1),
				next_item(-1)
			{
			}

			page_item* get_base()
			{
				return this - this->id;
			}

			page_item* get_parent()
			{
				return parent_id > -1 ? (get_base() + parent_id) : nullptr;
			}

			page_item* get_canvas()
			{
				return canvas_id > -1 ? (get_base() + canvas_id) : nullptr;
			}

			page_item* get_first_child()
			{
				return first_child > -1 ? (get_base() + first_child) : nullptr;
			}

			page_item* get_last_child()
			{
				return last_child > -1 ? (get_base() + last_child) : nullptr;
			}

			page_item* get_next()
			{
				return next_item > -1 ? (get_base() + next_item) : nullptr;
			}

			void set_parent(page_item* _parent)
			{
				if (_parent) 
				{
					parent_id = _parent->id;
					canvas_id = _parent->canvas_id;
					auto p = get_parent();
					assert(p == _parent);
					auto lc = p->get_last_child();
					if (lc) 
					{
						lc->next_item = id;
						_parent->last_child = id;
					}
					else {
						_parent->first_child = id;
						_parent->last_child = id;
					}
				}
				else 
				{
					parent_id = -1;
					if (layout == layout_types::canvas2d_absolute ||
						layout == layout_types::canvas2d_column ||
						layout == layout_types::canvas2d_row)
						canvas_id = id;
					else
						canvas_id = -1;
				}
			}

			bool is_drawable()
			{
				return canvas_id != -1;
			}

			bool is_canvas_child()
			{
				return canvas_id != -1 && canvas_id != id;
			}

			bool is_select()
			{
				return layout == layout_types::select || layout == layout_types::table_cell || layout == layout_types::navigate;
			}

			bool is_create()
			{
				return layout == layout_types::create;
			}

			bool is_set()
			{
				return layout == layout_types::set || layout == layout_types::table_cell || layout == layout_types::table_header;
			}

			bool is_command()
			{
				return is_create() || is_select() || is_set();
			}

			bool is_canvas2d()
			{
				return layout == layout_types::canvas2d_absolute || layout == layout_types::canvas2d_row || layout == layout_types::canvas2d_column;
			}

			page_item_identifier get_identifier()
			{
				page_item_identifier_x pii { null_row, null_row };
				if ((layout == layout_types::canvas2d_row) ||
					(layout == layout_types::canvas2d_column) ||
					(layout == layout_types::canvas2d_absolute) ||
					(layout == layout_types::canvas3d_row) ||
					(layout == layout_types::canvas3d_column) ||
					(layout == layout_types::canvas3d_absolute))
				{
					pii.layout = (int)layout;
					pii.object_id = 0;
					pii.class_id = 0;
					pii.field_id = 0;
					pii.item_uid = item_uid;
				}
				else 
				{
					pii.layout = (int)layout;
					pii.object_id = 0; // object_path.object.row_id;
					pii.class_id = class_id;
					pii.item_uid = item_uid;
					pii.field_id = 0;
				}
				page_item_identifier c = 17;
				c = c * 23 + pii.item_uid;
				c = c * 23 + pii.layout;
				c = c * 23 + pii.class_id;
				c = c * 23 + pii.field_id;
				c = c * 23 + pii.object_id;
				return c;
			}
		};

		enum class field_layout 
		{
			label_on_left = 1,
			label_on_top = 2
		};

		using page_base_type = iarray<page_item, 1024>;

		class layout_context 
		{
		public:
			point flow_origin;
			point container_origin;
			point container_size;
			point remaining_size;
			point space_amount;
		};

		class page : public page_base_type
		{
			void size_constant(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void size_constants(jobject& _style_sheet, page_item* _item, layout_context _ctx);

			void size_aspect(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void size_aspect_widths(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety);
			void size_aspect_heights(jobject& _style_sheet, page_item* _item, layout_context _ctx, int safety);
			void size_aspects(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void size_children(jobject& _style_sheet, page_item* _item, layout_context _ctx);

			void size_remaining(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void size_remainings(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			layout_context get_remaining(jobject& _style_sheet, page_item* _item, layout_context _ctx);

			void size_item(jobject& _style_sheet, page_item* _pi, layout_context _ctx);
			void size_items(jobject& _style_sheet, page_item* _pi, layout_context _ctx);
			void position(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void positions(jobject& _style_sheet, page_item* _item, layout_context _ctx);
			void styles(jobject& _style_sheet, int style_id, page_item* _item);

			dynamic_box data;

			rectangle layout(jobject& _style_sheet, page_item* _item, layout_context _ctx);

		public:


			page();
			void clear();
			const char* copy(const char* _src)
			{
				return data.copy(_src, 0);
			}

			page_item& operator[](int _id)
			{
				int id_to_idx = _id;
				auto& pi = this->get_at(id_to_idx);
				if (pi.id != _id) {
					throw std::logic_error("something bad happened with page item indeces");
				}
				return pi;
			}

			page_item* append(page_item* _parent, layout_types _layout, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment);

			page_item* row(page_item* _parent, relative_ptr_type _style_id = null_row,  layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* column( page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* absolute(page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, visual_alignment _alignment = visual_alignment::align_near);

			page_item* canvas2d_row(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* canvas2d_column(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* canvas2d_absolute(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);

			page_item* text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });

			page_item* space(page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });

			void arrange( double _width, double _height, jobject& _style_sheet, double _padding = 0.0 );
			void visit(std::function<bool(page_item* _parent)> fnin, std::function<bool(page_item* _parent)> fout);
			void visit_impl(page_item* _item, std::function<bool(page_item* _parent)> fnin, std::function<bool(page_item* _parent)> fout);

		};
	}
}
