
#pragma once

namespace corona
{
	namespace win32
	{

		class drawableHost;

		class directException {
		public:
			std::string message;
			unsigned int lastError;
		};

		enum dtoIconId
		{
			STOCKICO_DOCNOASSOC = 0,
			STOCKICO_DOCASSOC = 1,
			STOCKICO_APPLICATION = 2,
			STOCKICO_FOLDER = 3,
			STOCKICO_FOLDEROPEN = 4,
			STOCKICO_DRIVE525 = 5,
			STOCKICO_DRIVE35 = 6,
			STOCKICO_DRIVEREMOVE = 7,
			STOCKICO_DRIVEFIXED = 8,
			STOCKICO_DRIVENET = 9,
			STOCKICO_DRIVENETDISABLED = 10,
			STOCKICO_DRIVECD = 11,
			STOCKICO_DRIVERAM = 12,
			STOCKICO_WORLD = 13,
			STOCKICO_SERVER = 15,
			STOCKICO_PRINTER = 16,
			STOCKICO_MYNETWORK = 17,
			STOCKICO_FIND = 22,
			STOCKICO_HELP = 23,
			STOCKICO_SHARE = 28,
			STOCKICO_LINK = 29,
			STOCKICO_SLOWFILE = 30,
			STOCKICO_RECYCLER = 31,
			STOCKICO_RECYCLERFULL = 32,
			STOCKICO_MEDIACDAUDIO = 40,
			STOCKICO_LOCK = 47,
			STOCKICO_AUTOLIST = 49,
			STOCKICO_PRINTERNET = 50,
			STOCKICO_SERVERSHARE = 51,
			STOCKICO_PRINTERFAX = 52,
			STOCKICO_PRINTERFAXNET = 53,
			STOCKICO_PRINTERFILE = 54,
			STOCKICO_STACK = 55,
			STOCKICO_MEDIASVCD = 56,
			STOCKICO_STUFFEDFOLDER = 57,
			STOCKICO_DRIVEUNKNOWN = 58,
			STOCKICO_DRIVEDVD = 59,
			STOCKICO_MEDIADVD = 60,
			STOCKICO_MEDIADVDRAM = 61,
			STOCKICO_MEDIADVDRW = 62,
			STOCKICO_MEDIADVDR = 63,
			STOCKICO_MEDIADVDROM = 64,
			STOCKICO_MEDIACDAUDIOPLUS = 65,
			STOCKICO_MEDIACDRW = 66,
			STOCKICO_MEDIACDR = 67,
			STOCKICO_MEDIACDBURN = 68,
			STOCKICO_MEDIABLANKCD = 69,
			STOCKICO_MEDIACDROM = 70,
			STOCKICO_AUDIOFILES = 71,
			STOCKICO_IMAGEFILES = 72,
			STOCKICO_VIDEOFILES = 73,
			STOCKICO_MIXEDFILES = 74,
			STOCKICO_FOLDERBACK = 75,
			STOCKICO_FOLDERFRONT = 76,
			STOCKICO_SHIELD = 77,
			STOCKICO_WARNING = 78,
			STOCKICO_INFO = 79,
			STOCKICO_ERROR = 80,
			STOCKICO_KEY = 81,
			STOCKICO_SOFTWARE = 82,
			STOCKICO_RENAME = 83,
			STOCKICO_DELETE = 84,
			STOCKICO_MEDIAAUDIODVD = 85,
			STOCKICO_MEDIAMOVIEDVD = 86,
			STOCKICO_MEDIAENHANCEDCD = 87,
			STOCKICO_MEDIAENHANCEDDVD = 88,
			STOCKICO_MEDIAHDDVD = 89,
			STOCKICO_MEDIABLURAY = 90,
			STOCKICO_MEDIAVCD = 91,
			STOCKICO_MEDIADVDPLUSR = 92,
			STOCKICO_MEDIADVDPLUSRW = 93,
			STOCKICO_DESKTOPPC = 94,
			STOCKICO_MOBILEPC = 95,
			STOCKICO_USERS = 96,
			STOCKICO_MEDIASMARTMEDIA = 97,
			STOCKICO_MEDIACOMPACTFLASH = 98,
			STOCKICO_DEVICECELLPHONE = 99,
			STOCKICO_DEVICECAMERA = 100,
			STOCKICO_DEVICEVIDEOCAMERA = 101,
			STOCKICO_DEVICEAUDIOPLAYER = 102,
			STOCKICO_NETWORKCONNECT = 103,
			STOCKICO_INTERNET = 104,
			STOCKICO_ZIPFILE = 105,
			STOCKICO_SETTINGS = 106,
			STOCKICO_DRIVEHDDVD = 132,
			STOCKICO_DRIVEBD = 133,
			STOCKICO_MEDIAHDDVDROM = 134,
			STOCKICO_MEDIAHDDVDR = 135,
			STOCKICO_MEDIAHDDVDRAM = 136,
			STOCKICO_MEDIABDROM = 137,
			STOCKICO_MEDIABDR = 138,
			STOCKICO_MEDIABDRE = 139,
			STOCKICO_CLUSTEREDDRIVE = 140,
			STOCKICO_MAX_ICONS = 175
		};

