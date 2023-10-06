module;

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <compare>
#include <functional>

export module corona:presentation;
import "corona-windows-all.h";
import :constants;
import :visual;
import :styles;
import :store_box;
import :layout_rect_box;
import :point_box;
import :rectangle_box;
import :controller;
import :direct2dcontext;
import :application_base;
import :direct2dwindow;
import :json;

export presentation_style_factory styles;

export class list_data
{
public:

	std::string id_field;
	std::string text_field;

	json_navigator items;
};

export class table_column
{
public:
	std::string display_name;
	std::string json_field;
	int width;
	visual_alignment alignment;
};

export class table_data
{
public:
	std::vector<table_column> columns;
	std::string id_field;
	json_navigator items;
};

export class id_counter
{
public:
	static int id;
	static int next();
	static int check(int _id);
	static int status_bar_id;
	static int status_text_title_id;
	static int status_text_subtitle_id;
};

export class layout_context
{
public:
	point flow_origin;
	point container_origin;
	point container_size;
	point remaining_size;
	point space_amount;
};

class container_control;
class row_layout;
class column_layout;
class absolute_layout;
class frame_layout;
class row_view_layout;
class column_view_layout;
class absolute_view_layout;

class presentation;
class page;

class title_control;
class subtitle_control;
class chaptertitle_control;
class chaptersubtitle_control;
class paragraph_control;
class code_control;
class label_control;
class image_control;

class static_control;
class pushbutton_control;
class pressbutton_control;
class checkbox_control;
class radiobutton_control;
class linkbutton_control;
class listbox_control;
class combobox_control;
class edit_control;
class scrollbar_control;
class listview_control;
class treeview_control;
class header_control;
class toolbar_control;
class statusbar_control;
class hotkey_control;
class animate_control;
class richedit_control;
class draglistbox_control;
class comboboxex_control;
class datetimepicker_control;
class monthcalendar_control;
class control_base;

class minimize_button_control;
class maximize_button_control;
class close_button_control;
class menu_button_control;

class camera_control;

using menu_click_handler = std::function<void(presentation* _presentation, std::weak_ptr<page> _page)>;

export class menu_item_navigate
{
public:
	int control_id;
	std::string target_page;
	menu_click_handler handler;

	menu_item_navigate();
	menu_item_navigate(int _source_control_id, std::string _target_page);
	menu_item_navigate(const menu_item_navigate& _src);
	menu_item_navigate operator =(const menu_item_navigate& _src);
	menu_item_navigate(menu_item_navigate&& _src);
	menu_item_navigate& operator =(menu_item_navigate&& _src);
	void operator()(presentation* _presentation, std::weak_ptr<page> _page);
	operator bool() { return control_id > 0; }
};

export class menu_item : public std::enable_shared_from_this<menu_item>
{
	menu_item* parent;
	HMENU to_menu_children(HMENU hmenu, int idx = 0);
	HMENU created_menu;

public:

	int	 id;
	bool is_separator;
	std::string name;
	std::vector<std::shared_ptr<menu_item>> children;
	menu_item_navigate navigate_handler;

	menu_item();
	menu_item(int _id, std::string _name = "Empty", std::function<void(menu_item& _item)> _settings = nullptr);
	virtual ~menu_item();

	menu_item& item(int _id, std::string _name, std::function<void(menu_item& _item)>  _settings = nullptr);
	menu_item& destination(int _id, std::string _name, std::string _destination_name, std::function<void(menu_item& _item)>  _settings = nullptr);
	menu_item& separator(int _id, std::function<void(menu_item& _item)>  _settings = nullptr);

	menu_item& begin_submenu(int _id, std::string _name, std::function<void(menu_item& _item)>  _settings = nullptr);
	menu_item& end();

	void subscribe(presentation* _presentation, std::weak_ptr<page> _page);

	HMENU to_menu();
};

export enum control_push_property
{
	cp_none = 0,
	cp_left_bounds = 1,
	cp_top_bounds = 2,
	cp_right_bounds = 4,
	cp_bottom_bounds = 8
};

export class control_push_request
{
public:
	int dest_control_id;
	int properties_to_push;
};

export template <typename T> class change_monitored_property
{
	T last_value;
	T current_value;
	bool is_changed;

public:

	change_monitored_property()
	{
		current_value = {};
		last_value = {};
		is_changed = false;
	}

	change_monitored_property(T _default_value)
	{
		current_value = _default_value;
		last_value = _default_value;
		is_changed = false;
	}

	virtual ~change_monitored_property()
	{
		;
	}

	operator T ()
	{
		return current_value;
	}

	T operator = (T _new_value)
	{
		is_changed = (_new_value != current_value);
		last_value = current_value;
		current_value = _new_value;
		return current_value;
	}

	void reset(T _value)
	{
		is_changed = false;
		last_value = current_value;
		current_value = _value;
		return current_value;
	}

	T& value()
	{
		return current_value;
	}

	bool changed_from(T _from_value)
	{
		bool really_changed = (is_changed &&
			_from_value == current_value);
		return really_changed;
	}

	bool changed_to(T _to_value)
	{
		bool really_changed = (is_changed &&
			_to_value == current_value);
		return really_changed;
	}

	bool changed(T _from_value, T _to_value)
	{
		bool really_changed = (is_changed &&
			_from_value == last_value &&
			_to_value == current_value);
		return really_changed;
	}
};

export class control_base
{
protected:

	point get_size(rectangle _ctx, point _remaining);
	point get_position(rectangle _ctx);
	double to_pixels(measure _margin);
	virtual point get_remaining(point _ctx);
	virtual void on_resize();
	void arrange_children(rectangle _bounds,
		std::function<point(const rectangle* _bounds, control_base*)> _initial_origin,
		std::function<point(point* _origin, const rectangle* _bounds, control_base*)> _next_origin);

	rectangle				bounds;
	rectangle				inner_bounds;
	point					margin_amount;
	point					padding_amount;

	change_monitored_property<bool> mouse_over;
	change_monitored_property<bool> mouse_left_down;
	change_monitored_property<bool> mouse_right_down;
	point		 mouse_relative_position;

public:

	friend class absolute_layout;
	friend class row_layout;
	friend class column_layout;
	friend class draw_control;

	int						id;

	layout_rect				box;
	measure					margin;
	measure					padding;
	std::string				tooltip_text;

	applicationBase* app;
	container_control* parent;

	std::vector<std::shared_ptr<control_base>> children;

	LRESULT hittest = HTCLIENT;

	virtual LRESULT get_nchittest() { return hittest; }
	virtual void set_nchittest(LRESULT _hittest) { hittest = _hittest; }

	control_base() :
		id(-1),
		margin(),
		parent(nullptr),
		margin_amount({ 0.0, 0.0 })
	{
		id = id_counter::next();
	}

	control_base(container_control* _parent, int _id) : control_base()
	{
		parent = _parent;

		if (_id < 0) {
			id = id_counter::next();
		}
		else {
			id = _id;
		}
	}

	std::weak_ptr<control_base> get_shared();
	std::vector<control_push_request> push_requests;

	rectangle& get_bounds() { return bounds; }
	rectangle& get_inner_bounds() { return inner_bounds; }

	rectangle& set_bounds(rectangle& _bounds);

	virtual void arrange(rectangle _ctx);
	bool contains(point pt);

	void push(int _destination_control_id, bool _push_left, bool _push_top, bool _push_right, bool _push_bottom);

	control_base* find(int _id);
	control_base* find(point p);
	control_base* get(control_base* _root, int _id);

	virtual double get_font_size() { return 12; }

	template <typename control_type> control_type& find(int _id)
	{
		control_base* temp = find(_id);
		control_type* citem = dynamic_cast<control_type>(temp);
		return *citem;
	}

	template <typename control_type> control_type& get(control_base* _root, int _id)
	{
		control_base* temp = get(_root, _id);
		control_type* citem = dynamic_cast<control_type>(temp);
		return *citem;
	}

	void foreach(std::function<void(control_base* _root)> _item);
	control_base* find_if(std::function<bool(control_base* _root)> _item);

	virtual void create(std::weak_ptr<applicationBase> _host);
	virtual void destroy();
	virtual void draw();
	virtual void render(ID2D1DeviceContext* _dest);

	control_base& set_origin(measure _x, measure _y)
	{
		box.x = _x;
		box.y = _y;
		return *this;
	}

	control_base& set_size(measure _width, measure _height)
	{
		box.width = _width;
		box.height = _height;
		return *this;
	}

	control_base& set_position(layout_rect _new_layout)
	{
		box = _new_layout;
		return *this;
	}

	control_base& set_margin(measure _space)
	{
		margin = _space;
		return *this;
	}

	template <typename control_class> control_class& set_spacing(measure _spacing)
	{
		control_class* r = dynamic_cast<control_class>(this);
		r->margin = _spacing;
		return *this;
	}

	virtual void apply(control_base& _ref);
	virtual bool set_mouse(point _position,
		bool* _left_down,
		bool* _right_down,
		std::function<void(control_base* _item)> _left_click,
		std::function<void(control_base* _item)> _right_click
	);

	virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
};

export class draw_control : public control_base
{
public:

	HBRUSH background_brush_win32;
	solidBrushRequest	background_brush;

	std::weak_ptr<applicationBase> host;
	std::weak_ptr<direct2dChildWindow> window;
	std::function<void(draw_control*)> on_draw;
	std::function<void(draw_control*)> on_create;

	draw_control();
	draw_control(container_control* _parent, int _id);
	virtual void create(std::weak_ptr<applicationBase> _host);
	virtual void destroy();
	virtual void draw();
	virtual void render(ID2D1DeviceContext *_dest);
	virtual void on_resize();
};


class camera_control : public draw_control
{
	void init();
	solidBrushRequest	border_brush;
public:
	camera_control(container_control* _parent, int _id);
	virtual ~camera_control();
};

class grid_control : public draw_control
{
	void init();
	solidBrushRequest	border_brush;
public:
	grid_control(container_control* _parent, int _id);
	virtual ~grid_control();
};

const int IDC_CHART_BASE = 1000000;
const int IDC_CHART_PROGRAM = 1 + IDC_CHART_BASE;
const int IDC_CHART_BAR = 2 + IDC_CHART_BASE;
const int IDC_CHART_LINE = 3 + IDC_CHART_BASE;
const int IDC_CHART_PIE = 4 + IDC_CHART_BASE;
const int IDC_CHART_SCATTER = 5 + IDC_CHART_BASE;
const int IDC_CHART_BUBBLE = 6 + IDC_CHART_BASE;
const int IDC_CHART_TREE_BOX = 7 + IDC_CHART_BASE;
const int IDC_CHART_TREE_MAP = 8 + IDC_CHART_BASE;

export class chart_control : public draw_control
{
	void init();
	solidBrushRequest	border_brush;

	/*

	The big thing for the design here is to just be the mainframe sleazes when it comes to handling loads of options.
	For the type of chart, you can make in the menu into it.

	*/

public:

	int chart_type;
	table_data  data;
	std::string color_series;
	std::string sseries1;
	std::string sseries2;
	std::string sseries3;
	std::string sseries4;

	chart_control(container_control* _parent, int _id);
	virtual ~chart_control();
};


class slide_control : public draw_control
{
	void init();
	solidBrushRequest	border_brush;

public:

	int source_object_id;

	slide_control(container_control* _parent, int _id);
	virtual ~slide_control();
};


export class container_control : public draw_control
{

public:

	layout_rect				item_box = {};
	measure					item_margin = {};

	visual_alignment		content_alignment = visual_alignment::align_near;
	visual_alignment		content_cross_alignment = visual_alignment::align_near;

	container_control();
	container_control(container_control* _parent, int _id);
	virtual ~container_control() { ; }

	virtual void apply(control_base& _ref);

	template <typename control_type> control_type& create(int _id)
	{
		std::shared_ptr<control_type> temp = std::make_shared<control_type>(this, _id);
		children.push_back(temp);
		std::cout << " " << typeid(*this).name() << " ->create:" << typeid(control_type).name() << std::endl;
		return *temp.get();
	}

	container_control& set_item_origin(measure _x, measure _y);
	container_control& set_item_size(measure _width, measure _height);
	container_control& set_item_position(layout_rect _new_layout);
	container_control& set_item_margin(measure _item_margin);

	container_control& set_content_align(visual_alignment _new_alignment);
	container_control& set_content_cross_align(visual_alignment _new_alignment);

	container_control& set_origin(measure _x, measure _y);
	container_control& set_size(measure _width, measure _height);
	container_control& set_position(layout_rect _new_layout);
	container_control& set_margin(measure _space);

	container_control& set_background_color(solidBrushRequest _brushFill);
	container_control& set_background_color(std::string _color);

	row_layout& row_begin(int id = id_counter::next(), std::function<void(row_layout&)> _settings = nullptr);
	column_layout& column_begin(int id = id_counter::next(), std::function<void(column_layout&)> _settings = nullptr);
	absolute_layout& absolute_begin(int id = id_counter::next(), std::function<void(absolute_layout&)> _settings = nullptr);

	row_view_layout& row_view_begin(int id = id_counter::next(), std::function<void(row_view_layout&)> _settings = nullptr);
	column_view_layout& column_view_begin(int id = id_counter::next(), std::function<void(column_view_layout&)> _settings = nullptr);
	absolute_view_layout& absolute_view_begin(int id = id_counter::next(), std::function<void(absolute_view_layout&)> _settings = nullptr);

	frame_layout& frame_begin(int id = id_counter::next(), std::function<void(frame_layout&)> _settings = nullptr);

	row_layout& row_begin(std::function<void(row_layout&)> _settings) { return row_begin(id_counter::next(), _settings); }
	column_layout& column_begin(std::function<void(column_layout&)> _settings) { return column_begin(id_counter::next(), _settings); }
	absolute_layout& absolute_begin(std::function<void(absolute_layout&)> _settings) { return absolute_begin(id_counter::next(), _settings); }
	frame_layout& frame_begin(std::function<void(frame_layout&)> _settings) { return frame_begin(id_counter::next(), _settings); }

	container_control& end();

	container_control& title(std::string _text, std::function<void(title_control&)> _settings, int _id);
	container_control& subtitle(std::string _text, std::function<void(subtitle_control&)> _settings, int _id);
	container_control& chaptertitle(std::string _text, std::function<void(chaptertitle_control&)> _settings, int _id);
	container_control& chaptersubtitle(std::string _text, std::function<void(chaptersubtitle_control&)> _settings, int _id);
	container_control& paragraph(std::string _text, std::function<void(paragraph_control&)> _settings, int _id);
	container_control& code(std::string _text, std::function<void(code_control&)> _settings, int _id);
	container_control& label(std::string _text, std::function<void(label_control&)> _settings, int _id);

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

	container_control& image(int _id, int _control_id, std::function<void(image_control&)> _settings = nullptr);
	container_control& image(int _id, std::string _filename, std::function<void(image_control&)> _settings = nullptr);
	container_control& image(std::string _filename, std::function<void(image_control&)> _settings = nullptr);

	container_control& push_button(int _id, std::string text, std::function<void(pushbutton_control&)> _settings = nullptr);
	container_control& radio_button(int _id, std::string text, std::function<void(radiobutton_control&)> _settings = nullptr);
	container_control& checkbox(int _id, std::string text, std::function<void(checkbox_control&)> _settings = nullptr);
	container_control& press_button(int _id, std::string text, std::function<void(pressbutton_control&)> _settings = nullptr);

	container_control& listbox(int _id, std::function<void(listbox_control&)> _settings = nullptr);
	container_control& combobox(int _id, std::function<void(combobox_control&)> _settings = nullptr);
	container_control& edit(int _id, std::function<void(edit_control&)> _settings = nullptr);
	container_control& scrollbar(int _id, std::function<void(scrollbar_control&)> _settings = nullptr);

	container_control& listview(int _id, std::function<void(listview_control&)> _settings = nullptr);
	container_control& treeview(int _id, std::function<void(treeview_control&)> _settings = nullptr);
	container_control& header(int _id, std::function<void(header_control&)> _settings = nullptr);
	container_control& toolbar(int _id, std::function<void(toolbar_control&)> _settings = nullptr);
	container_control& statusbar(int _id, std::function<void(statusbar_control&)> _settings = nullptr);
	container_control& hotkey(int _id, std::function<void(hotkey_control&)> _settings = nullptr);
	container_control& animate(int _id, std::function<void(animate_control&)> _settings = nullptr);
	container_control& richedit(int _id, std::function<void(richedit_control&)> _settings = nullptr);
	container_control& draglistbox(int _id, std::function<void(draglistbox_control&)> _settings = nullptr);
	container_control& comboboxex(int _id, std::function<void(comboboxex_control&)> _settings = nullptr);
	container_control& datetimepicker(int _id, std::function<void(datetimepicker_control&)> _settings = nullptr);
	container_control& monthcalendar(int _id, std::function<void(monthcalendar_control&)> _settings = nullptr);

