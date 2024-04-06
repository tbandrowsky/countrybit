#ifndef CORONA_BITMAPS_H
#define CORONA_BITMAPS_H

namespace corona
{

	class direct2dBitmapCore
	{
	protected:
		ID2D1DeviceContext* targetContext;
		ID2D1RenderTarget* target;
		ID2D1Bitmap1* bitmap;

	public:

		D2D1_SIZE_F size;


		direct2dBitmapCore(D2D1_SIZE_F _size, std::weak_ptr<directXAdapterBase> _adapterSet, int dpi) :
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

		virtual ~direct2dBitmapCore()
		{
			if (target) target->Release();
			if (bitmap) bitmap->Release();
			if (targetContext) targetContext->Release();
		}

		ID2D1DeviceContext* beginDraw(bool& blownAdapter)
		{
			blownAdapter = false;
			targetContext->BeginDraw();
			return targetContext;
		}

		void endDraw(bool& blownAdapter)
		{
			blownAdapter = false;
			targetContext->EndDraw();
		}


		virtual bool isBitmap() { return true; }

		virtual ID2D1DeviceContext* getRenderTarget()
		{
			return targetContext;
		}

		ID2D1Bitmap1* getBitmap() {
			return bitmap;
		}

	};



	class direct2dBitmap : public std::enable_shared_from_this<direct2dBitmap>
	{
		ID2D1RenderTarget* target;
		IWICBitmap* wicBitmap;
		std::shared_ptr<direct2dContextBase> context;

	public:

		D2D1_SIZE_F size;


		direct2dBitmap(D2D1_SIZE_F _size, std::weak_ptr<directXAdapterBase> _factory)
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

