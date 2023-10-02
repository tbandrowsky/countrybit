

module;

#include <compare>
#include <vector>
#include "omp.h"

export module corona.database:bitmap_filters;

import :point_box;

export struct PBGRAPixel
{
	unsigned char blue, green, red, alpha;
};

export class nullFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		return true;
	}
};

export class whiteFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		PBGRAPixel* base = (PBGRAPixel*)pv;

		for (int r = 0; r < _size.y; r++) {
			auto row = (PBGRAPixel*)(pv + cbStride * r);
			auto rowo = (PBGRAPixel*)(pv + cbStride * (int)(_size.y - (r + 1)));
			for (int x = 0; x < _size.x; x++) {
				auto pix = row[x];
				if (pix.alpha == 0) {
					pix.green = 255;
					pix.blue = 255;
					pix.red = 255;
				}
				row[x] = pix;
			}
		}

		return true;
	}
};

export class testFilterFunction {
public:
	bool operator()(point _size, int cbBufferSize, int cbStride, char* pv)
	{
		PBGRAPixel* base = (PBGRAPixel*)pv;

		for (int r = 0; r < _size.y; r++) {
			auto row = (PBGRAPixel*)(pv + cbStride * r);
			auto rowo = (PBGRAPixel*)(pv + cbStride * (int)(_size.y - (r + 1)));
			for (int x = 0; x < _size.x; x++) {
				auto pix = row[x];
				pix.alpha = 255;
				if (x > _size.x / 2)
					pix.blue = 255;
				else
					pix.green = 255;
				row[x] = pix;
			}
		}

		return true;
	}
};

export class bitmapFilter {
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

bitmapFilter::bitmapFilter(bool _flipHorizontalEnable,
	bool _flipVerticalEnable,
	float _brightness,
	float _contrast,
	bool _chromaKeyEnable,
	int _chromaKeyY,
	float _chromaKeyThreshold) :
	flipHorizontalEnable(_flipHorizontalEnable),
	flipVerticalEnable(_flipVerticalEnable),
	brightness(_brightness),
	contrast(_contrast),
	chromaKeyEnable(_chromaKeyEnable),
	chromaKeyY(_chromaKeyY),
	chromaKeyThreshold(_chromaKeyThreshold)
{
	;
}

bool bitmapFilter::operator()(point _size, int cbBufferSize, int cbStride, char* pv)
{
	if (flipHorizontalEnable) flipHorizontal(_size, cbBufferSize, cbStride, pv);
	if (flipVerticalEnable) flipVertical(_size, cbBufferSize, cbStride, pv);
	if (brightness != 0.0) adjustBrightness(_size, cbBufferSize, cbStride, pv);
	if (contrast != 0.0) adjustContrast(_size, cbBufferSize, cbStride, pv);
	if (chromaKeyEnable) adjustChromaKey(_size, cbBufferSize, cbStride, pv);
	return true;
}

bool bitmapFilter::flipHorizontal(point _size, int cbBufferSize, int cbStride, char* pv)
{
	PBGRAPixel* base = (PBGRAPixel*)pv;

	for (int r = 0; r < _size.y; r++) {
		auto row = (PBGRAPixel*)(pv + cbStride * r);
		for (int x = 0; x < _size.x / 2; x++) {
			int op = _size.x - (x + 1);
			auto pix = row[op];
			row[op] = row[x];
			row[x] = pix;
		}
	}
	return true;

}

bool bitmapFilter::flipVertical(point _size, int cbBufferSize, int cbStride, char* pv)
{
	PBGRAPixel* base = (PBGRAPixel*)pv;

	for (int r = 0; r < _size.y / 2; r++) {
		auto row = (PBGRAPixel*)(pv + cbStride * r);
		auto rowo = (PBGRAPixel*)(pv + cbStride * (int)(_size.y - (r + 1)));
		for (int x = 0; x < _size.x; x++) {
			auto pix = row[x];
			row[x] = rowo[x];
			rowo[x] = pix;
		}
	}

	return true;
}

// rgb to hsv conversion from 

typedef struct {
	float r;       // percent
	float g;       // percent
	float b;       // percent
} rgb;

typedef struct {
	float h;       // angle in degrees
	float s;       // percent
	float l;       // percent
} hsl;

template <class T> T mint(T a, T b, T c)
{
	T r;
	if (a < b) {
		r = (a < c) ? a : c;
	}
	else {
		r = (b < c) ? b : c;
	}
	return r;
}

template <class T> T maxt(T a, T b, T c)
{
	T r;
	if (a > b) {
		r = (a > c) ? a : c;
	}
	else {
		r = (b > c) ? b : c;
	}
	return r;
}

hsl rgb2hsl(rgb in)
{
	hsl out;

	float minc, maxc, delta;

	minc = mint(in.r, in.g, in.b);
	maxc = maxt(in.r, in.g, in.b);

	out.l = (maxc + minc) / 2.0;				// l

	delta = maxc - minc;
	if (delta == 0.0) {
		out.s = 0;
		out.h = 0;
		return out;
	}
	else {
		out.s = out.l > 0.5 ? delta / (2 - maxc - minc) : delta / (maxc + minc);
		if (maxc == in.r) {
			out.h = (in.g - in.b) / delta + (in.g < in.b ? 6 : 0);
		}
		else if (maxc == in.g) {
			out.h = (in.b - in.r) / delta + 2;
		}
		else if (maxc == in.b) {
			out.h = (in.r - in.g) / delta + 4;
		}
		out.h /= 6;
	}

	return out;
}

float hue2rgb(float p, float q, float t) {
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1.0 / 6) return p + (q - p) * 6 * t;
	if (t < 1.0 / 2) return q;
	if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
	return p;
}

