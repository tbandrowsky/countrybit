#pragma once

namespace corona
{
	namespace win32
	{
		using namespace corona::database;

		void throwOnFail(HRESULT hr, const char* _message);
		void throwOnNull(void* _ptr, const char* _message);

		struct sizeCrop {
			D2D1_SIZE_U size;
			bool cropEnabled;
			D2D1_RECT_F crop;
		};

		sizeCrop toSizeC(point& _size, bool _cropEnabled, rectangle& _crop);
		D2D1_SIZE_U toSizeU(point& _size);
		D2D1_SIZE_F toSizeF(point& _size);
		point toSize(D2D1_SIZE_U& _size);
		D2D1_COLOR_F toColor(color& _color);
		D2D1_POINT_2F toPoint(point& _point);
		D2D1_GRADIENT_STOP toGradientStop(gradientStop& _gradientStop);

		class direct3dWindowInstance
		{
		public:

		};

		class direct3dDevice
		{
			ID3D11Device* d3d11Device;
			D3D_FEATURE_LEVEL	feature_level;

		public:

			direct3dDevice();
			~direct3dDevice();

			bool setDevice(IDXGIAdapter1* _adapter);
			inline ID3D11Device* getD3DDevice() { return d3d11Device; }
			inline D3D_FEATURE_LEVEL getFeatureLevel() { return feature_level; }
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

			direct2dDevice* direct2d;
			direct3dDevice* direct3d;

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
			direct2dChildWindow* findChild(relative_ptr_type _child);

			direct2dBitmap* createD2dBitmap(D2D1_SIZE_F size);

			void loadStyleSheet(jobject& sheet, int style_state);

		};

	}
}
