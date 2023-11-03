#ifndef CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H
#define CORONA_PRESENTATION_CONTROLS_DX_CONTAINER_H

namespace corona
{


	class container_control : public draw_control, public cloneable<container_control>
	{

		template <typename control_type> control_type *create(int _id)
		{
			std::shared_ptr<control_type> temp = std::make_shared<control_type>(this, _id);
			children.push_back(temp);
			std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
			return temp.get();
		}

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

		container_control(container_control_base* _parent, int _id)
		{
			parent = _parent;
			id = _id;
			if (parent && get_nchittest() == HTCLIENT) {
				set_nchittest(parent->get_nchittest());
			}
		}

		virtual ~container_control()
		{
			;
		}

		inline container_control& title(std::string _text) { return title(_text, nullptr, id_counter::next()); }
		inline container_control& subtitle(std::string _text) { return subtitle(_text, nullptr, id_counter::next()); }
		inline container_control& chaptertitle(std::string _text) { return chaptertitle(_text, nullptr, id_counter::next()); }
		inline container_control& chaptersubtitle(std::string _text) { return chaptersubtitle(_text, nullptr, id_counter::next()); }
		inline container_control& paragraph(std::string _text) { return paragraph(_text, nullptr, id_counter::next()); }
		inline container_control& code(std::string _text) { return code(_text, nullptr, id_counter::next()); }
		inline container_control& label(std::string _text) { return label(_text, nullptr, id_counter::next()); }

		inline container_control& title(int _id, std::string _text) { return title(_text, nullptr, _id); }
		inline container_control& subtitle(int _id, std::string _text) { return subtitle(_text, nullptr, _id); }
		inline container_control& chaptertitle(int _id, std::string _text) { return chaptertitle(_text, nullptr, _id); }
		inline container_control& chaptersubtitle(int _id, std::string _text) { return chaptersubtitle(_text, nullptr, _id); }
		inline container_control& paragraph(int _id, std::string _text) { return paragraph(_text, nullptr, _id); }
		inline container_control& code(int _id, std::string _text) { return code(_text, nullptr, _id); }
		inline container_control& label(int _id, std::string _text) { return label(_text, nullptr, _id); }

		inline container_control& title(int _id, std::function<void(title_control&)> _settings) { return title("", _settings, _id); }
		inline container_control& subtitle(int _id, std::function<void(subtitle_control&)> _settings) { return subtitle("", _settings, _id); }
		inline container_control& chaptertitle(int _id, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle("", _settings, _id); }
		inline container_control& chaptersubtitle(int _id, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle("", _settings, _id); }
		inline container_control& paragraph(int _id, std::function<void(paragraph_control&)> _settings) { return paragraph("", _settings, _id); }
		inline container_control& code(int _id, std::function<void(code_control&)> _settings) { return code("", _settings, _id); }
		inline container_control& label(int _id, std::function<void(label_control&)> _settings) { return label("", _settings, _id); }

