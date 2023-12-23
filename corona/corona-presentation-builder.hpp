
#ifndef CORONA_PRESENTATION_BUILDER_H
#define CORONA_PRESENTATION_BUILDER_H

#include <corona-presentation-controls-base.hpp>
#include <corona-presentation-controls-dx.hpp>
#include <corona-presentation-controls-dx-container.hpp>
#include <corona-presentation-controls-dx-text.hpp>
#include <corona-presentation-controls-win32.hpp>

namespace corona
{

	class tab_view_control;
	class search_view_control;
	class caption_bar_control;
	class status_bar_control;
	class form_single_column_control;
	class form_double_column_control;

	class control_builder
	{
		std::shared_ptr<container_control> root;
		control_builder *parent;

		template <typename control_type> std::shared_ptr<control_type> create(int _id)
		{
			auto cp = root.get();
			std::shared_ptr<control_type> temp;
			temp = std::make_shared<control_type>(cp, _id);
			if (temp) {
				root->children.push_back(temp);
				std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
			}
			return temp;
		}

		template <typename control_type> void apply_item_sizes(control_type _ref)
		{
			if (!_ref) {
				return;
			}

			if (root->item_box.height.amount > 0 && root->item_box.width.amount > 0)
			{
				_ref->box = root->item_box;
			}

			if (root->item_margin.amount > 0)
			{
				_ref->margin = root->item_margin;
			}
		}

		template <typename field_control> control_builder& create_field(
			int _id,
			std::string _field_label,
			std::string _tooltip_string,
			std::function<void(field_control&)> _settings = nullptr)
		{
			auto cl = column_begin(id_counter::next(), [](column_layout& _src) {
				_src.set_size(100.0_container, 50.0_px);
				_src.set_item_size(100.0_container, 1.3_fontgr);
			});
			auto& lb = cl.label(_field_label);
			auto tc = cl.create<field_control>(_id);
			tc->tooltip_text = _tooltip_string;
			cl.apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			cl.end();
			return *this;
		}

	public:

		control_builder() 
		{
			parent = nullptr;
			root = std::make_shared<row_layout>();
		}

		control_builder(std::shared_ptr<container_control>& _root)
		{
			parent = nullptr;
			root = _root;
		}

		control_builder(std::shared_ptr<column_layout> _root)
		{
			parent = nullptr;
			root = std::dynamic_pointer_cast<container_control>(_root);
		}

		control_builder(std::shared_ptr<row_layout> _root)
		{
			parent = nullptr;
			root = std::dynamic_pointer_cast<container_control>(_root);
		}

		control_builder(std::shared_ptr<absolute_layout> _root)
		{
			parent = nullptr;
			root = std::dynamic_pointer_cast<container_control>(_root);
		}

		control_builder(control_builder* _parent, std::shared_ptr<container_control>& _root)
		{
			parent = _parent;
			root = _root;
		}

		control_builder(const control_builder& _src)
		{
			parent = nullptr;
			root = _src.root;
		}

		control_builder &operator =(const control_builder& _src)
		{
			parent = nullptr;
			root = _src.root;
			return *this;
		}

		control_builder(control_builder&& _src)
		{
			parent = std::move(_src.parent);
			root = std::move(_src.root);
		}

		control_builder &operator =(control_builder&& _src)
		{
			parent = std::move(_src.parent);
			root = std::move(_src.root);
			return *this;
		}

		void apply_controls(control_base* _control)
		{
			if (_control == root.get())
				return;
			_control->children.clear();
			for (auto child : root->children) {
				child->parent = _control;
				_control->children.push_back(child);
			}
		}

		std::shared_ptr<container_control> get_root()
		{
			return root;
		}

		inline control_builder& title(std::string _text) { return title(_text, nullptr, id_counter::next()); }
		inline control_builder& subtitle(std::string _text) { return subtitle(_text, nullptr, id_counter::next()); }
		inline control_builder& chaptertitle(std::string _text) { return chaptertitle(_text, nullptr, id_counter::next()); }
		inline control_builder& chaptersubtitle(std::string _text) { return chaptersubtitle(_text, nullptr, id_counter::next()); }
		inline control_builder& paragraph(std::string _text) { return paragraph(_text, nullptr, id_counter::next()); }
		inline control_builder& code(std::string _text) { return code(_text, nullptr, id_counter::next()); }
		inline control_builder& label(std::string _text) { return label(_text, nullptr, id_counter::next()); }

