
#include "corona.h"

#define TRACE_LAYOUT 01

namespace corona
{
	using namespace win32;

	namespace database
	{
		presentation_style_factory styles;

		int id_counter::status_text_title_id = 100000;
		int id_counter::status_text_subtitle_id = 100001;
		int id_counter::status_bar_id = 100002;

		int id_counter::id = 100010;
		int id_counter::next()
		{
			id++;
			return id;
		}

		menu_item_navigate::menu_item_navigate()
		{
			control_id = {};
			target_page = {};
			handler = {};
		}

		menu_item_navigate::menu_item_navigate(int _source_control_id, std::string _target_page)
		{
			control_id = _source_control_id;
			target_page = _target_page;
			handler = [this](presentation* _presentation, std::weak_ptr<page> _page)
			{
				if (auto ppresent = _presentation.lock())
				{
					ppresent->select_page(target_page);
				}
			};
		}

		menu_item_navigate::menu_item_navigate(const menu_item_navigate& _src)
		{
			target_page = _src.target_page;
			handler = _src.handler;
		}

		menu_item_navigate menu_item_navigate::operator =(const menu_item_navigate& _src)
		{
			target_page = _src.target_page;
			handler = _src.handler;
			return *this;
		}

		menu_item_navigate::menu_item_navigate(menu_item_navigate&& _src)
		{
			target_page = std::move(_src.target_page);
			handler = std::move(_src.handler);
		}

		menu_item_navigate& menu_item_navigate::operator =(menu_item_navigate&& _src)
		{
			target_page = std::move(_src.target_page);
			handler = std::move(_src.handler);
			return *this;
		}


		menu_item_navigate::operator menu_click_handler ()
		{
			return handler;
		}

		menu_item::menu_item() :
			id(0),
			name("test"),
			is_separator(false),
			created_menu(nullptr)
		{
			;
		}

		menu_item::menu_item(int _id, std::string _name, std::function<void(menu_item& _item)> _settings) :
			id(_id),
			name(_name),
			is_separator(false),
			created_menu(nullptr)
		{
			;
		}

		menu_item::~menu_item()
		{
			if (created_menu) {
				::DestroyMenu(created_menu);
				created_menu = nullptr;
			}
		}

		menu_item& menu_item::item(int _id, std::string _name, std::function<void(menu_item& _item)> _settings)
		{
			auto mi = std::make_shared<menu_item>(_id, _name, _settings);
			mi->parent = this;
			if (_settings) {
				_settings(*mi.get());
			}
			children.push_back(mi);
			return *this;
		}

		menu_item& menu_item::destination(int _id, std::string _name, std::string _target_page, std::function<void(menu_item& _item)> _settings)
		{
			auto mi = std::make_shared<menu_item>(_id, _name, _settings);
			mi->parent = this;
			mi->handler = menu_item_navigate(_id, _target_page);
			if (_settings) {
				_settings(*mi.get());
			}
			children.push_back(mi);
			return *this;
		}

		menu_item& menu_item::separator(int _id, std::function<void(menu_item& _item)> _settings)
		{
			auto mi = std::make_shared<menu_item>(_id, "", _settings);
			mi->parent = this;
			if (_settings) {
				_settings(*mi.get());
			}
			mi->is_separator = true;
			children.push_back(mi);
			return *this;
		}

		menu_item& menu_item::begin_submenu(int _id, std::string _name, std::function<void(menu_item& _item)> _settings)
		{
			auto mi = std::make_shared<menu_item>(_id, _name, _settings);
			mi->parent = this;
			if (_settings) {
				_settings(*mi.get());
			}
			mi->is_separator = true;
			children.push_back(mi);
			auto ptr = mi.get();
			return *ptr;
		}

		menu_item& menu_item::end()
		{
			return *parent;
		}

		void menu_item::subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{
			if (auto ppage = _page.lock()) {
				ppage->on_command(id, [this, _present, _page](command_event evt) {
					if (auto ppresent = _present.lock()) {
						if (handler) {
							handler(_present, _page);
						}
					}
				});

				for (auto child : children)
				{
					child->subscribe(_present, _page);
				}
			}
		}

		void control_base::push(int _destination_control_id, bool _push_left, bool _push_top, bool _push_right, bool _push_bottom)
		{

			control_push_request cpr = {};

			cpr.dest_control_id = _destination_control_id;
			if (_push_left) {
				cpr.properties_to_push |= cp_left_bounds;
			}
			if (_push_top) {
				cpr.properties_to_push |= cp_top_bounds;
			}
			if (_push_right) {
				cpr.properties_to_push |= cp_right_bounds;
			}
			if (_push_bottom) {
				cpr.properties_to_push |= cp_bottom_bounds;
			}

			push_requests.push_back(cpr);
		}

		control_base* control_base::find(point p)
		{
			control_base *result = nullptr;

			if (rectangle_math::contains(inner_bounds, p.x, p.y)) 
			{
				result = this;
				for (auto child : children)
				{
					auto temp = child->find(p);
					if (temp) {
						return temp;
					}
				}
			}

			return result;
		}

		control_base* control_base::find(int _id)
		{
			control_base* root = this;
			while (root->parent) {
				root = root->parent;
			}
			// just to make sure you getting the right root.
			control_base* result = control_base::get(root, _id);
			return result;
		}

		control_base* control_base::get(control_base* _root, int _id)
		{
			control_base* result = _root->find_if([_id](control_base* c) { return c->id == _id; });
			return result;
		}

		control_base* control_base::find_if(std::function<bool(control_base* _root)> _item)
		{
			if (_item(this))
				return this;
			for (auto child : children) {
				auto result = child->find_if(_item);
				if (result)
					return result;
			}
			return nullptr;
		}

		void control_base::foreach(std::function<void(control_base* _root)> _item)
		{
			_item(this);
			for (auto child : children) {
				child->foreach(_item);
			}
		}

		void control_base::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{
			for (auto child : children) {
				child->create(_host);
			}
		}

		void control_base::destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void control_base::draw()
		{
			for (auto child : children) {
				child->draw();
			}
		}

		void control_base::render(CComPtr<ID2D1DeviceContext>& _dest)
		{
			for (auto child : children) {
				child->render(_dest);
			}
		}

		void control_base::apply(control_base& _ref)
		{

		}

		bool control_base::set_mouse(point _position, 
			bool *_left_down,
			bool *_right_down,
			std::function<void(control_base* _item)> _left_click,
			std::function<void(control_base* _item)> _right_click
		)
		{
			mouse_relative_position = {};
			mouse_over = rectangle_math::contains(bounds, _position.x, _position.y);

			if (mouse_over) 
			{
				mouse_relative_position = _position;
				mouse_relative_position.x -= bounds.x;
				mouse_relative_position.y -= bounds.y;
				if (_left_down) 
				{
					mouse_left_down = *_left_down;
				}
				if (_right_down) 
				{
					mouse_right_down = *_right_down;
				}
			}
			else 
			{
				mouse_left_down = false;
				mouse_right_down = false;
			}

			if (mouse_left_down.changed_to(false) && _left_click != nullptr) {
				_left_click(this);
			}

			if (mouse_right_down.changed_to(false) && _right_click != nullptr) {
				_right_click(this);
			}

			for (auto child : children)
			{
				child->set_mouse(_position, _left_down, _right_down, _left_click, _right_click);
			}
			return mouse_over;
		}

		row_layout& container_control::row_begin(int _id, std::function<void(row_layout&)> _settings)
		{
			auto& tc = create<row_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return tc;
		}

		column_layout& container_control::column_begin(int _id, std::function<void(column_layout&)> _settings)
		{
			auto& tc = create<column_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return tc;
		}

		absolute_layout& container_control::absolute_begin(int _id, std::function<void(absolute_layout&)> _settings)
		{
			auto& tc = create<absolute_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return tc;
		}

		frame_layout& container_control::frame_begin(int _id, std::function<void(frame_layout&)> _settings)
		{
			auto& tc = create<frame_layout>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return tc;
		}


