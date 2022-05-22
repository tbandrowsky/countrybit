
#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona 
{

	namespace win32 
	{

		pointDto::pointDto(float _x, float _y) : x(_x), y(_y)
		{
			;
		}

		pointDto::pointDto() : x(0), y(0)
		{
			;
		}

		marginDto::marginDto() : top(0), bottom(0), right(0), left(0)
		{
			;
		}

		marginDto::marginDto(float _top, float _bottom, float _left, float _right) : top(_top), bottom(_bottom), left(_left), right(_right)
		{
			;
		}

		rectDto::rectDto() : left(0), top(0), width(0), height(0)
		{
			;
		}

		rectDto::rectDto(float _left, float _top, float _width, float _height) : left(_left), top(_top), width(_width), height(_height)
		{
			;
		}

		sizeIntDto::sizeIntDto() : width(0), height(0)
		{
			;
		}

		sizeIntDto::sizeIntDto(int _width, int _height) : width(_width), height(_height)
		{
			;
		}

		sizeFloatDto::sizeFloatDto() : width(0.0f), height(0.0f)
		{
			;
		}

		sizeFloatDto::sizeFloatDto(float _width, float _height) : width(_width), height(_height)
		{
			;
		}

		pathLineDto::pathLineDto()
		{
			eType = e_line;
		}

		pathBaseDto* pathLineDto::clone()
		{
			pathLineDto* pld = new pathLineDto();
			*pld = *this;
			return pld;
		}

		pathArcDto::pathArcDto()
		{
			eType = e_arc;
		}

		pathBaseDto* pathArcDto::clone()
		{
			pathArcDto* pad = new pathArcDto();
			*pad = *this;
			return pad;
		}

		pathQuadraticBezierDto::pathQuadraticBezierDto()
		{
			eType = e_quadractic_bezier;
		}

		pathBaseDto* pathQuadraticBezierDto::clone()
		{
			pathQuadraticBezierDto* pad = new pathQuadraticBezierDto();
			*pad = *this;
			return pad;
		}

		pathBaseDto* pathBezierDto::clone()
		{
			pathBezierDto* pad = new pathBezierDto();
			*pad = *this;
			return pad;
		}

		pathBezierDto::pathBezierDto()
		{
			eType = e_bezier;
		}

		pathDto::pathDto()
		{
			;
		}

		pathDto::pathDto(const pathDto& _src)
		{
			name = _src.name;
			copyPoints(_src);
		}

		pathDto pathDto::operator =(const pathDto& _src)
		{
			name = _src.name;
			copyPoints(_src);
			return *this;
		}

		pathDto::~pathDto()
		{
			clear();
		}

		void pathDto::copyPoints(const pathDto& _src)
		{
			std::for_each(_src.points.begin(), _src.points.end(), [this](pathBaseDto* _item) { points.push_back(_item->clone()); });
		}

		void pathDto::clear()
		{
			std::for_each(points.begin(), points.end(), [this](pathBaseDto* _item) { delete _item; });
			points.clear();
		}

		textInstance2dDto::textInstance2dDto() : rotation(0.0)
		{
			;
		}

		dtoFactory::dtoFactory() : frame_elapsed(0)
		{
			;
		}

		std::string dtoFactory::createName(const char* _name, int _counter)
		{
			std::string name = _name;
			name = name + '.';
			char buff[256];
			sprintf_s(buff, "%d", _counter);
			name += buff;
			return name;
		}

		dtoFactory* dtoFactory::colorMake(float _red, float _green, float _blue, float _alpha)
		{
			color.alpha = _alpha;
			color.red = _red;
			color.green = _green;
			color.blue = _blue;
			return this;
		}

		dtoFactory* dtoFactory::gradientStopMake(float _red, float _green, float _blue, float _alpha, float _stop)
		{
			colorMake(_red, _green, _blue, _alpha);
			gradientStop.color = color;
			gradientStop.position = _stop;
			linearGradientBrush.gradientStops.push_back(gradientStop);
			radialGradientBrush.gradientStops.push_back(gradientStop);
			return this;
		}

		dtoFactory* dtoFactory::gradientStopMake(float _stop)
		{
			gradientStop.color = color;
			gradientStop.position = _stop;
			linearGradientBrush.gradientStops.push_back(gradientStop);
			radialGradientBrush.gradientStops.push_back(gradientStop);
			return this;
		}

		dtoFactory* dtoFactory::bitmapMake(drawableHost* _host, const char* _name, const char* _fileName, int _numSizes, sizeIntDto* _sizes)
		{
			bitmap.name = _name;
			bitmap.filename = _fileName;
			if (!_sizes) {
				bitmap.sizes.push_back(sizeIntDto(0, 0));
			}
			else {
				for (int i = 0; i < _numSizes; i++) {
					bitmap.sizes.push_back(_sizes[i]);
				}
			}
			_host->addBitmap(&bitmap);
			return this;
		}

		dtoFactory* dtoFactory::bitmapBrushMake(drawableHost* _host, const char* _name, const char* _bitmapName)
		{
			bitmapBrush.bitmapName = _bitmapName;
			bitmapBrush.name = _name;
			_host->addBitmapBrush(&bitmapBrush);
			return this;
		}

		dtoFactory* dtoFactory::solidBrushMake(drawableHost* _host, const char* _name, bool _stock)
		{
			solidBrush.color = color;
			solidBrush.name = _name;
			solidBrush.stock = _stock;
			_host->addSolidColorBrush(&solidBrush);
			return this;
		}

		dtoFactory* dtoFactory::point1Make(float _x, float _y)
		{
			point1.x = _x;
			point1.y = _y;
			return this;
		}

		dtoFactory* dtoFactory::point2Make(float _x, float _y)
		{
			point2.x = _x;
			point2.y = _y;
			return this;
		}

		dtoFactory* dtoFactory::point3Make(float _x, float _y)
		{
			point3.x = _x;
			point3.y = _y;
			return this;
		}

		dtoFactory* dtoFactory::pathArcMake(float _x, float _y, double _angle, float _radiusx, float _radiusy)
		{
			pathArcDto* pa = new pathArcDto();
			pa->point.x = _x;
			pa->point.y = _y;
			pa->angleDegrees = _angle;
			pa->radiusX = _radiusx;
			pa->radiusY = _radiusy;
			path.points.push_back(pa);
			return this;
		}

		dtoFactory* dtoFactory::pathBezierMake(float _x1, float _y1, float _x2, float _y2, float _x3, float _y3)
		{
			pathBezierDto* pb = new pathBezierDto();
			pb->point1.x = _x1;
			pb->point1.y = _y1;
			pb->point2.x = _x2;
			pb->point2.y = _y2;
			pb->point3.x = _x3;
			pb->point3.y = _y3;
			path.points.push_back(pb);
			return this;
		}

		dtoFactory* dtoFactory::pathQuadraticBezierMake(float _x1, float _y1, float _x2, float _y2)
		{
			pathBezierDto* pb = new pathBezierDto();
			pb->point1.x = _x1;
			pb->point1.y = _y1;
			pb->point2.x = _x2;
			pb->point2.y = _y2;
			path.points.push_back(pb);
			return this;
		}

		dtoFactory* dtoFactory::pathLineMake(float _x, float _y)
		{
			pathLineDto* pt = new pathLineDto();
			pt->point.x = _x;
			pt->point.y = _y;
			path.points.push_back(pt);
			return this;
		}

		dtoFactory* dtoFactory::pathLineMake(sPointDto* _points, int _count)
		{
			for (int i = 0; i < _count; i++) {
				pathLineMake(_points->x, _points->y);
				_points++;
			}
			return this;
		}

		dtoFactory* dtoFactory::pathLinePlotMake(double* _points, double _step, int _count)
		{
			double x = 0.0;
			for (int i = 0; i < _count; i++) {
				pathLineMake(x, *_points);
				_points++;
				x += _step;
			}
			return this;
		}

		dtoFactory* dtoFactory::pathSolidPlotMake(double* _points, double _step, int _count)
		{
			double x = 0.0;
			double maxy = _points[0];
			for (int i = 0; i < _count; i++) {
				double p = _points[i];
				pathLineMake(x, p);
				x += _step;
				if (p > maxy)
					maxy = p;
			}
			x -= _step;
			pathLineMake(x, maxy);
			pathLineMake(0.0, maxy);
			pathLineMake(0.0, _points[0]);
			return this;
		}

		dtoFactory* dtoFactory::linearGradientBrushMake(drawableHost* _host, const char* _name, bool _stock)
		{
			linearGradientBrush.name = _name;
			linearGradientBrush.start = point1;
			linearGradientBrush.stop = point2;
			linearGradientBrush.stock = _stock;
			_host->addLinearGradientBrush(&linearGradientBrush);
			linearGradientBrush.gradientStops.clear();
			radialGradientBrush.gradientStops.clear();
			return this;
		}

		dtoFactory* dtoFactory::linearGradientBrushMake(drawableHost* _host, pointDto point1, pointDto point2, const char* _name, bool _stock)
		{
			linearGradientBrush.name = _name;
			linearGradientBrush.start = point1;
			linearGradientBrush.stop = point2;
			linearGradientBrush.stock = _stock;
			_host->addLinearGradientBrush(&linearGradientBrush);
			linearGradientBrush.gradientStops.clear();
			radialGradientBrush.gradientStops.clear();
			return this;
		}

		dtoFactory* dtoFactory::radialGradientBrushMake(drawableHost* _host, const char* _name, bool _stock)
		{
			radialGradientBrush.center = point1;
			radialGradientBrush.name = _name;
			radialGradientBrush.offset = point2;
			radialGradientBrush.radiusX = point3.x;
			radialGradientBrush.radiusY = point3.y;
			radialGradientBrush.stock = _stock;
			_host->addRadialGradientBrush(&radialGradientBrush);
			linearGradientBrush.gradientStops.clear();
			radialGradientBrush.gradientStops.clear();
			return this;
		}

		dtoFactory* dtoFactory::radialGradientBrushMake(drawableHost* _host, pointDto center, pointDto offset, pointDto radius, const char* _name, bool _stock)
		{
			radialGradientBrush.center = center;
			radialGradientBrush.name = _name;
			radialGradientBrush.offset = offset;
			radialGradientBrush.radiusX = radius.x;
			radialGradientBrush.radiusY = radius.y;
			radialGradientBrush.stock = _stock;
			_host->addRadialGradientBrush(&radialGradientBrush);
			linearGradientBrush.gradientStops.clear();
			radialGradientBrush.gradientStops.clear();
			return this;
		}

		dtoFactory* dtoFactory::textStyleMake(drawableHost* _host, const char* _name, const char* _fontName, float _size, bool _bold, bool _italics)
		{
			textStyle.name = _name;
			textStyle.bold = _bold;
			textStyle.italics = _italics;
			if (_fontName)
				textStyle.fontName = _fontName;
			textStyle.fontSize = _size;
			textStyle.italics = _italics;
			_host->addTextStyle(&textStyle);
			return this;
		}

		dtoFactory* dtoFactory::pathMake(drawableHost* _host, const char* _name, bool _closed)
		{
			path.name = _name;
			_host->addPath(&path, _closed);
			path.clear();
			return this;
		}

		dtoFactory* dtoFactory::pathMake(pathImmediateDto* _dest, const char* _name, const char* _fillName, const char* _borderName, int _borderWidth, bool _closed)
		{
			_dest->path.clear();
			_dest->path.name = _name ? _name : "anonymous";
			_dest->path.copyPoints(path);
			path.clear();
			_dest->borderBrushName = _borderName ? _borderName : "unspecified";
			_dest->fillBrushName = _fillName ? _fillName : "unspecified";
			_dest->rotation = 0.0;
			_dest->strokeWidth = _borderWidth;
			_dest->position.x = 0;
			_dest->position.y = 0;
			_dest->closed = _closed;
			return this;
		}

		dtoFactory* dtoFactory::rectangleMake(pathImmediateDto* _dest, float _x1, float _y1, float _width, float _height, const char* _name, const char* _fillName, const char* _borderName, int _borderWidth)
		{
			pathLineMake(_x1, _y1);
			pathLineMake(_x1 + _width, _y1);
			pathLineMake(_x1 + _width, _y1 + _height);
			pathLineMake(_x1, _y1 + _height);
			pathLineMake(_x1, _y1);
			pathMake(_dest, _name, _fillName, _borderName, _borderWidth);
			return this;
		}

		dtoFactory* dtoFactory::rectangleMake(drawableHost* _host, float _x1, float _y1, float _width, float _height, const char* _name)
		{
			pathLineMake(_x1, _y1);
			pathLineMake(_x1 + _width, _y1);
			pathLineMake(_x1 + _width, _y1 + _height);
			pathLineMake(_x1, _y1 + _height);
			pathLineMake(_x1, _y1);
			pathMake(_host, _name);
			return this;
		}
	}
}

#endif
