

#include "corona.h"

namespace corona
{
	namespace win32
	{
		class nullFilterFunction {
		public:
			bool operator()(point, int, int, char*)
			{
				return true;
			}
		};

		//-------

		direct2dWindow::direct2dWindow(HWND _hwnd, std::weak_ptr<adapterSet> _adapterSet) : direct2dContext(_adapterSet)
		{

			HRESULT hr;

			hwnd = _hwnd;
			bitmap = nullptr;
			surface = nullptr;
			swapChain = nullptr;
			renderTarget = nullptr;

			D2D1_DEVICE_CONTEXT_OPTIONS options;

			options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;

			auto padapter = _adapterSet.lock();

			hr = padapter->getD2DDevice()->CreateDeviceContext(options, &renderTarget);
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

			hr = padapter->getDxFactory()->CreateSwapChainForHwnd(padapter->getD3DDevice(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
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

		std::weak_ptr<direct2dChildWindow> direct2dWindow::createChild(relative_ptr_type _id, UINT _x, UINT _y, UINT _w, UINT _h)
		{
			std::weak_ptr<direct2dChildWindow> child;
			if (children.contains(_id)) {
				child = children[_id];
				if (auto c = child.lock()) {
					c->moveWindow(_x, _y, _w, _h);
				}
			}
			else
			{
				auto pthis = std::enable_shared_from_this<direct2dWindow>::weak_from_this();
				auto new_ptr = std::make_shared<direct2dChildWindow>(pthis, factory, _x, _y, _w, _h);
				children.insert_or_assign(_id, new_ptr);
				child = new_ptr;
			}
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

		direct2dChildWindow::direct2dChildWindow(std::weak_ptr<direct2dWindow> _parent, std::weak_ptr<adapterSet> _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips) : direct2dContext(_adapterSet)
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

			childBitmap = nullptr;

			if (auto pparent = parent.lock()) {
				int dpiWindow;
				dpiWindow = ::GetDpiForWindow(pparent->getWindow());
				double dipsToPixels = dpiWindow / 96.0;

				D2D1_SIZE_F size;
				size.width = _wdips * dipsToPixels;
				size.height = _hdips * dipsToPixels;
				childBitmap = std::make_shared<direct2dBitmapCore>(size, factory, dpiWindow);
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

		direct2dChildWindow::~direct2dChildWindow()
		{
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

		direct2dContext::direct2dContext(std::weak_ptr<corona::win32::adapterSet> _factory) :
			factory(_factory)
		{
		}

		direct2dContext::~direct2dContext()
		{
			clearPaths();
			clearViewStyles();
			clearBitmapsAndBrushes(true);
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

				bmsize.height = _size.height;
				bmsize.width = _size.width;

				D2D1_BITMAP_PROPERTIES1 props = {};

				props.dpiX = dpi;
				props.dpiY = dpi;
				props.pixelFormat.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
				props.pixelFormat.alphaMode = D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_IGNORE;
				props.bitmapOptions = D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_TARGET;

				hr = targetContext->CreateBitmap(bmsize, nullptr, 0, props, &bitmap);
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
			}

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

		direct2dBitmap::direct2dBitmap(D2D1_SIZE_F _size, std::weak_ptr<adapterSet>& _factory) :
			direct2dContext(_factory)
		{
			HRESULT hr;
			D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();

			if (auto padapter = _factory.lock()) {

				hr = padapter->getWicFactory()->CreateBitmap(size.width, size.height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &wicBitmap);
				throwOnFail(hr, "Could not create WIC bitmap");

				hr = padapter->getD2DFactory()->CreateWicBitmapRenderTarget(wicBitmap, props, &target);
				throwOnFail(hr, "Could not create WIC render target");

				hr = target->QueryInterface(&targetContext);
				throwOnFail(hr, "Could not get WIC context");
			}
		}

		direct2dBitmap::~direct2dBitmap()
		{
			if (targetContext) targetContext->Release();
			if (target) target->Release();
			if (wicBitmap) wicBitmap->Release();
		}

		void directBitmapSaveImpl::save(const wchar_t* _filename)
		{

			HRESULT hr;

			if (auto padapter = dBitmap->getFactory().lock()) {

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

		bool textStyle::create(std::weak_ptr<direct2dContext>& target)
		{
			HRESULT hr = -1;

			if (auto ptarget = target.lock()) 
			{
				if (!ptarget->getRenderTarget())
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
					ptarget->getRenderTarget()->GetDpi(&dpiX, &dpiY);

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


		bool filteredBitmap::create(std::weak_ptr<direct2dContext>& _target, IWICBitmapSource* _source)
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

			if (auto ptarget = _target.lock()) {

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
			}

			return SUCCEEDED(hr);
		}

		bool filteredBitmap::make(std::weak_ptr<direct2dContext>& _target)
		{
			HRESULT hr = 0;

			if (filteredScaledBitmap) {
				filteredScaledBitmap->Release();
				filteredScaledBitmap = NULL;
			}

			if (wicFilteredScaledBitmap) {
				if (auto pTarget = _target.lock()) {
					hr = pTarget->getRenderTarget()->CreateBitmapFromWicBitmap(wicFilteredScaledBitmap, &filteredScaledBitmap);
				}
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

		void bitmap::copyFilteredBitmaps(std::weak_ptr<direct2dContext>& _targetContext, bitmap* _src)
		{
			for (auto i = _src->filteredBitmaps.begin(); i != _src->filteredBitmaps.end(); i++) {
				auto srcfiltered = *i;
				filteredBitmap* bm = srcfiltered->clone(_targetContext);
				filteredBitmaps.push_back(bm);
			}
		}

		bitmap::bitmap(std::weak_ptr<direct2dContext>& _targetContext, bitmap* _src)
			: useFile(_src->useFile),
			filename(_src->filename),
			filterFunction(_src->filterFunction)
		{
			copyFilteredBitmaps(_targetContext, _src);
			applyFilters(_targetContext);
		}

		bitmap::bitmap(std::string& _filename, std::list<sizeCrop>& _sizes) :
			useFile(true),
			filename(_filename)
		{
			nullFilterFunction defaultFunc;
			filterFunction = defaultFunc;
			setFilteredBitmaps(_sizes);
		}

		bitmap::~bitmap()
		{
			clearFilteredBitmaps();
		}

		bitmap* bitmap::clone(std::weak_ptr<direct2dContext>& _src)
		{
			return new bitmap(_src, this);
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
			auto iter = std::find_if(filteredBitmaps.begin(), filteredBitmaps.end(), [_width, _height](filteredBitmap* _bm) { return _bm->size.width == _width && _bm->size.height == _height; });
			if (iter != filteredBitmaps.end()) {
				return (*iter)->filteredScaledBitmap;
			}
			else {
				return NULL;
			}
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

		bool bitmap::applyFilters(std::weak_ptr<direct2dContext>& _target)
		{
			filter();

			for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
				filteredBitmap* bm = *ifb;
				bm->make(_target);
			}

			return true;
		}

		bool bitmap::create(std::weak_ptr<direct2dContext> & _target)
		{

			HRESULT hr;
			IWICBitmapDecoder* pDecoder = NULL;
			IWICBitmapFrameDecode* pSource = NULL;

			wchar_t fileBuff[1024];
			int ret = ::MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, fileBuff, sizeof(fileBuff) / sizeof(wchar_t) - 1);

			hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (auto pfactory = _target.lock()) {
				if (auto padapter = pfactory->getFactory().lock()) {
					hr = padapter->getWicFactory()->CreateDecoderFromFilename(fileBuff, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

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
				}
			}

			return SUCCEEDED(hr);
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

		bool bitmapBrush::create(std::weak_ptr<direct2dContext>& target)
		{
			HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);
			if (auto ptarget = target.lock()) {

				if (!ptarget || !ptarget->getRenderTarget())
				return false;

				HRESULT hr = ptarget->getRenderTarget()->CreateBitmapBrush(bm->getFirst(), &asset);
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

		bool solidColorBrush::create(std::weak_ptr<direct2dContext>& target)
		{
			HRESULT hr = -1;

			if (auto ptarget = target.lock()) 
			{
				if (!ptarget->getRenderTarget())
					return false;

				hr = ptarget->getRenderTarget()->CreateSolidColorBrush(color, &asset);
			}

			return SUCCEEDED(hr);
		}

		ID2D1Brush* solidColorBrush::getBrush()
		{
			return asset;
		}


		bool linearGradientBrush::create(std::weak_ptr<direct2dContext>& target)
		{
			ID2D1GradientStopCollection* pGradientStops = NULL;

			HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (auto ptarget = target.lock())
			{
				if (!ptarget->getRenderTarget())
					return false;

				hr = ptarget->getRenderTarget()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

				if (SUCCEEDED(hr))
				{
					hr = ptarget->getRenderTarget()->CreateLinearGradientBrush(
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


		bool radialGradientBrush::create(std::weak_ptr<direct2dContext>& target)
		{
			ID2D1GradientStopCollection* pGradientStops = NULL;

			HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (auto ptarget = target.lock())
			{
				if (!ptarget->getRenderTarget())
					return false;

				hr = ptarget->getRenderTarget()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

				if (SUCCEEDED(hr))
				{
					hr = ptarget->getRenderTarget()->CreateRadialGradientBrush(
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
				if (ptarget->getRenderTarget()) 
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

		point direct2dContext::getLayoutSize()
		{
			point fsize;
			fsize.x = size_dips.width;
			fsize.y = size_dips.height;
			return fsize;
		}

		point direct2dContext::getSize()
		{
			point asize;
			asize.x = size_pixels.width;
			asize.y = size_pixels.height;
			return asize;
		}

		void direct2dContext::clear(color* _color)
		{
			D2D1_COLOR_F color;

			color.a = _color->a;
			color.b = _color->b;
			color.g = _color->g;
			color.r = _color->r;

			this->getRenderTarget()->Clear(color);
		}

		void direct2dContext::addBitmap(bitmapRequest* _bitmap)
		{
			std::string filename, name;
			filename = _bitmap->file_name.c_str();
			name = _bitmap->name.c_str();
			std::list<sizeCrop> sizes;
			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				sizes.push_back(toSizeC(*it, _bitmap->cropEnabled, _bitmap->crop));
			}
			bitmap* bm = new bitmap(filename, sizes);
			bitmaps[name] = bm;
			auto wft = weak_from_this();
			bm->create(wft);

			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				int sx, sy;
				bm->getSize(&sx, &sy);
				it->x = sx;
				it->y = sy;
			}
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
					auto wft = weak_from_this();
					bm->create(wft);
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
				auto wft = weak_from_this();
				success = bm->applyFilters(wft);
			}
			return success;
		}

		void direct2dContext::addBitmapBrush(bitmapBrushRequest* _bitmapBrush)
		{
			bitmapBrush* brush = new bitmapBrush();
			std::string name, bitmapName;
			name = _bitmapBrush->name.c_str();
			bitmapName = _bitmapBrush->bitmapName;
			brush->bm = bitmaps[bitmapName];
			brushes[name] = brush;
			auto wft = weak_from_this();
			brush->create(wft);
		}

		void direct2dContext::addSolidColorBrush(solidBrushRequest* _solidBrushDto)
		{
			solidColorBrush* brush = new solidColorBrush();
			brush->stock = false;
			brush->color = toColor(_solidBrushDto->brushColor);
			brushes[_solidBrushDto->name.c_str()] = brush;
			auto wft = weak_from_this();
			brush->create(wft);
		}

		void direct2dContext::addLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto)
		{
			D2D1_GRADIENT_STOP gradientStop;
			linearGradientBrush* brush = new linearGradientBrush();
			brush->stock = _linearGradientBrushDto->stock;
			brush->start = toPoint(_linearGradientBrushDto->start);
			brush->stop = toPoint(_linearGradientBrushDto->stop);
			for (auto i : _linearGradientBrushDto->gradientStops) {
				gradientStop = toGradientStop(i.item);
				brush->stops.push_back(gradientStop);
			}
			brushes[_linearGradientBrushDto->name.c_str()] = brush;
			auto wft = weak_from_this();
			brush->create(wft);
		}

		void direct2dContext::addRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto)
		{
			D2D1_GRADIENT_STOP gradientStop;
			radialGradientBrush* brush = new radialGradientBrush();
			brush->stock = _radialGradientBrushDto->stock;
			brush->radialProperties.center = toPoint(_radialGradientBrushDto->center);
			brush->radialProperties.gradientOriginOffset = toPoint(_radialGradientBrushDto->offset);
			brush->radialProperties.radiusX = _radialGradientBrushDto->radiusX;
			brush->radialProperties.radiusY = _radialGradientBrushDto->radiusY;
			for (auto i : _radialGradientBrushDto->gradientStops) {
				gradientStop = toGradientStop(i.item);
				brush->stops.push_back(gradientStop);
			}
			brushes[_radialGradientBrushDto->name.c_str()] = brush;
			auto wft = weak_from_this();
			brush->create(wft);
		}

		void direct2dContext::clearBitmapsAndBrushes(bool deleteStock)
		{
			std::list<std::string> brushesToRemove, bitmapsToRemove;
			for (auto i = brushes.begin(); i != brushes.end(); i++)
			{
				if (i->second && (deleteStock || !i->second->stock)) {
					delete i->second;
					brushesToRemove.push_back(i->first);
				}
			};

			for (auto i = bitmaps.begin(); i != bitmaps.end(); i++)
			{
				if (i->second && (deleteStock || !i->second->stock)) {
					delete i->second;
					bitmapsToRemove.push_back(i->first);
				}
			};

			std::for_each(brushesToRemove.begin(), brushesToRemove.end(), [this](std::string ib) {
				brushes.erase(ib);
				});

			std::for_each(bitmapsToRemove.begin(), bitmapsToRemove.end(), [this](std::string ib) {
				bitmaps.erase(ib);
				});
		}

		path* direct2dContext::createPath(pathDto* _pathDto, bool _closed)
		{
			auto wft = weak_from_this();
			path* newPath = new path(wft);
			std::list<pathBaseDto*>::iterator i;

			// skip everything until we get to the starting point
			for (i = _pathDto->points.begin(); i != _pathDto->points.end(); i++) {
				pathBaseDto* t = *i;
				pathLineDto* l = t->asPathLineDto();
				if (l) {
					D2D1_POINT_2F point = toPoint(t->asPathLineDto()->point);
					newPath->start_figure(point);
					break;
				}
			}

			// now draw the rest of the path
			D2D1_POINT_2F point1, point2, point3;
			D2D1_SIZE_F size1;
			FLOAT float1;
			pathLineDto* pline;
			pathArcDto* parc;
			pathBezierDto* pbezier;
			pathQuadraticBezierDto* pquadraticbezier;

			while (i != _pathDto->points.end()) {
				pathBaseDto* t = *i;
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
				i++;
			}

			newPath->close_figure(_closed);
			return newPath;
		}

		void direct2dContext::addPath(pathDto* _pathDto, bool _closed)
		{
			path* newPath = createPath(_pathDto, _closed);
			paths[_pathDto->name.c_str()] = newPath;
		}

		void direct2dContext::clearPaths()
		{
			std::for_each(paths.begin(), paths.end(), [this](std::pair<std::string, path*> ib) {
				delete ib.second;
				});

			paths.clear();
		}

		void direct2dContext::addTextStyle(textStyleRequest* _textStyleDto)
		{
			textStyle* newStyle = new textStyle(
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
			auto wft = weak_from_this();
			newStyle->create(wft);
		}

		void direct2dContext::clearTextStyles()
		{
			std::for_each(textStyles.begin(), textStyles.end(), [this](std::pair<std::string, textStyle*> ib) {
				delete ib.second;
				});

			textStyles.clear();
		}

		void direct2dContext::addViewStyle(viewStyleRequest& _request)
		{
			viewStyles[_request.name.c_str()] = _request;
			addTextStyle(&_request.text_style);
			addSolidColorBrush(&_request.box_border_color);
			addSolidColorBrush(&_request.box_fill_color);
			addSolidColorBrush(&_request.shape_border_color);
			addSolidColorBrush(&_request.shape_fill_color);
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

			if ((!border && !fill) || !p)
				return;

			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathInstanceDto->rotation) * D2D1::Matrix3x2F::Translation(_pathInstanceDto->position.x, _pathInstanceDto->position.y);
			getRenderTarget()->SetTransform(product);

			if (fill) {
				getRenderTarget()->FillGeometry(p->geometry, fill->getBrush());
			}
			if (border && _pathInstanceDto->strokeWidth > 0.0) {
				getRenderTarget()->DrawGeometry(p->geometry, border->getBrush(), _pathInstanceDto->strokeWidth);
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

			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathImmediateDto->rotation) * D2D1::Matrix3x2F::Translation(_pathImmediateDto->position.x, _pathImmediateDto->position.y);
			getRenderTarget()->SetTransform(product);

			if (fill) {
				getRenderTarget()->FillGeometry(p->geometry, fill->getBrush());
			}
			if (border && _pathImmediateDto->strokeWidth > 0.0) {
				getRenderTarget()->DrawGeometry(p->geometry, border->getBrush(), _pathImmediateDto->strokeWidth);
			}

			delete p;
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
				getRenderTarget()->DrawLine(dstart, dstop, fill->getBrush(), thickness, nullptr);
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
					getRenderTarget()->FillRectangle(r, fill->getBrush());
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
					getRenderTarget()->DrawRectangle(&r, border->getBrush(), _borderWidth);
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
						getRenderTarget()->DrawTextLayout({ r.left, r.top }, textLayout, brush);
						textLayout->Release();
						textLayout = nullptr;
					}
					else
					{
						getRenderTarget()->DrawText(buff, ret, format, &r, brush);
					}
				}
			}
			else
			{
				getRenderTarget()->DrawText(buff, ret, format, &r, brush);
			}

			delete[] buff;
		}

		database::rectangle direct2dContext::getCanvasSize()
		{

			D2D1_SIZE_F size;

			if (getRenderTarget())
			{
				size = getRenderTarget()->GetSize();
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
			getRenderTarget()->SetTransform(product);

			D2D1_RECT_F rect = {};

			rect.left = _textInstanceDto->layout.x;
			rect.top = _textInstanceDto->layout.y;
			rect.right = _textInstanceDto->layout.x + _textInstanceDto->layout.w;
			rect.bottom = _textInstanceDto->layout.y + _textInstanceDto->layout.h;

			auto brush = fill->getBrush();
			int l = (_textInstanceDto->text.length() + 1) * 2;
			wchar_t* buff = new wchar_t[l];
			int ret = ::MultiByteToWideChar(CP_ACP, NULL, _textInstanceDto->text.c_str(), -1, buff, l - 1);
			getRenderTarget()->DrawText(buff, ret, style->getFormat(), &rect, brush);
			delete[] buff;
		}

		void direct2dContext::drawBitmap(bitmapInstanceDto* _bitmapInstanceDto)
		{
			auto bm = bitmaps[_bitmapInstanceDto->bitmapName];
			throwOnNull(bm, "Bitmap not found in context");
			auto ibm = bm->getBySize(_bitmapInstanceDto->width, _bitmapInstanceDto->height);
			throwOnNull(ibm, "Bitmap size not found in context");
			D2D1_RECT_F rect;
			rect.left = _bitmapInstanceDto->x;
			rect.top = _bitmapInstanceDto->y;
			rect.right = rect.left + _bitmapInstanceDto->width;
			rect.bottom = rect.top + _bitmapInstanceDto->height;
			getRenderTarget()->DrawBitmap(ibm, rect);
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
			getRenderTarget()->SetTransform(currentTransform);
		}

		void direct2dBitmap::beginDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;
			currentTransform = D2D1::Matrix3x2F::Identity();

			HRESULT hr = S_OK;

			if (getRenderTarget()) {
				getRenderTarget()->BeginDraw();
			}
		}

		void direct2dBitmap::endDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;

			HRESULT hr = S_OK;
			if (getRenderTarget()) {
				HRESULT hr = getRenderTarget()->EndDraw();
				if (hr == D2DERR_RECREATE_TARGET)
				{
					_adapter_blown_away = true;
				}
			}
		}


		drawableHost* direct2dContext::createBitmap(point& _size)
		{
			direct2dBitmap* bp = nullptr;
			auto rfact = getFactory();
			if (auto pfactory = rfact.lock()) {
				bp = new direct2dBitmap(toSizeF(_size), rfact );

				// now for the fun thing.  we need copy all of the objects over that we created from this context into the new one.  
				// i guess every architecture has its unforseen ugh moment, and this one is mine.

				std::for_each(bitmaps.begin(), bitmaps.end(), [this, bp](std::pair<std::string, deviceDependentAssetBase*> ib) {
					auto pcontext = weak_from_this();
					bp->bitmaps[ib.first] = ((bitmap*)ib.second)->clone(pcontext);
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
				HRESULT hr = this->getRenderTarget()->CreateBitmapFromWicBitmap(wicbitmap, &bitmap);
				throwOnFail(hr, "Could not create bitmap from wic bitmap");
				D2D1_RECT_F rect;
				rect.left = _dest.x;
				rect.top = _dest.y;
				rect.right = rect.left += _size.x > 0 ? _size.x : bp->size.width;
				rect.bottom = rect.top += _size.y > 0 ? _size.y : bp->size.height;
				getRenderTarget()->DrawBitmap(bitmap, &rect);
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

			auto& vs = viewStyles[style_composed_name.c_str()];
			auto& rectFill = vs.box_fill_color;
			drawRectangle(&_rect, vs.box_border_color.name, vs.box_border_thickness, vs.box_fill_color.name);

			_rect.h -= vs.box_border_thickness * 2.0;
			_rect.w -= vs.box_border_thickness * 2.0;
			_rect.x += vs.box_border_thickness;
			_rect.y += vs.box_border_thickness;

			drawText(_text, &_rect, vs.text_style.name, vs.shape_fill_color.name);

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

		void direct2dContext::loadStyleSheet(jobject& style_sheet, int _style_state)
		{
			;
		}

	}
}

