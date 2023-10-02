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

export module corona.database:brushes;

import "corona.database-windows-all.h";

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
import :direct2dcontextbase;
import :bitmap_filters;
import :direct2dresources;
import :bitmaps;


export class brush
{
public:
	virtual ID2D1Brush* getBrush() = 0;
};


export class bitmapBrush : public deviceDependentAsset<ID2D1BitmapBrush*>, brush {
public:

	std::weak_ptr<bitmap> bm;

	bitmapBrush() : deviceDependentAsset()
	{
		;
	}

	virtual ~bitmapBrush()
	{

	}

	bool create(direct2dContextBase* ptarget)
	{
		HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);
		if (ptarget) {

			if (!ptarget || !ptarget->getDeviceContext())
				return false;

			if (auto pbm = bm.lock()) {
				hr = ptarget->getDeviceContext()->CreateBitmapBrush(pbm->getFirst(), &asset);
			}
		}

		return SUCCEEDED(hr);
	}

	ID2D1Brush* getBrush()
	{
		return asset;
	}
};

export class solidColorBrush : public deviceDependentAsset<ID2D1SolidColorBrush*>, brush {
public:

	D2D1_COLOR_F color;

	solidColorBrush() : deviceDependentAsset()
	{
		;
	}

	virtual ~solidColorBrush()
	{

	}

	bool create(direct2dContextBase* ptarget)
	{
		HRESULT hr = -1;

		if (ptarget)
		{
			if (!ptarget->getDeviceContext())
				return false;

			hr = ptarget->getDeviceContext()->CreateSolidColorBrush(color, &asset);
		}

		return SUCCEEDED(hr);
	}

	virtual ID2D1Brush* getBrush()
	{
		return asset;
	}

};

export class linearGradientBrush : public deviceDependentAsset<ID2D1LinearGradientBrush*>, brush {
public:
	std::vector<D2D1_GRADIENT_STOP> stops;
	D2D1_POINT_2F					start,
		stop;

	bool create(direct2dContextBase* ptarget)
	{
		ID2D1GradientStopCollection* pGradientStops = NULL;

		HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

		if (ptarget)
		{
			if (!ptarget->getDeviceContext())
				return false;

			hr = ptarget->getDeviceContext()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

			if (SUCCEEDED(hr))
			{
				hr = ptarget->getDeviceContext()->CreateLinearGradientBrush(
					D2D1::LinearGradientBrushProperties(start, stop),
					D2D1::BrushProperties(),
					pGradientStops,
					&asset
				);
				pGradientStops->Release();
			}
		}

		return SUCCEEDED(hr);
	}

	virtual ID2D1Brush* getBrush()
	{
		return asset;
	}

};

export class radialGradientBrush : public deviceDependentAsset<ID2D1RadialGradientBrush*>, brush {
public:
	std::vector<D2D1_GRADIENT_STOP> stops;
	D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialProperties;

	bool radialGradientBrush::create(direct2dContextBase* ptarget)
	{
		ID2D1GradientStopCollection* pGradientStops = NULL;

		HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

		if (ptarget)
		{
			if (!ptarget->getDeviceContext())
				return false;

			hr = ptarget->getDeviceContext()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

			if (SUCCEEDED(hr))
			{
				hr = ptarget->getDeviceContext()->CreateRadialGradientBrush(
					radialProperties,
					D2D1::BrushProperties(),
					pGradientStops,
					&asset
				);
				pGradientStops->Release();
			}
		}
		return SUCCEEDED(hr);
	}

	virtual ID2D1Brush* getBrush()
	{
		return asset;
	}

};
