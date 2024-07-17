/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is an asset base contract for resources in a direct2d context.
IT allows for resource recreation in the event of lost surfaces.

Notes

For Future Consideration
*/

#ifndef CORONA_DIRECT2DRESOURCES_H
#define CORONA_DIRECT2DRESOURCES_H

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

