
#pragma once

#include "corona.h"

#ifdef WINDESKTOP_GUI


namespace corona
{
	namespace win32
	{
		void throwOnFail(HRESULT hr, const char* _message)
		{
			if (!SUCCEEDED(hr)) {
				throw std::exception("COM failure");
			}
		}

		void throwOnNull(void* _ptr, const char* _message)
		{
			if (!_ptr) {
				throw std::exception("null reference failure");
			}
		}

		adapterSet::adapterSet()
		{
			dxFactory = nullptr;
			dxAdapter = nullptr;
			direct2d = nullptr;
			direct3d = nullptr;
		}

		adapterSet::~adapterSet()
		{
			if (dxAdapter) {
				dxAdapter->Release();
				dxAdapter = nullptr;
			}

			if (dxFactory) {
				dxFactory->Release();
				dxFactory = nullptr;
			}
		}

		void adapterSet::cleanup()
		{

			if (direct2d)
				delete direct2d;

			if (direct3d)
				delete direct3d;

			if (dxAdapter)
				dxAdapter->Release();
			dxAdapter = nullptr;

			if (dxFactory)
				dxFactory->Release();
			dxFactory = nullptr;
		}

		void adapterSet::refresh()
		{
			cleanup();
			direct2d = new direct2dDevice();
			direct3d = new direct3dDevice();

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

		direct2dWindow* adapterSet::createD2dWindow(HWND parent)
		{
			direct2dWindow *win = new direct2dWindow(parent, this);
			parent_windows.insert_or_assign(parent, win);
			return win;
		}

		direct2dWindow* adapterSet::getWindow(HWND parent)
		{
			direct2dWindow* win = nullptr;
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
			auto *win = getWindow(hwnd);
			if (win) {
				delete win;
				parent_windows.erase(hwnd);
			}
		}

		void adapterSet::clearWindows()
		{
			for (auto win : parent_windows)
			{
				delete win.second;
			}
			parent_windows.clear();
		}

		direct2dChildWindow* adapterSet::findChild(relative_ptr_type _child)
		{
			direct2dChildWindow* w = nullptr;
			for (auto win : parent_windows)
			{
				w = win.second->getChild(_child);
				if (w) {
					break;
				}
			}
			return w;
		}

		void adapterSet::loadStyleSheet(jobject& sheet, int _state)
		{
			direct2dChildWindow* w = nullptr;
			for (auto win : parent_windows)
			{
				win.second->loadStyleSheet(sheet, _state);
				for (auto& child : win.second->getChildren())
				{
					child.second->loadStyleSheet(sheet, _state);
				}
			}
		}
 
		direct2dBitmap* adapterSet::createD2dBitmap(D2D1_SIZE_F size)
		{
			direct2dBitmap* win = new direct2dBitmap(size, this);
			return win;
		}

		direct3dDevice::direct3dDevice()
		{
			d3d11Device = nullptr;
		}

		direct3dDevice::~direct3dDevice()
		{
			if (d3d11Device) 
			{
				d3d11Device->Release();
			}
			d3d11Device = nullptr;
		}

		bool direct3dDevice::setDevice(IDXGIAdapter1* _adapter)
		{
			if (d3d11Device != nullptr)
			{
				d3d11Device->Release();
				d3d11Device = nullptr;
				feature_level = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE;
			}

			D3D_FEATURE_LEVEL feature_levels[] = {
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1
			};

			HRESULT hr = D3D11CreateDevice(_adapter, 
				D3D_DRIVER_TYPE_UNKNOWN,
				NULL, 
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,   
				feature_levels,
				2,
				D3D11_SDK_VERSION,
				&d3d11Device,
				&feature_level,
				NULL
			);

			if (SUCCEEDED(hr) && d3d11Device != nullptr) 
			{
				return true;
			}
		}

		direct2dDevice::direct2dDevice()
		{
			d2dDevice = nullptr;
			d2DFactory = nullptr;
			wicFactory = nullptr;
			dWriteFactory = nullptr;
			dxDevice = nullptr;

			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2DFactory);
			throwOnFail(hr, "Could not create D2D1 factory");

			hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
			throwOnFail(hr, "Could not create WIC Imaging factory");

			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dWriteFactory), reinterpret_cast<IUnknown**>(&dWriteFactory));
			throwOnFail(hr, "Could not create direct write factory");
		}

		direct2dDevice::~direct2dDevice()
		{
			if (wicFactory) {
				wicFactory->Release();
				wicFactory = NULL;
			}
			if (dWriteFactory) {
				dWriteFactory->Release();
				dWriteFactory = NULL;
			}
			if (d2dDevice) {
				d2dDevice->Release();
				d2dDevice = NULL;
			}
			if (d2DFactory) {
				d2DFactory->Release();
				d2DFactory = NULL;
			}
			if (dxDevice) {
				dxDevice->Release();
				dxDevice = NULL;
			}
		}

		bool direct2dDevice::setDevice(ID3D11Device* _d3dDevice)
		{
			HRESULT hr = _d3dDevice->QueryInterface(&this->dxDevice);

			hr = d2DFactory->CreateDevice(dxDevice, &d2dDevice);

			return SUCCEEDED(hr);
		}

		//-------

		direct2dWindow::direct2dWindow(HWND _hwnd, adapterSet* _adapterSet) : direct2dContext(_adapterSet)
		{

			HRESULT hr;

			hwnd = _hwnd;
			bitmap = nullptr;
			surface = nullptr;
			swapChain = nullptr;
			renderTarget = nullptr;

			D2D1_DEVICE_CONTEXT_OPTIONS options;

			options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;

			hr = _adapterSet->getD2DDevice()->CreateDeviceContext(options, &renderTarget);
			throwOnFail(hr, "Could not create device context");

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

			hr = _adapterSet->getDxFactory()->CreateSwapChainForHwnd(_adapterSet->getD3DDevice(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
			throwOnFail(hr, "Could not create swap chain");

			DXGI_RGBA color;
			color.a = 1.0;
			color.r = 0.0;
			color.g = 0.0;
			color.b = 0.2;

			swapChain->SetBackgroundColor(&color);

			applySwapChain();
		}

		void direct2dWindow::resize(UINT width, UINT height)
		{
			HRESULT hr;

#if TRACE_RENDER
				std::cout << "%%%%%%%%% parent resize " << GetDlgCtrlID(hwnd) << " " << width << " " << height << std::endl;
#endif

			if (renderTarget)
			{
				renderTarget->SetTarget(nullptr);
			}
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

			applySwapChain();
		}

		void direct2dWindow::moveWindow(UINT x, UINT y, UINT w, UINT h)
		{
			double dpi = ::GetDpiForWindow(hwnd);

			MoveWindow(hwnd, x, y, w, h, false);
		}

		void direct2dWindow::applySwapChain()
		{
			HRESULT hr;

			float dpix, dpiy;
			int dpiWindow;
			RECT r;

			dpiWindow = ::GetDpiForWindow(hwnd);
			renderTarget->GetDpi(&dpix, &dpiy);

			GetClientRect(hwnd, &r);
			int x = r.right - r.left;
			int y = r.bottom - r.top;

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
			hr = renderTarget->CreateBitmapFromDxgiSurface(
				surface,
				&bitmapProperties,
				&bitmap
			);

			auto pxsx = bitmap->GetPixelSize();
			auto sizex = bitmap->GetSize();

			throwOnFail(hr, "Could create bitmap from surface");

			// Now we can set the Direct2D render target.
			auto dipssz = renderTarget->GetSize();
			auto pixssz = renderTarget->GetPixelSize();
			renderTarget->SetDpi(dpiWindow, dpiWindow);
			renderTarget->SetTarget(bitmap);
			dipssz = renderTarget->GetSize();

#if TRACE_RENDER
			std::cout << "render target pixel size " << pixssz.width << " " << pixssz.height << " " << std::endl;
			std::cout << "render target dip size " << dipssz.width << " " << dipssz.height << " " << std::endl;
#endif
			return;

		}

		direct2dWindow::~direct2dWindow()
		{
			for (auto child : children)
			{
				delete child.second;
			}

			children.clear();

			if (bitmap)
				bitmap->Release();
			if (surface)
				surface->Release();
			if (swapChain)
				swapChain->Release();
			if (renderTarget)
				renderTarget->Release();
		}

		direct2dChildWindow* direct2dWindow::createChild(relative_ptr_type _id, UINT _x, UINT _y, UINT _w, UINT _h)
		{
			direct2dChildWindow* child = nullptr;
			if (children.contains(_id)) {
				child = children[_id];
				child->moveWindow(_x, _y, _w, _h);
			}
			else 
			{
				child = new direct2dChildWindow(this, factory, _x, _y, _w, _h);
				children.insert_or_assign(_id, child);
			}
			return child;
		}

		direct2dChildWindow* direct2dWindow::getChild(relative_ptr_type _id)
		{
			direct2dChildWindow* child = nullptr;
			if (children.contains(_id)) {
				child = children[ _id ];
			}
			return child;
		}
		
		direct2dChildWindow* direct2dWindow::deleteChild(relative_ptr_type _id)
		{
			direct2dChildWindow* child = getChild(_id);
			if (child) {
				delete child;
				children.erase(_id);
			}
			return nullptr;
		}

		void direct2dWindow::beginDraw(bool& _adapter_blown_away)
		{
			currentTransform = D2D1::Matrix3x2F::Identity();
			_adapter_blown_away = false;

			HRESULT hr = S_OK;

			if (getRenderTarget()) {
				getRenderTarget()->BeginDraw();
			}
			;
		}

		void direct2dWindow::endDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;
			if (getRenderTarget()) {
				HRESULT hr = getRenderTarget()->EndDraw();

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

		direct2dChildWindow::direct2dChildWindow(direct2dWindow* _parent, adapterSet* _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips) : direct2dContext(_adapterSet)
		{
			HRESULT hr;

			parent = _parent;
			childBitmap = nullptr;

			windowPosition.x = _xdips;
			windowPosition.y = _ydips;
			windowPosition.w = _wdips;
			windowPosition.h = _hdips;

			resize(_wdips, _hdips);
		}

		void direct2dChildWindow::resize(UINT _wdips, UINT _hdips)
		{
			HRESULT hr;

#if TRACE_RENDER
			std::cout << "%%%%%%%%% child resize " << GetDlgCtrlID(hwnd) << " " << width << " " << height << std::endl;
#endif

			if (childBitmap)
				delete childBitmap;
			childBitmap = nullptr;

			int dpiWindow;
			dpiWindow = ::GetDpiForWindow(parent->getWindow());
			double dipsToPixels = dpiWindow / 96.0;

			D2D1_SIZE_F size;
			size.width = _wdips * dipsToPixels;
			size.height = _hdips * dipsToPixels;
			childBitmap = new direct2dBitmapCore(size, factory, dpiWindow);
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

		direct2dChildWindow::~direct2dChildWindow()
		{
			if (childBitmap)
				delete childBitmap;
		}

		void direct2dChildWindow::beginDraw(bool& _adapter_blown_away)
		{
			currentTransform = D2D1::Matrix3x2F::Identity();
			_adapter_blown_away = false;

			HRESULT hr = S_OK;

			if (getRenderTarget()) {
				getRenderTarget()->BeginDraw();
			}
		}

		void direct2dChildWindow::endDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;
			if (getRenderTarget()) {
				HRESULT hr = getRenderTarget()->EndDraw();
			}
		}

		//---

		direct2dContext::direct2dContext(adapterSet* _factory) :
			factory(_factory)
		{
		}

		direct2dContext::~direct2dContext()
		{
			clearPaths();
			clearViewStyles();
			clearBitmapsAndBrushes(true);
		}

		adapterSet* direct2dContext::getFactory()
		{
			return factory;
		}

		direct2dBitmapCore::direct2dBitmapCore(D2D1_SIZE_F _size, adapterSet* _adapterSet, int dpi) :
			size(_size)
		{
			targetContext = nullptr;
			target = nullptr;

			auto options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

			auto hr = _adapterSet->getD2DDevice()->CreateDeviceContext(options, &targetContext);
			throwOnFail(hr, "Could not create device context");

			D2D1_SIZE_U bmsize;

			bmsize.height = _size.height;
			bmsize.width = _size.width;

			D2D1_BITMAP_PROPERTIES1 props = {};

			props.dpiX = dpi;
			props.dpiY = dpi;
			props.pixelFormat.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
			props.pixelFormat.alphaMode = D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_IGNORE;
			props.bitmapOptions = D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_TARGET;

			hr = targetContext->CreateBitmap(bmsize, nullptr, 0,  props, &bitmap);
			throwOnFail(hr, "Could not create BITMAP");

			auto pxs = bitmap->GetPixelSize();
			auto ps = bitmap->GetSize();

#if TRACE_RENDER
			std::cout << "bitmap pixel size " << pxs.width << " " << pxs.height << std::endl;
			std::cout << "bitmap dips size " << ps.width << " " << ps.height << std::endl;
#endif

			targetContext->SetDpi(dpi, dpi);
			targetContext->SetTarget(bitmap);

			ps = targetContext->GetSize();
			pxs = targetContext->GetPixelSize();

			auto unitMode = targetContext->GetUnitMode();

#if TRACE_RENDER
			std::cout << "target pixel size " << pxs.width << " " << pxs.height << std::endl;
			std::cout << "target dips size " << ps.width << " " << ps.height << std::endl;
#endif

			return;
		}

		direct2dBitmapCore::~direct2dBitmapCore()
		{
			if (targetContext) targetContext->Release();
			if (target) target->Release();
			if (bitmap) bitmap->Release();
		}

		void direct2dBitmapCore::beginDraw(bool& blownAdapter)
		{
			blownAdapter = false;
			targetContext->BeginDraw();
		}

		void direct2dBitmapCore::endDraw(bool& blownAdapter)
		{
			blownAdapter = false;
			targetContext->EndDraw();
		}

		direct2dBitmap::direct2dBitmap(D2D1_SIZE_F _size, adapterSet* _factory) : 
			direct2dContext(_factory)
		{
			HRESULT hr;
			D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();

			hr = _factory->getWicFactory()->CreateBitmap(size.width, size.height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicBitmap);
			throwOnFail(hr, "Could not create WIC bitmap");

			hr = _factory->getD2DFactory()->CreateWicBitmapRenderTarget(wicBitmap, props, &target);
			throwOnFail(hr, "Could not create WIC render target");

			hr = target->QueryInterface(&targetContext);
			throwOnFail(hr, "Could not get WIC context");
		}

		direct2dBitmap::~direct2dBitmap()
		{
			if (targetContext) targetContext->Release();
			if (target) target->Release();
			if (wicBitmap) wicBitmap->Release();
		}

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

			virtual void save(const wchar_t* _filename)
			{

				HRESULT hr;

				hr = dBitmap->getFactory()->getWicFactory()->CreateStream(&fileStream);
				throwOnFail(hr, "Could not create file stream");

				hr = fileStream->InitializeFromFilename(_filename, GENERIC_WRITE);
				throwOnFail(hr, "Could not initialize file stream");

				hr = dBitmap->getFactory()->getWicFactory()->CreateEncoder(GUID_ContainerFormatPng, NULL, &bitmapEncoder);
				throwOnFail(hr, "Could not create bitmap encoder");

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
		};

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

		class deviceDependentAssetBase {
		public:

			bool stock;

			deviceDependentAssetBase() : stock(false)
			{
				;
			}

			virtual ~deviceDependentAssetBase()
			{
				;
			}

			virtual bool create(direct2dContext* target) = 0;
			virtual void release() = 0;

			virtual ID2D1Brush* getBrush()
			{
				return NULL;
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

			std::string get_fontName() { return fontName;  };
			float get_size() { return size; }
			bool get_bold() { return bold; }
			bool get_italic()  { return italic; }
			bool get_underline() { return underline; }
			bool get_strike_through() { return strike_through; }
			double get_line_spacing() { return line_spacing; }
			visual_alignment get_horizontal_align() { return horizontal_align; }
			visual_alignment get_vertical_align() { return vertical_align; }
			bool get_wrap_text() { return wrap_text;  }

			virtual bool create(direct2dContext* target)
			{
				HRESULT hr = -1;

				if (!target || !target->getRenderTarget())
					return false;

				istring<2048> fontList = fontName;
				istring<2048> fontName;

				int state = 0;
				char* fontExtractedName = fontList.next_token(',', state);
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
					target->getRenderTarget()->GetDpi(&dpiX, &dpiY);

					HRESULT hr = target->factory->getDWriteFactory()->CreateTextFormat(wideName.c_str(),
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
				return false;
			}

			virtual void release()
			{
				if (lpWriteTextFormat)
					lpWriteTextFormat->Release();
				lpWriteTextFormat = NULL;
			}

			IDWriteTextFormat* getFormat()
			{
				return lpWriteTextFormat;
			}

		};

		template <class T> class deviceDependentAsset : public deviceDependentAssetBase {
		protected:
			T asset;

		public:

			deviceDependentAsset() : asset(NULL)
			{
				;
			}

			virtual ~deviceDependentAsset()
			{
				release();
			}

			virtual bool create(direct2dContext* target) = 0;

			bool recreate(direct2dContext* target)
			{
				release();
				create(target);
			}

			inline T getAsset() { return asset; }

		protected:

			virtual void release()
			{
				if (asset) asset->Release();
				asset = NULL;
			}
		};

		struct sizeCrop {
			D2D1_SIZE_U size;
			bool cropEnabled;
			D2D1_RECT_F crop;
		};


		sizeCrop toSizeC(point& _size, bool _cropEnabled, rectangle& _crop)
		{
			sizeCrop sz;
			sz.size.width = _size.x;
			sz.size.height = _size.y;
			sz.cropEnabled = _cropEnabled;
			sz.crop.left = _crop.x;
			sz.crop.top = _crop.y;
			sz.crop.right = _crop.x + _crop.w;
			sz.crop.bottom = _crop.y + _crop.h;
			return sz;
		}

		D2D1_SIZE_U toSizeU(point& _size)
		{
			D2D1_SIZE_U newSize;
			newSize.width = _size.x;
			newSize.height = _size.y;
			return newSize;
		}

		D2D1_SIZE_F toSizeF(point& _size)
		{
			D2D1_SIZE_F newSize;
			newSize.width = _size.x;
			newSize.height = _size.y;
			return newSize;
		}

		point toSize(D2D1_SIZE_U& _size)
		{
			point newSize;
			newSize.x = _size.width;
			newSize.y = _size.height;
			return newSize;
		}

		class brush
		{
		public:
			virtual ID2D1Brush* getBrush() = 0;
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

			bool create(direct2dContext* _target, IWICBitmapSource* _source)
			{
				HRESULT hr;

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

				hr = _target->getFactory()->getWicFactory()->CreateFormatConverter(&pConverter);
				hr = _source->GetSize(&originalWidth, &originalHeight);

				// If a new width or height was specified, create an
				// IWICBitmapScaler and use it to resize the image.
				if (size.width != 0 || size.height != 0)
				{
					if (SUCCEEDED(hr))
					{
						if (cropEnabled) {
							hr = _target->getFactory()->getWicFactory()->CreateBitmapClipper(&pClipper);
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


						hr = _target->getFactory()->getWicFactory()->CreateBitmapScaler(&pScaler);
						if (SUCCEEDED(hr))
							hr = pScaler->Initialize(_source, size.width, size.height, WICBitmapInterpolationModeCubic);
						if (SUCCEEDED(hr))
							hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
						if (SUCCEEDED(hr)) {
							hr = _target->getFactory()->getWicFactory()->CreateBitmapFromSource(pConverter, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &originalScaledBitmap);
							hr = _target->getFactory()->getWicFactory()->CreateBitmapFromSource(pConverter, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicFilteredScaledBitmap);
						}
					}
				}
				else // Don't scale the image.
				{
					hr = pConverter->Initialize(_source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
					if (SUCCEEDED(hr)) {
						if (cropEnabled) {
							hr = _target->getFactory()->getWicFactory()->CreateBitmapClipper(&pClipper);
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
						hr = _target->getFactory()->getWicFactory()->CreateBitmapFromSource(_source, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &originalScaledBitmap);
						hr = _target->getFactory()->getWicFactory()->CreateBitmapFromSource(_source, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicFilteredScaledBitmap);
					}
				}

				if (pConverter) { pConverter->Release(); pConverter = NULL; }
				if (pScaler) { pScaler->Release(); pScaler = NULL; }
				if (pClipper) { pClipper->Release(); pClipper = NULL; }

				return SUCCEEDED(hr);
			}

			bool make(direct2dContext* _target)
			{
				HRESULT hr = 0;
				if (filteredScaledBitmap) {
					filteredScaledBitmap->Release();
					filteredScaledBitmap = NULL;
				}

				if (wicFilteredScaledBitmap) {
					hr = _target->getRenderTarget()->CreateBitmapFromWicBitmap(wicFilteredScaledBitmap, &filteredScaledBitmap);
				}
				return hr == S_OK;
			}

			virtual ~filteredBitmap()
			{
				release();
			}

			void release()
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
		};

		class nullFilterFunction {
		public:
			bool operator()(point, int, int, char*)
			{
				return true;
			}
		};

		class bitmap : public deviceDependentAssetBase
		{
			bool useFile;
			std::string filename;
			std::list<filteredBitmap*> filteredBitmaps;
			std::function<bool(point, int, int, char*)> filterFunction;

			void clearFilteredBitmaps()
			{
				for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
					filteredBitmap* bm = *ifb;
					delete bm;
				}
				filteredBitmaps.clear();
			}

			void setFilteredBitmaps(std::list<sizeCrop>& _sizes)
			{
				for (auto i = _sizes.begin(); i != _sizes.end(); i++) {
					auto sizeCrop = *i;
					filteredBitmap* bm = new filteredBitmap(sizeCrop.size, sizeCrop.cropEnabled, sizeCrop.crop);
					filteredBitmaps.push_back(bm);
				}
			}

			void copyFilteredBitmaps(direct2dContext* _targetContext, bitmap* _src)
			{
				for (auto i = _src->filteredBitmaps.begin(); i != _src->filteredBitmaps.end(); i++) {
					auto srcfiltered = *i;
					filteredBitmap* bm = srcfiltered->clone(_targetContext);
					filteredBitmaps.push_back(bm);
				}
			}

			bitmap(direct2dContext* _targetContext, bitmap* _src)
				: useFile(_src->useFile),
				filename(_src->filename),
				filterFunction(_src->filterFunction)
			{
				copyFilteredBitmaps(_targetContext, _src);
				applyFilters(_targetContext);
			}

		public:

			bitmap(std::string& _filename, std::list<sizeCrop>& _sizes) :
				useFile(true),
				filename(_filename)
			{
				nullFilterFunction defaultFunc;
				filterFunction = defaultFunc;
				setFilteredBitmaps(_sizes);
			}

			virtual ~bitmap()
			{
				clearFilteredBitmaps();
			}

			virtual bitmap* clone(direct2dContext* _src)
			{
				return new bitmap(_src, this);
			}

			void setSizes(std::list<sizeCrop>& _sizes)
			{
				clearFilteredBitmaps();
				setFilteredBitmaps(_sizes);
			}

			bool getSize(int* _sizex, int* _sizey)
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

			ID2D1Bitmap* getFirst()
			{
				auto afb = filteredBitmaps.front();
				return afb->filteredScaledBitmap;
			}

			ID2D1Bitmap* getBySize(int _width, int _height)
			{
				auto iter = std::find_if(filteredBitmaps.begin(), filteredBitmaps.end(), [_width, _height](filteredBitmap* _bm) { return _bm->size.width == _width && _bm->size.height == _height; });
				if (iter != filteredBitmaps.end()) {
					return (*iter)->filteredScaledBitmap;
				}
				else {
					return NULL;
				}
			}

			color getColorAtPoint(int _width, int _height, point point)
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
									color.alpha = pix.alpha / 255.0;
									color.blue = pix.blue / 255.0;
									color.red = pix.red / 255.0;
									color.green = pix.green / 255.0;
									// because this is premultiplied alpha
									if (color.alpha >= 0.0) {
										color.blue /= color.alpha;
										color.green /= color.alpha;
										color.red /= color.alpha;
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

			void setFilter(std::function<bool(point, int, int, char* bytes)> _filter)
			{
				filterFunction = _filter;
			}

			void filter()
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

			virtual bool applyFilters(direct2dContext* _target)
			{
				filter();

				for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
					filteredBitmap* bm = *ifb;
					bm->make(_target);
				}

				return true;
			}

			virtual bool create(direct2dContext* _target)
			{

				HRESULT hr;
				IWICBitmapDecoder* pDecoder = NULL;
				IWICBitmapFrameDecode* pSource = NULL;

				wchar_t fileBuff[1024];
				int ret = ::MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, fileBuff, sizeof(fileBuff) / sizeof(wchar_t) - 1);

				hr = _target->getFactory()->getWicFactory()->CreateDecoderFromFilename(fileBuff, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

				if (SUCCEEDED(hr))
					hr = pDecoder->GetFrame(0, &pSource);

				if (SUCCEEDED(hr))
				{
					for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
						filteredBitmap* bm = *ifb;
						bm->create(_target, pSource);
					}

					filter();

					for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
						filteredBitmap* bm = *ifb;
						bm->make(_target);
					}

				}

				if (pDecoder) { pDecoder->Release(); pDecoder = NULL; }
				if (pSource) { pSource->Release(); pSource = NULL; }

				return SUCCEEDED(hr);
			}

			virtual void release()
			{
				for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
					filteredBitmap* bm = *ifb;
					bm->release();
				}
			}
		};

		class bitmapBrush : public deviceDependentAsset<ID2D1BitmapBrush*>, brush {
		public:

			bitmap* bm;

			bitmapBrush() : deviceDependentAsset()
			{
				;
			}

			virtual ~bitmapBrush()
			{

			}

			virtual bool create(direct2dContext* target)
			{
				if (!target || !target->getRenderTarget())
					return false;

				HRESULT hr = target->getRenderTarget()->CreateBitmapBrush(bm->getFirst(), &asset);

				return SUCCEEDED(hr);
			}

			virtual ID2D1Brush* getBrush()
			{
				return asset;
			}
		};

		class solidColorBrush : public deviceDependentAsset<ID2D1SolidColorBrush*>, brush {
		public:

			D2D1_COLOR_F color;

			solidColorBrush() : deviceDependentAsset()
			{
				;
			}

			virtual ~solidColorBrush()
			{

			}

			bool create(direct2dContext* target)
			{
				HRESULT hr = -1;

				if (!target || !target->getRenderTarget())
					return false;

				hr = target->getRenderTarget()->CreateSolidColorBrush(color, &asset);

				return SUCCEEDED(hr);
			}

			ID2D1Brush* getBrush()
			{
				return asset;
			}

		};

		class linearGradientBrush : public deviceDependentAsset<ID2D1LinearGradientBrush*>, brush {
		public:
			std::vector<D2D1_GRADIENT_STOP> stops;
			D2D1_POINT_2F					start,
				stop;

			virtual bool create(direct2dContext* target)
			{
				ID2D1GradientStopCollection* pGradientStops = NULL;

				if (!target || !target->getRenderTarget())
					return false;

				HRESULT hr = target->getRenderTarget()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

				if (SUCCEEDED(hr))
				{
					hr = target->getRenderTarget()->CreateLinearGradientBrush(
						D2D1::LinearGradientBrushProperties(start, stop),
						D2D1::BrushProperties(),
						pGradientStops,
						&asset
					);
					pGradientStops->Release();
				}
				return SUCCEEDED(hr);
			}

			virtual ID2D1Brush* getBrush()
			{
				return asset;
			}

		};

		class radialGradientBrush : public deviceDependentAsset<ID2D1RadialGradientBrush*>, brush {
		public:
			std::vector<D2D1_GRADIENT_STOP> stops;
			D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialProperties;

			bool create(direct2dContext* target)
			{
				ID2D1GradientStopCollection* pGradientStops = NULL;

				if (!target || !target->getRenderTarget())
					return false;

				HRESULT hr = target->getRenderTarget()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

				if (SUCCEEDED(hr))
				{
					hr = target->getRenderTarget()->CreateRadialGradientBrush(
						radialProperties,
						D2D1::BrushProperties(),
						pGradientStops,
						&asset
					);
					pGradientStops->Release();
				}
				return SUCCEEDED(hr);
			}

			virtual ID2D1Brush* getBrush()
			{
				return asset;
			}
		};

		class path {
		public:
			ID2D1PathGeometry* geometry;
			ID2D1GeometrySink* sink;

			path(direct2dContext* target) : geometry(NULL), sink(NULL)
			{
				HRESULT hr = target->factory->getD2DFactory()->CreatePathGeometry(&geometry);
				if (!SUCCEEDED(hr)) {
					// UH, SOMETHING;
				}
			}

			virtual ~path()
			{
				if (sink) sink->Release();
				if (geometry) geometry->Release();
				sink = NULL;
				geometry = NULL;
			}

			void start_figure(D2D1_POINT_2F point)
			{
				if (geometry) {
					geometry->Open(&sink);
					if (sink)
						sink->BeginFigure(point, D2D1_FIGURE_BEGIN_FILLED);
				}
			}

			void add_line(D2D1_POINT_2F point)
			{
				if (sink) sink->AddLine(point);
			}

			void add_arc(D2D1_POINT_2F point1, D2D1_SIZE_F size1, FLOAT rotationAngle)
			{
				D2D1_SWEEP_DIRECTION direction = rotationAngle > 0.0 ? D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE : D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_CLOCKWISE;
				rotationAngle = fabs(rotationAngle);
				D2D1_ARC_SIZE arcSize = rotationAngle > 180.0 ? D2D1_ARC_SIZE::D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL;
				if (sink) sink->AddArc(D2D1::ArcSegment(point1, size1, fabs(rotationAngle), direction, arcSize));
			}

			void add_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2, D2D1_POINT_2F point3)
			{
				if (sink) sink->AddBezier(D2D1::BezierSegment(point1, point2, point3));
			}

			void add_quadratic_bezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2)
			{
				if (sink) sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(point1, point2));
			}

			void close_figure(bool closed = true)
			{
				if (sink) {
					sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
					sink->Close();
					sink->Release();
					sink = NULL;
				}
			}
		};

		D2D1_COLOR_F toColor(color& _color)
		{
			D2D1_COLOR_F newColor;
			newColor.a = _color.alpha;
			newColor.b = _color.blue;
			newColor.r = _color.red;
			newColor.g = _color.green;
			return newColor;
		}

		D2D1_POINT_2F toPoint(point& _point)
		{
			D2D1_POINT_2F point2;
			point2.x = _point.x;
			point2.y = _point.y;
			return point2;
		}

		D2D1_GRADIENT_STOP toGradientStop(gradientStop& _gradientStop)
		{
			D2D1_GRADIENT_STOP stop;

			stop.position = _gradientStop.position;
			stop.color = toColor(_gradientStop.color);
			return stop;
		}

		// -------------------------------------------------------

		directApplicationWin32* directApplicationWin32::current;

		directApplicationWin32::directApplicationWin32(adapterSet* _factory) : factory(_factory), colorCapture(false)
		{
			current = this;
			previousController = NULL;
			currentController = NULL;
			controlFont = nullptr;
			labelFont = nullptr,
			titleFont = nullptr;
			dpiScale = 1.0;
			disableChangeProcessing = false;
		}

		directApplicationWin32::~directApplicationWin32()
		{

		}


		void directApplicationWin32::redraw(int controlId)
		{
			if (currentController)
			{
				bool failedDevice = currentController->drawCanvas(controlId);
				if (failedDevice) {
					factory->clearWindows();
					factory->refresh();
				}
			}
		}

		void directApplicationWin32::redraw()
		{
			static int counter = 0;

			if (currentController) 
			{
			
				currentController->drawFrame();

				bool failedDevice = false;

				auto winroot = factory->getWindow(hwndRoot);

				if (winroot == nullptr)
					return;

				double toDips = 96.0 / GetDpiForWindow(winroot->getWindow());

				winroot->beginDraw(failedDevice);						
				color c = { 1.0, 1.0, 1.0, 1.0 };
				winroot->clear(&c);

				if (!failedDevice) 
				{
					auto wins = winroot->getChildren();

					int iy = 200;
					relative_ptr_type id = 0;

					for (auto& w : wins)
					{
						rectangle r = w.second->getBoundsDips();

						id = w.first;

						D2D1_RECT_F dest;
						dest.left = r.x;
						dest.top = r.y;
						dest.right = r.x + r.w;
						dest.bottom = r.y + r.h;

#if TRACE_RENDER
						std::cout << "Compose item#" << id << " " << dest.left << " " << dest.top << " " << dest.right << " " << dest.bottom << std::endl;
#endif

						winroot->getRenderTarget()->DrawBitmap(w.second->getBitmap(), &dest);

						std::string temp = std::format("child {} {} counter", id, counter);

						rectangle dest2;
						dest2.x = 50.0;
						dest2.y = iy;
						dest2.w = 500.0;
						dest2.h = 50.0;

						iy += 50.0;
						counter++;
					}

					winroot->endDraw(failedDevice);
				}

			failed_check:

				if (failedDevice) {
					factory->clearWindows();
					factory->refresh();
				}
			}
		}

		void directApplicationWin32::destroyChildren()
		{
			for (auto& child : oldWindowControlMap)
			{
				if (!windowControlMap.contains(child.first)) 
				{
					if (child.second.window) 
					{
						char buff[512];
						GetClassName(child.second.window, buff, sizeof(buff));
#if TRACE_SIZE
						std::cout << "Destroying " << buff << std::endl;
#endif
						DestroyWindow(child.second.window);
					}
					else
					{
						factory->getWindow(hwndRoot)->deleteChild(child.first);
					}
				}
			}
			oldWindowControlMap = windowControlMap;
		}

		bool directApplicationWin32::createChildWindow(
			page_item_identifier pid,
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
		)
		{

			HWND hwnd = nullptr;
			bool created_something = false;

			x /= dpiScale;
			y /= dpiScale;
			nWidth /= dpiScale;
			nHeight /= dpiScale;

			dwStyle |= WS_CLIPSIBLINGS;

			if (oldWindowControlMap.contains(pid))
			{
				auto wi = oldWindowControlMap[pid];
				int old_id = GetDlgCtrlID(wi.window);
				SetWindowLongPtr(wi.window, GWL_ID, item.id);
				if (factory->containsWindow(wi.window)) {
					auto w = factory->getWindow(wi.window);
					w->moveWindow(x, y, nWidth, nHeight);
				}
				else 
				{
					MoveWindow(wi.window, x, y, nWidth, nHeight, true);
				}
				hwnd = wi.window;
			}
			else
			{
				created_something = true;
				hwnd = CreateWindow(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, directApplicationWin32::hwndRoot, (HMENU)windowId, hinstance, lpParam);
				if (font)
				{
					SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
				}
			}

			windowMapItem wmi = { hwnd };
			windowControlMap.insert_or_assign(pid, wmi);
			message_map.insert_or_assign(windowId, item);
			return created_something;
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

		void directApplicationWin32::loadStyleSheet()
		{

			if (currentController) 
			{
				auto styles = currentController->get_style_sheet(0);
				auto schema = styles.get_schema();

				HFONT oldControlFont = controlFont;
				HFONT oldLabelFont = labelFont;
				HFONT oldTitleFont = titleFont;

				controlFont = createFontFromStyleSheet(schema->idf_control_style);
				labelFont = createFontFromStyleSheet(schema->idf_label_style);
				titleFont = createFontFromStyleSheet(schema->idf_view_subtitle_style);

				if (oldControlFont) 
				{
					DeleteObject(oldControlFont);
				}

				if (oldLabelFont) 
				{
					DeleteObject(oldLabelFont);
				}

				if (oldTitleFont) 
				{
					DeleteObject(oldTitleFont);
				}

				for (int i = 0; i < styles_count; i++)
				{
					factory->loadStyleSheet(styles, i);
				}
			}
		}


		HFONT directApplicationWin32::createFontFromStyleSheet(relative_ptr_type _style_id)
		{
			HFONT hfont = nullptr;

			if (currentController) {
				auto sheet = currentController->get_style_sheet( style_normal );
				auto style_field = sheet.get_object(_style_id, true)
					.get_object(0);
				auto schema = style_field.get_schema();
				double fontSize = style_field.get(schema->idf_font_size);
				double ifontSize = fontSize / dpiScale;
				istring<2048> fontList = (const char *)style_field.get(schema->idf_font_name);
				bool italic = (int32_t)style_field.get(schema->idf_italic);
				bool bold = (int32_t)style_field.get(schema->idf_bold);

				int state = 0;
				char* fontExtractedName = fontList.next_token(',', state);

				while (fontExtractedName && !hfont)
				{
					hfont = CreateFont(-ifontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontExtractedName);
					fontExtractedName = fontList.next_token(',', state);
				}
			}
			return hfont;
		}

		int directApplicationWin32::renderPage(database::page& _page, database::jschema* _schema, database::actor_state& _state, database::jcollection& _collection)
		{
			if (disableChangeProcessing)
				return 0;

			disableChangeProcessing = true;

			int canvasWindowId = -1;

			windowControlMap.clear();
			message_map.clear();

			auto dpi = GetDpiForWindow(hwndRoot);
			auto *win = factory->getWindow(hwndRoot);

			bool created_anything = false;
			bool created_something = false;

			database::jobject slice;
			for (auto piter : _page)
			{
				auto pi = piter.item;

				if (pi.bounds.w < 1 || pi.bounds.h < 1)
					continue;

				auto pid = pi.get_identifier();

				switch (pi.layout)
				{
				case database::layout_types::canvas2d_row:
				case database::layout_types::canvas2d_column:
				case database::layout_types::canvas2d_absolute:
					{
						windowMapItem wmi{};

						auto child = win->createChild(pid, pi.bounds.x, pi.bounds.y, pi.bounds.w, pi.bounds.h);
						for (int i = 0; i < styles_count; i++) 
						{
							auto ss = currentController->get_style_sheet(i);
							child->loadStyleSheet(ss, i);
						}
						windowControlMap.insert_or_assign(pid, wmi);
					}
					break;
				case database::layout_types::text_window:
					{
						database::istring<256> x;
						auto slice = _collection.get_object(pi.object_path);
						int idx = slice.get_field_index_by_id(pi.field->field_id);
						switch (pi.field->type_id)
						{
						case database::type_int8:
							{
								auto bx = slice.get_int8(idx);
								x = bx;
							}
							break;
						case database::type_int16:
							{
								auto bx = slice.get_int16(idx);
								x = bx;
							}
							break;
						case database::type_int32:
							{
								auto bx = slice.get_int32(idx);
								x = bx;
							}
							break;
						case database::type_int64:
							{
								auto bx = slice.get_int64(idx);
								x = bx;
							}
							break;
						case database::type_float32:
							{
								auto bx = slice.get_float(idx);
								x = bx;
							}
							break;
						case database::type_float64:
							{
								auto bx = slice.get_double(idx);
								x = bx;
							}
							break;
						case database::type_string:
							{
								auto bx = slice.get_string(idx);
								x = bx;
							}
							break;
						default:
							break;
						}
						created_something = createChildWindow(pid, WC_EDIT, x.c_str(), WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VISIBLE, pi.bounds.x, pi.bounds.y, pi.bounds.w, pi.bounds.h, pi.id, NULL, controlFont, pi);
					}
					break;
/*				case database::layout_types::set:
					created_something = createChildWindow(pid, WC_BUTTON, pi.caption, BS_PUSHBUTTON | BS_FLAT | WS_TABSTOP | WS_CHILD | WS_VISIBLE, pi.bounds.x, pi.bounds.y, pi.bounds.w, pi.bounds.h, pi.id, NULL, controlFont, pi);
					break;
				case database::layout_types::create:
					created_something = createChildWindow(pid, WC_BUTTON, pi.caption, BS_PUSHBUTTON | BS_FLAT | WS_TABSTOP | WS_CHILD | WS_VISIBLE, pi.bounds.x, pi.bounds.y, pi.bounds.w, pi.bounds.h, pi.id, NULL, controlFont, pi);
					break;
				case database::layout_types::select:
					break;
				default:
					{
					auto bx = pi.bounds;
					RECT r;
					r.left = bx.x * 96.0 / dpi;
					r.top = bx.y * 96.0 / dpi;
					r.right = r.left + bx.w * 96.0 / dpi;
					r.bottom = r.top + bx.h * 96.0 / dpi;
					InvalidateRect(hwndRoot, &r, true);
					}
*/
				}
				if (created_something)
					created_anything = true;
			}

			destroyChildren();
			disableChangeProcessing = false;	
			UpdateWindow(hwndRoot);

			return created_anything;
		}

		LRESULT CALLBACK directApplicationWin32::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			return current->windowProcHandler(hwnd, message, wParam, lParam);
		}

		LRESULT directApplicationWin32::windowProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			bool found = false;
			point point;
			static HBRUSH hbrBkgnd = NULL;
			static HBRUSH hbrBkgnd2 = NULL;
			char className[256];
			database::point ptz;

			auto* winx = factory->getWindow(hwnd);

			switch (message)
			{
			case WM_CREATE:
				hwndRoot = hwnd;
				if (currentController) {
					auto* win = factory->createD2dWindow(hwnd);
					dpiScale = 96.0 / GetDpiForWindow(hwnd);
					loadStyleSheet();
					currentController->onCreated();
				}
				break;
			case WM_INITDIALOG:
				break;
			case WM_DESTROY:
				factory->closeWindow(hwnd);
				PostQuitMessage(0);
				return 0;
			case WM_COMMAND:
				if (currentController && !disableChangeProcessing)
				{
					UINT controlId = LOWORD(wParam);
					UINT notificationCode = HIWORD(wParam);
					database::page_item pi;
					if (message_map.contains(controlId)) {
						pi = message_map[controlId];
					}
					switch (notificationCode) {
					case BN_CLICKED: // button or menu
						currentController->onCommand(controlId, pi);
						break;
					case EN_UPDATE:
						currentController->onTextChanged(controlId, pi);
						break;
					case CBN_SELCHANGE:
						currentController->onDropDownChanged(controlId, pi);
						break;
					}
					break;
				}
				break;
			case WM_DPICHANGED:
				if (currentController)
				{
					RECT* const prcNewWindow = (RECT*)lParam;
					SetWindowPos(hwnd,
						NULL,
						prcNewWindow->left,
						prcNewWindow->top,
						prcNewWindow->right - prcNewWindow->left,
						prcNewWindow->bottom - prcNewWindow->top,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
				break;
			case WM_NOTIFY:
				if (currentController && !disableChangeProcessing)
				{
					//LVN_ITEMACTIVATE
					LPNMHDR lpnm = (LPNMHDR)lParam;
					switch (lpnm->code) {
					case UDN_DELTAPOS:
					{
						auto lpnmud = (LPNMUPDOWN)lParam;
						database::page_item pi;
						if (message_map.contains(lpnm->idFrom)) {
							pi = message_map[lpnm->idFrom];
						}
						currentController->onSpin(lpnm->idFrom, lpnmud->iPos + lpnmud->iDelta, pi);
						return 0;
					}
					break;
					case LVN_ITEMCHANGED:
					{
						auto lpmnlv = (LPNMLISTVIEW)lParam;
						database::page_item pi;
						if (message_map.contains(lpnm->idFrom)) {
							pi = message_map[lpnm->idFrom];
						}
						if (lpmnlv->uNewState & LVIS_SELECTED)
							currentController->onListViewChanged(lpnm->idFrom, pi);
					}
					break;
					case NM_CLICK:
					{

						::GetClassNameA(lpnm->hwndFrom, className, sizeof(className) - 1);
						if (strcmp(className, "SysLink") == 0) {
							auto plink = (PNMLINK)lParam;
							auto r = ::ShellExecuteW(NULL, L"open", plink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
						}
					}
					break;
					case NM_CUSTOMDRAW:
					{
						LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
						switch (lplvcd->nmcd.dwDrawStage) {
						case CDDS_PREPAINT:
							return CDRF_NOTIFYITEMDRAW;
						case CDDS_ITEMPREPAINT:
							LVITEM lvitem;
							char buff[16384];
							ZeroMemory(&lvitem, sizeof(lvitem));
							lvitem.iItem = lplvcd->nmcd.dwItemSpec;
							lvitem.stateMask = LVIS_SELECTED;
							lvitem.mask = LVIF_STATE | LVIF_TEXT;
							lvitem.cchTextMax = sizeof(buff) - 1;
							lvitem.pszText = buff;
							HWND control = ::GetDlgItem(hwndRoot, lplvcd->nmcd.hdr.idFrom);
							ListView_GetItem(control, &lvitem);

							RECT area = lplvcd->nmcd.rc;

							if (lvitem.state & LVIS_SELECTED) {
								::FillRect(lplvcd->nmcd.hdc, &area, GetSysColorBrush(COLOR_HIGHLIGHT));
								::SetTextColor(lplvcd->nmcd.hdc, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
								::DrawTextA(lplvcd->nmcd.hdc, lvitem.pszText, strlen(lvitem.pszText), &area, DT_CENTER);
							}
							else {
								::FillRect(lplvcd->nmcd.hdc, &area, GetSysColorBrush(COLOR_WINDOW));
								::SetTextColor(lplvcd->nmcd.hdc, ::GetSysColor(COLOR_WINDOWTEXT));
								::DrawTextA(lplvcd->nmcd.hdc, lvitem.pszText, strlen(lvitem.pszText), &area, DT_CENTER);
							}
							return CDRF_SKIPDEFAULT;
						}
					}
					}
				}
				break;
			case WM_GETMINMAXINFO:
				if (minimumWindowSize.x > 0) {
					auto minmaxinfo = (LPMINMAXINFO)lParam;
					minmaxinfo->ptMinTrackSize.x = minimumWindowSize.x;
					minmaxinfo->ptMinTrackSize.y = minimumWindowSize.y;
					return 0;
				}
				break;
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLOREDIT:
			{
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				if (hbrBkgnd == NULL)
				{
					hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
				}
				return (INT_PTR)hbrBkgnd;
			}
			break;
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORSTATIC:
			{
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				if (hbrBkgnd == NULL)
				{
					hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
				}
				return (INT_PTR)hbrBkgnd;
			}
			break;
			case WM_ERASEBKGND:
			{
				RECT rect, rect2;
				HDC eraseDc = (HDC)wParam;
				if (hbrBkgnd == NULL)
				{
					hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
				}
				::GetClientRect(hwnd, &rect);
				::FillRect((HDC)wParam, &rect, hbrBkgnd);
				return 0;
			}
			break;

			case WM_CHAR:
			case WM_RBUTTONDOWN:
			case WM_CANCELMODE:
				if (colorCapture) {
					colorCapture = false;
					::ReleaseCapture();
					::SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
				break;

			case WM_CAPTURECHANGED:
				colorCapture = false;
				break;

			case WM_LBUTTONDOWN:
				if (colorCapture) {
					colorCapture = false;
					::ReleaseCapture();
					::SetCursor(LoadCursor(NULL, IDC_ARROW));
					POINT p;
					if (GetCursorPos(&p))
					{
						HDC hdc = ::GetDC(NULL);
						if (hdc) {
							COLORREF cr = ::GetPixel(hdc, p.x, p.y);
							color pickedColor;
							pickedColor.red = GetRValue(cr) / 255.0;
							pickedColor.green = GetGValue(cr) / 255.0;
							pickedColor.blue = GetBValue(cr) / 255.0;
							ptz.x = p.x;
							ptz.y = p.y;
							if (currentController)
								currentController->pointSelected(winx, &ptz, &pickedColor);
						}
					}
				}
				else if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						currentController->mouseClick(winx, &ptxo);
					}
				}
				break;
			case WM_MOUSEMOVE:
				if (currentController)
				{
					POINT p;
					if (GetCursorPos(&p))
					{
						ScreenToClient(hwnd, &p);
						database::point ptxo;
						ptxo.x = p.x * 96.0 / GetDpiForWindow(hwnd);
						ptxo.y = p.y * 96.0 / GetDpiForWindow(hwnd);
						currentController->mouseMove(winx, &ptxo);
					}
				}
				break;
			case WM_PAINT:
			{
				ValidateRect(hwnd, nullptr);
/*				PAINTSTRUCT ps;
				BeginPaint(hwnd, &ps);
				EndPaint(hwnd, &ps); */
				redraw();
				return 0;
			}
			case WM_SIZE:
				{
					RECT l;
					::GetClientRect(hwnd, &l);
					rectangle rect;
					rect.x = 0;
					rect.y = 0;
					rect.w = abs(l.right - l.left);
					rect.h = abs(l.bottom - l.top);
					auto* win = factory->getWindow(hwnd);
					win->resize(rect.w, rect.h);
					if (currentController) {
						dpiScale = 96.0 / GetDpiForWindow(hwnd);
	#if TRACE_SIZE
						std::cout << " w " << rect.w << "h " << rect.h << std::endl;

#endif
						rect.w *= dpiScale;
						rect.h *= dpiScale;
						currentController->onResize(rect, dpiScale);
					}
				}
				break;
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		drawableHost* directApplicationWin32::getDrawable(relative_ptr_type i)
		{	
			auto witem = this->windowControlMap[i];
			auto w = witem.window;
			auto wmi = factory->getWindow(w);
			return wmi;
		}

		direct2dChildWindow* directApplicationWin32::getWindow(relative_ptr_type i)
		{
			auto wmi = factory->findChild(i);
			return wmi;
		}

		void directApplicationWin32::setController(controller* _newCurrentController)
		{
			::QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
			pressedKeys.clear();
			if (currentController)
				delete currentController;
			currentController = _newCurrentController;
			currentController->attach(this);
			::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);
		}

		void directApplicationWin32::pushController(controller* _newCurrentController)
		{
			pressedKeys.clear();
			if (previousController)
				delete previousController;
			previousController = currentController;
			currentController = _newCurrentController;
			currentController->attach(this);
			::QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
			::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);
		}

		void directApplicationWin32::popController()
		{
			pressedKeys.clear();
			if (currentController)
				delete currentController;
			currentController = previousController;
			previousController = NULL;
			::QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
			::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);
		}

		bool directApplicationWin32::runFull(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController)
		{
			if (!_firstController)
				return false;

			::InitCommonControls();

			WNDCLASS wcMain;
			MSG msg;
			DWORD dwStyle, dwExStyle;

			hinstance = _hinstance;

			// register the control for the direct2d WINDOW - THIS is the main window this time.

			wcMain.style = CS_OWNDC;
			wcMain.lpfnWndProc = &directApplicationWin32::windowProc;
			wcMain.cbClsExtra = 0;
			wcMain.cbWndExtra = DLGWINDOWEXTRA;
			wcMain.hInstance = hinstance;
			wcMain.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(_iconId));
			wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcMain.hbrBackground = NULL;
			wcMain.lpszMenuName = NULL;
			wcMain.lpszClassName = "Corona2dBase";
			if (!RegisterClass(&wcMain)) {
				::MessageBoxA(NULL, "Could not start because the  class could not be registered", "Couldn't Start", MB_ICONERROR);
				return 0;
			}

			if (_fullScreen) {
				dwStyle = WS_POPUP | WS_MAXIMIZE;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
				ShowCursor(FALSE);
			}
			else {
				dwStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			}

			hwndRoot = NULL;

			setController(_firstController);

			hwndRoot = CreateWindowEx(dwExStyle,
				wcMain.lpszClassName, _title,
				dwStyle | WS_CLIPSIBLINGS ,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, NULL, hinstance, NULL);

			if (!hwndRoot) {
				MessageBox(NULL, "Could not start because of a problem creating the main window.", _title, MB_OK);
				return FALSE;
			}

			::ShowWindow(hwndRoot, SW_SHOWNORMAL);
			::UpdateWindow(hwndRoot);

			::QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
			::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);

			while (true) {
				if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
					if (!::GetMessage(&msg, NULL, 0, 0))
						break;
					if (!::IsDialogMessage(hwndRoot, &msg)) {
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
				else {
					__int64 counter;
					::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
					double elapsedSeconds = (double)(counter - lastCounter) / (double)performanceFrequency;
					double totalSeconds = (double)(counter - startCounter) / (double)performanceFrequency;
					lastCounter = counter;
					if (currentController->update(elapsedSeconds, totalSeconds)) {
						redraw();
					}
				}
			}

			return true;
		}

		bool directApplicationWin32::runDialog(HINSTANCE _hinstance, const char* _title, int _iconId, bool _fullScreen, controller* _firstController)
		{
			if (!_firstController)
				return false;

			::InitCommonControls();

			WNDCLASS wcMain;
			MSG msg;
			DWORD dwStyle, dwExStyle;

			hinstance = _hinstance;

			// register the class for the main application window

			wcMain.style = CS_HREDRAW | CS_VREDRAW;
			wcMain.lpfnWndProc = &directApplicationWin32::windowProc;
			wcMain.cbClsExtra = 0;
			wcMain.cbWndExtra = DLGWINDOWEXTRA;
			wcMain.hInstance = hinstance;
			wcMain.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(_iconId));
			wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcMain.hbrBackground =	NULL;
			wcMain.lpszMenuName = NULL;
			wcMain.lpszClassName = "Corona2dBase";
			if (!RegisterClass(&wcMain)) {
				::MessageBoxA(NULL, "Could not start because the main window class could not be registered", "Couldn't Start", MB_ICONERROR);
				return 0;
			}

			hwndRoot = NULL;

			if (_fullScreen) {
				dwStyle = WS_POPUP | WS_MAXIMIZE;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
				ShowCursor(FALSE);
			}
			else {
				dwStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
				dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			}

			setController(_firstController);

			hwndRoot = CreateWindowEx(dwExStyle,
				wcMain.lpszClassName, _title,
				dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, NULL, hinstance, NULL);

			if (!hwndRoot) {
				MessageBox(NULL, "Could not start because of a problem creating the main window.", _title, MB_OK);
				return FALSE;
			}

			::ShowWindow(hwndRoot, SW_SHOWNORMAL);
			::UpdateWindow(hwndRoot);

			::QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
			::QueryPerformanceCounter((LARGE_INTEGER*)&lastCounter);

			while (true) {
				if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
					if (!::GetMessage(&msg, NULL, 0, 0))
						break;
					if (!::IsDialogMessage(hwndRoot, &msg)) {
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
				else {
					__int64 counter;
					::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
					double elapsedSeconds = (double)(counter - lastCounter) / (double)performanceFrequency;
					double totalSeconds = (double)(counter - startCounter) / (double)performanceFrequency;
					lastCounter = counter;
					if (currentController->update(elapsedSeconds, totalSeconds)) {
						redraw();
					}
				}
			}

			return true;
		}

		void directApplicationWin32::setPictureIcon(int controlId, dtoIconId iconId)
		{
			SHSTOCKICONINFO iconInfo;
			ZeroMemory(&iconInfo, sizeof(iconInfo));
			iconInfo.cbSize = sizeof(iconInfo);
			HWND control = ::GetDlgItem(hwndRoot, controlId);
			HRESULT hresult = ::SHGetStockIconInfo((SHSTOCKICONID)iconId, SHGSI_ICON, &iconInfo);
			if (hresult == S_OK)
				::SendMessage(control, STM_SETICON, (WPARAM)iconInfo.hIcon, NULL);
		}

		void directApplicationWin32::setButtonIcon(int controlId, dtoIconId iconId)
		{
			SHSTOCKICONINFO iconInfo;
			ZeroMemory(&iconInfo, sizeof(iconInfo));
			iconInfo.cbSize = sizeof(iconInfo);

			HWND control = ::GetDlgItem(hwndRoot, controlId);
			HRESULT hresult = ::SHGetStockIconInfo((SHSTOCKICONID)iconId, SHGSI_ICON | SHGSI_SMALLICON, &iconInfo);
			if (hresult == S_OK)
				::SendMessage(control, BM_SETIMAGE, IMAGE_ICON, (WPARAM)iconInfo.hIcon);
		}

		void directApplicationWin32::setVisible(int controlId, bool visible)
		{
			HWND control = ::GetDlgItem(hwndRoot, controlId);
			::ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
		}

		void directApplicationWin32::setEnable(int controlId, bool enabled)
		{
			HWND control = ::GetDlgItem(hwndRoot, controlId);
			::EnableWindow(control, enabled);
		}

		void directApplicationWin32::setEditText(int textControlId, std::string& _string)
		{
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			::SetWindowTextA(control, _string.c_str());
		}

		void directApplicationWin32::setEditText(int textControlId, const char* _string)
		{
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			::SetWindowTextA(control, _string);
		}

		std::string directApplicationWin32::getEditText(int textControlId)
		{
			std::string value = "";
			HWND control = ::GetDlgItem(hwndRoot, textControlId);
			int length = ::GetWindowTextLengthA(control) + 1;
			char* buffer = new char[length];
			if (buffer) {
				::GetWindowTextA(control, buffer, length);
				value = buffer;
				delete[] buffer;
			}
			return value;
		}

		std::string directApplicationWin32::getComboSelectedText(int ddlControlId)
		{
			std::string value = "";
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
			if (newSelection > -1) {
				int length = (int)::SendMessage(control, CB_GETLBTEXTLEN, newSelection, 0);
				char* buffer = new char[length + 16];
				if (buffer) {
					::SendMessage(control, CB_GETLBTEXT, newSelection, (LPARAM)buffer);
					value = buffer;
					delete[] buffer;
				}
			}
			return value;
		}

		int directApplicationWin32::getComboSelectedIndex(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
			return newSelection;
		}

		int directApplicationWin32::getComboSelectedValue(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newSelection = (int)::SendMessage(control, CB_GETCURSEL, 0, 0);
			int data = (int)::SendMessage(control, CB_GETITEMDATA, newSelection, 0);
			return data;
		}

		void directApplicationWin32::setComboSelectedIndex(int ddlControlId, int index)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessageA(control, CB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setComboSelectedText(int ddlControlId, std::string& _text)
		{
			setComboSelectedText(ddlControlId, _text.c_str());
		}

		void directApplicationWin32::setComboSelectedText(int ddlControlId, const char* _text)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int index = ::SendMessageA(control, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)_text);
			::SendMessageA(control, CB_SETCURSEL, index, NULL);
		}

		void directApplicationWin32::setComboSelectedValue(int ddlControlId, int value)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int count = ::SendMessageA(control, CB_GETCOUNT, NULL, NULL);
			for (int i = 0; i < count; i++) {
				int data = (int)::SendMessageA(control, CB_GETITEMDATA, i, 0);
				if (data == value) {
					::SendMessageA(control, CB_SETCURSEL, i, NULL);
					break;
				}
			}
		}

		void directApplicationWin32::clearComboItems(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessageA(control, CB_RESETCONTENT, NULL, NULL);
		}

		void directApplicationWin32::addComboItem(int ddlControlId, std::string& _text, int _data)
		{
			addComboItem(ddlControlId, _text.c_str(), _data);
		}

		void directApplicationWin32::addComboItem(int ddlControlId, const char* _text, int _data)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int newItemIndex = (int)::SendMessageA(control, CB_ADDSTRING, NULL, (LPARAM)_text);
			if (newItemIndex != CB_ERR) {
				int err = ::SendMessageA(control, CB_SETITEMDATA, newItemIndex, (LPARAM)_data);
			}
		}

		void directApplicationWin32::setFocus(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetFocus(control);
		}

		void directApplicationWin32::addFoldersToCombo(int ddlControlId, const char* _path)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			char searchPath[MAX_PATH + 8];
			strncpy_s(searchPath, _path, MAX_PATH);
			searchPath[MAX_PATH] = 0;

			char* lastChar = getLastChar(searchPath);
			if (!lastChar)
				return;

			if (*lastChar == '\\') {
				lastChar++;
				*lastChar = '*';
				lastChar++;
				*lastChar = 0;
			}
			else if (*lastChar != '*') {
				lastChar++;
				*lastChar = '\\';
				lastChar++;
				*lastChar = '*';
				lastChar++;
				*lastChar = 0;
			}

			WIN32_FIND_DATA findData;
			ZeroMemory(&findData, sizeof(findData));

			HANDLE hfind = INVALID_HANDLE_VALUE;

			hfind = ::FindFirstFileA(searchPath, &findData);
			if (hfind != INVALID_HANDLE_VALUE) {
				do
				{
					// oh windows, . is really a directory, seriously!
					if (strcmp(findData.cFileName, ".") == 0)
						continue;

					if (strcmp(findData.cFileName, "..") == 0)
						continue;

					if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						char recurseBuff[MAX_PATH + 8];
						strncpy_s(recurseBuff, _path, MAX_PATH);
						recurseBuff[MAX_PATH] = 0;
						::PathAddBackslashA(recurseBuff);
						::PathAppendA(recurseBuff, findData.cFileName);
						addComboItem(ddlControlId, recurseBuff, 0);
						addFoldersToCombo(ddlControlId, recurseBuff);
					}
				} while (FindNextFile(hfind, &findData) != 0);
			}
		}

		void directApplicationWin32::addPicturesFoldersToCombo(int ddlControlId)
		{
			char picturesPath[MAX_PATH * 2];
			::SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, picturesPath);
			addFoldersToCombo(ddlControlId, picturesPath);
		}

		void directApplicationWin32::setButtonChecked(int controlId, bool enabled)
		{
			::CheckDlgButton(hwndRoot, controlId, enabled ? BST_CHECKED : BST_UNCHECKED);
		}

		bool directApplicationWin32::getButtonChecked(int controlId)
		{
			return ::IsDlgButtonChecked(hwndRoot, controlId) == BST_CHECKED;
		}

		void directApplicationWin32::clearListView(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			ListView_DeleteAllItems(control);
		}

		void directApplicationWin32::addListViewItem(int ddlControlId, std::string& _text, int _data)
		{
			addListViewItem(ddlControlId, _text.c_str(), _data);
		}

		void directApplicationWin32::addListViewItem(int ddlControlId, const char* _text, int _data)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
			lvitem.iItem = ListView_GetItemCount(control);
			lvitem.pszText = (LPSTR)_text;
			lvitem.lParam = _data;
			ListView_InsertItem(control, &lvitem);
		}

		int directApplicationWin32::getListViewSelectedIndex(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int iPos = ListView_GetNextItem(control, -1, LVNI_SELECTED);
			return iPos;
		}

		int directApplicationWin32::getListViewSelectedValue(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int pos = getListViewSelectedIndex(ddlControlId);
			if (pos > -1) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = pos;
				lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				ListView_GetItem(control, &lvitem);
				return lvitem.lParam;
			}
			return -1;
		}

		std::string directApplicationWin32::getListViewSelectedText(int ddlControlId)
		{
			char buffer[16384];
			std::string result;
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			int pos = getListViewSelectedIndex(ddlControlId);
			if (pos > -1) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = pos;
				lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				lvitem.pszText = buffer;
				lvitem.cchTextMax = sizeof(buffer) - 1;
				ListView_GetItem(control, &lvitem);
				result = lvitem.pszText;
			}
			return result;
		}

		std::list<std::string> directApplicationWin32::getListViewSelectedTexts(int ddlControlId)
		{
			char buffer[16384];
			std::list<std::string> ret;

			std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			for (auto ix = indeces.begin(); ix != indeces.end(); ix++) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = *ix;
				lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				lvitem.pszText = buffer;
				lvitem.cchTextMax = sizeof(buffer) - 1;
				ListView_GetItem(control, &lvitem);
				ret.push_back(lvitem.pszText);
			}

			return ret;
		}

		std::list<int> directApplicationWin32::getListViewSelectedIndexes(int ddlControlId)
		{
			std::list<int> ret;

			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			int iPos = ListView_GetNextItem(control, -1, LVNI_SELECTED);
			while (iPos > -1) {
				ret.push_back(iPos);
				iPos = ListView_GetNextItem(control, iPos, LVNI_SELECTED);
			}

			return ret;
		}

		std::list<int> directApplicationWin32::getListViewSelectedValues(int ddlControlId)
		{
			std::list<int> ret;

			std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			for (auto ix = indeces.begin(); ix != indeces.end(); ix++) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = *ix;
				lvitem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				ListView_GetItem(control, &lvitem);
				ret.push_back(lvitem.lParam);
			}

			return ret;
		}


		void directApplicationWin32::clearListViewSelection(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			// deselect the items
			std::list<int> indeces = getListViewSelectedIndexes(ddlControlId);
			for (auto idx = indeces.begin(); idx != indeces.end(); idx++) {
				int pos = *idx;
				if (pos > -1) {
					LVITEM lvitem;
					ZeroMemory(&lvitem, sizeof(lvitem));
					lvitem.iItem = pos;
					lvitem.mask = LVIF_STATE;
					lvitem.state = 0;
					lvitem.stateMask = LVIS_SELECTED;
					ListView_SetItem(control, &lvitem);
				}
			}
		}

		void directApplicationWin32::setListViewSelectedIndex(int ddlControlId, int indexId)
		{
			clearListViewSelection(ddlControlId);

			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(lvitem));
			lvitem.iItem = indexId;
			lvitem.mask = LVIF_STATE;
			lvitem.state = LVIS_SELECTED;
			lvitem.stateMask = LVIS_SELECTED;
			ListView_SetItem(control, &lvitem);
		}

		void directApplicationWin32::setListViewSelectedText(int ddlControlId, std::string& _text)
		{
			setListViewSelectedText(ddlControlId, _text.c_str());
		}

		void directApplicationWin32::setListViewSelectedText(int ddlControlId, const char* _text)
		{
			clearListViewSelection(ddlControlId);

			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			LVFINDINFO lvfi;

			ZeroMemory(&lvfi, sizeof(lvfi));

			lvfi.flags = LVFI_STRING;
			lvfi.psz = _text;

			int pos = ListView_FindItem(control, -1, &lvfi);

			if (pos > -1) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = pos;
				lvitem.mask = LVIF_STATE;
				lvitem.state = LVIS_SELECTED;
				lvitem.stateMask = LVIS_SELECTED;
				ListView_SetItem(control, &lvitem);
				ListView_EnsureVisible(control, pos, false);
			}

		}

		void directApplicationWin32::setListViewSelectedValue(int ddlControlId, int value)
		{
			clearListViewSelection(ddlControlId);

			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			LVFINDINFO lvfi;

			ZeroMemory(&lvfi, sizeof(lvfi));

			lvfi.flags = LVIF_PARAM;
			lvfi.lParam = value;

			int pos = ListView_FindItem(control, -1, &lvfi);

			if (pos > -1) {
				LVITEM lvitem;
				ZeroMemory(&lvitem, sizeof(lvitem));
				lvitem.iItem = pos;
				lvitem.mask = LVIF_STATE;
				lvitem.state = LVIS_SELECTED;
				lvitem.stateMask = LVIS_SELECTED;
				ListView_SetItem(control, &lvitem);
			}
		}

		void directApplicationWin32::setScrollHeight(int ddlControlId, int height)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetScrollRange(control, SB_VERT, 0, height, TRUE);
		}

		void directApplicationWin32::setScrollWidth(int ddlControlId, int width)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetScrollRange(control, SB_HORZ, 0, width, TRUE);
		}

		point directApplicationWin32::getScrollPos(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			point pt;
			pt.x = ::GetScrollPos(control, SB_HORZ);
			pt.y = ::GetScrollPos(control, SB_VERT);
			return pt;
		}

		point directApplicationWin32::getScrollTrackPos(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			point pt;

			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS | SIF_POS;
			::GetScrollInfo(control, SB_HORZ, &info);
			pt.x = info.nTrackPos;
			::GetScrollInfo(control, SB_VERT, &info);
			pt.y = info.nTrackPos;

			return pt;
		}

		void directApplicationWin32::setScrollPos(int ddlControlId, point pt)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			pt.x = ::SetScrollPos(control, SB_HORZ, pt.x, TRUE);
			pt.y = ::SetScrollPos(control, SB_VERT, pt.y, TRUE);
		}

		point directApplicationWin32::getScrollRange(int ddlControlId)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			point sz;
			int dummy, dx, dy;
			::GetScrollRange(control, SB_HORZ, &dummy, &dx);
			::GetScrollRange(control, SB_VERT, &dummy, &dy);
			sz.x = dx;
			sz.y = dy;
			return sz;
		}

		rectangle directApplicationWin32::getWindowPos(int ddlControlId)
		{
			HWND control = ddlControlId > 0 ? ::GetDlgItem(hwndRoot, ddlControlId) : hwndRoot;

			RECT r;
			rectangle rd;

			::GetWindowRect(control, &r);

			if (ddlControlId >= 0) 
			{
				::ScreenToClient(hwndRoot, (LPPOINT)&r.left);
				::ScreenToClient(hwndRoot, (LPPOINT)&r.right);
			}

			rd.x = r.left;
			rd.y = r.top;
			rd.w = r.right - r.left;
			rd.h = r.bottom - r.top;

			dpiScale = 96.0 / GetDpiForWindow(hwndRoot);

			rd.x *= dpiScale;
			rd.y *= dpiScale;
			rd.w *= dpiScale;
			rd.h *= dpiScale;

			return rd;
		}

		rectangle directApplicationWin32::getWindowClientPos()
		{
			RECT r;
			rectangle rd;

			::GetClientRect(hwndRoot, &r);

			rd.x = r.left;
			rd.y = r.top;
			rd.w = r.right - r.left;
			rd.h = r.bottom - r.top;

			dpiScale = 96.0 / GetDpiForWindow(hwndRoot);

			rd.x *= dpiScale;
			rd.y *= dpiScale;
			rd.w *= dpiScale;
			rd.h *= dpiScale;

			return rd;
		}


		void directApplicationWin32::setWindowPos(int ddlControlId, rectangle rect)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);

			::MoveWindow(control, rect.x, rect.y, rect.w, rect.h, true);
		}

		void directApplicationWin32::setMinimumWindowSize(point size)
		{
			this->minimumWindowSize = size;
		}

		void directApplicationWin32::setSpinRange(int ddlControlId, int lo, int high)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessage(control, UDM_SETRANGE32, lo, high);
		}

		void directApplicationWin32::setSpinPos(int ddlControlId, int pos)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SendMessage(control, UDM_SETPOS32, 0, pos);
		}

		void directApplicationWin32::setSysLinkText(int ddlControlId, const char* _text)
		{
			HWND control = ::GetDlgItem(hwndRoot, ddlControlId);
			::SetWindowTextA(control, _text);
		}

		class WinHttpSession {
		private:

			HINTERNET	hSession,
				hConnect,
				hRequest;
		public:

			WinHttpSession() : hSession(NULL), hConnect(NULL), hRequest(NULL)
			{
				// Use WinHttpOpen to obtain a session handle.
				hSession = ::WinHttpOpen(L"Corona Win64-Direct2d/1.0",
					WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					WINHTTP_NO_PROXY_NAME,
					WINHTTP_NO_PROXY_BYPASS, 0);

				throwOnNull(hSession, "Could not create WinHttpSession");
			}

			virtual std::string Get(const char* _domain, const char* _path)
			{
				wchar_t domain[2048], path[2048];

				std::string result = "";

				::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _domain, strlen(_domain) + 1, domain, sizeof(domain) / sizeof(wchar_t));

				if (_path)
					::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _path, strlen(_path) + 1, path, sizeof(path) / sizeof(wchar_t));
				else
					path[0] = 0;

				if (hSession) {
					hConnect = ::WinHttpConnect(hSession, domain, INTERNET_DEFAULT_HTTP_PORT, 0);
					throwOnNull(hConnect, "Could not connect to domain");

					hRequest = ::WinHttpOpenRequest(hConnect, L"GET", _path ? path : NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
					throwOnNull(hRequest, "Could not open http request");

					bool success = ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
					if (success) {
						DWORD dwSize = 0;
						DWORD dwDownloaded = 0;
						LPSTR pszOutBuffer;
						BOOL  bResults = FALSE;

						if (!WinHttpReceiveResponse(hRequest, NULL)) {
							throwOnNull(hRequest, "Could not open receive http response");
						}

						do
						{
							// Check for available data.
							dwSize = 0;
							if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
							{
								throwOnNull(NULL, "Could not query data available");
							}

							// Allocate space for the buffer.
							pszOutBuffer = new char[dwSize + 1];
							throwOnNull(pszOutBuffer, "Out of Memory");
							// Read the data.
							ZeroMemory(pszOutBuffer, dwSize + 1);

							if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
								delete[] pszOutBuffer;
								throwOnNull(NULL, "Could not read");
							}

							result += pszOutBuffer;

							delete[] pszOutBuffer;

						} while (dwSize > 0);
					}
				}

				return result;
			}

			virtual ~WinHttpSession()
			{
				// Close any open handles.
				if (hRequest) WinHttpCloseHandle(hRequest);
				if (hConnect) WinHttpCloseHandle(hConnect);
				if (hSession) WinHttpCloseHandle(hSession);
			}
		};

		std::vector<std::string> directApplicationWin32::readInternet(const char* _domain, const char* _path)
		{
			WinHttpSession session;

			std::string resultString = session.Get(_domain, _path);

			return split(resultString, '\n');
		}

		// utility
		char* directApplicationWin32::getLastChar(char* _str)
		{
			char* pc = NULL;
			while (*_str) {
				pc = _str;
				_str++;
			}
			return pc;
		}

		bool directApplicationWin32::getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension)
		{
			char szFileName[MAX_PATH + 1] = "";
			bool retval;

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndRoot;
			ofn.lpstrFilter = _pathExtensions;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = _defaultExtension;

			retval = GetSaveFileName(&ofn);
			if (retval)
				_saveFileName = szFileName;

			return retval;
		}

		void directApplicationWin32::setColorCapture(int _iconResourceId)
		{
			::SetCapture(hwndRoot);
			colorCapture = true;
			SetCursor(LoadCursor(hinstance, MAKEINTRESOURCE(_iconResourceId)));
		}

		// This was taken from a stack overflow article

		void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
		{
			// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
			// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
			// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
			// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
			// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
			// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
			// using the "_dup2" function.
			if (bindStdIn)
			{
				FILE* dummyFile;
				freopen_s(&dummyFile, "nul", "r", stdin);
			}
			if (bindStdOut)
			{
				FILE* dummyFile;
				freopen_s(&dummyFile, "nul", "w", stdout);
			}
			if (bindStdErr)
			{
				FILE* dummyFile;
				freopen_s(&dummyFile, "nul", "w", stderr);
			}

			// Redirect unbuffered stdin from the current standard input handle
			if (bindStdIn)
			{
				HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
				if (stdHandle != INVALID_HANDLE_VALUE)
				{
					int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
					if (fileDescriptor != -1)
					{
						FILE* file = _fdopen(fileDescriptor, "r");
						if (file != NULL)
						{
							int dup2Result = _dup2(_fileno(file), _fileno(stdin));
							if (dup2Result == 0)
							{
								setvbuf(stdin, NULL, _IONBF, 0);
							}
						}
					}
				}
			}

			// Redirect unbuffered stdout to the current standard output handle
			if (bindStdOut)
			{
				HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
				if (stdHandle != INVALID_HANDLE_VALUE)
				{
					int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
					if (fileDescriptor != -1)
					{
						FILE* file = _fdopen(fileDescriptor, "w");
						if (file != NULL)
						{
							int dup2Result = _dup2(_fileno(file), _fileno(stdout));
							if (dup2Result == 0)
							{
								setvbuf(stdout, NULL, _IONBF, 0);
							}
						}
					}
				}
			}

			// Redirect unbuffered stderr to the current standard error handle
			if (bindStdErr)
			{
				HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
				if (stdHandle != INVALID_HANDLE_VALUE)
				{
					int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
					if (fileDescriptor != -1)
					{
						FILE* file = _fdopen(fileDescriptor, "w");
						if (file != NULL)
						{
							int dup2Result = _dup2(_fileno(file), _fileno(stderr));
							if (dup2Result == 0)
							{
								setvbuf(stderr, NULL, _IONBF, 0);
							}
						}
					}
				}
			}

			// Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
			// standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
			// versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
			// has been read from or written to the targets or not.
			if (bindStdIn)
			{
				std::wcin.clear();
				std::cin.clear();
			}
			if (bindStdOut)
			{
				std::wcout.clear();
				std::cout.clear();
			}
			if (bindStdErr)
			{
				std::wcerr.clear();
				std::cerr.clear();
			}
		}

		void EnableGuiStdOuts()
		{
			AllocConsole();
			BindCrtHandlesToStdHandles(true, true, true);
		}

	}
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#endif
