/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
Implements motion detection for the camera control.

Notes

For Future Consideration
*/

#pragma once

namespace corona
{

	struct argb32_pixel 
	{
		unsigned char a, r, g, b;

		argb32_pixel() : a(0), r(0), g(0), b(0) { };
	};

	struct color_hash_count
	{
		int hash_color;
		int hash_count;

		color_hash_count() : hash_color(0), hash_count(0) { ; }
	};

	struct bgra32_pixel
	{
		unsigned char b, g, r, a;

		bgra32_pixel() : b(0), g(0), r(0), a(0) { };
	};

	class sprinkle
	{
	public:
		bgra32_pixel pixel_start;
		bgra32_pixel pixel_end;
		point current;
		point velocity;
		point acceleration;
		int	  life_remaining;
	};

	using sprinkle_buffer = ring_buffer<sprinkle, 4>;

	struct signal_pixel
	{
		float			signal;
		int				detected;
		bool			activated;
		float			detect_counter;
		float			dh, dl, ds;

		signal_pixel() : signal(0.0), detected(0), activated(false)
		{
			;
		}
	};

	class movement_box
	{
	public:
		std::string image_hash;
		rectangle	area;
		std::vector<point> left;
		std::vector<point> right;
	};

	class movement_box_instance
	{
	public:

		movement_box	 box;
		int64_t			 frame_last_detected;
		int64_t			 frame_first_detected;

		movement_box_instance()
		{
			;
		}
	};

	point hsl_to_point(hsl in)
	{
		double ar = in.h * 6.28;
		double r = in.s;

		point pt;
		pt.z = in.l;
		pt.x = cos(ar) * r;
		pt.y = sin(ar) * r;

		return pt;
	}

	double hsl_distance(hsl h1, hsl h2)
	{
		point p1 = hsl_to_point(h1);
		point p2 = hsl_to_point(h2);
		double distance = point_math::distance(p1, p2);
		return distance;
	}

	template <typename pixel_type> hsl to_hsl(pixel_type _pt)
	{
		rgb rx;

		rx.r = _pt.r / 255.0;
		rx.g = _pt.g / 255.0;
		rx.b = _pt.b / 255.0;

		return rgb2hsl(rx);
	}

	void calculate_hsl_ranges()
	{
		rgb r;
		hsl mins = {};
		hsl maxes = {};

		for (r.r = 0; r.r < 255; r.r += 16)
		{
			for (r.g = 0; r.g < 255; r.g += 16)
			{
				for (r.b = 0; r.b < 255; r.b += 16)
				{
					hsl t = to_hsl(r);

					if (t.h < mins.h)
						mins.h = t.h;
					if (t.s < mins.s)
						mins.s = t.s;
					if (t.l < mins.l)
						mins.l = t.l;

					if (t.h >= maxes.h)
						maxes.h = t.h;
					if (t.s >= maxes.s)
						maxes.s = t.s;
					if (t.l >= maxes.l)
						maxes.l = t.l;

				}
			}
		}

		std::cout << std::format("hsl ranges (h:{0},s:{1},l:{2})-(h:{3},s:{4},l:{5}",
			mins.h, mins.s, mins.l, maxes.h, maxes.s, maxes.l) << std::endl;
	}

