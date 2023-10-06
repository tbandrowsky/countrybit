
module;

#include "atlbase.h"

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

export module corona.database:directxdevices;
import "corona.database-windows-all.h";
import :constants;
import :visual;
import :store_box;
import :rectangle_box;
import :point_box;
import :datatransfer;

export class direct3dDevice : public std::enable_shared_from_this<direct3dDevice>
{
	ID3D11Device *d3d11Device;
	D3D_FEATURE_LEVEL	feature_level;

public:

	direct3dDevice()
	{
		d3d11Device = nullptr;
	}

	~direct3dDevice()
	{
	}

	bool setDevice(IDXGIAdapter1* _adapter)
	{
		if (d3d11Device)
			d3d11Device->Release();
		d3d11Device = nullptr;

		feature_level = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE;

		D3D_FEATURE_LEVEL feature_levels[] = {
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_11_1
		};

		ID3D11Device* temp = nullptr;

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

	direct2dDevice()
	{
		dxDevice = nullptr;
		d2dDevice = nullptr;

		d2DFactory = nullptr;
		wicFactory = nullptr;
		dWriteFactory = nullptr;

		D2D1_FACTORY_OPTIONS options;
		options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, options, &d2DFactory);
		throwOnFail(hr, "Could not create D2D1 factory");

		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
		throwOnFail(hr, "Could not create WIC Imaging factory");

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dWriteFactory), reinterpret_cast<IUnknown**>(&dWriteFactory));
		throwOnFail(hr, "Could not create direct write factory");
	}

	~direct2dDevice()
	{
		if (wicFactory)
			wicFactory->Release();
		if (dWriteFactory)
			dWriteFactory->Release();

		if (dxDevice)
			dxDevice->Release();
		if (d2dDevice)
			d2dDevice->Release();
		if (d2DFactory)
			d2DFactory->Release();

		dxDevice = nullptr;
		d2dDevice = nullptr;

		d2DFactory = nullptr;
		wicFactory = nullptr;
		dWriteFactory = nullptr;

	}

	bool setDevice(ID3D11Device* _d3dDevice)
	{
		HRESULT hr = _d3dDevice->QueryInterface(&this->dxDevice);

		hr = d2DFactory->CreateDevice(dxDevice, &d2dDevice);

		return SUCCEEDED(hr);
	}

	ID2D1Factory1* getD2DFactory() { return d2DFactory; }
	IWICImagingFactory* getWicFactory() { return wicFactory; }
	IDWriteFactory* getDWriteFactory() { return dWriteFactory; }
	ID2D1Device* getD2DDevice() { return d2dDevice; }

};



