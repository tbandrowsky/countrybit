module;

#include "corona_platform.h"

#include <string>
#include <exception>
#include <stdexcept>
#include <format>

export module corona.database:direct2dcontext;
import :assert_if;
import :color_box;
import :point_box;
import :rectangle_box;
import :visual;
import :point_box;


export class direct3dWindowInstance
{
public:

};

export class direct3dDevice : public std::enable_shared_from_this<direct3dDevice>
{
	CComPtr<ID3D11Device> d3d11Device;
	D3D_FEATURE_LEVEL	feature_level;

public:

	direct3dDevice();
	~direct3dDevice();

	bool setDevice(IDXGIAdapter1* _adapter);
	ID3D11Device* getD3DDevice() { return d3d11Device; }
	D3D_FEATURE_LEVEL getFeatureLevel() { return feature_level; }
};

export class direct2dDevice : public std::enable_shared_from_this<direct2dDevice>
{
	IDXGIDevice* dxDevice;
	ID2D1Device* d2dDevice;

	ID2D1Factory1* d2DFactory;
	IWICImagingFactory* wicFactory;
	IDWriteFactory* dWriteFactory;

public:

	direct2dDevice();
	virtual ~direct2dDevice();

	ID2D1Factory1* getD2DFactory() { return d2DFactory; }
	IWICImagingFactory* getWicFactory() { return wicFactory; }
	IDWriteFactory* getDWriteFactory() { return dWriteFactory; }
	ID2D1Device* getD2DDevice() { return d2dDevice; }

	bool setDevice(ID3D11Device* _d3dDevice);
};

export class adapterSet : public std::enable_shared_from_this<adapterSet>
{
	CComPtr<IDXGIFactory2> dxFactory;
	CComPtr<IDXGIAdapter1> dxAdapter;

	std::shared_ptr<direct2dDevice> direct2d;
	std::shared_ptr<direct3dDevice> direct3d;

	std::map<HWND, std::shared_ptr<direct2dWindow>> parent_windows;

public:

	adapterSet();
	virtual ~adapterSet();

	void cleanup();
	void refresh();

	IDXGIFactory2* getDxFactory() { return dxFactory; }
	IDXGIAdapter1* getDxAdapter() { return dxAdapter; }

	ID2D1Factory* getD2DFactory() { return direct2d->getD2DFactory(); }
	IWICImagingFactory* getWicFactory() { return direct2d->getWicFactory(); }
	IDWriteFactory* getDWriteFactory() { return direct2d->getDWriteFactory(); }
	ID2D1Device* getD2DDevice() { return direct2d->getD2DDevice(); }

	ID3D11Device* getD3DDevice() { return direct3d->getD3DDevice(); }
	D3D_FEATURE_LEVEL getFeatureLevel() { return direct3d->getFeatureLevel(); }

	std::weak_ptr<direct2dWindow> createD2dWindow(HWND parent, color backgroundColor);

	std::weak_ptr<direct2dWindow> getWindow(HWND parent);
	bool containsWindow(HWND parent);
	void closeWindow(HWND hwnd);
	void clearWindows();

	std::weak_ptr<direct2dChildWindow> findChild(relative_ptr_type _child);

	std::unique_ptr<direct2dBitmap> createD2dBitmap(D2D1_SIZE_F size);

};

export struct PBGRAPixel
{
	unsigned char blue, green, red, alpha;
};

export class deviceDependentAssetBase {
public:

	bool stock;

	deviceDependentAssetBase();
	virtual ~deviceDependentAssetBase();
	virtual bool create(direct2dContext* target) = 0;
	virtual void release() = 0;

	virtual ID2D1Brush* getBrush();
};

class direct2dContext;

export template <typename Direct2DAsset> class deviceDependentAsset : public deviceDependentAssetBase {
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

	deviceDependentAsset& operator=(deviceDependentAsset&& _src)
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

export class textStyle : public deviceDependentAssetBase
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

	virtual bool create(direct2dContext* target);
	virtual void release();

	IDWriteTextFormat* getFormat()
	{
		return lpWriteTextFormat;
	}

};

export class filteredBitmap
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

export class brush
{
public:
	virtual ID2D1Brush* getBrush() = 0;
};

export class bitmap : public deviceDependentAssetBase
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

	virtual bool create_from_file(direct2dContext* _target);
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


export class bitmapBrush : public deviceDependentAsset<ID2D1BitmapBrush*>, brush {
public:

	std::weak_ptr<bitmap> bm;

	bitmapBrush();
	virtual ~bitmapBrush();

	virtual bool create(direct2dContext* target);
	virtual ID2D1Brush* getBrush();
};

export class solidColorBrush : public deviceDependentAsset<ID2D1SolidColorBrush*>, brush {
public:

	D2D1_COLOR_F color;

	solidColorBrush();
	virtual ~solidColorBrush();
	bool create(direct2dContext* target);
	ID2D1Brush* getBrush();
};

export class linearGradientBrush : public deviceDependentAsset<ID2D1LinearGradientBrush*>, brush {
public:
	std::vector<D2D1_GRADIENT_STOP> stops;
	D2D1_POINT_2F					start,
		stop;

	virtual bool create(direct2dContext* target);
	virtual ID2D1Brush* getBrush();
};

export class radialGradientBrush : public deviceDependentAsset<ID2D1RadialGradientBrush*>, brush {
public:
	std::vector<D2D1_GRADIENT_STOP> stops;
	D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialProperties;

	bool create(direct2dContext* target);
	virtual ID2D1Brush* getBrush();
};

export class path {
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

export class nullFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		return true;
	}
};

export class whiteFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		PBGRAPixel* base = (PBGRAPixel*)pv;

		for (int r = 0; r < _size.y; r++) {
			auto row = (PBGRAPixel*)(pv + cbStride * r);
			auto rowo = (PBGRAPixel*)(pv + cbStride * (int)(_size.y - (r + 1)));
			for (int x = 0; x < _size.x; x++) {
				auto pix = row[x];
				if (pix.alpha == 0) {
					pix.green = 255;
					pix.blue = 255;
					pix.red = 255;
				}
				row[x] = pix;
			}
		}

		return true;
	}
};

export class testFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		PBGRAPixel* base = (PBGRAPixel*)pv;

		for (int r = 0; r < _size.y; r++) {
			auto row = (PBGRAPixel*)(pv + cbStride * r);
			auto rowo = (PBGRAPixel*)(pv + cbStride * (int)(_size.y - (r + 1)));
			for (int x = 0; x < _size.x; x++) {
				auto pix = row[x];
				pix.alpha = 255;
				if (x > _size.x / 2)
					pix.blue = 255;
				else
					pix.green = 255;
				row[x] = pix;
			}
		}

		return true;
	}
};


export class direct2dBitmapCore
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

export class direct2dBitmap : public std::enable_shared_from_this<direct2dBitmap>
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

export class direct2dContext : public drawableHost, public std::enable_shared_from_this<direct2dContext>
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


export class direct2dWindow : public std::enable_shared_from_this<direct2dWindow>
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

export class direct2dChildWindow
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
export nullFilterFunction defaultfilter;
export whiteFilterFunction whitefilter;
export testFilterFunction testfilter;