		inline control_builder& title(int _id, std::string _text) { return title(_text, nullptr, _id); }
		inline control_builder& subtitle(int _id, std::string _text) { return subtitle(_text, nullptr, _id); }
		inline control_builder& chaptertitle(int _id, std::string _text) { return chaptertitle(_text, nullptr, _id); }
		inline control_builder& chaptersubtitle(int _id, std::string _text) { return chaptersubtitle(_text, nullptr, _id); }
		inline control_builder& paragraph(int _id, std::string _text) { return paragraph(_text, nullptr, _id); }
		inline control_builder& code(int _id, std::string _text) { return code(_text, nullptr, _id); }
		inline control_builder& label(int _id, std::string _text) { return label(_text, nullptr, _id); }

		inline control_builder& title(int _id, std::function<void(title_control&)> _settings) { return title("", _settings, _id); }
		inline control_builder& subtitle(int _id, std::function<void(subtitle_control&)> _settings) { return subtitle("", _settings, _id); }
		inline control_builder& chaptertitle(int _id, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle("", _settings, _id); }
		inline control_builder& chaptersubtitle(int _id, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle("", _settings, _id); }
		inline control_builder& paragraph(int _id, std::function<void(paragraph_control&)> _settings) { return paragraph("", _settings, _id); }
		inline control_builder& code(int _id, std::function<void(code_control&)> _settings) { return code("", _settings, _id); }
		inline control_builder& label(int _id, std::function<void(label_control&)> _settings) { return label("", _settings, _id); }

