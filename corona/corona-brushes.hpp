/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
Brushes in a corona device context.  Bitmap, solid and gradient brushes
are supported.

Notes

For Future Consideration
*/


#ifndef CORONA_BRUSHES_H
#define CORONA_BRUSHES_H


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

				if (!ptarget or !ptarget->getDeviceContext())
					return false;

				if (asset) {
					asset->Release();
					asset = nullptr;
				}

				if (auto pbm = bm.lock()) {
					hr = ptarget->getDeviceContext()->CreateBitmapBrush(pbm->getFirst(), &asset);
					if (!SUCCEEDED(hr)) {
						system_monitoring_interface::global_mon->log_warning("Could not create bitnap brush");
					}
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

				if (asset) {
					asset->Release();
					asset = nullptr;
				}

				hr = ptarget->getDeviceContext()->CreateSolidColorBrush(color, &asset);
			}

			if (!SUCCEEDED(hr)) {
				system_monitoring_interface::global_mon->log_warning("Could not create solid brush ");
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
		D2D1_POINT_2F					start;
		D2D1_POINT_2F					stop;

		bool create(direct2dContextBase* ptarget)
		{
			ID2D1GradientStopCollection* pGradientStops = NULL;

			HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT);

			if (ptarget)
			{
				if (!ptarget->getDeviceContext())
					return false;

				if (stops.size() > 0) {
					hr = ptarget->getDeviceContext()->CreateGradientStopCollection(&stops[0], stops.size(), &pGradientStops);

					if (SUCCEEDED(hr))
					{
						if (asset) {
							asset->Release();
							asset = nullptr;
						}
						hr = ptarget->getDeviceContext()->CreateLinearGradientBrush(
							D2D1::LinearGradientBrushProperties(start, stop),
							D2D1::BrushProperties(),
							pGradientStops,
							&asset
						);
						pGradientStops->Release();
						if (!SUCCEEDED(hr)) {
							system_monitoring_interface::global_mon->log_warning("Could not create linear gradient brush ");
						}
					}
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
					if (asset) {
						asset->Release();
						asset = nullptr;
					}
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