direct2dWindow::direct2dWindow(HWND _hwnd, std::weak_ptr<adapterSet> _adapterSet, color _background_color ) 
{
	HRESULT hr;

	hwnd = _hwnd;
	context = std::make_shared<direct2dContext>(_adapterSet);
	bitmap = nullptr;
	surface = nullptr;
	swapChain = nullptr;

	RECT rect;
	::GetClientRect(hwnd, &rect);
	int width = abs(rect.right - rect.left);
	int height = abs(rect.bottom - rect.top);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2;                     // use double buffering to enable flip
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = 0;

	if (auto padapter = _adapterSet.lock())
	{
		hr = padapter->getDxFactory()->CreateSwapChainForHwnd(padapter->getD3DDevice(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
		throwOnFail(hr, "Could not create swap chain");

		DXGI_RGBA color;
		color.a = _background_color.a;
		color.r = _background_color.r;
		color.g = _background_color.g;
		color.b = _background_color.b;

		swapChain->SetBackgroundColor(&color);

		applySwapChain(width, height);
	}
}

void direct2dWindow::resize(UINT width, UINT height)
{
	HRESULT hr;

#if TRACE_RENDER
	std::cout << "%%%%%%%%% parent resize " << GetDlgCtrlID(hwnd) << " " << width << " " << height << std::endl;
#endif

	context->getDeviceContext()->SetTarget(nullptr);

	if (bitmap)
	{
		bitmap->Release();
		bitmap = nullptr;
	}
	if (surface)
	{
		surface->Release();
		surface = nullptr;
	}

	applySwapChain(width, height);
}

void direct2dWindow::moveWindow(UINT x, UINT y, UINT w, UINT h)
{
	double dpi = ::GetDpiForWindow(hwnd);

	MoveWindow(hwnd, x, y, w, h, false);
}

void direct2dWindow::applySwapChain(UINT w, UINT h)
{
	HRESULT hr;

	float dpix, dpiy;
	int dpiWindow;
	RECT r;

	dpiWindow = ::GetDpiForWindow(hwnd);
	context->getDeviceContext()->GetDpi(&dpix, &dpiy);

	int x = w;
	int y = h;

	hr = swapChain->ResizeBuffers(2, x, y, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
	throwOnFail(hr, "Couldn't resize swapchain");

	// Now we set up the Direct2D render target bitmap linked to the swapchain. 
	// Whenever we render to this bitmap, it is directly rendered to the 
	// swap chain associated with the window.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};

	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bitmapProperties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	bitmapProperties.colorContext = nullptr;
	bitmapProperties.dpiX = dpiWindow;
	bitmapProperties.dpiY = dpiWindow;

	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	hr = swapChain->GetBuffer(0, IID_IDXGISurface, (void**)&surface);
	throwOnFail(hr, "Could not get swap chain surface");

	DXGI_SURFACE_DESC sdec;
	surface->GetDesc(&sdec);

	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	hr = context->getDeviceContext()->CreateBitmapFromDxgiSurface(
		surface,
		&bitmapProperties,
		&bitmap
	);

	auto pxsx = bitmap->GetPixelSize();
	auto sizex = bitmap->GetSize();

	throwOnFail(hr, "Could create bitmap from surface");

	// Now we can set the Direct2D render target.

	context->getDeviceContext()->SetDpi(dpiWindow, dpiWindow);
	context->getDeviceContext()->SetTarget(bitmap);
	throwOnFail(hr, "Could not set device context target");

	// just look at the sizes here and see if that much lines up.
	auto dipssz = context->getDeviceContext()->GetSize();
	auto pixssz = context->getDeviceContext()->GetPixelSize();

#if TRACE_RENDER
	std::cout << "render target pixel size " << pixssz.width << " " << pixssz.height << " " << std::endl;
	std::cout << "render target dip size " << dipssz.width << " " << dipssz.height << " " << std::endl;
#endif
	return;

}

rectangle direct2dWindow::getBoundsDips()
{
	const rectangle r;

	float dpiScale = 96.0 / GetDpiForWindow(hwnd);
	rectangle rx;

	RECT rct;
	GetClientRect(hwnd, &rct);

	rx.x = 0;
	rx.y = 0;
	rx.w = (rct.right - rct.left) * dpiScale;
	rx.h = (rct.bottom - rct.top) * dpiScale;
	return rx;
}

direct2dWindow::~direct2dWindow()
{
	children.clear();

	if (bitmap)
		bitmap->Release();
	if (surface)
		surface->Release();
	if (swapChain)
		swapChain->Release();
}

std::weak_ptr<direct2dChildWindow> direct2dWindow::createChild(relative_ptr_type _id, UINT _x, UINT _y, UINT _w, UINT _h)
{
	if (!children.contains(_id)) {
		auto pthis = weak_from_this();
		auto new_ptr = std::make_shared<direct2dChildWindow>(pthis, getContext().getFactory(), _x, _y, _w, _h);
		children.insert_or_assign(_id, new_ptr);
	}

	auto child = children[_id];
	child->moveWindow(_x, _y, _w, _h);

	return child;
}

std::weak_ptr<direct2dChildWindow> direct2dWindow::getChild(relative_ptr_type _id)
{
	std::weak_ptr<direct2dChildWindow> child;
	if (children.contains(_id)) {
		child = children[_id];
	}
	return child;
}

void direct2dWindow::deleteChild(relative_ptr_type _id)
{
	if (children.contains(_id)) {
		children.erase(_id);
	}
}

CComPtr<ID2D1DeviceContext> direct2dWindow::beginDraw(bool& _adapter_blown_away)
{
	return context->beginDraw(_adapter_blown_away);
}

void direct2dWindow::endDraw(bool& _adapter_blown_away)
{
	_adapter_blown_away = false;

	if (context->getDeviceContext()) {

		HRESULT hr = context->endDraw(_adapter_blown_away);

		if (hr == D2DERR_RECREATE_TARGET)
		{
			_adapter_blown_away = true;
		}
		else if (SUCCEEDED(hr))
		{
			RECT r;
			hr = swapChain->Present(1, 0);

			switch (hr)
			{
			case DXGI_ERROR_ACCESS_DENIED:
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
			case DXGI_ERROR_DEVICE_REMOVED:
			case DXGI_ERROR_DEVICE_RESET:
			case DXGI_ERROR_ALREADY_EXISTS:
				_adapter_blown_away = true;
				break;
			}
		}
	}
}

//-------

direct2dChildWindow::direct2dChildWindow(std::weak_ptr<direct2dWindow> _parent, std::weak_ptr<adapterSet> _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips) 
{
	HRESULT hr;
			
	context = std::make_shared<direct2dContext>(_adapterSet);
	parent = _parent;
	childBitmap = nullptr;

	windowPosition.x = _xdips;
	windowPosition.y = _ydips;
	windowPosition.w = _wdips;
	windowPosition.h = _hdips;

	zOrder = 0;

	resize(_wdips, _hdips);
}

void direct2dChildWindow::resize(UINT _wdips, UINT _hdips)
{
	HRESULT hr;

	windowPosition.w = _wdips;
	windowPosition.h = _hdips;

	if (auto pwindow = parent.lock()) {
		int dpiWindow;
		dpiWindow = ::GetDpiForWindow(pwindow->getWindow());

		D2D1_SIZE_F size;
		size.width = _wdips;
		size.height = _hdips;

		if (auto pfactory = context->getFactory().lock()) 
		{
			auto tempBitmap = std::make_shared<direct2dBitmapCore>(size, pfactory, dpiWindow);
			context->getDeviceContext()->SetTarget(tempBitmap->getBitmap());
			childBitmap = tempBitmap;
		}
	}
			
}

void direct2dChildWindow::moveWindow(UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips)
{
	windowPosition.x = _xdips;
	windowPosition.y = _ydips;
	windowPosition.w = _wdips;
	windowPosition.h = _hdips;
	resize(_wdips, _hdips);
}

rectangle direct2dChildWindow::getBoundsDips()
{
	return windowPosition;
}

CComPtr<ID2D1DeviceContext>  direct2dChildWindow::beginDraw(bool& _adapter_blown_away)
{
	return context->beginDraw(_adapter_blown_away);
}

void direct2dChildWindow::endDraw(bool& _adapter_blown_away)
{
	context->endDraw(_adapter_blown_away);
}

direct2dChildWindow::~direct2dChildWindow()
{
}

//---

direct2dContext::direct2dContext(std::weak_ptr<corona::win32::adapterSet> _factory) :
	factory(_factory)
{
	currentTransform = D2D1::Matrix3x2F::Identity();

	D2D1_DEVICE_CONTEXT_OPTIONS options;

	options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

	auto padapter = _factory.lock();
	ID2D1DeviceContext *temp = nullptr;

	HRESULT hr = padapter->getD2DDevice()->CreateDeviceContext(options, &temp);
	context.Attach(temp);
	throwOnFail(hr, "Could not create device context");
}

direct2dContext::direct2dContext(std::weak_ptr<corona::win32::adapterSet> _factory, ID2D1DeviceContext* _context) :
	factory(_factory)
{
	currentTransform = D2D1::Matrix3x2F::Identity();
	context = _context;
}

direct2dContext::~direct2dContext()
{
	clearPaths();
	clearViewStyles();
	clearBitmapsAndBrushes(true);
}

CComPtr<ID2D1DeviceContext> direct2dContext::beginDraw(bool& _adapter_blown_away)
{
	currentTransform = D2D1::Matrix3x2F::Identity();
	_adapter_blown_away = false;

	HRESULT hr = S_OK;

	context->BeginDraw();
	return context;
}

HRESULT direct2dContext::endDraw(bool& _adapter_blown_away)
{
	_adapter_blown_away = false;
	HRESULT hr = context->EndDraw();
	return hr;
}

std::weak_ptr<adapterSet> direct2dContext::getFactory()
{
	return factory;
}

direct2dBitmapCore::direct2dBitmapCore(D2D1_SIZE_F _size, std::weak_ptr<adapterSet> _adapterSet, int dpi) :
	size(_size)
{
	targetContext = nullptr;
	target = nullptr;

	auto options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

	if (auto padapterSet = _adapterSet.lock()) {

		auto hr = padapterSet->getD2DDevice()->CreateDeviceContext(options, &targetContext);
		throwOnFail(hr, "Could not create device context");

		D2D1_SIZE_U bmsize;

		bmsize.height = _size.height * dpi / 96.0;
		bmsize.width = _size.width * dpi / 96.0;

		D2D1_BITMAP_PROPERTIES1 props = {};

		props.dpiX = 96;
		props.dpiY = 96;
		props.pixelFormat.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
		props.pixelFormat.alphaMode = D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_PREMULTIPLIED;
		props.bitmapOptions = D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_TARGET;

		auto ps = targetContext->GetSize();
		auto pxs = targetContext->GetPixelSize();
		auto unitMode = targetContext->GetUnitMode();

		targetContext->SetDpi(dpi, dpi);
		hr = targetContext->CreateBitmap(bmsize, nullptr, 0, props, &bitmap);
		throwOnFail(hr, "Could not create BITMAP");

		pxs = bitmap->GetPixelSize();
		ps = bitmap->GetSize();

#if TRACE_RENDER
		std::cout << "bitmap pixel size " << pxs.width << " " << pxs.height << std::endl;
		std::cout << "bitmap dips size " << ps.width << " " << ps.height << std::endl;
#endif

		targetContext->SetTarget(bitmap);
		ps = targetContext->GetSize();
		pxs = targetContext->GetPixelSize();

		unitMode = targetContext->GetUnitMode();
		return;

#if TRACE_RENDER
		std::cout << "target pixel size " << pxs.width << " " << pxs.height << std::endl;
		std::cout << "target dips size " << ps.width << " " << ps.height << std::endl;
#endif
	}

}

direct2dBitmapCore::~direct2dBitmapCore()
{
	if (target) target->Release();
	if (bitmap) bitmap->Release();
}

CComPtr<ID2D1DeviceContext> direct2dBitmapCore::beginDraw(bool& blownAdapter)
{
	blownAdapter = false;
	targetContext->BeginDraw();
	return targetContext;
}

void direct2dBitmapCore::endDraw(bool& blownAdapter)
{
	blownAdapter = false;
	targetContext->EndDraw();
}

direct2dBitmap::direct2dBitmap(D2D1_SIZE_F _size, std::weak_ptr<adapterSet>& _factory) 
{
	HRESULT hr;
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();

	if (auto padapter = _factory.lock()) {

		hr = padapter->getWicFactory()->CreateBitmap(size.width, size.height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicBitmap);
		throwOnFail(hr, "Could not create WIC bitmap");

		hr = padapter->getD2DFactory()->CreateWicBitmapRenderTarget(wicBitmap, props, &target);
		throwOnFail(hr, "Could not create WIC render target");

		ID2D1DeviceContext* targetContext = nullptr;

		hr = target->QueryInterface(&targetContext);
		throwOnFail(hr, "Could not get WIC context");

		context = std::make_shared<direct2dContext>(_factory, targetContext);
	}
}

direct2dBitmap::~direct2dBitmap()
{
	if (target) target->Release();
	if (wicBitmap) wicBitmap->Release();
}

void directBitmapSaveImpl::save(const wchar_t* _filename)
{

	HRESULT hr;

	if (auto padapter = dBitmap->getContext().getFactory().lock()) {

		hr = padapter->getWicFactory()->CreateStream(&fileStream);
		throwOnFail(hr, "Could not create file stream");

		hr = padapter->getWicFactory()->CreateEncoder(GUID_ContainerFormatPng, NULL, &bitmapEncoder);
		throwOnFail(hr, "Could not create bitmap encoder");

		hr = fileStream->InitializeFromFilename(_filename, GENERIC_WRITE);
		throwOnFail(hr, "Could not initialize file stream");

		hr = bitmapEncoder->Initialize(fileStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
		throwOnFail(hr, "Could not intialize bitmap encoder");

		hr = bitmapEncoder->CreateNewFrame(&bitmapFrameEncode, NULL);
		throwOnFail(hr, "Could not create frame");

		hr = bitmapFrameEncode->Initialize(NULL);
		throwOnFail(hr, "Could not initialize bitmap frame encoder");

		hr = bitmapFrameEncode->SetSize(dBitmap->size.width, dBitmap->size.height);
		throwOnFail(hr, "Could not initialize set size");

		WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

		WICRect rect;
		rect.X = 0;
		rect.Y = 0;
		rect.Width = dBitmap->size.width;
		rect.Height = dBitmap->size.height;

		hr = bitmapFrameEncode->WriteSource(dBitmap->getBitmap(), &rect);
		throwOnFail(hr, "Could not write source");

		hr = bitmapFrameEncode->Commit();
		throwOnFail(hr, "Could not commit frame");

		hr = bitmapEncoder->Commit();
		throwOnFail(hr, "Could not commit bitmap");
	}

}

IWICBitmap* direct2dBitmap::getBitmap()
{
	return wicBitmap;
}

void direct2dBitmap::save(const char* _filename)
{
	wchar_t buff[8192];
	int ret = ::MultiByteToWideChar(CP_ACP, 0, _filename, -1, buff, sizeof(buff) - 1);
	directBitmapSaveImpl saver(this);
	saver.save(buff);
}

deviceDependentAssetBase::deviceDependentAssetBase() : stock(false)
{
	;
}

deviceDependentAssetBase::~deviceDependentAssetBase()
{
	;
}


ID2D1Brush* deviceDependentAssetBase::getBrush()
{
	return NULL;
}

bool textStyle::create(direct2dContext* ptarget)
{
	HRESULT hr = -1;

	if (ptarget) 
	{
		if (!ptarget->getDeviceContext())
			return false;

		istring<2048> fontList = fontName;
		istring<2048> fontName;

		int state = 0;
		char* fontExtractedName = fontList.next_token({ ',', ';' }, state);
		lpWriteTextFormat = NULL;

		while (fontExtractedName)
		{
			fontName = fontExtractedName;
			iwstring<2048> wideName = fontName;

			DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;

			if (italic) {
				fontStyle = DWRITE_FONT_STYLE_ITALIC;
			}

			FLOAT dpiX = 96.0, dpiY = 96.0;
			ptarget->getDeviceContext()->GetDpi(&dpiX, &dpiY);

			if (auto fact = ptarget->getFactory().lock())
			{

				HRESULT hr = fact->getDWriteFactory()->CreateTextFormat(wideName.c_str(),
					NULL,
					bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR,
					fontStyle,
					DWRITE_FONT_STRETCH_NORMAL,
					size,
					L"en-US",
					&lpWriteTextFormat);

				if (SUCCEEDED(hr) || lpWriteTextFormat != nullptr) {
					break;
				}
			}

			fontExtractedName = fontList.next_token(',', state);
		};

		if (lpWriteTextFormat != nullptr)
		{
			if (line_spacing > 0.0) {
				lpWriteTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, line_spacing, line_spacing * .8);
			}

			switch (horizontal_align)
			{
			case visual_alignment::align_near:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				break;
			case visual_alignment::align_center:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				break;
			case visual_alignment::align_far:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				break;
			case visual_alignment::align_justify:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
				break;
			}

			switch (horizontal_align)
			{
			case visual_alignment::align_near:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				break;
			case visual_alignment::align_center:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				break;
			case visual_alignment::align_far:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				break;
			case visual_alignment::align_justify:
				lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
				break;
			}

			switch (vertical_align)
			{
			case visual_alignment::align_near:
				lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
				break;
			case visual_alignment::align_center:
				lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				break;
			case visual_alignment::align_far:
				lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
				break;
			case visual_alignment::align_justify:
				lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				break;
			}

			if (wrap_text)
			{
				lpWriteTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
			}

			return true;
		}
	}
	return false;
}

void textStyle::release()
{
	if (lpWriteTextFormat)
		lpWriteTextFormat->Release();
	lpWriteTextFormat = NULL;
}

bool filteredBitmap::create(direct2dContext* ptarget, HBITMAP _source)
{
	HRESULT hr = -1;

	if (originalScaledBitmap) {
		originalScaledBitmap->Release();
		originalScaledBitmap = NULL;
	}

	if (wicFilteredScaledBitmap) {
		wicFilteredScaledBitmap->Release();
		wicFilteredScaledBitmap = NULL;
	}

	if (auto pfactory = ptarget->getFactory().lock()) {
		hr = pfactory->getWicFactory()->CreateBitmapFromHBITMAP(_source, nullptr, WICBitmapAlphaChannelOption::WICBitmapIgnoreAlpha, &originalScaledBitmap);
		hr = pfactory->getWicFactory()->CreateBitmapFromHBITMAP(_source, nullptr, WICBitmapAlphaChannelOption::WICBitmapIgnoreAlpha, &wicFilteredScaledBitmap);
	}

	return SUCCEEDED(hr);
}

bool filteredBitmap::create(direct2dContext* ptarget, IWICBitmapSource* _source)
{
	HRESULT hr = -1;

	IWICBitmapScaler* pScaler = NULL;
	IWICBitmapClipper* pClipper = NULL;
	IWICFormatConverter* pConverter = NULL;

	if (originalScaledBitmap) {
		originalScaledBitmap->Release();
		originalScaledBitmap = NULL;
	}

	if (wicFilteredScaledBitmap) {
		wicFilteredScaledBitmap->Release();
		wicFilteredScaledBitmap = NULL;
	}


	if (auto pfactory = ptarget->getFactory().lock()) {
		hr = pfactory->getWicFactory()->CreateFormatConverter(&pConverter);
		hr = _source->GetSize(&originalWidth, &originalHeight);

		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (size.width != 0 || size.height != 0)
		{
			if (SUCCEEDED(hr))
			{
				if (cropEnabled) {
					hr = pfactory->getWicFactory()->CreateBitmapClipper(&pClipper);
					if (SUCCEEDED(hr)) {
						WICRect clipRect;
						clipRect.X = crop.left * originalWidth;
						clipRect.Y = crop.top * originalHeight;
						clipRect.Height = originalHeight - (clipRect.Y + originalHeight * crop.bottom);
						clipRect.Width = originalWidth - (clipRect.X + originalWidth * crop.right);
						hr = pClipper->Initialize(_source, &clipRect);
						_source = pClipper;
						if (size.width == 0)
						{
							FLOAT scalar = static_cast<FLOAT>(size.height) / static_cast<FLOAT>(clipRect.Height);
							size.width = static_cast<UINT>(scalar * static_cast<FLOAT>(clipRect.Width));
						}
						else if (size.height == 0)
						{
							FLOAT scalar = static_cast<FLOAT>(size.width) / static_cast<FLOAT>(clipRect.Width);
							size.height = static_cast<UINT>(scalar * static_cast<FLOAT>(clipRect.Height));
						}
					}
				}
				else
				{
					if (size.width == 0)
					{
						FLOAT scalar = static_cast<FLOAT>(size.height) / static_cast<FLOAT>(originalHeight);
						size.width = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
					}
					else if (size.height == 0)
					{
						FLOAT scalar = static_cast<FLOAT>(size.width) / static_cast<FLOAT>(originalWidth);
						size.height = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
					}
				}


				hr = pfactory->getWicFactory()->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
					hr = pScaler->Initialize(_source, size.width, size.height, WICBitmapInterpolationModeCubic);
				if (SUCCEEDED(hr))
					hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
				if (SUCCEEDED(hr)) {
					hr = pfactory->getWicFactory()->CreateBitmapFromSource(pConverter, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &originalScaledBitmap);
					hr = pfactory->getWicFactory()->CreateBitmapFromSource(pConverter, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicFilteredScaledBitmap);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(_source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
			if (SUCCEEDED(hr)) {
				if (cropEnabled) {
					hr = pfactory->getWicFactory()->CreateBitmapClipper(&pClipper);
					if (SUCCEEDED(hr)) {
						WICRect clipRect;
						clipRect.X = crop.left * originalWidth;
						clipRect.Y = crop.top * originalHeight;
						clipRect.Height = (originalHeight - (crop.bottom * originalHeight)) - clipRect.Y;
						clipRect.Width = (originalWidth - (crop.right * originalWidth)) - clipRect.X;
						hr = pClipper->Initialize(_source, &clipRect);
						_source = pClipper;
					}
				}
				else {
					_source = pConverter;
				}
				hr = pfactory->getWicFactory()->CreateBitmapFromSource(_source, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &originalScaledBitmap);
				hr = pfactory->getWicFactory()->CreateBitmapFromSource(_source, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicFilteredScaledBitmap);
			}
		}

		if (pConverter) { pConverter->Release(); pConverter = NULL; }
		if (pScaler) { pScaler->Release(); pScaler = NULL; }
		if (pClipper) { pClipper->Release(); pClipper = NULL; }
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);
	}

	return SUCCEEDED(hr);
}

bool filteredBitmap::make(direct2dContext* ptarget)
{
	HRESULT hr = 0;

	if (filteredScaledBitmap) {
		filteredScaledBitmap->Release();
		filteredScaledBitmap = NULL;
	}

	if (wicFilteredScaledBitmap) {
		hr = ptarget->getDeviceContext()->CreateBitmapFromWicBitmap(wicFilteredScaledBitmap, &filteredScaledBitmap);
	}
	return hr == S_OK;
}

filteredBitmap::~filteredBitmap()
{
	release();
}

void filteredBitmap::release()
{
	if (originalScaledBitmap)
		originalScaledBitmap->Release();
	originalScaledBitmap = NULL;
	if (wicFilteredScaledBitmap)
		wicFilteredScaledBitmap->Release();
	wicFilteredScaledBitmap = NULL;
	if (filteredScaledBitmap)
		filteredScaledBitmap->Release();
	filteredScaledBitmap = NULL;
}

void bitmap::clearFilteredBitmaps()
{
	for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
		filteredBitmap* bm = *ifb;
		delete bm;
	}
	filteredBitmaps.clear();
}

void bitmap::setFilteredBitmaps(std::list<sizeCrop>& _sizes)
{
	for (auto i = _sizes.begin(); i != _sizes.end(); i++) {
		auto sizeCrop = *i;
		filteredBitmap* bm = new filteredBitmap(sizeCrop.size, sizeCrop.cropEnabled, sizeCrop.crop);
		filteredBitmaps.push_back(bm);
	}
}

void bitmap::copyFilteredBitmaps(direct2dContext* _targetContext, bitmap* _src)
{
	for (auto i = _src->filteredBitmaps.begin(); i != _src->filteredBitmaps.end(); i++) {
		auto srcfiltered = *i;
		filteredBitmap* bm = srcfiltered->clone(_targetContext);
		filteredBitmaps.push_back(bm);
	}
}

bitmap::bitmap(direct2dContext* _targetContext, bitmap* _src)
	: useFile(_src->useFile),
	useResource(_src->useResource),
	filename(_src->filename),
	filterFunction(_src->filterFunction)
{
	copyFilteredBitmaps(_targetContext, _src);
	applyFilters(_targetContext);
}

bitmap::bitmap(std::string& _filename, std::list<sizeCrop>& _sizes) :
	useFile(true),
	filename(_filename),
	filterFunction(defaultfilter)
{
	setFilteredBitmaps(_sizes);
}

bitmap::bitmap(int _resource_id, std::list<sizeCrop>& _sizes) :
	useFile(false),
	useResource(true), 
	filterFunction(defaultfilter)
{		
	setFilteredBitmaps(_sizes);
}

bitmap::~bitmap()
{
	clearFilteredBitmaps();
}

std::shared_ptr<bitmap> bitmap::clone(direct2dContext* _src)
{
	return std::make_shared<bitmap>(_src, this);
}

void bitmap::setSizes(std::list<sizeCrop>& _sizes)
{
	clearFilteredBitmaps();
	setFilteredBitmaps(_sizes);
}

bool bitmap::getSize(int* _sizex, int* _sizey)
{
	bool success = false;
	if (filteredBitmaps.size() > 0) {
		if (*_sizex == 0 && *_sizey == 0) {
			auto bm = filteredBitmaps.front();
			*_sizex = bm->originalWidth;
			*_sizey = bm->originalHeight;
			success = true;
		}
		else if (*_sizey == 0) {
			auto iter = std::find_if(filteredBitmaps.begin(), filteredBitmaps.end(), [_sizex](filteredBitmap* _bm) { return _bm->size.width == *_sizex; });
			if (iter != filteredBitmaps.end()) {
				auto ix = *iter;
				*_sizex = ix->size.width;
				*_sizey = ix->size.height;
				success = true;
			}
		}
	}
	return success;
}

ID2D1Bitmap* bitmap::getFirst()
{
	auto afb = filteredBitmaps.front();
	return afb->filteredScaledBitmap;
}

ID2D1Bitmap* bitmap::getBySize(int _width, int _height)
{
	ID2D1Bitmap* current = nullptr;
	int current_largest = 0;

	for (auto fbm : filteredBitmaps) {
		if ((fbm->size.height >= _width && fbm->size.height <= current_largest) 
			|| !_height 
			|| !current_largest) {
			current_largest = fbm->size.height;
			current = fbm->filteredScaledBitmap;
		}
	}
	return current;
}

color bitmap::getColorAtPoint(int _width, int _height, point point)
{
	color color;
	IWICBitmap* bm = NULL;
	auto iter = std::find_if(filteredBitmaps.begin(), filteredBitmaps.end(), [_width, _height](filteredBitmap* _bm) { return _bm->size.width == _width && _bm->size.height == _height; });
	if (iter != filteredBitmaps.end()) {
		auto fbm = *iter;
		bm = fbm->wicFilteredScaledBitmap;

		HRESULT hr;
		if (point.x >= 0 && (int)point.x < fbm->size.width && (int)point.y < fbm->size.height && point.y >= 0 && bm) {
			WICRect rcLock = { 0, 0, fbm->size.width, fbm->size.height };
			IWICBitmapLock* pLock = NULL;

			hr = bm->Lock(&rcLock, WICBitmapLockWrite | WICBitmapLockRead, &pLock);

			if (SUCCEEDED(hr))
			{
				UINT cbBufferSize = 0;
				UINT cbStride = 0;
				BYTE* pv = NULL;

				hr = pLock->GetStride(&cbStride);

				if (SUCCEEDED(hr))
				{
					hr = pLock->GetDataPointer(&cbBufferSize, &pv);
					if (SUCCEEDED(hr) && pv && cbBufferSize) {
						auto row = (PBGRAPixel*)(pv + cbStride * (int)point.y);
						auto pix = row[(int)point.x];
						color.a = pix.alpha / 255.0;
						color.b = pix.blue / 255.0;
						color.r = pix.red / 255.0;
						color.g = pix.green / 255.0;
						// because this is premultiplied alpha
						if (color.a >= 0.0) {
							color.b /= color.a;
							color.g /= color.a;
							color.r /= color.a;
						}
					}
				}

				// Release the bitmap lock.
				pLock->Release();
			}
		}
	}
	return color;
}

void bitmap::setFilter(std::function<bool(point, int, int, char* bytes)> _filter)
{
	filterFunction = _filter;
}

void bitmap::filter()
{
	HRESULT hr;

	for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
		filteredBitmap* bm = *ifb;
		WICRect rcLock = { 0, 0, bm->size.width, bm->size.height };
		IWICBitmapLock* pLockOriginal = NULL;
		IWICBitmapLock* pLockFiltered = NULL;

		if (bm->originalScaledBitmap == NULL)
			continue;

		hr = bm->originalScaledBitmap->Lock(&rcLock, WICBitmapLockWrite | WICBitmapLockRead, &pLockOriginal);

		if (SUCCEEDED(hr))
		{
			hr = bm->wicFilteredScaledBitmap->Lock(&rcLock, WICBitmapLockWrite | WICBitmapLockRead, &pLockFiltered);

			if (SUCCEEDED(hr)) {

				UINT cbBufferSizeSrc = 0;
				UINT cbBufferSizeDst = 0;
				UINT cbStride = 0;
				BYTE* pvSrc = NULL;
				BYTE* pvDst = NULL;

				hr = pLockFiltered->GetStride(&cbStride);

				if (SUCCEEDED(hr))
				{
					hr = pLockOriginal->GetDataPointer(&cbBufferSizeSrc, &pvSrc);
					if (SUCCEEDED(hr)) {
						hr = pLockFiltered->GetDataPointer(&cbBufferSizeDst, &pvDst);

						if (SUCCEEDED(hr) && pvSrc && pvDst && cbBufferSizeDst && cbBufferSizeSrc && (cbBufferSizeSrc == cbBufferSizeDst)) {
							memcpy(pvDst, pvSrc, cbBufferSizeSrc);
							point size = toSize(bm->size);
							filterFunction(size, (int)cbBufferSizeDst, (int)cbStride, (char*)pvDst);
						}
					}
				}

			}

			// Release the bitmap lock.
			pLockOriginal->Release();
			pLockFiltered->Release();
		}
	}
}

bool bitmap::applyFilters(direct2dContext* _target)
{
	filter();

	for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
		filteredBitmap* bm = *ifb;
		bm->make(_target);
	}

	return true;
}

bool bitmap::create_from_file(direct2dContext* pfactory)
{
	HRESULT hr;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;

	wchar_t fileBuff[1024];
	int ret = ::MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, fileBuff, sizeof(fileBuff) / sizeof(wchar_t) - 1);

	hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

	if (pfactory) {
		if (auto padapter = pfactory->getFactory().lock()) {
			hr = padapter->getWicFactory()->CreateDecoderFromFilename(fileBuff, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

			if (SUCCEEDED(hr))
				hr = pDecoder->GetFrame(0, &pSource);

			if (SUCCEEDED(hr))
			{
				for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
					filteredBitmap* bm = *ifb;
					bm->create(pfactory, pSource);
				}

				filter();

				for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
					filteredBitmap* bm = *ifb;
					bm->make(pfactory);
				}

			}

			if (pDecoder) { pDecoder->Release(); pDecoder = NULL; }
			if (pSource) { pSource->Release(); pSource = NULL; }
		}
	}

	return SUCCEEDED(hr);
}

