#ifndef CORONA_APPLICATION_BASE_H
#define CORONA_APPLICATION_BASE_H

namespace corona
{

	class applicationBase
	{
	protected:

		std::shared_ptr<controller> currentController;

	public:


		virtual HWND getMainWindow() = 0;
		virtual HWND getTooltipWindow() = 0;
		virtual HWND createWindow(DWORD window_id, LPCTSTR		lpClassName, LPCTSTR		lpWindowName, DWORD       dwStyle, rectangle bounds, LPVOID		lpParam, HFONT		font) = 0;
		virtual void destroyWindow(HWND hwnd) = 0;

		virtual void setController(std::shared_ptr<controller> _newCurrentController)
		{
			currentController = _newCurrentController;
		}

		virtual std::weak_ptr<direct2dChildWindow> createDirect2Window(DWORD control_id, rectangle bounds) = 0;

		virtual void setPictureIcon(int controlId, dtoIconId iconId) = 0;
		virtual void setButtonIcon(int controlId, dtoIconId iconId) = 0;

		virtual void setVisible(int controlId, bool visible) = 0;
		virtual void setEnable(int controlId, bool enabled) = 0;
		virtual void setFocus(int ddlControlId) = 0;
		virtual void killFocus(int ddlControlId) = 0;

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

		virtual rectangle getWindowClientPos() = 0;
		virtual rectangle getWindowPos(int ddlControlId) = 0;
		virtual void setWindowPos(int ddlControlId, rectangle rect) = 0;
		virtual void setMinimumWindowSize(point size) = 0;

		virtual double toDipsFromPixels(double r) = 0;
		virtual double toPixelsFromDips(double r) = 0;
		virtual rectangle toPixelsFromDips(const rectangle& r) = 0;
		virtual rectangle toDipsFromPixels(const rectangle& r) = 0;

		virtual HFONT createFontDips(HWND target, std::string _fontName, double fontSize, bool bold, bool italic) = 0;
		virtual HFONT createFontPixels(std::string  _fontName, double fontSize, bool bold, bool italic) = 0;
		virtual HFONT createFontIndirect(LOGFONTA _font, std::string _fontName, double fontSize, bool bold, bool italic) = 0;
	};
}

#endif
