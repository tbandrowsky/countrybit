/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/

#ifndef CORONA_DIRECTXADAPTERBASE_H
#define CORONA_DIRECTXADAPTERBASE_H

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
			dxAdapter = nullptr;
			if (dxFactory)
				dxFactory->Release();
			dxFactory = nullptr;
		}

		void refresh()
		{
			cleanup();
			direct3d = std::make_shared<direct3dDevice>();
			direct2d = std::make_shared<direct2dDevice>();

			HRESULT hr = CreateDXGIFactory1(IID_IDXGIFactory1, (void**)&dxFactory);
			throwOnFail(hr, "Could not create DXGI factory");

			for (UINT adapterIndex = 0; ; ++adapterIndex)
			{
				IDXGIAdapter1* currentAdapter = nullptr;

				if (DXGI_ERROR_NOT_FOUND == dxFactory->EnumAdapters1(adapterIndex, &currentAdapter))
				{
					// No more adapters to enumerate.
					std::cout << "adapter not found" << std::endl;
					break;
				}
				else 
				{
					DXGI_ADAPTER_DESC desc;
					istring<2048> temp;
					if (currentAdapter->GetDesc(&desc) == S_OK) {
						temp = desc.Description;
						std::cout << "Scanning " << temp << std::endl;
					}
				}

				if (direct3d->setDevice(currentAdapter))
				{
					std::cout << "Adapter Ok" << std::endl;
					dxAdapter = currentAdapter;
					break;
				}
				else
				{
					std::cout << "No adapters available, so, we are going nowhere." << std::endl;
					currentAdapter->Release();
				}
			}

			if (direct3d->getD3DDevice()) 
			{
				direct2d->setDevice(direct3d->getD3DDevice());
			}
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