bool bitmap::create_from_resource(direct2dContext* _target)
{
	bool hr = false;
	HBITMAP hbitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(resource_id));

	if (hbitmap) {

		for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
			filteredBitmap* bm = *ifb;
			bm->create(_target, hbitmap);
		}

		filter();

		for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
			filteredBitmap* bm = *ifb;
			bm->make(_target);
		}
		hr = true;
	}

	return hr;
}

bool bitmap::create(direct2dContext* _target)
{
	bool result = false;
	if (useFile) {
		result = create_from_file(_target);
	} 
	else if (useResource) 
	{
		result = create_from_resource(_target);
	}
	return result;
}

void bitmap::release()
{
	for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
		filteredBitmap* bm = *ifb;
		bm->release();
	}
}


bitmapBrush::bitmapBrush() : deviceDependentAsset()
{
	;
}

bitmapBrush::~bitmapBrush()
{

}

bool bitmapBrush::create(direct2dContext* ptarget)
{
	HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);
	if (ptarget) {

		if (!ptarget || !ptarget->getDeviceContext())
		return false;

		if (auto pbm = bm.lock()) {
			hr = ptarget->getDeviceContext()->CreateBitmapBrush(pbm->getFirst(), &asset);
		}
	}

	return SUCCEEDED(hr);
}

