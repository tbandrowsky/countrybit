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

		class directApplicationWin32 : public win32ControllerHost
		{
		protected:
			bool controllerLoaded;

			controller	*currentController,
						*previousController;

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

			struct windowMapItem
			{
				HWND				window;
			};

			std::map<page_item_identifier, windowMapItem> 
				windowControlMap, 
				oldWindowControlMap;

			std::map<int, page_item> message_map;

			HFONT	controlFont,
					labelFont,
					titleFont;

			double dpiScale;

			void loadStyleSheet();

			HFONT createFontFromStyleSheet(relative_ptr_type _style_id);

			bool createChildWindow(
				page_item_identifier pii,
				LPCTSTR		lpClassName,
				LPCTSTR		lpWindowName,
				DWORD       dwStyle,
				int         x,
				int         y,
				int         nWidth,
				int         nHeight,
				int			windowId,
				LPVOID		lpParam,
				HFONT		font,
				database::page_item item
			);

			void destroyChildren();

			bool disableChangeProcessing;
			adapterSet* factory;

		public:

			directApplicationWin32(adapterSet* _factory);
			virtual ~directApplicationWin32();

			int renderPage(database::page& _page, database::jschema* _schema, database::actor_state& _state, database::jcollection& _collection);

			virtual drawableHost* getDrawable(relative_ptr_type ctrlId);
			virtual direct2dChildWindow* getWindow(relative_ptr_type ctrlId);

			virtual bool runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController);
			virtual bool runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController);

			virtual void setController(controller* _newCurrentController);
			virtual void pushController(controller* _newCurrentController);
			virtual void popController();

			// general
			virtual void redraw();
			virtual void redraw(int controlId);
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
			virtual void setEditText(int textControlId, std::string& _string);
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

			// buttons
			virtual void setButtonChecked(int controlId, bool enabled);
			virtual bool getButtonChecked(int controlId);

			// list view
			virtual void clearListView(int ddlControlId);
			virtual void addListViewItem(int ddlControlId, std::string& _text, int _data);
			virtual void addListViewItem(int ddlControlId, const char* _text, int _data);
			virtual std::string getListViewSelectedText(int ddlControlId);
			virtual int getListViewSelectedIndex(int ddlControlId);
			virtual int getListViewSelectedValue(int ddlControlId);
			virtual std::list<std::string> getListViewSelectedTexts(int ddlControlId);
			virtual std::list<int> getListViewSelectedIndexes(int ddlControlId);
			virtual std::list<int> getListViewSelectedValues(int ddlControlId);

			virtual void setListViewSelectedIndex(int ddlControlId, int indexId);
			virtual void setListViewSelectedText(int ddlControlId, std::string& _text);
			virtual void setListViewSelectedText(int ddlControlId, const char* _text);
			virtual void setListViewSelectedValue(int ddlControlId, int value);
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

