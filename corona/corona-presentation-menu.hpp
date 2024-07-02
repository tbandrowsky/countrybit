/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
Payload to create a Win32 menu
Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_MENU_H
#define CORONA_PRESENTATION_MENU_H

namespace corona
{
	class menu_item;

	class presentation_base {
	public:
		virtual void select_page(const std::string& _page_name) = 0;
		virtual void restore_window() = 0;
		virtual void minimize_window() = 0;
		virtual void close_window() = 0;
		virtual void open_menu(control_base* _base, menu_item& _menu) = 0;
	};

	using menu_click_handler = std::function<void(presentation_base* _presentation, std::weak_ptr<page> _page)>;

	class menu_item_navigate
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
		void operator()(presentation_base* _presentation);
		operator bool() { return control_id > 0; }
	};

	class menu_item;

	class menu_item : public std::enable_shared_from_this<menu_item>
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

		void subscribe(presentation_base* _presentation, page_base *_page);

		HMENU to_menu();
	};

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

	void menu_item_navigate::operator()(presentation_base* _presentation)
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

	void menu_item::subscribe(presentation_base* _presentation, page_base *ppage)
	{
		ppage->on_command(id, [this, _presentation, ppage](command_event evt) {
			if (navigate_handler) {
				navigate_handler(_presentation);
			}
		});

		for (auto child : children)
		{
			child->subscribe(_presentation, ppage);
		}
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

}

#endif
