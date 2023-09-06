#pragma once

namespace corona 
{
	namespace win32 
	{
		class bitmapFilter {
			bool flipHorizontalEnable,
				flipVerticalEnable;
			float brightness;
			float contrast;
			bool chromaKeyEnable;
			int chromaKeyY;
			float chromaKeyThreshold;

			bool flipHorizontal(point _size, int cbBufferSize, int cbStride, char* pv);
			bool flipVertical(point _size, int cbBufferSize, int cbStride, char* pv);
			bool adjustBrightness(point _size, int cbBufferSize, int cbStride, char* pv);
			bool adjustContrast(point _size, int cbBufferSize, int cbStride, char* pv);
			bool adjustChromaKey(point _size, int cbBufferSize, int cbStride, char* pv);

		public:
			bitmapFilter(bool _flipHorizontalEnable,
				bool _flipVerticalEnable,
				float _brightness,
				float _contrast,
				bool _chromaKeyEnable,
				int _chromaKeyY,
				float _chromaKeyThreshold);
			bool operator()(point _size, int cbBufferSize, int cbStride, char* pv);
		};

	}

}