ID2D1Brush* bitmapBrush::getBrush()
{
	return asset;
}

solidColorBrush::solidColorBrush() : deviceDependentAsset()
{
	;
}

solidColorBrush::~solidColorBrush()
{

}

bool solidColorBrush::create(direct2dContext* ptarget)
{
	HRESULT hr = -1;

	if (ptarget) 
	{
		if (!ptarget->getDeviceContext())
			return false;

		hr = ptarget->getDeviceContext()->CreateSolidColorBrush(color, &asset);
	}

	return SUCCEEDED(hr);
}

ID2D1Brush* solidColorBrush::getBrush()
{
	return asset;
}


bool linearGradientBrush::create(direct2dContext* ptarget)
{
	ID2D1GradientStopCollection* pGradientStops = NULL;

	HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

	if (ptarget)
	{
		if (!ptarget->getDeviceContext())
			return false;

		hr = ptarget->getDeviceContext()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

		if (SUCCEEDED(hr))
		{
			hr = ptarget->getDeviceContext()->CreateLinearGradientBrush(
				D2D1::LinearGradientBrushProperties(start, stop),
				D2D1::BrushProperties(),
				pGradientStops,
				&asset
			);
			pGradientStops->Release();
		}
	}
	
	return SUCCEEDED(hr);
}