	container_control& listbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listbox_control&)> _settings = nullptr);
	container_control& combobox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(combobox_control&)> _settings = nullptr);
	container_control& edit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(edit_control&)> _settings = nullptr);
	container_control& listview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listview_control&)> _settings = nullptr);
	container_control& treeview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(treeview_control&)> _settings = nullptr);
	container_control& header_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(header_control&)> _settings = nullptr);
	container_control& toolbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(toolbar_control&)> _settings = nullptr);
	container_control& statusbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(statusbar_control&)> _settings = nullptr);
	container_control& hotkey_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(hotkey_control&)> _settings = nullptr);
	container_control& animate_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(animate_control&)> _settings = nullptr);
	container_control& richedit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(richedit_control&)> _settings = nullptr);
	container_control& draglistbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(draglistbox_control&)> _settings = nullptr);
	container_control& comboboxex_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(comboboxex_control&)> _settings = nullptr);
	container_control& datetimepicker_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(datetimepicker_control&)> _settings = nullptr);
	container_control& monthcalendar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(monthcalendar_control&)> _settings = nullptr);
	container_control& minimize_button(std::function<void(minimize_button_control&)> _settings = nullptr);
	container_control& maximize_button(std::function<void(maximize_button_control&)> _settings = nullptr);
	container_control& close_button(std::function<void(close_button_control&)> _settings = nullptr);
	container_control& menu_button(int _id, std::function<void(menu_button_control&)> _settings = nullptr);

	container_control& camera(int _id, std::function<void(camera_control&)> _settings = nullptr);
	container_control& grid(int _id, std::function<void(grid_control&)> _settings = nullptr);
	container_control& chart(int _id, std::function<void(chart_control&)> _settings = nullptr);
	container_control& slide(int _id, std::function<void(slide_control&)> _settings = nullptr);

	container_control& caption_bar(
		presentation_style& st,
		int	title_bar_id,
		int menu_button_id,
		menu_item& menu,
		int image_control_id,
		std::string image_file,
		std::string corporate_name,
		int id_title_column_id,
		std::string title_name,
		std::string subtitle_name
	);

	container_control& form_single_column(int _align_id,
		std::string _form_name,
		std::function<void(container_control& _settings)> _add_controls
	);

	container_control& form_double_column(int _align_id,
		std::string _form_name,
		std::function<void(container_control& _settings)> _add_controls1,
		std::function<void(container_control& _settings)> _add_controls2
	);

	container_control& status_bar(presentation_style& st);

};

export class text_display_control : public draw_control
{
public:
	std::string			text;
	solidBrushRequest	text_fill_brush;
	textStyleRequest	text_style;

	text_display_control();
	text_display_control(container_control* _parent, int _id);

	void init();
	virtual double get_font_size() { return text_style.fontSize; }
	text_display_control& set_text(std::string _text);
	text_display_control& set_text_fill(solidBrushRequest _brushFill);
	text_display_control& set_text_fill(std::string _color);
	text_display_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
	text_display_control& set_text_style(textStyleRequest request);

};

export class gradient_button_control : public draw_control
{
public:

	linearGradientBrushRequest buttonFaceNormal;
	linearGradientBrushRequest buttonFaceDown;
	linearGradientBrushRequest buttonFaceOver;
	radialGradientBrushRequest buttonBackLight;

	solidBrushRequest foregroundNormal;
	solidBrushRequest foregroundOver;
	solidBrushRequest foregroundDown;

	gradient_button_control(container_control* _parent, int _id, std::string _name);
	virtual ~gradient_button_control();

	virtual void arrange(rectangle _ctx);
	virtual void draw_button(std::function<void(rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape);

};

export class minimize_button_control : public gradient_button_control
{
public:

	minimize_button_control(container_control* _parent, int _id);

	virtual ~minimize_button_control();
	virtual LRESULT get_nchittest() {
		return HTCLIENT; // we lie here 
	}

	virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
};

export class maximize_button_control : public gradient_button_control
{
public:

	maximize_button_control(container_control* _parent, int _id);

	virtual ~maximize_button_control();
	virtual LRESULT get_nchittest() {
		return HTCLIENT;// we lie here 
	}

	virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
};

export class close_button_control : public gradient_button_control
{
public:

	close_button_control(container_control* _parent, int _id);

	virtual ~close_button_control();
	virtual LRESULT get_nchittest() {
		return HTCLIENT;// we lie here 
	}

	virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
};

export class menu_button_control : public gradient_button_control
{
public:

	menu_item menu;

	menu_button_control(container_control* _parent, int _id);
	virtual ~menu_button_control() { ; }

	virtual void on_subscribe(presentation* _presentation, std::weak_ptr<page> _page);
};

export class title_control : public text_display_control
{
	void set_default_styles();
public:
	title_control();
	title_control(container_control* _parent, int _id);
	virtual ~title_control();
};

export class subtitle_control : public text_display_control
{
	void set_default_styles();
public:
	subtitle_control();
	subtitle_control(container_control* _parent, int _id);
	virtual ~subtitle_control();
};

export class chaptertitle_control : public text_display_control
{
	void set_default_styles();
public:
	chaptertitle_control();
	chaptertitle_control(container_control* _parent, int _id);
	virtual ~chaptertitle_control();
};

export class chaptersubtitle_control : public text_display_control
{
	void set_default_styles();
public:
	chaptersubtitle_control();
	chaptersubtitle_control(container_control* _parent, int _id);
	virtual ~chaptersubtitle_control();
};

export class paragraph_control : public text_display_control
{
	void set_default_styles();
public:
	paragraph_control();
	paragraph_control(container_control* _parent, int _id);
	virtual ~paragraph_control();
};

export class code_control : public text_display_control
{
	void set_default_styles();
public:
	code_control();
	code_control(container_control* _parent, int _id);
	virtual ~code_control();
};

export class label_control : public text_display_control
{
	void set_default_styles();
public:
	label_control();
	label_control(container_control* _parent, int _id);
	virtual ~label_control();
};

export class placeholder_control : public text_display_control
{
	void set_default_styles();
public:
	placeholder_control();
	placeholder_control(container_control* _parent, int _id);
	virtual ~placeholder_control();
};

export class image_control :
	public draw_control
{

	enum image_modes {
		no_image,
		use_file_name,
		use_control_id,
		use_resource_id
	};

	bitmapInstanceDto instance;

	image_modes		image_mode;

	std::string		image_file_name;
	int				image_control_id;
	DWORD			image_resource_id;
	std::string		image_name;

	void init();

public:
	image_control();
	image_control(container_control* _parent, int _id);
	image_control(container_control* _parent, int _id, std::string _file_name);
	image_control(container_control* _parent, int _id, int _source_control_id);
	virtual ~image_control();

	void load_from_file(std::string _name);
	void load_from_resource(DWORD _resource_id);
	void load_from_control(int _control_id);
};

export class absolute_layout :
	public container_control
{
public:
	absolute_layout() { ; }
	absolute_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
	virtual ~absolute_layout() { ; }

	virtual void arrange(rectangle _ctx);
};

export class column_layout :
	public container_control
{
	layout_rect item_size;
public:
	column_layout() { ; }
	column_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
	virtual ~column_layout() { ; }

	virtual void arrange(rectangle _ctx);
	virtual point get_remaining(point _ctx);
};

export class row_layout :
	public container_control
{
protected:
public:
	row_layout() { ; }
	row_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
	virtual ~row_layout() { ; }

	virtual void arrange(rectangle _ctx);
	virtual point get_remaining(point _ctx);
};

export class data_scroller
{
	table_data data;
	control_base* parent;

	void update_children(control_base* _target);
	std::map<int, bool> selected_indeces;

	int current_scroll_location;
	int current_scroll_page;
	int current_item;
	double page_size;
	bool size_valid;

public:

	std::function<control_base* (table_data& td, int index, bool selected, bool navigated)> mapper;

	data_scroller(control_base* _parent) : parent(_parent)
	{
		size_valid = false;
	}

	data_scroller(control_base* _parent, table_data& td) : parent(_parent)
	{
		data = td;
		data.items.size();
		size_valid = false;
	}

	data_scroller operator = (table_data& td)
	{
		data = td;
		size_valid = false;
		return *this;
	}

	void generate_items(rectangle _ctx)
	{
		// loop through the items and call the mapper and put the data into the children
		// parent->children.clear();

		// we don't need to do this on every frame, so
		if (!size_valid) {

			size_valid = true;
		}
	}

	void line_down()
	{
		size_valid = false;
	}

	void line_up()
	{
		size_valid = false;
	}

	void page_up()
	{
		size_valid = false;
	}

	void page_down()
	{
		size_valid = false;
	}

	void home()
	{
		size_valid = false;
	}

	void end()
	{
		size_valid = false;
	}

};

export class column_view_layout :
	public column_layout, public data_scroller
{
public:
	column_view_layout() : data_scroller(this) {
		;
	}
	column_view_layout(container_control* _parent, int _id) : data_scroller(this), column_layout(_parent, _id) { ; }
	virtual ~column_view_layout() { ; }

	virtual void arrange(rectangle _ctx)
	{
		generate_items(_ctx);
		column_layout::arrange(_ctx);
	}
};

export class row_view_layout :
	public row_layout, public data_scroller
{
protected:
public:
	row_view_layout() : data_scroller(this) { ; }
	row_view_layout(container_control* _parent, int _id) : data_scroller(this), row_layout(_parent, _id) { ; }
	virtual ~row_view_layout() { ; }

	virtual void arrange(rectangle _ctx)
	{
		generate_items(_ctx);
		row_layout::arrange(_ctx);
	}
};

export class absolute_view_layout :
	public absolute_layout, public data_scroller
{
protected:
public:
	absolute_view_layout() : data_scroller(this) { ; }
	absolute_view_layout(container_control* _parent, int _id) : data_scroller(this), absolute_layout(_parent, _id) { ; }
	virtual ~absolute_view_layout() { ; }

	virtual void arrange(rectangle _ctx)
	{
		generate_items(_ctx);
		absolute_layout::arrange(_ctx);
	}
};

export class frame_layout :
	public container_control
{
protected:
	std::string selected_page_name;

public:
	frame_layout() { ; }
	frame_layout(container_control* _parent, int _id) : container_control(_parent, _id) { ; }
	virtual ~frame_layout() { ; }

	virtual void set_page(page& _page);
};

export class banner_control :
	public row_layout
{
protected:
	std::string logo_filename;
public:
	banner_control() { ; }
	banner_control(container_control* _parent, int _id) : row_layout(_parent, _id) { ; }
	virtual ~banner_control() { ; }

	virtual void arrange(rectangle _ctx);
	virtual point get_remaining(point _ctx);
};

export class windows_control : public control_base
{
protected:

