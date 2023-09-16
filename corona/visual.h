#pragma once

namespace corona
{
	namespace database
	{
		struct gradientStop 
		{
			float stop_position;
			color stop_color;

			gradientStop();
			gradientStop(color _color, float _position);
		};

		/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

		struct bitmapRequest {
			object_name name;
			file_path file_name;
			DWORD resource_id;
			bool cropEnabled;
			rectangle crop;
			std::list<point> sizes;
		};

		struct bitmapBrushRequest {
			object_name name;
			std::string bitmapName;
		};

		struct linearGradientBrushRequest {
			object_name name;
			point	start,
				stop;
			std::vector<gradientStop> gradientStops;
		};

		struct radialGradientBrushRequest {
			object_name name;
			point	center,
				offset;
			float		radiusX,
				radiusY;
			std::vector<gradientStop> gradientStops;
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

		struct solidBrushRequest {
			object_name_composed name;
			color brushColor;
			bool active;
		};

		struct drawTextRequest {
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
			object_name_composed name;
			object_name fontName;
			float fontSize;
			bool bold, italics, underline, strike_through;
			double line_spacing;
			visual_alignment horizontal_align;
			visual_alignment vertical_align;
			bool wrap_text;
		};

		struct viewStyleRequest
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

		struct bitmapFilter {
			std::function<bool(point, int, int, char*)> filterFunction;
		};

	}

}