rgb hsl2rgb(hsl in)
{
	rgb out;

	if (in.s == 0) {
		out.r = out.g = out.b = in.l;
		return out;
	}

	//var q = l < 0.5 ? l * (1 + s) : l + s - l * s
	float q = in.l < 0.5 ? in.l * (1.0 + in.s) : in.l + in.s - in.l * in.s;
	float p = 2.0 * in.l - q;
	out.r = hue2rgb(p, q, in.h + 1.0 / 3.0);
	out.g = hue2rgb(p, q, in.h);
	out.b = hue2rgb(p, q, in.h - 1.0 / 3.0);
	return out;
}

void adjustBrightnessInner(char* pv, int r, int cbStride, int width, float brightness)
{
	auto row = (PBGRAPixel*)(pv + cbStride * r);
	for (int x = 0; x < width; x++) {
		PBGRAPixel* pixel = row + x;
		rgb in, fin;
		in.r = pixel->red;
		in.g = pixel->green;
		in.b = pixel->blue;
		double a = pixel->alpha / 255.0;
		in.r = (in.r / 255.0) / a;
		in.g = (in.g / 255.0) / a;
		in.b = (in.b / 255.0) / a;
		hsl out;
		out = rgb2hsl(in);
		out.l += brightness;
		if (out.l > 1.0) out.l = 1.0;
		if (out.l < 0.0) out.l = 0.0;
		fin = hsl2rgb(out);
		pixel->red = fin.r * 255.0 * a;
		pixel->green = fin.g * 255.0 * a;
		pixel->blue = fin.b * 255.0 * a;
	}
}

bool bitmapFilter::adjustBrightness(point size, int cbBufferSize, int cbStride, char* pv)
{
	PBGRAPixel* base = (PBGRAPixel*)pv;
	int r, x;

	int szy = size.y;

#pragma omp parallel for
	for (r = 0; r < szy; r++) {
		adjustBrightnessInner(pv, cbStride, r, size.x, brightness);
	}

	return true;
}

void adjustContrastInner(char* pv, int cbStride, int r, int width, float center, float mRange)
{
	auto row = (PBGRAPixel*)(pv + cbStride * r);
	for (int x = 0; x < width; x++) {
		PBGRAPixel* pixel = row + x;
		rgb in, fin;
		in.r = pixel->red;
		in.g = pixel->green;
		in.b = pixel->blue;
		double a = pixel->alpha / 255.0;
		in.r = (in.r / 255.0) / a;
		in.g = (in.g / 255.0) / a;
		in.b = (in.b / 255.0) / a;
		hsl out;
		out = rgb2hsl(in);
		float f = (out.l - center) * mRange;
		out.l = f + center;
		if (out.l > 1.0) out.l = 1.0;
		if (out.l < 0.0) out.l = 0.0;
		fin = hsl2rgb(out);
		pixel->red = fin.r * 255.0 * a;
		pixel->green = fin.g * 255.0 * a;
		pixel->blue = fin.b * 255.0 * a;
	}
}

