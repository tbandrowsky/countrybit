/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This file contains controls that are compositions, and a more
fluent code style builder for building them.  The fluent code builder
is then used to build up the page collection.

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


	class corona_button_control : public pushbutton_control
	{
	public:
		using control_base::id;

		std::shared_ptr<call_status>	status;

		using windows_control::enable;
		using windows_control::disable;

		corona_button_control(container_control_base* _parent, int _id) : pushbutton_control(_parent, _id)
		{
			init();
		}

		corona_button_control(const corona_button_control& _src) : pushbutton_control(_src)
		{
			init();
			status = _src.status;
		}

		virtual ~corona_button_control() { ; }

		void init();
		virtual double get_font_size() { return text_style.fontSize; }

		virtual void get_json(json& _dest)
		{
			pushbutton_control::get_json(_dest);
		}
		virtual void put_json(json& _src)
		{
			pushbutton_control::put_json(_src);
		}
	};

	class form_field 
	{
	public:
		int		 	  field_id;
		std::string	  form_name;
		std::string   json_field_name;
		std::string   label_text;
		std::string   class_name;
		std::string   tooltip_text;
		std::string   input_format;
		std::string   suffix;
		std::string   prefix;
		bool		  read_only;
		bool		  is_default_focus;
        bool		  is_default_button;
		json		  control_settings;
		layout_rect	  box;
		layout_rect	  label_box;
		layout_rect	  visualization_box;
		layout_rect	  prefix_box;
		layout_rect	  field_box;
		layout_rect	  suffix_box;
		std::function<void(form_field_control* _src, draw_control* _dest)> draw_visualization;

		form_field()
		{
			field_id = {};
			read_only = false;
			is_default_focus = false;
			is_default_button = false;
		}

		form_field(const form_field& _src) = default;
		form_field(form_field&& _src) = default;
		form_field& operator = (const form_field& _src) = default;
		form_field& operator = (form_field&& _src) = default;

		virtual void get_json(json& _dest)
		{
			control_settings.put_member("default_focus", is_default_focus);
			control_settings.put_member("default_button", is_default_button);

			_dest.put_member("field_id", field_id);
			_dest.put_member("json_field_name", json_field_name);
			_dest.put_member("label_text", label_text);
			_dest.put_member("class_name", class_name);
			_dest.put_member("tooltip_text", tooltip_text);
			_dest.put_member("input_format", input_format);

			_dest.put_member("suffix", suffix);
			_dest.put_member("prefix", prefix);
			_dest.put_member("read_only", read_only);
			_dest.put_member("default_button", is_default_button);
			_dest.put_member("default_focus", is_default_focus);
			_dest.put_member("control_settings", control_settings);
			_dest.put_member("form_name", form_name);

			json_parser jp;
			json jsource_list = jp.create_object();
			json jsource_table = jp.create_object();
			json jvisualization_height = jp.create_object();

			json jcontainer_box = jp.create_object();
			corona::get_json(jcontainer_box, box);
			_dest.put_member("box", jcontainer_box);

			json jlabel_box = jp.create_object();
			corona::get_json(jlabel_box, label_box);
			_dest.put_member("label_box", jlabel_box);

			json jvisualization_box = jp.create_object();
			corona::get_json(jvisualization_box, visualization_box);
			_dest.put_member("visualization_box", jvisualization_box);

			json jprefix_box = jp.create_object();
			corona::get_json(jprefix_box, prefix_box);
			_dest.put_member("prefix_box", jprefix_box);

			json jfield_box = jp.create_object();
			corona::get_json(jfield_box, field_box);
			_dest.put_member("field_box", jfield_box);

			json jsuffix_box = jp.create_object();
			corona::get_json(jsuffix_box, suffix_box);
			_dest.put_member("suffix_box", jsuffix_box);

		}

		virtual void put_json(json& _src)
		{

			field_id = _src["field_id"];
			json_field_name = _src["json_field_name"];
			label_text = _src["label_text"];
			class_name = _src["class_name"];
			tooltip_text = _src["tooltip_text"];
			input_format = _src["input_format"];
			suffix = _src["suffix"];
			prefix = _src["prefix"];
			read_only = (bool)_src["read_only"];
			is_default_focus = (bool)_src["default_focus"];
			is_default_button = (bool)_src["default_button"];
			control_settings = _src["control_settings"];
            if (control_settings.has_member("default_focus")) {
                is_default_focus = (bool)control_settings["default_focus"];
            }
			if (control_settings.has_member("default_button")) {
				is_default_button = (bool)control_settings["default_button"];
			}
			form_name = _src["form_name"];

			json jcontainer_box = _src["box"];
			corona::put_json(box, jcontainer_box);

			json jlabel_box = _src["label_box"];
			corona::put_json(label_box, jlabel_box);

			json jvisualization_box = _src["visualization_box"];
			corona::put_json(visualization_box, jvisualization_box);

			json jprefix_box = _src["prefix_box"];
			corona::put_json(prefix_box, jprefix_box);

			json jfield_box = _src["field_box"];
			corona::put_json(field_box, jfield_box);

			json jsuffix_box = _src["suffix_box"];
			corona::put_json(suffix_box, jsuffix_box);
		}

	};

	class control_builder
	{
		std::shared_ptr<container_control> root;
		control_builder *parent;

	public:

		comm_bus_app_interface* bus;

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

			if (_id == 0)
				throw std::logic_error("cannot create a control with id 0");

			auto cp = root.get();
			std::shared_ptr<control_type> temp;
			auto found = std::find_if(root->children.begin(), root->children.end(), [_id](auto& ch) {
				return ch->id == _id;
				});

			// check to see if an element of this id exists
			if (found != std::end(root->children)) {
				// ok, so now we see if it is of the same type
				temp = std::dynamic_pointer_cast<control_type>(*found);

				// and, it is not, so we replace it
				if (temp == nullptr) {
					temp = std::make_shared<control_type>(cp, _id);
					if (temp) {
						// here's our new temp, and we copy the children
						// this may not be the best plan but it allows us
						// to easily switch container types.
						temp->children = (*found)->children;
						int found_index = std::distance(found, root->children.begin());
						root->children[found_index] = temp;
					}
				}

				// otherwise, we found an existing control of the same type and we use that.
			}
			else 
			{
				// and, since we found nothing, we go ahead and just shove our new element at the back
				temp = std::make_shared<control_type>(cp, _id);
				if (temp) {
					root->children.push_back(temp);
				}
			}
			return temp;
		}

		template <typename control_type> void apply_item_sizes(control_type _ref)
		{
			if (not _ref) {
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

		control_builder& link_button(int _id, std::function<void(linkbutton_control&)> _settings = nullptr)
		{
			auto tc = create<linkbutton_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& link_button(int _id, std::string text, std::function<void(linkbutton_control&) > _settings = nullptr)
		{
			auto tc = create<linkbutton_control>(_id);
			apply_item_sizes(tc);
			tc->set_text(text);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}


		control_builder& push_button(int _id, std::function<void(pushbutton_control&)> _settings = nullptr)
		{
			auto tc = create<pushbutton_control>(_id);
			apply_item_sizes(tc);
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

		control_builder& radio_button(int _id, std::function<void(radiobutton_control&)> _settings = nullptr)
		{
			auto tc = create<radiobutton_control>(_id);
			apply_item_sizes(tc);
			if (_settings) {
				_settings(*tc);
			}
			return *this;
		}

		control_builder& checkbox(int _id, std::function<void(checkbox_control&)> _settings = nullptr)
		{
			auto tc = create<checkbox_control>(_id);
			apply_item_sizes(tc);
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
		control_builder& password(int _id, std::function<void(edit_control&)> _settings = nullptr)
		{
			auto tc = create<password_control>(_id);
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
		control_builder& form_fieldy(int _id, json _field_def);
		control_builder& form_fieldy(int _id, form_field _field_def);
		control_builder& image(int _id, std::function<void(image_control&)> _settings = nullptr);

		std::shared_ptr<control_base> from_json(json _control_properties, layout_rect _default = {});
	};

	class form_field_control : public column_layout
	{
		int								form_id;
		int								field_id;
		int								label_id;
		int								prefix_id;
		int								suffix_id;
		int								visualization_id;
		int								edit_block_id;

		form_field						field_def;
		std::function<void(std::shared_ptr<direct2dContext>& _context, form_field_control* _src, draw_control* _dest)> draw_visualization;

		std::string						error_text;

	public:

		form_field_control() 
		{
			field_id = id_counter::next();
			label_id = id_counter::next();
			prefix_id = id_counter::next();
			suffix_id = id_counter::next();
			visualization_id = id_counter::next();
			edit_block_id = id_counter::next();
		}

		form_field_control(const form_field_control& _src) = default;
		form_field_control(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			label_id = id_counter::next();
			prefix_id = id_counter::next();
			suffix_id = id_counter::next();
			visualization_id = id_counter::next();
			edit_block_id = id_counter::next();
		}

		void set_error(std::string _text)
		{
			error_text = _text;
		}

		std::string get_error()
		{
			return error_text;
		}

		form_control* find_my_form()
		{
			std::string my_name = form_name();
			control_base *cb = find(my_name);
			if (cb and cb->form_name() == my_name) {
				// lord help me this is wrong.
				return (form_control *)cb;
			}
			return nullptr;
		}

		virtual void get_json(json& _dest)
		{
			json jfield;
			json_parser jp;
			jfield = jp.create_object();
			column_layout::get_json(_dest);
			field_def.get_json(jfield);
			_dest.put_member("options", jfield);
		}

		virtual void put_json(json& _src)
		{
			column_layout::put_json(_src);

			form_field temp;
			temp.put_json(_src);

			set_field(temp,
				nullptr
			);
		}

		virtual json get_data()
		{
			json result;
			auto field = find_by_id<control_base>(field_id);
			if (field) {
				result = field->get_data();
			}
			return result;
		}

		void set_field_visualization(std::function<void(std::shared_ptr<direct2dContext>& _context, form_field_control* _src, draw_control* _dest)> _draw_visualization)
		{
			set_field(field_def, _draw_visualization);
		}

		void set_field(
			form_field _field,
			std::function<void(std::shared_ptr<direct2dContext>& _context, form_field_control* _src, draw_control* _dest)> _draw_visualization)
		{
			control_builder cb;
			json_parser jp;

			field_id = _field.field_id;
			field_def = _field;
			box = _field.box;

			if (_draw_visualization) {
				draw_visualization = _draw_visualization;
			}

			if (not field_def.label_text.empty())
			{
				if (field_def.label_box.width.empty())
				{
					std::string msg = std::format("'{0}' {1} missing label_box width", field_def.label_text, field_def.class_name);
					comm_bus_app_interface::global_bus->log_warning(msg);
				}
				if (field_def.label_box.height.empty())
				{
					std::string msg = std::format("'{0}' {1} missing label_box height", field_def.label_text, field_def.class_name);
					comm_bus_app_interface::global_bus->log_warning(msg);
				}

				cb.label(label_id, [this](label_control& _settings) {
					_settings.text = field_def.label_text;
					_settings.set_box(field_def.label_box);
					});
			}

			if (draw_visualization)
			{
				cb.draw(visualization_id, [this](draw_control& _settings) {
					_settings.set_box(field_def.visualization_box);
					_settings.on_draw = [this](std::shared_ptr<direct2dContext>& _context, draw_control* control) {
						draw_visualization(_context, this, control);
						};
					});
			}

			auto edit_row = cb.row_begin(id_counter::next(), [this](row_layout& _row) {
				_row.set_size(1.0_container, field_def.field_box.height);
				});

			if (not field_def.prefix.empty())
			{
				edit_row.label(prefix_id, [this](label_control& _settings) {
					_settings.set_box(field_def.prefix_box);
					_settings.text = field_def.prefix;
					});
			}

			if (field_def.field_box.width.empty())
			{
				std::string msg = std::format("'{0}' {1} missing field_box width", field_def.label_text, field_def.class_name);
				comm_bus_app_interface::global_bus->log_warning(msg);
			}
			if (field_def.field_box.height.empty())
			{
				std::string msg = std::format("'{0}' {1} missing field_box height", field_def.label_text, field_def.class_name);
				comm_bus_app_interface::global_bus->log_warning(msg);
			}

			field_def.control_settings.put_member("default_button", field_def.is_default_button);
			field_def.control_settings.put_member("default_focus", field_def.is_default_focus);

			edit_row.from_json(field_def.control_settings);

			if (not field_def.suffix.empty())
			{
				edit_row.label(suffix_id, [this](label_control& _settings) {
					_settings.text = field_def.suffix;
					});
			}

			cb.apply_controls(this);
		}

		virtual std::shared_ptr<control_base> clone()
		{
			auto my_clone = std::make_shared<form_field_control>(*this);
			my_clone->set_field(field_def, draw_visualization);
			return my_clone;
		}

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

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



	class form_model : public json_serializable
	{
	private:
		void apply_defaults()
		{
			json_parser jp;

			int index_lists = 0;
			for (auto& fld : fields)
			{
				fld.form_name = name;
				if (fld.field_id == 0) {
					fld.field_id = id_counter::next();
				}
				if (fld.box.empty())
				{
					fld.box = defaults.box;
				}
				if (fld.label_box.empty())
				{
					fld.label_box = defaults.label_box;
				}
				if (fld.prefix_box.empty())
				{
					fld.prefix_box = defaults.prefix_box;
				}
				if (fld.field_box.empty())
				{
					fld.field_box = defaults.field_box;
				}
				if (fld.suffix_box.empty())
				{
					fld.suffix_box = defaults.suffix_box;
				}

				if (fld.class_name.empty()) {
					std::string msg;
					msg = std::format("form '{0}' fields.[{1}] does not have a class_name", name, index_lists);
					system_monitoring_interface::global_mon->log_warning(msg);
				}

				if (fld.control_settings.empty())
				{
					fld.control_settings = jp.create_object();
					fld.control_settings.put_member("id", fld.field_id);
					fld.control_settings.put_member("class_name", fld.class_name);
					fld.control_settings.put_member("json_field_name", fld.json_field_name);
					json jbox = jp.create_object();
					corona::get_json(jbox, fld.field_box);
					fld.control_settings.put_member("box", jbox);
				}
				else if (fld.control_settings.object())
				{
					fld.control_settings.put_member("id", fld.field_id);
					fld.control_settings.put_member("class_name", fld.class_name);
					fld.control_settings.put_member("json_field_name", fld.json_field_name);

					if (not fld.control_settings.has_member("box")) {
						json fbox = jp.create_object();
						corona::get_json(fbox, fld.field_box);
						fld.control_settings.put_member("box", fbox);
					}
				}
				index_lists++;
			}
		}

	public:
		std::string				name;
		std::vector<form_field> fields;
		json					data;
		form_field				defaults;

		form_list_changed_handler	on_list_changed;
		form_item_changed_handler	on_item_changed;
		form_command_handler		on_command;
		form_mouse_click_handler	on_mouse_click;
		form_mouse_move_handler		on_mouse_move;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("name", name);
			_dest.put_member("data", data);

			json_parser jp;
			json jfields = jp.create_array();
			json jdefault = jp.create_object();
			defaults.get_json(jdefault);

			for (auto fld : fields) {
				json jfield = jp.create_object();
				fld.get_json(jfield);
				jfields.append_element(jfield);
			}
			_dest.put_member("fields", jfields);
			_dest.put_member("defaults", jdefault);
		}

		virtual void put_json(json& _src)
		{
			name = _src["name"];
			data = _src["data"];
			json jitembox = _src["defaults"];
			defaults.put_json(jitembox);
			fields.clear();
			json field_array = _src["fields"];
			if (field_array.array()) 
			{
				for (auto fld : field_array) 
				{
					form_field ff;
					ff.put_json(fld);
					
					fields.push_back(ff);
				}
			}
			apply_defaults();
		}

	};

	class form_control : public column_layout
	{
		form_model ids;
		presentation_base* current_presentation;
		page_base* current_page;
		json current_object;

	public:

		form_control()
		{
			;
		}

		form_control(const form_control& _src) = default;

		form_control(container_control_base* _parent, int _id) : column_layout(_parent, _id)
		{
			current_presentation = nullptr;
			current_page = nullptr;
		}

		virtual ~form_control()
		{
			;
		}

		virtual std::string form_name() { return ids.name;  }

		std::function<void(int _control_id, form_control* _fv)> on_changed;

		virtual json get_data()
		{
			json_parser jp;

			if (current_object.empty()) {
				current_object = jp.create_object();
			}

			for (auto& ctrl : ids.fields)
			{
				if (ctrl.json_field_name.empty()) {
					continue;
				}
				control_base *cb = find(ctrl.field_id);
				if (cb) {
					json field_data = cb->get_data();
					current_object.copy_member(ctrl.json_field_name, field_data);
				}
			}

			return current_object;
		}

		virtual json set_data(json _data)
		{
			json empty;

			current_object = _data;

			for (auto& ctrl : ids.fields)
			{
				control_base* cb = find(ctrl.field_id);
				if (cb and cb->json_field_name.size()) {
					if (_data.has_member(cb->json_field_name)) {
						cb->set_data(_data);
					}
				}
			}

			return empty;
		}

		void set_model(form_model _ids)
		{
			children.clear();

			control_builder cb;

			if (_ids.name.empty()) {
				system_monitoring_interface::global_mon->log_warning("Forms without names basically don't work");
/*				json_parser jp;
				json diag = jp.create_object();
				_ids.get_json(diag);
				system_monitoring_interface::global_mon->log_json<json>(diag);
	*/		}

			ids = _ids;
			auto pform = this;

			for (auto &ctrl : ids.fields) 
			{
				ctrl.form_name = ids.name;
				cb.form_fieldy(id_counter::next(), ctrl);
			}

			cb.apply_controls(this);
			calculate_margins();
			arrange(bounds);
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			json props = jp.create_object();

			column_layout::get_json(_dest);
			ids.get_json(props);
			_dest.put_member("model", props);
		}

		virtual void put_json(json& _ids)
		{
			form_model fm;
			column_layout::put_json(_ids);
			json props = _ids["model"];
			fm.put_json(props);
			set_model(fm);
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

		virtual std::shared_ptr<control_base> clone()
		{
			auto tv = std::make_shared<form_control>(*this);
			return tv;
		}

	};


	class tab_pane
	{
	public:
		int id;
		std::string name;
		form_model form;
		std::vector<json> create_objects;
		std::vector<json> create_classes;
	};

	class tab_pane_instance
	{
	public:
		tab_pane					  pane;
		std::shared_ptr<form_control> tab_form;
		json						  tab_form_data;
	};

	class tab_view_control : public windows_control
	{
		std::vector<tab_pane_instance> tab_panes;
		std::shared_ptr<frame_layout> content_frame;
		presentation_base* current_presentation;
		page_base* current_page;
		int active_id;

		void init()
		{
			children.clear();

			control_builder builder;

			on_create = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
				{
					_context->setBrush(border_brush.get(), &inner_bounds);
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

				if (not i and active_id <= 0)
				{
					active_id = dat.pane.id;
				}

				tab_row.tab_button(dat.pane.id, [this, dat](tab_button_control& _tb) {
					_tb.text = dat.pane.id;
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
		std::function<void(std::shared_ptr<direct2dContext>& _context, tab_view_control*)> on_draw;
		std::function<void(std::shared_ptr<direct2dContext>& _context, tab_view_control*)> on_create;

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
			set_bounds(_bounds, true);

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
			bool is_message = is_focused and 
				(_key == VK_TAB or 
					_key == VK_LEFT or _key == VK_RIGHT or 
					_key == VK_UP or _key == VK_DOWN or 
					_key == VK_PRIOR or _key == VK_NEXT or 
					_key == VK_HOME or _key == VK_END ||
					_key == VK_DELETE or _key == VK_INSERT ||
					_key == VK_RETURN or _key == VK_DELETE);
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
			tab_panes.clear();
			for (auto tp : _new_panes) {
				tab_pane_instance tpi;
				tpi.pane = tp;
				tab_panes.push_back(tpi);
			}
			init();
			tab_selected(active_id);
		}

		void tab_selected(std::vector<tab_pane_instance>::iterator tbi)
		{
			if (tbi != tab_panes.end())
			{
				active_id = tbi->pane.id;

				contents_generator<tab_pane_instance*> cg;

				// set contents will clone this for us.
				cg.data = &*tbi;
				cg.generator = [this](tab_pane_instance* _tp, control_base* _args)
					{
						_args->children.clear();
						if (not _tp->tab_form) {
							_tp->tab_form = std::make_shared<form_control>(this, active_id);
							_tp->tab_form->set_model(_tp->pane.form);
						}
						_args->children.push_back(_tp->tab_form);
					};
				content_frame->set_contents(cg);

				if (current_presentation and current_page) {
					content_frame->on_subscribe(current_presentation, current_page);
				}
			}
			else if (tab_panes.size()>0) {
				tab_selected(tab_panes.begin());
			}
		}

		void tab_selected(int _active_id)
		{
			auto tbi = std::find_if(tab_panes.begin(), tab_panes.end(), [this](tab_pane_instance& _tb) {
				return _tb.pane.id == this->active_id;
				});
			tab_selected(tbi);
		}

		void tab_selected(tab_button_control& _tab)
		{
			auto tbi = std::find_if(tab_panes.begin(), tab_panes.end(), [this](tab_pane_instance& _tb) {
				return _tb.pane.id == this->active_id;
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



		virtual void create(std::shared_ptr<direct2dContext>& _context, std::weak_ptr<applicationBase> _host) override
		{
			host = _host;
			if (auto phost = _host.lock()) {
				if (inner_bounds.x < 0 or inner_bounds.y < 0 or inner_bounds.w < 0 or inner_bounds.h < 0) {
					throw std::logic_error("inner bounds not initialized");
				}
				window = phost->createDirect2Window(id, inner_bounds);
			}
			if (on_create) {
				on_create(_context, this);
			}
			for (auto child : children) {
				child->create(_context, _host);
			}
			windows_control::create(_context, _host);
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

		virtual void draw_impl(std::shared_ptr<direct2dContext>& _context)
		{
			bool adapter_blown_away = false;

			auto& context = _context;
			const char* border_name = nullptr;
			const char* background_name = nullptr;

			if (border_brush->get_name())
			{
				_context->setBrush(border_brush.get(), &inner_bounds);
				auto dc = _context->getDeviceContext();
				border_name = border_brush->get_name();
			}

			if (background_brush->get_name())
			{
				_context->setBrush(background_brush.get(), &inner_bounds);
				auto dc = _context->getDeviceContext();
				background_name = background_brush->get_name();
			}

			if (background_name) {
				rectangle r = inner_bounds;
				_context->drawRectangle(&r, "", 0.0, background_name);
			}

			if (on_draw)
			{
				on_draw(_context, this);
			} 

			if (is_focused and border_name) 
			{
				rectangle r = inner_bounds;
				_context->drawRectangle(&r, border_name, 4, "");
			}

			for (auto& child : children) {
				try {
					child->draw(_context);
				}
				catch (std::exception exc)
				{
					system_monitoring_interface::global_mon->log_exception(exc);
				}
			}
		}

		virtual void draw(std::shared_ptr<direct2dContext>& _dest)
		{

		}

		virtual void render(std::shared_ptr<direct2dContext>& _dest)
		{
			draw_impl(_dest);
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
			return *this;
		}

		tab_view_control& set_background_color(std::string _color)
		{
			background_brush = std::make_shared<generalBrushRequest>();
			background_brush->setColor(_color);
			background_brush->set_name(typeid(*this).name());
			return *this;
		}

		tab_view_control& set_border_color(solidBrushRequest _brushFill)
		{
			border_brush = std::make_shared<generalBrushRequest>(_brushFill);
			border_brush->set_name(typeid(*this).name());
			return *this;
		}

		tab_view_control& set_border_color(std::string _color)
		{
			border_brush = std::make_shared<generalBrushRequest>();
			border_brush->setColor(_color);
			border_brush->set_name( typeid(*this).name() );
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
			control_builder cb;

			auto main_row = cb.row_begin(id_counter::next(), [](row_layout& rl) {
				rl.set_size(1.0_container, 100.0_px);
				rl.set_content_align(visual_alignment::align_far);
				rl.set_content_cross_align(visual_alignment::align_center);
				rl.set_item_margin(10.0_px);
				rl.set_style(presentation_style_factory::get_current()->get_style()->CaptionStyle);
				});

			if (image_file.size())
			{
				auto image_column = main_row.column_begin(id_counter::next(), [](column_layout& cl) {
					cl.set_size(75.0_px, 1.0_container);
					cl.set_content_align(visual_alignment::align_center);
					});

				image_column.image(image_control_id, image_file, [](image_control& _settings) {
					_settings.box.x = 10.0_px;
					_settings.set_size(1.0_container, 1.0_aspect);
					});
			}

			auto title_column = main_row.column_begin(id_counter::next(), [this](column_layout& cl) {
				cl.set_size(1.0_remaining, 1.0_container);
				cl.set_content_align(visual_alignment::align_center);
				cl.set_item_margin(10.0_px);
				cl.set_origin(title_start, 12.0_px);
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
				.menu_button(menu_button_id, [](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
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
		int image_control_id;
		std::string image_file;
		std::string corporate_name;
		std::string title_name;
		std::string subtitle_name;
		int title_id;
		int subtitle_id;
		measure title_start;

		caption_bar_control()
		{
			menu_button_id = 0;
			image_control_id = 0;
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

		virtual void get_json(json& _dest)
		{
			container_control::get_json(_dest);

			_dest.put_member("image_file", image_file);
			_dest.put_member("corporate_name", corporate_name);
			_dest.put_member("title_name", title_name);
			_dest.put_member("subtitle_name", subtitle_name);
			
			json_parser jp;
			json jtitle_start = jp.create_object();
			corona::get_json(jtitle_start, title_start);
			_dest.put_member("title_start", jtitle_start);
		}

		virtual void put_json(json& _src)
		{
			container_control::put_json(_src);
			image_file = _src["image_file"];
			corporate_name = _src["corporate_name"];
			title_name = _src["title_name"];
			subtitle_name = _src["subtitle_name"];

			json jtitle_start = _src["title_start"];
			corona::put_json(title_start, jtitle_start);
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

	control_builder& control_builder::image(int _id, std::function<void(image_control&)> _settings)
	{
		auto tc = create<image_control>(_id);
		apply_item_sizes(tc);
		if (_settings) {
			_settings(*tc);
		}
		return *this;
	}

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

	control_builder& control_builder::form_fieldy(int _id, json _field_def)
	{
		form_field fld;
		fld.put_json(_field_def);
		auto tc = create<form_field_control>(_id);
		apply_item_sizes(tc);
		tc->set_field(fld, nullptr);
		return *this;
	}

	control_builder& control_builder::form_fieldy(int _id, form_field _field_def)
	{
		auto tc = create<form_field_control>(_id);
		apply_item_sizes(tc);
		tc->set_field(_field_def, nullptr);
		return *this;
	}

	std::shared_ptr<control_base> control_builder::from_json(json _control_properties, layout_rect _default)
	{
		json_parser jp;

		json control_properties = _control_properties;

		std::string class_name = control_properties["class_name"];
		std::string field_name = control_properties["name"];
		json control_data = control_properties["data"];

		int id = (int)_control_properties.get_member("id");

		int field_id = id ? id : id_counter::next();

		std::string default_text = "";
		call_status default_status;

		if (class_name == "title")
		{
			title(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "subtitle")
		{
			subtitle(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "authorscredit")
		{
			authorscredit(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "chaptertitle")
		{
			chaptertitle(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "chaptersubtitle")
		{
			chaptersubtitle(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "paragraph")
		{
			paragraph(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "code")
		{
			code(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "label")
		{
			label(default_text, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "error")
		{
			error(default_status, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "status")
		{
			status(default_status, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "success")
		{
			success(default_status, [&control_properties, control_data](auto& _ctrl) ->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				}, field_id);
		}
		else if (class_name == "row")
		{
			row_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "column")
		{
			column_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "absolute")
		{
			absolute_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "row_view")
		{
			row_view_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid_view")
		{
			grid_view_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "absolute_view")
		{
			absolute_view_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "frame")
		{
			frame_begin(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "listbox")
		{
			listbox(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "combobox")
		{
			combobox(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "password")
		{
			password(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "edit")
		{
			edit(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "listview")
		{
			listview(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "treeview")
		{
			treeview(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "header")
		{
			header(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "toolbar")
		{
			toolbar(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "statusbar")
		{
			statusbar_field(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "hotkey")
		{
			hotkey(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "animate")
		{
			animate(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "edit")
		{
			edit(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
				}
		else if (class_name == "richedit")
		{
			richedit(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "draglistbox")
		{
			draglistbox(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "comboboxex")
		{
			comboboxex(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "datetimepicker")
		{
			datetimepicker(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "monthcalendar")
		{
			monthcalendar(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "radiobutton_list")
		{
			radiobutton_list(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "checkbox_list")
		{
			checkbox_list(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "minimize_button")
		{
			minimize_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "maximize_button")
		{
			maximize_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "close_button")
		{
			close_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "menu_button")
		{
			menu_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "corona_button")
		{
			corona_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "link_button")
		{
			link_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
				}
		else if (class_name == "push_button")
		{
			push_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "radio_button")
		{
			radio_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "checkbox")
		{
			checkbox(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "camera")
		{
			camera(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid")
		{
			grid(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "chart")
		{
			chart(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "slide")
		{
			slide(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "grid")
		{
			grid(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "tab_button")
		{
			tab_button(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "tab_view")
		{
			tab_view(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "search_view")
		{
			search_view(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "caption_bar")
		{
			caption_bar(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "status_bar")
		{
			status_bar(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "form")
		{
			form(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "image")
		{
			image(field_id, [&control_properties, control_data](auto& _ctrl)->void {
				_ctrl.put_json(control_properties);
				_ctrl.set_data(control_data);
				});
		}
		else if (class_name == "form_field")
		{
			form_fieldy(field_id, control_properties);
		}
		else 
		{
			std::string msg;
			msg = std::format("class_name '{0}' is invalid", class_name);
			system_monitoring_interface::global_mon->log_warning(msg);
			std::cout << "Currently the following control classes are supported.  Set class_name to one of these." << std::endl;
			std::cout << "Special types" << std::endl;
			std::cout << "image, camera, " << std::endl;
			std::cout << "Text Box types" << std::endl;
			std::cout << "title, subtitle, chaptertitle, chaptersubtitle, paragraph, " << std::endl;
			std::cout << "code, label, error, status, success" << std::endl;
			std::cout << "Layout types" << class_name << std::endl;
			std::cout << "row, column, absolute, row_view, absolute_view, grid_view, grid, slide, frame, tab_button, tab_view, search_view" << class_name << std::endl;
			std::cout << "Windows SDK types" << class_name << std::endl;
			std::cout << "combobox, listbox, edit, password, listview, treeview, header, toolbar, statusbar, hotkey, animate, richedit, draglistbox, comboboxex, datetimepicker, monthcalendar, radiobutton_list, checkbox_list" << std::endl;
			std::cout << "System Button types" << class_name << std::endl;
			std::cout << "caption_bar, minimize_button, mnaximize_button, close_button, menu_button, corona_button" << std::endl;
			std::cout << "Forms types" << class_name << std::endl;
			std::cout << "form, form_field" << std::endl;
			std::cout << "" << class_name << std::endl;
		}

		std::shared_ptr<control_base> ret = get<control_base>(field_id);

		if (ret) {
			json children = _control_properties.get_member("children");
			if (children.array()) {

				control_builder cb;
				for (auto child : children)
				{
					auto new_child = cb.from_json(child);
					if (new_child) {
						ret->children.push_back(new_child);
					}
				}
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
			auto tv = std::make_shared<radiobutton_list_control>(*this);
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
			if (not json_field_name.empty()) {
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
				if (field_items.array()) {
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
			auto tv = std::make_shared<checkbox_list_control>(*this);
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
			if (not json_field_name.empty()) {
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
				if (field_items.array()) {
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

	void form_field_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{

		_page->on_list_changed(field_id, [this](list_changed_event lce)
			{
				form_control* form = find_my_form();
				if (form) {
					form->list_changed(lce, this);
				}
			});

		_page->on_item_changed(field_id, [this](item_changed_event lce)
			{
				form_control* form = find_my_form();
				if (form) {
					form->item_changed(lce, this);
				}
			});

		_page->on_command(field_id, [this](command_event lce)
			{
				form_control* form = find_my_form();
				if (form) {
					form->field_command(lce, this);
				}
			});

		auto fld = find_by_id<control_base>(field_id);
		_page->on_mouse_click(fld.get(), [this](mouse_click_event lce)
			{
				form_control* form = find_my_form();
				if (form) {
					form->field_mouse_click(lce, this);
				}
			});

		_page->on_mouse_move(fld.get(), [this](mouse_move_event lce)
			{
				form_control* form = find_my_form();
				if (form) {
					form->field_mouse_move(lce, this);
				}
			});

	}

	void corona_button_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(100.0_px, 30.0_px);
	}

	void frame_layout::set_contents(presentation_base* _presentation, page_base* _parent_page, page_base* _contents)
	{
		control_builder cb;

		for (auto child : children) {
			child->on_unsubscribe(_presentation, _parent_page);
		}
		children.clear();

		cb.column_begin(id_counter::next(), [_contents](column_layout& _settings) {
			_settings.set_size(1.0_container, 1.0_container);
			for (auto srcchild : _contents->root->children)
			{
				auto new_child = srcchild->clone();
				_settings.children.push_back(new_child);
			}
		});

		cb.apply_controls(this);

		for (auto child : children) {
			child->on_subscribe(_presentation, _parent_page);
		}

		arrange(bounds);
	}

	json corona_set_property_command::execute(json context, comm_bus_app_interface* bus)
	{
		json obj;
		control_base* cb = {};
		std::string property_value = context["property_value"];
		if (not control_name.empty()) {
			cb = bus->find_control(control_name);
			if (property_value.empty()) {
				property_value = value;
			}
			if (this->property_name == "text") {
				text_control_base* tcb = dynamic_cast<text_control_base*>(cb);
				if (tcb != nullptr) {
					tcb->set_text(property_value);
				}
				else {
					text_display_control* tdc = dynamic_cast<text_display_control*>(cb);
					if (tdc != nullptr) {
						tdc->set_text(property_value);
					}
				}
			}
		}
		return obj;
	}


}

#endif