				context = std::make_shared<direct2dContextBase>(_factory, targetContext);
			}
		}

		virtual ~direct2dBitmap()
		{
			if (target) target->Release();
			if (wicBitmap) wicBitmap->Release();
		}

		IWICBitmap* getBitmap()
		{
			return wicBitmap;
		}

		virtual bool isBitmap() { return true; }

		void save(const char* _filename)
		{
			IWICStream* fileStream = nullptr;
			IWICBitmapEncoder* bitmapEncoder = nullptr;
			IWICBitmapFrameEncode* bitmapFrameEncode = nullptr;

			HRESULT hr;
			wchar_t buff[8192];
			int ret = ::MultiByteToWideChar(CP_ACP, 0, _filename, -1, buff, sizeof(buff) - 1);

			if (auto padapter = getContext().getAdapter().lock()) {

				hr = padapter->getWicFactory()->CreateStream(&fileStream);
				throwOnFail(hr, "Could not create file stream");

				hr = padapter->getWicFactory()->CreateEncoder(GUID_ContainerFormatPng, NULL, &bitmapEncoder);
				throwOnFail(hr, "Could not create bitmap encoder");

				hr = fileStream->InitializeFromFilename(buff, GENERIC_WRITE);
				throwOnFail(hr, "Could not initialize file stream");

				hr = bitmapEncoder->Initialize(fileStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
				throwOnFail(hr, "Could not intialize bitmap encoder");

				hr = bitmapEncoder->CreateNewFrame(&bitmapFrameEncode, NULL);
				throwOnFail(hr, "Could not create frame");

				hr = bitmapFrameEncode->Initialize(NULL);
				throwOnFail(hr, "Could not initialize bitmap frame encoder");

				hr = bitmapFrameEncode->SetSize(size.width, size.height);
				throwOnFail(hr, "Could not initialize set size");

				WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

				WICRect rect;
				rect.X = 0;
				rect.Y = 0;
				rect.Width = size.width;
				rect.Height = size.height;

				hr = bitmapFrameEncode->WriteSource(getBitmap(), &rect);
				throwOnFail(hr, "Could not write source");

				hr = bitmapFrameEncode->Commit();
				throwOnFail(hr, "Could not commit frame");

				hr = bitmapEncoder->Commit();
				throwOnFail(hr, "Could not commit bitmap");
			}

			if (fileStream) fileStream->Release();
			if (bitmapEncoder) bitmapEncoder->Release();
			if (bitmapFrameEncode) bitmapFrameEncode->Release();

		}


		virtual direct2dContextBase& getContext()
		{
			return *context.get();
		}

		ID2D1DeviceContext* beginDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;

			return context->beginDraw(_adapter_blown_away);
		}

		void endDraw(bool& _adapter_blown_away)
		{
			_adapter_blown_away = false;

			context->endDraw(_adapter_blown_away);
		}

	};

	class filteredBitmap
	{
		filteredBitmap(direct2dContextBase* _targetContext, filteredBitmap* _src)
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

		filteredBitmap* clone(direct2dContextBase* _targetContext)
		{
			return new filteredBitmap(_targetContext, this);
		}

		bool create(direct2dContextBase* ptarget, HBITMAP _source)
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

			if (auto pfactory = ptarget->getAdapter().lock()) {
				hr = pfactory->getWicFactory()->CreateBitmapFromHBITMAP(_source, nullptr, WICBitmapAlphaChannelOption::WICBitmapIgnoreAlpha, &originalScaledBitmap);
				hr = pfactory->getWicFactory()->CreateBitmapFromHBITMAP(_source, nullptr, WICBitmapAlphaChannelOption::WICBitmapIgnoreAlpha, &wicFilteredScaledBitmap);
			}

			return SUCCEEDED(hr);
		}

		bool create(direct2dContextBase* ptarget, IWICBitmapSource* _source)
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


			if (auto pfactory = ptarget->getAdapter().lock()) {
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

		bool make(direct2dContextBase* ptarget)
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


	class bitmap : public deviceDependentAssetBase
	{
		bool useFile;
		bool useResource;
		std::string filename;
		int resource_id;

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

		void copyFilteredBitmaps(direct2dContextBase* _targetContext, bitmap* _src)
		{
			for (auto i = _src->filteredBitmaps.begin(); i != _src->filteredBitmaps.end(); i++) {
				auto srcfiltered = *i;
				filteredBitmap* bm = srcfiltered->clone(_targetContext);
				filteredBitmaps.push_back(bm);
			}
		}


		bool create_from_file(direct2dContextBase* pfactory)
		{
			HRESULT hr;
			IWICBitmapDecoder* pDecoder = NULL;
			IWICBitmapFrameDecode* pSource = NULL;

			wchar_t fileBuff[1024];
			int ret = ::MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, fileBuff, sizeof(fileBuff) / sizeof(wchar_t) - 1);

			hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (pfactory) {
				if (auto padapter = pfactory->getAdapter().lock()) {
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

		bool create_from_resource(direct2dContextBase* _target)
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

	public:

		bitmap(direct2dContextBase* _targetContext, bitmap* _src)
			: useFile(_src->useFile),
			useResource(_src->useResource),
			filename(_src->filename),
			filterFunction(_src->filterFunction)
		{
			copyFilteredBitmaps(_targetContext, _src);
			applyFilters(_targetContext);
		}

		bitmap(std::string& _filename, std::list<sizeCrop>& _sizes) :
			useFile(true),
			filename(_filename)
		{
			setFilteredBitmaps(_sizes);
		}

		bitmap(int _resource_id, std::list<sizeCrop>& _sizes) :
			useFile(false),
			useResource(true)
		{
			setFilteredBitmaps(_sizes);
		}

		virtual ~bitmap()
		{
			clearFilteredBitmaps();
		}

		std::shared_ptr<bitmap> clone(direct2dContextBase* _src)
		{
			return std::make_shared<bitmap>(_src, this);
		}


		virtual bool applyFilters(direct2dContextBase* _target)
		{
			filter();

			for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
				filteredBitmap* bm = *ifb;
				bm->make(_target);
			}

			return true;
		}

		bool create(direct2dContextBase* _target)
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

		void release()
		{
			for (auto ifb = filteredBitmaps.begin(); ifb != filteredBitmaps.end(); ifb++) {
				filteredBitmap* bm = *ifb;
				bm->release();
			}
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

		color getColorAtPoint(int _width, int _height, point point)
		{
			color c;
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
								c.a = pix.alpha / 255.0;
								c.b = pix.blue / 255.0;
								c.r = pix.red / 255.0;
								c.g = pix.green / 255.0;
								// because this is premultiplied alpha
								if (c.a >= 0.0) {
									c.b /= c.a;
									c.g /= c.a;
									c.r /= c.a;
								}
							}
						}

						// Release the bitmap lock.
						pLock->Release();
					}
				}
			}
			return c;
		}

		void setFilter(std::function<bool(point, int, int, char* bytes)> _filter)
		{
			filterFunction = _filter;
		}

		void filter()
		{
			HRESULT hr;

			if (!filterFunction)
			{
				filterFunction = [](point _size, int cbBufferSize, int cbStride, char* pv) {
					return true;
					};
			}

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
									if (filterFunction) {
										filterFunction(size, (int)cbBufferSizeDst, (int)cbStride, (char*)pvDst);
									}
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
	};
}

#endif
