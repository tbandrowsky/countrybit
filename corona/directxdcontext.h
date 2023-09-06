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

		class bitmap;
		class deviceDependentAssetBase;
		class path;
		class textStyle;

		struct PBGRAPixel
		{
			unsigned char blue, green, red, alpha;
		};

		class direct2dBitmap;

		class direct2dContext : public drawableHost, public std::enable_shared_from_this<direct2dContext>
		{
		protected:

			std::map<std::string, std::shared_ptr<bitmap>> bitmaps;
			std::map<std::string, std::shared_ptr<deviceDependentAssetBase>> brushes;
			std::map<std::string, std::shared_ptr<path>> paths;
			std::map<std::string, std::shared_ptr<textStyle>> textStyles;
			std::map<std::string, std::shared_ptr<viewStyleRequest>> viewStyles;

			std::weak_ptr<adapterSet> factory;
			CComPtr<ID2D1DeviceContext> context;

		protected:

			void view_style_name(const object_name& _style_sheet_name, object_name& _object_style_name, int _index);
			void text_style_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index);
			void box_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index);
			void box_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index);
			void shape_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index);
			void shape_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index);

		public:
			direct2dContext(std::weak_ptr<adapterSet> _factory);
			direct2dContext(std::weak_ptr<corona::win32::adapterSet> _factory, ID2D1DeviceContext* _context);

			virtual ~direct2dContext();

			virtual std::weak_ptr<adapterSet> getFactory();
			virtual CComPtr<ID2D1DeviceContext>& getDeviceContext()
			{
				return context;
			}

			CComPtr<ID2D1DeviceContext> beginDraw(bool& _adapter_blown_away);
			HRESULT endDraw(bool& _adapter_blown_away);

			virtual void clear(color* _color);

			virtual void setViewStyle(viewStyleRequest& _textStyle);
			virtual void clearViewStyles();

			virtual std::string setBitmap(bitmapRequest* _bitmap);
			virtual bool getBitmapSize(bitmapRequest* _bitmap, point* _size);
			virtual color getColorAtPoint(bitmapInstanceDto* _bitmap, point& _point);
			virtual bool setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize);
			virtual bool setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter);

			virtual std::string setBitmapBrush(bitmapBrushRequest* _bitmapBrush);
			virtual std::string setSolidColorBrush(solidBrushRequest* _solidBrushDto);
			virtual std::string setLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto);
			virtual std::string setRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto);
			virtual void clearBitmapsAndBrushes(bool deleteStockObjects);

			virtual std::string  setPath(pathDto* _pathDto, bool _closed);
			virtual void clearPaths();

			virtual void setTextStyle(textStyleRequest* _textStyle);
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

			virtual std::shared_ptr<direct2dBitmap> createBitmap(point& _size);
			virtual void drawBitmap(drawableHost* _directBitmap, point& _dest, point& _size);
			virtual void save(const char* _filename);

			virtual void drawView(const char* _style, const char* _text, rectangle& _rect, int _state, const char* _debug_comment);

			virtual void loadStyleSheet(jobject& style_sheet, int _style_state);

		protected:

			std::stack<D2D1::Matrix3x2F> transforms;
			D2D1::Matrix3x2F currentTransform;
			std::shared_ptr<path> createPath(pathDto* _pathDto, bool _closed);

		public:

			friend class textStyle;
			friend class bitmap;
			friend class bitmapBrush;
			friend class solidColorBrush;
			friend class radialGradientBrush;
			friend class linearGradientBrush;
			friend class path;

			friend class direct2dWindow;
		};

		class directApplicationWin32;

		class direct2dBitmapCore
		{
		protected:
			CComPtr<ID2D1DeviceContext> targetContext;
			ID2D1RenderTarget* target;
			ID2D1Bitmap1* bitmap;

		public:

			D2D1_SIZE_F size;

			direct2dBitmapCore(D2D1_SIZE_F _size_dips, std::weak_ptr<adapterSet> _factory, int _dpi);
			virtual ~direct2dBitmapCore();

			virtual bool isBitmap() { return true; }

			virtual ID2D1DeviceContext* getRenderTarget()
			{
				return targetContext;
			}

			ID2D1Bitmap1* getBitmap() {
				return bitmap;
			}

			virtual CComPtr<ID2D1DeviceContext>  beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

		};

		class direct2dBitmap : public std::enable_shared_from_this<direct2dBitmap>
		{
			ID2D1RenderTarget* target;
			IWICBitmap* wicBitmap;
			std::shared_ptr<direct2dContext> context;

		public:

			D2D1_SIZE_F size;

			direct2dBitmap(D2D1_SIZE_F _size, std::weak_ptr<adapterSet>& _factory);
			virtual ~direct2dBitmap();

			IWICBitmap* getBitmap();
			void save(const char* _filename);
			virtual bool isBitmap() { return true; }

			virtual direct2dContext& getContext()
			{
				return *context.get();
			}

			virtual CComPtr<ID2D1DeviceContext>  beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

		};

		class direct2dChildWindow;

		class direct2dWindow : public std::enable_shared_from_this<direct2dWindow>
		{
		private:

			HWND hwnd;
			std::shared_ptr<direct2dContext> context;

			// for main window
			IDXGISwapChain1* swapChain;
			IDXGISurface* surface;
			ID3D11Texture2D* texture;
			ID2D1Bitmap1* bitmap;

			std::map<relative_ptr_type, std::shared_ptr<direct2dChildWindow>> children;

			void applySwapChain(UINT w, UINT h);

		public:

			direct2dWindow(HWND hwnd, std::weak_ptr<adapterSet> _adapter, color _background_color);
			virtual ~direct2dWindow();

			rectangle getBoundsDips();

			void resize(UINT x, UINT y);
			void moveWindow(UINT x, UINT y, UINT h, UINT w);

			virtual direct2dContext& getContext()
			{
				return *context.get();
			}

			ID2D1Bitmap1* getBitmap() { return bitmap; }

			virtual CComPtr<ID2D1DeviceContext>  beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);

			HWND getWindow() { return hwnd; }

			virtual std::weak_ptr<direct2dChildWindow> createChild(relative_ptr_type _id, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);
			virtual std::weak_ptr<direct2dChildWindow> getChild(relative_ptr_type _id);
			virtual void deleteChild(relative_ptr_type _id);

			auto& getChildren() { return children; }
		};

		class direct2dChildWindow 
		{

		private:

			std::weak_ptr<direct2dWindow> parent;
			std::shared_ptr<direct2dBitmapCore> childBitmap;
			std::shared_ptr<direct2dContext> context;
			rectangle windowPosition;


		public:

			direct2dChildWindow(std::weak_ptr<direct2dWindow> _parent, std::weak_ptr<adapterSet> _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);
			virtual ~direct2dChildWindow();

			friend class direct2dWindow;

			rectangle getBoundsDips();

			void resize(UINT _wdips, UINT _hdips);
			void moveWindow(UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);

			int zOrder;

			ID2D1Bitmap1* getBitmap() { return childBitmap->getBitmap(); }

			virtual direct2dContext& getContext()
			{
				return *context.get();
			}

			virtual CComPtr<ID2D1DeviceContext>  beginDraw(bool& _adapter_blown_away);
			virtual void endDraw(bool& _adapter_blown_away);
		};

		class deviceDependentAssetBase {
		public:

			bool stock;

			deviceDependentAssetBase();
			virtual ~deviceDependentAssetBase();
			virtual bool create(direct2dContext* target) = 0;
			virtual void release() = 0;

			virtual ID2D1Brush* getBrush();
		};

		template <typename Direct2DAsset> class deviceDependentAsset : public deviceDependentAssetBase {
		protected:
			Direct2DAsset asset;

		public:

			deviceDependentAsset() : asset(NULL)
			{
				;
			}

			virtual ~deviceDependentAsset()
			{
				release();
			}

			deviceDependentAsset(const deviceDependentAsset& _src) : asset(_src.asset)
			{
				if (asset)
				{
					asset->AddRef();
				}
			}

			deviceDependentAsset operator=(const deviceDependentAsset& _src)
			{
				deviceDependentAsset copyasset(_src);
				return *this;
			}

			deviceDependentAsset(deviceDependentAsset&& _src) : asset(_src.asset)
			{
				_src.asset = nullptr;
			}

			deviceDependentAsset &operator=(deviceDependentAsset&& _src)
			{
				asset = _src.asset;
				_src.asset = nullptr;
				return *this;
			}

			virtual bool create(direct2dContext* target) = 0;

			bool recreate(direct2dContext* target)
			{
				release();
				create(target);
			}

			inline Direct2DAsset getAsset() { return asset; }

		protected:

			virtual void release()
			{
				if (asset) asset->Release();
				asset = NULL;
			}
		};

		class textStyle : public deviceDependentAssetBase
		{
			IDWriteTextFormat* lpWriteTextFormat;

			std::string fontName;
			float size;
			bool bold;
			bool italic;
			bool underline;
			bool strike_through;
			double line_spacing;
			visual_alignment horizontal_align;
			visual_alignment vertical_align;
			bool wrap_text;

		public:

			textStyle(std::string _fontName,
				float _size,
				bool _bold,
				bool _italic,
				bool _underline,
				bool _strike_through,
				double _line_spacing,
				visual_alignment _horizontal_align,
				visual_alignment _vertical_align,
				bool _wrap_text) :
				fontName(_fontName),
				size(_size),
				bold(_bold),
				italic(_italic),
				underline(_underline),
				strike_through(_strike_through),
				line_spacing(_line_spacing),
				horizontal_align(_horizontal_align),
				vertical_align(_vertical_align),
				lpWriteTextFormat(NULL)
			{
				;
			}

			virtual ~textStyle()
			{
				release();
			}

			std::string get_fontName() { return fontName; };
			float get_size() { return size; }
			bool get_bold() { return bold; }
			bool get_italic() { return italic; }
			bool get_underline() { return underline; }
			bool get_strike_through() { return strike_through; }
			double get_line_spacing() { return line_spacing; }
			visual_alignment get_horizontal_align() { return horizontal_align; }
			visual_alignment get_vertical_align() { return vertical_align; }
			bool get_wrap_text() { return wrap_text; }

			virtual bool create(direct2dContext * target);
			virtual void release();

			IDWriteTextFormat* getFormat()
			{
				return lpWriteTextFormat;
			}

		};

		class filteredBitmap
		{
			filteredBitmap(direct2dContext* _targetContext, filteredBitmap* _src)
				: size(_src->size),
				cropEnabled(_src->cropEnabled),
				crop(_src->crop),
				filteredScaledBitmap(NULL),
				originalScaledBitmap(_src->originalScaledBitmap),
				wicFilteredScaledBitmap(_src->wicFilteredScaledBitmap),
				originalWidth(_src->originalWidth),
				originalHeight(_src->originalHeight)
			{
				HRESULT hr = wicFilteredScaledBitmap->AddRef();
				throwOnFail(hr, "Could not clone wic filtered bitmap");
				hr = originalScaledBitmap->AddRef();
				throwOnFail(hr, "Could not clone original bitmap");
				make(_targetContext);
			}

		public:
			D2D1_SIZE_U size;
			bool cropEnabled;
			D2D1_RECT_F crop;
			IWICBitmap* originalScaledBitmap,
				* wicFilteredScaledBitmap;
			ID2D1Bitmap* filteredScaledBitmap;
			UINT originalWidth, originalHeight;

			filteredBitmap(D2D1_SIZE_U _size, bool _cropEnabled, D2D1_RECT_F _crop)
				: size(_size),
				cropEnabled(_cropEnabled),
				crop(_crop),
				originalScaledBitmap(NULL),
				wicFilteredScaledBitmap(NULL),
				filteredScaledBitmap(NULL)
			{

			}

			filteredBitmap* clone(direct2dContext* _targetContext)
			{
				return new filteredBitmap(_targetContext, this);
			}

			bool create(direct2dContext* _target, IWICBitmapSource* _source);
			bool create(direct2dContext* _target, HBITMAP _source);
			bool make(direct2dContext* _target);
			virtual ~filteredBitmap();
			void release();

		};

		class brush
		{
		public:
			virtual ID2D1Brush* getBrush() = 0;
		};

		class bitmap : public deviceDependentAssetBase
		{
			bool useFile;
			bool useResource;
			std::string filename;
			int resource_id;

			std::list<filteredBitmap*> filteredBitmaps;
			std::function<bool(point, int, int, char*)> filterFunction;

			void clearFilteredBitmaps();
			void setFilteredBitmaps(std::list<sizeCrop>& _sizes);
			void copyFilteredBitmaps(direct2dContext* _targetContext, bitmap* _src);

			virtual bool create_from_file(direct2dContext *_target);
			virtual bool create_from_resource(direct2dContext* _target);

		public:

			bitmap(direct2dContext* _targetContext, bitmap* _src);
			bitmap(std::string& _filename, std::list<sizeCrop>& _sizes);
			bitmap(int _resouce_id, std::list<sizeCrop>& _sizes);
			virtual ~bitmap();
			virtual std::shared_ptr<bitmap> clone(direct2dContext* _src);
			void setSizes(std::list<sizeCrop>& _sizes);
			bool getSize(int* _sizex, int* _sizey);
			ID2D1Bitmap* getFirst();
			ID2D1Bitmap* getBySize(int _width, int _height);

			color getColorAtPoint(int _width, int _height, point point);
			void setFilter(std::function<bool(point, int, int, char* bytes)> _filter);
			void filter();

			virtual bool applyFilters(direct2dContext* _target);
			virtual bool create(direct2dContext* _target);
			virtual void release();
		};


		class bitmapBrush : public deviceDependentAsset<ID2D1BitmapBrush*>, brush {
		public:

			std::weak_ptr<bitmap> bm;

			bitmapBrush();
			virtual ~bitmapBrush();

			virtual bool create(direct2dContext *target);
			virtual ID2D1Brush* getBrush();
		};

		class solidColorBrush : public deviceDependentAsset<ID2D1SolidColorBrush*>, brush {
		public:

			D2D1_COLOR_F color;

			solidColorBrush();
			virtual ~solidColorBrush();
			bool create(direct2dContext* target);
			ID2D1Brush* getBrush();
		};

		class linearGradientBrush : public deviceDependentAsset<ID2D1LinearGradientBrush*>, brush {
		public:
			std::vector<D2D1_GRADIENT_STOP> stops;
			D2D1_POINT_2F					start,
				stop;

			virtual bool create(direct2dContext* target);
			virtual ID2D1Brush* getBrush();
		};

		class radialGradientBrush : public deviceDependentAsset<ID2D1RadialGradientBrush*>, brush {
		public:
			std::vector<D2D1_GRADIENT_STOP> stops;
			D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialProperties;

			bool create(direct2dContext* target);
			virtual ID2D1Brush* getBrush();
		};

		class path {
		public:
			ID2D1PathGeometry* geometry;
			ID2D1GeometrySink* sink;

			path(std::weak_ptr<direct2dContext>& target);

			virtual ~path();
			void start_figure(D2D1_POINT_2F point);
			void add_line(D2D1_POINT_2F point);
			void add_arc(D2D1_POINT_2F point1, D2D1_SIZE_F size1, FLOAT rotationAngle);
			void add_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2, D2D1_POINT_2F point3);
			void add_quadratic_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2);
			void close_figure(bool closed = true);
		};

		class directBitmapSaveImpl {
		public:

			direct2dBitmap* dBitmap;
			IWICStream* fileStream;
			IWICBitmapEncoder* bitmapEncoder;
			IWICBitmapFrameEncode* bitmapFrameEncode;

			directBitmapSaveImpl(direct2dBitmap* _dbitmap) :
				dBitmap(_dbitmap),
				fileStream(NULL),
				bitmapEncoder(NULL),
				bitmapFrameEncode(NULL)
			{

			}

			virtual ~directBitmapSaveImpl()
			{
				if (fileStream) fileStream->Release();
				if (bitmapEncoder) bitmapEncoder->Release();
				if (bitmapFrameEncode) bitmapFrameEncode->Release();
			}

			virtual void save(const wchar_t* _filename);
		};

	}
}