	void set_default_styles()
	{
		text_style = {};
		text_style.name = "windows_control_style";
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

	HFONT text_font;
	HWND window;

public:


	using control_base::id;

	std::weak_ptr<applicationBase> window_host;
	textStyleRequest	text_style;

	windows_control()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(1.0_container, 1.2_fontgr);
		set_default_styles();
	}

	windows_control(container_control* _parent, int _id) : control_base(_parent, _id)
	{
		set_origin(0.0_px, 0.0_px);
		set_size(1.0_container, 1.2_fontgr);
		set_default_styles();
	}

	virtual const wchar_t* get_window_class() = 0;
	virtual DWORD get_window_style() = 0;
	virtual DWORD get_window_ex_style() = 0;

	virtual double get_font_size() { return text_style.fontSize; }

	virtual void on_resize()
	{
		if (auto phost = window_host.lock()) {
			HWND parent = phost->getMainWindow();

			auto boundsPixels = phost->toPixelsFromDips(inner_bounds);
			RECT r;
			r.left = boundsPixels.x;
			r.top = boundsPixels.y;
			r.right = boundsPixels.x + boundsPixels.w;
			r.bottom = boundsPixels.y + boundsPixels.h;

			if (window != nullptr) {
				MoveWindow(window, r.left, r.top, r.right, r.bottom, TRUE);
			}
		}
	}

	virtual void create(std::weak_ptr<applicationBase> _host)
	{
		window_host = _host;

		if (auto phost = window_host.lock()) {
			auto boundsPixels = phost->toPixelsFromDips(inner_bounds);

			if (window == nullptr) {
				HWND parent = phost->getMainWindow();
				window = CreateWindowEx(get_window_ex_style(), get_window_class(), L"", get_window_style(), boundsPixels.x, boundsPixels.y, boundsPixels.w, boundsPixels.h, parent, (HMENU)id, NULL, NULL);
				text_font = phost->createFontDips(window, text_style.fontName, text_style.fontSize, text_style.bold, text_style.italics);
				SendMessage(window, WM_SETFONT, (WPARAM)text_font, 0);
				HWND tooltip = phost->getTooltipWindow();
				if (tooltip && tooltip_text.size() > 0) {
					TOOLINFOA toolInfo = { 0 };
					toolInfo.cbSize = sizeof(toolInfo);
					toolInfo.hwnd = parent;
					toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
					toolInfo.uId = (UINT_PTR)window;
					toolInfo.lpszText = (LPSTR)tooltip_text.c_str();
					SendMessageA(tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
				}
				on_create();
			}
		}
	}

	virtual void on_create() { ; }

	virtual void destroy()
	{
		if (::IsWindow(window)) {
			DestroyWindow(window);
		}
	}

	virtual ~windows_control()
	{
		destroy();
	}
};


export class text_control_base : public windows_control
{
	std::string text;

public:

	using control_base::id;
	using windows_control::window_host;

	text_control_base()
	{
		;
	}

	text_control_base(container_control* _parent, int _id) : windows_control(_parent, _id)
	{
		;
	}

	virtual ~text_control_base() { ; }

	void set_text(const std::string& _text)
	{
		text = _text;
		if (auto phost = window_host.lock()) {
			phost->setEditText(id, _text);
		}
	}

	std::string get_text()
	{
		if (auto phost = window_host.lock()) {
			text = phost->getEditText(id);
		}
		return text;
	}

	virtual void create(std::weak_ptr<applicationBase> _host)
	{
		windows_control::create(_host);
		if (auto phost = window_host.lock()) {
			phost->setEditText(id, text);
		}
	}

};

export class mini_table
{
	std::shared_ptr<dynamic_box> box;
	std::vector<relative_ptr_type> indeces;
	int row_size;
	int max_rows;
public:

	mini_table()
	{
		row_size = 1;
		max_rows = 1;
		box = std::make_shared<dynamic_box>();
	}

	void init(int _row_size, int _max_rows)
	{
		max_rows = _max_rows;
		row_size = _row_size;
		indeces.clear();
		int c = (max_rows + 1) * row_size;
		if (indeces.size() < max_rows)
		{
			indeces.resize(c);
		}
		int b = c * 100;
		box->init(b);
	}

	char* set(int i, int j, std::string& src)
	{
		int idx = j * row_size + i;

		indeces[idx] = box->put_null_terminated(src.c_str(), 0);
		char* temp = box->get_object<char>(indeces[idx]);
		return temp;
	}
};

export class table_control_base : public windows_control
{
	mini_table mtable;
	char blank[256] = { 0 };

	void data_changed()
	{
		strcpy_s(blank, "N/A");
		if (auto phost = window_host.lock())
		{
			phost->clearListView(id);
			int row_size = choices.columns.size();
			int num_rows = choices.items.size() * 2;
			mtable.init(row_size, num_rows);
			std::map<std::string, int> column_map;
			int row_index = 0;
			int col_index = 0;
			for (auto col : choices.columns) {
				char* t = mtable.set(col_index, row_index, col.display_name);
				phost->addListViewColumn(id, col_index, t, col.width, col.alignment);
				column_map[col.json_field] = col_index;
				col_index++;
			}
			std::vector<char*> data_row;
			data_row.resize(choices.columns.size());
			row_index++;
			
			for (int i = 0; i < choices.items.size(); i++)
			{
				auto item = choices.items[i];
				col_index = 0;
				for (auto col : choices.columns)
				{
					data_row[col_index] = blank;
					bool has_field = item.has_member(col.json_field);
					if (has_field) {
						std::string item_value = item[col.json_field];
						char* value = mtable.set(col_index, row_index, item_value);
						if (value) {
							data_row[col_index] = value;
						}
					}
					col_index++;
				}
				//virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<std::string>&_items) = 0;
				phost->addListViewRow(id, row_index, data_row);
				row_index++;
			}
		}
	}

public:

	using control_base::id;
	using windows_control::window_host;
	table_data choices;

	table_control_base()
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 10.0_fontgr);
	}

	table_control_base(container_control* _parent, int _id) : windows_control(_parent, _id)
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 10.0_fontgr);
	}

	virtual ~table_control_base() { ; }

	virtual void on_create()
	{
		ListView_SetExtendedListViewStyle(window, LVS_EX_FULLROWSELECT);
		data_changed();
	}

	void set_table(table_data& _choices)
	{
		choices = _choices;
		data_changed();
	}
};

export class list_control_base : public windows_control
{
public:
	using control_base::id;
	using windows_control::window_host;
	list_data choices;

	list_control_base()
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 10.0_fontgr);
	}

	list_control_base(container_control* _parent, int _id) : windows_control(_parent, _id)
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 10.0_fontgr);
	}

	virtual ~list_control_base() { ; }

	void data_changed()
	{
		if (auto phost = window_host.lock()) {
			phost->clearListItems(id);
			for (int i = 0; i < choices.items.size(); i++)
			{
				auto c = choices.items[i];
				int lid = c[choices.id_field];
				std::string description = c[choices.text_field];
				phost->addListItem(id, description, lid);
			}
		}
	}

	void set_list(list_data& _choices)
	{
		choices = _choices;
		data_changed();
	}

	virtual void on_create()
	{
		data_changed();
	}

};

export class dropdown_control_base : public windows_control
{
public:

	using control_base::id;
	using windows_control::window_host;
	list_data choices;

	dropdown_control_base()
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 2.0_fontgr);
	}

	dropdown_control_base(container_control* _parent, int _id) : windows_control(_parent, _id)
	{
		control_base::set_origin(0.0_px, 0.0_px);
		control_base::set_size(1.0_container, 2.0_fontgr);
	}

	virtual ~dropdown_control_base() { ; }

	void data_changed()
	{
		if (auto phost = window_host.lock()) {
			phost->clearComboItems(id);
			for (int i = 0; i < choices.items.size(); i++)
			{
				auto element = choices.items[i];
				int lid = element[choices.id_field];
				std::string description = element[choices.text_field];
				phost->addComboItem(id, description, lid);
			}
		}
	}

	void set_list(list_data& _choices)
	{
		choices = _choices;
		data_changed();
	}

	virtual void on_resize()
	{
		if (auto phost = window_host.lock()) {
			auto boundsPixels = phost->toPixelsFromDips(control_base::get_inner_bounds());

			if (windows_control::window != nullptr) {
				int h = windows_control::text_style.fontSize * 8;
				::MoveWindow(windows_control::window,  boundsPixels.x, boundsPixels.y, boundsPixels.w, h, TRUE);
			}
		}
	}

	virtual void on_create()
	{
		on_resize();
		data_changed();
	}

};