ID2D1Brush* linearGradientBrush::getBrush()
{
	return asset;
}


bool radialGradientBrush::create(direct2dContext* ptarget)
{
	ID2D1GradientStopCollection* pGradientStops = NULL;

	HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

	if (ptarget)
	{
		if (!ptarget->getDeviceContext())
			return false;

		hr = ptarget->getDeviceContext()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

		if (SUCCEEDED(hr))
		{
			hr = ptarget->getDeviceContext()->CreateRadialGradientBrush(
				radialProperties,
				D2D1::BrushProperties(),
				pGradientStops,
				&asset
			);
			pGradientStops->Release();
		}
	}
	return SUCCEEDED(hr);
}

ID2D1Brush* radialGradientBrush::getBrush()
{
	return asset;
}

path::path(std::weak_ptr<direct2dContext>& target) : geometry(NULL), sink(NULL)
{
	HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

	if (auto ptarget = target.lock())
	{
		if (ptarget->getDeviceContext())
		{
			if (auto pfactory = ptarget->getFactory().lock()) {
				hr = pfactory->getD2DFactory()->CreatePathGeometry(&geometry);
				if (!SUCCEEDED(hr)) {
					// UH, SOMETHING;
				}
			}
		}
	}
}

path::~path()
{
	if (sink) sink->Release();
	if (geometry) geometry->Release();
	sink = NULL;
	geometry = NULL;
}

void path::start_figure(D2D1_POINT_2F point)
{
	if (geometry) {
		geometry->Open(&sink);
		if (sink)
			sink->BeginFigure(point, D2D1_FIGURE_BEGIN_FILLED);
	}
}

void path::add_line(D2D1_POINT_2F point)
{
	if (sink) sink->AddLine(point);
}

