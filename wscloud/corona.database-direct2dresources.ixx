

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

export module corona.database:direct2dresources;
import :constants;
import :directxdevices;
import :direct2dcontextbase;
import :visual;
import :store_box;
import :string_box;
import :rectangle_box;
import :point_box;
import :datatransfer;

export class deviceDependentAssetBase {
public:

	bool stock;

	virtual bool create(direct2dContextBase* target) = 0;
	virtual void release() = 0;

	virtual ID2D1Brush* getBrush();

	deviceDependentAssetBase() : stock(false)
	{
		;
	}

	~deviceDependentAssetBase()
	{
		;
	}

	ID2D1Brush* getBrush()
	{
		return NULL;
	}

};


export template <typename Direct2DAsset> class deviceDependentAsset : public deviceDependentAssetBase {
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