		container_control& container_control::caption_bar(
				presentation_style& st,
				int	title_bar_id,
				int menu_button_id,
				menu_item &menu,
				int image_control_id, 
				std::string image_file,
				std::string corporate_name,
				int id_title_column_id,
				std::string title_name,
				std::string subtitle_name		
		)
		{
			auto return_control = 
			row_begin(title_bar_id, [st](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_background_color(st.HeaderBackgroundColor);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				rl.set_nchittest(HTCAPTION);
				})
				.row_begin([](row_layout& rl) {
					rl.set_size(.24_container, 1.0_container);
					})
					.row_begin([](row_layout& cl) {
						cl.set_content_align(visual_alignment::align_near);
						cl.set_content_cross_align(visual_alignment::align_near);
						cl.set_size( 120.0_px, 1.0_container);
						cl.set_item_margin(5.0_px);
						})
						.menu_button(menu_button_id,[menu](auto& _ctrl) { 
							_ctrl.set_size(50.0_px, 50.0_px); 
							_ctrl.set_margin(5.0_px);
							_ctrl.menu = menu;
							})
						.image(image_control_id, image_file, [](image_control& control) { control.set_size(50.0_px, 50.0_px); })
					.end()
					.column_begin([](column_layout& cl) {
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
				.row_begin([](row_layout& rl) {
					rl.set_size(.95_remaining, 1.0_container);
					rl.set_item_margin(5.0_px );
					rl.set_content_cross_align(visual_alignment::align_center);
					rl.set_content_align(visual_alignment::align_far);
						})
					.minimize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
					.maximize_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
					.close_button([](auto& _ctrl) { _ctrl.set_size(50.0_px, 50.0_px); })
				.end()
			.end();
			return *this;
		}

		container_control& container_control::form_single_column(int _align_id,
			std::string _form_name,
			std::function<void (container_control& _settings)> _add_controls
		)
		{
			auto return_control = row_begin([](row_layout& r)
				{
					r.set_size(1.0_container, 1.0_container);
					r.set_content_align(visual_alignment::align_center);
					r.set_background_color(styles.get_style().FormBackgroundColor);
				})
				.column_begin([_add_controls, _align_id](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.50_container, 1.0_container);
						r.push(_align_id, true, false, false, false);
						_add_controls(r);
					})
				.end()
			.end();

			return *this;
		}

		void control_base::on_subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{
			for(auto child : children) {
				child->on_subscribe(_presentation, _page);
			}
		}

		container_control& container_control::form_double_column(int _align_id,
			std::string _form_name,
			std::function<void(container_control& _settings)> _add_controls1,
			std::function<void(container_control& _settings)> _add_controls2
		)
		{
			auto return_control = row_begin([](row_layout& r)
				{
					r.set_size(1.0_container, 1.0_container);
					r.set_content_align(visual_alignment::align_center);
				})
				.column_begin([_add_controls1, _align_id](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.30_container, 1.0_container);
						r.push(_align_id, true, false, false, false);
						r.set_background_color(styles.get_style().FormBackgroundColor);
						_add_controls1(r);
					})
				.end()
				.column_begin([_add_controls2, _align_id](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.30_container, 1.0_container);
						r.push(_align_id, true, false, false, false);
						r.set_background_color(styles.get_style().FormBackgroundColor);
						_add_controls2(r);
					})
				.end()
			.end();

			return *this;
		}

