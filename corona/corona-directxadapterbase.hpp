#ifndef CORONA_DIRECTXADAPTERBASE_H
#define CORONA_DIRECTXADAPTERBASE_H

#include "corona-windows-all.h"
#include "corona-constants.hpp"
#include "corona-directxdevices.hpp"
#include "corona-visual.hpp"
#include "corona-store_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-point_box.hpp"
#include "corona-datatransfer.hpp"

#include <string>
#include <exception>
#include <stdexcept>
#include <format>
#include <compare>
#include <map>
#include <vector>
#include <stack>
#include <functional>
#include <memory>
#include <algorithm>

namespace corona
{



	class directXAdapterBase : public std::enable_shared_from_this<directXAdapterBase>
	{
		IDXGIFactory2* dxFactory;
		IDXGIAdapter1* dxAdapter;

		std::shared_ptr<direct2dDevice> direct2d;
		std::shared_ptr<direct3dDevice> direct3d;

	public:

		directXAdapterBase()
		{
			dxFactory = nullptr;
			dxAdapter = nullptr;
		}

		virtual ~directXAdapterBase()
		{
			cleanup();
		}

		void cleanup()
		{
			if (dxAdapter)
				dxAdapter->Release();
			if (dxFactory)
				dxFactory->Release();
		}

		void refresh()
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

		IDXGIFactory2* getDxFactory() { return dxFactory; }
		IDXGIAdapter1* getDxAdapter() { return dxAdapter; }

		ID2D1Factory* getD2DFactory() { return direct2d->getD2DFactory(); }
		IWICImagingFactory* getWicFactory() { return direct2d->getWicFactory(); }
		IDWriteFactory* getDWriteFactory() { return direct2d->getDWriteFactory(); }
		ID2D1Device* getD2DDevice() { return direct2d->getD2DDevice(); }

		ID3D11Device* getD3DDevice() { return direct3d->getD3DDevice(); }
		D3D_FEATURE_LEVEL getFeatureLevel() { return direct3d->getFeatureLevel(); }



	};

}

#endif