void path::add_arc(D2D1_POINT_2F point1, D2D1_SIZE_F size1, FLOAT rotationAngle)
{
	D2D1_SWEEP_DIRECTION direction = rotationAngle > 0.0 ? D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_CLOCKWISE;
	rotationAngle = fabs(rotationAngle);
	D2D1_ARC_SIZE arcSize = rotationAngle > 180.0 ? D2D1_ARC_SIZE::D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL;
	if (sink) sink->AddArc(D2D1::ArcSegment(point1, size1, fabs(rotationAngle), direction, arcSize));
}

void path::add_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2, D2D1_POINT_2F point3)
{
	if (sink) sink->AddBezier(D2D1::BezierSegment(point1, point2, point3));
}

void path::add_quadratic_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2)
{
	if (sink) sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(point1, point2));
}

void path::close_figure(bool closed)
{
	if (sink) {
		sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
		sink->Close();
		sink->Release();
		sink = NULL;
	}
}

// -------------------------------------------------------


void direct2dContext::clear(color* _color)
{
	D2D1_COLOR_F color;

	color.a = _color->a;
	color.b = _color->b;
	color.g = _color->g;
	color.r = _color->r;

	this->getDeviceContext()->Clear(color);
}

std::string direct2dContext::setBitmap(bitmapRequest* _bitmap)
{
	std::string filename, name;

	filename = _bitmap->file_name.c_str();
	name = _bitmap->name.c_str();

	std::list<sizeCrop> sizes;
	for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
		sizes.push_back(toSizeC(*it, _bitmap->cropEnabled, _bitmap->crop));
	}
	std::shared_ptr<bitmap> bm;

	if (_bitmap->resource_id)
	{
		bm = std::make_shared<bitmap>(_bitmap->resource_id, sizes);
	}
	else 
	{
		bm = std::make_shared<bitmap>(filename, sizes);
	}

	bitmaps[name] = bm;
	bm->create(this);

	for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
		int sx = 0, sy = 0;
		bm->getSize(&sx, &sy);
		it->x = sx;
		it->y = sy;
	}
	return name;
}

bool direct2dContext::getBitmapSize(bitmapRequest* _bitmap, point* _size)
{
	bool success = false;
	auto i = bitmaps[_bitmap->name.c_str()];
	if (i) {
		int bsx, bsy;
		success = i->getSize(&bsx, &bsy);
		_size->x = bsx;
		_size->y = bsy;
	}
	return success;
}

color direct2dContext::getColorAtPoint(bitmapInstanceDto* _bitmap, point& _point)
{
	color color;
	auto i = bitmaps[_bitmap->bitmapName];
	if (i) {
		color = i->getColorAtPoint(_bitmap->width, _bitmap->height, _point);
	}
	return color;
}

bool direct2dContext::setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize)
{
	bool success = false;
	auto bm = bitmaps[_bitmap->name.c_str()];
	if (bm) {

		// first, check to see if we really need to do this
		bool allGood = true;
		if (_forceResize)
			allGood = false;
		else
			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				int width = it->x;
				int height = it->y;
				allGood = bm->getSize(&width, &height);
				it->x = width;
				it->y = height;
				if (!allGood) break;
			}

		// then, if we do, go ahead and create the bitmaps
		if (!allGood) {
			std::list<sizeCrop> sizes;
			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				sizes.push_back(toSizeC(*it, _bitmap->cropEnabled, _bitmap->crop));
			}
			bm->setSizes(sizes);
			bm->create(this);
			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				int width = it->x;
				int height = it->y;
				bm->getSize(&width, &height);
				it->x = width;
				it->y = height;
			}
		}

		success = true;
	}
	return success;
}

bool direct2dContext::setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter)
{
	bool success = false;
	auto bm = bitmaps[_bitmap->name.c_str()];
	if (bm) {
		bm->setFilter(_filter);
		success = bm->applyFilters(this);
	}
	return success;
}

std::string direct2dContext::setBitmapBrush(bitmapBrushRequest* _bitmapBrush)
{
	auto brush = std::make_shared<bitmapBrush>();
	std::string name, bitmapName;
	name = _bitmapBrush->name.c_str();
	bitmapName = _bitmapBrush->bitmapName;
	brush->bm = bitmaps[bitmapName];
	brushes[name] = brush;
	brush->create(this);
	return name;
}

std::string direct2dContext::setSolidColorBrush(solidBrushRequest* _solidBrushDto)
{
	auto brush = std::make_shared<solidColorBrush>();
	brush->stock = false;
	brush->color = toColor(_solidBrushDto->brushColor);
	brushes[_solidBrushDto->name.c_str()] = brush;
	brush->create(this);
	return _solidBrushDto->name.c_str();
}

std::string direct2dContext::setLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto)
{
	D2D1_GRADIENT_STOP gradientStop;
	auto brush = std::make_shared<linearGradientBrush>();
	brush->stock = false;
	brush->start = toPoint(_linearGradientBrushDto->start);
	brush->stop = toPoint(_linearGradientBrushDto->stop);
	for (auto i : _linearGradientBrushDto->gradientStops) {
		gradientStop = toGradientStop(i);
		brush->stops.push_back(gradientStop);
	}
	brushes[_linearGradientBrushDto->name.c_str()] = brush;
	brush->create(this);
	return _linearGradientBrushDto->name.c_str();
}

std::string direct2dContext::setRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto)
{
	D2D1_GRADIENT_STOP gradientStop;
	auto brush = std::make_shared<radialGradientBrush>();
	brush->stock = false;
	brush->radialProperties.center = toPoint(_radialGradientBrushDto->center);
	brush->radialProperties.gradientOriginOffset = toPoint(_radialGradientBrushDto->offset);
	brush->radialProperties.radiusX = _radialGradientBrushDto->radiusX;
	brush->radialProperties.radiusY = _radialGradientBrushDto->radiusY;
	for (auto i : _radialGradientBrushDto->gradientStops) {
		gradientStop = toGradientStop(i);
		brush->stops.push_back(gradientStop);
	}
	brushes[_radialGradientBrushDto->name.c_str()] = brush;
	brush->create(this);
	return _radialGradientBrushDto->name.c_str();
}

void direct2dContext::clearBitmapsAndBrushes(bool deleteStock)
{
	brushes.clear();
	bitmaps.clear();
}

std::shared_ptr<path> direct2dContext::createPath(pathDto* _pathDto, bool _closed)
{
	auto wft = weak_from_this();
	std::shared_ptr<path> newPath = std::make_shared<path>(wft);

	D2D1_POINT_2F point1, point2, point3;
	D2D1_SIZE_F size1;
	FLOAT float1;
	pathLineDto* pline;
	pathArcDto* parc;
	pathBezierDto* pbezier;
	pathQuadraticBezierDto* pquadraticbezier;

	bool findingMoveTo = true;

	for (auto i : _pathDto->points) {
		pathBaseDto* t = i.get();
		if (findingMoveTo) {
			pathLineDto* l = t->asPathLineDto();
			if (l) {
				D2D1_POINT_2F point = toPoint(t->asPathLineDto()->point);
				newPath->start_figure(point);
				findingMoveTo = false;
			}
		}
		else 
		{
			switch (t->eType) {
			case e_line:
				pline = t->asPathLineDto();
				point1 = toPoint(pline->point);
				newPath->add_line(point1);
				break;
			case e_arc:
				parc = t->asPathArcDto();
				point1 = toPoint(parc->point);
				size1.height = parc->radiusX;
				size1.width = parc->radiusY;
				newPath->add_arc(point1, size1, parc->angleDegrees);
				break;
			case e_bezier:
				pbezier = t->asPathBezierDto();
				point1 = toPoint(pbezier->point1);
				point2 = toPoint(pbezier->point2);
				point3 = toPoint(pbezier->point3);
				newPath->add_bezier(point1, point2, point3);
				break;
			case e_quadractic_bezier:
				pquadraticbezier = t->asPathQuadraticBezierDto();
				point1 = toPoint(pquadraticbezier->point1);
				point2 = toPoint(pquadraticbezier->point2);
				newPath->add_quadratic_bezier(point1, point2);
				break;
			}
		}
	}

	// now draw the rest of the path

	newPath->close_figure(_closed);
	return newPath;
}

std::string direct2dContext::setPath(pathDto* _pathDto, bool _closed)
{
	auto newPath = createPath(_pathDto, _closed);
	paths[_pathDto->name.c_str()] = newPath;
	return _pathDto->name.c_str();
}

void direct2dContext::clearPaths()
{
	paths.clear();
}

