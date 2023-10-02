module;

#include <memory>
#include <string>
#include <vector>
#include <list>
#include "windows.h"

export module corona.database:controller;
import :constants;
import :direct2dcontext;
import :datatransfer;
import :point_box;
import :rectangle_box;
import :visual;

export enum scrollTypes
{
	ScrollPageUp,
	ScrollPageDown,
	ScrollLineUp,
	ScrollLineDown,
	ThumbTrack
};

export class controller  {

public:

	virtual HWND getMainWindow() = 0;
	virtual HWND createWindow(DWORD window_id, LPCTSTR		lpClassName, LPCTSTR		lpWindowName, DWORD       dwStyle, rectangle bounds, LPVOID		lpParam, HFONT		font) = 0;
	virtual void destroyWindow(HWND hwnd) = 0;

	virtual std::weak_ptr<direct2dChildWindow> createDirect2Window(DWORD control_id, rectangle bounds) = 0;
	virtual std::weak_ptr<direct2dChildWindow> getDirect2dWindow(relative_ptr_type ctrlId) = 0;

	virtual void setController(std::shared_ptr<controller> _newCurrentController) = 0;

	virtual void setPictureIcon(int controlId, dtoIconId iconId) = 0;
	virtual void setButtonIcon(int controlId, dtoIconId iconId) = 0;

	virtual void setVisible(int controlId, bool visible) = 0;
	virtual void setEnable(int controlId, bool enabled) = 0;
	virtual void setFocus(int ddlControlId) = 0;

	virtual void setEditText(int textControlId, const std::string& _string) = 0;
	virtual void setEditText(int textControlId, const char* _string) = 0;
	virtual std::string getEditText(int textControlId) = 0;

	virtual std::string getComboSelectedText(int ddlControlId) = 0;
	virtual int getComboSelectedValue(int ddlControlId) = 0;
	virtual int getComboSelectedIndex(int ddlControlId) = 0;
	virtual void setComboSelectedIndex(int ddlControlId, int index) = 0;
	virtual void setComboSelectedText(int ddlControlId, std::string& _text) = 0;
	virtual void setComboSelectedText(int ddlControlId, const char* _text) = 0;
	virtual void setComboSelectedValue(int ddlControlId, int value) = 0;
	virtual void clearComboItems(int ddlControlId) = 0;
	virtual void addComboItem(int ddlControlId, std::string& _text, int _data) = 0;
	virtual void addComboItem(int ddlControlId, const char* _text, int _data) = 0;
	virtual void addFoldersToCombo(int ddlControlId, const char* _path) = 0;
	virtual void addPicturesFoldersToCombo(int ddlControlId) = 0;

