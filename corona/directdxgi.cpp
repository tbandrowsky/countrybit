#pragma once

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

	}
}