		inline container_control& title(std::string _text, std::function<void(title_control&)> _settings) { return title(_text, _settings, id_counter::next()); }
		inline container_control& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings) { return subtitle(_text, _settings, id_counter::next()); }
		inline container_control& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle(_text, _settings, id_counter::next()); }
		inline container_control& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle(_text, _settings, id_counter::next()); }
		inline container_control& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings) { return paragraph(_text, _settings, id_counter::next()); }
		inline container_control& code(std::string _text, std::function<void(code_control&)> _settings) { return code(_text, _settings, id_counter::next()); }
		inline container_control& label(std::string _text, std::function<void(label_control&)> _settings) { return label(_text, _settings, id_counter::next()); }

		template <typename control_type> void apply(control_type* _ref)
		{
			if (!_ref) {
				return;
			}

			if (item_box.height.amount > 0 && item_box.width.amount > 0)
			{
				_ref->box = item_box;
			}

			if (item_margin.amount > 0)
			{
				_ref->margin = item_margin;
			}
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

		row_layout& row_begin(int _id, std::function<void(row_layout&)> _settings)
		{
			auto tc = create<row_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		column_layout& column_begin(int _id, std::function<void(column_layout&)> _settings)
		{
			auto tc = create<column_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		absolute_layout& absolute_begin(int _id, std::function<void(absolute_layout&)> _settings)
		{
			auto tc = create<absolute_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		row_view_layout& row_view_begin(int id, std::function<void(row_view_layout&)> _settings)
		{
			auto tc = create<row_view_layout>(id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		column_view_layout& column_view_begin(int id, std::function<void(column_view_layout&)> _settings)
		{
			auto tc = create<column_view_layout>(id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		absolute_view_layout& absolute_view_begin(int id, std::function<void(absolute_view_layout&)> _settings)
		{
			auto tc = create<absolute_view_layout>(id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		frame_layout& frame_begin(int _id, std::function<void(frame_layout&)> _settings)
		{
			auto tc = create<frame_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *tc;
		}

		container_control& caption_bar(int _id, std::function<void(caption_bar_control&)> _settings)
		{
			auto tc = create<caption_bar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& form_single_column(int _id, std::function<void(form_single_column_control&)> _settings)
		{
			auto tc = create<form_single_column_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& form_double_column(int _id, std::function<void(form_double_column_control&)> _settings)
		{
			auto tc = create<form_double_column_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& status_bar(int _id, std::function<void(form_double_column_control&)> _settings)
		{
			auto tc = create<form_double_column_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& end()
		{
			if (parent) {
				container_control* temp = dynamic_cast<container_control*>(parent);
				//		auto string_name = typeid(temp).name();
		//				std::string indent(debug_indent, ' ');
			//			std::cout << indent << " " << typeid(*this).name() << " ->navigate " << string_name << std::endl;
				return *temp;
			}
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

		container_control& image(int id, int _control_id, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id);
			apply(tc);
			tc->load_from_control(_control_id);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& image(int id, std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id);
			apply(tc);
			tc->load_from_file(_filename);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& image(std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id_counter::next());
			apply(tc);
			tc->load_from_file(_filename);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& title(std::string text, std::function<void(title_control&)> _settings, int _id)
		{
			auto tc = create<title_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& subtitle(std::string text, std::function<void(subtitle_control&)> _settings, int _id)
		{
			auto tc = create<subtitle_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& chaptertitle(std::string text, std::function<void(chaptertitle_control&)> _settings, int _id)
		{
			auto tc = create<chaptertitle_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& chaptersubtitle(std::string text, std::function<void(chaptersubtitle_control&)> _settings, int _id)
		{
			auto tc = create<chaptersubtitle_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& paragraph(std::string text, std::function<void(paragraph_control&)> _settings, int _id)
		{
			auto tc = create<paragraph_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& code(std::string text, std::function<void(code_control&)> _settings, int _id)
		{
			auto tc = create<code_control>(_id);
			apply(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& label(std::string _text, std::function<void(label_control&)> _settings, int _id)
		{
			auto tc = create<label_control>(_id);
			apply(tc);
			tc->set_text(_text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& push_button(int _id, std::string text, std::function<void(pushbutton_control&)> _settings = nullptr)
		{
			auto tc = create<pushbutton_control>(_id);
			apply(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& press_button(int _id, std::string text, std::function<void(pressbutton_control&)> _settings = nullptr)
		{
			auto tc = create<pressbutton_control>(_id);
			apply(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& radio_button(int _id, std::string text, std::function<void(radiobutton_control&)> _settings = nullptr)
		{
			auto tc = create<radiobutton_control>(_id);
			apply(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& checkbox(int _id, std::string text, std::function<void(checkbox_control&)> _settings = nullptr)
		{
			auto tc = create<checkbox_control>(_id);
			apply(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& listbox(int _id, std::function<void(listbox_control&)> _settings = nullptr)
		{
			auto tc = create<listbox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& combobox(int _id, std::function<void(combobox_control&)> _settings = nullptr)
		{
			auto tc = create<combobox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& edit(int _id, std::function<void(edit_control&)> _settings = nullptr)
		{
			auto tc = create<edit_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& scrollbar(int _id, std::function<void(scrollbar_control&)> _settings = nullptr)
		{
			auto tc = create<scrollbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& listview(int _id, std::function<void(listview_control&)> _settings = nullptr)
		{
			auto tc = create<listview_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& treeview(int _id, std::function<void(treeview_control&)> _settings = nullptr)
		{
			auto tc = create<treeview_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& header(int _id, std::function<void(header_control&)> _settings = nullptr)
		{
			auto tc = create<header_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& toolbar(int _id, std::function<void(toolbar_control&)> _settings = nullptr)
		{
			auto tc = create<toolbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& statusbar(int _id, std::function<void(statusbar_control&)> _settings = nullptr)
		{
			auto tc = create<statusbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& hotkey(int _id, std::function<void(hotkey_control&)> _settings = nullptr)
		{
			auto tc = create<hotkey_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& animate(int _id, std::function<void(animate_control&)> _settings = nullptr)
		{
			auto tc = create<animate_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& richedit(int _id, std::function<void(richedit_control&)> _settings = nullptr)
		{
			auto tc = create<richedit_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& draglistbox(int _id, std::function<void(draglistbox_control&)> _settings = nullptr)
		{
			auto tc = create<draglistbox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}


		container_control& comboboxex(int _id, std::function<void(comboboxex_control&)> _settings = nullptr)
		{
			auto tc = create<comboboxex_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& datetimepicker(int _id, std::function<void(datetimepicker_control&)> _settings = nullptr)
		{
			auto tc = create<datetimepicker_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& monthcalendar(int _id, std::function<void(monthcalendar_control&)> _settings = nullptr)
		{
			auto tc = create<monthcalendar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& minimize_button(std::function<void(minimize_button_control&)> _settings = nullptr)
		{
			auto tc = create<minimize_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
			;
		}

		container_control& maximize_button(std::function<void(maximize_button_control&)> _settings = nullptr)
		{
			auto tc = create<maximize_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& close_button(std::function<void(close_button_control&)> _settings = nullptr)
		{
			auto tc = create<close_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& menu_button(int _id, std::function<void(menu_button_control&)> _settings = nullptr)
		{
			auto tc = create<menu_button_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& camera(int _id, std::function<void(camera_control&)> _settings = nullptr)
		{
			auto tc = create<camera_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& grid(int _id, std::function<void(grid_control&)> _settings = nullptr)
		{
			auto tc = create<grid_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& chart(int _id, std::function<void(chart_control&)> _settings = nullptr)
		{
			auto tc = create<chart_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		container_control& slide(int _id, std::function<void(slide_control&)> _settings = nullptr)
		{
			auto tc = create<slide_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		template <typename field_control> container_control& create_field(
			container_control* _parent,
			int _id,
			std::string _field_label,
			std::string _tooltip_string,
			std::function<void(field_control&)> _settings = nullptr)
		{
			auto& cl = _parent->column_begin();
			cl.set_size(100.0_container, 50.0_px);
			cl.set_item_size(100.0_container, 1.3_fontgr);
			auto& lb = cl.label(_field_label);
			auto tc = cl.create<field_control>(_id);
			tc->tooltip_text = _tooltip_string;
			cl.apply(tc);
			if (_settings) {
				_settings(tc);
			}
			cl.end();
			return *_parent;
		}

		container_control& listbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listbox_control&)> _settings)
		{
			return create_field<listbox_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& combobox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(combobox_control&)> _settings)
		{
			return create_field<combobox_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& edit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(edit_control&)> _settings)
		{
			return create_field<edit_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& listview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listview_control&)> _settings)
		{
			return create_field<listview_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& treeview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(treeview_control&)> _settings)
		{
			return create_field<treeview_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& header_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(header_control&)> _settings)
		{
			return create_field<header_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& toolbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(toolbar_control&)> _settings)
		{
			return create_field<toolbar_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& statusbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(statusbar_control&)> _settings)
		{
			return create_field<statusbar_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& hotkey_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(hotkey_control&)> _settings)
		{
			return create_field<hotkey_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& animate_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(animate_control&)> _settings)
		{
			return create_field<animate_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& richedit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(richedit_control&)> _settings)
		{
			return create_field<richedit_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& draglistbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(draglistbox_control&)> _settings)
		{
			return create_field<draglistbox_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& comboboxex_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(comboboxex_control&)> _settings)
		{
			return create_field<comboboxex_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& datetimepicker_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(datetimepicker_control&)> _settings)
		{
			return create_field<datetimepicker_control>(this, _id, _field_label, _tooltip_text, _settings);
		}
		container_control& monthcalendar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(monthcalendar_control&)> _settings)
		{
			return create_field<monthcalendar_control>(this, _id, _field_label, _tooltip_text, _settings);
		}


	};

	class absolute_layout :
		public container_control, public cloneable<grid_control>
	{
	public:
		absolute_layout() { ; }
		absolute_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~absolute_layout() { ; }

		virtual void arrange(rectangle _ctx);
	};

	class column_layout :
		public container_control, public cloneable<column_layout>
	{
		layout_rect item_size;
	public:
		column_layout() { ; }
		column_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~column_layout() { ; }

		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
	};

	class row_layout :
		public container_control, public cloneable<row_layout>
	{
	protected:
	public:
		row_layout() { ; }
		row_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~row_layout() { ; }

		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
	};

	class frame_layout :
		public container_control, public cloneable<frame_layout>
	{
	protected:
	public:
		frame_layout() { ; }
		frame_layout(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~frame_layout() { ; }

		void set_contents(control_base *_page)
		{
			children.clear();
			children.push_back(_page->clone(this));
			arrange(bounds);
		}
	};

	class tabbed_container : public column_layout, public cloneable<tabbed_container>
	{
		json tab_data;

		std::shared_ptr<row_layout>	 tabs;
		std::shared_ptr<frame_layout> frame;

		int active_id;

		void init()
		{

			children.clear();
			row_begin(id_counter::next(), [](row_layout& _settings) {
				_settings.set_size(1.0_container, 50.0_px);
				});
			frame_begin(id_counter::next(), [](frame_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_remaining);
				});
			tabs = std::dynamic_pointer_cast<row_layout>(children[0]);
			frame = std::dynamic_pointer_cast<frame_layout>(children[1]);
		}

	public:

		tabbed_container()
		{
			init();
		}

		tabbed_container(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			init();
		}

		void set_tab_data(json& _data)
		{
			tabs->children.clear();
			for (int i = 0; i < _data.size(); i++)
			{
				auto tb = std::make_shared<tab_button_control>();
				auto dat = _data[i];
				tb->id = dat["tab_id"];
				tb->text = dat["tab_text"];
				tb->active_id = &active_id;
				tabs->children.push_back(tb);
				if (!i) {
					active_id = tb->id;
				}
			}
		}
	};

	class column_view_layout :
		public column_layout, public cloneable<column_view_layout>
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
				auto cb = item_source.data_to_control(item_source.data, i);
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

	};

	class row_view_layout :
		public row_layout, public cloneable<row_view_layout>
	{
	protected:
	public:
		row_view_layout() { ; }
		row_view_layout(container_control_base* _parent, int _id) : row_layout(_parent, _id) { ; }
		virtual ~row_view_layout() { ; }
	};

	class absolute_view_layout :
		public absolute_layout
	{
	protected:
	public:
		absolute_view_layout() { ; }
		absolute_view_layout(container_control_base* _parent, int _id) : absolute_layout(_parent, _id) { ; }
		virtual ~absolute_view_layout() { ; }
	};

	class search_view : public column_layout, public cloneable<search_view>
	{
		void init()
		{
			idc_search_text = id_counter::next();
			idc_search_command = id_counter::next();
			idc_search_results = id_counter::next();

			row_begin(id_counter::next(), [](row_layout& _rl)
				{
					_rl.set_size(1.0_container, 75.0_px);
				})
				.label("Search")
				.edit(idc_search_text)
				.push_button(idc_search_command, "Go")
				.end()
				.column_view_begin(idc_search_results, nullptr)
				.set_size(1.0_container, 1.0_remaining)
				.end();
		}

	public:

		int idc_search_text;
		int idc_search_command;
		int idc_search_results;

		search_view()
		{
			init();
		}

		search_view(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			init();
		}

		void set_item_source(array_data_source _item_source)
		{
			auto& cview = control_base::find<column_view_layout>(idc_search_results);
			cview.set_item_source(_item_source);
		}

		virtual ~search_view()
		{
			;
		}
	};

	class banner_control :
		public row_layout, public cloneable<banner_control>
	{
	protected:
		std::string logo_filename;
	public:
		banner_control() { ; }
		banner_control(container_control_base* _parent, int _id) : row_layout(_parent, _id) { ; }
		virtual ~banner_control() { ; }

		virtual void arrange(rectangle _ctx);
		virtual point get_remaining(point _ctx);
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


	class caption_bar_control : public container_control, public cloneable<caption_bar_control>
	{
		void init() 
		{
				row_begin(title_bar_id, [this](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_background_color(st->HeaderBackgroundColor);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				rl.set_nchittest(HTCAPTION);
					})
				.row_begin(id_counter::next(), [](row_layout& rl) {
						rl.set_size(.24_container, 1.0_container);
					})
						.row_begin(id_counter::next(), [](row_layout& cl) {
						cl.set_content_align(visual_alignment::align_near);
						cl.set_content_cross_align(visual_alignment::align_near);
						cl.set_size(120.0_px, 1.0_container);
						cl.set_item_margin(5.0_px);
							})
						.menu_button(menu_button_id, [this](auto& _ctrl) {
								_ctrl.set_size(50.0_px, 50.0_px);
								_ctrl.set_margin(5.0_px);
								_ctrl.menu = *menu;
							})
								.image(image_control_id, image_file, [](image_control& control) { control.set_size(50.0_px, 50.0_px); })
								.end()
								.column_begin(id_counter::next(), [](column_layout& cl) {
								cl.set_content_align(visual_alignment::align_near);
								cl.set_content_cross_align(visual_alignment::align_near);
								cl.set_size(1.0_container, 1.0_container);
								cl.set_item_margin(0.0_px);
									})
								.title(corporate_name, [](title_control& control) {
										control.text_style.horizontal_align = visual_alignment::align_near;
										control.text_style.vertical_align = visual_alignment::align_near;
										control.set_size(300.0_px, 1.0_container);
									})
										.end()
										.end()
										.column_begin(id_title_column_id, [](column_layout& cl) {
										cl.set_content_align(visual_alignment::align_near);
										cl.set_content_cross_align(visual_alignment::align_near);
										cl.set_item_margin(0.0_px);
										cl.set_size(.33_container, 1.0_container);
											})
										.title(title_name, [](title_control& control) {
												control.text_style.horizontal_align = visual_alignment::align_near;
												control.text_style.vertical_align = visual_alignment::align_near;
												control.set_size(400.0_px, 1.0_fontgr);
											})
												.subtitle(subtitle_name, [](subtitle_control& control) {
												control.text_style.horizontal_align = visual_alignment::align_near;
												control.text_style.vertical_align = visual_alignment::align_near;
												control.text_style.underline = true;
												control.set_size(400.0_px, 1.0_fontgr);
													})
												.end()
														.row_begin(id_counter::next(), [](row_layout& rl) {
														rl.set_size(.95_remaining, 1.0_container);
														rl.set_item_margin(5.0_px);
														rl.set_content_cross_align(visual_alignment::align_center);
														rl.set_content_align(visual_alignment::align_far);
															})
														.minimize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
																.maximize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
																.close_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
																.end()
																.end();
		}

	public:
		presentation_style *st;
		int	title_bar_id;
		int menu_button_id;
		menu_item* menu;
		int image_control_id;
		std::string image_file;
		std::string corporate_name;
		int id_title_column_id;
		std::string title_name;
		std::string subtitle_name;

		caption_bar_control() 
		{ 
			init(); 
		}

		caption_bar_control(container_control_base* _parent, int _id) : container_control(_parent, _id) 
		{ 
			; 
		}

		virtual ~caption_bar_control() { ; }
	};

	class status_bar_control : public container_control, public cloneable<status_bar_control>
	{
		void init() {
			row_begin(id_counter::status_bar_id, [this](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_background_color(st->HeaderBackgroundColor);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				})
				.column_begin(id_counter::next(), [](column_layout& cl) {
					cl.set_content_align(visual_alignment::align_near);
					cl.set_content_cross_align(visual_alignment::align_near);
					cl.set_size(.3_container, 1.0_container);
					cl.set_item_margin(0.0_px);
					})
					.title(id_counter::status_text_title_id, [](title_control& control) {
						control.text_style.horizontal_align = visual_alignment::align_near;
						control.text_style.vertical_align = visual_alignment::align_near;
						control.set_size(300.0_px, 1.2_fontgr);
						})
					.subtitle(id_counter::status_text_subtitle_id, [](subtitle_control& control) {
						control.text_style.horizontal_align = visual_alignment::align_near;
						control.text_style.vertical_align = visual_alignment::align_near;
						control.set_size(300.0_px, 1.2_fontgr);
						})
				.end();
			end();

		}
	public:

		presentation_style* st = nullptr;

		status_bar_control() { ; }
		status_bar_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~status_bar_control() { ; }
	};

	class form_single_column_control : public container_control, public cloneable<form_single_column_control>
	{
		void init() 
		{
			row_begin(id_counter::next(), [](row_layout& r)
				{
					r.set_size(1.0_container, 1.0_container);
					r.set_content_align(visual_alignment::align_center);
					r.set_background_color(styles.get_style()->FormBackgroundColor);
				})
				.column_begin(id_counter::next(), [this](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.50_container, 1.0_container);
						r.push(align_id, true, false, false, false);
						add_controls(r);
					})
					.end()
			.end();
		}
	public:

		int align_id;
		std::string form_name;
		std::function<void(container_control& _settings)> add_controls;

		form_single_column_control() { ; }
		form_single_column_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~form_single_column_control() { ; }
	};

	class form_double_column_control : public container_control, public cloneable<container_control>
	{
		void init() 
		{
			row_begin(id_counter::next(), [](row_layout& r)
				{
					r.set_size(1.0_container, 1.0_container);
					r.set_content_align(visual_alignment::align_center);
					r.set_background_color(styles.get_style()->FormBackgroundColor);
				})
				.column_begin(id_counter::next(), [this](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.30_container, 1.0_container);
						r.push(align_id, true, false, false, false);
						add_controls1(r);
					})
					.end()
						.column_begin(id_counter::next(), [this](column_layout& r)
							{
								r.set_margin(10.0_px);
								r.set_size(.30_container, 1.0_container);
								r.push(align_id, true, false, false, false);
								add_controls2(r);
							})
					.end()
				.end();
		}

	public:

		int align_id;
		std::string form_name;
		std::function<void(container_control& _settings)> add_controls1;
		std::function<void(container_control& _settings)> add_controls2;

		form_double_column_control() { ; }
		form_double_column_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~form_double_column_control() { ; }

	};

}

#endif