const int DefaultWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
const int DisplayOnlyWindowStyles = WS_VISIBLE | WS_CHILD;
const int EditWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
const int RichEditWindowStyles = WS_VISIBLE | WS_BORDER | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
const int ComboWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
const int ComboExWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT;
const int PushButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT;
const int PressButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_FLAT | BS_AUTOCHECKBOX | BS_PUSHLIKE;
const int CheckboxWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | BS_FLAT;
const int RadioButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_FLAT;
const int LinkButtonWindowStyles = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_COMMANDLINK | BS_FLAT;
const int ListViewWindowsStyles = DefaultWindowStyles | LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL;
const int ListBoxWindowsStyles = DefaultWindowStyles | WS_BORDER | WS_VSCROLL;

export class static_control : public text_control_base
{
public:
	static_control(container_control* _parent, int _id) : text_control_base(_parent, _id) { ; }
	virtual ~static_control() { ; }

	virtual const wchar_t *get_window_class() { return WC_STATIC; }
	virtual DWORD get_window_style() { return DisplayOnlyWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }
};

export template <long ButtonWindowStyles> class button_control : public text_control_base
{
	using control_base::id;
	using windows_control::window_host;
	std::string caption_text;
	long caption_icon_id;
	HICON caption_icon;
public:
	button_control(container_control* _parent, int _id) : text_control_base(_parent, _id) { ; }
	virtual ~button_control() { ; }
	virtual const wchar_t* get_window_class() { return WC_BUTTON; }
	virtual DWORD get_window_style() { return ButtonWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }
};

export class pushbutton_control : public button_control<PushButtonWindowStyles>
{
public:
	pushbutton_control(container_control* _parent, int _id) : button_control<PushButtonWindowStyles>(_parent, _id) { ; }
	virtual ~pushbutton_control() { ; }
};

export class pressbutton_control : public button_control<PressButtonWindowStyles>
{
public:
	pressbutton_control(container_control* _parent, int _id) : button_control<PressButtonWindowStyles>(_parent, _id) { ; }
	virtual ~pressbutton_control() { ; }
};

export class radiobutton_control : public button_control<RadioButtonWindowStyles>
{
public:
	radiobutton_control(container_control* _parent, int _id) : button_control<RadioButtonWindowStyles>(_parent, _id) { ; }
	virtual ~radiobutton_control() { ; }
};

export class checkbox_control : public button_control<CheckboxWindowStyles>
{
public:
	checkbox_control(container_control* _parent, int _id) : button_control<CheckboxWindowStyles>(_parent, _id) { ; }
	virtual ~checkbox_control() { ; }
};

export class linkbutton_control : public button_control<LinkButtonWindowStyles>
{
public:
	linkbutton_control(container_control* _parent, int _id) : button_control<LinkButtonWindowStyles>(_parent, _id) { ; }
	virtual ~linkbutton_control() { ; }
};

