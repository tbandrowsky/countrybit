#pragma once

//#define TRACE_GUI 1
//#define OUTLINE_GUI 1
//#define TRACE_SIZE 1
//#define TRACE_RENDER 1

#if TRACE_GUI
#define OUTLINE_GUI 1
#endif

namespace corona
{
	namespace win32 
	{

		class controller;

		class directApplicationWin32 : public win32ControllerHost
		{
		protected:

			bool controllerLoaded;

			std::shared_ptr<controller> currentController;

			static directApplicationWin32* current;
			static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			virtual LRESULT windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			HINSTANCE hinstance;
			HWND hwndRoot;
			std::list<int> pressedKeys;

			__int64 performanceFrequency;
			__int64 startCounter, lastCounter;
			point minimumWindowSize;

			bool colorCapture;
			int iconResourceId;

			HFONT	controlFont,
					labelFont,
					titleFont;

			double dpiScale;

			void loadStyleSheet();

			bool disableChangeProcessing;
			std::weak_ptr<adapterSet> factory;

		public:

			directApplicationWin32(std::weak_ptr<adapterSet>  _factory);
			virtual ~directApplicationWin32();

			HWND getMainWindow() { return hwndRoot;  }
			HWND createWindow( DWORD window_id, LPCTSTR		lpClassName, LPCTSTR		lpWindowName, DWORD       dwStyle, rectangle bounds, LPVOID		lpParam, HFONT		font);
			void destroyWindow( HWND hwnd );

			std::weak_ptr<direct2dChildWindow> createDirect2Window(DWORD control_id, rectangle bounds);
			virtual std::weak_ptr<direct2dChildWindow> getDirect2dWindow(relative_ptr_type ctrlId);

			virtual bool runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController);
			virtual bool runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, std::shared_ptr<controller> _firstController);

			virtual void setController(std::shared_ptr<controller> _newCurrentController);

			HFONT createFont(const char* _fontName, double fontSize, bool bold, bool italic);

			// general
			virtual void redraw();
			virtual void setVisible(int controlId, bool visible);
			virtual void setEnable(int controlId, bool enabled);
			virtual void setFocus(int ddlControlId);

			virtual rectangle getWindowClientPos();
			virtual rectangle getWindowPos(int ddlControlId);
			virtual void setWindowPos(int ddlControlId, rectangle rect);
			virtual void setMinimumWindowSize(point size);

			// icon control
			virtual void setPictureIcon(int controlId, dtoIconId iconId);
			virtual void setButtonIcon(int controlId, dtoIconId iconId);

			//text
			virtual void setEditText(int textControlId, const std::string& _string);
			virtual void setEditText(int textControlId, const char* _string);
			virtual std::string getEditText(int textControlId);

			// drop downs
			virtual void clearComboItems(int ddlControlId);
			virtual void addComboItem(int ddlControlId, std::string& _src, int _data);
			virtual void addComboItem(int ddlControlId, const char* _text, int _data);
			virtual std::string getComboSelectedText(int ddlControlId);
			virtual int getComboSelectedIndex(int ddlControlId);
			virtual int getComboSelectedValue(int ddlControlId);
			virtual void setComboSelectedIndex(int ddlControlId, int index);
			virtual void setComboSelectedText(int ddlControlId, std::string& _text);
			virtual void setComboSelectedText(int ddlControlId, const char* _text);
			virtual void setComboSelectedValue(int ddlControlId, int value);
			virtual void addFoldersToCombo(int ddlControlId, const char* _path);
			virtual void addPicturesFoldersToCombo(int ddlControlId);

			// ListBox
			virtual std::string getListSelectedText(int ddlControlId);
			virtual int getListSelectedValue(int ddlControlId);
			virtual int getListSelectedIndex(int ddlControlId);
			virtual void setListSelectedIndex(int ddlControlId, int index);
			virtual void setListSelectedText(int ddlControlId, std::string& _text);
			virtual void setListSelectedText(int ddlControlId, const char* _text);
			virtual void setListSelectedValue(int ddlControlId, int value);
			virtual void clearListItems(int ddlControlId);
			virtual void addListItem(int ddlControlId, std::string& _text, int _data);
			virtual void addListItem(int ddlControlId, const char* _text, int _data);
			virtual void addFoldersToList(int ddlControlId, const char* _path);
			virtual void addPicturesFoldersToList(int ddlControlId);

			// buttons
			virtual void setButtonChecked(int controlId, bool enabled);
			virtual bool getButtonChecked(int controlId);

			// list view
			virtual void clearListView(int ddlControlId);
			virtual void addListViewItem(int ddlControlId, std::string& _text, LPARAM _data);
			virtual void addListViewItem(int ddlControlId, const char* _text, LPARAM _data);
			virtual void addListViewColumn(int ddlControlId, int column_id, const char* _text, int _width, visual_alignment _alignment);
			virtual void addListViewRow(int ddlControlId, LPARAM data, const std::vector<std::string>& _items);

			virtual std::string getListViewSelectedText(int ddlControlId);
			virtual int getListViewSelectedIndex(int ddlControlId);
			virtual LPARAM getListViewSelectedValue(int ddlControlId);
			virtual std::list<std::string> getListViewSelectedTexts(int ddlControlId);
			virtual std::list<int> getListViewSelectedIndexes(int ddlControlId);
			virtual std::list<LPARAM> getListViewSelectedValues(int ddlControlId);

			virtual void setListViewSelectedIndex(int ddlControlId, int indexId);
			virtual void setListViewSelectedText(int ddlControlId, std::string& _text);
			virtual void setListViewSelectedText(int ddlControlId, const char* _text);
			virtual void setListViewSelectedValue(int ddlControlId, LPARAM value);
			virtual void clearListViewSelection(int ddlControlId);

			// scroller
			virtual void setScrollHeight(int ddlControlId, int height);
			virtual void setScrollWidth(int ddlControlId, int width);
			virtual point getScrollPos(int ddlControlId);
			virtual point getScrollTrackPos(int ddlControlId);
			virtual point getScrollRange(int ddlControlId);
			virtual void setScrollPos(int ddlControlId, point pt);

			// spinner
			virtual void setSpinRange(int ddlControlId, int lo, int high);
			virtual void setSpinPos(int ddlControlId, int pos);

			virtual void setSysLinkText(int ddlControlId, const char* _text);
			std::vector<std::string> readInternet(const char* _domain, const char* _path);

			// utility
			char* getLastChar(char* _str);
			bool getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension);

			virtual void setColorCapture(int _iconResourceId);

		};

		void EnableGuiStdOuts();

	}
}

