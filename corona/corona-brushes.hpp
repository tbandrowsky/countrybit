#ifndef CORONA_BRUSHES_H
#define CORONA_BRUSHES_H

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

#include "corona-windows-all.h";
#include "corona-assert_if.hpp";
#include "corona-constants.hpp";
#include "corona-color_box.hpp";
#include "corona-point_box.hpp";
#include "corona-string_box.hpp";
#include "corona-rectangle_box.hpp";
#include "corona-visual.hpp";
#include "corona-point_box.hpp";
#include "corona-directxadapterbase.hpp";
#include "corona-directxdevices.hpp";
#include "corona-direct2dcontextbase.hpp";
#include "corona-bitmap_filters.hpp";
#include "corona-direct2dresources.hpp";
#include "corona-bitmaps.hpp";


namespace corona
{

	class brush
	{
	public:
		virtual ID2D1Brush* getBrush() = 0;
	};


	class bitmapBrush : public deviceDependentAsset<ID2D1BitmapBrush*>, brush {
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

	class solidColorBrush : public deviceDependentAsset<ID2D1SolidColorBrush*>, brush {
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

	class linearGradientBrush : public deviceDependentAsset<ID2D1LinearGradientBrush*>, brush {
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

	class radialGradientBrush : public deviceDependentAsset<ID2D1RadialGradientBrush*>, brush {
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
}

#endif
