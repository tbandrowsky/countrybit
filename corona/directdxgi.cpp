#pragma once

#include "corona.h"

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
			direct2dWindow* win = new direct2dWindow(parent, this);
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
			auto* win = getWindow(hwnd);
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



	}
}
