


#include "corona_platform.h"

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

export module corona.database:direct2dwindow;
import :constants;
import :directxdevices;
import :direct2dcontext;
import :visual;
import :store_box;
import :rectangle_box;
import :point_box;
import :datatransfer;
import :controller;

export class direct2dChildWindow
{

private:

	std::shared_ptr<direct2dBitmapCore> childBitmap;
	std::shared_ptr<direct2dContext> context;
	rectangle windowPosition;


public:

	direct2dChildWindow(std::weak_ptr<directXAdapterBase> _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips);
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

	direct2dWindow(HWND hwnd, std::weak_ptr<directXAdapterBase> _adapter, color _background_color);
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

export nullFilterFunction defaultfilter;
export whiteFilterFunction whitefilter;
export testFilterFunction testfilter;


direct2dWindow::direct2dWindow(HWND _hwnd, std::weak_ptr<directXAdapterBase> _adapterSet, color _background_color)
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

direct2dChildWindow::direct2dChildWindow(std::weak_ptr<directXAdapterBase> _adapterSet, UINT _xdips, UINT _ydips, UINT _wdips, UINT _hdips)
{
	HRESULT hr;

	context = std::make_shared<direct2dContext>(_adapterSet);
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