bool bitmapFilter::adjustContrast(point _size, int cbBufferSize, int cbStride, char* pv)
{
	PBGRAPixel* base = (PBGRAPixel*)pv;

	double minL, maxL;

	for (int r = 0; r < _size.y; r++) {
		auto row = (PBGRAPixel*)(pv + cbStride * r);
		for (int x = 0; x < _size.x; x++) {
			PBGRAPixel* pixel = row + x;
			rgb in, fin;
			in.r = pixel->red;
			in.g = pixel->green;
			in.b = pixel->blue;
			double a = pixel->alpha / 255.0;
			in.r = (in.r / 255.0) / a;
			in.g = (in.g / 255.0) / a;
			in.b = (in.b / 255.0) / a;
			hsl out;
			out = rgb2hsl(in);
			if (x == 0 && r == 0) {
				minL = out.l;
				maxL = out.l;
			}
			else {
				if (out.l < minL)
					minL = out.l;
				if (out.l > maxL)
					maxL = out.l;
			}
		}
	}

	double center = (maxL + minL) / 2.0;
	double mRange = (1.0 + this->contrast);

	// 10 - 30
	// center - 20
	// nv = 

	int szy = _size.y;

#pragma omp parallel for
	for (int r = 0; r < szy; r++) {
		adjustContrastInner(pv, cbStride, r, _size.x, center, mRange);
	}

	return true;
}

void adjustChromaKeyInner(char* pv, int cbStride, int r, int width, std::vector<hsl>& chromas, float chromaKeyThreshold)
{
	auto row = (PBGRAPixel*)(pv + cbStride * r);
	int x, x2;
	for (x = 0; x < width; x++) {
		PBGRAPixel* pixel = row + x;
		rgb in, fin;
		in.r = pixel->red;
		in.g = pixel->green;
		in.b = pixel->blue;
		double a = pixel->alpha / 255.0;
		in.r = (in.r / 255.0) / a;
		in.g = (in.g / 255.0) / a;
		in.b = (in.b / 255.0) / a;
		hsl out;
		out = rgb2hsl(in);

		for (x2 = 0; x2 < chromas.size(); x2++)
		{
			if (fabs(out.h - chromas[x2].h) < chromaKeyThreshold && fabs(out.s - chromas[x2].s) < chromaKeyThreshold) {
				// it's premultiplied alpha, so we blank everything
				pixel->red = 0.0;
				pixel->green = 0.0;
				pixel->blue = 0.0;
				pixel->alpha = 0.0;
				break;
			}
		}
	}
}

bool bitmapFilter::adjustChromaKey(point _size, int cbBufferSize, int cbStride, char* pv)
{
	PBGRAPixel* base = (PBGRAPixel*)pv,
		* row;
	int x, cy;
	rgb keyRgb;
	hsl keyHsl;

	cy = chromaKeyY;
	if (cy >= _size.y)
	{
		cy = _size.y - 1;
	}
	if (cy < 0)
	{
		cy = 0;
	}

	std::vector<hsl> chromas;

	auto chromaRow = (PBGRAPixel*)(pv + cbStride * cy);
	for (int x2 = 0; x2 < _size.x; x2++)
	{
		PBGRAPixel* pixelChroma = chromaRow + x2;

		keyRgb.r = pixelChroma->red;
		keyRgb.g = pixelChroma->green;
		keyRgb.b = pixelChroma->blue;

		double a = pixelChroma->alpha / 255.0;
		keyRgb.r = (keyRgb.r / 255.0) / a;
		keyRgb.g = (keyRgb.g / 255.0) / a;
		keyRgb.b = (keyRgb.b / 255.0) / a;

		keyHsl = rgb2hsl(keyRgb);
		chromas.push_back(keyHsl);
	}

	int szy = _size.y;
#pragma omp parallel for
	for (int r = 0; r < szy; r++) {
		adjustChromaKeyInner(pv, cbStride, r, _size.x, chromas, chromaKeyThreshold);
	}

	return true;
}

