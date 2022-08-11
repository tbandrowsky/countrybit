#pragma once

namespace corona
{
	namespace database
	{
		enum class layout_types
		{
			space = 0,
			row = 1,
			column = 2,
			absolute = 3,
			canvas2d_row = 4,
			canvas2d_column = 5,
			canvas2d_absolute = 6,
			canvas3d_row = 7,
			canvas3d_column = 8,
			canvas3d_absolute = 9,
			field = 10,
			label = 11,
			create = 12,
			select = 13,
			select_cell = 14,
			navigate = 15,
			text = 16,
			set = 17
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
			int						parent_id;
			layout_types			layout;
			relative_ptr_type		style_id;

			measure					item_space;
			double					item_space_amount;

			layout_rect				box;
			rectangle				bounds;

			relative_ptr_type		class_id;
			relative_ptr_type		item_uid;

			object_member_path		object_path;

			jfield*					field;
			create_object_request	create_request;
			select_object_request	select_request;
			jobject					slice;

			const char*				caption;
			int						canvas_id;
			int						old_id;

			bool					windowsRegion;

			dynamic_value			dest_value;

			bool					mouse_over;
			bool					pressed;
			bool					selected;

			page_item() :
				id(-1),
				parent_id(-1),
				layout(layout_types::space),
				field(nullptr),
				caption(nullptr),
				canvas_id(-1),
				style_id(null_row),
				old_id(-1),
				item_space(),
				item_space_amount(0.0),
				item_uid(null_row),
				windowsRegion(false),
				mouse_over(false),
				pressed(false),
				selected(false)
			{
				object_path.object.collection_id = {};
				object_path.object.row_id = null_row;
			}

			void set_parent(page_item* _parent)
			{
				if (_parent) 
				{
					parent_id = _parent->id;
					canvas_id = _parent->canvas_id;
				}
				else {
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
				return canvas_id != -1 && canvas_id != id;
			}

			bool is_select()
			{
				return layout == layout_types::select || layout == layout_types::select_cell || layout == layout_types::navigate;
			}

			bool is_create()
			{
				return layout == layout_types::create;
			}

			bool is_set()
			{
				return layout == layout_types::set;
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
				if ((layout == layout_types::field) ||
					(layout == layout_types::set) ||
					(layout == layout_types::label) ||
					(layout == layout_types::create)) {
					pii.layout = (int)layout;
					pii.object_id = object_path.object.row_id;
					pii.class_id = class_id;
					pii.item_uid = item_uid;
					if (field != nullptr) 
					{
						pii.field_id = field->field_id;
					}
				}
				else if ((layout == layout_types::canvas2d_row) ||
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

		class page : public iarray<page_item, 1024>
		{
			void calculate_sizes(jobject& _style_sheet, page::iterator_type children, double offx, double offy, double x, double y, double width, double height, double& remaining_width, double& remaining_height);
			void set_bound_size(jobject& _style_sheet, page_item* _item, double offx, double offy, double x, double y, double width, double height, double remaining_width, double remaining_height);
			void arrange_impl(jobject& _style_sheet, page_item *_item, double offx, double offy, double x, double y, double width, double height);

			dynamic_box data;
			using base_type = iarray<page_item, 1024>;

			void calculate_bounds_w(jobject& _style_sheet, page_item* _pi, double width, double height, double remaining_width, double remaining_height, int safety);
			void calculate_bounds_h(jobject& _style_sheet, page_item* _pi, double width, double height, double remaining_width, double remaining_height, int safety);

		public:

			page();
			void clear();
			const char* copy(const char* _src)
			{
				return data.copy(_src, 0);
			}

			page_item& operator[](int _id)
			{
				int id_to_idx = _id - 1;
				auto& pi = this->get_at(id_to_idx);
				if (pi.id != _id) {
					throw std::logic_error("something bad happened with page item indeces");
				}
				return pi;
			}

			page_item* row(page_item* _parent, relative_ptr_type _style_id = null_row,  layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct }, measure _item_space = { 0.0, measure_units::pixels });
			page_item* column( page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct }, measure _item_space = { 0.0, measure_units::pixels });
			page_item* absolute(page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			page_item* canvas2d_row(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d_column(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d_absolute(relative_ptr_type _item_uid, page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			page_item* text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			page_item* set(page_item* _parent, actor_state* _state, const object_member_path path, int field_id, dynamic_value dv, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* select(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type _id_name, jobject slice, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* select_cell(page_item* _parent, actor_state* _state, int object_id, jobject slice, const char *_caption, relative_ptr_type _style_id, layout_rect _box);
			page_item* navigate(page_item* _parent, actor_state* _state, int object_id, relative_ptr_type _style_id, const char *_caption, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 20.0_px });
			page_item* space(page_item* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			void arrange( double _width, double _height, jobject& _style_sheet, double _padding = 0.0 );
			void visit(std::function<bool(page_item* _parent)> fnin, std::function<bool(page_item* _parent)> fout);
			void visit_impl(page_item* _item, std::function<bool(page_item* _parent)> fnin, std::function<bool(page_item* _parent)> fout);

		};
	}
}
