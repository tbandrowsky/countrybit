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
			canvas2d = 4,
			canvas3d = 5,
			field = 6,
			label = 7,
			create = 8,
			select = 9,
			h1 = 10,
			h2 = 11,
			h3 = 12
		};

		class page_item
		{
		public:
			int						id;
			int						parent_id;
			layout_types			layout;

			dyrect					box;
			rectangle				bounds;

			relative_ptr_type		object_id;
			jfield*					field;
			create_object_request	create_request;
			select_object_request	select_request;
			jslice					slice;			

			const char*				caption;
			int						canvas_id;

			page_item() :
				id(-1),
				parent_id(-1),
				layout(layout_types::space),
				object_id(-1),
				field(nullptr),
				caption(nullptr),
				canvas_id(-1)
			{
				;
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
					canvas_id = ((layout == layout_types::canvas2d) ? id : -1);
				}
			}

			bool is_drawable()
			{
				return canvas_id != -1 && canvas_id != id;
			}
		};

		class page : public iarray<page_item, 1024>
		{
			void arrange_impl(page_item *_item, double offx, double offy, double x, double y, double width, double height);
			void visit_impl(page_item* _item, std::function<bool(page_item* _parent)> fn);		
			dynamic_box data;
			using base_type = iarray<page_item, 1024>;

		public:

			page();
			void clear();

			page_item* row(page_item* _parent, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			page_item* column( page_item* _parent, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			page_item* absolute(page_item* _parent, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d(page_item* _parent, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h1(page_item* _parent, const char* _text, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h2(page_item* _parent, const char* _text, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h3(page_item* _parent, const char* _text, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* space(page_item* _parent, dyrect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			page_item* field(page_item* _parent, int object_id, int field_id, jslice slice);

			page_item* actor_update_fields(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection);
			page_item* actor_create_buttons(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection);
			page_item* actor_select_items(page_item* _parent, actor_state* _state, jschema* _schema, jcollection* _collection);

			void arrange( double width, double height );
			void visit(std::function<bool(page_item* _parent)> fn);

		};
	}
}
