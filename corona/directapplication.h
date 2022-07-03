#pragma once

namespace corona
{
	namespace win32 
	{
		void throwOnFail(HRESULT hr, const char* _message);

		class direct2dFactory {
			ID2D1Factory* d2DFactory;
			IWICImagingFactory* wicFactory;
			IDWriteFactory* dWriteFactory;

		public:

			direct2dFactory();
			virtual ~direct2dFactory();

			inline ID2D1Factory* getD2DFactory() { return d2DFactory; }
			inline IWICImagingFactory* getWicFactory() { return wicFactory; }
			inline IDWriteFactory* getDWriteFactory() { return dWriteFactory; }

		};

		class bitmap;
		class deviceDependentAssetBase;
		class path;
		class textStyle;

		struct PBGRAPixel {
			unsigned char blue, green, red, alpha;
		};

		class direct2dContext : public drawableHost {
		protected:

			SIZE size;
			D2D1_SIZE_F size_dips;
			D2D1_SIZE_U size_pixels;

			std::map<std::string, bitmap*> bitmaps;
			std::map<std::string, deviceDependentAssetBase*> brushes;
			std::map<std::string, path*> paths;
			std::map<std::string, textStyle*> textStyles;
			std::map<std::string, viewStyleRequest> viewStyles;

			virtual void beginDraw();
			virtual void endDraw();

			ID2D1BitmapRenderTarget* bitmapRenderTarget;
			ID2D1HwndRenderTarget* hwndRenderTarget;
			ID2D1RenderTarget* renderTarget;

			direct2dFactory* factory;

			OPENFILENAMEA ofn;

		protected:

			direct2dContext(direct2dFactory* _factory);
			virtual ~direct2dContext();

			void text_style_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void box_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void box_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void shape_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void shape_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);

		public:

			inline direct2dFactory* getFactory() { return factory; }
			inline ID2D1RenderTarget* getRenderTarget() { return renderTarget; }

			virtual point getLayoutSize();
			virtual point getSize();

			virtual void clear(color* _color);

			virtual void addViewStyle(viewStyleRequest& _textStyle);
			virtual void clearViewStyles();

			virtual void addBitmap(bitmapRequest* _bitmap);
			virtual bool getBitmapSize(bitmapRequest* _bitmap, point* _size);
			virtual color getColorAtPoint(bitmapInstanceDto* _bitmap, point& _point);
			virtual bool setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize);
			virtual bool setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter);

			virtual void addBitmapBrush(bitmapBrushRequest* _bitmapBrush);
			virtual void addSolidColorBrush(solidBrushRequest* _solidBrushDto);
			virtual void addLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto);
			virtual void addRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto);
			virtual void clearBitmapsAndBrushes(bool deleteStockObjects);

			virtual void addPath(pathDto* _pathDto, bool _closed);
			virtual void clearPaths();

			virtual void addTextStyle(textStyleRequest* _textStyle);
			virtual void clearTextStyles();

			virtual void popCamera();
			virtual void pushCamera(point* _position, float _rotation, float _scale = 1.0);

			virtual void drawPath(pathInstance2dDto* _pathInstanceDto);
			virtual void drawPath(pathImmediateDto* _pathImmediateDto);
			virtual void drawText(drawTextRequest* _textInstanceDto);
			virtual void drawBitmap(bitmapInstanceDto* _bitmapInstanceDto);

			virtual void drawLine(database::point* start, database::point* stop, const char* _fillBrush, double thickness);
			virtual void drawRectangle(database::rectangle* _rectangle, const char* _borderBrush, double _borderWidth, const char* _fillBrush);
			virtual void drawText(const char* _text, database::rectangle* _rectangle, const char* _textStyle, const char* _fillBrush);
			virtual database::rectangle getCanvasSize();

			virtual bool createRenderTarget() = 0;
			virtual void destroyRenderTarget();

			virtual drawableHost* createBitmap(point& _size);
			virtual void drawBitmap(drawableHost* _directBitmap, point& _dest, point& _size);
			virtual void save(const char* _filename);

			virtual void drawView(const char* _style, const char *_text, rectangle& _rect, const char *_debug_comment);

		protected:

			std::stack<D2D1::Matrix3x2F> transforms;
			D2D1::Matrix3x2F currentTransform;
			path* createPath(pathDto* _pathDto, bool _closed);

			bool createRenderTarget(HWND hwnd);
			bool createRenderTarget(ID2D1RenderTarget* renderTarget, D2D1_SIZE_F _size);

		public:

			friend class textStyle;
			friend class bitmap;
			friend class bitmapBrush;
			friend class solidColorBrush;
			friend class radialGradientBrush;
			friend class linearGradientBrush;
			friend class path;
		};

		class directApplication;

		class directBitmap : public direct2dContext
		{
		private:
			ID2D1RenderTarget* wicTarget;
			IWICBitmap* wicBitmap;

			directBitmap(direct2dFactory* _factory, D2D1_SIZE_F _size);
			virtual ~directBitmap();

		protected:
			virtual bool createRenderTarget();
			virtual void destroyRenderTarget();

		public:
			D2D1_SIZE_F size;
			IWICBitmap* getBitmap();
			void save(const char* _filename);
			friend class direct2dContext;

			virtual bool isBitmap() { return true; }

		};

		class directApplication : public direct2dContext, public controllerHost
		{
		protected:
			bool controllerLoaded;

			controller	*currentController,
						*previousController;

			static directApplication* current;
			static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			static LRESULT CALLBACK d2dWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			virtual LRESULT d2dWindowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			virtual LRESULT windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			HINSTANCE hinstance;
			HWND hwndRoot, hwndDirect2d;
			std::list<int> pressedKeys;

			__int64 performanceFrequency;
			__int64 startCounter, lastCounter;
			point minimumWindowSize;

			bool colorCapture;
			int iconResourceId;

			struct windowMapItem
			{
				HWND				window;
				database::page_item item;
			};

			std::map<int, windowMapItem>	windowControlMap;

			HFONT	controlFont,
					labelFont,
					titleFont;

			double dpiScale;

			void loadStyleSheet();

			HFONT createFontFromStyleSheet(relative_ptr_type _style_id);

			void createChildWindow(
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

		public:

			directApplication(direct2dFactory* _factory);
			virtual ~directApplication();

			int renderPage(database::page& _page, database::jschema* _schema, database::actor_state& _state, database::jcollection& _collection);

			virtual drawableHost* getDrawable(int i);

			virtual bool runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController);
			virtual bool runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController);
			virtual bool createRenderTarget();

			virtual void setController(controller* _newCurrentController);
			virtual void pushController(controller* _newCurrentController);
			virtual void popController();

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