export class edit_control : public text_control_base
{
public:
	edit_control(container_control* _parent, int _id) : text_control_base(_parent, _id) { ; }
	virtual ~edit_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_EDIT; }
	virtual DWORD get_window_style() { return EditWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class listbox_control : public list_control_base
{
public:
	listbox_control(container_control* _parent, int _id) : list_control_base(_parent, _id) { ; }
	virtual ~listbox_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_LISTBOX; }
	virtual DWORD get_window_style() { return ListBoxWindowsStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class combobox_control : public dropdown_control_base
{
public:
	combobox_control(container_control* _parent, int _id) : dropdown_control_base(_parent, _id) { ; }
	virtual ~combobox_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_COMBOBOX; }
	virtual DWORD get_window_style() { return ComboWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class comboboxex_control : public windows_control
{
public:
	using control_base::id;
	using windows_control::window_host;
	list_data choices;

	comboboxex_control();
	comboboxex_control(container_control* _parent, int _id);
	virtual ~comboboxex_control() { ; }
	void data_changed();
	void set_list(list_data& _choices);
	virtual void on_create();
	virtual void on_resize();

	virtual const wchar_t* get_window_class() { return WC_COMBOBOXEX; }
	virtual DWORD get_window_style() { return ComboWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class listview_control : public table_control_base
{
public:
	listview_control(container_control* _parent, int _id) : table_control_base(_parent, _id) { ; }
	virtual ~listview_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_LISTVIEW; }
	virtual DWORD get_window_style() { return ListViewWindowsStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class scrollbar_control : public windows_control
{
public:
	scrollbar_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~scrollbar_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_SCROLLBAR; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class richedit_control : public text_control_base
{
public:
	void set_html(const std::string& _text);
	std::string get_html();

	richedit_control(container_control* _parent, int _id) : text_control_base(_parent, _id) {
		LoadLibrary(TEXT("Msftedit.dll"));
	}
	virtual ~richedit_control() { ; }

	virtual const wchar_t* get_window_class() { return MSFTEDIT_CLASS; }
	virtual DWORD get_window_style() { return RichEditWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class datetimepicker_control : public windows_control
{
public:
	void set_text(const std::string& _text);
	std::string get_text();

	datetimepicker_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~datetimepicker_control() { ; }

	virtual const wchar_t* get_window_class() { return DATETIMEPICK_CLASS; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class monthcalendar_control : public windows_control
{
public:
	monthcalendar_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~monthcalendar_control() { ; }

	virtual const wchar_t* get_window_class() { return MONTHCAL_CLASS; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class animate_control : public windows_control
{
public:
	animate_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~animate_control() { ; }

	bool open(const std::string& _name);
	bool open(DWORD resource_id);
	bool play(UINT from, UINT to, UINT rep);
	bool play();
	bool stop();

	virtual const wchar_t* get_window_class() { return ANIMATE_CLASS; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }
};

export class treeview_control : public windows_control
{
public:
	treeview_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~treeview_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_TREEVIEW; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class header_control : public windows_control
{
public:
	header_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~header_control() { ; }

	virtual const wchar_t* get_window_class() { return WC_HEADER; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class toolbar_control : public windows_control
{
public:
	toolbar_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~toolbar_control() { ; }
	
	virtual const wchar_t* get_window_class() { return TOOLBARCLASSNAME; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class statusbar_control : public windows_control
{
public:
	statusbar_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~statusbar_control() { ; }

	virtual const wchar_t* get_window_class() { return STATUSCLASSNAME; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }

};

export class hotkey_control : public windows_control
{
public:
	hotkey_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~hotkey_control() { ; }

	virtual const wchar_t* get_window_class() { return TOOLBARCLASSNAME; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }
};

export class draglistbox_control : public windows_control
{
public:
	draglistbox_control(container_control* _parent, int _id) : windows_control(_parent, _id) { ; }
	virtual ~draglistbox_control() { ; }

	virtual const wchar_t* get_window_class() { return HOTKEY_CLASS; }
	virtual DWORD get_window_style() { return DefaultWindowStyles; }
	virtual DWORD get_window_ex_style() { return 0; }
};

export enum class field_layout
{
	label_on_left = 1,
	label_on_top = 2
};

export class control_event
{
public:
	int control_id;
	control_base* control;
};

export class command_event : public control_event
{
public:
};

export class mouse_event : public control_event
{
public:
	point absolute_point;
	point relative_point;
};

export class mouse_move_event : public mouse_event
{

};

export class mouse_click_event : public mouse_event
{

};

export class mouse_left_click_event : public mouse_event
{

};

export class mouse_right_click_event : public mouse_event
{

};

export class key_event : public control_event
{
public:
	short key;
};

export class key_down_event : public key_event
{
public:

};

export class key_up_event : public key_event
{
public:

};

export class draw_event : public control_event
{
public:
	int state;
};

export class item_changed_event : public control_event
{
public:
	std::string text_value;
};

export class list_changed_event : public control_event
{
public:
	int state;

	std::string selected_text;
	int selected_index;
	relative_ptr_type selected_value;
};

export class key_up_event_binding
{
public:
	int subscribed_item_id;
	std::function< void(key_up_event) > on_key_up;
};

export class key_down_event_binding
{
public:
	int subscribed_item_id;
	std::function< void(key_down_event) > on_key_down;
};

export class mouse_move_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(mouse_move_event) > on_mouse_move;
};

export class mouse_click_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(mouse_click_event) > on_mouse_click;
};

export class mouse_left_click_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(mouse_left_click_event) > on_mouse_left_click;
};

export class mouse_right_click_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(mouse_right_click_event) > on_mouse_right_click;
};

export class draw_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(draw_event) > on_draw;
};

export class item_changed_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(item_changed_event) > on_change;
};

export class command_event_binding
{
public:
	int subscribed_item_id;
	std::function< void(command_event) > on_command;
};

export class list_changed_event_binding
{
public:
	int subscribed_item_id;
	std::weak_ptr<control_base> control;
	std::function< void(list_changed_event) > on_change;
};

export using update_function = std::function< void(page* _page, double _elapsedSeconds, double _totalSeconds) >;

export class page : public std::enable_shared_from_this<page>
{

	rectangle layout(control_base* _item, layout_context _ctx);
	std::map<int, std::shared_ptr<key_up_event_binding> > key_up_events;
	std::map<int, std::shared_ptr<key_down_event_binding> > key_down_events;
	std::map<int, std::shared_ptr<mouse_move_event_binding> > mouse_move_events;
	std::map<int, std::shared_ptr<mouse_click_event_binding> > mouse_click_events;
	std::map<int, std::shared_ptr<mouse_left_click_event_binding> > mouse_left_click_events;
	std::map<int, std::shared_ptr<mouse_right_click_event_binding> > mouse_right_click_events;
	std::map<int, std::shared_ptr<item_changed_event_binding> > item_changed_events;
	std::map<int, std::shared_ptr<list_changed_event_binding> > list_changed_events;
	std::map<int, std::shared_ptr<command_event_binding> > command_events;
	update_function update_event;

protected:

	void handle_key_up(int _control_id, key_up_event evt);
	void handle_key_down(int _control_id, key_down_event evt);
	void handle_mouse_move(int _control_id, mouse_move_event evt);
	void handle_mouse_click(int _control_id, mouse_click_event evt);
	void handle_mouse_left_click(int _control_id, mouse_left_click_event evt);
	void handle_mouse_right_click(int _control_id, mouse_right_click_event evt);
	void handle_item_changed(int _control_id, item_changed_event evt);
	void handle_list_changed(int _control_id, list_changed_event evt);
	void handle_command(int _command_id, command_event evt);

	void arrange(double _width, double _height, double _padding = 0.0);

	virtual void create(std::weak_ptr<applicationBase> _host);
	virtual void destroy();
	virtual void draw();
	virtual void render(ID2D1DeviceContext *_context);
	virtual void update(double _elapsedSeconds, double _totalSeconds);

	void subscribe(presentation* _presentation);

public:

	std::shared_ptr<menu_item> menu;
	std::shared_ptr<control_base> root;
	std::string name;

	page(const char* _name = nullptr);
	virtual ~page();

	void clear();

	menu_item& create_menu();

	void on_key_up(int _control_id, std::function< void(key_up_event) >);
	void on_key_down(int _control_id, std::function< void(key_down_event) >);
	void on_mouse_move(std::weak_ptr<control_base> _base, std::function< void(mouse_move_event) >);
	void on_mouse_click(std::weak_ptr<control_base> _base, std::function< void(mouse_click_event) >);
	void on_mouse_left_click(std::weak_ptr<control_base> _base, std::function< void(mouse_left_click_event) >);
	void on_mouse_right_click(std::weak_ptr<control_base> _base, std::function< void(mouse_right_click_event) >);
	void on_item_changed(int _control_id, std::function< void(item_changed_event) >);
	void on_list_changed(int _control_id, std::function< void(list_changed_event) >);
	void on_command(int _item_id, std::function< void(command_event) >);
	void on_update(update_function fnc);

	row_layout& row_begin(int id = id_counter::next());
	column_layout& column_begin(int id = id_counter::next());
	absolute_layout& absolute_begin(int id = id_counter::next());
	control_base& end();

	inline control_base* get_root() {
		return root.get();
	}

	control_base* operator[](int _id)
	{
		return get_root()->find(_id);
	}

	friend class presentation;
};

export class presentation : public controller
{
protected:

	std::weak_ptr<page> current_page;
	rectangle current_size;

public:

	std::map<std::string, std::shared_ptr<page>> pages;
	std::weak_ptr<applicationBase> window_host;

	presentation();
	virtual ~presentation();

	void open_menu(control_base* _base, menu_item& _menu);

	virtual page& create_page(std::string _name, std::function<void(page& pg)> _settings = nullptr);
	virtual void select_page(const std::string& _page_name);
	menu_item& create_menu();

	template <typename control_type> control_type& find(int _id)
	{
		if (auto cp = current_page.lock()) {
			control_base* temp = cp->root->find(_id);
			if (temp == nullptr)
			{
				auto str = std::format("Control {0} not found ", _id);
				throw std::invalid_argument(str);
			}
			control_type* citem = dynamic_cast<control_type*>(temp);
			if (citem == nullptr)
			{
				auto str = std::format("Object is not {0} ", typeid(control_type).name());
				throw std::invalid_argument(str);
			}
			return *citem;
		}
		throw std::exception("could not lock current page");
	}

	template <typename control_type> control_type& get(control_base* _root, int _id)
	{
		if (auto cp = current_page.lock()) {
			control_base* temp = cp->root->get(_root, _id);
			if (temp == nullptr)
			{
				auto str = std::format("Control {0} not found ", _id);
				throw std::invalid_argument(str);
			}
			control_type* citem = dynamic_cast<control_type*>(temp);
			if (citem == nullptr)
			{
				auto str = std::format("Object is not {0} ", typeid(*citem).name());
				throw std::invalid_argument(str);
			}
			return *citem;
		}
		throw std::exception("could not lock current page");
	}

	virtual bool drawFrame(direct2dContext&_ctx);
	virtual bool update(double _elapsedSeconds, double _totalSeconds);

	virtual void keyDown(short _key);
	virtual void keyUp(short _key);
	virtual void mouseMove(point* _point);
	virtual void mouseLeftDown(point* _point);
	virtual void mouseLeftUp(point* _point);
	virtual void mouseRightDown(point* _point);
	virtual void mouseRightUp(point* _point);
	virtual void pointSelected(point* _point, color* _color);
	virtual LRESULT ncHitTest(point* _point);

	virtual void onCreated();
	virtual void onCommand(int buttonId);
	virtual void onTextChanged(int textControlId);
	virtual void onDropDownChanged(int dropDownId);
	virtual void onListBoxChanged(int listBoxId);
	virtual void onListViewChanged(int listViewId);
	virtual int onHScroll(int controlId, scrollTypes scrollType);
	virtual int onVScroll(int controlId, scrollTypes scrollType);
	virtual int onResize(const rectangle& newSize, double d2dScale);
	virtual int onSpin(int controlId, int newPosition);

	template <typename control_type> control_type* get_control(int _id)
	{
		control_type* r = nullptr;
		if (auto ppage = current_page.lock())
		{
			auto& rpage = *ppage;
			control_base* cb = rpage[_id];
			r = dynamic_cast<control_type*>(cb);
		}
		return r;
	}

	template <typename control_type> control_type* get_parent_control(int _id)
	{
		control_type* r = nullptr;
		if (auto ppage = current_page.lock())
		{
			auto& rpage = *ppage;
			control_base* cb = rpage[_id];
			r = dynamic_cast<control_type*>(cb);
			while (!r && cb) {
				cb = cb->parent;
				if (cb) {
					r = dynamic_cast<control_type*>(cb);
				}
			}
		}
		return r;
	}

};

export int id_counter::status_text_title_id = 100000;
export int id_counter::status_text_subtitle_id = 100001;
export int id_counter::status_bar_id = 100002;

export int id_counter::id = 100010;
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
}

menu_item_navigate::menu_item_navigate(const menu_item_navigate& _src)
{
	control_id = _src.control_id;
	target_page = _src.target_page;
	handler = _src.handler;
}

menu_item_navigate menu_item_navigate::operator =(const menu_item_navigate& _src)
{
	control_id = _src.control_id;
	target_page = _src.target_page;
	handler = _src.handler;
	return *this;
}

menu_item_navigate::menu_item_navigate(menu_item_navigate&& _src)
{
	control_id = _src.control_id;
	target_page = std::move(_src.target_page);
	handler = std::move(_src.handler);
}

menu_item_navigate& menu_item_navigate::operator =(menu_item_navigate&& _src)
{
	control_id = _src.control_id;
	target_page = std::move(_src.target_page);
	handler = std::move(_src.handler);
	return *this;
}

void menu_item_navigate::operator()(presentation* _presentation, std::weak_ptr<page> _page)
{
	_presentation->select_page(this->target_page);
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
	menu_item_navigate min_nav(_id, _target_page);
	mi->navigate_handler = min_nav;
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
		ppage->on_command(id, [this, _presentation, _page](command_event evt) {
			if (navigate_handler) {
				navigate_handler(_presentation, _page);
			}
			});

		for (auto child : children)
		{
			child->subscribe(_presentation, _page);
		}
	}
}

std::weak_ptr<control_base> control_base::get_shared()
{
	std::weak_ptr<control_base> empty;
	if (parent) {
		for (auto& child : parent->children)
		{
			if (child->id == id) {
				return child;
			}
		}
	}
	return empty;
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
	control_base* result = nullptr;

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

void control_base::create(std::weak_ptr<applicationBase> _host)
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

void control_base::render(ID2D1DeviceContext *_dest)
{
	for (auto child : children) {
		child->render(_dest);
	}
}

void control_base::apply(control_base& _ref)
{

}

bool control_base::set_mouse(point _position,
	bool* _left_down,
	bool* _right_down,
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

row_view_layout& container_control::row_view_begin(int id, std::function<void(row_view_layout&)> _settings)
{
	auto& tc = create<row_view_layout>(id);
	apply(tc);
	if (_settings) {
		_settings(tc);
	}
	return tc;
}

column_view_layout& container_control::column_view_begin(int id, std::function<void(column_view_layout&)> _settings)
{
	auto& tc = create<column_view_layout>(id);
	apply(tc);
	if (_settings) {
		_settings(tc);
	}
	return tc;
}

absolute_view_layout& container_control::absolute_view_begin(int id, std::function<void(absolute_view_layout&)> _settings)
{
	auto& tc = create<absolute_view_layout>(id);
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
	menu_item& menu,
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
				cl.set_size(120.0_px, 1.0_container);
				cl.set_item_margin(5.0_px);
					})
				.menu_button(menu_button_id, [menu](auto& _ctrl) {
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
												rl.set_item_margin(5.0_px);
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
	std::function<void(container_control& _settings)> _add_controls
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
	for (auto child : children) {
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
			r.set_background_color(styles.get_style().FormBackgroundColor);
		})
		.column_begin([_add_controls1, _align_id](column_layout& r)
			{
				r.set_margin(10.0_px);
				r.set_size(.30_container, 1.0_container);
				r.push(_align_id, true, false, false, false);
				_add_controls1(r);
			})
			.end()
				.column_begin([_add_controls2, _align_id](column_layout& r)
					{
						r.set_margin(10.0_px);
						r.set_size(.30_container, 1.0_container);
						r.push(_align_id, true, false, false, false);
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
	auto& tc = create<title_control>(_id);
	apply(tc);
	tc.text = text;
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::subtitle(std::string text, std::function<void(subtitle_control&)> _settings, int _id)
{
	auto& tc = create<subtitle_control>(_id);
	apply(tc);
	tc.text = text;
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::chaptertitle(std::string text, std::function<void(chaptertitle_control&)> _settings, int _id)
{
	auto& tc = create<chaptertitle_control>(_id);
	apply(tc);
	tc.text = text;
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::chaptersubtitle(std::string text, std::function<void(chaptersubtitle_control&)> _settings, int _id)
{
	auto& tc = create<chaptersubtitle_control>(_id);
	apply(tc);
	tc.text = text;
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::paragraph(std::string text, std::function<void(paragraph_control&)> _settings, int _id)
{
	auto& tc = create<paragraph_control>(_id);
	apply(tc);
	tc.text = text;
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::code(std::string text, std::function<void(code_control&)> _settings, int _id)
{
	auto& tc = create<code_control>(_id);
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

container_control& container_control::camera(int _id, std::function<void(camera_control&)> _settings)
{
	auto& tc = create<camera_control>(_id);
	apply(tc);
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::grid(int _id, std::function<void(grid_control&)> _settings)
{
	auto& tc = create<grid_control>(_id);
	apply(tc);
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::chart(int _id, std::function<void(chart_control&)> _settings)
{
	auto& tc = create<chart_control>(_id);
	apply(tc);
	if (_settings) {
		_settings(tc);
	}
	return *this;
}

container_control& container_control::slide(int _id, std::function<void(slide_control&)> _settings)
{
	auto& tc = create<slide_control>(_id);
	apply(tc);
	if (_settings) {
		_settings(tc);
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
	auto& tc = cl.create<field_control>(_id);
	tc.tooltip_text = _tooltip_string;
	cl.apply(tc);
	if (_settings) {
		_settings(tc);
	}
	cl.end();
	return *_parent;
}

container_control& container_control::listbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listbox_control&)> _settings)
{
	return create_field<listbox_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::combobox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(combobox_control&)> _settings)
{
	return create_field<combobox_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::edit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(edit_control&)> _settings)
{
	return create_field<edit_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::listview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(listview_control&)> _settings)
{
	return create_field<listview_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::treeview_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(treeview_control&)> _settings)
{
	return create_field<treeview_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::header_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(header_control&)> _settings)
{
	return create_field<header_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::toolbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(toolbar_control&)> _settings)
{
	return create_field<toolbar_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::statusbar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(statusbar_control&)> _settings)
{
	return create_field<statusbar_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::hotkey_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(hotkey_control&)> _settings)
{
	return create_field<hotkey_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::animate_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(animate_control&)> _settings)
{
	return create_field<animate_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::richedit_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(richedit_control&)> _settings)
{
	return create_field<richedit_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::draglistbox_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(draglistbox_control&)> _settings)
{
	return create_field<draglistbox_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::comboboxex_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(comboboxex_control&)> _settings)
{
	return create_field<comboboxex_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::datetimepicker_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(datetimepicker_control&)> _settings)
{
	return create_field<datetimepicker_control>(this, _id, _field_label, _tooltip_text, _settings);
}
container_control& container_control::monthcalendar_field(int _id, std::string _field_label, std::string _tooltip_text, std::function<void(monthcalendar_control&)> _settings)
{
	return create_field<monthcalendar_control>(this, _id, _field_label, _tooltip_text, _settings);
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
	std::function<point(point* _origin, const rectangle* _bounds, control_base*)> _next_origin)
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
		::AppendMenuA(hmenu, MF_POPUP, (UINT_PTR)popupMenu, name.c_str());
	}
	else if (is_separator)
	{
		::AppendMenuA(hmenu, MF_SEPARATOR, id, nullptr);
	}
	else
	{
		::AppendMenuA(hmenu, MF_STRING, id, name.c_str());
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

void draw_control::create(std::weak_ptr<applicationBase> _host)
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

void draw_control::render(ID2D1DeviceContext* _dest)
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
	for (auto child : children)
	{
		child->render(_dest);
	}
}

camera_control::camera_control(container_control* _parent, int _id) : draw_control(_parent, _id)
{
	init();
}

void camera_control::init()
{
	set_origin(0.0_px, 0.0_px);
	set_size(1.0_container, 1.2_fontgr);

	on_create = [this](draw_control* _src)
		{
			if (auto pwindow = this->window.lock())
			{
				solidBrushRequest	border_brush;
				pwindow->getContext().setSolidColorBrush(&this->background_brush);
				pwindow->getContext().setSolidColorBrush(&this->border_brush);
			}
		};

	on_draw = [this](draw_control* _src) {
		if (auto pwindow = this->window.lock())
		{
			if (auto phost = host.lock()) {
				auto draw_bounds = inner_bounds;

				draw_bounds.x = 0;
				draw_bounds.y = 0;

				pwindow->getContext().drawRectangle(&draw_bounds, this->border_brush.name, 8, this->background_brush.name);
			}
		}
		};
}

camera_control::~camera_control()
{
	;
}

grid_control::grid_control(container_control* _parent, int _id)
{
	;
}

grid_control::~grid_control()
{
	;
}


chart_control::chart_control(container_control* _parent, int _id)
{
	;
}

chart_control::~chart_control()
{
	;
}


slide_control::slide_control(container_control* _parent, int _id)
{
	;
}

slide_control::~slide_control()
{
	;
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

				std::string test_text = std::format("{0}, {1}, {2}", text, draw_bounds.x, draw_bounds.y, (long)this);

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
				auto& context = pwindow->getContext();

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

comboboxex_control::comboboxex_control(container_control* _parent, int _id) : windows_control(_parent, _id)
{
	control_base::set_origin(0.0_px, 0.0_px);
	control_base::set_size(1.0_container, 2.0_fontgr);
}

void comboboxex_control::data_changed()
{
	if (IsWindow(window)) {
		if (auto phost = window_host.lock()) {
			phost->clearComboItems(id);
			for (int i = 0; i < choices.items.size(); i++)
			{
				auto c = choices.items[i];
				if (c.has_member(choices.id_field) && c.has_member(choices.text_field)) {
					int lid = c[choices.id_field];
					std::string description = c[choices.text_field];

					COMBOBOXEXITEMA cbex = {};
					cbex.mask = CBEIF_TEXT | CBEIF_LPARAM;
					cbex.iItem = -1;
					cbex.pszText = (LPSTR)description.c_str();
					cbex.iImage = 0;
					cbex.iSelectedImage = 0;
					cbex.iIndent = 0;
					cbex.lParam = lid;
					SendMessage(windows_control::window, CBEM_INSERTITEM, 0, (LPARAM) &cbex);
				}
			}
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

		int h = text_style.fontSize * 8;
		MoveWindow(windows_control::window, boundsPixels.x, boundsPixels.y, boundsPixels.w, h, TRUE);
	}

	data_changed();
}

void comboboxex_control::on_resize()
{
	if (auto phost = window_host.lock()) {
		auto boundsPixels = phost->toPixelsFromDips(get_inner_bounds());

		if (windows_control::window) {
			MoveWindow( window, boundsPixels.x, boundsPixels.y, boundsPixels.w, windows_control::text_style.fontSize * 8, TRUE);
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
	return Animate_Open( windows_control::window, _name.c_str());
}

bool animate_control::open(DWORD resource_id)
{
	return Animate_Open(windows_control::window, resource_id);
}

bool animate_control::play(UINT from, UINT to, UINT rep)
{
	return Animate_Play(windows_control::window, from, to, rep);
}

bool animate_control::play()
{
	return Animate_Play(windows_control::window, 0, -1, 1);
}

bool animate_control::stop()
{
	return Animate_Stop(windows_control::window);
}

gradient_button_control::gradient_button_control(container_control* _parent, int _id, std::string _base_name) : draw_control(_parent, _id)
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

		buttonBackLight.center = rectangle_math::center(_ctx);
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

			auto& context = pwindow->getContext();

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
		menu.subscribe(_presentation, _page);
		auto pcontrol = get_shared();
		ppage->on_mouse_left_click(pcontrol, [this, _presentation, _page](mouse_left_click_event evt)
			{
				_presentation->open_menu(this, this->menu);
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

void page::create(std::weak_ptr<applicationBase> _host)
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

void page::render(ID2D1DeviceContext* _context)
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
	root->on_subscribe(_presentation, shared_from_this());
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
	if (auto ptr = window_host.lock()) {
		HWND hwndMenu = ptr->getMainWindow();
		POINT tpstart;
		auto& bpos = _base->get_bounds();
		tpstart.x = bpos.right() * ::GetDpiForWindow(hwndMenu) / 96.0;
		tpstart.y = bpos.bottom() * ::GetDpiForWindow(hwndMenu) / 96.0;
		::ClientToScreen(hwndMenu, &tpstart);
		::TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN, tpstart.x, tpstart.y, hwndMenu, nullptr);
	}
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

void page::on_mouse_left_click(std::weak_ptr<control_base> _base, std::function< void(mouse_left_click_event) > handler)
{
	auto evt = std::make_shared<mouse_left_click_event_binding>();
	if (auto pbase = _base.lock())
	{
		evt->control = pbase;
		evt->subscribed_item_id = pbase->id;
		evt->on_mouse_left_click = handler;
		mouse_left_click_events[pbase->id] = evt;
	}
}

void page::on_mouse_right_click(std::weak_ptr<control_base> _base, std::function< void(mouse_right_click_event) > handler)
{
	auto evt = std::make_shared<mouse_right_click_event_binding>();
	if (auto pbase = _base.lock())
	{
		evt->control = pbase;
		evt->subscribed_item_id = pbase->id;
		evt->on_mouse_right_click = handler;
		mouse_right_click_events[pbase->id] = evt;
	}
}

void page::on_mouse_move(std::weak_ptr<control_base> _base, std::function< void(mouse_move_event) > handler)
{
	auto evt = std::make_shared<mouse_move_event_binding>();
	if (auto pbase = _base.lock())
	{
		evt->control = pbase;
		evt->subscribed_item_id = pbase->id;
		evt->on_mouse_move = handler;
		mouse_move_events[pbase->id] = evt;
	}
}

void page::on_mouse_click(std::weak_ptr<control_base> _base, std::function< void(mouse_click_event) > handler)
{
	auto evt = std::make_shared<mouse_click_event_binding>();
	if (auto pbase = _base.lock())
	{
		evt->control = pbase;
		evt->subscribed_item_id = pbase->id;
		evt->on_mouse_click = handler;
		mouse_click_events[pbase->id] = evt;
	}
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
	evt->on_command = handler;
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
}

void page::handle_mouse_click(int _control_id, mouse_click_event evt)
{
	if (mouse_click_events.contains(_control_id)) {
		auto& ptrx = mouse_click_events[_control_id];
		evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
		evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
		evt.control = evt.control;
		evt.control_id = evt.control->id;
		ptrx->on_mouse_click(evt);
	}
}

void page::handle_mouse_left_click(int _control_id, mouse_left_click_event evt)
{
	if (mouse_left_click_events.contains(_control_id)) {
		auto& ptrx = mouse_left_click_events[_control_id];
		evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
		evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
		ptrx->on_mouse_left_click(evt);
	}
}

void page::handle_mouse_right_click(int _control_id, mouse_right_click_event evt)
{
	if (mouse_right_click_events.contains(_control_id)) {
		auto& ptrx = mouse_right_click_events[_control_id];
		evt.relative_point.x = evt.absolute_point.x - evt.control->get_bounds().x;
		evt.relative_point.y = evt.absolute_point.y - evt.control->get_bounds().y;
		evt.control = evt.control;
		evt.control_id = evt.control->id;
		ptrx->on_mouse_right_click(evt);
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

	if (auto phost = window_host.lock())
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
		if (auto phost = window_host.lock()) {
			auto sheet = styles.get_style();
			auto pos = phost->getWindowClientPos();
			phost->toPixelsFromDips(pos);
			cp->arrange(pos.w, pos.h);
			cp->create(phost);
			cp->subscribe(this);
		}
	}
}

bool presentation::drawFrame(direct2dContext& _ctx)
{
	auto cp = current_page.lock();
	if (cp) {
		cp->draw();

		auto dc = _ctx.getDeviceContext();
		cp->render(dc);

		if (auto phost = window_host.lock()) {
			auto pos = phost->getWindowClientPos();

			double border_thickness = 4;

			linearGradientBrushRequest lgbr;
			lgbr.start.x = pos.w;
			lgbr.start.y = pos.h;
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

			double inner_right = pos.w - border_thickness * 2;
			double inner_bottom = pos.h - border_thickness * 2;
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
			pathx.path.addLineTo(pos.w, 0);
			pathx.path.addLineTo(pos.w, pos.h);
			pathx.path.addLineTo(0, pos.h);
			pathx.path.addLineTo(0, 0);
			pathx.closed = true;

			_ctx.drawPath(&pathx);
		}

	}
	return false;
}

bool presentation::update(double _elapsedSeconds, double _totalSeconds)
{
	auto cp = current_page.lock();
	if (cp) {
		cp->update(_elapsedSeconds, _totalSeconds);
	}
	return true;
}

void presentation::keyDown(short _key)
{
	auto cp = current_page.lock();
	key_down_event kde;
	kde.control_id = 0;
	kde.key = _key;
	if (cp) {
		cp->handle_key_down(0, kde);
	}
}

void presentation::keyUp(short _key)
{
	auto cp = current_page.lock();
	key_up_event kde;
	kde.control_id = 0;
	kde.key = _key;
	if (cp) {
		cp->handle_key_up(0, kde);
	}
}

void presentation::mouseMove(point* _point)
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

void presentation::mouseLeftDown(point* _point)
{
	bool leftMouse = true;
	auto cp = current_page.lock();
	presentation* p = this;
	cp->root->set_mouse(*_point, &leftMouse, nullptr, [cp, p, _point](control_base* _item) {
		mouse_left_click_event mcel = {};
		mcel.control = _item;
		mcel.control_id = _item->id;
		mcel.absolute_point.x = _point->x;
		mcel.absolute_point.y = _point->y;
		mcel.absolute_point.z = 0;
		if (_item) {
			mcel.relative_point.x = _point->x - _item->get_bounds().x;
			mcel.relative_point.y = _point->y - _item->get_bounds().y;
			mcel.relative_point.z = 0;
		}
		cp->handle_mouse_left_click(_item->id, mcel);

		mouse_click_event mce = {};
		mce.control = _item;
		mce.control_id = _item->id;
		mce.absolute_point.x = _point->x;
		mce.absolute_point.y = _point->y;
		mce.absolute_point.z = 0;
		if (_item) {
			mcel.relative_point.x = _point->x - _item->get_bounds().x;
			mcel.relative_point.y = _point->y - _item->get_bounds().y;
			mcel.relative_point.z = 0;
		}
		cp->handle_mouse_click(_item->id, mce);
		}, nullptr);
}

void presentation::mouseLeftUp(point* _point)
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

		mouse_left_click_event mcel;
		mcel.control = _item;
		mcel.control_id = _item->id;
		mcel.absolute_point.x = _point->x;
		mcel.absolute_point.y = _point->y;
		mcel.absolute_point.z = 0;
		cp->handle_mouse_left_click(_item->id, mcel);
		}, nullptr);
}

void presentation::mouseRightDown(point* _point)
{
	bool rightMouse = true;
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
		});
}

void presentation::mouseRightUp(point* _point)
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
void presentation::pointSelected(point* _point, color* _color)
{
	;
}

LRESULT presentation::ncHitTest(point* _point)
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
	if (auto ptr = window_host.lock()) {
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
	if (auto ptr = window_host.lock()) {
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
	if (auto ptr = window_host.lock()) {
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
	if (auto ptr = window_host.lock()) {
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

int presentation::onHScroll(int controlId, scrollTypes scrollType)
{
	return 0;
}

int presentation::onVScroll(int controlId, scrollTypes scrollType)
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

