#ifndef CORONA_VISUAL
#define CORONA_VISUAL

namespace corona {

	struct sizeCrop
	{
		D2D1_SIZE_U size;
		bool cropEnabled;
		D2D1_RECT_F crop;
	};

	enum class visual_alignment
	{
		align_none = 0,
		align_near = 1,
		align_center = 2,
		align_justify = 3,
		align_far = 4,
	};

	struct gradientStop
	{
		float stop_position;
		ccolor stop_color;

		gradientStop()
		{
			stop_position = 0;
			stop_color.r = 0;
			stop_color.g = 0;
			stop_color.b = 0;
			stop_color.a = 1.0;
		}

		gradientStop(ccolor _color, float _position)
		{
			stop_position = _position;
			stop_color = _color;
		}
	};

	/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

	class bitmapRequest 
	{
	public:
		std::string	name;
		std::string	file_name;
		DWORD		resource_id;
		bool		cropEnabled;
		rectangle	crop;
		std::list<point> sizes;

		bitmapRequest()
		{
			resource_id = {};
			cropEnabled = false;
			crop = {};
		}

		bitmapRequest(const bitmapRequest& _request) = default;
		bitmapRequest(bitmapRequest&& _request) = default;
		bitmapRequest& operator = (const bitmapRequest& _request) = default;
		bitmapRequest& operator = (bitmapRequest&& _request) = default;

		bitmapRequest(std::shared_ptr<bitmapRequest>& _request)
		{
			name = _request->name;
			file_name = _request->file_name;
			resource_id = _request->resource_id;
			cropEnabled = _request->cropEnabled;
			crop = _request->crop;
			sizes = _request->sizes;
		}
	};

	class bitmapBrushRequest 
	{
	public:
		std::string name;
		std::string bitmapName;

		bitmapBrushRequest() = default;
		bitmapBrushRequest(const bitmapBrushRequest& _request) = default;
		bitmapBrushRequest(bitmapBrushRequest&& _request) = default;
		bitmapBrushRequest& operator = (const bitmapBrushRequest& _request) = default;
		bitmapBrushRequest& operator = (bitmapBrushRequest&& _request) = default;

		bitmapBrushRequest(std::shared_ptr<bitmapBrushRequest> _request)
		{
			name = _request->name;
			bitmapName = _request->bitmapName;
		}

	};

	class linearGradientBrushRequest 
	{
	public:
		std::string name;
		point		start,
					stop;
		std::vector<gradientStop> gradientStops;

		linearGradientBrushRequest() = default;
		linearGradientBrushRequest(const linearGradientBrushRequest& _request) = default;
		linearGradientBrushRequest(linearGradientBrushRequest&& _request) = default;
		linearGradientBrushRequest& operator = (const linearGradientBrushRequest& _request) = default;
		linearGradientBrushRequest& operator = (linearGradientBrushRequest&& _request) = default;

		linearGradientBrushRequest(std::shared_ptr<linearGradientBrushRequest> _request)
		{
			name = _request->name;
			start = _request->start;
			stop = _request->stop;
			gradientStops = _request->gradientStops;
		}

	};

	class radialGradientBrushRequest 
	{
	public:
		std::string name;
		point		center,
					offset;
		float		radiusX,
					radiusY;
		std::vector<gradientStop> gradientStops;

		radialGradientBrushRequest() = default;
		radialGradientBrushRequest(const radialGradientBrushRequest& _request) = default;
		radialGradientBrushRequest(radialGradientBrushRequest&& _request) = default;
		radialGradientBrushRequest& operator = (const radialGradientBrushRequest& _request) = default;
		radialGradientBrushRequest& operator = (radialGradientBrushRequest&& _request) = default;

		radialGradientBrushRequest(std::shared_ptr<radialGradientBrushRequest> _request)
		{
			name = _request->name;
			center = _request->center;
			offset = _request->offset;
			radiusX = _request->radiusX;
			radiusY = _request->radiusY;
			gradientStops = _request->gradientStops;
		}

	};

	class solidBrushRequest
	{
	public:
		std::string name;
		ccolor brushColor;
		bool active;

		solidBrushRequest() = default;
		solidBrushRequest(const solidBrushRequest& _request) = default;
		solidBrushRequest(solidBrushRequest&& _request) = default;
		solidBrushRequest& operator = (const solidBrushRequest& _request) = default;
		solidBrushRequest& operator = (solidBrushRequest&& _request) = default;

		solidBrushRequest(std::shared_ptr<solidBrushRequest> _request)
		{
			name = _request->name;
			brushColor = _request->brushColor;
			active = _request->active;
		}


	};

	enum ePathPointType {
		e_line,
		e_arc,
		e_bezier,
		e_quadractic_bezier
	};

	class pathLineDto;
	class pathArcDto;
	class pathBezierDto;
	class pathQuadraticBezierDto;