		enum valignment { e_valign_top, e_valign_center, e_valign_bottom };
		enum halignment { e_halign_left, e_halign_center, e_halign_right };

		struct spriteDto
		{
			std::string spriteName;
			std::string spriteState;
			int frameId;
			int frameDelay;
		};

		struct colorDto {
			float red, green, blue, alpha;
		};

		struct pointDto {
			float x, y;
			pointDto();
			pointDto(float _x, float _y);
		};

		struct marginDto {
			float top, bottom, left, right;
			marginDto();
			marginDto(float _top, float _bottom, float _left, float _right);
		};

		struct rectDto {
			float left, top, width, height;
			rectDto();
			rectDto(float _left, float _top, float _width, float _height);
		};

		struct sizeIntDto {
			int width, height;
			sizeIntDto();
			sizeIntDto(int _width, int _height);
		};

		struct sizeFloatDto {
			float width, height;
			sizeFloatDto();
			sizeFloatDto(float _width, float _height);
		};

		struct sPointDto {
			float x, y;
		};

		struct gradientStopDto {
			float position;
			colorDto color;
		};

		/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

		struct bitmapDto {
			std::string name;
			std::string filename;
			bool cropEnabled;
			marginDto crop;
			std::list<sizeIntDto> sizes;
		};

		struct bitmapBrushDto {
			std::string name;
			std::string bitmapName;
		};

		struct solidBrushDto {
			std::string name;
			colorDto color;
			bool stock;
		};

		struct linearGradientBrushDto {
			std::string name;
			pointDto	start,
				stop;
			std::list<gradientStopDto> gradientStops;
			bool stock;
		};

		struct radialGradientBrushDto {
			std::string name;
			pointDto	center,
				offset;
			float		radiusX,
				radiusY;
			bool stock;
			std::list<gradientStopDto> gradientStops;
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
			pointDto point;

			pathLineDto();
			virtual pathBaseDto* clone();
		};

		class pathArcDto : public pathBaseDto {
		public:
			pointDto point;
			double angleDegrees;
			double radiusX;
			double radiusY;

			pathArcDto();
			virtual pathBaseDto* clone();
		};

		class pathQuadraticBezierDto : public pathBaseDto {
		public:
			pointDto point1;
			pointDto point2;

			pathQuadraticBezierDto();
			virtual pathBaseDto* clone();
		};

		class pathBezierDto : public pathBaseDto {
		public:
			pointDto point1;
			pointDto point2;
			pointDto point3;

			pathBezierDto();
			virtual pathBaseDto* clone();
		};

		class pathDto {
		public:
			std::string name;
			std::list<pathBaseDto*> points;

			pathDto();
			pathDto(const pathDto& _src);
			pathDto operator =(const pathDto& _src);
			virtual ~pathDto();
			void copyPoints(const pathDto& _src);

			void clear();
		};

		struct textStyleDto {
			std::string name;
			std::string fontName;
			float fontSize;
			bool bold, italics;
		};

		struct textInstance2dDto {
			std::string text;
			std::string fillBrushName;
			std::string backgroundBrushName;
			std::string styleName;
			pointDto position;
			float rotation;
			rectDto layout;