void direct2dContext::setTextStyle(textStyleRequest* _textStyleDto)
{
	auto newStyle = std::make_shared<textStyle>(
		_textStyleDto->fontName.c_str(),
		_textStyleDto->fontSize,
		_textStyleDto->bold,
		_textStyleDto->italics,
		_textStyleDto->underline,
		_textStyleDto->strike_through,
		_textStyleDto->line_spacing,
		_textStyleDto->horizontal_align,
		_textStyleDto->vertical_align,
		_textStyleDto->wrap_text
	);
	textStyles[_textStyleDto->name.c_str()] = newStyle;
	newStyle->create(this);
}

void direct2dContext::clearTextStyles()
{
	textStyles.clear();
}

void direct2dContext::setViewStyle(viewStyleRequest& _request)
{
	auto vs = std::make_shared<viewStyleRequest>();
	*(vs.get()) = _request;
	viewStyles[_request.name.c_str()] = vs;
	setTextStyle(&_request.text_style);
	setSolidColorBrush(&_request.box_border_color);
	setSolidColorBrush(&_request.box_fill_color);
	setSolidColorBrush(&_request.shape_border_color);
	setSolidColorBrush(&_request.shape_fill_color);
}

void direct2dContext::clearViewStyles()
{
	viewStyles.clear();
}

void direct2dContext::drawPath(pathInstance2dDto* _pathInstanceDto)
{
	auto fill = brushes[_pathInstanceDto->fillBrushName];
	auto border = brushes[_pathInstanceDto->borderBrushName];
	auto p = paths[_pathInstanceDto->pathName];

	/*
	if (!fill) {
#if TRACE_GUI
		std::cout << "missing fill " << _pathInstanceDto->fillBrushName << std::endl;
#endif
		return;
	}

	if (!border) {
#if TRACE_GUI
		std::cout << "missing border " << _pathInstanceDto->borderBrushName << std::endl;
#endif
		return;
	}
	*/

	if (!p)
		return;

//			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathInstanceDto->rotation) * D2D1::Matrix3x2F::Translation(_pathInstanceDto->position.x, _pathInstanceDto->position.y);
//			getDeviceContext()->SetTransform(product);

	if (fill) {
		getDeviceContext()->FillGeometry(p->geometry, fill->getBrush());
	}
	if (border && _pathInstanceDto->strokeWidth > 0.0) {
		getDeviceContext()->DrawGeometry(p->geometry, border->getBrush(), _pathInstanceDto->strokeWidth);
	}
}

void direct2dContext::drawPath(pathImmediateDto* _pathImmediateDto)
{
	auto fill = brushes[_pathImmediateDto->fillBrushName];
	auto border = brushes[_pathImmediateDto->borderBrushName];

	if (!fill) {
#if TRACE_GUI
		std::cout << "missing fill " << _pathImmediateDto->fillBrushName << std::endl;
#endif
		return;
	}

	if (!border) {
#if TRACE_GUI
		std::cout << "missing border " << _pathImmediateDto->borderBrushName << std::endl;
#endif
		return;
	}

	auto p = createPath(&_pathImmediateDto->path, _pathImmediateDto->closed);
	if (!p)
		return;

//			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathImmediateDto->rotation) * D2D1::Matrix3x2F::Translation(_pathImmediateDto->position.x, _pathImmediateDto->position.y);
//		getDeviceContext()->SetTransform(product);

	if (fill && _pathImmediateDto->fillBrushName.size() > 0) {
		getDeviceContext()->FillGeometry(p->geometry, fill->getBrush());
	}
	if (border && _pathImmediateDto->borderBrushName.size() > 0 && _pathImmediateDto->strokeWidth > 0.0) {
		getDeviceContext()->DrawGeometry(p->geometry, border->getBrush(), _pathImmediateDto->strokeWidth);
	}
}

void direct2dContext::drawLine(database::point* start, database::point* stop, const char* _fillBrush, double thickness)
{
	auto fill = brushes[_fillBrush];

	D2D1_POINT_2F dstart, dstop;

	dstart.x = start->x;
	dstart.y = start->y;
	dstop.x = stop->x;
	dstop.y = stop->y;

	if (fill) {
		getDeviceContext()->DrawLine(dstart, dstop, fill->getBrush(), thickness, nullptr);
	}
}

void direct2dContext::drawRectangle(database::rectangle* _rectangle, const char* _borderBrush, double _borderWidth, const char* _fillBrush)
{
	D2D1_RECT_F r;
	r.left = _rectangle->x;
	r.top = _rectangle->y;
	r.right = _rectangle->x + _rectangle->w;
	r.bottom = _rectangle->y + _rectangle->h;

	if (_fillBrush)
	{
		auto fill = brushes[_fillBrush];
		if (!fill) {
#if TRACE_GUI
			std::cout << "missing fill " << _fillBrush << std::endl;
#endif
		}
		else
			getDeviceContext()->FillRectangle(r, fill->getBrush());
	}

	if (_borderBrush)
	{
		auto border = brushes[_borderBrush];
		if (!border) {
#if TRACE_GUI
			std::cout << "missing border " << _borderBrush << std::endl;
#endif
		}
		else
			getDeviceContext()->DrawRectangle(&r, border->getBrush(), _borderWidth);
	}
}

void direct2dContext::drawText(const char* _text, database::rectangle* _rectangle, const char* _textStyle, const char* _fillBrush)
{
	auto style = _textStyle ? textStyles[_textStyle] : nullptr;
	auto fill = _fillBrush ? brushes[_fillBrush] : nullptr;

	if (!style) {
#if TRACE_GUI
		std::cout << "missing textStyle " << _textStyle << std::endl;

#if TRACE_STYLES
		std::cout << "styles loaded" << std::endl;
		for (auto vs : viewStyles) {
			std::cout << vs.first << std::endl;
		}
#endif
#endif

		return;
	}

	if (!fill) {
#if TRACE_GUI
		std::cout << "missing fillBrush " << _fillBrush << std::endl;
#endif
		return;
	}

	auto format = style->getFormat();
	if (!format) {
#if TRACE_GUI
		std::cout << "missing format " << _textStyle << std::endl;
#endif

		return;
	}

	D2D1_RECT_F r;
	r.left = _rectangle->x;
	r.top = _rectangle->y;
	r.right = _rectangle->x + _rectangle->w;
	r.bottom = _rectangle->y + _rectangle->h;

	auto brush = fill->getBrush();
	int len = (strlen(_text) + 1) * 2;
	wchar_t* buff = new wchar_t[len];
	int ret = ::MultiByteToWideChar(CP_ACP, NULL, _text, -1, buff, len - 1);

	if (style->get_strike_through() || style->get_underline())
	{
		int l = wcslen(buff);
		IDWriteTextLayout* textLayout = nullptr;
				
		if (auto pfactory = getFactory().lock()) {

			pfactory->getDWriteFactory()->CreateTextLayout(buff, l, format, r.right - r.left, r.bottom - r.top, &textLayout);
			if (textLayout != nullptr) {
				textLayout->SetUnderline(style->get_underline(), { (UINT32)0, (UINT32)l });
				textLayout->SetStrikethrough(style->get_strike_through(), { (UINT32)0, (UINT32)l });
				getDeviceContext()->DrawTextLayout({ r.left, r.top }, textLayout, brush);
				textLayout->Release();
				textLayout = nullptr;
			}
			else
			{
				getDeviceContext()->DrawText(buff, ret, format, &r, brush);
			}
		}
	}
	else
	{
		getDeviceContext()->DrawText(buff, ret, format, &r, brush);
	}

	// uncomment this to show the text borders 
	//getDeviceContext()->DrawRectangle(&r, brush);

	delete[] buff;
}

database::rectangle direct2dContext::getCanvasSize()
{

	D2D1_SIZE_F size;

	if (getDeviceContext())
	{
		size = getDeviceContext()->GetSize();
	}
	else
	{
		size.width = 0;
		size.height = 0;
	}

	return database::rectangle{ 0, 0, size.width, size.height };
}

void direct2dContext::drawText(drawTextRequest* _textInstanceDto)
{
	auto style = textStyles[_textInstanceDto->styleName];
	auto fill = brushes[_textInstanceDto->fillBrushName];

	if (!style) {
#if TRACE_GUI				
		std::cout << "missing text style " << _textInstanceDto->styleName << std::endl;
#endif
		return;
	}

	if (!fill) {
#if TRACE_GUI
		std::cout << "missing fill " << _textInstanceDto->fillBrushName << std::endl;
#endif
		return;
	}

	D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_textInstanceDto->rotation) * D2D1::Matrix3x2F::Translation(_textInstanceDto->position.x, _textInstanceDto->position.y);
	getDeviceContext()->SetTransform(product);

	D2D1_RECT_F rect = {};

	rect.left = _textInstanceDto->layout.x;
	rect.top = _textInstanceDto->layout.y;
	rect.right = _textInstanceDto->layout.x + _textInstanceDto->layout.w;
	rect.bottom = _textInstanceDto->layout.y + _textInstanceDto->layout.h;

	auto brush = fill->getBrush();
	int l = (_textInstanceDto->text.length() + 1) * 2;
	wchar_t* buff = new wchar_t[l];
	int ret = ::MultiByteToWideChar(CP_ACP, NULL, _textInstanceDto->text.c_str(), -1, buff, l - 1);
	getDeviceContext()->DrawText(buff, ret, style->getFormat(), &rect, brush);
	delete[] buff;
}

