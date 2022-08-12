#pragma once

namespace corona
{
	namespace database
	{
		enum class measure_units
		{
			percent_container = 0,
			percent_remaining = 1,
			percent_aspect = 2,
			pixels = 3,
			font = 4,
			font_golden_ratio = 5
		};

		class measure
		{
		public:
			long double amount;
			measure_units units;

			measure() : amount(0), units(measure_units::pixels) { ; }
			measure(long double _amount, measure_units _units) : amount(_amount), units(_units) { ; }
		};

		measure operator ""_px(long double px);
		measure operator ""_pcc(long double pct);
		measure operator ""_pct(long double pct);
		measure operator ""_pca(long double pct);
		measure operator ""_fnt(long double fnt);
		measure operator ""_fntgr(long double fnt);
		measure operator -(const measure& _src);

		class layout_rect
		{
		public:
			measure x, y, width, height;

			layout_rect() { ; }
			layout_rect(measure _x, measure _y, measure _width, measure _height) :
				x(_x), y(_y), width(_width), height(_height)
			{
				;
			}
		};

		class layout_rect_box : protected boxed<layout_rect>
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

	}
}
