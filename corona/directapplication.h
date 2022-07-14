#pragma once

namespace corona
{
	namespace win32 
	{
		void throwOnFail(HRESULT hr, const char* _message);

		class direct3dWindowInstance
		{
		public:

		};

		class direct3dDevice
		{
			ID3D11Device*		d3d11Device;
			D3D_FEATURE_LEVEL	feature_level;

		public:

			direct3dDevice();
			~direct3dDevice();

			bool setDevice(IDXGIAdapter1* _adapter);
			inline ID3D11Device* getD3DDevice() { return d3d11Device; }
			inline D3D_FEATURE_LEVEL getFeatureLevel() { return feature_level;  }
		};

		class direct2dContext;
		class direct2dWindow;
		class direct2dChildWindow;
		class direct2dBitmap;

		class direct2dDevice
		{
			IDXGIDevice* dxDevice;
			ID2D1Device* d2dDevice;
			ID2D1Factory1* d2DFactory;
			IWICImagingFactory* wicFactory;
			IDWriteFactory* dWriteFactory;

		public:

			direct2dDevice();
			~direct2dDevice();

			inline ID2D1Factory1* getD2DFactory() { return d2DFactory; }
			inline IWICImagingFactory* getWicFactory() { return wicFactory; }
			inline IDWriteFactory* getDWriteFactory() { return dWriteFactory; }
			inline ID2D1Device* getD2DDevice() { return d2dDevice; }

			bool setDevice(ID3D11Device* _d3dDevice);
			void release();
		};

		class adapterSet
		{
			IDXGIFactory2* dxFactory;
			IDXGIAdapter1* dxAdapter;

			direct2dDevice *direct2d;
			direct3dDevice *direct3d;

		public:

			adapterSet();
			~adapterSet();

			void cleanup();
			void refresh();

			inline IDXGIFactory2* getDxFactory() { return dxFactory; }
			inline IDXGIAdapter1* getDxAdapter() { return dxAdapter; }

			inline ID2D1Factory* getD2DFactory() { return direct2d->getD2DFactory(); }
			inline IWICImagingFactory* getWicFactory() { return direct2d->getWicFactory(); }
			inline IDWriteFactory* getDWriteFactory() { return direct2d->getDWriteFactory(); }
			inline ID2D1Device* getD2DDevice() { return direct2d->getD2DDevice(); }

			inline ID3D11Device* getD3DDevice() { return direct3d->getD3DDevice(); }
			inline D3D_FEATURE_LEVEL getFeatureLevel() { return direct3d->getFeatureLevel(); }

			std::map<HWND, direct2dWindow*> parent_windows;

			direct2dWindow* createD2dWindow(HWND parent);
			direct2dWindow* getWindow(HWND parent);
			bool containsWindow(HWND parent);
			void closeWindow(HWND hwnd);
			void clearWindows();
			direct2dChildWindow *findChild(relative_ptr_type _child);

			direct2dBitmap* createD2dBitmap(D2D1_SIZE_F size);

			void loadStyleSheet(jobject& sheet);

		};

		class bitmap;
		class deviceDependentAssetBase;
		class path;
		class textStyle;

		struct PBGRAPixel 
		{
			unsigned char blue, green, red, alpha;
		};

		class direct2dContext : public drawableHost {
		protected:

			D2D1_SIZE_F size_dips;
			D2D1_SIZE_U size_pixels;

			std::map<std::string, bitmap*> bitmaps;
			std::map<std::string, deviceDependentAssetBase*> brushes;
			std::map<std::string, path*> paths;
			std::map<std::string, textStyle*> textStyles;
			std::map<std::string, viewStyleRequest> viewStyles;

			adapterSet* factory;

			OPENFILENAMEA ofn;

		protected:

			direct2dContext(adapterSet* _factory);
			virtual ~direct2dContext();

			void text_style_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void box_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void box_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void shape_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);
			void shape_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name);

		public:

			virtual ID2D1DeviceContext* getRenderTarget() = 0;
			virtual adapterSet* getFactory();

			virtual point getLayoutSize();
			virtual point getSize();

