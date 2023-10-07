
#ifndef CORONA_DIRECT2DCONTEXT_H
#define CORONA_DIRECT2DCONTEXT_H

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
#include "corona-constants.hpp"
#include "corona-color_box.hpp"
#include "corona-point_box.hpp"
#include "corona-string_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-visual.hpp"
#include "corona-point_box.hpp"
#include "corona-directxadapterbase.hpp"
#include "corona-directxdevices.hpp"
#include "corona-direct2dresources.hpp"
#include "corona-direct2dcontextbase.hpp"
#include "corona-bitmap_filters.hpp"
#include "corona-brushes.hpp"
#include "corona-bitmaps.hpp"
#include "corona-paths.hpp"
#include "corona-textstyles.hpp"


namespace corona
{
	class direct2dWindow;
	class direct2dChildWindow;
	class direct2dBitmap;

	class drawableHost {
	public:

		virtual void clear(color* _color) = 0;
		virtual std::string setBitmap(bitmapRequest* _bitmap) = 0;
		virtual bool setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize) = 0;
		virtual bool setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter) = 0;
		virtual std::string  setBitmapBrush(bitmapBrushRequest* _bitmapBrush) = 0;
		virtual std::string setSolidColorBrush(solidBrushRequest* _solidBrushDto) = 0;
		virtual std::string setLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto) = 0;
		virtual std::string setRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto) = 0;
		virtual void clearBitmapsAndBrushes(bool deleteStockObjects = false) = 0;

		virtual std::string  setPath(pathDto* _pathDto, bool _closed = true) = 0;

		virtual void setViewStyle(viewStyleRequest& _textStyle) = 0;
		virtual void clearViewStyles() = 0;
		virtual void setTextStyle(textStyleRequest* _textStyle) = 0;
		virtual void clearTextStyles() = 0;

		virtual void clearPaths() = 0;

		virtual void drawPath(pathInstance2dDto* _pathInstanceDto) = 0;
		virtual void drawPath(pathImmediateDto* _pathImmediateDto) = 0;
		virtual void drawText(drawTextRequest* _textInstanceDto) = 0;
		virtual void drawBitmap(bitmapInstanceDto* _bitmapInstanceDto) = 0;

		virtual void drawLine(point* start, point* stop, const char* _fillBrush, double thickness) = 0;
		virtual void drawRectangle(rectangle* _rectDto, const char* _borderBrush, double _borderWidth, const char* _fillBrush) = 0;
		virtual void drawText(const char* _text, rectangle* _rectDto, const char* _textStyle, const char* _fillBrush) = 0;
		virtual rectangle getCanvasSize() = 0;

		virtual void popCamera() = 0;
		virtual void pushCamera(point* _position, float _rotation, float _scale = 1.0) = 0;

		virtual std::shared_ptr<direct2dBitmap> createBitmap(point& _size) = 0;
		virtual void drawBitmap(drawableHost* _directBitmap, point& _dest, point& _size) = 0;
		virtual bool isBitmap() { return false; }
		virtual void save(const char* _filename) = 0;

		virtual void drawView(const char* _style, const char* _text, rectangle& _rect, int _state, const char* _debug_comment) = 0;
	};


	class direct2dContext : public direct2dContextBase, public drawableHost, public std::enable_shared_from_this<direct2dContext>
	{
	protected:

		std::map<std::string, std::shared_ptr<bitmap>> bitmaps;
		std::map<std::string, std::shared_ptr<deviceDependentAssetBase>> brushes;
		std::map<std::string, std::shared_ptr<path>> paths;
		std::map<std::string, std::shared_ptr<textStyle>> textStyles;
		std::map<std::string, std::shared_ptr<viewStyleRequest>> viewStyles;


		void view_style_name(const object_name& _style_sheet_name, object_name& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-view-" + std::to_string(_index);
		}

		void text_style_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-text-" + std::to_string(_index);
		}

		void box_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-box-border-" + std::to_string(_index);
		}

		void box_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-box-fill-" + std::to_string(_index);
		}

		void shape_fill_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-shape-fill-" + std::to_string(_index);
		}

		void shape_border_brush_name(const object_name& _style_sheet_name, object_name_composed& _object_style_name, int _index)
		{
			_object_style_name = _style_sheet_name + "-shape-border-" + std::to_string(_index);
		}


	public:

		direct2dContext(std::weak_ptr<directXAdapterBase> _factory) :
			direct2dContextBase(_factory)
		{
		}

		virtual ~direct2dContext()
		{
		}

		virtual void clear(color* _color)
		{
			D2D1_COLOR_F color;

			color.a = _color->a;
			color.b = _color->b;
			color.g = _color->g;
			color.r = _color->r;

			this->getDeviceContext()->Clear(color);
		}

		virtual std::string setBitmap(bitmapRequest* _bitmap)
		{
			std::string filename, name;

			filename = _bitmap->file_name.c_str();
			name = _bitmap->name.c_str();

			std::list<sizeCrop> sizes;
			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				sizes.push_back(toSizeC(*it, _bitmap->cropEnabled, _bitmap->crop));
			}
			std::shared_ptr<bitmap> bm;

			if (_bitmap->resource_id)
			{
				bm = std::make_shared<bitmap>(_bitmap->resource_id, sizes);
			}
			else
			{
				bm = std::make_shared<bitmap>(filename, sizes);
			}

			bitmaps[name] = bm;
			bm->create(this);

			for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
				int sx = 0, sy = 0;
				bm->getSize(&sx, &sy);
				it->x = sx;
				it->y = sy;
			}
			return name;
		}

		virtual bool getBitmapSize(bitmapRequest* _bitmap, point* _size)
		{
			bool success = false;
			auto i = bitmaps[_bitmap->name.c_str()];
			if (i) {
				int bsx, bsy;
				success = i->getSize(&bsx, &bsy);
				_size->x = bsx;
				_size->y = bsy;
			}
			return success;
		}

		virtual color getColorAtPoint(bitmapInstanceDto* _bitmap, point& _point)
		{
			color color;
			auto i = bitmaps[_bitmap->bitmapName];
			if (i) {
				color = i->getColorAtPoint(_bitmap->width, _bitmap->height, _point);
			}
			return color;
		}

		virtual bool setBitmapSizes(bitmapRequest* _bitmap, bool _forceResize)
		{
			bool success = false;
			auto bm = bitmaps[_bitmap->name.c_str()];
			if (bm) {

				// first, check to see if we really need to do this
				bool allGood = true;
				if (_forceResize)
					allGood = false;
				else
					for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
						int width = it->x;
						int height = it->y;
						allGood = bm->getSize(&width, &height);
						it->x = width;
						it->y = height;
						if (!allGood) break;
					}

				// then, if we do, go ahead and create the bitmaps
				if (!allGood) {
					std::list<sizeCrop> sizes;
					for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
						sizes.push_back(toSizeC(*it, _bitmap->cropEnabled, _bitmap->crop));
					}
					bm->setSizes(sizes);
					bm->create(this);
					for (auto it = _bitmap->sizes.begin(); it != _bitmap->sizes.end(); it++) {
						int width = it->x;
						int height = it->y;
						bm->getSize(&width, &height);
						it->x = width;
						it->y = height;
					}
				}

				success = true;
			}
			return success;
		}

		virtual bool setBitmapFilter(bitmapRequest* _bitmap, std::function<bool(point, int, int, char* bytes)> _filter)
		{
			bool success = false;
			auto bm = bitmaps[_bitmap->name.c_str()];
			if (bm) {
				bm->setFilter(_filter);
				success = bm->applyFilters(this);
			}
			return success;
		}

		virtual std::string setBitmapBrush(bitmapBrushRequest* _bitmapBrush)
		{
			auto brush = std::make_shared<bitmapBrush>();
			std::string name, bitmapName;
			name = _bitmapBrush->name.c_str();
			bitmapName = _bitmapBrush->bitmapName;
			brush->bm = bitmaps[bitmapName];
			brushes[name] = brush;
			brush->create(this);
			return name;
		}

		virtual std::string setSolidColorBrush(solidBrushRequest* _solidBrushDto)
		{
			auto brush = std::make_shared<solidColorBrush>();
			brush->stock = false;
			brush->color = toColor(_solidBrushDto->brushColor);
			brushes[_solidBrushDto->name.c_str()] = brush;
			brush->create(this);
			return _solidBrushDto->name.c_str();
		}

		virtual std::string setLinearGradientBrush(linearGradientBrushRequest* _linearGradientBrushDto)
		{
			D2D1_GRADIENT_STOP gradientStop;
			auto brush = std::make_shared<linearGradientBrush>();
			brush->stock = false;
			brush->start = toPoint(_linearGradientBrushDto->start);
			brush->stop = toPoint(_linearGradientBrushDto->stop);
			for (auto i : _linearGradientBrushDto->gradientStops) {
				gradientStop = toGradientStop(i);
				brush->stops.push_back(gradientStop);
			}
			brushes[_linearGradientBrushDto->name.c_str()] = brush;
			brush->create(this);
			return _linearGradientBrushDto->name.c_str();
		}

		virtual std::string setRadialGradientBrush(radialGradientBrushRequest* _radialGradientBrushDto)
		{
			D2D1_GRADIENT_STOP gradientStop;
			auto brush = std::make_shared<radialGradientBrush>();
			brush->stock = false;
			brush->radialProperties.center = toPoint(_radialGradientBrushDto->center);
			brush->radialProperties.gradientOriginOffset = toPoint(_radialGradientBrushDto->offset);
			brush->radialProperties.radiusX = _radialGradientBrushDto->radiusX;
			brush->radialProperties.radiusY = _radialGradientBrushDto->radiusY;
			for (auto i : _radialGradientBrushDto->gradientStops) {
				gradientStop = toGradientStop(i);
				brush->stops.push_back(gradientStop);
			}
			brushes[_radialGradientBrushDto->name.c_str()] = brush;
			brush->create(this);
			return _radialGradientBrushDto->name.c_str();
		}

		virtual void clearBitmapsAndBrushes(bool deleteStock)
		{
			brushes.clear();
			bitmaps.clear();
		}

		virtual std::shared_ptr<path> createPath(pathDto* _pathDto, bool _closed)
		{
			std::shared_ptr<path> newPath = std::make_shared<path>(this);

			D2D1_POINT_2F point1, point2, point3;
			D2D1_SIZE_F size1;
			FLOAT float1;
			pathLineDto* pline;
			pathArcDto* parc;
			pathBezierDto* pbezier;
			pathQuadraticBezierDto* pquadraticbezier;

			bool findingMoveTo = true;

			for (auto i : _pathDto->points) {
				pathBaseDto* t = i.get();
				if (findingMoveTo) {
					pathLineDto* l = t->asPathLineDto();
					if (l) {
						D2D1_POINT_2F point = toPoint(t->asPathLineDto()->point);
						newPath->start_figure(point);
						findingMoveTo = false;
					}
				}
				else
				{
					switch (t->eType) {
					case e_line:
						pline = t->asPathLineDto();
						point1 = toPoint(pline->point);
						newPath->add_line(point1);
						break;
					case e_arc:
						parc = t->asPathArcDto();
						point1 = toPoint(parc->point);
						size1.height = parc->radiusX;
						size1.width = parc->radiusY;
						newPath->add_arc(point1, size1, parc->angleDegrees);
						break;
					case e_bezier:
						pbezier = t->asPathBezierDto();
						point1 = toPoint(pbezier->point1);
						point2 = toPoint(pbezier->point2);
						point3 = toPoint(pbezier->point3);
						newPath->add_bezier(point1, point2, point3);
						break;
					case e_quadractic_bezier:
						pquadraticbezier = t->asPathQuadraticBezierDto();
						point1 = toPoint(pquadraticbezier->point1);
						point2 = toPoint(pquadraticbezier->point2);
						newPath->add_quadratic_bezier(point1, point2);
						break;
					}
				}
			}

			// now draw the rest of the path

			newPath->close_figure(_closed);
			return newPath;
		}

		virtual std::string setPath(pathDto* _pathDto, bool _closed)
		{
			auto newPath = createPath(_pathDto, _closed);
			paths[_pathDto->name.c_str()] = newPath;
			return _pathDto->name.c_str();
		}

		virtual void clearPaths()
		{
			paths.clear();
		}

		virtual void setTextStyle(textStyleRequest* _textStyleDto)
		{
			auto newStyle = std::make_shared<textStyle>(
				_textStyleDto->fontName.c_str(),
				_textStyleDto->fontSize,
				_textStyleDto->bold,
				_textStyleDto->italics,
				_textStyleDto->underline,
				_textStyleDto->strike_through,
				_textStyleDto->line_spacing,
				_textStyleDto->horizontal_align,
				_textStyleDto->vertical_align,
				_textStyleDto->wrap_text
			);
			textStyles[_textStyleDto->name.c_str()] = newStyle;
			newStyle->create(this);
		}

		virtual void clearTextStyles()
		{
			textStyles.clear();
		}

		virtual void setViewStyle(viewStyleRequest& _request)
		{
			auto vs = std::make_shared<viewStyleRequest>();
			*(vs.get()) = _request;
			viewStyles[_request.name.c_str()] = vs;
			setTextStyle(&_request.text_style);
			setSolidColorBrush(&_request.box_border_color);
			setSolidColorBrush(&_request.box_fill_color);
			setSolidColorBrush(&_request.shape_border_color);
			setSolidColorBrush(&_request.shape_fill_color);
		}

		virtual void clearViewStyles()
		{
			viewStyles.clear();
		}

		virtual void drawPath(pathInstance2dDto* _pathInstanceDto)
		{
			auto fill = brushes[_pathInstanceDto->fillBrushName];
			auto border = brushes[_pathInstanceDto->borderBrushName];
			auto p = paths[_pathInstanceDto->pathName];

			/*
			if (!fill) {
		#if TRACE_GUI
				std::cout << "missing fill " << _pathInstanceDto->fillBrushName << std::endl;
		#endif
				return;
			}

			if (!border) {
		#if TRACE_GUI
				std::cout << "missing border " << _pathInstanceDto->borderBrushName << std::endl;
		#endif
				return;
			}
			*/

			if (!p)
				return;

			//			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathInstanceDto->rotation) * D2D1::Matrix3x2F::Translation(_pathInstanceDto->position.x, _pathInstanceDto->position.y);
			//			getDeviceContext()->SetTransform(product);

			if (fill) {
				getDeviceContext()->FillGeometry(p->geometry, fill->getBrush());
			}
			if (border && _pathInstanceDto->strokeWidth > 0.0) {
				getDeviceContext()->DrawGeometry(p->geometry, border->getBrush(), _pathInstanceDto->strokeWidth);
			}
		}

		virtual void drawPath(pathImmediateDto* _pathImmediateDto)
		{
			auto fill = brushes[_pathImmediateDto->fillBrushName];
			auto border = brushes[_pathImmediateDto->borderBrushName];

			if (!fill) {
#if TRACE_GUI
				std::cout << "missing fill " << _pathImmediateDto->fillBrushName << std::endl;
#endif
				return;
			}

			if (!border) {
#if TRACE_GUI
				std::cout << "missing border " << _pathImmediateDto->borderBrushName << std::endl;
#endif
				return;
			}

			auto p = createPath(&_pathImmediateDto->path, _pathImmediateDto->closed);
			if (!p)
				return;

			//			D2D1::Matrix3x2F product = currentTransform * D2D1::Matrix3x2F::Rotation(_pathImmediateDto->rotation) * D2D1::Matrix3x2F::Translation(_pathImmediateDto->position.x, _pathImmediateDto->position.y);
			//		getDeviceContext()->SetTransform(product);

			if (fill && _pathImmediateDto->fillBrushName.size() > 0) {
				getDeviceContext()->FillGeometry(p->geometry, fill->getBrush());
			}
			if (border && _pathImmediateDto->borderBrushName.size() > 0 && _pathImmediateDto->strokeWidth > 0.0) {
				getDeviceContext()->DrawGeometry(p->geometry, border->getBrush(), _pathImmediateDto->strokeWidth);
			}
	}

		virtual void drawLine(point* start, point* stop, const char* _fillBrush, double thickness)
		{
			auto fill = brushes[_fillBrush];

			D2D1_POINT_2F dstart, dstop;

			dstart.x = start->x;
			dstart.y = start->y;
			dstop.x = stop->x;
			dstop.y = stop->y;

			if (fill) {
				getDeviceContext()->DrawLine(dstart, dstop, fill->getBrush(), thickness, nullptr);
			}
		}

		virtual void drawRectangle(rectangle* _rectangle, const char* _borderBrush, double _borderWidth, const char* _fillBrush)
		{
			D2D1_RECT_F r;
			r.left = _rectangle->x;
			r.top = _rectangle->y;
			r.right = _rectangle->x + _rectangle->w;
			r.bottom = _rectangle->y + _rectangle->h;

			if (_fillBrush)
			{
				auto fill = brushes[_fillBrush];
				if (!fill) {
#if TRACE_GUI
					std::cout << "missing fill " << _fillBrush << std::endl;
#endif
				}
				else
					getDeviceContext()->FillRectangle(r, fill->getBrush());
			}

			if (_borderBrush)
			{
				auto border = brushes[_borderBrush];
				if (!border) {
#if TRACE_GUI
					std::cout << "missing border " << _borderBrush << std::endl;
#endif
				}
				else
					getDeviceContext()->DrawRectangle(&r, border->getBrush(), _borderWidth);
			}
		}

		virtual void drawText(const char* _text, rectangle* _rectangle, const char* _textStyle, const char* _fillBrush)
		{
			auto style = _textStyle ? textStyles[_textStyle] : nullptr;
			auto fill = _fillBrush ? brushes[_fillBrush] : nullptr;

			if (!style) {
#if TRACE_GUI
				std::cout << "missing textStyle " << _textStyle << std::endl;

#if TRACE_STYLES
				std::cout << "styles loaded" << std::endl;
				for (auto vs : viewStyles) {
					std::cout << vs.first << std::endl;
				}
#endif
#endif

				return;
}

			if (!fill) {
#if TRACE_GUI
				std::cout << "missing fillBrush " << _fillBrush << std::endl;
#endif
				return;
			}

			auto format = style->getFormat();
			if (!format) {
#if TRACE_GUI
				std::cout << "missing format " << _textStyle << std::endl;
#endif

				return;
			}

			D2D1_RECT_F r;
			r.left = _rectangle->x;
			r.top = _rectangle->y;
			r.right = _rectangle->x + _rectangle->w;
			r.bottom = _rectangle->y + _rectangle->h;

			auto brush = fill->getBrush();
			int len = (strlen(_text) + 1) * 2;
			wchar_t* buff = new wchar_t[len];
			int ret = ::MultiByteToWideChar(CP_ACP, NULL, _text, -1, buff, len - 1);

			if (style->get_strike_through() || style->get_underline())
			{
				int l = wcslen(buff);
				IDWriteTextLayout* textLayout = nullptr;

				if (auto pfactory = getAdapter().lock()) {

					pfactory->getDWriteFactory()->CreateTextLayout(buff, l, format, r.right - r.left, r.bottom - r.top, &textLayout);
					if (textLayout != nullptr) {
						textLayout->SetUnderline(style->get_underline(), { (UINT32)0, (UINT32)l });
						textLayout->SetStrikethrough(style->get_strike_through(), { (UINT32)0, (UINT32)l });
						getDeviceContext()->DrawTextLayout({ r.left, r.top }, textLayout, brush);
						textLayout->Release();
						textLayout = nullptr;
					}
					else
					{
						getDeviceContext()->DrawText(buff, ret, format, &r, brush);
					}
				}
			}
			else
			{
				getDeviceContext()->DrawText(buff, ret, format, &r, brush);
			}

			// uncomment this to show the text borders 
			//getDeviceContext()->DrawRectangle(&r, brush);

			delete[] buff;
			}

		virtual rectangle getCanvasSize()
		{

			D2D1_SIZE_F size;

			if (getDeviceContext())
			{
				size = getDeviceContext()->GetSize();
			}
			else
			{
				size.width = 0;
				size.height = 0;
			}

			return rectangle{ 0, 0, size.width, size.height };
		}

		virtual void drawText(drawTextRequest* _textInstanceDto)
		{
			auto style = textStyles[_textInstanceDto->styleName];
			auto fill = brushes[_textInstanceDto->fillBrushName];

			if (!style) {
#if TRACE_GUI				
				std::cout << "missing text style " << _textInstanceDto->styleName << std::endl;
#endif
				return;
			}

			if (!fill) {
#if TRACE_GUI
				std::cout << "missing fill " << _textInstanceDto->fillBrushName << std::endl;
#endif
				return;
			}

			D2D1::Matrix3x2F product = D2D1::Matrix3x2F::Rotation(_textInstanceDto->rotation) * D2D1::Matrix3x2F::Translation(_textInstanceDto->position.x, _textInstanceDto->position.y);
			getDeviceContext()->SetTransform(product);

			D2D1_RECT_F rect = {};

			rect.left = _textInstanceDto->layout.x;
			rect.top = _textInstanceDto->layout.y;
			rect.right = _textInstanceDto->layout.x + _textInstanceDto->layout.w;
			rect.bottom = _textInstanceDto->layout.y + _textInstanceDto->layout.h;

			auto brush = fill->getBrush();
			int l = (_textInstanceDto->text.length() + 1) * 2;
			wchar_t* buff = new wchar_t[l];
			int ret = ::MultiByteToWideChar(CP_ACP, NULL, _textInstanceDto->text.c_str(), -1, buff, l - 1);
			getDeviceContext()->DrawText(buff, ret, style->getFormat(), &rect, brush);
			delete[] buff;
	}

		virtual void drawBitmap(bitmapInstanceDto* _bitmapInstanceDto)
		{
			auto bm = bitmaps[_bitmapInstanceDto->bitmapName];
			throwOnFalse(static_cast<bool>(bm), "Bitmap not found in context");
			auto ibm = bm->getBySize(_bitmapInstanceDto->width, _bitmapInstanceDto->height);
			throwOnNull(ibm, "Bitmap size not found in context");
			D2D1_RECT_F rect, source;
			rect.left = _bitmapInstanceDto->x;
			rect.top = _bitmapInstanceDto->y;
			auto size = ibm->GetSize();
			if (_bitmapInstanceDto->width) {
				rect.right = rect.left + _bitmapInstanceDto->width;
			}
			else
			{
				rect.right = rect.left + size.width;
			}
			if (_bitmapInstanceDto->height) {
				rect.bottom = rect.top + _bitmapInstanceDto->height;
			}
			else {
				rect.bottom = rect.top + size.height;
			}
			source.left = 0;
			source.top = 0;
			source.right = size.width;
			source.bottom = size.height;
			getDeviceContext()->DrawBitmap(ibm, rect, _bitmapInstanceDto->alpha, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, source);
		}

		virtual void popCamera()
		{
			if (transforms.empty())
				currentTransform = D2D1::Matrix3x2F::Identity();
			else {
				currentTransform = transforms.top();
				transforms.pop();
			}
		}

		virtual void pushCamera(point* _position, float _rotation, float _scale)
		{
			transforms.push(currentTransform);
			currentTransform = currentTransform * D2D1::Matrix3x2F::Rotation(_rotation)
				* D2D1::Matrix3x2F::Translation(_position->x, _position->y)
				* D2D1::Matrix3x2F::Scale(_scale, _scale);
			getDeviceContext()->SetTransform(currentTransform);
		}


		virtual std::shared_ptr<direct2dBitmap> createBitmap(point& _size)
		{
			auto adapt = getAdapter();
			auto bp = std::make_shared<direct2dBitmap>(toSizeF(_size), adapt);
			return bp;
		}

		virtual std::unique_ptr<direct2dBitmap> createD2dBitmap(D2D1_SIZE_F _size)
		{
			auto adapt = getAdapter();
			std::unique_ptr<direct2dBitmap> win = std::make_unique<direct2dBitmap>(_size, adapt);
			return win;
		}

		virtual void drawBitmap(drawableHost* _drawableHost, point& _dest, point& _size)
		{
			if (_drawableHost->isBitmap()) {
				direct2dBitmap* bp = (direct2dBitmap*)_drawableHost;
				auto wicbitmap = bp->getBitmap();
				ID2D1Bitmap* bitmap = NULL;
				HRESULT hr = this->getDeviceContext()->CreateBitmapFromWicBitmap(wicbitmap, &bitmap);
				throwOnFail(hr, "Could not create bitmap from wic bitmap");
				D2D1_RECT_F rect;
				rect.left = _dest.x;
				rect.top = _dest.y;
				rect.right = rect.left += _size.x > 0 ? _size.x : bp->size.width;
				rect.bottom = rect.top += _size.y > 0 ? _size.y : bp->size.height;
				getDeviceContext()->DrawBitmap(bitmap, &rect);
				bitmap->Release();
				bitmap = NULL;
			}
		}

		virtual void drawView(const char* _style, const char* _text, rectangle& _rect, int _state, const char* _debug_comment)
		{
			if (!_style) return;

			object_name style_name = _style;
			object_name style_composed_name;

			view_style_name(style_name, style_composed_name, _state);

			auto vs = viewStyles[style_composed_name.c_str()];
			auto& rectFill = vs->box_fill_color;
			drawRectangle(&_rect, vs->box_border_color.name, vs->box_border_thickness, vs->box_fill_color.name);

			_rect.h -= vs->box_border_thickness * 2.0;
			_rect.w -= vs->box_border_thickness * 2.0;
			_rect.x += vs->box_border_thickness;
			_rect.y += vs->box_border_thickness;

			drawText(_text, &_rect, vs->text_style.name, vs->shape_fill_color.name);

#if OUTLINE_GUI

			drawRectangle(&_rect, "debug-border", 2.0, nullptr);
			if (_debug_comment) {
				drawText(_debug_comment, &_rect, "debug-text", "debug-border");
			}
#endif
		}


		virtual void save(const char* _filename)
		{
			;
		}


	protected:

		std::stack<D2D1::Matrix3x2F> transforms;
		D2D1::Matrix3x2F currentTransform;

	public:

		friend class textStyle;
		friend class bitmap;
		friend class bitmapBrush;
		friend class solidColorBrush;
		friend class radialGradientBrush;
		friend class linearGradientBrush;
		friend class path;

		friend class direct2dWindow;
};

}

#endif
