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
		float			total_red;
		float			total_green;
		float			total_blue;

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

		friend class cursor;

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
				if (cur_pos < 0 || cur_pos >= pf->pixels.size())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* down()
			{
				cur_pos += pf->get_width();
				if (cur_pos < 0 || cur_pos >= pf->pixels.size())
				{
					return nullptr;
				}
				return &pf->pixels[cur_pos];
			}

			inline pixel_type* up()
			{
				cur_pos += pf->get_width();
				if (cur_pos < 0 || cur_pos >= pf->pixels.size())
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
				if (cur_pos < 0 || cur_pos >= pf->get_area())
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
				if (cur_pos < 0 || cur_pos >= pf->get_area())
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

			inline void set(int _new_x, int _new_y)
			{
				cur_pos = _new_y * pf->get_width() + _new_x;
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

		void for_each(std::function<pixel_type (pixel_type)> _xform)
		{
			threadomatic::run_each<pixel_type>(pixels, [_xform](pixel_type& px) -> void {
				pixel_type np = _xform(px);
				px = np;
			});
		}

		ID2D1Bitmap1* get_bitmap(ID2D1DeviceContext *_context, std::function<bgra32_pixel(int x, int y, pixel_type)> _xform)
		{
			ID2D1Bitmap1* new_bitmap = nullptr;

			D2D1_SIZE_U size = {};
			size.width = width;
			size.height = height;

			if (!size_pixels)
				return nullptr;

			bgra32_pixel* data = new bgra32_pixel[size_pixels];

			if (data) {

				threadomatic::run_each<bgra32_pixel, pixel_type>(data, pixels, [data, size, _xform](bgra32_pixel* _target, pixel_type& _src) -> void {
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

	struct delta_frame
	{
	public:

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

		timer	sprinkle_timer;
		int		sprinkle_counter;
		double	last_sprinkle_seconds;
		double	total_sprinkle_seconds;

		pixel_frame<signal_pixel> activation_frame;
		pixel_frame<bgra32_pixel> last_frame;

		double hue_detection_threshold;
		double sat_detection_threshold;
		double lum_detection_threshold;

		double activation_area_percentage;
		double detection_pulse;
		double detection_cooldown;
		double motion_spacing;

		int color_cycle_frames;
		ccolor color_cycle_start, 
			color_cycle_step;

		std::mt19937 random_engine;
		std::uniform_real_distribution<double> color_distribution;
		std::uniform_real_distribution<double> color_cycle_distribution;

		delta_frame() : activation_frame(256, 256), 
			color_distribution(.0, 1.0),
			color_cycle_distribution(0.0, .1),
			total_frame_seconds(0),
			last_frame_seconds(0),
			frame_counter(0)
		{
			reset_defaults();
				
		}

		void reset_defaults()
		{
			hue_detection_threshold = .05;
			sat_detection_threshold = .1;
			lum_detection_threshold = .1;
			activation_area_percentage = .3;
			detection_pulse = .8;
			detection_cooldown = .020;
			motion_spacing = 16;
			color_cycle_start = {};
			init_color_cycle();
			next_color_cycle();
		}

		lockable activation_locker;

		ID2D1Bitmap1* get_activation(ID2D1DeviceContext* _context, sprinkle_buffer& _sprinkles)
		{

			ID2D1Bitmap1* new_bitmap;

			bool sprinkle_enable = false;

			sprinkle_counter++;
			last_sprinkle_seconds = sprinkle_timer.get_elapsed_seconds();
			total_sprinkle_seconds += last_sprinkle_seconds;
			if (total_sprinkle_seconds > 1/4.0) {
				sprinkle_enable = true;
				total_sprinkle_seconds = 0.0;
			}

			new_bitmap = activation_frame.get_bitmap(_context, [sprinkle_enable, this, &_sprinkles](int x, int y, signal_pixel src)->bgra32_pixel {

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

				if (src.activated)
				{

					sprinkle new_sprinkle;
					new_sprinkle.acceleration.x = 0.00;
					new_sprinkle.acceleration.y = -0.02;
					new_sprinkle.acceleration.z = 0.00;
					new_sprinkle.velocity.x = 0.00;
					new_sprinkle.velocity.y = 0.0;
					new_sprinkle.velocity.z = 0.00;
					new_sprinkle.current.x = x;
					new_sprinkle.current.y = y % 100;
					new_sprinkle.current.z = 0.00;
					new_sprinkle.pixel_start = temp;
					new_sprinkle.pixel_end.b /= 3;
					new_sprinkle.pixel_end.r /= 3;
					new_sprinkle.pixel_end.g /= 3;
					new_sprinkle.pixel_end.a = 0.0;
					_sprinkles.put(new_sprinkle);
				}

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

		std::vector<movement_box> collapse_movement_boxes(std::vector<movement_box> list)
		{

			std::list<movement_box> new_list;
			movement_box found_item;

			int list_index;

			for (list_index = 0; list_index < list.size(); list_index++)
			{
				bool was_found = false;

				movement_box mb1 = list[list_index];

				auto nli = new_list.begin();

				while (nli != new_list.end())
				{
					movement_box mb2 = *nli;
					bool x_ok = false;
					bool y_ok = false;

					if (mb1.area.x <= mb2.area.x && mb1.area.right() >= mb2.area.x)
					{
						x_ok = true;
					}
					else if (mb2.area.x <= mb1.area.x && mb2.area.right() >= mb2.area.x)
					{
						x_ok = true;
					}

					if (mb1.area.y <= mb2.area.y && mb1.area.bottom() >= mb2.area.y)
					{
						y_ok = true;
					}
					else if (mb2.area.y <= mb1.area.y && mb2.area.bottom() >= mb1.area.y)
					{
						y_ok = true;
					}

					if (x_ok && y_ok && mb1.image_hash == mb2.image_hash)
					{
						double right1 = mb1.area.right();
						double bottom1 = mb1.area.bottom();
						if (mb1.area.x > mb2.area.x)
						{
							mb1.area.x = mb2.area.x;
						}
						if (mb1.area.y > mb2.area.y)
						{
							mb1.area.y = mb2.area.y;
						}
						mb1.area.w = std::max(right1, mb2.area.right()) - mb1.area.x;
						mb1.area.h = std::max(bottom1, mb2.area.bottom()) - mb1.area.y;
						was_found = true;

						list.push_back(mb1);
						nli = new_list.erase(nli);
					}
					else
					{
						nli++;
					}
				}

				if (!was_found)
				{
					new_list.push_back(mb1);
				}
			}

			std::vector<movement_box> new_vector;

			for (auto nlb : new_list)
				new_vector.push_back(nlb);

			return new_vector;
		}


		std::vector<movement_box> get_movement_boxes()
		{
			std::vector<movement_box> movement_boxes;

			point candidate0;

			int activated_wait = 0;		

			pixel_frame<signal_pixel>::cursor c = activation_frame.get_cursor(0, 0);

			int activation_distance = 16;

			for (signal_pixel* sp = c.first(); sp; sp = c.right())
			{
				if (sp->activated)
				{
					// make sure we didn't somehow pick up this pixel already
					auto found = std::find_if(movement_boxes.begin(), movement_boxes.end(), [sp, &c](movement_box& r) {
						return rectangle_math::contains(r.area, c.get_x(), c.get_y());
						});

					if (found == movement_boxes.end()) 
					{
						movement_box r;
						r.area.x = c.get_x()- activation_distance/2;
						r.area.y = c.get_y()- activation_distance/2;
						if (r.area.x < 0) {
							r.area.x = 0;
						}
						if (r.area.y < 0) {
							r.area.y = 0;
						}
						r.area.w = activation_distance;
						r.area.h = activation_distance;
						

						auto mbcursor = activation_frame.get_cursor(r.area.x, r.area.y);
						signal_pixel* spmb = mbcursor.first();

						for (int y = 0; y < r.area.h; y++)
						{
							for (int x = 0; x < r.area.w; x++)
							{
								if (spmb)
								{
									int count = spmb->detected;
									rgb hasho;
									if (count) {
										hasho.r = spmb->total_red / count;
										hasho.g = spmb->total_green / count;
										hasho.b = spmb->total_blue / count;
										r.image_hash = std::to_string((int)(rgb2hsl(hasho).h) * 16);
									}
									spmb = mbcursor.right();
								}
							}
							spmb = mbcursor.carriage_return(r.area.x);
						}

						movement_boxes.push_back(r);
					}
				}
			}

			//std::cout << "movement boxes before collapse:" << movement_boxes.size() << std::endl;

			movement_boxes = collapse_movement_boxes(movement_boxes);

			point scale = activation_frame.get_scale();

			scale.x *= last_frame.get_width();
			scale.y *= last_frame.get_height();
			//std::cout << "movement boxes after collapse:" << movement_boxes.size() << std::endl;

			for (auto& mb : movement_boxes)
			{
				mb.area.x *= scale.x;
				mb.area.y *= scale.y;
				mb.area.w *= scale.x;
				mb.area.h *= scale.y;
				//std::cout << std::format("{4}:{0},{1}-{2}x{3}", mb.area.x, mb.area.y, mb.area.w, mb.area.h, mb.image_hash) << std::endl;
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

			if (total_color_seconds > 3 || color_counter == 0) {
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

			activation_frame.for_each([this](signal_pixel _src) -> signal_pixel {
				if (_src.signal > 0.0)
				{
					_src.signal -= detection_cooldown;
					if (_src.signal < 0.0)
						_src.signal = 0.0;
				}
				_src.total_blue = _src.total_green = _src.total_red = 0.0;
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

			auto frame0_cursor = last_frame.get_cursor(0, 0);
			auto frame1_cursor = _frame1.get_cursor(0, 0);
			auto activation_cursor = activation_frame.get_cursor(0, 0);

			auto sp0 = frame0_cursor.first();
			auto sp1 = frame1_cursor.first();

			while (sp0 && sp1)
			{

				hsl hsl0 = to_hsl(*sp0);
				hsl hsl1 = to_hsl(*sp1);

				double dh = std::abs(hsl0.h - hsl1.h);
				double dl = std::abs(hsl0.l - hsl1.l);
				double ds = std::abs(hsl0.s - hsl1.s);

				int activation_x = (frame0_cursor.get_x() * activation_frame.get_width()) / last_frame.get_width();
				int activation_y = (frame0_cursor.get_y() * activation_frame.get_height()) / last_frame.get_height();
				activation_cursor.set(activation_x, activation_y);

				auto spa = activation_cursor.get();

				if (dh > hue_detection_threshold && dl > lum_detection_threshold && ds > sat_detection_threshold)
				{
					m_pixel_count++;

					if (!spa->detected) {
						spa->signal += detection_pulse;
						spa->detect_counter = frame_counter;
					}

					auto sp0l = *sp0;
					auto sp1l = *sp1;

					spa->total_blue += sp0l.b + sp1l.b;
					spa->total_green += sp0l.g + sp1l.g;
					spa->total_red += sp0l.r + sp1l.r;
					spa->detected++;
					
				}
				else if (spa->signal > 0.0)
				{
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
