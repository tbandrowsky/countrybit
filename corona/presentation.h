#pragma once

namespace corona
{
	namespace database
	{
		class layout_context
		{
		public:
			point flow_origin;
			point container_origin;
			point container_size;
			point remaining_size;
			point space_amount;
		};

		template <typename cwindow> class windows_control;

		class row_container_control;
		class column_container_control;
		class absolute_container_control;

		using static_control = windows_control<CStatic>;
		using button_control = windows_control<CButton>;
		using listbox_control = windows_control<CListBox>;
		using combobox_control = windows_control<CComboBox>;
		using edit_control = windows_control<CEdit>;
		using scrollbar_control = windows_control<CScrollBar>;

		using imagelist_control = windows_control<CImageList>;
		using listview_control = windows_control<CListViewCtrl>;
		using treeview_control = windows_control<CTreeViewCtrl>;
		using header_control = windows_control<CHeaderCtrl>;
		using toolbar_control = windows_control<CToolBarCtrl>;
		using statusbar_control = windows_control<CStatusBarCtrl>;
		using hotkey_control = windows_control<CHotKeyCtrl>;
		using animate_control = windows_control<CAnimateCtrl>;
		using richedit_control = windows_control<CRichEditCtrl>;
		using draglistbox_control = windows_control<CDragListBox>;
		using rebar_control = windows_control<CReBarCtrl>;
		using comboboxex_control = windows_control<CComboBoxEx>;
		using datetimepicker_control = windows_control<CDateTimePickerCtrl>;
		using monthcalendar_control = windows_control<CMonthCalendarCtrl>;

		class control_base
		{
		protected:

			virtual void size_constant(layout_context _ctx);
			virtual void size_constants(layout_context _ctx);

			virtual void size_aspect(layout_context _ctx);
			virtual void size_aspect_widths(layout_context _ctx, int safety);
			virtual void size_aspect_heights(layout_context _ctx, int safety);
			virtual void size_aspects(layout_context _ctx);
			virtual void size_children(layout_context _ctx);

			virtual void size_remaining(layout_context _ctx);
			virtual void size_remainings(layout_context _ctx);
			virtual layout_context get_remaining(layout_context _ctx);

			virtual void size_item(layout_context _ctx);
			virtual void size_items(layout_context _ctx);
			virtual void positions(layout_context _ctx);

		public:

			int						id;
			std::string				control_type;
			visual_alignment		alignment;

			layout_rect				box;
			measure					item_space;

			rectangle				bounds;
			point					item_space_amount;

			std::string				caption;

			bool					mouse_over;
			bool					pressed;
			bool					selected;

			win32::directApplicationWin32* app;
			std::weak_ptr<control_base> parent;
			std::vector<std::shared_ptr<control_base>> children;

			control_base() :
				id(-1),
				item_space(),
				item_space_amount({ 0.0, 0.0 } ),
				mouse_over(false),
				pressed(false),
				selected(false)
			{
			}

			virtual rectangle layout(layout_context _ctx);
			virtual void position(layout_context _ctx);

			std::shared_ptr<row_container_control> create_row(int id);
			std::shared_ptr<column_container_control> create_column(int id);
			std::shared_ptr<absolute_container_control> create_absolute(int id);

			std::shared_ptr<static_control> create_static(int id);
			std::shared_ptr<button_control> create_button(int id);
			std::shared_ptr<listbox_control> create_listbox(int id);
			std::shared_ptr<combobox_control> create_combobox(int id);
			std::shared_ptr<edit_control> create_edit(int id);
			std::shared_ptr<scrollbar_control> create_scrollbar(int id);

			std::shared_ptr<imagelist_control> create_imagelist(int id);
			std::shared_ptr<listview_control> create_listview(int id);
			std::shared_ptr<treeview_control> create_treeview(int id);
			std::shared_ptr<header_control>  create_header(int id);
			std::shared_ptr<toolbar_control> create_toolbar(int id);
			std::shared_ptr<statusbar_control> create_statusbar(int id);
			std::shared_ptr<hotkey_control> create_hotkey(int id);
			std::shared_ptr<animate_control> create_animate(int id);
			std::shared_ptr<richedit_control> create_richedit(int id);
			std::shared_ptr<draglistbox_control> create_draglistbox(int id);
			std::shared_ptr<rebar_control> create_rebar(int id);
			std::shared_ptr<comboboxex_control> create_comboboxex(int id);
			std::shared_ptr<datetimepicker_control> create_datetimepicker(int id);
			std::shared_ptr<monthcalendar_control> create_monthcalendar(int id);

		};

		class container_control : public control_base
		{
		public:

		};

		class absolute_container_control : public container_control
		{
		protected:


		public:
		};

		class column_container_control : public container_control
		{
		protected:
			virtual layout_context get_remaining(layout_context _ctx);
			virtual void size_children(layout_context _ctx);
			virtual void positions(layout_context _ctx);
		public:
		};

		class row_container_control : public container_control
		{
		protected:
			virtual layout_context get_remaining(layout_context _ctx);
			virtual void size_children(layout_context _ctx);
			virtual void positions(layout_context _ctx);
		public:
		};

		class canvas_control : public control_base
		{
		public:

			bool is_canvas2d()
			{
				return true;
			}
		};


		template <typename cwindow> class windows_control : public control_base
		{
		public:
			cwindow					window;
		};

		enum class field_layout 
		{
			label_on_left = 1,
			label_on_top = 2
		};

		class page
		{

			rectangle layout(control_base* _item, layout_context _ctx);

			std::unique_ptr<control_base> root;

		public:

			page();

			void clear();

			control_base *operator[](int _id)
			{
				return nullptr;
			}

			control_base* append(control_base* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment);

			control_base* row(control_base* _parent, relative_ptr_type _style_id = null_row,  layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			control_base* column( control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			control_base* absolute(control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, visual_alignment _alignment = visual_alignment::align_near);

			control_base* canvas2d_row(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			control_base* canvas2d_column(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			control_base* canvas2d_absolute(relative_ptr_type _item_uid, control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);

			control_base* text(control_base* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });

			control_base* space(control_base* _parent, relative_ptr_type _style_id = null_row, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });

			void arrange( double _width, double _height, double _padding = 0.0 );
			void visit(std::function<bool(control_base* _parent)> fnin, std::function<bool(control_base* _parent)> fout);
			void visit_impl(control_base* _item, std::function<bool(control_base* _parent)> fnin, std::function<bool(control_base* _parent)> fout);

		};
	}
}
