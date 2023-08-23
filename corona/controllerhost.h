
#pragma once

namespace corona
{

	namespace win32
	{
		using namespace database;

		class direct2dWindow;
		class direct2dChildWindow;
		class direct2dBitmap;

		class drawableHost {
		public:

			virtual void clear(color* _color) = 0;
			virtual std::string setBitmap(bitmapRequest* _bitmap) = 0;
			virtual bool setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize) = 0;
			virtual bool setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter) = 0;
			virtual std::string  setBitmapBrush(bitmapBrushRequest* _bitmapBrush) = 0;
			virtual std::string setSolidColorBrush(solidBrushRequest* _solidBrushDto) = 0;
			virtual std::string setLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto) = 0;
			virtual std::string setRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto) = 0;
			virtual void clearBitmapsAndBrushes(bool deleteStockObjects = false) = 0;

			virtual std::string  setPath(pathDto* _pathDto, bool _closed = true) = 0;

			virtual void setViewStyle(viewStyleRequest& _textStyle) = 0;
			virtual void clearViewStyles() = 0;
			virtual void setTextStyle(textStyleRequest* _textStyle) = 0;
			virtual void clearTextStyles() = 0;

			virtual void clearPaths() = 0;

			virtual void drawPath(pathInstance2dDto* _pathInstanceDto) = 0;
			virtual void drawPath(pathImmediateDto* _pathImmediateDto) = 0;
			virtual void drawText(drawTextRequest* _textInstanceDto) = 0;
			virtual void drawBitmap(bitmapInstanceDto* _bitmapInstanceDto) = 0;

			virtual void drawLine(database::point* start, database::point* stop, const char* _fillBrush, double thickness) = 0;
			virtual void drawRectangle(database::rectangle* _rectDto, const char* _borderBrush, double _borderWidth, const char* _fillBrush) = 0;
			virtual void drawText(const char* _text, database::rectangle* _rectDto, const char* _textStyle, const char* _fillBrush) = 0;
			virtual database::rectangle getCanvasSize() = 0;

			virtual void popCamera() = 0;
			virtual void pushCamera(point* _position, float _rotation, float _scale = 1.0) = 0;

			virtual std::shared_ptr<direct2dBitmap> createBitmap(point& _size) = 0;
			virtual void drawBitmap(drawableHost* _directBitmap, point& _dest, point& _size) = 0;
			virtual bool isBitmap() { return false; }
			virtual void save(const char* _filename) = 0;

			virtual void drawView(const char* _style, const char* _text, rectangle& _rect, int _state, const char *_debug_comment) = 0;
		};

		class controllerHost {

		public:


			virtual bool getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension) = 0;
			virtual rectangle getWindowClientPos() = 0;
			virtual rectangle getWindowPos(int ddlControlId) = 0;
			virtual void setWindowPos(int ddlControlId, rectangle rect) = 0;
			virtual void setMinimumWindowSize(point size) = 0;
		};

		class controller;

		class win32ControllerHost : public controllerHost {

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
			virtual void addListViewColumn(int ddlControlId, int column_id, const char* _text, int _width, visual_alignment _alignment) = 0;
			virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<std::string>& _items) = 0;

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

		};

	}
}
