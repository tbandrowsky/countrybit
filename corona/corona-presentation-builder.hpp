/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This file contains advanced composed controllers and a more
fluent code style builder for building them.

Notes

For Future Consideration
*/



#ifndef CORONA_PRESENTATION_BUILDER_H
#define CORONA_PRESENTATION_BUILDER_H

namespace corona
{

	class tab_view_control;
	class search_view_control;
	class caption_bar_control;
	class status_bar_control;
	class form_control;
	class form_field_control;
	class checkbox_list_control;
	class radiobutton_list_control;

	class form_field 
	{
	public:
		int		 	  field_id;
		std::string   json_member_name;
		std::string   label_text;
		std::string   field_type;
		std::string   tooltip_text;
		std::string   input_format;
		std::string   suffix;
		std::string   prefix;
		bool		  read_only;
		bool		  is_default_focus;
		json		  settings_data;
		list_data	  source_list;
		table_data	  source_table;
		measure		  visualization_height;
		std::function<void(form_field_control* _src, draw_control* _dest)> draw_visualization;
		function_bag<control_base>  settings;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("field_id", field_id);
			_dest.put_member("json_member_name", json_member_name);
			_dest.put_member("label_text", label_text);
			_dest.put_member("field_type", field_type);
			_dest.put_member("tooltip_text", tooltip_text);
			_dest.put_member("input_format", input_format);
			_dest.put_member("suffix", suffix);
			_dest.put_member("prefix", prefix);
			_dest.put_member("read_only", read_only);
			_dest.put_member("is_default_focus", is_default_focus);
			_dest.put_member("settings_data", settings_data);

			json_parser jp;
			json jsource_list = jp.create_object();
			json jsource_table = jp.create_object();
			json jvisualization_height = jp.create_object();
			source_list.get_json(jsource_list);
			source_table.get_json(jsource_table);
			corona::get_json(jvisualization_height, visualization_height);
			_dest.put_member("source_list", jsource_list);
			_dest.put_member("source_table", jsource_table);
			_dest.put_member("visualization", jvisualization_height);
		}