		container_control& container_control::status_bar(presentation_style& st)
		{
			auto return_control = row_begin(id_counter::status_bar_id, [st](row_layout& rl) {
				rl.set_size(1.0_container, 80.0_px);
				rl.set_background_color(st.HeaderBackgroundColor);
				rl.set_content_align(visual_alignment::align_near);
				rl.set_content_cross_align(visual_alignment::align_near);
				rl.set_item_margin(10.0_px);
				})
					.column_begin([](column_layout& cl) {
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
			return *this;
		}

		container_control& container_control::end()
		{
			if (parent) {
				auto& temp = *parent;
				//		auto string_name = typeid(temp).name();
		//				std::string indent(debug_indent, ' ');
			//			std::cout << indent << " " << typeid(*this).name() << " ->navigate " << string_name << std::endl;
				return temp;
			}
		}

		container_control& container_control::set_origin(measure _x, measure _y)
		{
			box.x = _x;
			box.y = _y;
			return *this;
		}

		container_control& container_control::set_size(measure _width, measure _height)
		{
			box.width = _width;
			box.height = _height;
			return *this;
		}

		container_control& container_control::set_background_color(solidBrushRequest _brushFill)
		{
			background_brush = _brushFill;
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& container_control::set_background_color(std::string _color)
		{
			background_brush.brushColor = toColor(_color.c_str());
			background_brush.name = typeid(*this).name();
			background_brush.active = true;
			return *this;
		}

		container_control& container_control::set_position(layout_rect _new_layout)
		{
			box = _new_layout;
			return *this;
		}

		container_control& container_control::set_margin(measure _item_space)
		{
			margin = _item_space;
			return *this;
		}

		container_control& container_control::image(int id, int _control_id, std::function<void(image_control&)> _settings)
		{
			auto& tc = create<image_control>(id);
			apply(tc);
			tc.load_from_control(_control_id);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::image(int id, std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto& tc = create<image_control>(id);
			apply(tc);
			tc.load_from_file(_filename);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::image(std::string _filename, std::function<void(image_control&)> _settings)
		{
			auto& tc = create<image_control>(id_counter::next());
			apply(tc);
			tc.load_from_file(_filename);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::title(std::string text, std::function<void(title_control&)> _settings, int _id)
		{
			auto &tc = create<title_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::subtitle(std::string text, std::function<void(subtitle_control&)> _settings, int _id)
		{
			auto &tc = create<subtitle_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::chaptertitle(std::string text, std::function<void(chaptertitle_control&)> _settings, int _id)
		{
			auto &tc = create<chaptertitle_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::chaptersubtitle(std::string text, std::function<void(chaptersubtitle_control&)> _settings, int _id)
		{
			auto &tc = create<chaptersubtitle_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::paragraph(std::string text, std::function<void(paragraph_control&)> _settings, int _id)
		{
			auto &tc = create<paragraph_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::code(std::string text, std::function<void(code_control&)> _settings, int _id)
		{
			auto &tc = create<code_control>(_id);
			apply(tc);
			tc.text = text;
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::label(std::string _text, std::function<void(label_control&)> _settings, int _id)
		{
			auto& tc = create<label_control>(_id);
			apply(tc);
			tc.set_text(_text);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::push_button(int _id, std::string text, std::function<void(pushbutton_control&)> _settings)
		{
			auto& tc = create<pushbutton_control>(_id);
			apply(tc);
			tc.set_text(text);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::press_button(int _id, std::string text, std::function<void(pressbutton_control&)> _settings)
		{
			auto& tc = create<pressbutton_control>(_id);
			apply(tc);
			tc.set_text(text);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::radio_button(int _id, std::string text, std::function<void(radiobutton_control&)> _settings)
		{
			auto& tc = create<radiobutton_control>(_id);
			apply(tc);
			tc.set_text(text);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::checkbox(int _id, std::string text, std::function<void(checkbox_control&)> _settings)
		{
			auto& tc = create<checkbox_control>(_id);
			apply(tc);
			tc.set_text(text);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::listbox(int _id, std::function<void(listbox_control&)> _settings)
		{
			auto& tc = create<listbox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::combobox(int _id, std::function<void(combobox_control&)> _settings)
		{
			auto& tc = create<combobox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::edit(int _id, std::function<void(edit_control&)> _settings)
		{
			auto& tc = create<edit_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::scrollbar(int _id, std::function<void(scrollbar_control&)> _settings)
		{
			auto& tc = create<scrollbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::listview(int _id, std::function<void(listview_control&)> _settings)
		{
			auto& tc = create<listview_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::treeview(int _id, std::function<void(treeview_control&)> _settings)
		{
			auto& tc = create<treeview_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::header(int _id, std::function<void(header_control&)> _settings)
		{
			auto& tc = create<header_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::toolbar(int _id, std::function<void(toolbar_control&)> _settings)
		{
			auto& tc = create<toolbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::statusbar(int _id, std::function<void(statusbar_control&)> _settings)
		{
			auto& tc = create<statusbar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::hotkey(int _id, std::function<void(hotkey_control&)> _settings)
		{
			auto& tc = create<hotkey_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::animate(int _id, std::function<void(animate_control&)> _settings)
		{
			auto& tc = create<animate_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::richedit(int _id, std::function<void(richedit_control&)> _settings)
		{
			auto& tc = create<richedit_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::draglistbox(int _id, std::function<void(draglistbox_control&)> _settings)
		{
			auto& tc = create<draglistbox_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::rebar(int _id, std::function<void(rebar_control&)> _settings)
		{
			auto& tc = create<rebar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::comboboxex(int _id, std::function<void(comboboxex_control&)> _settings)
		{
			auto& tc = create<comboboxex_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::datetimepicker(int _id, std::function<void(datetimepicker_control&)> _settings)
		{
			auto& tc = create<datetimepicker_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::monthcalendar(int _id, std::function<void(monthcalendar_control&)> _settings)
		{
			auto& tc = create<monthcalendar_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::minimize_button(std::function<void(minimize_button_control&)> _settings)
		{
			auto& tc = create<minimize_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
			;
		}

		container_control& container_control::maximize_button(std::function<void(maximize_button_control&)> _settings)
		{
			auto& tc = create<maximize_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::close_button(std::function<void(close_button_control&)> _settings)
		{
			auto& tc = create<close_button_control>(id_counter::next());
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		container_control& container_control::menu_button(int _id, std::function<void(menu_button_control&)> _settings)
		{
			auto& tc = create<menu_button_control>(_id);
			apply(tc);
			if (_settings) {
				_settings(tc);
			}
			return *this;
		}

		double control_base::to_pixels(measure length)
		{
			double sz = 0.0;

			control_base& pi = *this;

			switch (length.units) {
			case measure_units::pixels:
				sz = length.amount;
				break;
			case measure_units::percent_container:
				sz = length.amount * bounds.w;
				break;
			case measure_units::percent_remaining:
				sz = length.amount * bounds.w;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
				double font_height = 12.0;
				sz = font_height * pi.box.width.amount;
				if (pi.box.width.units == measure_units::font_golden_ratio)
				{
					sz /= 1.618;
				}
				break;
			}
			return sz;
		}

		point control_base::get_size(rectangle _ctx, point _remaining)
		{
			point sz;

			control_base& pi = *this;
			auto mm = to_pixels(margin);

			if (pi.box.width.units == measure_units::pixels)
			{
				sz.x = pi.box.width.amount;
				sz.x += mm * 2.0;
			}
			else if (box.width.units == measure_units::percent_remaining)
			{
				sz.x = box.width.amount * _remaining.x;
			}
			else if (pi.box.width.units == measure_units::percent_container)
			{
				sz.x = pi.box.width.amount * _ctx.w;
			}
			else if (pi.box.width.units == measure_units::font || pi.box.width.units == measure_units::font_golden_ratio)
			{
				double font_height = get_font_size();
				sz.x = font_height * pi.box.width.amount;
				if (pi.box.width.units == measure_units::font_golden_ratio)
				{
					sz.x /= 1.618;
				}
				sz.x += mm * 2.0;
			}

			if (pi.box.height.units == measure_units::pixels)
			{
				sz.y = pi.box.height.amount;
				sz.y += mm * 2.0;
			}
			else if (box.height.units == measure_units::percent_remaining)
			{
				sz.y = pi.box.height.amount * _remaining.y;
			}
			else if (pi.box.height.units == measure_units::percent_container)
			{
				sz.y = pi.box.height.amount * _ctx.h;
			}
			else if (pi.box.height.units == measure_units::font || pi.box.height.units == measure_units::font_golden_ratio)
			{
				double font_height = get_font_size();
				sz.y = font_height * pi.box.height.amount;
				if (pi.box.height.units == measure_units::font_golden_ratio)
				{
					sz.y *= 1.618;
				}
				sz.y += mm * 2.0;
			}

			if (box.width.units == measure_units::percent_aspect)
			{
				sz.x = box.width.amount * bounds.h;
			}

			if (box.height.units == measure_units::percent_aspect)
			{
				sz.y = box.height.amount * bounds.w;
			}

			if (sz.x < 0)
				sz.x = 0;
			if (sz.y < 0)
				sz.y = 0;

			return sz;
		}

		point control_base::get_position(rectangle _ctx)
		{
			point pos;

			switch (box.x.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				pos.x = box.x.amount * _ctx.w;
				break;
			case measure_units::pixels:
				pos.x = box.x.amount;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:				
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				pos.x = 0;
				break;
			}

			switch (box.y.units)
			{
			case measure_units::percent_container:
			case measure_units::percent_remaining:
				pos.y = box.y.amount * _ctx.h;
				break;
			case measure_units::pixels:
				pos.y = box.y.amount;
				break;
			case measure_units::font:
			case measure_units::font_golden_ratio:
			case measure_units::percent_aspect:
			case measure_units::percent_child:
				throw std::logic_error("font, aspect and child units cannot be used for position");
				break;
			default:
				pos.y = 0;
				break;
			}

			return pos;
		}

		point control_base::get_remaining(point _ctx)
		{
			point pt = { 0.0, 0.0, 0.0 };
			pt = _ctx - pt;
			return pt;
		}

		void container_control::apply(control_base& _ref)
		{
			if (item_box.height.amount > 0 && item_box.width.amount > 0)
			{
				_ref.box = item_box;
			}

			if (item_margin.amount > 0)
			{
				_ref.margin = item_margin;
			}
		}

		container_control& container_control::set_content_align(visual_alignment _new_alignment)
		{
			content_alignment = _new_alignment;
			return *this;
		}

		container_control& container_control::set_content_cross_align(visual_alignment _new_alignment)
		{
			content_cross_alignment = _new_alignment;
			return *this;
		}

		container_control& container_control::set_item_origin(measure _x, measure _y)
		{
			item_box.x = _x;
			item_box.y = _y;
			return *this;
		}

		container_control& container_control::set_item_size(measure _width, measure _height)
		{
			if (_width.amount > 0)
				item_box.width = _width;
			if (_height.amount > 0)
				item_box.height = _height;
			return *this;
		}

		container_control& container_control::set_item_position(layout_rect _new_layout)
		{
			item_box = _new_layout;
			return *this;
		}

		container_control& container_control::set_item_margin(measure _item_margin)
		{
			item_margin = _item_margin;
			return *this;
		}

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

		bool control_base::contains(point pt)
		{
			return rectangle_math::contains(bounds, pt.x, pt.y);
		}

		rectangle& control_base::set_bounds(rectangle& _bounds)
		{
			bounds = _bounds;

			margin_amount.x = margin_amount.y = to_pixels(margin);
			padding_amount.x = padding_amount.y = to_pixels(padding);

			if (parent) 
			{
				auto pbounds = parent->get_inner_bounds();
				if (bounds.x < pbounds.x)
					bounds.x = pbounds.x;
				if (bounds.y < pbounds.y)
					bounds.y = pbounds.y;
				if (bounds.right() > pbounds.right())
				{
					bounds.w -= (bounds.right() - pbounds.right());
				}
				if (bounds.bottom() > pbounds.bottom())
				{
					bounds.h -= (bounds.bottom() - pbounds.bottom());
				}
				if (bounds.w < 0) bounds.w = 0;
				if (bounds.h < 0) bounds.h = 0;
			}

			inner_bounds = bounds;

			inner_bounds.x += margin_amount.x;
			inner_bounds.y += margin_amount.y;
			inner_bounds.w -= (margin_amount.x * 2);
			inner_bounds.h -= (margin_amount.y * 2);

			if (inner_bounds.w < 0) inner_bounds.w = 0;
			if (inner_bounds.h < 0) inner_bounds.h = 0;

			for (auto pr : push_requests) {
				auto target = find(pr.dest_control_id);

				if (target) {
					auto temp_bounds = target->bounds;
					if (pr.properties_to_push & control_push_property::cp_left_bounds)
					{
						temp_bounds.x = bounds.x;
					}
					if (pr.properties_to_push & control_push_property::cp_top_bounds)
					{
						temp_bounds.y = bounds.y;
					}
					if (pr.properties_to_push & control_push_property::cp_right_bounds)
					{
						temp_bounds.x += bounds.right() - temp_bounds.right();
					}
					if (pr.properties_to_push & control_push_property::cp_bottom_bounds)
					{
						temp_bounds.x += bounds.bottom() - temp_bounds.bottom();
					}
					target->arrange(temp_bounds);
				}
			}

			on_resize();
			return bounds;
		}

		void control_base::arrange(rectangle _bounds)
		{
			set_bounds(_bounds);
		}

		void control_base::arrange_children(rectangle _bounds, 
			std::function<point(const rectangle* _bounds, control_base*)> _initial_origin,
			std::function<point (point* _origin, const rectangle *_bounds, control_base *)> _next_origin)
		{
			point origin = { _bounds.x, _bounds.y, 0.0 };
			point remaining = { _bounds.w, _bounds.h, 0.0 };

			if (children.size()) {

				auto sichild = std::begin(children);

				origin = _initial_origin(&_bounds, sichild->get());

				remaining = get_remaining(remaining);

				while (sichild != std::end(children))
				{
					auto child = *sichild;

					auto sz = child->get_size(_bounds, remaining);
					auto pos = child->get_position(_bounds);

					rectangle new_bounds;
					new_bounds.x = origin.x + pos.x;
					new_bounds.y = origin.y + pos.y;
					new_bounds.w = sz.x;
					new_bounds.h = sz.y;

					child->arrange(new_bounds);

					origin = _next_origin(&origin, &bounds, child.get());

					sichild++;
				}

			}
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
							temp.y = _bounds->y + (_bounds->h -  sz.y) / 2.0;
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
							origin.y= _bounds->y + (_bounds->h - sz.y) / 2.0;
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
						temp.y = _bounds->y +_bounds->h - h;
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

		void frame_layout::set_page(page& _page)
		{
			children.clear();
			children.push_back(_page.root);
			arrange(bounds);
		}

		HMENU menu_item::to_menu_children(HMENU hmenu, int idx)
		{
			if (children.size())
			{
				HMENU popupMenu = ::CreatePopupMenu();
				int counter = 0;
				for (auto child : children) 
				{
					child->to_menu_children(popupMenu, counter++);
				}
				::AppendMenu(hmenu, MF_POPUP, (UINT_PTR)popupMenu, name.c_str());
			}
			else if (is_separator)
			{
				::AppendMenu(hmenu, MF_SEPARATOR, id, nullptr);
			}
			else
			{
				::AppendMenu(hmenu, MF_STRING, id, name.c_str());
			}

			return hmenu;
		}

		HMENU menu_item::to_menu()
		{

			MENUITEMINFO info = {};

			if (!created_menu) {

				created_menu = ::CreatePopupMenu();

				if (children.size())
				{
					int counter = 0;
					for (auto child : children)
					{
						child->to_menu_children(created_menu, counter++);
					}
				}

			}

			return created_menu;
		}

		void control_base::on_resize()
		{
			auto ti = typeid(*this).name();
//			std::cout << "resize control_base:" << ti << " " << bounds.x << "," << bounds.y << " x " << bounds.w << " " << bounds.h << std::endl;
		}

		container_control::container_control()
		{
			parent = nullptr;
			id = id_counter::next();
		}

		container_control::container_control(container_control* _parent, int _id)
		{
			parent = _parent;
			id = _id;
			if (parent && get_nchittest() == HTCLIENT) {
				set_nchittest(parent->get_nchittest());
			}
		}

		draw_control::draw_control()
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			parent = nullptr;
			id = id_counter::next();
		}

		draw_control::draw_control(container_control* _parent, int _id)
		{
			background_brush_win32 = nullptr;
			background_brush = {};
			parent = _parent;
			id = _id;
		}

		void draw_control::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{
			host = _host;
			if (auto phost = _host.lock()) {
				window = phost->createDirect2Window(id, inner_bounds);
			}
			if (on_create) {
				on_create(this);
			}
			for (auto child : children) {
				child->create(_host);
			}
		}

		void draw_control::destroy()
		{
			for (auto child : children) {
				child->destroy();
			}
		}

		void draw_control::on_resize()
		{
			auto ti = typeid(*this).name();

			if (auto pwindow = window.lock()) 
			{
				pwindow->moveWindow(inner_bounds.x, inner_bounds.y, inner_bounds.w, inner_bounds.h);
			}
		}

		void draw_control::draw()
		{
			bool adapter_blown_away = false;

			if (auto pwindow = window.lock()) 
			{
				pwindow->beginDraw(adapter_blown_away);
				if (!adapter_blown_away)
				{
					auto context = pwindow->getContext();

					auto& bc = background_brush.brushColor;

					if (background_brush.active) 
					{
						if (background_brush_win32)
							::DeleteBrush(background_brush_win32);

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

					if (on_draw != nullptr) {
						on_draw(this);
					}
					else 
					{
						
					}
				}
				pwindow->endDraw(adapter_blown_away);
			}
			for (auto& child : children) {
				child->draw();
			}
		}

		void draw_control::render(CComPtr<ID2D1DeviceContext>& _dest)
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
				_dest->DrawBitmap(bm, &dest, 1.0, D2D1_INTERPOLATION_MODE::D2D1_INTERPOLATION_MODE_LINEAR, &source );
			}
			for (auto child : children) {
				child->render(_dest);
			}
		}
 
		/*
		class scrollbar_control : public windows_control<WTL::CScrollBar, WS_VISIBLE | WS_BORDER | WS_CHILD>
		{
		public:
		};
		*/

		text_display_control::text_display_control()
		{
			init();
		}

		text_display_control::text_display_control(container_control* _parent, int _id)
			: draw_control(_parent, _id)
		{
			init();
			if (_parent)
			{
				set_nchittest(_parent->get_nchittest());
			}
		}

		void text_display_control::init()
		{
			set_origin(0.0_px, 0.0_px);
			set_size(1.0_container, 1.2_fontgr);

			on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					pwindow->getContext().setSolidColorBrush(&this->text_fill_brush);
					pwindow->getContext().setTextStyle(&this->text_style);
				}
			};

			on_draw = [this](draw_control* _src) {
				if (auto pwindow = this->window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						std::string test_text = std::format("{0}, {1}, {2}", text, draw_bounds.x, draw_bounds.y,  (long)this);

						pwindow->getContext().drawText(text.c_str(), &draw_bounds, this->text_style.name, this->text_fill_brush.name);
					//	pwindow->getContext().drawRectangle(&draw_bounds, this->text_fill_brush.name, 4, nullptr);
					}
				}
			};
		}

		text_display_control& text_display_control::set_text(std::string _text)
		{
			text = _text;
			return *this;
		}

		text_display_control& text_display_control::set_text_fill(solidBrushRequest _brushFill)
		{
			text_fill_brush = _brushFill;
			return *this;
		}

		text_display_control& text_display_control::set_text_fill(std::string _color)
		{
			text_fill_brush.name = typeid(*this).name();
			text_fill_brush.brushColor = toColor(_color.c_str());
			return *this;
		}

		text_display_control& text_display_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
		{
			text_style.name = typeid(*this).name();
			text_style.fontName = _font_name;
			text_style.fontSize = _font_size;
			text_style.bold = _bold;
			text_style.underline = _underline;
			text_style.italics = _italic;
			text_style.strike_through = _strike_through;
			return *this;
		}

		text_display_control& text_display_control::set_text_style(textStyleRequest request)
		{
			text_style = request;
			return *this;
		}

		image_control::image_control()
		{
			init();
		}

		image_control::image_control(container_control* _parent, int _id)
			: draw_control(_parent, _id)
		{
			init();
		}

		image_control::image_control(container_control* _parent, int _id, std::string _file_name) : draw_control(_parent, _id)
		{
			init();
			load_from_file(_file_name);
		}

		image_control::image_control(container_control* _parent, int _id, int _source_control_id) : draw_control(_parent, _id)
		{
			init();
			load_from_control(_source_control_id);
		}

		void image_control::load_from_file(std::string _name)
		{
			image_mode = image_modes::use_file_name;
			image_file_name = _name;
			instance.bitmapName = std::format("bitmap_file_{0}", id);
		}

		void image_control::load_from_resource(DWORD _resource_id)
		{
			image_mode = image_modes::use_resource_id;
			image_resource_id = _resource_id;
			instance.bitmapName = std::format("bitmap_resource_{0}_{1}", id, _resource_id);
		}

		void image_control::load_from_control(int _control_id)
		{
			image_mode = image_modes::use_control_id;
			image_control_id = _control_id;
			instance.bitmapName = std::format("bitmap_control_{0}_{1}", id, _control_id);
		}

		void image_control::init()
		{
			set_origin(0.0_px, 0.0_px);
			set_size(50.0_px, 50.0_px);

			on_create = [this](draw_control* _src)
			{
				if (auto pwindow = this->window.lock())
				{
					auto &context = pwindow->getContext();

					solidBrushRequest sbr;
					sbr.active = true;
					sbr.brushColor = toColor("FFFF00");
					sbr.name = "image_control_test";
					context.setSolidColorBrush(&sbr);

					switch (image_mode) {
					case image_modes::use_control_id:
						break;
					case image_modes::use_resource_id:
						{
							bitmapRequest request = {};
							request.resource_id = image_resource_id;
							request.name = instance.bitmapName;
							request.cropEnabled = false;
							point pt = { inner_bounds.w, inner_bounds.h };
							request.sizes.push_back(pt);
							context.setBitmap(&request);
							break;
						}
						break;
					case image_modes::use_file_name:
						{
							bitmapRequest request = {};
							request.file_name = image_file_name;
							request.name = instance.bitmapName;
							request.cropEnabled = false;
							point pt = { inner_bounds.w, inner_bounds.h };
							request.sizes.push_back(pt);
							context.setBitmap(&request);
							auto szfound = std::begin(request.sizes);
							if (szfound != std::end(request.sizes)) {
								instance.width = request.sizes.begin()->x;
								instance.height = request.sizes.begin()->y;
							}
							else 
							{
								instance.width = 0;
								instance.height = 0;
							}
						break;
						}
					}
				}
			};

			on_draw = [this](draw_control* _src) {
				if (auto pwindow = this->window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						instance.copyId = 0;
						instance.selected = false;
						instance.x = draw_bounds.x;
						instance.y = draw_bounds.y;
						instance.width = draw_bounds.w;
						instance.height = draw_bounds.h;
						instance.alpha = 1.0;

						auto& context = pwindow->getContext();

						context.drawBitmap(&instance);
					}
				}
			};
		}

		image_control::~image_control()
		{
			;
		}

		void title_control::set_default_styles()
		{

			auto& st = styles.get_style();

			background_brush.name = "title_fill";
			background_brush.brushColor = toColor(st.TitleBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "title_text_fill";
			text_fill_brush.brushColor = toColor(st.TitleTextColor);

			text_style = {};
			text_style.name = "title_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 24;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = st.PrevailingAlignment;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
		}

		title_control::title_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		title_control::title_control()
		{
			set_default_styles();
		}

		title_control::~title_control()
		{
		}

		void subtitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "subtitle_fill";
			background_brush.brushColor = toColor(st.SubtitleBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "subtitle_text_fill";
			text_fill_brush.brushColor = toColor(st.SubtitleTextColor);

			text_style = {};
			text_style.name = "subtitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 18;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = true;
			text_style.strike_through = false;
			text_style.horizontal_align = st.PrevailingAlignment;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
		}

		subtitle_control::subtitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		subtitle_control::subtitle_control()
		{
			set_default_styles();
		}

		subtitle_control::~subtitle_control()
		{
		}

		void chaptertitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "chaptertitle_fill";
			background_brush.brushColor = toColor(st.ChapterTitleBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "chaptertitle_text_fill";
			text_fill_brush.brushColor = toColor(st.ChapterTitleTextColor);

			text_style = {};
			text_style.name = "chaptertitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 16;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = st.PrevailingAlignment;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
		}

		chaptertitle_control::chaptertitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		chaptertitle_control::chaptertitle_control()
		{
			set_default_styles();
		}

		chaptertitle_control::~chaptertitle_control()
		{
		}

		void chaptersubtitle_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "chaptersubtitle_fill";
			background_brush.brushColor = toColor(st.SubchapterTitleBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "chaptersubtitle_text_fill";
			text_fill_brush.brushColor = toColor(st.SubchapterTitleTextColor);

			text_style = {};
			text_style.name = "chaptersubtitle_text_style";
			text_style.fontName = st.PrimaryFont;
			text_style.fontSize = 14;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = st.PrevailingAlignment;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
		}


		chaptersubtitle_control::chaptersubtitle_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		chaptersubtitle_control::chaptersubtitle_control()
		{
			set_default_styles();
		}

		chaptersubtitle_control::~chaptersubtitle_control()
		{
		}

		void paragraph_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "paragraph_fill";
			background_brush.brushColor = toColor(st.ParagraphBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "paragraph_text_fill";
			text_fill_brush.brushColor = toColor(st.ParagraphTextColor);

			text_style = {};
			text_style.name = "paragraph_text_style";
			text_style.fontName = styles.get_style().PrimaryFont;
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_near;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = true;
		}

		paragraph_control::paragraph_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		paragraph_control::paragraph_control()
		{
			set_default_styles();
		}

		paragraph_control::~paragraph_control()
		{
		}

		void code_control::set_default_styles()
		{
			auto& st = styles.get_style();

			background_brush.name = "code_fill";
			background_brush.brushColor = toColor(st.CodeBackgroundColor);
			background_brush.active = true;

			text_fill_brush.name = "code_text_fill";
			text_fill_brush.brushColor = toColor(st.CodeTextColor);

			text_style = {};
			text_style.name = "code_text_style";
			text_style.fontName = "Cascadia Mono,Courier New";
			text_style.fontSize = 10;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_near;
			text_style.vertical_align = visual_alignment::align_near;
			text_style.wrap_text = false;
		}

		code_control::code_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		code_control::code_control()
		{
			set_default_styles();
		}

		code_control::~code_control()
		{
		}

		void label_control::set_default_styles()
		{
			text_fill_brush.name = "label_text_fill";
			text_fill_brush.brushColor = toColor(styles.get_style().TextColor.c_str());

			text_style = {};
			text_style.name = "label_text_style";
			text_style.fontName = styles.get_style().PrimaryFont;
			text_style.fontSize = 12;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_near;
			text_style.vertical_align = visual_alignment::align_far;
			text_style.wrap_text = false;
		}

		label_control::label_control()
		{
			set_default_styles();
		}

		label_control::label_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			set_default_styles();
		}

		label_control::~label_control()
		{
			
		}

		void placeholder_control::set_default_styles()
		{
			text_fill_brush.name = "placeholder_text_fill";
			text_fill_brush.brushColor = toColor(styles.get_style().TextColor.c_str());

			text_style = {};
			text_style.name = "placeholder_text_style";
			text_style.fontName = styles.get_style().PrimaryFont;
			text_style.fontSize = 14;
			text_style.bold = false;
			text_style.italics = false;
			text_style.underline = false;
			text_style.strike_through = false;
			text_style.horizontal_align = visual_alignment::align_center;
			text_style.vertical_align = visual_alignment::align_center;
			text_style.wrap_text = false;
		}

		placeholder_control::placeholder_control()
		{
			text = "Placeholder";
		}

		placeholder_control::placeholder_control(container_control* _parent, int _id) : text_display_control(_parent, _id)
		{
			text = "Placeholder";
		}

		placeholder_control::~placeholder_control()
		{

		}

		comboboxex_control::comboboxex_control()
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		comboboxex_control::comboboxex_control(container_control* _parent, int _id) : windows_control<WTL::CComboBoxEx, ComboExWindowStyles>(_parent, _id)
		{
			control_base::set_origin(0.0_px, 0.0_px);
			control_base::set_size(1.0_container, 2.0_fontgr);
		}

		void comboboxex_control::data_changed()
		{
			if (window.IsWindow()) {
				window.ResetContent();
				for (auto element : choices.items.items())
				{
					auto c = element.value();
					int lid = c[choices.id_field].template get<int>();
					std::string description = c[choices.text_field].template get<std::string>();

					COMBOBOXEXITEM cbex = {};
					cbex.mask = CBEIF_TEXT | CBEIF_LPARAM;
					cbex.iItem = -1;
					cbex.pszText = (LPTSTR)description.c_str();
					cbex.iImage = 0;
					cbex.iSelectedImage = 0;
					cbex.iIndent = 0;
					cbex.lParam = lid;
					window.InsertItem(&cbex);
				}
			}
		}

		void comboboxex_control::set_list(list_data& _choices)
		{
			choices = _choices;
			data_changed();
		}

		void comboboxex_control::on_create()
		{
			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

				RECT r;
				r.left = boundsPixels.x;
				r.top = boundsPixels.y;
				r.right = boundsPixels.x + bounds.w;
				r.bottom = boundsPixels.y + text_style.fontSize * 8;
				window.MoveWindow(&r);
			}

			data_changed();
		}

		void comboboxex_control::on_resize()
		{
			if (auto phost = window_host.lock()) {
				auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

				RECT r;
				r.left = boundsPixels.x;
				r.top = boundsPixels.y;
				r.right = boundsPixels.x + boundsPixels.w;
				r.bottom = boundsPixels.y + windows_control<WTL::CComboBoxEx, ComboExWindowStyles>::text_style.fontSize * 8;
				if (window.m_hWnd) {
					windows_control<WTL::CComboBoxEx, ComboExWindowStyles>::window.MoveWindow(&r);
				}
			}
		}

		void richedit_control::set_html(const std::string& _text)
		{

		}

		std::string richedit_control::get_html()
		{
			return "";
		}

		void datetimepicker_control::set_text(const std::string& _text)
		{

		}

		std::string datetimepicker_control::get_text()
		{
			return "";
		}

		/*
		class monthcalendar_control : public windows_control<CMonthCalendarCtrl, WS_VISIBLE | WS_BORDER | WS_CHILD>
		{
		public:
		};
		*/

		bool animate_control::open(const std::string& _name)
		{
			return window.Open(_name.c_str());
		}

		bool animate_control::open(DWORD resource_id)
		{
			return window.Open(resource_id);
		}

		bool animate_control::play(UINT from, UINT to, UINT rep)
		{
			return window.Play(from, to, rep);
		}

		bool animate_control::play()
		{
			return window.Play(0, -1, 1);
		}

		bool animate_control::stop()
		{
			return window.Stop();
		}

		gradient_button_control::gradient_button_control(container_control* _parent, int _id, std::string _base_name)
		{
			buttonFaceNormal.name = _base_name + "_face_normal";
			buttonFaceOver.name = _base_name + "_face_over";
			buttonFaceDown.name = _base_name + "_face_down";

			foregroundNormal.name = _base_name + "_fore_normal";
			foregroundOver.name = _base_name + "_fore_over";
			foregroundDown.name = _base_name + "_fore_down";

			buttonFaceNormal.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.8 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonFaceOver.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.8 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonFaceDown.gradientStops = {
				{ toColor("#202020FF"), 0.0 },
				{ toColor("#707070FF"), 0.9 },
				{ toColor("#202020FF"), 1.0 },
			};

			buttonBackLight.gradientStops = {
				{ toColor("#00000000"), 0.0 },
				{ toColor("#20202020"), 0.9 },
				{ toColor("#E0F0E0FF"), 1.0 }
			};

			foregroundNormal.active = true;
			foregroundNormal.brushColor = toColor("#808080");
			foregroundOver.active = true;
			foregroundOver.brushColor = toColor("#70A070");
			foregroundDown.active = true;
			foregroundDown.brushColor = toColor("#00FF00");

		}

		void gradient_button_control::arrange(rectangle _ctx)
		{
			draw_control::arrange(_ctx);

			if (auto pwindow = this->window.lock())
			{
				buttonFaceNormal.start.x = inner_bounds.w / 2;
				buttonFaceNormal.start.y = 0;
				buttonFaceNormal.stop.y = inner_bounds.h;
				buttonFaceNormal.stop.x = inner_bounds.w / 2;

				buttonFaceDown.start.x = inner_bounds.w / 2;
				buttonFaceDown.start.y = 0;
				buttonFaceDown.stop.y = inner_bounds.h;
				buttonFaceDown.stop.x = inner_bounds.w / 2;

				buttonFaceOver.start.x = inner_bounds.w / 2;
				buttonFaceOver.start.y = 0;
				buttonFaceOver.stop.y = inner_bounds.h;
				buttonFaceOver.stop.x = inner_bounds.w / 2;

				buttonBackLight.center = rectangle_math::center( _ctx );
				buttonBackLight.offset = {};
				buttonBackLight.radiusX = inner_bounds.w / 2.0;
				buttonBackLight.radiusY = inner_bounds.h / 2.0;

				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceNormal);
				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceDown);
				pwindow->getContext().setLinearGradientBrush(&this->buttonFaceOver);
				pwindow->getContext().setRadialGradientBrush(&this->buttonBackLight);
				pwindow->getContext().setSolidColorBrush(&this->foregroundNormal);
				pwindow->getContext().setSolidColorBrush(&this->foregroundDown);
				pwindow->getContext().setSolidColorBrush(&this->foregroundOver);
			}
		}

		gradient_button_control::~gradient_button_control()
		{
			;
		}

		void gradient_button_control::draw_button(std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape)
		{
			if (auto pwindow = window.lock())
			{
				if (auto phost = host.lock()) {
					auto draw_bounds = inner_bounds;

					draw_bounds.x = 0;
					draw_bounds.y = 0;

					auto &context = pwindow->getContext();

					if (mouse_left_down.value())
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceDown.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(&face_bounds, &foregroundDown);
					}
					else if (mouse_over.value())
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceOver.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(&face_bounds, &foregroundOver);
					}
					else 
					{
						context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonFaceNormal.name);
						auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
						//context.drawRectangle(&draw_bounds, nullptr, 0.0, buttonBackLight.name);
						draw_shape(&face_bounds, &foregroundNormal);
					}
				}
			}
		}

		menu_button_control::menu_button_control(container_control* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
		{
			auto ctrl = this;

			on_draw = [this](control_base* _item)
			{
				if (auto pwindow = window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						point shape_origin;
						point* porigin = &shape_origin;

						auto& context = pwindow->getContext();
						auto pcontext = &context;

						draw_shape = [this, porigin, pcontext](rectangle* _bounds, solidBrushRequest* _foreground) {

							point start;
							point stop;

							start.x = _bounds->x;
							start.y = _bounds->y + _bounds->h / 2.0;
							stop.x = _bounds->right();
							stop.y = _bounds->y + _bounds->h / 2.0;

							pcontext->drawLine(&start, &stop, _foreground->name, 4);

							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->bottom());
							pid.path.addLineTo(_bounds->x, _bounds->bottom());
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
						};

						draw_button(draw_shape);

					}
				}
			};
		}

		void menu_button_control::on_subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{
			if (auto ppage = _page.lock()) {
				ppage->on_mouse_left_click(id, [this, _presentation, _page](mouse_left_click_event evt)
				{
						this->menu.subscribe(_presentation, _page);
				});
			}
		}

		minimize_button_control::minimize_button_control(container_control* _parent, int _id) : gradient_button_control(_parent, _id, "minimize")
		{
			auto ctrl = this;

			on_draw = [this](control_base* _item) 
			{
				if (auto pwindow = window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						std::function<void(rectangle *_bounds, solidBrushRequest *_foreground)> draw_shape;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						point shape_origin;
						point *porigin = &shape_origin;

						auto& context = pwindow->getContext();
						auto pcontext = &context;

						draw_shape = [this, porigin, pcontext](rectangle* _bounds, solidBrushRequest* _foreground) {
							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->y);
							pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->bottom());
							pid.path.addLineTo(_bounds->right(), _bounds->y);
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
						};

						draw_button(draw_shape);

					}
				}
			};
		}

		void minimize_button_control::on_subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{

		}

		minimize_button_control::~minimize_button_control() 
		{ 
			; 
		}

		maximize_button_control::maximize_button_control(container_control* _parent, int _id) : gradient_button_control(_parent, _id, "maximize")
		{ 
			auto ctrl = this;

			on_draw = [this](control_base* _item)
			{
				if (auto pwindow = window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						point shape_origin;
						point* porigin = &shape_origin;

						auto& context = pwindow->getContext();
						auto pcontext = &context;

						draw_shape = [this, porigin, pcontext](rectangle* _bounds, solidBrushRequest* _foreground) {
							pathImmediateDto pid;
							porigin->x = _bounds->x;
							porigin->y = _bounds->y;
							porigin->z = 0;
							pid.path.addLineTo(_bounds->x, _bounds->bottom());
							pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->y);
							pid.path.addLineTo(_bounds->right(), _bounds->bottom());
							pid.position = *porigin;
							pid.rotation = 0;
							pid.strokeWidth = 4;
							pid.borderBrushName = _foreground->name;
							pid.closed = true;
							pcontext->drawPath(&pid);
						};

						draw_button(draw_shape);

					}
				}
			};
		}

		void maximize_button_control::on_subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{

		}

		maximize_button_control::~maximize_button_control() 
		{
			; 
		}

		close_button_control::close_button_control(container_control* _parent, int _id) : gradient_button_control(_parent, _id, "close")
		{
			auto ctrl = this;

			on_draw = [this](control_base* _item)
			{
				if (auto pwindow = window.lock())
				{
					if (auto phost = host.lock()) {
						auto draw_bounds = inner_bounds;

						std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;

						draw_bounds.x = 0;
						draw_bounds.y = 0;

						point shape_origin;
						point* porigin = &shape_origin;

						auto& context = pwindow->getContext();
						auto pcontext = &context;

						draw_shape = [this, porigin, pcontext](rectangle* _bounds, solidBrushRequest* _foreground) {
							point start, stop;
							start.x = _bounds->x;
							start.y = _bounds->y;
							stop.x = _bounds->right();
							stop.y = _bounds->bottom();
							pcontext->drawLine(&start, &stop, _foreground->name, 4);
							start.x = _bounds->right();
							start.y = _bounds->y;
							stop.x = _bounds->x;
							stop.y = _bounds->bottom();
							pcontext->drawLine(&start, &stop, _foreground->name, 4);
						};

						draw_button(draw_shape);
					}
				}
			};
		}

		void close_button_control::on_subscribe(presentation* _presentation, std::weak_ptr<page> _page)
		{
			;
		}

		close_button_control::~close_button_control() 
		{ 
			; 
		}

		page::page(const char* _name)
		{
			name = _name != nullptr ? _name : "Test";
			root = std::make_shared<column_layout>();
		}

		page::~page()
		{
			destroy();
		}

		void page::clear()
		{
			destroy();
			root = std::make_shared<column_layout>();
		}

		void page::create(std::weak_ptr<win32::directApplicationWin32> _host)
		{
			if (auto whost = _host.lock()) {
				auto pos = whost->getWindowClientPos();
				pos = whost->toDipsFromPixels(pos);
				arrange(pos.w, pos.h);
				if (root.get())
				{
					root->create(_host);
				}
			}
		}

		void page::destroy()
		{
			if (root.get())
			{
				root->destroy();
			}
		}

		void page::draw()
		{
			if (root.get())
			{
				root->draw();
			}
		}

		void page::render(CComPtr<ID2D1DeviceContext>& _context)
		{
			if (root.get())
			{
				root->render(_context);
			}
		}

		void page::update(double _elapsedSeconds, double _totalSeconds)
		{
			if (update_event) {
				update_event(this, _elapsedSeconds, _totalSeconds);
			}
		}

		void page::subscribe(presentation* _presentation)
		{
			root->on_subscribe(_presentation, shared_from_this() );
		}

		row_layout& page::row_begin(int id)
		{
//			std::cout << "create: row"<< std::endl;
			auto new_row = std::make_shared<row_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		column_layout& page::column_begin(int id)
		{
	//		std::cout << "create: column" << std::endl;
			auto new_row = std::make_shared<column_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		absolute_layout& page::absolute_begin(int id)
		{
//			std::cout << "create: begin" << std::endl;
			auto new_row = std::make_shared<absolute_layout>((container_control*)nullptr, id);
			root = new_row;
			return *new_row.get();
		}

		presentation::presentation()
		{
			;
		}

		presentation::~presentation()
		{
			;
		}

		void presentation::open_menu(control_base* _base, menu_item& _menu)
		{
			auto menu = _menu.to_menu();
			HWND hwndMenu = this->getHost()->getMainWindow();
			point tpstart;
			auto &bpos = _base->get_bounds();
			tpstart.x = bpos.x * ::GetDpiForWindow(hwndMenu) / 96.0;
			tpstart.y = bpos.y * ::GetDpiForWindow(hwndMenu) / 96.0;
			::TrackPopupMenuEx(menu, TPM_CENTERALIGN | TPM_TOPALIGN, tpstart.x, tpstart.y, hwndMenu, nullptr);
		}

		void page::arrange(double width, double height, double _padding)
		{

			double pd = _padding * 2.0;
			rectangle bounds;
			bounds.x = 0;
			bounds.y = 0;
			bounds.w = width;
			bounds.h = height;

//			std::cout << "page arrange: " << bounds.w << " " << bounds.h << std::endl;

			root->arrange(bounds);
//			std::cout << std::endl;
		}

		void page::on_key_up(int _control_id, std::function< void(key_up_event) > handler)
		{
			auto evt = std::make_shared<key_up_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_up = handler;
			key_up_events[_control_id] = evt;
		}

		void page::on_key_down(int _control_id, std::function< void(key_down_event) >  handler)
		{
			auto evt = std::make_shared<key_down_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_key_down = handler;
			key_down_events[_control_id] = evt;
		}

		void page::on_mouse_left_click(int _control_id, std::function< void(mouse_left_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_left_click_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_left_click = handler;
			mouse_left_click_events[_control_id] = evt;
		}

		void page::on_mouse_right_click(int _control_id, std::function< void(mouse_right_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_right_click_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_right_click = handler;
			mouse_right_click_events[_control_id] = evt;
		}


		void page::on_mouse_move(int _control_id, std::function< void(mouse_move_event) > handler)
		{
			auto evt = std::make_shared<mouse_move_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_move = handler;
			mouse_move_events[_control_id] = evt;
		}

		void page::on_mouse_click(int _control_id, std::function< void(mouse_click_event) > handler)
		{
			auto evt = std::make_shared<mouse_click_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_mouse_click = handler;
			mouse_click_events[_control_id] = evt;
		}

		void page::on_item_changed(int _control_id, std::function< void(item_changed_event) > handler)
		{
			auto evt = std::make_shared<item_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			item_changed_events[_control_id] = evt;
		}

		void page::on_list_changed(int _control_id, std::function< void(list_changed_event) > handler)
		{
			auto evt = std::make_shared<list_changed_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_change = handler;
			list_changed_events[_control_id] = evt;
		}

		void page::on_command(int _control_id, std::function< void(command_event) > handler)
		{
			auto evt = std::make_shared<command_event_binding>();
			evt->subscribed_item_id = _control_id;
			evt->on_command  = handler;
			command_events[_control_id] = evt;
		}

		void page::on_update(update_function fnc)
		{
			update_event = fnc;
		}

		void page::handle_key_up(int _control_id, key_up_event evt)
		{
			if (key_up_events.contains(_control_id)) {
				key_up_events[_control_id]->on_key_up(evt);
			}
		}

		void page::handle_key_down(int _control_id, key_down_event evt)
		{
			if (key_down_events.contains(_control_id)) {
				key_down_events[_control_id]->on_key_down(evt);
			}
		}

		void page::handle_mouse_move(int _control_id, mouse_move_event evt)
		{
			if (mouse_move_events.contains(_control_id)) {
				auto& ptrx = mouse_move_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->get_bounds().x;
					evt.relative_point.y = evt.absolute_point.y - temp->get_bounds().y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_move(evt);
				}
			}

			if (!_control_id) 
			{
				for (auto evh : mouse_move_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->get_bounds().x;
						evt.relative_point.y = evt.absolute_point.y - lck->get_bounds().y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_move(evt);
					}
				}
			}

		}

		void page::handle_mouse_click(int _control_id, mouse_click_event evt)
		{
			if (mouse_click_events.contains(_control_id)) {
				auto& ptrx = mouse_click_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->get_bounds().x;
					evt.relative_point.y = evt.absolute_point.y - temp->get_bounds().y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_click(evt);
				}
			}

			if (!_control_id)
			{
				for (auto evh : mouse_click_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->get_bounds().x;
						evt.relative_point.y = evt.absolute_point.y - lck->get_bounds().y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_click(evt);
					}
				}
			}
		}

		void page::handle_mouse_left_click(int _control_id, mouse_left_click_event evt)
		{
			if (mouse_left_click_events.contains(_control_id)) {
				auto& ptrx = mouse_left_click_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->get_bounds().x;
					evt.relative_point.y = evt.absolute_point.y - temp->get_bounds().y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_left_click(evt);
				}
			}

			if (!_control_id)
			{
				for (auto evh : mouse_left_click_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->get_bounds().x;
						evt.relative_point.y = evt.absolute_point.y - lck->get_bounds().y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_left_click(evt);
					}
				}
			}
		}

		void page::handle_mouse_right_click(int _control_id, mouse_right_click_event evt)
		{
			if (mouse_right_click_events.contains(_control_id)) {
				auto& ptrx = mouse_right_click_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.relative_point.x = evt.absolute_point.x - temp->get_bounds().x;
					evt.relative_point.y = evt.absolute_point.y - temp->get_bounds().y;
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_mouse_right_click(evt);
				}
			}

			if (!_control_id)
			{
				for (auto evh : mouse_right_click_events)
				{
					auto lck = evh.second->control.lock();
					if (lck && lck->contains(evt.absolute_point))
					{
						evt.relative_point.x = evt.absolute_point.x - lck->get_bounds().x;
						evt.relative_point.y = evt.absolute_point.y - lck->get_bounds().y;
						evt.control = lck.get();
						evt.control_id = lck->id;
						evh.second->on_mouse_right_click(evt);
					}
				}
			}
		}


		void page::handle_item_changed(int _control_id, item_changed_event evt)
		{
			if (item_changed_events.contains(_control_id)) {
				auto& ptrx = item_changed_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_change(evt);
				}
			}
		}

		void page::handle_list_changed(int _control_id, list_changed_event evt)
		{
			if (list_changed_events.contains(_control_id)) {
				auto& ptrx = list_changed_events[_control_id];
				if (auto temp = ptrx.get()->control.lock()) {
					evt.control = temp.get();
					evt.control_id = temp->id;
					ptrx->on_change(evt);
				}
			}
		}

		void page::handle_command(int _control_id, command_event evt)
		{
			if (command_events.contains(_control_id)) {
				auto& ptrx = command_events[_control_id];
				ptrx->on_command(evt);
			}
		}

		page& presentation::create_page(std::string _name, std::function<void(page& pg)> _settings)
		{
			auto new_page = std::make_shared<page>();
			pages[_name] = new_page;
			if (current_page.expired()) {
				current_page = new_page;
			}
			page& pg = *new_page.get();
			if (_settings) {
				_settings(pg);
			}
			return pg;
		}

		void presentation::select_page(const std::string& _page_name)
		{
			if (pages.contains(_page_name)) {
				if (auto ppage = current_page.lock()) {
					ppage->destroy();
				}
				current_page = pages[_page_name];
			}

			onCreated();
			onResize(current_size, 1.0);

			if (auto phost = getHost())
			{
				HWND hwndMainMenu = phost->getMainWindow();
				if (auto ppage = current_page.lock()) {
					if (ppage->menu) 
					{
						HMENU hmenu = ppage->menu->to_menu();
						::SetMenu(hwndMainMenu, hmenu);
						::DrawMenuBar(hwndMainMenu);
					}
				}
			}
		}

		void presentation::onCreated()
		{
			auto cp = current_page.lock();
			if (cp) {
				auto host = getHost();
				auto sheet = styles.get_style();
			
				auto post = host->getWindowClientPos();
				host->toPixelsFromDips(post);
				cp->arrange(post.w, post.h);
				cp->create(host);
				cp->subscribe(this);
			}
		}

		bool presentation::drawFrame(win32::direct2dContext& _ctx)
		{
			auto cp = current_page.lock();
			if (cp) {
				cp->draw();

				auto dc = _ctx.getDeviceContext();
				cp->render(dc);

				auto host = getHost();
				auto post = host->getWindowClientPos();

				double border_thickness = 4;

				linearGradientBrushRequest lgbr;
				lgbr.start.x = post.w;
				lgbr.start.y = post.h;
				lgbr.stop.x = 0;
				lgbr.stop.y = 0;
				lgbr.name = "presentation_shade";
				lgbr.gradientStops = {
					{ toColor("#F0F0F0FF"), 0.0 },
					{ toColor("#303030FF"), 0.10 },
					{ toColor("#404040FF"), 0.90 },
					{ toColor("#202020FF"), 0.95 },
					{ toColor("#A0A0A0FF"), 1.0 },
				};
				_ctx.setLinearGradientBrush(&lgbr);

				double inner_right = post.w - border_thickness * 2;
				double inner_bottom = post.h - border_thickness * 2;
				double left_side = border_thickness - 1;

				pathImmediateDto pathx;
				pathx.fillBrushName = "presentation_shade";
				pathx.borderBrushName = "presentation_shade";
				pathx.strokeWidth = 2;
				pathx.path.addLineTo(0, 0);
				pathx.path.addLineTo(left_side, 0);
				pathx.path.addLineTo(left_side, inner_bottom);
				pathx.path.addLineTo(inner_right, inner_bottom);
				pathx.path.addLineTo(inner_right, 0);
				pathx.path.addLineTo(post.w, 0);
				pathx.path.addLineTo(post.w, post.h);
				pathx.path.addLineTo(0, post.h);
				pathx.path.addLineTo(0, 0);
				pathx.closed = true;

				_ctx.drawPath(&pathx);

			}
			return false;
		}

		bool presentation::update(double _elapsedSeconds, double _totalSeconds)
		{
			auto cp = current_page.lock();
			if (cp) {
				cp->update(_elapsedSeconds, _totalSeconds );
			}
			return true;
		}

		void presentation::keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key)
		{
			auto cp = current_page.lock();
			key_down_event kde;
			kde.control_id = 0;
			kde.key = _key;
			if (cp) {
				cp->handle_key_down(0, kde);
			}
		}

		void presentation::keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key)
		{
			auto cp = current_page.lock();
			key_up_event kde;
			kde.control_id = 0;
			kde.key = _key;
			if (cp) {
				cp->handle_key_up(0, kde);
			}
		}

		void presentation::mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			auto cp = current_page.lock();
			mouse_move_event kde;
			kde.control_id = 0;
			kde.absolute_point = *_point;
			if (cp) {
				cp->handle_mouse_move(0, kde);
			}
			cp->root->set_mouse(*_point, nullptr, nullptr, nullptr, nullptr);
		}

		void presentation::mouseLeftDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			bool leftMouse = true;
			auto cp = current_page.lock();
			presentation* p = this;
			cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p,_point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				cp->handle_mouse_click(_item->id, mce);
				}, nullptr);
		}

		void presentation::mouseLeftUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			bool leftMouse = false;
			auto cp = current_page.lock();
			presentation* p = this;
			cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				cp->handle_mouse_click(_item->id, mce);
				mouse_right_click_event mcel;
				mcel.control = _item;
				mcel.control_id = _item->id;
				mcel.absolute_point.x = _point->x;
				mcel.absolute_point.y = _point->y;
				mcel.absolute_point.z = 0;
				cp->handle_mouse_right_click(_item->id, mcel);
				}, nullptr);
		}

		void presentation::mouseRightDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			bool rightMouse = true;
			auto cp = current_page.lock();
			presentation* p = this;
			cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr,[cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				cp->handle_mouse_click(_item->id, mce);
				});
		}

		void presentation::mouseRightUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			bool rightMouse = false;
			auto cp = current_page.lock();
			presentation* p = this;
			cp->root->set_mouse(*_point, &rightMouse, nullptr, nullptr, [cp, p, _point](control_base* _item) {
				mouse_click_event mce;
				mce.control = _item;
				mce.control_id = _item->id;
				mce.absolute_point.x = _point->x;
				mce.absolute_point.y = _point->y;
				mce.absolute_point.z = 0;
				cp->handle_mouse_click(_item->id, mce);
				mouse_right_click_event mcel;
				mcel.control = _item;
				mcel.control_id = _item->id;
				mcel.absolute_point.x = _point->x;
				mcel.absolute_point.y = _point->y;
				mcel.absolute_point.z = 0;
				cp->handle_mouse_right_click(_item->id, mcel);
				});
		}
		void presentation::pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color)
		{
			;
		}

		LRESULT presentation::ncHitTest(std::shared_ptr<win32::direct2dWindow>& win, point* _point)
		{
			LRESULT result = HTCLIENT;

			if (auto ppage = current_page.lock()) 
			{
				auto hitBox = ppage->root->find(*_point);
				if (hitBox) {
					result = hitBox->get_nchittest();
				}
			}

			return result;
		}

		void presentation::onCommand(int buttonId)
		{
			auto cp = current_page.lock();
			command_event ce;
			ce.control_id = buttonId;
			if (cp) {
				cp->handle_command(buttonId, ce);
			}
		}

		void presentation::onTextChanged(int textControlId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getEditText(textControlId);
				item_changed_event lce;
				lce.control_id = textControlId;
				lce.text_value = new_text;
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_item_changed(textControlId, lce);
				}
			}
		}

