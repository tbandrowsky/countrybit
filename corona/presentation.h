#pragma once

namespace corona
{
	namespace database
	{
		enum class layout_types
		{
			row = 1,
			column = 2,
			absolute = 3,
			canvas2d = 4,
			canvas3d = 5,
			field = 6,
			create = 7,
			select = 8
		};

		enum class measure_units
		{
			percent = 0,
			pixels = 1
		};

		class measure
		{
		public:
			long double amount;
			measure_units units;
		};

		measure operator ""px(long double px);
		measure operator ""pct(long double pct);

		class measure_box
		{
		public:
			measure x, y, width, height;
		};

		class page_item
		{
		public:
			int					id;
			int					parent_id;
			layout_types		layout;

			measure_box			box;
			rectangle			bounds;

			relative_ptr_type	object_id;
			relative_ptr_type	field_id;
		};

		class page : public iarray<page_item, 1024>
		{
			void arrange_impl(page_item *_item, double offx, double offy, double x, double y, double width, double height);
			void visit_impl(page_item* _item, std::function<bool(page_item* _parent)> fn);

		public:

			page_item* row(page_item* _parent, measure_box _box = { 0.0px, 0.0px, 100.0pct, 100.0pct });
			page_item* column( page_item* _parent, measure_box _box = { 0.0px, 0.0px, 100.0pct, 100.0pct });
			page_item* absolute(page_item* _parent, measure_box _box = { 0.0px, 0.0px, 100.0pct, 100.0px });
			page_item* canvas2d(page_item* _parent, measure_box _box = { 0.0px, 0.0px, 100.0pct, 100.0px });

			page_item* field(page_item* _parent, int object_id, int field_id);

			page_item* actor_update_fields(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection);
			page_item* actor_create_buttons(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection);
			page_item* actor_select_buttons(page_item* _parent, actor_state& _state, jschema* _schema, jcollection* _collection);

			void arrange( double width, double height );
			void visit(std::function<bool(page_item* _parent)> fn);
		};
	}
}
