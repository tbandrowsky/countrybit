#ifndef CORONA_DIRECT2DRESOURCES_H
#define CORONA_DIRECT2DRESOURCES_H


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
#include "corona-constants.hpp"
#include "corona-directxdevices.hpp"
#include "corona-direct2dcontextbase.hpp"
#include "corona-visual.hpp"
#include "corona-store_box.hpp"
#include "corona-string_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-point_box.hpp"
#include "corona-datatransfer.hpp"

namespace corona
{
	class deviceDependentAssetBase {
	public:

		bool stock;

		virtual bool create(direct2dContextBase* target) = 0;
		virtual void release() = 0;

		deviceDependentAssetBase() : stock(false)
		{
			;
		}

		virtual ~deviceDependentAssetBase()
		{
			;
		}

		virtual ID2D1Brush* getBrush()
		{
			return NULL;
		}

	};


	template <typename Direct2DAsset> class deviceDependentAsset : public deviceDependentAssetBase {
	protected:
		Direct2DAsset asset;

	public:

		deviceDependentAsset() : asset(NULL)
		{
			;
		}

		virtual ~deviceDependentAsset()
		{
			release();
		}

		deviceDependentAsset(const deviceDependentAsset& _src) : asset(_src.asset)
		{
			if (asset)
			{
				asset->AddRef();
			}
		}

		deviceDependentAsset operator=(const deviceDependentAsset& _src)
		{
			deviceDependentAsset copyasset(_src);
			return *this;
		}

		deviceDependentAsset(deviceDependentAsset&& _src) : asset(_src.asset)
		{
			_src.asset = nullptr;
		}

		deviceDependentAsset& operator=(deviceDependentAsset&& _src)
		{
			asset = _src.asset;
			_src.asset = nullptr;
			return *this;
		}

		virtual bool create(direct2dContextBase* target) = 0;

		bool recreate(direct2dContextBase* target)
		{
			release();
			create(target);
		}

		inline Direct2DAsset getAsset() { return asset; }

	protected:

		virtual void release()
		{
			if (asset) asset->Release();
			asset = NULL;
		}
	};

}

#endif