	virtual std::string getListSelectedText(int ddlControlId) = 0;
	virtual int getListSelectedValue(int ddlControlId) = 0;
	virtual int getListSelectedIndex(int ddlControlId) = 0;
	virtual void setListSelectedIndex(int ddlControlId, int index) = 0;
	virtual void setListSelectedText(int ddlControlId, std::string& _text) = 0;
	virtual void setListSelectedText(int ddlControlId, const char* _text) = 0;
	virtual void setListSelectedValue(int ddlControlId, int value) = 0;
	virtual void clearListItems(int ddlControlId) = 0;
	virtual void addListItem(int ddlControlId, std::string& _text, int _data) = 0;
	virtual void addListItem(int ddlControlId, const char* _text, int _data) = 0;
	virtual void addFoldersToList(int ddlControlId, const char* _path) = 0;
	virtual void addPicturesFoldersToList(int ddlControlId) = 0;
	virtual void addListViewColumn(int ddlControlId, int column_id, char* _text, int _width, visual_alignment _alignment) = 0;
	virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<char*>& _items) = 0;

	virtual void setButtonChecked(int controlId, bool enabled) = 0;
	virtual bool getButtonChecked(int controlId) = 0;

	virtual void clearListView(int ddlControlId) = 0;
	virtual void addListViewItem(int ddlControlId, std::string& _text, LPARAM _data) = 0;
	virtual void addListViewItem(int ddlControlId, const char* _text, LPARAM _data) = 0;
	virtual std::string getListViewSelectedText(int ddlControlId) = 0;
	virtual int getListViewSelectedIndex(int ddlControlId) = 0;
	virtual LPARAM getListViewSelectedValue(int ddlControlId) = 0;
	virtual std::list<std::string> getListViewSelectedTexts(int ddlControlId) = 0;
	virtual std::list<int> getListViewSelectedIndexes(int ddlControlId) = 0;
	virtual std::list<LPARAM> getListViewSelectedValues(int ddlControlId) = 0;
	virtual void setListViewSelectedIndex(int ddlControlId, int indexId) = 0;
	virtual void setListViewSelectedText(int ddlControlId, std::string& _text) = 0;
	virtual void setListViewSelectedText(int ddlControlId, const char* _text) = 0;
	virtual void setListViewSelectedValue(int ddlControlId, LPARAM value) = 0;

	virtual void setScrollHeight(int ddlControlId, int height) = 0;
	virtual void setScrollWidth(int ddlControlId, int width) = 0;
	virtual point getScrollPos(int ddlControlId) = 0;
	virtual point getScrollTrackPos(int ddlControlId) = 0;
	virtual point getScrollRange(int ddlControlId) = 0;
	virtual void setScrollPos(int ddlControlId, point pt) = 0;

	virtual void setSpinRange(int ddlControlId, int lo, int high) = 0;
	virtual void setSpinPos(int ddlControlId, int pos) = 0;

	virtual void setSysLinkText(int ddlControlId, const char* _text) = 0;
	virtual std::vector<std::string> readInternet(const char* _domain, const char* _path) = 0;

	virtual void redraw() = 0;

	virtual void setColorCapture(int _iconResourceId) = 0;

	virtual bool getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension) = 0;
	virtual rectangle getWindowClientPos() = 0;
	virtual rectangle getWindowPos(int ddlControlId) = 0;
	virtual rectangle toPixelsFromDips(const rectangle& r) = 0;
	virtual void setWindowPos(int ddlControlId, rectangle rect) = 0;
	virtual void setMinimumWindowSize(point size) = 0;

	virtual bool drawFrame(win32::direct2dContext& _ctx);
	virtual bool update(double _elapsedSeconds, double _totalSeconds);

	virtual void keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key)=0;
	virtual void keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key) = 0;
	virtual void mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
	virtual void mouseLeftDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
	virtual void mouseLeftUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
	virtual void mouseRightDown(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
	virtual void mouseRightUp(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
	virtual void pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color) = 0;
	virtual LRESULT ncHitTest(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;

	virtual void onCreated() = 0;
	virtual void onCommand(int buttonId) = 0;
	virtual void onTextChanged(int textControlId) = 0;
	virtual void onDropDownChanged(int dropDownId) = 0;
	virtual void onListBoxChanged(int listBoxId) = 0;
	virtual void onListViewChanged(int listViewId) = 0;
	virtual int onHScroll(int controlId, win32::scrollTypes scrollType) = 0;
	virtual int onVScroll(int controlId, win32::scrollTypes scrollType) = 0;
	virtual int onResize(const rectangle& newSize, double d2dScale) = 0;
	virtual int onSpin(int controlId, int newPosition) = 0;


};

/*
corona_controller::~corona_controller()
{
	;
}

void corona_controller::onCreated()
{
	enableEditMessages = false;

	auto pos = host->getWindowClientPos();

	//			host->setMinimumWindowSize(point{ pos.w - pos.x, pos.h - pos.y });

	stateChanged(pos);

	enableEditMessages = true;
}

void corona_controller::randomAdvertisement()
{
}

void corona_controller::keyDown(direct2dWindow* win, short _key)
{
	;
}

void corona_controller::keyUp(direct2dWindow* win, short _key)
{
	;
}

void corona_controller::mouseMove(direct2dWindow* win, point* _point)
{
	for (auto pgi : pg) {
		pgi.item.mouse_over = false;
	}

	auto over_items = pg.where([this, _point](const auto& pi) {
		return pi.item.is_command() &&
			rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });

	for (auto ov : over_items)
	{
		ov.item.mouse_over = true;
	}
	host->redraw();
}

int corona_controller::onHScroll(int controlId, scrollTypes scrollType)
{
	if (controlId != canvasWindowsId)
		return 0;

	rectangle r = host->getWindowPos(controlId);
	int pos = host->getScrollPos(controlId).x;
	int max = host->getScrollRange(controlId).x;

	switch (scrollType) {
	case ScrollPageUp:
		pos -= r.w;
		if (pos < 0) pos = 0;
		break;
	case ScrollPageDown:
		pos += r.w;
		if (pos > max) pos = max;
		break;
	case ScrollLineUp:
		pos -= 16;
		if (pos < 0) pos = 0;
		break;
	case ScrollLineDown:
		pos += 16;
		if (pos > max) pos = max;
		break;
	case ThumbTrack:
		pos = host->getScrollTrackPos(controlId).x;
		break;
	}

	currentScroll.x = pos;
	host->redraw();

	return pos;
}

int corona_controller::onVScroll(int controlId, scrollTypes scrollType)
{
	if (controlId != canvasWindowsId)
		return 0;

	rectangle r = host->getWindowPos(controlId);
	int pos = host->getScrollPos(controlId).y;
	int max = host->getScrollRange(controlId).y;

	switch (scrollType) {
	case ScrollPageUp:
		pos -= r.h;
		if (pos < 0) pos = 0;
		break;
	case ScrollPageDown:
		pos += r.h;
		if (pos > max) pos = max;
		break;
	case ScrollLineUp:
		pos -= 16;
		if (pos < 0) pos = 0;
		break;
	case ScrollLineDown:
		pos += 16;
		if (pos > max) pos = max;
		break;
	case ThumbTrack:
		pos = host->getScrollTrackPos(controlId).y;
		break;
	}

	currentScroll.y = pos;
	host->redraw();

	return pos;
}

int corona_controller::onSpin(int controlId, int newPosition)
{
	double newPositionF = newPosition / 100.0;
	return 0;
}

int corona_controller::onResize(const rectangle& newSize, double d2dScale)
{
	setScrollBars();

	//			state = user_collection.get_actor_state(state.actor_id, null_row, "state");
	stateChanged(newSize);

	return 0;
}

void corona_controller::setScrollBars()
{
}

void corona_controller::mouseClick(direct2dWindow* win, point* _point)
{
	auto clicked_items = pg.where([this, _point](const auto& pi) { return pi.item.is_command() && rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });
	auto size = host->getWindowClientPos();

#if TRACE_CONTROLLER
			std::cout << std::format("clicked {},{}", _point->x, _point->y) << std::endl;
#endif

			if (clicked_items != std::end(pg))
			{
				auto clicked_item = clicked_items.get_object();

#if TRACE_CONTROLLER
				std::cout << std::format("{} clicked", clicked_item.item.id) << std::endl;
#endif

				if (clicked_item.item.is_select())
				{

				}
				else if (clicked_item.item.is_create())
				{
				}
				else if (clicked_item.item.is_set())
				{
				}

				stateChanged(size);
			}
		}

		void corona_controller::exportBitmap(const char* _filenameImage)
		{
			point dt = { 100, 100 };
			auto bm = host->getDrawable(0)->createBitmap(dt);

			color c;
			c.a = 1.0;
			c.r = c.g = c.b = 0.0;

			bool blown_away;

			bm->beginDraw(blown_away);
			bm->clear(&c);
			bm->endDraw(blown_away);

			bm->save(_filenameImage);

			delete bm;
		}

		void corona_controller::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
		{
			;
		}

		bool corona_controller::update(double _elapsedSeconds, double _totalSeconds)
		{
			bool oldShowUpdate = showUpdate;
			showUpdate = false;
			return oldShowUpdate;
		}

		void corona_controller::onCommand(int buttonId)
		{
			auto size = host->getWindowClientPos();

			stateChanged(size);
		}

		void corona_controller::onTextChanged(int textControlId)
		{
		}

		void corona_controller::onDropDownChanged(int dropDownId)
		{
		}

		void corona_controller::onListViewChanged(int listViewId)
		{

		}

		void corona_controller::fromImage()
		{
			std::string temp;
			bool tempB;
			int tempI;

			enableEditMessages = false;

			enableEditMessages = true;
		}

		void corona_controller::pointSelected(direct2dWindow* win, point* _point, color* _color)
		{

		}

		void corona_controller::clearErrors(base_result* _error)
		{
			;
		}

		void corona_controller::addError(base_result* _error)
		{
			;
		}

		const char* corona_controller::style_id(relative_ptr_type _style_field_id)
		{
			const char* r = nullptr;
			if (_style_field_id > 0) {
				r = schema.get_field(_style_field_id).name;
			}
			return r;
		}


		void corona_controller::stateChanged(const rectangle& newSize)
		{
			render(newSize);
		}

		bool corona_controller::drawCanvas(int _id)
		{
			bool adapter_blown_away = false;
			if (pg.size() > _id && _id >= 0)
			{
				auto& item = pg[_id];
				if (item.is_canvas2d()) {
					auto item_id = item.get_identifier();
					auto  host = getHost()->getWindow(item_id);
					if (!host) // this can be, the size of the window was too small so the host was never made.
						return false;
					host->beginDraw(adapter_blown_away);

					auto location = item.bounds;
					location.x = 0;
					location.y = 0;

#if TRACE_RENDER
					std::string labelo = std::format("{} {}", "testo", _id);
					host->drawView("label_style", labelo.c_str(), location, "commment");
					std::cout << "Draw Canvas Item" << item.id << " " << location.x << " " << location.y << " " << location.w << " " << location.h << std::endl;
#else

					pg.visit_impl(&item, [this, host](control_base* _in_page)
						{
							if (_in_page->is_drawable())
							{
#if TRACE_RENDER
								std::cout << ".. render item!!" << _in_page->id << " " << std::endl;
#endif
								render_item(host, _in_page);
							}
							return true;
						},
						[this](control_base* _out_page)
						{
							return true;
						}
						);
#endif
					host->endDraw(adapter_blown_away);
				}
			}
			return adapter_blown_away;
		}
*/