		inline control_builder& title(std::string _text, std::function<void(title_control&)> _settings) { return title(_text, _settings, id_counter::next()); }
		inline control_builder& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings) { return subtitle(_text, _settings, id_counter::next()); }
		inline control_builder& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle(_text, _settings, id_counter::next()); }
		inline control_builder& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle(_text, _settings, id_counter::next()); }
		inline control_builder& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings) { return paragraph(_text, _settings, id_counter::next()); }
		inline control_builder& code(std::string _text, std::function<void(code_control&)> _settings) { return code(_text, _settings, id_counter::next()); }
		inline control_builder& label(std::string _text, std::function<void(label_control&)> _settings) { return label(_text, _settings, id_counter::next()); }

		control_builder row_begin(int _id, std::function<void(row_layout&)> _settings)
		{
			auto tc = create<row_layout>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			auto temp = control_builder(this, cc);
			return temp;
		}

		control_builder column_begin(int _id, std::function<void(column_layout&)> _settings)
		{
			auto tc = create<column_layout>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder absolute_begin(int _id, std::function<void(absolute_layout&)> _settings)
		{
			auto tc = create<absolute_layout>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder row_view_begin(int id, std::function<void(row_view_layout&)> _settings)
		{
			auto tc = create<row_view_layout>(id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder column_view_begin(int id, std::function<void(column_view_layout&)> _settings)
		{
			auto tc = create<column_view_layout>(id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder absolute_view_begin(int id, std::function<void(absolute_view_layout&)> _settings)
		{
			auto tc = create<absolute_view_layout>(id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder frame_begin(int _id, std::function<void(frame_layout&)> _settings)
		{
			auto tc = create<frame_layout>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			auto cc = std::dynamic_pointer_cast<container_control>(tc);
			return control_builder(this, cc);
		}

		control_builder end()
		{
			if (parent) {
				return *parent;
			}
			else 
			{
				return *this;
			}
		}

		control_builder& listbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listbox_control&)> _settings)
		{
			return create_field<listbox_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& combobox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(combobox_control&)> _settings)
		{
			return create_field<combobox_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& edit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(edit_control&)> _settings)
		{
			return create_field<edit_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& listview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listview_control&)> _settings)
		{
			return create_field<listview_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& treeview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(treeview_control&)> _settings)
		{
			return create_field<treeview_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& header_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(header_control&)> _settings)
		{
			return create_field<header_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& toolbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(toolbar_control&)> _settings)
		{
			return create_field<toolbar_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& statusbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(statusbar_control&)> _settings)
		{
			return create_field<statusbar_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& hotkey_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(hotkey_control&)> _settings)
		{
			return create_field<hotkey_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& animate_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(animate_control&)> _settings)
		{
			return create_field<animate_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& richedit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(richedit_control&)> _settings)
		{
			return create_field<richedit_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& draglistbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(draglistbox_control&)> _settings)
		{
			return create_field<draglistbox_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& comboboxex_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(comboboxex_control&)> _settings)
		{
			return create_field<comboboxex_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& datetimepicker_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(datetimepicker_control&)> _settings)
		{
			return create_field<datetimepicker_control>(_id, _field_label, _tooltip_text, _settings);
		}
		control_builder& monthcalendar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(monthcalendar_control&)> _settings)
		{
			return create_field<monthcalendar_control>(_id, _field_label, _tooltip_text, _settings);
		}

		control_builder& image(int id, int _control_id, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id);
			apply_item_sizes(tc);
			tc->load_from_control(_control_id);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& image(int id, std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id);
			apply_item_sizes(tc);
			tc->load_from_file(_filename);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& image(std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto tc = create<image_control>(id_counter::next());
			apply_item_sizes(tc);
			tc->load_from_file(_filename);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& title(std::string text, std::function<void(title_control&)> _settings, int _id)
		{
			auto tc = create<title_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& subtitle(std::string text, std::function<void(subtitle_control&)> _settings, int _id)
		{
			auto tc = create<subtitle_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& chaptertitle(std::string text, std::function<void(chaptertitle_control&)> _settings, int _id)
		{
			auto tc = create<chaptertitle_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& chaptersubtitle(std::string text, std::function<void(chaptersubtitle_control&)> _settings, int _id)
		{
			auto tc = create<chaptersubtitle_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& paragraph(std::string text, std::function<void(paragraph_control&)> _settings, int _id)
		{
			auto tc = create<paragraph_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& code(std::string text, std::function<void(code_control&)> _settings, int _id)
		{
			auto tc = create<code_control>(_id);
			apply_item_sizes(tc);
			tc->text = text;
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& label(std::string _text, std::function<void(label_control&)> _settings, int _id)
		{
			auto tc = create<label_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(_text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& push_button(int _id, std::string text, std::function<void(pushbutton_control&)> _settings = nullptr)
		{
			auto tc = create<pushbutton_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& press_button(int _id, std::string text, std::function<void(pressbutton_control&)> _settings = nullptr)
		{
			auto tc = create<pressbutton_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& radio_button(int _id, std::string text, std::function<void(radiobutton_control&)> _settings = nullptr)
		{
			auto tc = create<radiobutton_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& checkbox(int _id, std::string text, std::function<void(checkbox_control&)> _settings = nullptr)
		{
			auto tc = create<checkbox_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& listbox(int _id, std::function<void(listbox_control&)> _settings = nullptr)
		{
			auto tc = create<listbox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& combobox(int _id, std::function<void(combobox_control&)> _settings = nullptr)
		{
			auto tc = create<combobox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& edit(int _id, std::function<void(edit_control&)> _settings = nullptr)
		{
			auto tc = create<edit_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& scrollbar(int _id, std::function<void(scrollbar_control&)> _settings = nullptr)
		{
			auto tc = create<scrollbar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& listview(int _id, std::function<void(listview_control&)> _settings = nullptr)
		{
			auto tc = create<listview_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& treeview(int _id, std::function<void(treeview_control&)> _settings = nullptr)
		{
			auto tc = create<treeview_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& header(int _id, std::function<void(header_control&)> _settings = nullptr)
		{
			auto tc = create<header_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& toolbar(int _id, std::function<void(toolbar_control&)> _settings = nullptr)
		{
			auto tc = create<toolbar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& statusbar(int _id, std::function<void(statusbar_control&)> _settings = nullptr)
		{
			auto tc = create<statusbar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& hotkey(int _id, std::function<void(hotkey_control&)> _settings = nullptr)
		{
			auto tc = create<hotkey_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& animate(int _id, std::function<void(animate_control&)> _settings = nullptr)
		{
			auto tc = create<animate_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& richedit(int _id, std::function<void(richedit_control&)> _settings = nullptr)
		{
			auto tc = create<richedit_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& draglistbox(int _id, std::function<void(draglistbox_control&)> _settings = nullptr)
		{
			auto tc = create<draglistbox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}


		control_builder& comboboxex(int _id, std::function<void(comboboxex_control&)> _settings = nullptr)
		{
			auto tc = create<comboboxex_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& datetimepicker(int _id, std::function<void(datetimepicker_control&)> _settings = nullptr)
		{
			auto tc = create<datetimepicker_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& monthcalendar(int _id, std::function<void(monthcalendar_control&)> _settings = nullptr)
		{
			auto tc = create<monthcalendar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& minimize_button(std::function<void(minimize_button_control&)> _settings = nullptr)
		{
			auto tc = create<minimize_button_control>(id_counter::next());
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
			;
		}

		control_builder& maximize_button(std::function<void(maximize_button_control&)> _settings = nullptr)
		{
			auto tc = create<maximize_button_control>(id_counter::next());
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& close_button(std::function<void(close_button_control&)> _settings = nullptr)
		{
			auto tc = create<close_button_control>(id_counter::next());
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& menu_button(int _id, std::function<void(menu_button_control&)> _settings = nullptr)
		{
			auto tc = create<menu_button_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& camera(int _id, std::function<void(camera_control&)> _settings = nullptr)
		{
			auto tc = create<camera_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& grid(int _id, std::function<void(grid_control&)> _settings = nullptr)
		{
			auto tc = create<grid_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& chart(int _id, std::function<void(chart_control&)> _settings = nullptr)
		{
			auto tc = create<chart_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& slide(int _id, std::function<void(slide_control&)> _settings = nullptr)
		{
			auto tc = create<slide_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& tab_button(int _id, std::function<void(tab_button_control&)> _settings = nullptr);
		control_builder& tab_view(int _id, std::function<void(tab_view_control&)> _settings = nullptr);
		control_builder& search_view(int _id, std::function<void(search_view_control&)> _settings = nullptr);
		control_builder& caption_bar(int _id, presentation_style* _st, menu_item* _mi, std::function<void(caption_bar_control&)> _settings = nullptr);
		control_builder& status_bar(int _id, std::function<void(status_bar_control&)> _settings = nullptr);
		control_builder& form_single_column(int _id, std::function<void(form_single_column_control&)> _settings = nullptr);
		control_builder& form_double_column(int _id, std::function<void(form_double_column_control&)> _settings = nullptr);

	};

	class tab_pane
	{
	public:
		int id;
		std::string name;
		std::function<void(tab_pane& _src, control_base*)> create_tab_controls;
	};

	class tab_view_control : public windows_control
	{
		std::vector<tab_pane> tab_panes;
		std::shared_ptr<frame_layout> content_frame;

		int active_id;



		void init()
		{
			children.clear();

			control_builder builder;

			on_create = [this](control_base* _item)
				{
					if (auto pwindow = window.lock())
					{
						pwindow->getContext().setSolidColorBrush(&border_brush);
					}
				};

			on_draw = [this](control_base* _item)
				{
					if (auto pwindow = window.lock())
					{
						if (auto phost = host.lock()) {
							auto draw_bounds = inner_bounds;

							draw_bounds.x = 0;
							draw_bounds.y = 0;

							auto& context = pwindow->getContext();

							if (this->is_focused) {
								context.drawRectangle(&draw_bounds, border_brush.name, 4, nullptr);
							}
						}
					}
				};

			auto main = builder.column_begin(id_counter::next(), [this](column_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_container);
				_settings.set_item_size(1.0_container, 1.0_container);
				});

			auto tab_row = main.row_begin(id_counter::next(), [](row_layout& _settings) {
				_settings.set_size(1.0_container, 30.0_px);
				});

			auto frame_row = main.frame_begin(id_counter::next(), [this](frame_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_remaining);
				});

			auto froot = frame_row.get_root();
			content_frame = std::dynamic_pointer_cast<frame_layout>(froot);

			for (int i = 0; i < tab_panes.size(); i++)
			{
				auto dat = tab_panes[i];

				if (!i)
				{
					active_id = dat.id;
				}

				tab_row.tab_button(dat.id, [this, dat](tab_button_control& _tb) {
					_tb.text = dat.name;
					_tb.active_id = &active_id;
					_tb.tab_selected = [this](tab_button_control& _src)->void
						{
							tab_selected(_src);
						};
				});
			}

			main.apply_controls(this);
		}

	public:

		virtual const char* get_window_class() { return "Corona2dControl"; }
		virtual DWORD get_window_style() { return DefaultWindowStyles; }
		virtual DWORD get_window_ex_style() { return WS_EX_LAYERED; }

		HBRUSH background_brush_win32;
		solidBrushRequest	background_brush;

		HBRUSH border_brush_win32;
		solidBrushRequest	border_brush;

		std::weak_ptr<applicationBase> host;
		std::weak_ptr<direct2dChildWindow> window;
		std::function<void(tab_view_control*)> on_draw;
		std::function<void(tab_view_control*)> on_create;

		tab_view_control()
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			border_brush_win32 = nullptr;
			border_brush = {};
			parent = nullptr;
			id = id_counter::next();
			set_border_color("#C0C0C0");
		}

		tab_view_control(const tab_view_control& _src)
		{
			tab_panes = _src.tab_panes;
			background_brush_win32 = nullptr;
			background_brush = _src.background_brush;
			border_brush_win32 = nullptr;
			border_brush = _src.border_brush;
			on_draw = _src.on_draw;
			on_create = _src.on_create;
			set_border_color("#C0C0C0");
		}

		tab_view_control(container_control_base* _parent, int _id)
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			border_brush_win32 = nullptr;
			border_brush = {};
			parent = _parent;
			id = _id;
			set_border_color("#C0C0C0");
		}

		virtual ~tab_view_control()
		{
			if (background_brush_win32)
			{
				::DeleteObject(background_brush_win32);
			}
			if (border_brush_win32)
			{
				::DeleteObject(border_brush_win32);
			}
		}

		virtual void arrange(rectangle _bounds)
		{
			set_bounds(_bounds);

			point origin = { 0, 0, 0 };
			point remaining = { _bounds.w, _bounds.h, 0.0 };

			arrange_children(bounds,
				[this](const rectangle* _bounds, control_base* _item) {
					point temp = { 0, 0, 0 };
					temp.x = _bounds->x;
					temp.y = _bounds->y;
					return temp;
				},
				[this](point* _origin, const rectangle* _bounds, control_base* _item) {
					point temp = *_origin;
					auto sz = _item->get_size(bounds, { _bounds->w, _bounds->h });
					temp.y += sz.y;
					temp.x = _bounds->x;
					return temp;
				}
			);
		}

		virtual point get_remaining(point _ctx)
		{
			return _ctx;
		}


		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<tab_view_control>(*this);
			return tv;
		}


		void set_tabs(std::vector<tab_pane> _new_panes)
		{
			tab_panes = _new_panes;
			init();

			
			default_tab_selected();
		}

		void tab_selected(std::vector<tab_pane>::iterator tbi)
		{
			if (tbi != tab_panes.end())
			{
				contents_generator<tab_pane*> cg;
				// set contents will clone this for us.
				cg.data = &*tbi;
				cg.generator = [](tab_pane* _tp, control_base* _args)
					{
						_tp->create_tab_controls(*_tp, _args);
					};
				content_frame->set_contents(cg);
			}
		}

		void tab_selected(tab_button_control& _tab)
		{
			auto tbi = std::find_if(tab_panes.begin(), tab_panes.end(), [this](tab_pane& _tb) {
				return _tb.id == this->active_id;
				});

			tab_selected(tbi);
		}

		void default_tab_selected()
		{

			auto tbi = tab_panes.begin();
			tab_selected(tbi);
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}
		virtual void create(std::weak_ptr<applicationBase> _host)
		{
			host = _host;
			if (auto phost = _host.lock()) {
				if (inner_bounds.x < 0 || inner_bounds.y < 0 || inner_bounds.w < 0 || inner_bounds.h < 0) {
					throw std::logic_error("inner bounds not initialized");
				}
				window = phost->createDirect2Window(id, inner_bounds);
			}
			if (on_create) {
				on_create(this);
			}
			for (auto child : children) {
				child->create(_host);
			}
			windows_control::create(_host);
		}

		void destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void on_resize()
		{
			auto ti = typeid(*this).name();

			if (auto pwindow = window.lock())
			{
				pwindow->moveWindow(inner_bounds.x, inner_bounds.y, inner_bounds.w, inner_bounds.h);
			}
		}

		void draw()
		{
			bool adapter_blown_away = false;

			if (auto pwindow = window.lock())
			{
				pwindow->beginDraw(adapter_blown_away);
				if (!adapter_blown_away)
				{
					auto& context = pwindow->getContext();

					auto& bc = background_brush.brushColor;

					if (border_brush.active)
					{
						if (border_brush_win32)
							DeleteBrush(border_brush_win32);

						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor(bc);
						border_brush_win32 = ::CreateSolidBrush(RGB(color.a * color.r * 255.0, color.a * color.g * 255.0, color.a * color.b * 255.0));
					}

					if (background_brush.active)
					{
						if (background_brush_win32)
							DeleteBrush(background_brush_win32);

						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor(bc);
						background_brush_win32 = ::CreateSolidBrush(RGB(color.a * color.r * 255.0, color.a * color.g * 255.0, color.a * color.b * 255.0));
						dc->Clear(color);
					}
					else
					{
						auto dc = context.getDeviceContext();
						D2D1_COLOR_F color = toColor("00000000");
						dc->Clear(color);
					}

					if (on_draw)
					{
						on_draw(this);
					}

					if (is_focused) 
					{
						rectangle r = get_inner_bounds();
						r.x = 0;
						r.y = 0;
						context.drawRectangle(&r, border_brush.name, 4, nullptr);
					}
				}
				pwindow->endDraw(adapter_blown_away);
			}
			for (auto& child : children) {
				child->draw();
			}
		}

		void render(ID2D1DeviceContext* _dest)
		{
			if (auto pwindow = window.lock())
			{
				auto bm = pwindow->getBitmap();
				D2D1_RECT_F dest;
				dest.left = inner_bounds.x;
				dest.top = inner_bounds.y;
				dest.right = inner_bounds.w + inner_bounds.x;
				dest.bottom = inner_bounds.h + inner_bounds.y;

				auto size = bm->GetPixelSize();
				D2D1_RECT_F source;
				source.left = 0;
				source.top = 0;
				source.bottom = inner_bounds.h;
				source.right = inner_bounds.w;
				_dest->DrawBitmap(bm, &dest, 1.0, D2D1_INTERPOLATION_MODE::D2D1_INTERPOLATION_MODE_LINEAR, &source);
			}
			for (auto& child : children)
			{
				child->render(_dest);
			}
		}

		tab_view_control& set_origin(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return *this;
		}

		tab_view_control& set_size(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return *this;
		}

		tab_view_control& set_background_color(solidBrushRequest _brushFill)
		{
			background_brush = _brushFill;
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&background_brush);
			}
			return *this;
		}

		tab_view_control& set_background_color(std::string _color)
		{
			background_brush.brushColor = toColor(_color.c_str());
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&background_brush);
			}
			return *this;
		}

		tab_view_control& set_border_color(solidBrushRequest _brushFill)
		{
			border_brush = _brushFill;
			border_brush.name = typeid(*this).name();
			border_brush.active = true;
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&border_brush);
			}
			return *this;
		}

		tab_view_control& set_border_color(std::string _color)
		{
			border_brush.brushColor = toColor(_color.c_str());
			border_brush.name = typeid(*this).name();
			border_brush.active = true;
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setSolidColorBrush(&border_brush);
			}
			return *this;
		}

		tab_view_control& set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		tab_view_control& set_margin(measure _item_space)
		{
			margin = _item_space;
			return *this;
		}
	};

	class search_view_control : public column_layout
	{

		void init()
		{
			idc_search_text = id_counter::next();
			idc_search_command = id_counter::next();
			idc_search_results = id_counter::next();

			control_builder cb;

			cb.row_begin(id_counter::next(), [](row_layout& _rl)
				{
					_rl.set_size(1.0_container, 75.0_px);
				})
				.label("Search")
					.edit(idc_search_text)
					.push_button(idc_search_command, "Go")
					.end()
					.column_view_begin(idc_search_results, [](column_view_layout& _cvl) {
					_cvl.set_size(1.0_container, 1.0_remaining);
						})
					.end();

			cb.apply_controls(this);
		}

	public:

		int idc_search_text;
		int idc_search_command;
		int idc_search_results;

		search_view_control()
		{
			idc_search_text = -1;
			idc_search_command = -1;
			idc_search_results = -1;
		}

		search_view_control(const search_view_control& _src) = default;

		search_view_control(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			idc_search_text = -1;
			idc_search_command = -1;
			idc_search_results = -1;
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<search_view_control>(*this);
			return tv;
		}

		void set_item_source(array_data_source _item_source)
		{
			auto& cview = control_base::find<column_view_layout>(idc_search_results);
			cview.set_item_source(_item_source);
		}

		virtual ~search_view_control()
		{
			;
		}
	};

	class caption_bar_control : public container_control
	{
		void init()
		{
			set_size(1.0_container, 80.0_px);

			control_builder cb;

			auto main_row = cb.row_begin(id_counter::next(), [this](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_background_color(st->HeaderBackgroundColor);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				rl.set_nchittest(HTCAPTION);
				});

			auto logo_row = main_row.row_begin(id_counter::next(), [](row_layout& rl) {
				rl.set_size(.50_container, 1.0_container);
				})
				.row_begin(id_counter::next(), [](row_layout& cl) {
					cl.set_content_align(visual_alignment::align_near);
					cl.set_content_cross_align(visual_alignment::align_near);
					cl.set_size(130.0_px, 1.0_container);
					cl.set_item_margin(5.0_px);
					})
					.menu_button(menu_button_id, [this](auto& _ctrl) {
						_ctrl.set_size(50.0_px, 50.0_px);
						_ctrl.set_margin(5.0_px);
						_ctrl.menu = *menu;
						})
				.end()
				.column_begin(id_counter::next(), [](column_layout& cl) {
							cl.set_content_align(visual_alignment::align_center);
							cl.set_content_cross_align(visual_alignment::align_near);
							cl.set_size(1.0_container, 1.0_container);
							cl.set_item_margin(5.0_px);
				})
				.code(code_status_id, [](code_control& control) {
					control.text_style.horizontal_align = visual_alignment::align_near;
					control.text_style.vertical_align = visual_alignment::align_near;
					control.text_style.underline = true;
					control.set_size(1.0_container, 1.4_fontgr);
								})
				.code(code_detail_id, [](code_control& control) {
									control.text_style.horizontal_align = visual_alignment::align_near;
									control.text_style.vertical_align = visual_alignment::align_far;
									control.text_style.underline = false;
									control.set_size(1.0_container, 2.2_fontgr);
						})
			.end();

			auto title_column = main_row.row_begin(id_counter::next(), [](row_layout& cl) {
				cl.set_content_align(visual_alignment::align_near);
				cl.set_content_cross_align(visual_alignment::align_near);
				cl.set_item_margin(0.0_px);
				cl.set_size(500.0_px, 1.0_container);
					})
				.column_begin(id_counter::next(), [](column_layout& cl) {
						cl.set_content_align(visual_alignment::align_near);
						cl.set_content_cross_align(visual_alignment::align_near);
						cl.set_item_margin(6.0_px);
						cl.set_size(80.0_px, 1.0_container);
					})
				.image(image_control_id, image_file, [](image_control& control) {
						control.set_size(50.0_px, 50.0_px);
				})
				.end()
				.column_begin(id_counter::next(), [](column_layout& cl) {
								cl.set_content_align(visual_alignment::align_near);
								cl.set_content_cross_align(visual_alignment::align_near);
								cl.set_item_margin(0.0_px);
								cl.set_size(1.0_remaining, 1.0_container);
					})
						.title(corporate_name, [](title_control& control) {
						control.text_style.horizontal_align = visual_alignment::align_near;
						control.text_style.vertical_align = visual_alignment::align_near;
						control.set_size(400.0_px, 1.2_font);
							})
						.title(title_name, [](title_control& control) {
								control.text_style.horizontal_align = visual_alignment::align_near;
								control.text_style.vertical_align = visual_alignment::align_near;
								control.set_size(400.0_px, 1.2_font);
							})
				.end()
				.end()
			.end();

/*			if (title_bar_id)
			{
				title_column.get_root()->push(title_bar_id, true, false, false, false);
			}
			*/

			auto frame_buttons = main_row.row_begin(id_counter::next(), [](row_layout& rl) {
					rl.set_size(.95_remaining, 1.0_container);
					rl.set_item_margin(5.0_px);
					rl.set_content_cross_align(visual_alignment::align_center);
					rl.set_content_align(visual_alignment::align_far);
				})
				.minimize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
				.maximize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
				.close_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
			.end();

			cb.apply_controls(this);

		}

	public:
		presentation_style* st;
		int menu_button_id;
		menu_item* menu;
		int image_control_id;
		std::string image_file;
		std::string corporate_name;
		std::string title_name;
		int code_status_id;
		int code_detail_id;

		caption_bar_control()
		{
			st = nullptr;
			menu = nullptr;
			menu_button_id = 0;
			image_control_id = 0;
		}

		caption_bar_control(const caption_bar_control& _src) = default;

		caption_bar_control(container_control_base *_parent, 
			int _id, 
			presentation_style *_st, 
			menu_item *_mi ) : container_control(_parent, _id)
		{
			st = _st;
			menu = _mi;
			menu_button_id = id_counter::next();
			image_control_id = id_counter::next();
		}

		std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<caption_bar_control>(*this);
			return tv;
		}

		void build()
		{
			init();
		}

		virtual ~caption_bar_control() { ; }

	};

	class status_bar_control : public container_control
	{
	public:

		void init()
		{
			control_builder cb;
			cb.row_begin(id_counter::status_bar_id, [this](row_layout& rl) {
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
							.end()
								.end();

							cb.apply_controls(this);
		}

		presentation_style* st = nullptr;

		status_bar_control() { ; }
		status_bar_control(const status_bar_control& _src) { ; }
		status_bar_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }

		std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<status_bar_control>(*this);
			return tv;
		}

		virtual ~status_bar_control() { ; }
	};

	class form_single_column_control : public container_control
	{
	public:

		int align_id;
		std::string form_name;
		std::function<void(container_control& _settings)> add_controls;

		form_single_column_control() { ; }
		form_single_column_control(const form_single_column_control& _src) = default;
		form_single_column_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~form_single_column_control() { ; }

		std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<form_single_column_control>(*this);
			return tv;
		}

		void init()
		{
			control_builder cb;

			cb.row_begin(id_counter::next(), [](row_layout& r)
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

			cb.apply_controls(this);

		}
	};

	class form_double_column_control : public container_control
	{

	public:

		void init()
		{
			control_builder cb;

			cb.row_begin(id_counter::next(), [](row_layout& r)
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

			cb.apply_controls(this);
		}

		int align_id;
		std::string form_name;
		std::function<void(container_control& _settings)> add_controls1;
		std::function<void(container_control& _settings)> add_controls2;

		form_double_column_control() { ; }
		form_double_column_control(const form_double_column_control& _src) = default;
		form_double_column_control(container_control_base* _parent, int _id) : container_control(_parent, _id) { ; }
		virtual ~form_double_column_control() { ; }

		std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<form_double_column_control>(*this);
			return tv;
		}

	};

	// implementation

	control_builder& control_builder::tab_button(int _id, std::function<void(tab_button_control&)> _settings)
	{
		auto tc = create<tab_button_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::tab_view(int _id, std::function<void(tab_view_control&)> _settings)
	{
		auto tc = create<tab_view_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::search_view(int _id, std::function<void(search_view_control&)> _settings)
	{
		auto tc = create<search_view_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::caption_bar(int _id, presentation_style* _st, menu_item* _mi, std::function<void(caption_bar_control&)> _settings)
	{

		auto cp = root.get();
		std::shared_ptr<caption_bar_control> tc;
		tc = std::make_shared<caption_bar_control>(cp, _id, _st, _mi);
		if (tc) {
			root->children.push_back(tc);
			std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			tc->build();
		}
		return *this;
	}

	control_builder& control_builder::status_bar(int _id, std::function<void(status_bar_control&)> _settings)
	{
		auto tc = create<status_bar_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::form_single_column(int _id, std::function<void(form_single_column_control&)> _settings)
	{
		auto tc = create<form_single_column_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::form_double_column(int _id, std::function<void(form_double_column_control&)> _settings)
	{
		auto tc = create<form_double_column_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}
}

#endif