	class pathBaseDto {
	public:
		ePathPointType eType;

		inline pathLineDto* asPathLineDto() { return eType == e_line ? (pathLineDto*)this : NULL; }
		inline pathArcDto* asPathArcDto() { return eType == e_line ? (pathArcDto*)this : NULL; }
		inline pathBezierDto* asPathBezierDto() { return eType == e_line ? (pathBezierDto*)this : NULL; }
		inline pathQuadraticBezierDto* asPathQuadraticBezierDto() { return eType == e_line ? (pathQuadraticBezierDto*)this : NULL; }
	};

	class pathLineDto : public pathBaseDto {
	public:
		point point;

		pathLineDto() {
			eType = ePathPointType::e_line;
			point = {};
		}
	};

	class pathArcDto : public pathBaseDto {
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

	class pathQuadraticBezierDto : public pathBaseDto {
	public:
		point point1;
		point point2;

		pathQuadraticBezierDto() {
			eType = ePathPointType::e_quadractic_bezier;
			point1 = {};
			point2 = {};
		}
	};

	class pathBezierDto : public pathBaseDto {
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

	class pathDto {
	public:
		std::string name;
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



	D2D1_COLOR_F toColor(std::string _htmlColor)
	{
		return toColor(_htmlColor.c_str());
	}


	D2D1_COLOR_F toColor(const ccolor& _color)
	{
		return _color;
	}

	enum brush_types 
	{
		no_brush_type = 0,
		solid_brush_type = 1,
		linear_brush_type = 2,
		radial_brush_type = 3,
		bitmap_brush_type = 4
	};

	class generalBrushRequest
	{
	public:

		brush_types brush_type;
		std::shared_ptr<solidBrushRequest> solid_brush;
		std::shared_ptr<radialGradientBrushRequest> radial_brush;
		std::shared_ptr<linearGradientBrushRequest> linear_brush;
		std::shared_ptr<bitmapBrushRequest> bitmap_brush;
		bool active;

		generalBrushRequest()
		{
			clear();
		}

		void clear()
		{
			brush_type = brush_types::no_brush_type;
			solid_brush = nullptr;
			radial_brush = nullptr;
			linear_brush = nullptr;
			bitmap_brush = nullptr;
			active = false;
		}

		std::string set_name(std::string _name)
		{
			switch (brush_type) {
			case brush_types::solid_brush_type:
				solid_brush->name = _name;
				break;
			case brush_types::radial_brush_type:
				radial_brush->name = _name;
				break;
			case brush_types::linear_brush_type:
				linear_brush->name = _name;
				break;
			case brush_types::bitmap_brush_type:
				bitmap_brush->name = _name;
				break;
			}
			return _name;
		}

		const char *get_name()
		{
			const char *name = nullptr;
			switch (brush_type) {
			case brush_types::solid_brush_type:
				name = solid_brush->name.c_str();
				break;
			case brush_types::radial_brush_type:
				name = radial_brush->name.c_str();
				break;
			case brush_types::linear_brush_type:
				name = linear_brush->name.c_str();
				break;
			case brush_types::bitmap_brush_type:
				name = bitmap_brush->name.c_str();
				break;
			}
			return name;
		}


		generalBrushRequest(const generalBrushRequest& gbr)
		{
			clear();
			brush_type = gbr.brush_type;
			switch (brush_type) {
			case brush_types::solid_brush_type:
				solid_brush = std::make_shared<solidBrushRequest>(gbr.solid_brush);
				break;
			case brush_types::radial_brush_type:
				radial_brush = std::make_shared<radialGradientBrushRequest>(gbr.radial_brush);
				break;
			case brush_types::linear_brush_type:
				linear_brush = std::make_shared<linearGradientBrushRequest>(gbr.linear_brush);
				break;
			case brush_types::bitmap_brush_type:
				bitmap_brush = std::make_shared<bitmapBrushRequest>(gbr.bitmap_brush);
				break;
			}
		}

		generalBrushRequest operator = (const generalBrushRequest& gbr)
		{
			clear();
			brush_type = gbr.brush_type;
			switch (brush_type) {
			case brush_types::solid_brush_type:
				solid_brush = std::make_shared<solidBrushRequest>(gbr.solid_brush);
				break;
			case brush_types::radial_brush_type:
				radial_brush = std::make_shared<radialGradientBrushRequest>(gbr.radial_brush);
				break;
			case brush_types::linear_brush_type:
				linear_brush = std::make_shared<linearGradientBrushRequest>(gbr.linear_brush);
				break;
			case brush_types::bitmap_brush_type:
				bitmap_brush = std::make_shared<bitmapBrushRequest>(gbr.bitmap_brush);
			}

			return *this;
		}

		generalBrushRequest(solidBrushRequest sbr)
		{
			clear();
			brush_type = brush_types::solid_brush_type;
			solid_brush = std::make_shared<solidBrushRequest>(sbr);
		}