	template <typename pixel_type> class pixel_frame
	{
		unsigned int width, height, size_pixels;
		std::vector<pixel_type> pixels;

	public:

		friend class cursor;
		comm_bus_interface* bus;

		pixel_frame() : width(0), height(0),  size_pixels(0), bus(nullptr)
		{
			
		}

		pixel_frame(unsigned int _width, unsigned int _height)
			: width(_width), height(_height), size_pixels(_height * _width)
		{
			pixels.resize(size_pixels);
			for (int i = 0; i < size_pixels; i++) {
				pixels[i] = {};
			}
		}

		pixel_frame(comm_bus_interface* _bus, pixel_type *_src, unsigned int _width, unsigned int _height)
			: bus(_bus), width(_width), height(_height), size_pixels(_height* _width)
		{
			pixels.resize(size_pixels);
			for (int i = 0; i < size_pixels; i++) {
				pixels[i] = *_src;
				_src++;
			}
		}

		pixel_frame(pixel_frame&& _src)
			: bus(_src.bus), width(_src.width), height(_src.height), size_pixels(_src.size_pixels)
		{
			pixels = std::move(_src.pixels);
		}

		bool is_same_size(pixel_frame& _src)
		{
			if (_src.width == width and _src.height == height)
				return true;
			return false;
		}

		pixel_frame& operator = (pixel_frame&& _src)
		{
			width = _src.width;
			height = _src.height;
			size_pixels = _src.size_pixels;
			pixels = std::move(_src.pixels);
			bus = _src.bus;
			return *this;
		}

		pixel_frame(const pixel_frame& _src)
			: width(_src.width), height(_src.height), size_pixels(_src.size_pixels)
		{
			pixels = _src.pixels;
			bus = _src.bus;
		}

		pixel_frame& operator = (const pixel_frame& _src)
		{
			width = _src.width;
			height = _src.height;
			size_pixels = _src.size_pixels;
			pixels = _src.pixels;
			bus = _src.bus;
			return *this;
		}

		point get_scale()
		{
			point pt;
			pt.x = 1.0 / width;
			pt.y = 1.0 / height;
			return pt;
		}

		int get_area()
		{
			return size_pixels;
		}

		int get_width()
		{
			return width;
		}

		int get_height()
		{
			return height;
		}

		inline bool empty()
		{
			return size_pixels == 0;
		}

		class cursor
		{
		private:

			pixel_frame* pf;
			int cur_pos;

			cursor() :
				pf(nullptr), cur_pos(0)
			{

			}

			cursor(pixel_frame* _pf) : 
				pf(_pf), cur_pos(0)
			{

			}

			cursor(pixel_frame* _pf, measure x, measure y) :
				pf(_pf)
			{

				unsigned int pixel_x;
				unsigned int pixel_y;

				if (x.units == measure_units::percent_container)
				{
					pixel_x = x.amount * pf->get_width();
				}
				else if (x.units == measure_units::pixels)
				{
					pixel_x = x.amount;
				}

				if (y.units == measure_units::percent_container)
				{
					pixel_y = y.amount * pf->get_height();
				}
				else if (y.units == measure_units::pixels)
				{
					pixel_y = y.amount;
				}

				cur_pos = pixel_y * width + pixel_x;

				if (cur_pos > pixels.size())
				{
					cur_pos = pixels.size() - 1;
				}
			}

			cursor(pixel_frame* _pf, int x, int y) :
				pf(_pf)
			{
				unsigned int pixel_x = x;
				unsigned int pixel_y = y;

				cur_pos = pixel_y * pf->get_width() + pixel_x;

				if (cur_pos > pf->get_area())
				{
					cur_pos = pf->get_area() - 1;
				}
			}

		public:

			friend class pixel_frame;

			inline pixel_type* first()
			{
				cur_pos = 0;
				if (cur_pos < 0 or cur_pos >= pf->pixels.size())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* down()
			{
				cur_pos += pf->get_width();
				if (cur_pos < 0 or cur_pos >= pf->pixels.size())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* up()
			{
				cur_pos += pf->get_width();
				if (cur_pos < 0 or cur_pos >= pf->pixels.size())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* right( int _start_x = 0 )
			{
				cur_pos++;
				if ((cur_pos % pf->get_width()) == 0)
				{
					cur_pos += _start_x;
				}
				if (cur_pos < 0 or cur_pos >= pf->get_area())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* carriage_return(int _start_x = 0)
			{
				cur_pos -= get_x();
				cur_pos += pf->get_width();
				cur_pos += _start_x;
				if (cur_pos >= pf->get_area())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* get()
			{
				if (cur_pos < 0 or cur_pos >= pf->get_area())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline int get_x()
			{
				return cur_pos % pf->get_width();
			}

			inline int get_y()
			{
				return cur_pos / pf->get_width();
			}

			inline pixel_type *set(int _new_x, int _new_y)
			{
				cur_pos = _new_y * pf->get_width() + _new_x;
				return get();
			}

			inline void set_x(int _new_x)
			{
				int x = get_x();
				cur_pos -= x;
				cur_pos += _new_x;
			}

			inline void set_y(int _new_y)
			{
				int y = get_y();
				cur_pos -= y * pf->get_width();
				cur_pos += _new_y * pf->get_width();
			}

		};

		cursor get_cursor_scaled(double px, double py)
		{
			if (empty()) {
				cursor pt = {};
				return pt;
			}

			return cursor(this, px, py);
		};

		cursor get_cursor(unsigned int pixel_x, unsigned int pixel_y)
		{
			if (empty()) {
				cursor pt;
				return pt;
			}

			return cursor(this, pixel_x, pixel_y);
		}

		void for_each(std::function<pixel_type(int x, int y, pixel_type)> _xform)
		{
			if (bus) {
				bus->run_each<pixel_type>(pixels, get_width(), get_height(), [_xform](int x, int y, pixel_type& px) -> void {
					pixel_type np = _xform(x, y, px);
					px = np;
					});
			}
		}

		void for_each(std::function<pixel_type (pixel_type)> _xform)
		{
			if (bus) {
				bus->run_each<pixel_type>(pixels, [_xform](pixel_type& px) -> void {
					pixel_type np = _xform(px);
					px = np;
					});
			}
		}

		ID2D1Bitmap1* get_bitmap(ID2D1DeviceContext *_context, std::function<bgra32_pixel(int x, int y, pixel_type)> _xform)
		{
			ID2D1Bitmap1* new_bitmap = nullptr;

			D2D1_SIZE_U size = {};
			size.width = width;
			size.height = height;

			if (not size_pixels)
				return nullptr;

			bgra32_pixel* data = new bgra32_pixel[size_pixels];

			if (data) {

				bus->run_each<bgra32_pixel, pixel_type>(data, pixels, [data, size, _xform](bgra32_pixel* _target, pixel_type& _src) -> void {
					int pos = _target - data;
					int x = pos % size.width;
					int y = pos / size.width;
					*_target = _xform(x, y, _src);
					});

				D2D1_BITMAP_PROPERTIES1 props = {};
				props.dpiX = 96;
				props.dpiY = 96;
				props.pixelFormat.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
				props.pixelFormat.alphaMode = D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_PREMULTIPLIED;
				props.bitmapOptions = D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_TARGET;
				_context->CreateBitmap(size, data, sizeof(bgra32_pixel) * width, props, &new_bitmap);

				delete[] data;
			}

			return new_bitmap;
		}
	};

	class sample_point
	{
	public:
		LONG	point;
		LONG	count;
		LONG	length;

		sample_point() : point(0), count(0), length(0)
		{
			;
		}

		sample_point(LONG _point, LONG _count, LONG _length) : point(_point), count(_count), length(_length)
		{
			;
		}
	};

	class samples
	{
		std::vector<sample_point> sample_points;

	public:

		samples()
		{
			;
		}

		virtual ~samples()
		{
			;
		}

		void start(int _size)
		{
			sample_points.clear();
			sample_points.resize(_size);
		}

		void count(int _point)
		{
			sample_point& pt = sample_points[_point];
			::InterlockedIncrement(&pt.count);
			pt.point = _point;
			pt.length = 1;
		}

		std::vector<sample_point> get_points()
		{
			std::vector<sample_point> points;

			auto start_span = sample_points.begin();
			while (start_span != sample_points.end())
			{
				int total = 0;
				int count = 0;
				auto next_span = start_span;
				int distance = 1;

				while (next_span->count)
				{
					total += next_span->count;
					count++;
					next_span++;
					if (next_span == sample_points.end())
						break;
				}

				if (total) {
					sample_point ptnew;
					ptnew.count = total;
					ptnew.length = count;
					ptnew.point = start_span->point;
					points.push_back(ptnew);
				}
				else
				{
					next_span++;
				}
				start_span = next_span;
			}

			return points;
		}

		static void test()
		{
			samples sc;
			sc.start(20);

			sc.count(1);
			sc.count(3);
			sc.count(4);
			sc.count(4);
			sc.count(4);
			sc.count(5);
			sc.count(10);
			sc.count(11);
			sc.count(12);
			sc.count(13);
			sc.count(14);
			sc.count(15);
			sc.count(17);

			auto pt = sc.get_points();
			
			std::vector<sample_point> check_points = {
				{ 1, 1, 1 },
				{ 3, 5, 3 },
				{ 10, 6, 6 },
				{ 17, 1, 1 }
			};

			int isx = std::min(pt.size(), check_points.size());

			if (isx < check_points.size())
			{
				std::cout << __LINE__ << " check points size incorrect" << std::endl;
			}

			for (int i = 0; i < pt.size(); i++)
			{
				if (pt[i].count != check_points[i].count) {
					std::cout << __LINE__ << " check point count incorrect" << std::endl;
				}
				if (pt[i].point != check_points[i].point) {
					std::cout << __LINE__ << " check point point incorrect" << std::endl;
				}
				if (pt[i].length != check_points[i].length) {
					std::cout << __LINE__ << " check point length incorrect" << std::endl;
				}

				rgb sample_pixel;
				sample_pixel.r = 255;
				sample_pixel.g = 255;
				sample_pixel.b = 255;

				hsl sph = to_hsl(sample_pixel);
				std::cout << __LINE__ << " hsl check: h:" << sph.h << " l:" << sph.l << " s:" << sph.s <<  std::endl;

				rgb sp2 = hsl2rgb(sph);
				sp2.r *= 255;
				sp2.g *= 255;
				sp2.b *= 255;
				if (sp2.b != sample_pixel.b ||
					sp2.g != sample_pixel.g ||
					sp2.r != sample_pixel.r) {
					std::cout << __LINE__ << " hsl round trip fails" << std::endl;
				}

				rgb wall1;
				wall1.r = 122;
				wall1.g = 128;
				wall1.b = 141;
				hsl hwall1 = to_hsl(wall1);

				rgb wall2;
				wall2.r = 164;
				wall2.g = 175;
				wall2.b = 180;
				hsl hwall2 = to_hsl(wall2);

				rgb hand1;
				hand1.r = 205;
				hand1.g = 142;
				hand1.b = 124;
				hsl hhand1 = to_hsl(hand1);

				rgb hand2;
				hand2.r = 205;
				hand2.g = 142;
				hand2.b = 124;
				hsl hhand2 = to_hsl(hand2);

				rgb hand3;
				hand3.r = 225;
				hand3.g = 179;
				hand3.b = 169;
				hsl hhand3 = to_hsl(hand3);

				double d = hsl_distance(hhand1, hwall1);
				std::cout << "d h1 w1:" << d << std::endl;
				d = hsl_distance(hhand1, hwall2);
				std::cout << "d h1 w2:" << d << std::endl;
				d = hsl_distance(hhand2, hwall1);
				std::cout << "d h2 w1:" << d << std::endl;
				d = hsl_distance(hhand2, hwall2);
				std::cout << "d h2 w2:" << d << std::endl;
				d = hsl_distance(hhand3, hwall1);
				std::cout << "d h3 w1:" << d << std::endl;
				d = hsl_distance(hhand3, hwall2);
				std::cout << "d h3 w2:" << d << std::endl;
				d = hsl_distance(hhand1, hhand2);
				std::cout << "d h1 h2:" << d << std::endl;
				d = hsl_distance(hhand1, hhand3);
				std::cout << "d h1 h3:" << d << std::endl;
				d = hsl_distance(hhand2, hhand3);
				std::cout << "d h2 h3:" << d << std::endl;

				calculate_hsl_ranges();
			}

		}
	};

	struct delta_frame
	{
	public:

		comm_bus_interface* bus;
		timer	frame_timer;
		int		frame_counter;
		double	last_frame_seconds;
		double	total_frame_seconds;

		ring_buffer<ccolor, 256> colors;
		timer	color_timer;
		int		color_counter;
		double	last_color_seconds;
		double	total_color_seconds;
		double	total_cycle_seconds;


		pixel_frame<signal_pixel> activation_frame;
		pixel_frame<bgra32_pixel> last_frame;

		double detection_threshold;
		double activation_area_percentage;
		double detection_pulse;
		double detection_cooldown;

		int color_cycle_frames;
		ccolor color_cycle_start, 
			color_cycle_step;

		std::mt19937 random_engine;
		std::uniform_real_distribution<double> color_distribution;
		std::uniform_real_distribution<double> color_cycle_distribution;

		std::vector<movement_box> boxes;
		samples active_columns;
		samples active_rows;

		delta_frame() : activation_frame(256, 256), 
			color_distribution(.0, 1.0),
			color_cycle_distribution(0.0, .1),
			total_frame_seconds(0),
			last_frame_seconds(0),
			frame_counter(0)
		{
			reset_defaults();
		}

		void set_bus(comm_bus_interface* _bus)
		{
			bus = _bus;
			activation_frame.bus = _bus;
			last_frame.bus = _bus;
		}

		void reset_defaults()
		{
			active_columns.start(1024);
			active_rows.start(1024);

			detection_threshold = .3;
			activation_area_percentage = .5;
			detection_pulse = .8;
			detection_cooldown = .08;
			color_cycle_start = {};
			init_color_cycle();
			next_color_cycle();
		}

		lockable activation_locker;

		ID2D1Bitmap1* get_activation(ID2D1DeviceContext* _context, sprinkle_buffer& _sprinkles)
		{

			ID2D1Bitmap1* new_bitmap;


			new_bitmap = activation_frame.get_bitmap(_context, [this, &_sprinkles](int x, int y, signal_pixel src)->bgra32_pixel {

				bgra32_pixel temp = {};
				ccolor root_color;

				if (colors.get_size() > 0) {
					root_color = colors.get(src.detect_counter + color_counter);
				}
				else {
					root_color.r = 0;
					root_color.g = 0;
					root_color.b = .5;
				}

				double alpha = src.signal;

				if (alpha < .0) {
					alpha = .0;
				}
				else if (alpha > 1) {
					alpha = 1.0;
				}

				temp.a = 255.0 * alpha;
				temp.b = root_color.b * temp.a;
				temp.g = root_color.g * temp.a;
				temp.r = root_color.r * temp.a;

				return temp;
			});

			return new_bitmap;
		}

		ID2D1Bitmap1* get_frame(ID2D1DeviceContext* _context)
		{
			ID2D1Bitmap1* new_bitmap;
			new_bitmap = last_frame.get_bitmap(_context, [](int x, int y, bgra32_pixel src)->bgra32_pixel {
				return src;
			});

			return new_bitmap;
		}


		std::vector<movement_box> get_movement_boxes()
		{
			return boxes;
		}

		void calculate_movement_boxes()
		{
			boxes = get_frame_movement_boxes();

/*			std::vector<movement_box> new_boxes;

			for (auto b : boxes) 
			{
				auto found = std::find_if(frame_boxes.begin(), frame_boxes.end(), [b](movement_box& r) {
					return rectangle_math::intersect(&b.area, &r.area);
					});

				if (found == std::end(frame_boxes))
				{
					new_boxes.push_back(b);
				}
			}

			for (auto bx : frame_boxes)
			{
				new_boxes.push_back(bx);
			}

			boxes = std::move(new_boxes);
			*/

		}

		// there is a way, way better way to do this.
		// use the thing that recursively divides it into boxes.
		// 
		std::vector<movement_box> get_frame_movement_boxes()
		{
			std::vector<movement_box> new_boxes;

			point scale = activation_frame.get_scale();

			scale.x *= last_frame.get_width();
			scale.y *= last_frame.get_height();

			auto box_ys = active_rows.get_points();
			auto box_xs = active_columns.get_points();

			for (auto box_y : box_ys)
			{
				for (auto box_x : box_xs)
				{
					std::list <point> left_polygon;
					std::list <point> right_polygon;

					movement_box mb;

					mb.area.x = box_x.point;
					mb.area.y = box_y.point;
					mb.area.w = box_x.length;
					mb.area.h = box_y.length;

					int active_count = 0;
					auto mbcursor = activation_frame.get_cursor(mb.area.x, mb.area.y);
					signal_pixel* spmb = nullptr;

					for (int y = 0; y < mb.area.h; y++)
					{
						int first_active_x = -1;
						int last_active_x = -1;
						signal_pixel* xfound = mbcursor.set(mb.area.x + mb.area.w / 2, mb.area.y + y);

						spmb = mbcursor.set(mb.area.x, mb.area.y + y);

						for (int x = 0; x < mb.area.w; x++)
						{
							if (spmb and spmb->activated)
							{
								active_count++;
								if (first_active_x < 0) {
									first_active_x = x;
								}
								auto dh = std::abs(spmb->dh - xfound->dh);
								auto dl = std::abs(spmb->dl - xfound->dl);
								auto ds = std::abs(spmb->ds - xfound->ds);
								if (xfound and dh < .05) {
									last_active_x = x;
								}
							}
							spmb = mbcursor.right();
						}

						if (first_active_x > -1)
						{
							point pt;
							pt.x = first_active_x;
							pt.y = y;
							left_polygon.push_back(pt);
							if (last_active_x >= pt.x) {
								pt.x = last_active_x;
								pt.y = y;
								right_polygon.push_back(pt);
							}
						}
					}

					if (active_count)
					{
						mb.area.x *= scale.x;
						mb.area.y *= scale.y;
						mb.area.w *= scale.x;
						mb.area.h *= scale.y;

						for (auto& pt : left_polygon)
						{
							pt.x *= scale.x;
							pt.y *= scale.y;
							mb.left.push_back(pt);
						}

						for (auto& pt : right_polygon)
						{
							pt.x *= scale.x;
							pt.y *= scale.y;
							mb.right.push_back(pt);
						}

						new_boxes.push_back(mb);
					}

				}
			}

			std::sort(new_boxes.begin(), new_boxes.end(), [](movement_box& a, movement_box& b) {
				return a.area.h > b.area.h;
				});

			return new_boxes;
		}

		std::vector<ID2D1Bitmap1*> get_motion(ID2D1DeviceContext* _context)
		{
			std::vector<ID2D1Bitmap1*> bitmaps;
			return bitmaps;
		}

		void next_frame(bgra32_pixel* _src, int _width, int _height, int _stride)
		{
			pixel_frame<bgra32_pixel> new_frame(bus, _src, _width, _height);
			next_frame(new_frame);
		}

		

		void init_color_cycle()
		{
			double c = colors.get_capacity();

			color_cycle_start.r = color_distribution(random_engine);
			color_cycle_start.g = color_distribution(random_engine);
			color_cycle_start.b = color_distribution(random_engine);

			color_cycle_step.r = color_cycle_distribution(random_engine);
			color_cycle_step.g = color_cycle_distribution(random_engine);
			color_cycle_step.b = color_cycle_distribution(random_engine);

/*			color_cycle_start.r = 0;
			color_cycle_start.g = 0;
			color_cycle_start.b = 0;

			color_cycle_step.r = 0;
			color_cycle_step.g = 1.0 / c;
			color_cycle_step.b = 0;
			*/
			for (int i = 0; i < colors.get_capacity()/2; i++)
			{
				colors.put(color_cycle_start);
				color_cycle_start.r += color_cycle_step.r;
				if (color_cycle_start.r > 1.0) color_cycle_start.r = 0;
				color_cycle_start.g += color_cycle_step.g;
				if (color_cycle_start.g > 1.0) color_cycle_start.g = 0;
				color_cycle_start.b += color_cycle_step.b;
				if (color_cycle_start.b > 1.0) color_cycle_start.b = 0;
			}
		}

		void next_color_cycle()
		{

			if (total_color_seconds > 3 or color_counter == 0) {
				total_color_seconds = 0.0;
				init_color_cycle();
			}

			if (total_cycle_seconds > 1/16.0) {
				total_cycle_seconds = 0.0;
				color_counter++;
			}

			last_color_seconds = color_timer.get_elapsed_seconds();
			total_color_seconds += last_color_seconds;
			total_cycle_seconds += last_color_seconds;
		}

		void next_frame(pixel_frame<bgra32_pixel>& _frame1)
		{		
			frame_counter++;
			last_frame_seconds = frame_timer.get_elapsed_seconds();
			total_frame_seconds += last_frame_seconds;

			next_color_cycle();

			active_columns.start(activation_frame.get_width());
			active_rows.start(activation_frame.get_height());

			activation_frame.for_each([this](signal_pixel _src) -> signal_pixel {
				if (_src.signal > 0.0)
				{
					_src.signal -= detection_cooldown;
					if (_src.signal < 0.0)
						_src.signal = 0.0;
				}
				_src.dh = 0.0;
				_src.dl = 0.0;
				_src.ds = 0.0;
				_src.detected = 0;
				_src.activated = false;
				return _src;
				});

			if (last_frame.empty() or !last_frame.is_same_size(_frame1)) {
				last_frame = _frame1;
				return;
			}


			long long m_pixel_count = 0;
			long long a_pixel_count = 0;

			auto frame0_cursor = last_frame.get_cursor(0, 0);
			auto frame1_cursor = _frame1.get_cursor(0, 0);
			auto activation_cursor = activation_frame.get_cursor(0, 0);

			auto sp0 = frame0_cursor.first();
			auto sp1 = frame1_cursor.first();

			while (sp0 and sp1)
			{

				hsl hsl0 = to_hsl(*sp0);
				hsl hsl1 = to_hsl(*sp1);

				int activation_x = (frame0_cursor.get_x() * activation_frame.get_width()) / last_frame.get_width();
				int activation_y = (frame0_cursor.get_y() * activation_frame.get_height()) / last_frame.get_height();
				activation_cursor.set(activation_x, activation_y);

				auto spa = activation_cursor.get();

				double d = hsl_distance(hsl0, hsl1);

				if (d > detection_threshold)
				{
					m_pixel_count++;

					if (not spa->detected) {
						spa->signal += detection_pulse;
						spa->detect_counter = frame_counter;
					}

					spa->dl += hsl1.l;
					spa->dh += hsl1.h;
					spa->ds += hsl1.s;
					spa->detected++;
					
				}
				else if (spa->signal > 0.0)
				{
					spa->dl += hsl1.l;
					spa->dh += hsl1.h;
					spa->ds += hsl1.s;
					spa->detected++;
				}
				else
				{
					a_pixel_count++;
				}

				sp0 = frame0_cursor.right();
				sp1 = frame1_cursor.right();

			}

			double frame_area = (double)_frame1.get_area() / (double)activation_frame.get_area();

			activation_frame.for_each([this, frame_area](int x, int y, signal_pixel _src) -> signal_pixel {
				double d = _src.detected / frame_area;
				if (_src.detected) {
					_src.dl /= _src.detected;
					_src.dh /= _src.detected;
					_src.ds /= _src.detected;
				}
				if (d > this->activation_area_percentage) {
					_src.activated = true;
					active_columns.count(x);
					active_rows.count(y);
				}
				return _src;
				});

			last_frame = _frame1;
			calculate_movement_boxes();
		}
	};

}