		virtual void put_json(json& _src)
		{
			field_id = _src.get_member("field_id");
			json_member_name = _src.get_member("json_member_name");
			label_text = _src.get_member("label_text");
			field_type = _src.get_member("field_type");
			tooltip_text = _src.get_member("tooltip_text");
			input_format = _src.get_member("input_format");
			suffix = _src.get_member("suffix");
			prefix = _src.get_member("prefix");
			read_only = (bool)_src.get_member("read_only");
			is_default_focus = (bool)_src.get_member("is_default_focus");
			settings_data = _src.get_member("settings_data");

			json jsource_list = _src["source_list"];
			json jsource_table = _src["source_table"];
			json jvisualization_height = _src["visualization"];

			source_list.put_json(jsource_list);
			source_table.put_json(jsource_table);
			corona::put_json(visualization_height, jvisualization_height);

		}

	};

	class control_builder
	{
		std::shared_ptr<container_control> root;
		control_builder *parent;

	public:

		template <typename control_type> std::shared_ptr<control_type> get(int _id)
		{
			std::shared_ptr<control_type> temp;
			auto found = std::find_if(root->children.begin(), root->children.end(), [_id](auto& ch) {
				return ch->id == _id;
				});
			if (found != std::end(root->children)) {
				temp = std::dynamic_pointer_cast<control_type>(*found);
			}
			return temp;
		}

		template <typename control_type> std::shared_ptr<control_type> create(int _id)
		{
			auto cp = root.get();
			std::shared_ptr<control_type> temp;
			auto found = std::find_if(root->children.begin(), root->children.end(), [_id](auto& ch) {
				return ch->id == _id;
				});
			if (found != std::end(root->children)) {
				temp = std::dynamic_pointer_cast<control_type>(*found);
				if (temp == nullptr) {
					temp = std::make_shared<control_type>(cp, _id);
					*found = temp;
				}
			}
			else 
			{
				temp = std::make_shared<control_type>(cp, _id);
				if (temp) {
					root->children.push_back(temp);
				}
			}
			return temp;
		}

		template <typename control_type> void apply_item_sizes(control_type _ref)
		{
			if (!_ref) {
				return;
			}

			if (root->item_box.height.amount > 0)
			{
				_ref->box.height = root->item_box.height;
			}

			if (root->item_box.width.amount > 0)
			{
				_ref->box.width = root->item_box.width;
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
			std::function<void(field_control&)> _settings = nullptr, 
			measure _height = 50.0_px)
		{
			auto cl = column_begin(id_counter::next(), [_height](column_layout& _src) {
				_src.set_size(1.0_container, _height);
			});
			auto& lb = cl.label(_field_label);
			auto tc = cl.create<field_control>(_id);
			tc->set_size(1.0_container, 1.0_remaining);
			tc->tooltip_text = _tooltip_string;
			cl.apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			cl.end();
			return *this;
		}

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

		control_builder(std::shared_ptr<container_control> _root, int _id )
		{
			parent = nullptr;
			root = _root->find_by_id<container_control>(_id);
		}

		control_builder(control_builder* _parent, std::shared_ptr<container_control>& _root)
		{
			parent = _parent;
			root = _root;
		}

		control_builder(const control_builder& _src)
		{
			parent = _src.parent;
			root = _src.root;
		}

		control_builder &operator =(const control_builder& _src)
		{
			parent = _src.parent;
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

		inline control_builder& error(call_status _status) { return error(_status, nullptr, id_counter::next()); }
		inline control_builder& status(call_status _status) { return status(_status, nullptr, id_counter::next()); }
		inline control_builder& success(call_status _status) { return success(_status, nullptr, id_counter::next()); }

		inline control_builder& title(int _id, std::string _text) { return title(_text, nullptr, _id); }
		inline control_builder& subtitle(int _id, std::string _text) { return subtitle(_text, nullptr, _id); }
		inline control_builder& chaptertitle(int _id, std::string _text) { return chaptertitle(_text, nullptr, _id); }
		inline control_builder& chaptersubtitle(int _id, std::string _text) { return chaptersubtitle(_text, nullptr, _id); }
		inline control_builder& paragraph(int _id, std::string _text) { return paragraph(_text, nullptr, _id); }
		inline control_builder& code(int _id, std::string _text) { return code(_text, nullptr, _id); }
		inline control_builder& label(int _id, std::string _text) { return label(_text, nullptr, _id); }

		inline control_builder& title(int _id, std::function<void(title_control&)> _settings) { return title("", _settings, _id); }
		inline control_builder& subtitle(int _id, std::function<void(subtitle_control&)> _settings) { return subtitle("", _settings, _id); }
		inline control_builder& authorscredit(int _id, std::function<void(authorscredit_control&)> _settings) { return authorscredit("", _settings, _id); }
		inline control_builder& chaptertitle(int _id, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle("", _settings, _id); }
		inline control_builder& chaptersubtitle(int _id, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle("", _settings, _id); }
		inline control_builder& paragraph(int _id, std::function<void(paragraph_control&)> _settings) { return paragraph("", _settings, _id); }
		inline control_builder& code(int _id, std::function<void(code_control&)> _settings) { return code("", _settings, _id); }
		inline control_builder& label(int _id, std::function<void(label_control&)> _settings) { return label("", _settings, _id); }

		inline control_builder& title(std::string _text, std::function<void(title_control&)> _settings) { return title(_text, _settings, id_counter::next()); }
		inline control_builder& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings) { return subtitle(_text, _settings, id_counter::next()); }
		inline control_builder& authorscredit(std::string _text, std::function<void(authorscredit_control&)> _settings) { return authorscredit(_text, _settings, id_counter::next()); }
		inline control_builder& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings) { return chaptertitle(_text, _settings, id_counter::next()); }
		inline control_builder& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings) { return chaptersubtitle(_text, _settings, id_counter::next()); }
		inline control_builder& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings) { return paragraph(_text, _settings, id_counter::next()); }
		inline control_builder& code(std::string _text, std::function<void(code_control&)> _settings) { return code(_text, _settings, id_counter::next()); }
		inline control_builder& label(std::string _text, std::function<void(label_control&)> _settings) { return label(_text, _settings, id_counter::next()); }

		inline control_builder& error(call_status _status, std::function<void(error_control&)> _settings) { return error(_status, _settings, id_counter::next()); }
		inline control_builder& status(call_status _status, std::function<void(status_control&)> _settings) { return status(_status, _settings, id_counter::next()); }
		inline control_builder& success(call_status _status, std::function<void(success_control&)> _settings) { return success(_status, _settings, id_counter::next()); }

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

		control_builder grid_view_begin(int id, std::function<void(grid_view&)> _settings)
		{
			auto tc = create<grid_view>(id);
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

		control_builder& listbox(int _id, std::function<void(listbox_control&)> _settings)
		{
			auto tc = create<listbox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& combobox(int _id, std::function<void(combobox_control&)> _settings)
		{
			auto tc = create<combobox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& edit_field(int _id, std::function<void(edit_control&)> _settings)
		{
			auto tc = create<edit_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& listview_field(int _id, std::function<void(listview_control&)> _settings)
		{
			auto tc = create<listview_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& treeview_field(int _id, std::function<void(treeview_control&)> _settings)
		{
			auto tc = create<treeview_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& header_field(int _id, std::function<void(header_control&)> _settings)
		{
			auto tc = create<header_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& toolbar_field(int _id, std::function<void(toolbar_control&)> _settings)
		{
			auto tc = create<toolbar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& statusbar_field(int _id, std::function<void(statusbar_control&)> _settings)
		{
			auto tc = create<statusbar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& hotkey_field(int _id, std::function<void(hotkey_control&)> _settings)
		{
			auto tc = create<hotkey_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& animate_field(int _id, std::function<void(animate_control&)> _settings)
		{
			auto tc = create<animate_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& richedit_field(int _id, std::function<void(richedit_control&)> _settings)
		{
			auto tc = create<richedit_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& draglistbox_field(int _id, std::function<void(draglistbox_control&)> _settings)
		{
			auto tc = create<draglistbox_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& comboboxex_field(int _id, std::function<void(comboboxex_control&)> _settings)
		{
			auto tc = create<comboboxex_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& datetimepicker_field(int _id, std::function<void(datetimepicker_control&)> _settings)
		{
			auto tc = create<datetimepicker_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
		control_builder& monthcalendar_field(int _id, std::function<void(monthcalendar_control&)> _settings)
		{
			auto tc = create<monthcalendar_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
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

		control_builder& draw(int id, std::function<void(draw_control&)> _settings)
		{
			auto tc = create<draw_control>(id);
			apply_item_sizes(tc);
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

		control_builder& camera(std::function<void(camera_control&)> _settings)
		{
			auto tc = create<camera_control>(id_counter::next());
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}
	
		control_builder& camera_view(std::function<void(camera_view_control&)> _settings)
		{
			auto tc = create<camera_view_control>(id_counter::next());
			apply_item_sizes(tc);
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

		control_builder& authorscredit(std::string text, std::function<void(authorscredit_control&)> _settings, int _id)
		{
			auto tc = create<authorscredit_control>(_id);
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

		control_builder& success(call_status _status, std::function<void(success_control&)> _settings, int _id)
		{
			auto tc = create<success_control>(_id);
			apply_item_sizes(tc);
			tc->set_status(_status);
			if (_settings) {
				_settings(*tc);

			}
			return *this;
		}

		control_builder& error(call_status _status, std::function<void(error_control&)> _settings, int _id)
		{
			auto tc = create<error_control>(_id);
			apply_item_sizes(tc);
			tc->set_status(_status);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& status(call_status _status, std::function<void(status_control&)> _settings, int _id)
		{
			auto tc = create<status_control>(_id);
			apply_item_sizes(tc);
			tc->set_status(_status);
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

		control_builder& radiobutton_list(int _id, std::function<void(radiobutton_list_control&)> _settings = nullptr)
		{
			auto tc = create<radiobutton_list_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& checkbox_list(int _id,  std::function<void(checkbox_list_control&)> _settings = nullptr)
		{
			auto tc = create<checkbox_list_control>(_id);
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

		control_builder& minimize_button(int _id, std::function<void(minimize_button_control&)> _settings = nullptr)
		{
			auto tc = create<minimize_button_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
			;
		}

		control_builder& maximize_button(int _id, std::function<void(maximize_button_control&)> _settings = nullptr)
		{
			auto tc = create<maximize_button_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& close_button(int _id, std::function<void(close_button_control&)> _settings = nullptr)
		{
			auto tc = create<close_button_control>(_id);
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

		control_builder& corona_button(int _id, std::function<void(corona_button_control&)> _settings = nullptr)
		{
			auto tc = create<corona_button_control>(_id);
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
		control_builder& caption_bar(int _id, std::function<void(caption_bar_control&)> _settings = nullptr);
		control_builder& status_bar(int _id, std::function<void(status_bar_control&)> _settings = nullptr);
		control_builder& form(int _id, std::function<void(form_control&)> _settings = nullptr);
		control_builder& form_field(int _id, std::function<void(form_field_control&)> _settings = nullptr);

		std::shared_ptr<control_base> from_json(json _control_properties);
	};

	class form_field_control : public column_layout
	{
		std::shared_ptr<label_control>  label;
		std::shared_ptr<label_control>  prefix;
		std::shared_ptr<label_control>  suffix;
		std::shared_ptr<draw_control>	visualization;
		std::shared_ptr<control_base>	field;
		std::string						error_text;

		form_control*					form;

		template <typename field_control> std::shared_ptr<field_control> create_control(control_builder& cl, int _field_id, std::function<void(field_control&)> _settings)
		{			
			auto tc = cl.create<field_control>(_field_id);
			tc->set_size(1.0_container, 1.0_remaining);
			cl.apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return tc;
		}


	public:

		form_field_control() : form(nullptr)
		{
			;
		}

		form_field_control(const form_field_control& _src) : column_layout(_src)
		{
			label = _src.label;
			prefix = _src.prefix;
			suffix = _src.suffix;
			visualization = _src.visualization;
			field = _src.field;
			error_text = _src.error_text;
			form = _src.form;
		}

		form_field_control(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			;
		}

		std::string get_suffix()
		{
			if (suffix) {
				return suffix->text;
			}
			return std::string();
		}

		void set_suffix(std::string _text)
		{
			if (suffix) {
				suffix->set_text(_text);
			}
		}

		std::string get_prefix()
		{
			if (prefix) {
				return prefix->text;
			}
			return std::string();
		}

		void set_prefix(std::string _text)
		{
			if (prefix) {
				prefix->set_text(_text);
			}
		}

		std::string get_label()
		{
			if (label) {
				return label->text;
			}
			return std::string();
		}

		void set_label(std::string _text)
		{
			if (label) {
				label->set_text(_text);
			}
		}

		void set_error(std::string _text)
		{
			error_text = _text;
		}

		std::string get_error()
		{
			return error_text;
		}

		void set_field(
			int _field_id,
			std::string _label,
			std::string _prefix,
			std::string _suffix,
			json control_props,
			measure _visualization_height,
			std::function<void(form_field_control* _src, draw_control* _dest)> _draw_visualization)
		{
			control_builder cb;

			set_label(_label);

			if (!_prefix.empty())
			{
				set_prefix(_prefix);
			}

			if (!_suffix.empty())
			{
				set_suffix(_suffix);
			}

			if (_draw_visualization)
			{
				int draw_id = id_counter::next();
				visualization = create_control<draw_control>(cb, draw_id, [this, _label, _visualization_height, _draw_visualization](draw_control& _settings) {
					_settings.set_size(1.0_container, _visualization_height);
					_settings.on_draw = [this, _draw_visualization](draw_control* control) {
						_draw_visualization(this, control);
						};
					});
			}

			if (!_label.empty())
			{
				int label_id = id_counter::next();
				label = create_control<label_control>(cb, label_id, [_label](label_control& _settings) {
					_settings.text = _label;
					});
			}

			auto edit_row = cb.row_begin(id_counter::next(), [](row_layout& _row) {
				_row.set_size(1.0_remaining, 50.0_px);
				});

			if (!_prefix.empty())
			{
				int prefix_id = id_counter::next();
				prefix = create_control<label_control>(edit_row, prefix_id, [_prefix](label_control& _settings) {
					_settings.text = _prefix;
					});
			}

			control_props.put_member("id", _field_id);
			cb.from_json(control_props);
			field = cb.get<control_base>(_field_id);

			if (!_suffix.empty())
			{
				int suffix_id = id_counter::next();
				suffix = create_control<label_control>(edit_row, suffix_id, [_prefix](label_control& _settings) {
					_settings.text = _prefix;
					});
			}

			cb.apply_controls(this);

			double form_height = { 0 };

			for (auto r : children)
			{
				double height = to_pixels_y(r->box.height);
				form_height += height;
			}

			set_size(1.0_container, measure(form_height, measure_units::pixels));
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			_page->on_list_changed(field->id, [this](list_changed_event lce)
				{
					if (form) {
						form->list_changed(lce, this);
					}
				});

			_page->on_item_changed(field->id, [this](item_changed_event lce)
				{
					if (form) {
						form->item_changed(lce, this);
					}
				});

			_page->on_command(field->id, [this](command_event lce)
				{
					if (form) {
						form->field_command(lce, this);
					}
				});

			_page->on_mouse_click(field.get(), [this](mouse_click_event lce)
				{
					if (form) {
						form->field_mouse_click(lce, this);
					}
				});

			_page->on_mouse_move(field.get(), [this](mouse_move_event lce)
				{
					if (form) {
						form->field_mouse_move(lce, this);
					}
				});

		}

		virtual ~form_field_control()
		{
			;
		}

	};

	using form_list_changed_handler = std::function<void(list_changed_event lce, form_field_control* _ctrl)>;
	using form_item_changed_handler = std::function<void(item_changed_event lce, form_field_control* _ctrl)>;
	using form_command_handler = std::function<void(command_event lce, form_field_control* _ctrl)>;
	using form_mouse_click_handler = std::function<void(mouse_click_event lce, form_field_control* _ctrl)>;
	using form_mouse_move_handler = std::function<void(mouse_move_event lce, form_field_control* _ctrl)>;

	class form_model
	{
	public:
		std::string				name;
		std::vector<form_field> fields;
		json					data;

		form_list_changed_handler	on_list_changed;
		form_item_changed_handler	on_item_changed;
		form_command_handler		on_command;
		form_mouse_click_handler	on_mouse_click;
		form_mouse_move_handler		on_mouse_move;
	};

	class form_control : public column_layout
	{
		form_model ids;
		presentation_base* current_presentation;
		page_base* current_page;

	public:

		form_control()
		{
			;
		}

		form_control(const form_control& _src) : column_layout(_src)
		{
			ids = _src.ids;
			current_presentation = _src.current_presentation;
			current_page = _src.current_page;
		}

		form_control(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			current_presentation = nullptr;
			current_page = nullptr;
		}

		virtual ~form_control()
		{
			;
		}

		std::function<void(int _control_id, form_control* _fv)> on_changed;

		virtual json get_data()
		{
			json_parser jp;
			json obj = jp.create_object();

			for (auto& ctrl : ids.fields)
			{
				control_base *cb = find(ctrl.field_id);
				if (cb) {
					json field = cb->get_data();
					if (field.is_object())
					{
						for (auto member : field.get_members())
						{
							obj.put_member(member.first, member.second);
						}
					}
				}
			}

			return obj;
		}

		virtual json set_data(json _data)
		{
			json empty;

			for (auto& ctrl : ids.fields)
			{
				control_base* cb = find(ctrl.field_id);
				if (cb && cb->json_field_name.size()) {
					if (_data.has_member(cb->json_field_name)) {
						cb->set_data(_data);
					}
				}
			}

			return empty;
		}

		void set_data(form_model _ids)
		{
			children.clear();

			std::stack<control_builder> cbs;

			control_builder cb;
			cbs.push(cb);

			ids = _ids;

			bool is_default = true;

			for (auto &ctrl : ids.fields) 
			{
				auto* cbx = &cbs.top();

				if (ctrl.field_type == "row" || 
					ctrl.field_type == "column" || 
					ctrl.field_type == "absolute")
				{
					control_builder cb;
					cbs.push(cb);
				}
				else if (ctrl.field_type == "end")
				{
					cbs.pop();
					cbx = &cbs.top();
				}

				auto pctrl = &ctrl;
				cbx->form_field(id_counter::next(), [this,pctrl](form_field_control& _settings) {
					_settings.set_field(*pctrl, ids.data);
					_settings.
					});

				is_default = false;
			}

			auto& cbx = cbs.top();
			cbx.apply_controls(this);

			create(host);
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{

			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}

		virtual void list_changed(list_changed_event lce, form_field_control* _ctrl)
		{
			if (ids.on_list_changed) {
				ids.on_list_changed(lce, _ctrl);
			}
		}

		virtual void item_changed(item_changed_event lce, form_field_control* _ctrl)
		{
			if (ids.on_item_changed) {
				ids.on_item_changed(lce, _ctrl);
			}
		}

		virtual void field_command(command_event lce, form_field_control* _ctrl)
		{
			if (ids.on_command) {
				ids.on_command(lce, _ctrl);
			}
		}

		virtual void field_mouse_click(mouse_click_event lce, form_field_control *_ctrl)
		{
			if (ids.on_mouse_click) {
				ids.on_mouse_click(lce, _ctrl);
			}
		}

		virtual void field_mouse_move(mouse_move_event lce, form_field_control* _ctrl)
		{
			if (ids.on_mouse_move) {
				ids.on_mouse_move(lce, _ctrl);
			}
		}

	};

	class tab_pane
	{
	public:
		int id;
		std::string name;
		std::function<void(tab_pane& _src, control_base*)> apply_data;
		std::function<void(tab_pane& _src, control_base*)> create_tab_controls;
		std::vector<json> create_objects;
		std::vector<json> create_classes;
	};

	class tab_view_control : public windows_control
	{
		std::vector<tab_pane> tab_panes;
		std::shared_ptr<frame_layout> content_frame;
		presentation_base* current_presentation;
		page_base* current_page;
		std::map<int, std::shared_ptr<control_base>> pane_controls;
		int active_id;

		void init()
		{
			children.clear();

			control_builder builder;

			on_create = [this](control_base* _item)
				{
					if (auto pwindow = window.lock())
					{
						pwindow->getContext().setBrush(border_brush.get(), &inner_bounds);
					}
				};


			auto main = builder.column_begin(id_counter::next(), [this](column_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_container);
				_settings.set_item_size(1.0_container, 1.0_container);
				});

			auto tab_row = main.row_begin(id_counter::next(), [](row_layout& _settings) {
				_settings.set_size(1.0_container, 40.0_px);
				});

			auto frame_row = main.frame_begin(id_counter::next(), [this](frame_layout& _settings) {
				_settings.set_size(1.0_container, 1.0_remaining);
				});

			auto froot = frame_row.get_root();
			content_frame = std::dynamic_pointer_cast<frame_layout>(froot);

			for (int i = 0; i < tab_panes.size(); i++)
			{
				auto dat = tab_panes[i];

				if (!i && active_id <= 0)
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

		std::shared_ptr<generalBrushRequest>	background_brush;
		std::shared_ptr<generalBrushRequest>	border_brush;

		std::weak_ptr<applicationBase> host;
		std::weak_ptr<direct2dChildWindow> window;
		std::function<void(tab_view_control*)> on_draw;
		std::function<void(tab_view_control*)> on_create;

		tab_view_control()
		{
			parent = nullptr;
			id = id_counter::next();
			current_presentation = nullptr;
			current_page = nullptr;
			set_border_color("#C0C0C0");
			active_id = 0;
		}

		tab_view_control(const tab_view_control& _src)
		{
			tab_panes = _src.tab_panes;
			background_brush = _src.background_brush;
			border_brush = _src.border_brush;
			on_draw = _src.on_draw;
			on_create = _src.on_create;
			current_presentation = nullptr;
			current_page = nullptr;
			active_id = 0;

			set_border_color("#C0C0C0");
		}

		tab_view_control(container_control_base* _parent, int _id)
		{
			parent = _parent;
			id = _id;
			current_presentation = nullptr;
			current_page = nullptr;

			set_border_color("#C0C0C0");
		}

		virtual ~tab_view_control()
		{
		}

		virtual void arrange(rectangle _bounds)
		{
			set_bounds(_bounds);

			point origin = { 0, 0, 0 };
			point remaining = { _bounds.w, _bounds.h, 0.0 };

			arrange_children(bounds,
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

		virtual bool is_control_message(int _key)
		{
			bool is_message = is_focused && 
				(_key == VK_TAB || 
					_key == VK_LEFT || _key == VK_RIGHT || 
					_key == VK_UP || _key == VK_DOWN || 
					_key == VK_PRIOR || _key == VK_NEXT || 
					_key == VK_HOME || _key == VK_END ||
					_key == VK_DELETE || _key == VK_INSERT ||
					_key == VK_RETURN || _key == VK_DELETE);
			return is_message;
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
			tab_selected(active_id);
		}

		void tab_selected(std::vector<tab_pane>::iterator tbi)
		{
			if (tbi != tab_panes.end())
			{
				active_id = tbi->id;

				if (pane_controls.contains(active_id)) 
				{
					contents_generator<tab_pane*> cg;
					// set contents will clone this for us.
					cg.data = &*tbi;
					cg.generator = [this](tab_pane* _tp, control_base* _args)
						{
							_args->children.clear();
							_args->children.push_back(pane_controls[active_id]);
							_tp->apply_data(*_tp, pane_controls[active_id].get());
						};
					content_frame->set_contents(cg);
				}
				else 
				{
					contents_generator<tab_pane*> cg;
					// set contents will clone this for us.
					cg.data = &*tbi;
					cg.generator = [](tab_pane* _tp, control_base* _args)
						{
							_tp->create_tab_controls(*_tp, _args);
						};
					content_frame->set_contents(cg);
					pane_controls[active_id] = content_frame->children[0];
				}
				if (current_presentation && current_page) {
					content_frame->on_subscribe(current_presentation, current_page);
				}
			}
			else if (tab_panes.size()>0) {
				tab_selected(tab_panes.begin());
			}
		}

		void tab_selected(int _active_id)
		{
			auto tbi = std::find_if(tab_panes.begin(), tab_panes.end(), [this](tab_pane& _tb) {
				return _tb.id == this->active_id;
				});
			tab_selected(tbi);
		}

		void tab_selected(tab_button_control& _tab)
		{
			auto tbi = std::find_if(tab_panes.begin(), tab_panes.end(), [this](tab_pane& _tb) {
				return _tb.id == this->active_id;
				});
			tab_selected(tbi);
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			current_presentation = _presentation;
			current_page = _page;
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

		virtual void draw()
		{
			bool adapter_blown_away = false;

			std::cout << typeid(*this).name() << " tab_view::draw" << std::endl;

			if (auto pwindow = window.lock())
			{
				pwindow->beginDraw(adapter_blown_away);
				if (!adapter_blown_away)
				{
					auto& context = pwindow->getContext();

					const char* border_name = nullptr;
					const char* background_name = nullptr;
					D2D1_COLOR_F color;

					if (border_brush->get_name())
					{
						pwindow->getContext().setBrush(border_brush.get(), &inner_bounds);
						auto dc = context.getDeviceContext();
						border_name = border_brush->get_name();
					}

					if (background_brush->get_name())
					{
						pwindow->getContext().setBrush(background_brush.get(), &inner_bounds);
						auto dc = context.getDeviceContext();
						background_name = background_brush->get_name();
					}

					if (border_name || background_name) {
						rectangle r = inner_bounds;
						r.x = 0;
						r.y = 0;
						context.drawRectangle(&r, border_name, 4, background_name);
					}

					if (on_draw)
					{
						on_draw(this);
					} 

					if (is_focused && border_name) 
					{
						rectangle r = get_inner_bounds();
						r.x = 0;
						r.y = 0;
						context.drawRectangle(&r, border_name, 4, "");
					}
				}
				pwindow->endDraw(adapter_blown_away);
			}
			for (auto& child : children) {
				try {
					child->draw();
				}
				catch (std::exception exc)
				{
					std::cout << "Exception " << exc.what() << std::endl;
				}
			}
		}

		virtual void render(ID2D1DeviceContext* _dest)
		{
			if (auto pwindow = window.lock())
			{
				std::cout << typeid(*this).name() << " " << bounds.x << ", " << bounds.y << " " << bounds.w << " " << bounds.h << std::endl;

				auto bm = pwindow->getBitmap();
				D2D1_RECT_F dest;
				dest.left = bounds.x;
				dest.top = bounds.y;
				dest.right = bounds.w + bounds.x;
				dest.bottom = bounds.h + bounds.y;

				auto size = bm->GetPixelSize();
				D2D1_RECT_F source;
				source.left = 0;
				source.top = 0;
				source.bottom = bounds.h;
				source.right = bounds.w;
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
			background_brush = std::make_shared<generalBrushRequest>(_brushFill);
			background_brush->set_name(typeid(*this).name() );
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setBrush(background_brush.get(), &inner_bounds);
			}
			return *this;
		}

		tab_view_control& set_background_color(std::string _color)
		{
			background_brush->setColor(_color);
			background_brush->set_name(typeid(*this).name());
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setBrush(background_brush.get(), &inner_bounds);
			}
			return *this;
		}

		tab_view_control& set_border_color(solidBrushRequest _brushFill)
		{
			border_brush = std::make_shared<generalBrushRequest>(_brushFill);
			border_brush->set_name(typeid(*this).name());
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setBrush(border_brush.get(), &inner_bounds);
			}
			return *this;
		}

		tab_view_control& set_border_color(std::string _color)
		{
			border_brush->setColor(_color);
			border_brush->set_name( typeid(*this).name() );
			if (auto pwindow = window.lock())
			{
				pwindow->getContext().setBrush(border_brush.get(), &inner_bounds);
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
					.grid_view_begin(idc_search_results, [](grid_view& _cvl) {
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
			auto& cview = control_base::find<grid_view>(idc_search_results);
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
			set_size(1.0_container, 100.0_px);

			control_builder cb;

			auto main_row = cb.row_begin(id_counter::next(), [this](row_layout& rl) {
				rl.set_size(1.0_container, 100.0_px);
				rl.set_content_align(visual_alignment::align_far);
				rl.set_content_cross_align(visual_alignment::align_center);
				rl.set_item_margin(10.0_px);
				rl.set_style(presentation_style_factory::get_current()->get_style()->CaptionStyle);
				});

			main_row.column_begin(id_counter::next(), [](column_layout& cl) {
				cl.set_size(10.0_px, 1.0_container);
				cl.set_content_align(visual_alignment::align_center);
				});

			auto title_column = main_row.column_begin(id_counter::next(), [](column_layout& cl) {
				cl.set_size(1.0_remaining, 1.0_container);
				cl.set_content_align(visual_alignment::align_center);
				cl.set_item_margin(10.0_px);
				})
				.title(title_name, [this](title_control& control) {
						control.set_nchittest(HTCAPTION);
						control.set_size(0.8_container, 1.3_fontgr);
					}, title_id)
				.subtitle(subtitle_name, [this](subtitle_control& control) {
						control.set_nchittest(HTCAPTION);
						control.set_size(0.8_container, 1.3_fontgr);
					}, subtitle_id)
			.end();

			auto frame_buttons = main_row.row_begin(id_counter::next(), [](row_layout& rl) {
					rl.set_size(250.0_px, 1.0_container);
					rl.set_item_margin(5.0_px);
					rl.set_content_cross_align(visual_alignment::align_center);
					rl.set_content_align(visual_alignment::align_far);
				})
				.column_begin(id_counter::next(), [](column_layout& cl) {
					cl.set_content_align(visual_alignment::align_center);
					cl.set_content_cross_align(visual_alignment::align_near);
					cl.set_size(250.0_px, 1.0_container);
					cl.set_item_margin(0.0_px);
				})
				.end()
					.menu_button(menu_button_id, [this](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); if (menu) { _ctrl.menu = *menu };	})
				.minimize_button(min_button_id, [](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
				.maximize_button(max_button_id, [](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
				.close_button(close_button_id, [](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
			.end();

			cb.apply_controls(this);

		}

	public:

		int menu_button_id;
		int min_button_id;
		int max_button_id;
		int close_button_id;
		menu_item* menu;
		int image_control_id;
		std::string image_file;
		std::string corporate_name;
		std::string title_name;
		std::string subtitle_name;
		int title_id;
		int subtitle_id;
		data_lake* lake;

		caption_bar_control()
		{
			menu = nullptr;
			menu_button_id = 0;
			image_control_id = 0;
			lake = nullptr;
		}

		caption_bar_control(const caption_bar_control& _src) = default;

		caption_bar_control(container_control_base *_parent, int _id) : container_control(_parent, _id)
		{
			menu_button_id = id_counter::next();
			image_control_id = id_counter::next();
			min_button_id = id_counter::next();
			max_button_id = id_counter::next();
			close_button_id = id_counter::next();
			title_id = id_counter::next();
			subtitle_id = id_counter::next();
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

		void set_status(std::string _status, std::string _detail)
		{
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page)
		{
			for (auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}
	};

	class status_bar_control : public container_control
	{
	public:

		void init()
		{
			control_builder cb;
			cb.row_begin(id_counter::status_bar_id, [this](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				})
				.column_begin(id_counter::next(), [](column_layout& cl) {
					cl.set_content_align(visual_alignment::align_center);
					cl.set_content_cross_align(visual_alignment::align_near);
					cl.set_size(.3_container, 1.0_container);
					cl.set_item_margin(0.0_px);
					})
					.title(id_counter::status_text_title_id, [](title_control& control) {
						control.set_size(300.0_px, 1.2_fontgr);
						})
						.subtitle(id_counter::status_text_subtitle_id, [](subtitle_control& control) {
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

	control_builder& control_builder::caption_bar(int _id, std::function<void(caption_bar_control&)> _settings)
	{

		auto cp = root.get();
		std::shared_ptr<caption_bar_control> tc;
		tc = std::make_shared<caption_bar_control>(cp, _id);
		if (tc) {
			root->children.push_back(tc);
//			std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
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

	control_builder& control_builder::form(int _id, std::function<void(form_control&)> _settings)
	{
		auto tc = create<form_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	control_builder& control_builder::form_field(int _id, std::function<void(form_field_control&) > _settings)
	{
		auto tc = create<form_field_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

	std::shared_ptr<control_base> control_builder::from_json(json _control_properties)
	{
		json_parser jp;

		std::string class_name = _control_properties["class_name"];
		std::string field_name = _control_properties["name"];
		std::string field_type = _control_properties["type"];
		json control_data = _control_properties["data"];

		int id = (int)_control_properties.get_member("id");

		int field_id = id ? id : id_counter::next();

		std::string default_text = "";
		call_status default_status;

		json control_data = _control_properties.get_member("data");

		if (class_name == "title")
		{
			title(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "subtitle")
		{
			subtitle(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "authorscredit")
		{
			authorscredit(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "chaptertitle")
		{
			chaptertitle(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "chaptersubtitle")
		{
			chaptersubtitle(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "paragraph")
		{
			paragraph(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "code")
		{
			code(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "label")
		{
			label(default_text, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "error")
		{
			error(default_status, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "status")
		{
			status(default_status, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "success")
		{
			success(default_status, [this, _control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "row")
		{
			row_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "column")
		{
			column_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "absolute")
		{
			absolute_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "row_view")
		{
			row_view_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid_view")
		{
			grid_view_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "absolute_view")
		{
			absolute_view_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "frame")
		{
			frame_begin(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "listbox")
		{
			listbox(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "combobox")
		{
			combobox(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "edit")
		{
			edit(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "listview")
		{
			listview(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "treeview")
		{
			treeview(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "header")
		{
			header(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "toolbar")
		{
			toolbar(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "statusbar")
		{
			statusbar_field(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "hotkey")
		{
			hotkey(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "animate")
		{
			animate(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "richedit")
		{
			richedit(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "draglistbox")
		{
			draglistbox(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "comboboxex")
		{
			comboboxex(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "datetimepicker")
		{
			datetimepicker(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "monthcalendar")
		{
			monthcalendar(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "radiobutton_list")
		{
			radiobutton_list(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "checkbox_list")
		{
			checkbox_list(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "minimize_button")
		{
			minimize_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "maximize_button")
		{
			maximize_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "close_button")
		{
			close_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "menu_button")
		{
			menu_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "corona_button")
		{
			corona_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "camera")
		{
			camera(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid")
		{
			grid(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "chart")
		{
			chart(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "slide")
		{
			slide(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid")
		{
			grid(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "tab_button")
		{
			tab_button(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "tab_view")
		{
			tab_view(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "search_view")
		{
			search_view(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "caption_bar")
		{
			caption_bar(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "status_bar")
		{
			status_bar(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "form")
		{
			form(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "form_field")
		{
			form_field(field_id, [this, _control_properties, control_data](auto& _ctrl)->void {
				_ctrl.set_json(_control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else 
		{
			std::cout << "Undefined class_name for control:" << class_name << std::endl;
			std::cout << "Currently the following control classes are supported.  Set class_name to one of these." << std::endl;
			std::cout << "Text Box types" << std::endl;
			std::cout << "title, subtitle, chaptertitle, chaptersubtitle, paragraph, " << std::endl;
			std::cout << "code, label, error, status, success" << std::endl;
			std::cout << "Layout types" << class_name << std::endl;
			std::cout << "row, column, absolute, row_view, absolute_view, grid_view, grid, slide, frame, tab_button, tab_view, search_view" << class_name << std::endl;
			std::cout << "Windows SDK types" << class_name << std::endl;
			std::cout << "combobox, listbox, edit, listview, treeview, header, toolbar, statusbar, hotkey, animate, richedit, draglistbox, comboboxex, datetimepicker, monthcalendar, radiobutton_list, checkbox_list" << std::endl;
			std::cout << "System Button types" << class_name << std::endl;
			std::cout << "caption_bar, minimize_button, mnaximize_button, close_button, menu_button, corona_button" << std::endl;
			std::cout << "Forms types" << class_name << std::endl;
			std::cout << "form, form_field" << std::endl;
			std::cout << "" << class_name << std::endl;
		}

		std::shared_ptr<control_base> ret = get<control_base>(field_id);

		json children = _control_properties.get_member("children");
		if (children.is_array()) {

			control_builder cb;
			for (auto child : children)
			{
				auto new_child = cb.from_json(child);
				ret->children.push_back(new_child);
			}
		}

		return ret;
	}

	class radiobutton_list_control :
		public column_layout
	{
	protected:
		list_data choices;
		json data;

	public:
		radiobutton_list_control() { ; }
		radiobutton_list_control(const radiobutton_list_control& _src) = default;
		radiobutton_list_control(container_control_base* _parent, int _id) : column_layout(_parent, _id) { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<absolute_view_layout>(*this);
			return tv;
		}

		virtual ~radiobutton_list_control() { ; }

		void list_changed()
		{
			control_builder cb;

			children.clear();

			int count = choices.items.size();

			for (int i = 0; i < count; i++)
			{
				json item = choices.items.get_element(i);
				int id = item.get_member(choices.id_field);
				std::string text = item.get_member(choices.text_field);
				bool selected = (bool)item.get_member(choices.selected_field);
				cb.radio_button(id, text, [item, this, i](radiobutton_control& _rbc) {
					_rbc.json_field_name = choices.selected_field;
					_rbc.is_group = i == 0;
					_rbc.set_data(item);
					});
			}
		}

		virtual json get_data()
		{
			json result;
			if (!json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();
				json result_array = jp.create_array();

				for (auto child : children)
				{
					json data = child->get_data();
					result_array.put_element(-1, data);
				}

				result.put_member(json_field_name, result_array);

			}
			return result;
		}

		virtual json set_data(json _data)
		{
			data = _data;
			if (_data.has_member(json_field_name)) {
				json field_items = _data[json_field_name];
				if (field_items.is_array()) {
					json as_object = field_items.array_to_object(

						[this](json& _item)->std::string {
							return _item.get_member(choices.id_field);
						},
						[](json& _item)->json {
							return _item;
						}
					);
					for (auto child : children) {
						json existing = child->get_data();
						std::string key = choices.id_field;
						json item = as_object.get_member(key);
						child->set_data(item);
					}
				}
			}
			return _data;
		}

		void set_list(list_data& _choices)
		{
			choices = _choices;
			list_changed();
		}

		virtual void on_create()
		{
			list_changed();
		}

	};

	class checkbox_list_control :
		public column_layout
	{
	protected:
		list_data choices;
		json data;

	public:
		checkbox_list_control() { ; }
		checkbox_list_control(const checkbox_list_control& _src) = default;
		checkbox_list_control(container_control_base* _parent, int _id) : column_layout(_parent, _id) { ; }

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<absolute_view_layout>(*this);
			return tv;
		}

		virtual ~checkbox_list_control() { ; }

		void list_changed()
		{
			control_builder cb;

			children.clear();

			int count = choices.items.size();

			for (int i = 0; i < count; i++)
			{
				json item = choices.items.get_element(i);
				int id = item.get_member(choices.id_field);
				std::string text = item.get_member(choices.text_field);
				bool selected = (bool)item.get_member(choices.selected_field);
				cb.checkbox(id, text, [item, this](checkbox_control& _rbc) {
					_rbc.json_field_name = choices.selected_field;
					_rbc.set_data(item);
					});
			}
		}

		virtual json get_data()
		{
			json result;
			if (!json_field_name.empty()) {
				json_parser jp;
				result = jp.create_object();
				json result_array = jp.create_array();

				for (auto child : children)
				{
					json data = child->get_data();
					result_array.put_element(-1, data);
				}

				result.put_member(json_field_name, result_array);

			}
			return result;
		}

		virtual json set_data(json _data)
		{
			data = _data;
			if (_data.has_member(json_field_name)) {
				json field_items = _data[json_field_name];
				if (field_items.is_array()) {
					json as_object = field_items.array_to_object(

						[this](json& _item)->std::string {
							return _item.get_member(choices.id_field);
						},
						[](json& _item)->json {
							return _item;
						}
					);
					for (auto child : children) {
						json existing = child->get_data();
						std::string key = choices.id_field;
						json item = as_object.get_member(key);
						child->set_data(item);
					}
				}
			}
			return _data;
		}

		void set_list(list_data& _choices)
		{
			choices = _choices;
			list_changed();
		}

		virtual void on_create()
		{
			list_changed();
		}

	};
}

#endif
