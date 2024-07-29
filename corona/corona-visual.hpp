/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/

#ifndef CORONA_VISUAL
#define CORONA_VISUAL

namespace corona {

	using ccolor = D2D1_COLOR_F;

	struct sizeCrop
	{
		D2D1_SIZE_U size;
		bool cropEnabled;
		D2D1_RECT_F crop;
	};

	void get_json(json& _dest, D2D1_SIZE_U& _src)
	{
		_dest.put_member("width", _src.width);
		_dest.put_member("height", _src.height);
	}

	void put_json(D2D1_SIZE_U& _dest, json& _src)
	{
		if (!_src.has_members({ "width", "height" })) {
			system_monitoring_interface::global_mon->log_warning("D2D1_SIZE_U needs a width and height");
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
		}
		_dest.width = (double)_src["width"];
		_dest.height = (double)_src["height"];
	}

	void get_json(json& _dest, D2D1_RECT_F& _src)
	{
		_dest.put_member("left", _src.left);
		_dest.put_member("top", _src.top);
		_dest.put_member("right", _src.right);
		_dest.put_member("bottom", _src.bottom);
	}

	void put_json(D2D1_RECT_F& _dest, json& _src)
	{
		std::vector<std::string> missing;
		if (!_src.has_members(missing, { "left", "top", "right", "bottom" })) {
			system_monitoring_interface::global_mon->log_warning("D2D1_RECT_F needs a crop and size");
			system_monitoring_interface::global_mon->log_warning("is missing:");
			std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
				system_monitoring_interface::global_mon->log_bus(s);
				});
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		_dest.left = (double)_src["left"];
		_dest.top = (double)_src["top"];
		_dest.right = (double)_src["right"];
		_dest.bottom = (double)_src["bottom"];
	}

	void get_json(json& _dest, sizeCrop& _src)
	{
		json_parser jp;
		json jcrop, jsize;
		jcrop = jp.create_object();
		jsize = jp.create_object();
		get_json(jcrop, _src.crop);
		get_json(jsize, _src.size);
		_dest.put_member("crop", jcrop);
		_dest.put_member("size", jsize);
		_dest.put_member("enabled", _src.cropEnabled);
	}

	void put_json(sizeCrop& _dest, json& _src)
	{

		std::vector<std::string> missing;
		if (!_src.has_members(missing, { "crop", "size" })) {
			system_monitoring_interface::global_mon->log_warning("sizeCrop needs a crop and size");
			system_monitoring_interface::global_mon->log_warning("is missing:");
			std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
				system_monitoring_interface::global_mon->log_bus(s);
				});
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		json jcrop, jsize;
		jcrop = _src["crop"];
		jsize = _src["size"];
		_dest.cropEnabled = (bool)_src["enabled"];
		put_json(_dest.crop, jcrop);
		put_json(_dest.size, jsize);
	}

	D2D1_COLOR_F toColorBase(const char* _htmlColor)
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

	D2D1_COLOR_F toColor(const char* _htmlColor)
	{
		return toColorBase(_htmlColor);
	}


	D2D1_COLOR_F toColor(std::string _htmlColor)
	{
		return toColorBase(_htmlColor.c_str());
	}


	D2D1_COLOR_F toColor(const ccolor& _color)
	{
		return _color;
	}

	enum class visual_alignment
	{
		align_none = 0,
		align_near = 1,
		align_center = 2,
		align_justify = 3,
		align_far = 4,
	};

	void get_json(json& _dest, std::string _member_name, visual_alignment& _src)
	{
		std::string dva = "near";
		switch (_src) {
		case visual_alignment::align_none:
		case visual_alignment::align_near:
			break;
		case visual_alignment::align_center:
			dva = "center";
			break;
		case visual_alignment::align_justify:
			dva = "justify";
			break;
		case visual_alignment::align_far:
			dva = "far";
			break;
		}
		_dest.put_member(_member_name, dva);
	}

	void put_json(visual_alignment& _dest, json& _src, std::string _member_name)
	{
		std::string dva = _src[_member_name];

		if (dva == "near")
		{
			_dest = visual_alignment::align_near;
		}
		else if (dva == "far")
		{
			_dest = visual_alignment::align_far;
		}
		else if (dva == "center")
		{
			_dest = visual_alignment::align_center;
		}
		else if (dva == "justify")
		{
			_dest = visual_alignment::align_justify;
		}
		else
			_dest = visual_alignment::align_near;
	}

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

	void get_json(json& _dest, ccolor & _src)
	{
		_dest.put_member("a", _src.a);
		_dest.put_member("r", _src.r);
		_dest.put_member("g", _src.g);
		_dest.put_member("b", _src.b);
	}

	void put_json(ccolor& _dest, std::string _member_name, json& _src)
	{
		json jcolor = _src[_member_name];
		if (jcolor.is_string()) {
			std::string color_string = jcolor;
			_dest = corona::toColor(color_string);
		}
		else if (jcolor.object())
		{
			if (jcolor.has_member("a"))
				_dest.a = (double)jcolor["a"];
			else
				_dest.a = 1.0;

			_dest.r = (double)jcolor["r"];
			_dest.g = (double)jcolor["g"];
			_dest.b = (double)jcolor["b"];
		}
	}

	void get_json(json& _dest, gradientStop& _src)
	{
		json_parser jp;
		json jcolor = jp.create_object();
		get_json(jcolor, _src.stop_color);
		_dest.put_member("color", jcolor);
		_dest.put_member("position", _src.stop_position);
	}

	void put_json(gradientStop& _dest, json& _src)
	{
		std::vector<std::string> missing;
		if (!_src.has_members(missing, { "color", "position" })) {
			system_monitoring_interface::global_mon->log_warning("gradientStop needs a color and position");
			system_monitoring_interface::global_mon->log_warning("is missing:");
			std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
				system_monitoring_interface::global_mon->log_bus(s);
				});
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		_dest.stop_position = (double)_src["position"];
		put_json(_dest.stop_color, "color", _src);
	}

	/* std::string& _name, std::string& _filename, UINT _destinationWidth = 0, UINT _destinationHeight  = 0 */

	class bitmapRequest
	{
	public:
		std::string	name;
		std::string	file_name;
		DWORD		resource_id;
		bool		cropEnabled;
		rectangle	crop;
		HBITMAP		source;
		std::list<point> sizes;

		bitmapRequest()
		{
			resource_id = {};
			cropEnabled = false;
			crop = {};
			source = nullptr;
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
			source = _request->source;
		}
	};

	void get_json(json& _dest, bitmapRequest& _src)
	{
		json_parser jp;

		_dest.put_member("name", _src.name);
		_dest.put_member("file_name", _src.file_name);
		_dest.put_member("resource_id", _src.resource_id);
		_dest.put_member("crop_enabled", _src.cropEnabled);

		json jcrop = jp.create_object();
		get_json(jcrop, _src.crop);
		_dest.put_member("crop", jcrop);

		json jsizes = jp.create_array();
		for (auto sz : _src.sizes)
		{
			json jcrop = jp.create_object();
			get_json(jcrop, _src.crop);
		}
		_dest.put_member("sizes", jsizes);
	}

	void put_json(bitmapRequest& _dest, json& _src)
	{
		json_parser jp;

		if (!_src.has_members({ "file_name", "crop", "sizes" })) {
			system_monitoring_interface::global_mon->log_warning("bitmap needs a filename, crop, and sizes");
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		_dest.file_name = _src["file_name"];
		_dest.resource_id = (int)_src["resource_id"];
		_dest.cropEnabled = (bool)_src["crop_enabled"];

		json jcrop = _src["crop"];
		put_json(_dest.crop, jcrop);

		json jsizes = _src["sizes"];
		if (jsizes.array()) {
			_dest.sizes.clear();
			int asz = jsizes.size();
			for (int i = 0; i < asz; i++)
			{
				json jsize = jsizes.get_element(i);
				point sz;
				put_json(sz, jsize);
				_dest.sizes.push_back(sz);
			}
		}
	}

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

	void get_json(json& _dest, bitmapBrushRequest& _src)
	{
		_dest.put_member("name", _src.name);
		_dest.put_member("bitmap_name", _src.bitmapName);
	}

	void put_json(bitmapBrushRequest& _dest, json& _src)
	{
		if (!_src.has_members({ "file_name" })) {
			system_monitoring_interface::global_mon->log_warning("bitmap_brush must have file_name");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		_dest.bitmapName = _src["file_name"];
	}

	class linearGradientBrushRequest 
	{
	public:
		std::string name;
		point		start,
					stop;
		point		size;
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

	void get_json(json& _dest, linearGradientBrushRequest& _src)
	{
		json_parser jp;

		_dest.put_member("name", _src.name);
		
		json jstart, jstop, jsize, jstops;

		jstart = jp.create_object();
		get_json(jstart, _src.start);
		_dest.put_member("start", jstart);

		jstop = jp.create_object();
		get_json(jstop, _src.stop);
		_dest.put_member("stop", jstop);

		jsize = jp.create_object();
		get_json(jsize, _src.size);
		_dest.put_member("size", jsize);

		jstops = jp.create_array();
		for (auto st : _src.gradientStops) {
			jstop = jp.create_object();
			get_json(jstop, st);
			jstops.append_element(jstop);
		}
		_dest.put_member("stops", jstops);
	}

	void put_json(linearGradientBrushRequest& _dest, json& _src)
	{
		json_parser jp;

		if (!_src.has_members({ "start", "stop", "stops" })) {
			system_monitoring_interface::global_mon->log_warning("linear_brush must have start, stop and stops");
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		json jstart, jstop, jstops;

		jstart = _src["start"];
		put_json(_dest.start, jstart);

		jstop = _src["stop"];
		put_json(_dest.stop, jstop);

		jstops = _src["stops"];
		if (jstops.array()) {
			int msz = jstops.size();
			_dest.gradientStops.clear();

			for (int i = 0; i < msz; i++)
			{
				json jgs = jstops.get_element(i);
				gradientStop gs;
				put_json(gs, jgs);
				_dest.gradientStops.push_back(gs);
			}
		}
	}


	class radialGradientBrushRequest 
	{
	public:
		std::string name;
		point		center,
					offset;
		float		radiusX,
					radiusY;
		point		size;
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

	void get_json(json& _dest, radialGradientBrushRequest& _src)
	{
		json_parser jp;

		_dest.put_member("name", _src.name);

		json jcenter, joffset, jsize, jstops, jstop;

		jcenter = jp.create_object();
		get_json(jcenter, _src.center);
		_dest.put_member("center", jcenter);

		joffset = jp.create_object();
		get_json(joffset, _src.offset);
		_dest.put_member("offset", joffset);

		_dest.put_member("radiusX", _src.radiusX);
		_dest.put_member("radiusY", _src.radiusY);

		jstops = jp.create_array();
		for (auto st : _src.gradientStops) {
			jstop = jp.create_object();
			get_json(jstop, st);
			jstops.append_element(jstop);
		}
		_dest.put_member("stops", jstops);
	}

	void put_json(radialGradientBrushRequest& _dest, json& _src)
	{
		json_parser jp;

		std::vector<std::string> missing;
		if (!_src.has_members(missing, { "center", "offset", "size", "radiusX", "radiusY", "stops" })) {
			system_monitoring_interface::global_mon->log_warning("radial_brush must have name, center, offset, size, radiusX, radiusY and stops");
			system_monitoring_interface::global_mon->log_warning("is missing:");
			std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
				system_monitoring_interface::global_mon->log_bus(s);
				});
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		json jcenter, joffset, jsize, jstops, jstop;

		jcenter = _src["center"];
		put_json(_dest.center, jcenter);

		joffset = _src["offset"];
		put_json(_dest.offset, joffset);

		jsize = _src["size"];
		put_json(_dest.size, jsize);

		_dest.radiusX = (double)_src["radiusX"];
		_dest.radiusY = (double)_src["radiusY"];

		jstops = _src["stops"];
		if (jstops.array()) {
			int msz = jstops.size();
			_dest.gradientStops.clear();

			for (int i = 0; i < msz; i++)
			{
				json jgs = jstops.get_element(i);
				gradientStop gs;
				put_json(gs, jgs);
				_dest.gradientStops.push_back(gs);
			}
		}
	}

	class solidBrushRequest
	{
	public:
		std::string name;
		ccolor brushColor;

		solidBrushRequest() = default;
		solidBrushRequest(const solidBrushRequest& _request) = default;
		solidBrushRequest(solidBrushRequest&& _request) = default;
		solidBrushRequest& operator = (const solidBrushRequest& _request) = default;
		solidBrushRequest& operator = (solidBrushRequest&& _request) = default;

		solidBrushRequest(std::shared_ptr<solidBrushRequest> _request)
		{
			name = _request->name;
			brushColor = _request->brushColor;
		}
	};

	void get_json(json& _dest, solidBrushRequest& _src)
	{
		json_parser jp;
		_dest.put_member("name", _src.name);

		json jcolor = jp.create_object();
		get_json(_dest, _src.brushColor);
		_dest.put_member("color", jcolor);
	}

	void put_json(solidBrushRequest& _dest, json& _src)
	{
		json_parser jp;

		if (!_src.has_members({ "color" })) {
			system_monitoring_interface::global_mon->log_warning("solid_brush must have color");
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}

		put_json(_dest.brushColor, "color", _src);
	}

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

		bool has_brush()
		{
			return solid_brush || radial_brush || linear_brush || bitmap_brush;
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

		generalBrushRequest(std::string name, std::string color)
		{
			clear();
			brush_type = brush_types::solid_brush_type;
			solid_brush = std::make_shared<solidBrushRequest>();
			set_name(name);
			setColor(color);
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
			return *this;
		}

		void setColor(std::string _color)
		{
			solidBrushRequest sbr;
			sbr.brushColor = toColor(_color);
			if (this->get_name()) {
				sbr.name = this->get_name();
			}
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

		void apply_scale(point _size)
		{
			switch (brush_type) {
			case brush_types::solid_brush_type:
				break;
			case brush_types::linear_brush_type:
				if (linear_brush) {
					linear_brush->size *= _size;
					linear_brush->start *= _size;
					linear_brush->stop *= _size;
				}
				break;
			case brush_types::radial_brush_type:
				if (radial_brush) {
					radial_brush->center *= _size;
					radial_brush->radiusX *= _size.x;
					radial_brush->radiusY *= _size.y;
				}
				break;
			case brush_types::bitmap_brush_type:
				break;
			}
		}


	};

	void get_json(json& _dest, generalBrushRequest& _src)
	{
		json_parser jp;

		switch (_src.brush_type) {
		case brush_types::solid_brush_type:
			_dest.put_member("class_name", "solid_brush");
			get_json(_dest, *_src.solid_brush);
			break;
		case brush_types::linear_brush_type:
			_dest.put_member("class_name", "linear_brush");
			get_json(_dest, *_src.linear_brush);
			break;
		case brush_types::radial_brush_type:
			_dest.put_member("class_name", "radial_brush");
			get_json(_dest, *_src.radial_brush);
			break;
		case brush_types::bitmap_brush_type:
			_dest.put_member("class_name", "bitmap_brush");
			get_json(_dest, *_src.bitmap_brush);
			break;
		}
	}


	void put_json(generalBrushRequest& _dest, json& _src)
	{
		json_parser jp;

		std::string jtype = _src.get_member("class_name");
		if (jtype == "solid_brush")
		{
			solidBrushRequest sbr;
			put_json(sbr, _src);
			_dest = sbr;
		}
		else if (jtype == "linear_brush")
		{
			linearGradientBrushRequest lgbr;
			put_json(lgbr, _src);
			_dest = lgbr;
		}
		else if (jtype == "radial_brush")
		{
			radialGradientBrushRequest rgbr;
			put_json(rgbr, _src);
			_dest = rgbr;
		}
		else if (jtype == "bitmap_brush")
		{
			radialGradientBrushRequest rgbr;
			put_json(rgbr, _src);
			_dest = rgbr;
		}
		else
			_dest.clear();
	}

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

	void get_json(json& _dest, textStyleRequest& _src)
	{
		json_parser jp;

		_dest.put_member("name", _src.name);
		_dest.put_member("font_name", _src.fontName);
		_dest.put_member("font_size", _src.fontSize);
		_dest.put_member("bold", _src.bold);
		_dest.put_member("italics", _src.italics);
		_dest.put_member("underline", _src.underline);
		_dest.put_member("strike_through", _src.strike_through);
		_dest.put_member("line_spacing", _src.line_spacing);
		_dest.put_member("wrap_text", _src.wrap_text);
		_dest.put_member("character_spacing", _src.character_spacing);

		switch (_src.font_stretch) {
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_CONDENSED:
			_dest.put_member("font_stretch", "condensed");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXPANDED:
			_dest.put_member("font_stretch", "expanded");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXTRA_CONDENSED:
			_dest.put_member("font_stretch", "extra_condensed");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXTRA_EXPANDED:
			_dest.put_member("font_stretch", "extra_expanded");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL:
			_dest.put_member("font_stretch", "normal");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_SEMI_CONDENSED:
			_dest.put_member("font_stretch", "semi_condensed");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_SEMI_EXPANDED:
			_dest.put_member("font_stretch", "semi_expanded");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_ULTRA_CONDENSED:
			_dest.put_member("font_stretch", "ultra_condensed");
			break;
		case DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_ULTRA_EXPANDED:
			_dest.put_member("font_stretch", "ultra_expanded");
			break;
		default:
			_dest.put_member("font_stretch", "normal");
			break;
		}

		_dest.put_member("font_stretch", _src.font_stretch);
		get_json( _dest, "horizontal_alignment", _src.horizontal_align);
		get_json(_dest, "vertical_alignment", _src.vertical_align);
	}

	void put_json(textStyleRequest& _dest, json& _src)
	{

		if (!_src.has_members({ "font_name", "font_size" })) {
			system_monitoring_interface::global_mon->log_warning("text style must have font_name, and font_size.");
			system_monitoring_interface::global_mon->log_bus("text style may also have bold, italics, underline, strike_through, line_spacing, wrap_text, character_spacing, font_stretch." );
			system_monitoring_interface::global_mon->log_bus("source json:");
			system_monitoring_interface::global_mon->log_json(_src, 2);
			return;
		}
		_dest.fontName = _src["font_name"];
		_dest.fontSize = (double)_src["font_size"];
		_dest.bold = (bool)_src["bold"];
		_dest.italics = (bool)_src["italics"];
		_dest.underline = (bool)_src["underline"];
		_dest.strike_through = (bool)_src["strike_through"];
		_dest.line_spacing =  (double)_src["line_spacing"];
		_dest.wrap_text = (bool)_src["wrap_text"];
		_dest.character_spacing = (double)_src["character_spacing"];
		std::string stretch = _src["font_stretch"];

		if (stretch == "condensed")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_CONDENSED;
		else if (stretch == "expanded")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXPANDED;
		else if (stretch == "extra_condensed")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
		else if (stretch == "extra_expanded")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
		else if (stretch == "normal")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL;
		else if (stretch == "semi_condensed")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_SEMI_CONDENSED;
		else if (stretch == "semi_expanded")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_SEMI_EXPANDED;
		else if (stretch == "ultra_condensed")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
		else if (stretch == "ultra_expanded")
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
		else
			_dest.font_stretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL;

		put_json(_dest.horizontal_align, _src, "horizontal_alignment");
		put_json(_dest.vertical_align, _src, "vertical_alignment");
	}

	struct viewStyleRequest
	{
		std::string			name;
		textStyleRequest	text_style;
		double				shape_border_thickness;
		double				box_border_thickness;
		generalBrushRequest box_border_brush;
		generalBrushRequest shape_border_brush;
		generalBrushRequest box_fill_brush;
		generalBrushRequest shape_fill_brush;

		void set_default_name(std::string _name)
		{
			name = _name;
			text_style.name = "tx_" + _name;
			box_border_brush.set_name("bb_" + _name);
			shape_border_brush.set_name("sb_" + _name);
			box_fill_brush.set_name("bf_" + _name);
			shape_fill_brush.set_name("sf_" + _name);
		}

		void apply_scale(point _size)
		{
			box_border_brush.apply_scale( _size);
			shape_border_brush.apply_scale(_size);
			box_fill_brush.apply_scale(_size);
			shape_fill_brush.apply_scale(_size);
		}
	};

	void get_json(json& _dest, viewStyleRequest& _src)
	{
		json_parser jp;
		json text_style = jp.create_object();
		json box_border = jp.create_object();
		json shape_border = jp.create_object();
		json box_fill = jp.create_object();
		json shape_fill = jp.create_object();

		get_json(text_style, _src.text_style);
		_dest.put_member("text_style", text_style);

		if (_src.box_border_brush.has_brush()) {
			get_json(box_border, _src.box_border_brush);
			_dest.put_member("box_border_brush", box_border);
		}
		if (_src.shape_border_brush.has_brush()) {
			get_json(shape_border, _src.shape_border_brush);
			_dest.put_member("shape_border_brush", shape_border);
		}
		if (_src.box_fill_brush.has_brush()) {
			get_json(box_fill, _src.box_fill_brush);
			_dest.put_member("box_fill_brush", box_fill);
		}
		if (_src.shape_fill_brush.has_brush()) {
			get_json(shape_fill, _src.shape_fill_brush);
			_dest.put_member("shape_fill_brush", shape_fill);
		}

		_dest.put_member("name", _src.name);
		_dest.put_member("shape_border_thickness", _src.shape_border_thickness);
		_dest.put_member("box_border_thickness", _src.box_border_thickness);

	}

	void put_json(viewStyleRequest& _dest, json& _src)
	{

		json text_style = _src["text_style"];
		json box_border = _src["box_border_brush"];
		json shape_border = _src["shape_border_brush"];
		json box_fill = _src["box_fill_brush"];
		json shape_fill = _src["shape_fill_brush"];

		_dest.shape_border_thickness = (double)_src["shape_border_thickness"];
		_dest.box_border_thickness = (double)_src["box_border_thickness"];

		if (text_style.object()) {
			put_json(_dest.text_style, text_style);
		}
		put_json(_dest.box_border_brush, box_border);
		put_json(_dest.shape_border_brush, shape_border);
		put_json(_dest.box_fill_brush, box_fill);
		put_json(_dest.shape_fill_brush, shape_fill);

		_dest.name = (std::string)_src["name"];
		_dest.set_default_name(_dest.name);
	}

	void put_json(std::shared_ptr<viewStyleRequest>& _dest, json& _src)
	{
		if (_dest) {
			put_json(*_dest, _src);
		}
	}

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

	D2D1_GRADIENT_STOP toGradientStop(const gradientStop& _gradientStop)
	{
		D2D1_GRADIENT_STOP stop;

		stop.position = _gradientStop.stop_position;
		stop.color = toColor(_gradientStop.stop_color);
		return stop;
	}

}

#endif