		generalBrushRequest(linearGradientBrushRequest sbr)
		{
			clear();
			brush_type = brush_types::linear_brush_type;
			linear_brush = std::make_shared<linearGradientBrushRequest>(sbr);
		}

		generalBrushRequest(radialGradientBrushRequest sbr)
		{
			clear();
			brush_type = brush_types::radial_brush_type;
			radial_brush = std::make_shared<radialGradientBrushRequest>(sbr);
		}

		generalBrushRequest operator = (const solidBrushRequest& sbr)
		{
			clear();
			brush_type = brush_types::solid_brush_type;
			solid_brush = std::make_shared<solidBrushRequest>(sbr);
			return *this;
		}

		generalBrushRequest operator = (linearGradientBrushRequest sbr)
		{
			clear();
			brush_type = brush_types::linear_brush_type;
			linear_brush = std::make_shared<linearGradientBrushRequest>(sbr);
			return *this;
		}

		generalBrushRequest operator = (radialGradientBrushRequest sbr)
		{
			clear();
			brush_type = brush_types::radial_brush_type;
			radial_brush = std::make_shared<radialGradientBrushRequest>(sbr);
		}

		void setColor(std::string _color)
		{
			solidBrushRequest sbr;
			sbr.brushColor = toColor(_color);
			sbr.active = true;
			*this = sbr;
		}

		ccolor getColor()
		{
			ccolor t = {};

			switch (brush_type) {
			case brush_types::solid_brush_type:
				t = solid_brush->brushColor;
				break;
			case brush_types::radial_brush_type:
				if (radial_brush->gradientStops.size())
					t = radial_brush->gradientStops[0].stop_color;
				break;
			case brush_types::linear_brush_type:
				if (linear_brush->gradientStops.size())
					t = linear_brush->gradientStops[0].stop_color;
				break;
			}

			return t;
		}

	};

	struct drawTextRequest 
	{
		std::string text;
		std::string fillBrushName;
		std::string backgroundBrushName;
		std::string styleName;
		point position;
		float rotation;
		rectangle layout;

		drawTextRequest();
	};

	struct pathImmediateDto {
		pathDto path;
		std::string fillBrushName,
			borderBrushName;
		int strokeWidth;
		point position;
		float rotation;
		bool closed;
	};

	struct pathInstance2dDto {
		std::string pathName,
			fillBrushName,
			borderBrushName;
		int strokeWidth;
		point position;
		float rotation;
	};

	struct bitmapInstanceDto {
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

	struct textStyleRequest
	{
		std::string name;
		std::string fontName;
		float fontSize;
		bool bold, italics, underline, strike_through;
		double line_spacing;
		visual_alignment horizontal_align;
		visual_alignment vertical_align;
		bool wrap_text;
		double character_spacing;
		DWRITE_FONT_STRETCH font_stretch;
	};

	struct viewStyleRequest
	{
		std::string			name;
		textStyleRequest	text_style;
		double shape_border_thickness;
		double box_border_thickness;
		solidBrushRequest box_border_color;
		solidBrushRequest shape_border_color;
		solidBrushRequest box_fill_color;
		solidBrushRequest shape_fill_color;
	};

	struct bitmapFilterFunction {
		std::function<bool(point, int, int, char*)> filterFunction;
	};

	sizeCrop toSizeC(point& _size, bool _cropEnabled, rectangle& _crop)
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

	D2D1_SIZE_U toSizeU(point& _size)
	{
		D2D1_SIZE_U newSize;
		newSize.width = _size.x;
		newSize.height = _size.y;
		return newSize;
	}

	D2D1_SIZE_F toSizeF(point& _size)
	{
		D2D1_SIZE_F newSize;
		newSize.width = _size.x;
		newSize.height = _size.y;
		return newSize;
	}

	point toSize(D2D1_SIZE_U& _size)
	{
		point newSize;
		newSize.x = _size.width;
		newSize.y = _size.height;
		return newSize;
	}


	D2D1_COLOR_F toColor(const char* _htmlColor)
	{
		D2D1_COLOR_F new_color = {};

		int si = {}, r = {}, g = {}, b = {}, a = 255;
		int sz = strlen(_htmlColor);

		if (sz > 0)
		{
			si = (_htmlColor[0] == '#') ? 1 : 0;
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

	D2D1_POINT_2F toPoint(const point& _point)
	{
		D2D1_POINT_2F point2;
		point2.x = _point.x;
		point2.y = _point.y;
		return point2;
	}

	D2D1_GRADIENT_STOP toGradientStop(const gradientStop& _gradientStop)
	{
		D2D1_GRADIENT_STOP stop;

		stop.position = _gradientStop.stop_position;
		stop.color = toColor(_gradientStop.stop_color);
		return stop;
	}

}

#endif
