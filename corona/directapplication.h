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

			std::map<std::string, bitmap*> bitmaps;
			std::map<std::string, deviceDependentAssetBase*> brushes;
			std::map<std::string, path*> paths;
			std::map<std::string, textStyle*> textStyles;

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

		public:

			inline direct2dFactory* getFactory() { return factory; }
			inline ID2D1RenderTarget* getRenderTarget() { return renderTarget; }

			virtual sizeIntDto getSize();

			virtual void clear(colorDto* _color);
			virtual void addBitmap(bitmapDto* _bitmap);
			virtual bool getBitmapSize(bitmapDto* _bitmap, sizeIntDto* _size);
			virtual colorDto getColorAtPoint(bitmapInstanceDto* _bitmap, pointDto& _point);
			virtual bool setBitmapSizes(bitmapDto* _bitmap, bool _forceResize);
			virtual bool setBitmapFilter(bitmapDto* _bitmap, std::function<bool(sizeIntDto, int, int, char* bytes)> _filter);

			virtual void addBitmapBrush(bitmapBrushDto* _bitmapBrush);
			virtual void addSolidColorBrush(solidBrushDto* _solidBrushDto);
			virtual void addLinearGradientBrush(linearGradientBrushDto* _linearGradientBrushDto);
			virtual void addRadialGradientBrush(radialGradientBrushDto* _radialGradientBrushDto);
			virtual void clearBitmapsAndBrushes(bool deleteStockObjects);

			virtual void addPath(pathDto* _pathDto, bool _closed);
			virtual void clearPaths();

			virtual void addTextStyle(textStyleDto* _textStyle);
			virtual void clearTextStyles();

			virtual void popCamera();
			virtual void pushCamera(pointDto* _position, float _rotation, float _scale = 1.0);

			virtual void drawPath(pathInstance2dDto* _pathInstanceDto);
			virtual void drawPath(pathImmediateDto* _pathImmediateDto);
			virtual void drawText(textInstance2dDto* _textInstanceDto);
			virtual void drawBitmap(bitmapInstanceDto* _bitmapInstanceDto);

			virtual bool createRenderTarget() = 0;
			virtual void destroyRenderTarget();

			virtual drawableHost* createBitmap(sizeIntDto& _size);
			virtual void drawBitmap(drawableHost* _directBitmap, pointDto& _dest, sizeIntDto& _size);
			virtual void save(const char* _filename);

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

		class view 
		{
		public:

		};

		class field_map
		{
		public:
			database::relative_ptr_type field_id;
			database::jslice			slice;
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
			sizeIntDto minimumWindowSize;

			bool colorCapture;
			int iconResourceId;

			std::map<int, field_map>	windowControlMap;
			std::vector<HWND>			childWindows;

			void destroyChildren();

			void createChildWindow(
				LPCTSTR		lpClassName,
				LPCTSTR		lpWindowName,
				DWORD       dwStyle,
				int         x,
				int         y,
				int         nWidth,
				int         nHeight,
				int			windowId,
				LPVOID		lpParam
			);

			void createChildWindow(
				database::jslice slice,
				int			field_id,
				LPCTSTR		lpClassName,
				LPCTSTR		lpWindowName,
				DWORD       dwStyle,
				int         x,
				int         y,
				int         nWidth,
				int         nHeight,
				LPVOID		lpParam
			);

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

			virtual rectDto getWindowPos(int ddlControlId);
			virtual void setWindowPos(int ddlControlId, rectDto rect);
			virtual void setMinimumWindowSize(sizeIntDto size);

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
			virtual pointDto getScrollPos(int ddlControlId);
			virtual pointDto getScrollTrackPos(int ddlControlId);
			virtual sizeIntDto getScrollRange(int ddlControlId);
			virtual void setScrollPos(int ddlControlId, pointDto pt);

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

	}
}

