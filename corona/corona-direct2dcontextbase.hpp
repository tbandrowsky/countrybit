#ifndef CORONA_DIRECT2DCONTEXTBASE_H
#define CORONA_DIRECT2DCONTEXTBASE_H

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

#include "corona-windows-all.h"
#include "corona-assert_if.hpp"
#include "corona-constants.hpp"
#include "corona-color_box.hpp"
#include "corona-point_box.hpp"
#include "corona-string_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-visual.hpp"
#include "corona-point_box.hpp"
#include "corona-directxadapterbase.hpp"
#include "corona-directxdevices.hpp"
#include "corona-bitmap_filters.hpp"


namespace corona
{


	class direct2dContextBase
	{
	protected:

		std::weak_ptr<directXAdapterBase> adapter;
		ID2D1DeviceContext* context;

	public:

		direct2dContextBase(std::weak_ptr<directXAdapterBase> _factory) :
			adapter(_factory)
		{
			D2D1_DEVICE_CONTEXT_OPTIONS options;

			options = D2D1_DEVICE_CONTEXT_OPTIONS::D2D1_DEVICE_CONTEXT_OPTIONS_NONE;

			auto padapter = _factory.lock();
			HRESULT hr = padapter->getD2DDevice()->CreateDeviceContext(options, &context);
			throwOnFail(hr, "Could not create device context");
		}

		direct2dContextBase(std::weak_ptr<directXAdapterBase> _factory, ID2D1DeviceContext* _context) :
			adapter(_factory)
		{
			context = _context;
		}

		virtual ~direct2dContextBase()
		{
			if (context) {
				context->Release();
				context = nullptr;
			}
		}

		ID2D1DeviceContext* beginDraw(bool& _adapter_blown_away)
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

		std::weak_ptr<directXAdapterBase> getAdapter()
		{
			return adapter;
		}

		virtual ID2D1DeviceContext* getDeviceContext()
		{
			return context;
		}

	};

}

#endif
