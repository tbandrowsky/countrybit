
module;


#include <list>
#include <vector>
#include <string>
#include <memory>
#include <compare>
#include <functional>

export module corona.database:visual;
import "corona.database-windows-all.h";

import :constants;
import :string_box;
import :color_box;
import :point_box;
import :rectangle_box;

export struct sizeCrop 
{
	D2D1_SIZE_U size;
	bool cropEnabled;
	D2D1_RECT_F crop;
};

export enum class visual_alignment
{
	align_none = 0,
	align_near = 1,
	align_center = 2,
	align_justify = 3,
	align_far = 4,
};

export struct gradientStop
{
	float stop_position;
	color stop_color;

	gradientStop()
	{
		stop_position = 0;
		stop_color.r = 0;
		stop_color.g = 0;
		stop_color.b = 0;
		stop_color.a = 1.0;
	}

	gradientStop(color _color, float _position)
	{
		stop_position = _position;
		stop_color = _color;
	}
};

/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

export struct bitmapRequest {
	object_name name;
	file_path file_name;
	DWORD resource_id;
	bool cropEnabled;
	rectangle crop;
	std::list<point> sizes;
};

export struct bitmapBrushRequest {
	object_name name;
	std::string bitmapName;
};

export struct linearGradientBrushRequest {
	object_name name;
	point	start,
		stop;
	std::vector<gradientStop> gradientStops;
};

export struct radialGradientBrushRequest {
	object_name name;
	point	center,
		offset;
	float		radiusX,
		radiusY;
	std::vector<gradientStop> gradientStops;
};

export enum ePathPointType {
	e_line,
	e_arc,
	e_bezier,
	e_quadractic_bezier
};

class pathLineDto;
class pathArcDto;
class pathBezierDto;
class pathQuadraticBezierDto;

export class pathBaseDto {
public:
	ePathPointType eType;

	inline pathLineDto* asPathLineDto() { return eType == e_line ? (pathLineDto*)this : NULL; }
	inline pathArcDto* asPathArcDto() { return eType == e_line ? (pathArcDto*)this : NULL; }
	inline pathBezierDto* asPathBezierDto() { return eType == e_line ? (pathBezierDto*)this : NULL; }
	inline pathQuadraticBezierDto* asPathQuadraticBezierDto() { return eType == e_line ? (pathQuadraticBezierDto*)this : NULL; }
};

export class pathLineDto : public pathBaseDto {
public:
	point point;

	pathLineDto() {
		eType = ePathPointType::e_line;
		point = {};
	}
};

export class pathArcDto : public pathBaseDto {
public:
	point point;
	double angleDegrees;
	double radiusX;
	double radiusY;

	pathArcDto() {
		eType = ePathPointType::e_arc;
		point = {};
	}
};

export class pathQuadraticBezierDto : public pathBaseDto {
public:
	point point1;
	point point2;

	pathQuadraticBezierDto() {
		eType = ePathPointType::e_quadractic_bezier;
		point1 = {};
		point2 = {};
	}
};

export class pathBezierDto : public pathBaseDto {
public:
	point point1;
	point point2;
	point point3;

	pathBezierDto() {
		eType = ePathPointType::e_bezier;
		point1 = {};
		point2 = {};
		point3 = {};
	}
};

export class pathDto {
public:
	object_name name;
	std::vector<std::shared_ptr<pathBaseDto>> points;

	pathDto& addLineTo(double x, double y)
	{
		std::shared_ptr<pathLineDto> ndto = std::make_shared<pathLineDto>();
		ndto->point.x = x;
		ndto->point.y = y;
		ndto->point.z = 0;
		points.push_back(ndto);
		return *this;
	}

	pathDto& addPathArc(double x, double y, double _angleDegrees, double _radiusX, double _radiusY)
	{
		std::shared_ptr<pathArcDto> ndto = std::make_shared<pathArcDto>();
		ndto->point.x = x;
		ndto->point.y = y;
		ndto->point.z = 0;
		ndto->angleDegrees = _angleDegrees;
		ndto->radiusX = _radiusX;
		ndto->radiusY = _radiusY;
		points.push_back(ndto);
		return *this;
	}

	pathDto& addPathQuadraticBezier(double x, double y, double x1, double y1)
	{
		std::shared_ptr<pathQuadraticBezierDto> ndto = std::make_shared<pathQuadraticBezierDto>();
		ndto->point1.x = x;
		ndto->point1.y = y;
		ndto->point1.z = 0;
		ndto->point2.x = x1;
		ndto->point2.y = y1;
		ndto->point2.z = 0;
		points.push_back(ndto);
		return *this;
	}

	pathDto& addPathQuadraticBezier(double x, double y, double x1, double y1, double x2, double y2)
	{
		std::shared_ptr<pathBezierDto> ndto = std::make_shared<pathBezierDto>();
		ndto->point1.x = x;
		ndto->point1.y = y;
		ndto->point1.z = 0;
		ndto->point2.x = x1;
		ndto->point2.y = y1;
		ndto->point2.z = 0;
		ndto->point3.x = x2;
		ndto->point3.y = y2;
		ndto->point3.z = 0;
		points.push_back(ndto);
		return *this;
	}

};