			textInstance2dDto();
		};

		struct pathImmediateDto {
			pathDto path;
			std::string fillBrushName,
				borderBrushName;
			int strokeWidth;
			pointDto position;
			float rotation;
			bool closed;
		};

		struct pathInstance2dDto {
			std::string pathName,
				fillBrushName,
				borderBrushName;
			int strokeWidth;
			pointDto position;
			float rotation;
		};


		struct textDto {
			std::string text;
			std::string textStyleName;
		};

		struct bitmapInstanceDto {
			std::string bitmapName;
			int copyId;
			float x, y, width, height;
			bool selected;
			spriteDto sprite;

			bool contains(pointDto pt)
			{
				return (pt.x >= x) && (pt.x < (x + width)) && (pt.y >= y) && (pt.y < (y + height));
			}
		};

		struct bitmapFilterDto {
			std::function<bool(sizeIntDto, int, int, char*)> filterFunction;
		};


		struct errorDto {
		public:
			bool error;
			std::string message;
		};


		class dtoFactory {
		public:
			colorDto color;
			pointDto point1, point2, point3;
			gradientStopDto gradientStop;
			bitmapDto bitmap;
			bitmapBrushDto bitmapBrush;
			solidBrushDto solidBrush;
			linearGradientBrushDto linearGradientBrush;
			radialGradientBrushDto radialGradientBrush;
			pathDto path;
			textStyleDto textStyle;
			double frame_elapsed;

			dtoFactory();
			std::string createName(const char* _name, int _counter);
			dtoFactory* colorMake(float _red, float _green, float _blue, float _alpha);
			dtoFactory* gradientStopMake(float _stop);
			dtoFactory* gradientStopMake(float _red, float _green, float _blue, float _alpha, float _stop);
			dtoFactory* bitmapMake(drawableHost* _host, const char* _name, const char* _fileName, int numSizes, sizeIntDto* _sizes);
			dtoFactory* bitmapBrushMake(drawableHost* _host, const char* _name, const char* _bitmapName);
			dtoFactory* solidBrushMake(drawableHost* _host, const char* _name, bool stock);
			dtoFactory* point1Make(float _x, float _y);
			dtoFactory* point2Make(float _x, float _y);
			dtoFactory* point3Make(float _x, float _y);
			dtoFactory* pathArcMake(float _x, float _y, double _angle, float _radiusx, float _radiusy);
			dtoFactory* pathBezierMake(float _x1, float _y1, float _x2, float _y2, float _x3, float _y3);
			dtoFactory* pathQuadraticBezierMake(float _x1, float _y1, float _x2, float _y2);
			dtoFactory* pathLineMake(float _x, float _y);
			dtoFactory* pathLineMake(sPointDto* _points, int _count);
			dtoFactory* pathLinePlotMake(double* _points, double _step, int _count);
			dtoFactory* pathSolidPlotMake(double* _points, double _step, int _count);
			dtoFactory* linearGradientBrushMake(drawableHost* _host, const char* _name, bool stock);
			dtoFactory* radialGradientBrushMake(drawableHost* _host, const char* _name, bool stock);
			dtoFactory* linearGradientBrushMake(drawableHost* _host, pointDto point1, pointDto point2, const char* _name, bool stock);
			dtoFactory* radialGradientBrushMake(drawableHost* _host, pointDto center, pointDto offset, pointDto radius, const char* _name, bool _stock);
			dtoFactory* textStyleMake(drawableHost* _host, const char* _name, const char* _fontName, float _size, bool _bold, bool _italics);
			dtoFactory* pathMake(drawableHost* _host, const char* _name, bool _closed = true);
			dtoFactory* pathMake(pathImmediateDto* _dest, const char* _name, const char* _fillName, const char* _borderName, int _borderWidth, bool _closed = true);
			dtoFactory* rectangleMake(pathImmediateDto* _dest, float _x1, float _y1, float _width, float _height, const char* _name, const char* _fillName, const char* _borderName, int _borderWidth);
			dtoFactory* rectangleMake(drawableHost* _host, float _x1, float _y1, float _width, float _height, const char* _name);
		};
	}
}