		void presentation::onDropDownChanged(int dropDownId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getComboSelectedText(dropDownId);
				int index = ptr->getComboSelectedIndex(dropDownId);
				int value = ptr->getComboSelectedValue(dropDownId);
				list_changed_event lce;
				lce.control_id = dropDownId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(dropDownId, lce);
				}
			}
		}

		void presentation::onListBoxChanged(int dropDownId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getListSelectedText(dropDownId);
				int index = ptr->getListSelectedIndex(dropDownId);
				int value = ptr->getListSelectedValue(dropDownId);
				list_changed_event lce;
				lce.control_id = dropDownId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(dropDownId, lce);
				}
			}
		}

		void presentation::onListViewChanged(int listViewId)
		{
			auto ptr = getHost();
			if (ptr) {
				std::string new_text = ptr->getListViewSelectedText(listViewId);
				int index = ptr->getListViewSelectedIndex(listViewId);
				int value = ptr->getListViewSelectedValue(listViewId);
				list_changed_event lce;
				lce.control_id = listViewId;
				lce.selected_text = new_text;
				lce.selected_value = value;
				lce.selected_index = index;
				lce.state = 0;
				lce.control = nullptr; // the page will assign this.
				auto cp = current_page.lock();
				if (cp) {
					cp->handle_list_changed(listViewId, lce);
				}
			}
		}

		int presentation::onHScroll(int controlId, win32::scrollTypes scrollType)
		{
			return 0;
		}

		int presentation::onVScroll(int controlId, win32::scrollTypes scrollType)
		{
			return 0;
		}

		int presentation::onResize(const rectangle& newSize, double d2dScale)
		{
			current_size = newSize;
			auto pg = current_page.lock();
			if (pg) {
				pg->arrange(newSize.w, newSize.h);
			}
			return 0;
		}

		int presentation::onSpin(int controlId, int newPosition)
		{
			int value = newPosition;
			return 0;
		}

	}
}
