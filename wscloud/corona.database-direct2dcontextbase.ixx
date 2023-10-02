module;

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

export module corona.database:direct2dcontextbase;
import :assert_if;
import :constants;
import :color_box;
import :point_box;
import :string_box;
import :rectangle_box;
import :visual;
import :point_box;
import :directxadapterbase;
import :directxdevices;
import :direct2dresources;
import :bitmap_filters;

export class direct2dContextBase
{
protected:

	std::weak_ptr<directXAdapterBase> factory;
	CComPtr<ID2D1DeviceContext> context;

public:

	direct2dContextBase(std::weak_ptr<directXAdapterBase> _factory) :
		factory(_factory)
	{
		D2D1_DEVICE_CONTEXT_OPTIONS options;

		options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

		auto padapter = _factory.lock();
		ID2D1DeviceContext* temp = nullptr;

		HRESULT hr = padapter->getD2DDevice()->CreateDeviceContext(options, &temp);
		context.Attach(temp);
		throwOnFail(hr, "Could not create device context");
	}

	direct2dContextBase(std::weak_ptr<directXAdapterBase> _factory, ID2D1DeviceContext* _context) :
		factory(_factory)
	{
		context = _context;
	}

	~direct2dContextBase()
	{
	}

	CComPtr<ID2D1DeviceContext> beginDraw(bool& _adapter_blown_away)
	{
		_adapter_blown_away = false;

		HRESULT hr = S_OK;

		context->BeginDraw();
		return context;
	}

	HRESULT endDraw(bool& _adapter_blown_away)
	{
		_adapter_blown_away = false;
		HRESULT hr = context->EndDraw();
		return hr;
	}

	std::weak_ptr<directXAdapterBase> getFactory()
	{
		return factory;
	}

	virtual CComPtr<ID2D1DeviceContext>& getDeviceContext()
	{
		return context;
	}

	void clear(color* _color)
	{
		D2D1_COLOR_F color;

		color.a = _color->a;
		color.b = _color->b;
		color.g = _color->g;
		color.r = _color->r;

		this->getDeviceContext()->Clear(color);
	}

};