			virtual void beginDraw(bool& _adapter_blown_away) = 0;
			virtual void endDraw(bool& _adapter_blown_away) = 0;

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

			virtual drawableHost* createBitmap(point& _size);
			virtual void drawBitmap(drawableHost* _directBitmap, point& _dest, point& _size);
			virtual void save(const char* _filename);

			virtual void drawView(const char* _style, const char *_text, rectangle& _rect, const char *_debug_comment);

			virtual void loadStyleSheet(jobject& style_sheet);

		protected:

			std::stack<D2D1::Matrix3x2F> transforms;
			D2D1::Matrix3x2F currentTransform;
			path* createPath(pathDto* _pathDto, bool _closed);

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

		class direct2dBitmapCore
		{
		protected:
			ID2D1DeviceContext* targetContext;
			ID2D1RenderTarget* target;
			ID2D1Bitmap1* bitmap;

		public:

			D2D1_SIZE_F size;

			direct2dBitmapCore(D2D1_SIZE_F _size, adapterSet* _factory, int _dpi);
			virtual ~direct2dBitmapCore();

			virtual bool isBitmap() { return true; }

			virtual ID2D1DeviceContext* getRenderTarget()
			{
				return targetContext;
			}

			ID2D1Bitmap1* getBitmap() {
				return bitmap;
			}

			virtual void beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

		};

		class direct2dBitmap : public direct2dContext
		{
			ID2D1DeviceContext* targetContext;
			ID2D1RenderTarget* target;
			IWICBitmap* wicBitmap;

		public:

			D2D1_SIZE_F size;

			direct2dBitmap(D2D1_SIZE_F _size, adapterSet* _factory);
			virtual ~direct2dBitmap();

			IWICBitmap* getBitmap();
			void save(const char* _filename);
			virtual bool isBitmap() { return true; }

			virtual ID2D1DeviceContext* getRenderTarget()
			{
				return targetContext;
			}

			virtual void beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

		};

		class direct2dChildWindow;

		class direct2dWindow : public direct2dContext
		{
		private:

			HWND hwnd;

			// for main window
			ID2D1DeviceContext* renderTarget;
			IDXGISwapChain1* swapChain;
			IDXGISurface* surface;
			ID3D11Texture2D* texture;
			ID2D1Bitmap1* bitmap;

			std::map<relative_ptr_type, direct2dChildWindow*> children;

			void applySwapChain();

		public:

			direct2dWindow(HWND hwnd, adapterSet* _adapter);
			virtual ~direct2dWindow();

			rectangle getBoundsDips();

			void resize(UINT x, UINT y);
			void moveWindow(UINT x, UINT y, UINT h, UINT w);

			virtual ID2D1DeviceContext* getRenderTarget()
			{
				return renderTarget;
			}

			ID2D1Bitmap1* getBitmap() { return bitmap; }

			virtual void beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

			HWND getWindow() { return hwnd; }

			direct2dChildWindow* createChild(relative_ptr_type _id, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);
			direct2dChildWindow* getChild(relative_ptr_type _id);
			direct2dChildWindow* deleteChild(relative_ptr_type _id);

			auto& getChildren() { return children; }
		};

		class direct2dChildWindow : public direct2dContext
		{

		private:

			direct2dWindow* parent;
			direct2dBitmapCore* childBitmap;
			rectangle windowPosition;

			direct2dChildWindow(direct2dWindow* _parent, adapterSet* _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);
			virtual ~direct2dChildWindow();

		public:

			friend class direct2dWindow;

			rectangle getBoundsDips();

			void resize(UINT _wdips, UINT _hdips);
			void moveWindow(UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);

			virtual ID2D1DeviceContext* getRenderTarget()
			{
				return childBitmap->getRenderTarget();
			}

			ID2D1Bitmap1* getBitmap() { return childBitmap->getBitmap(); }

			virtual void beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);
		};

		class directApplication : public controllerHost
		{
		protected:
			bool controllerLoaded;

			controller	*currentController,
						*previousController;

			static directApplication* current;
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

			void createChildWindow(
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

			directApplication(adapterSet* _factory);
			virtual ~directApplication();

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

