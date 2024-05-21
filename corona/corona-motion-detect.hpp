#pragma once

namespace corona
{
	struct argb32_pixel 
	{
		unsigned char a, r, g, b;

		argb32_pixel() : a(0), r(0), g(0), b(0) { };
	};

	struct bgra32_pixel
	{
		unsigned char b, g, r, a;

		bgra32_pixel() : b(0), g(0), r(0), a(0) { };
	};

	struct signal_pixel
	{
		double signal;
		int	   detected;
		bool   activated;

		signal_pixel() : signal(0.0), detected(0), activated(false)
		{
			;
		}
	};

	template <typename pixel_type> hsl to_hsl(pixel_type _pt)
	{
		rgb rx;

		rx.r = _pt.r / 255.0;
		rx.g = _pt.g / 255.0;
		rx.b = _pt.b / 255.0;

		return rgb2hsl(rx);
	}

	template <typename pixel_type> struct pixel_frame
	{
		unsigned int width, height, size_pixels;
		std::vector<pixel_type> pixels;

	public:

		pixel_frame() : width(0), height(0),  size_pixels(0)
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

		pixel_frame(pixel_type *_src, unsigned int _width, unsigned int _height)
			: width(_width), height(_height), size_pixels(_height* _width)
		{
			pixels.resize(size_pixels);
			for (int i = 0; i < size_pixels; i++) {
				pixels[i] = *_src;
				_src++;
			}
		}

		pixel_frame(pixel_frame&& _src)
			: width(_src.width), height(_src.height), size_pixels(_src.size_pixels)
		{
			pixels = std::move(_src.pixels);
		}

		bool is_same_size(pixel_frame& _src)
		{
			if (_src.width == width && _src.height == height)
				return true;
			return false;
		}

		pixel_frame& operator = (pixel_frame&& _src)
		{
			width = _src.width;
			height = _src.height;
			size_pixels = _src.size_pixels;
			pixels = std::move(_src.pixels);
			return *this;
		}

		pixel_frame(const pixel_frame& _src)
			: width(_src.width), height(_src.height), size_pixels(_src.size_pixels)
		{
			pixels = _src.pixels;
		}

		pixel_frame& operator = (const pixel_frame& _src)
		{
			width = _src.width;
			height = _src.height;
			size_pixels = _src.size_pixels;
			pixels = _src.pixels;
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

		pixel_type get(double px, double py)
		{
			if (empty()) {
				pixel_type pt = {};
				return pt;
			}

			unsigned int pixel_x = px * width;
			unsigned int pixel_y = py * height;

			unsigned int position = pixel_y * width + pixel_x;

			if (position > pixels.size()) 
			{
				position = pixels.size() - 1;
			}

			return pixels[position];
		}

		pixel_type get_pixel(unsigned int pixel_x, unsigned int pixel_y)
		{
			if (empty()) {
				pixel_type pt = {};
				return pt;
			}

			unsigned int position = pixel_y * width + pixel_x;

			if (position > pixels.size())
			{
				position = pixels.size() - 1;
			}

			return pixels[position];
		}

		void put(double px, double py, pixel_type _pixel)
		{
			if (empty()) {
				return;
			}

			unsigned int pixel_x = px * width;
			unsigned int pixel_y = py * height;

			unsigned int position = pixel_y * width + pixel_x;

			if (position > pixels.size())
			{
				position = pixels.size() - 1;
			}

			pixels[position] = _pixel;
		}

		void for_each(std::function<pixel_type (pixel_type)> _xform)
		{
			for (auto& px : pixels) {
				pixel_type np = _xform(px);
				px = np;
			}
		}

		ID2D1Bitmap1* get_bitmap(ID2D1DeviceContext *_context, std::function<bgra32_pixel(pixel_type)> _xform)
		{
			ID2D1Bitmap1* new_bitmap = nullptr;

			D2D1_SIZE_U size = {};
			size.width = width;
			size.height = height;

			if (!size_pixels)
				return nullptr;

			bgra32_pixel* data = new bgra32_pixel[size_pixels];

			if (data) {

				for (int i = 0; i < size_pixels; i++)
				{
					data[i] = _xform(pixels[i]);
//					data[i].a = 128;
					//data[i].b = 128;
				}

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

	struct delta_frame
	{
	public:

		pixel_frame<signal_pixel> activation_frame;
		pixel_frame<bgra32_pixel> last_frame;

		double hue_detection_threshold;
		double activation_area_percentage;
		double detection_pulse;
		double detection_cooldown;

		delta_frame() : activation_frame(128,128)
		{		
			hue_detection_threshold = .4;
			activation_area_percentage = .9;
			detection_pulse = .3;
			detection_cooldown = .3;
		}
		
		ID2D1Bitmap1* get_activation(ID2D1DeviceContext* _context)
		{
			ID2D1Bitmap1* new_bitmap;
			new_bitmap = activation_frame.get_bitmap(_context, [this](signal_pixel src)->bgra32_pixel {

				bgra32_pixel temp = {};

				if (src.activated) 
				{
					temp.a = 255.0;
					temp.b = 255.0;
				}
				else 
				{
					double alpha = src.signal;
					if (alpha < 0) {
						alpha = 0;
					}
					else if (alpha > 1) {
						alpha = 1.0;
					}
					temp.a = 255.0 * alpha;
					temp.b = 255.0 * alpha;

				}

				return temp;
			});

			return new_bitmap;
		}

		ID2D1Bitmap1* get_frame(ID2D1DeviceContext* _context)
		{
			ID2D1Bitmap1* new_bitmap;
			new_bitmap = last_frame.get_bitmap(_context, [](bgra32_pixel src)->bgra32_pixel {
				return src;
			});

			return new_bitmap;
		}

		std::vector<rectangle> get_movement_boxes()
		{
			rectangle _src = {};
			_src.x = 0;
			_src.y = 0;
			_src.w = activation_frame.get_width();
			_src.h = activation_frame.get_height();
			return get_movement_boxes(_src);
		}

		std::vector<rectangle> get_movement_boxes(rectangle _src)
		{
			std::vector<rectangle> movement_boxes;

			// look for movement, then look for a y split.

			enum analysis_states { start_area, extend_area };

			analysis_states astate = start_area;

			point candidate0, candidate1;

			int activated_wait = 0;
			
			for (int y = _src.y; y < _src.bottom(); y++)
			{
				for (int x = _src.x; x < _src.right(); x++)
				{
					auto px = activation_frame.get_pixel(x, y);

					if (astate == analysis_states::start_area)
					{
						if (px.activated)
						{
							candidate0.x = x;
							candidate0.y = y;
							astate = extend_area;
							activated_wait = 16;
						}
					}
					else if (astate == analysis_states::extend_area)
					{
						if (px.activated)
						{
							candidate1.x = x;
							candidate1.y = y;
							activated_wait = 16;
						}
						else 
						{
							activated_wait--;
							if (activated_wait <= 0)
							{
								candidate1.x = x;
								candidate1.y = y;
								astate = start_area;
								rectangle r;

								if (candidate0.x < x) 
								{
									r.x = candidate0.x;
									r.w = candidate1.x - candidate0.x;
								}
								else 
								{
									r.x = candidate1.x;
									r.w = candidate0.x - candidate1.x;
								}

								if (candidate0.y < y) 
								{
									r.y = candidate0.y;
									r.h = candidate1.y - candidate0.y;
								}
								else
								{
									r.y = candidate1.y;
									r.h = candidate0.y - candidate1.y;
								}

								movement_boxes.push_back(r);
							}
						}
					}
				}
			}
		
			point scale = activation_frame.get_scale();

			scale.x *= last_frame.get_width();
			scale.y *= last_frame.get_height();

			for (auto& mb : movement_boxes)
			{
				mb.x *= scale.x;
				mb.y *= scale.y;
				mb.w *= scale.x;
				mb.h *= scale.y;
			}
			return movement_boxes;
		}

		std::vector<ID2D1Bitmap1*> get_motion(ID2D1DeviceContext* _context)
		{
			std::vector<ID2D1Bitmap1*> bitmaps;
			return bitmaps;
		}

		void next_frame(bgra32_pixel* _src, int _width, int _height, int _stride)
		{
			pixel_frame<bgra32_pixel> new_frame(_src, _width, _height);
			next_frame(new_frame);
		}

		void next_frame(pixel_frame<bgra32_pixel>& _frame1)
		{
			activation_frame.for_each([this](signal_pixel _src) -> signal_pixel {
				if (_src.signal > 0.0)
				{
					_src.signal -= detection_cooldown;
					if (_src.signal < 0.0)
						_src.signal = 0.0;
				}
				_src.detected = 0;
				_src.activated = false;
				return _src;
				});

			if (last_frame.empty() || !last_frame.is_same_size(_frame1)) {
				last_frame = _frame1;
				return;
			}

			long long m_pixel_count = 0;
			long long a_pixel_count = 0;

			int w, h;

			w = _frame1.get_width();
			h = _frame1.get_height();

#pragma omp parallel for
			for (int iy = 0; iy < h; iy ++)
			{
				for (int ix = 0; ix < w; ix ++) 
				{
					auto pixel0 = last_frame.get_pixel(ix, iy);
					auto pixel1 = _frame1.get_pixel(ix, iy);

					hsl hsl0 = to_hsl(pixel0);
					hsl hsl1 = to_hsl(pixel1);

					double dh = std::abs(hsl0.h - hsl1.h);
					double dl = std::abs(hsl0.l - hsl1.l);
					double ds = std::abs(hsl0.s - hsl1.s);

					if (dh > .4 || dl > .8 || ds > .8) 
					{
						double px, py;
						px = ix / (double)w;
						py = iy / (double)h;
						m_pixel_count++;
						signal_pixel d = activation_frame.get(px, py);
						if (!d.detected) {
							d.signal += detection_pulse;
						}
						d.detected++;
						activation_frame.put(px, py, d);
					}
					else 
					{
						a_pixel_count++;
					}
				}
			}

			double frame_area = (double)_frame1.get_area() / (double)activation_frame.get_area();

			activation_frame.for_each([this, frame_area](signal_pixel _src) -> signal_pixel {
				double d = _src.detected / frame_area;
				if (d > this->activation_area_percentage) {
					_src.activated = true;
				}
				return _src;
				});

			last_frame = _frame1;
		}
	};

}

