#pragma once

namespace corona
{
	namespace database
	{

		struct gradientStop {
			float position;
			color color;
		};

		/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

		struct bitmapRequest {
			object_name name;
			file_path file_name;
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
			iarray<gradientStop, 16> gradientStops;
			bool stock;
		};

		struct radialGradientBrushRequest {
			object_name name;
			point	center,
				offset;
			float		radiusX,
				radiusY;
			bool stock;
			iarray<gradientStop, 16> gradientStops;
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
			virtual pathBaseDto* clone() = 0;
		};

		class pathLineDto : public pathBaseDto {
		public:
			point point;

			pathLineDto();
			virtual pathBaseDto* clone();
		};

		class pathArcDto : public pathBaseDto {
		public:
			point point;
			double angleDegrees;
			double radiusX;
			double radiusY;

			pathArcDto();
			virtual pathBaseDto* clone();
		};

		class pathQuadraticBezierDto : public pathBaseDto {
		public:
			point point1;
			point point2;

			pathQuadraticBezierDto();
			virtual pathBaseDto* clone();
		};

		class pathBezierDto : public pathBaseDto {
		public:
			point point1;
			point point2;
			point point3;

			pathBezierDto();
			virtual pathBaseDto* clone();
		};

		class pathDto {
		public:
			object_name name;
			std::list<pathBaseDto*> points;

			pathDto();
			pathDto(const pathDto& _src);
			pathDto operator =(const pathDto& _src);
			virtual ~pathDto();
			void copyPoints(const pathDto& _src);

			void clear();
		};

		struct solidBrushRequest {
			object_name name;
			color brushColor;
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

			bool contains(point pt)
			{
				return (pt.x >= x) && (pt.x < (x + width)) && (pt.y >= y) && (pt.y < (y + height));
			}
		};

		struct viewStyleRequest
		{
			object_name name;
			object_name fontName;
			float fontSize;
			bool bold, italics;
			double line_spacing;
			visual_alignment horizontal_align;
			visual_alignment vertical_align;
			bool wrap_text;
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
