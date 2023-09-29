module;

export module corona.database:layout_rect_box;
import :store_box;
import :stdapi;

export enum class measure_units
		{
			percent_container = 0,
			percent_remaining = 1,
			percent_aspect = 2,
			pixels = 3,
			font = 4,
			font_golden_ratio = 5,
			percent_child = 6
		};

export class measure
		{
		public:
			long double amount;
			measure_units units;

			measure() : amount(0), units(measure_units::pixels) { ; }
			measure(long double _amount, measure_units _units) : amount(_amount), units(_units) { ; }
		};

export measure operator ""_px(long double px);
export 		measure operator ""_container(long double pct);
export measure operator ""_remaining(long double pct);
export measure operator ""_children(long double pct);
export measure operator ""_aspect(long double pct);
export measure operator ""_font(long double fnt);
export measure operator ""_fontgr(long double fnt);
export measure operator -(const measure& _src);

export class layout_rect
		{
		public:
			measure x, y, width, height;

			layout_rect() { ; }
			layout_rect(measure _x, measure _y, measure _width, measure _height) :
				x(_x), y(_y), width(_width), height(_height)
			{
				;
			}
			layout_rect(measure _width, measure _height) :
				x(0.0_px), y(0.0_px), width(_width), height(_height)
			{
				;
			}
		};

export class layout_rect_box : protected boxed<layout_rect>
		{
		public:
			layout_rect_box(char* t) : boxed<layout_rect>(t)
			{
				;
			}

			layout_rect_box operator = (const layout_rect_box& _src)
			{
				boxed<layout_rect>::operator =(_src);
				return *this;
			}

			layout_rect_box operator = (layout_rect_box _src)
			{
				set_data(_src);
				return *this;
			}

			layout_rect_box operator = (layout_rect _src)
			{
				set_value(_src);
				return *this;
			}

			layout_rect* operator->()
			{
				layout_rect& t = boxed<layout_rect>::get_data_ref();
				return &t;
			}

			operator layout_rect& ()
			{
				layout_rect& t = boxed<layout_rect>::get_data_ref();
				return t;
			}

			layout_rect value() const { return boxed<layout_rect>::get_value(); }

		};

		measure operator ""_px(long double px)
		{
			return measure(px, measure_units::pixels);
		}

		measure operator ""_remaining(long double pct)
		{
			return measure(pct, measure_units::percent_remaining);
		}

		measure operator ""_container(long double pcc)
		{
			return measure(pcc, measure_units::percent_container);
		}

		measure operator ""_aspect(long double pct)
		{
			return measure(pct, measure_units::percent_aspect);
		}

		measure operator ""_children(long double pct)
		{
			return measure(pct, measure_units::percent_child);
		}

		measure operator ""_font(long double fnt)
		{
			return measure(fnt, measure_units::font);
		}

		measure operator ""_fontgr(long double fnt)
		{
			return measure(fnt, measure_units::font_golden_ratio);
		}

		measure operator -(const measure& _src)
		{
			return measure(-_src.amount, _src.units);
		}