void direct2dContext::drawBitmap(bitmapInstanceDto* _bitmapInstanceDto)
{
	auto bm = bitmaps[_bitmapInstanceDto->bitmapName];
	throwOnFalse(static_cast<bool>(bm), "Bitmap not found in context");
	auto ibm = bm->getBySize(_bitmapInstanceDto->width, _bitmapInstanceDto->height);
	throwOnNull(ibm, "Bitmap size not found in context");
	D2D1_RECT_F rect, source;
	rect.left = _bitmapInstanceDto->x;
	rect.top = _bitmapInstanceDto->y;
	auto size = ibm->GetSize();
	if (_bitmapInstanceDto->width) {
		rect.right = rect.left + _bitmapInstanceDto->width;
	}
	else 
	{
		rect.right = rect.left + size.width;
	}
	if (_bitmapInstanceDto->height) {
		rect.bottom = rect.top + _bitmapInstanceDto->height;
	}
	else {
		rect.bottom = rect.top + size.height;
	}
	source.left = 0;
	source.top = 0;
	source.right = size.width;
	source.bottom = size.height;
	getDeviceContext()->DrawBitmap(ibm, rect, _bitmapInstanceDto->alpha, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, source);
}

void direct2dContext::popCamera()
{
	if (transforms.empty())
		currentTransform = D2D1::Matrix3x2F::Identity();
	else {
		currentTransform = transforms.top();
		transforms.pop();
	}
}

void direct2dContext::pushCamera(point* _position, float _rotation, float _scale)
{
	transforms.push(currentTransform);
	currentTransform = currentTransform * D2D1::Matrix3x2F::Rotation(_rotation)
		* D2D1::Matrix3x2F::Translation(_position->x, _position->y)
		* D2D1::Matrix3x2F::Scale(_scale, _scale);
	getDeviceContext()->SetTransform(currentTransform);
}

CComPtr<ID2D1DeviceContext>  direct2dBitmap::beginDraw(bool& _adapter_blown_away)
{
	_adapter_blown_away = false;

	return context->beginDraw(_adapter_blown_away);
}

void direct2dBitmap::endDraw(bool& _adapter_blown_away)
{
	_adapter_blown_away = false;

	context->endDraw(_adapter_blown_away);
}

std::shared_ptr<direct2dBitmap> direct2dContext::createBitmap(point& _size)
{
	auto rfact = getFactory();
	auto bp = std::make_shared<direct2dBitmap>(toSizeF(_size), rfact);

	if (auto pfactory = rfact.lock()) {

		// now for the fun thing.  we need copy all of the objects over that we created from this context into the new one.  
		// i guess every architecture has its unforseen ugh moment, and this one is mine.

		std::for_each(bitmaps.begin(), bitmaps.end(), [this, bp](auto ib) {
			bp->getContext().bitmaps[ib.first] = ib.second->clone(this);
			});

		// will need this for all objects
		/*
		std::for_each( brushes.begin(), brushes.end(), [ this, bp ]( std::pair<std::string, deviceDependentAssetBase *> ib ) {
			ib.second->create(this);
		});
		std::for_each( textStyles.begin(), textStyles.end(), [ this, bp ]( std::pair<std::string, textStyle *> ib ) {
			ib.second->create(this);
		});
		*/
	}

	return bp;
}

void direct2dContext::drawBitmap(drawableHost* _drawableHost, point& _dest, point& _size)
{
	if (_drawableHost->isBitmap()) {
		direct2dBitmap* bp = (direct2dBitmap*)_drawableHost;
		auto wicbitmap = bp->getBitmap();
		ID2D1Bitmap* bitmap = NULL;
		HRESULT hr = this->getDeviceContext()->CreateBitmapFromWicBitmap(wicbitmap, &bitmap);
		throwOnFail(hr, "Could not create bitmap from wic bitmap");
		D2D1_RECT_F rect;
		rect.left = _dest.x;
		rect.top = _dest.y;
		rect.right = rect.left += _size.x > 0 ? _size.x : bp->size.width;
		rect.bottom = rect.top += _size.y > 0 ? _size.y : bp->size.height;
		getDeviceContext()->DrawBitmap(bitmap, &rect);
		bitmap->Release();
		bitmap = NULL;
	}
}

void direct2dContext::drawView(const char* _style, const char* _text, rectangle& _rect, int _state, const char* _debug_comment)
{
	if (!_style) return;

	object_name style_name = _style;
	object_name style_composed_name;

	view_style_name(style_name, style_composed_name, _state);

	auto vs = viewStyles[style_composed_name.c_str()];
	auto& rectFill = vs->box_fill_color;
	drawRectangle(&_rect, vs->box_border_color.name, vs->box_border_thickness, vs->box_fill_color.name);

	_rect.h -= vs->box_border_thickness * 2.0;
	_rect.w -= vs->box_border_thickness * 2.0;
	_rect.x += vs->box_border_thickness;
	_rect.y += vs->box_border_thickness;

	drawText(_text, &_rect, vs->text_style.name, vs->shape_fill_color.name);

#if OUTLINE_GUI

	drawRectangle(&_rect, "debug-border", 2.0, nullptr);
	if (_debug_comment) {
		drawText(_debug_comment, &_rect, "debug-text", "debug-border");
	}
#endif
}


void direct2dContext::save(const char* _filename)
{
	;
}

void direct2dContext::view_style_name(const object_name& _style_sheet_name, object_name& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-view-" + std::to_string(_index);
}

void direct2dContext::text_style_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-text-" + std::to_string(_index);
}

void direct2dContext::box_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-box-border-" + std::to_string(_index);
}

void direct2dContext::box_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-box-fill-" + std::to_string(_index);
}

void direct2dContext::shape_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-shape-fill-" + std::to_string(_index);
}

void direct2dContext::shape_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
{
	_object_style_name = _style_sheet_name + "-shape-border-" + std::to_string(_index);
}



adapterSet::adapterSet()
{
}

adapterSet::~adapterSet()
{
}

void adapterSet::cleanup()
{
	dxAdapter.Release();
	dxFactory.Release();
}

void adapterSet::refresh()
{
	cleanup();
	direct2d = std::make_shared<direct2dDevice>();
	direct3d = std::make_shared<direct3dDevice>();

	HRESULT hr = CreateDXGIFactory1(IID_IDXGIFactory1, (void**)&dxFactory);
	throwOnFail(hr, "Could not create DXGI factory");

	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		IDXGIAdapter1* currentAdapter = nullptr;

		if (DXGI_ERROR_NOT_FOUND == dxFactory->EnumAdapters1(adapterIndex, &currentAdapter))
		{
			// No more adapters to enumerate.
			break;
		}

		if (direct3d->setDevice(currentAdapter))
		{
			dxAdapter = currentAdapter;
			break;
		}
		else
		{
			currentAdapter->Release();
		}
	}

	direct2d->setDevice(direct3d->getD3DDevice());
}

std::weak_ptr<direct2dWindow> adapterSet::createD2dWindow(HWND parent, color backgroundColor)
{
	auto pthis = weak_from_this();
	std::shared_ptr<direct2dWindow> win = std::make_shared<direct2dWindow>(parent, pthis, backgroundColor);
	parent_windows.insert_or_assign(parent, win);
	return win;
}

std::weak_ptr<direct2dWindow> adapterSet::getWindow(HWND parent)
{
	std::shared_ptr<direct2dWindow> win;
	if (parent_windows.contains(parent)) {
		win = parent_windows[parent];
	}
	return win;
}

bool adapterSet::containsWindow(HWND parent)
{
	return parent_windows.contains(parent);
}

void adapterSet::closeWindow(HWND hwnd)
{
	auto win = getWindow(hwnd);
	if (!win.expired()) {
		parent_windows.erase(hwnd);
	}
}

void adapterSet::clearWindows()
{
	parent_windows.clear();
}

std::weak_ptr<direct2dChildWindow> adapterSet::findChild(relative_ptr_type _child)
{
	std::weak_ptr<direct2dChildWindow> w;
	for (auto win : parent_windows)
	{
		w = win.second->getChild(_child);
		if (!w.expired()) {
			break;
		}
	}
	return w;
}

std::unique_ptr<direct2dBitmap> adapterSet::createD2dBitmap(D2D1_SIZE_F size)
{
	auto padapter = weak_from_this();
	std::unique_ptr<direct2dBitmap> win = std::make_unique<direct2dBitmap>(size, padapter);
	return win;
}