export struct solidBrushRequest {
	object_name_composed name;
	color brushColor;
	bool active;
};

export struct drawTextRequest {
	std::string text;
	std::string fillBrushName;
	std::string backgroundBrushName;
	std::string styleName;
	point position;
	float rotation;
	rectangle layout;

	drawTextRequest();
};



export struct pathImmediateDto {
	pathDto path;
	std::string fillBrushName,
		borderBrushName;
	int strokeWidth;
	point position;
	float rotation;
	bool closed;
};

export struct pathInstance2dDto {
	std::string pathName,
		fillBrushName,
		borderBrushName;
	int strokeWidth;
	point position;
	float rotation;
};

export struct bitmapInstanceDto {
	std::string bitmapName;
	int copyId;
	float x, y, width, height;
	bool selected;
	double alpha;

	bool contains(point pt)
	{
		return (pt.x >= x) && (pt.x < (x + width)) && (pt.y >= y) && (pt.y < (y + height));
	}
};

export struct textStyleRequest
{
	object_name_composed name;
	object_name fontName;
	float fontSize;
	bool bold, italics, underline, strike_through;
	double line_spacing;
	visual_alignment horizontal_align;
	visual_alignment vertical_align;
	bool wrap_text;
};

export struct viewStyleRequest
{
	object_name			name;
	textStyleRequest	text_style;
	double shape_border_thickness;
	double box_border_thickness;
	solidBrushRequest box_border_color;
	solidBrushRequest shape_border_color;
	solidBrushRequest box_fill_color;
	solidBrushRequest shape_fill_color;
};

export struct bitmapFilterFunction {
	std::function<bool(point, int, int, char*)> filterFunction;
};

export sizeCrop toSizeC(point& _size, bool _cropEnabled, rectangle& _crop)
{
	sizeCrop sz;
	sz.size.width = _size.x;
	sz.size.height = _size.y;
	sz.cropEnabled = _cropEnabled;
	sz.crop.left = _crop.x;
	sz.crop.top = _crop.y;
	sz.crop.right = _crop.x + _crop.w;
	sz.crop.bottom = _crop.y + _crop.h;
	return sz;
}

export D2D1_SIZE_U toSizeU(point& _size)
{
	D2D1_SIZE_U newSize;
	newSize.width = _size.x;
	newSize.height = _size.y;
	return newSize;
}

export D2D1_SIZE_F toSizeF(point& _size)
{
	D2D1_SIZE_F newSize;
	newSize.width = _size.x;
	newSize.height = _size.y;
	return newSize;
}

export point toSize(D2D1_SIZE_U& _size)
{
	point newSize;
	newSize.x = _size.width;
	newSize.y = _size.height;
	return newSize;
}

export int toInt(char hex, int shift)
{
	int d = {};
	hex = toupper(hex);

	if (hex >= 'A' && hex <= 'F')
	{
		d = hex - 'A' + 10;
	}
	else if (hex >= '0' && hex <= '9')
	{
		d = hex - '0';
	}
	d <<= shift;
	return d;
}

export int toInt2(const std::string& item, int _baseIndex)
{
	int r = toInt(item[_baseIndex], 4) + toInt(item[_baseIndex + 1], 0);
	return r;
}

export D2D1_COLOR_F toColor(const char* _htmlColor)
{
	D2D1_COLOR_F new_color = {};

	int si = {}, r = {}, g = {}, b = {}, a = 255;
	int sz = strlen(_htmlColor);

	if (sz > 0)
	{
		si = _htmlColor[0] == '#' ? 1 : 0;
	}

	if (sz >= 6)
	{
		r = toInt2(_htmlColor, si);
		g = toInt2(_htmlColor, si + 2);
		b = toInt2(_htmlColor, si + 4);
	}

	if (sz >= 8)
	{
		a = toInt2(_htmlColor, si + 6);
	}

	new_color.r = r / 255.0;
	new_color.g = g / 255.0;
	new_color.b = b / 255.0;
	new_color.a = a / 255.0;

	return new_color;
}

export D2D1_COLOR_F toColor(std::string& _htmlColor)
{
	return toColor(_htmlColor.c_str());
}


export D2D1_COLOR_F toColor(color& _color)
{
	return _color;
}

export D2D1_POINT_2F toPoint(point& _point)
{
	D2D1_POINT_2F point2;
	point2.x = _point.x;
	point2.y = _point.y;
	return point2;
}

export D2D1_GRADIENT_STOP toGradientStop(gradientStop& _gradientStop)
{
	D2D1_GRADIENT_STOP stop;

	stop.position = _gradientStop.stop_position;
	stop.color = toColor(_gradientStop.stop_color);
	return stop;
}


