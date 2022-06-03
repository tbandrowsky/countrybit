#pragma once

namespace corona
{
	namespace database
	{
		enum class measure_units
		{
			percent = 0,
			pixels = 1,
			size = 2
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
		measure operator ""_pct(long double pct);
		measure operator ""_psz(long double pct);

		class dyrect
		{
		public:
			measure x, y, width, height;

			dyrect() { ; }
			dyrect(measure _x, measure _y, measure _width, measure _height) :
				x(_x), y(_y), width(_width), height(_height)
			{
				;
			}
		};

		class dyrect_box : protected boxed<dyrect>
		{
		public:
			dyrect_box(char* t) : boxed<dyrect>(t)
			{
				;
			}

			dyrect_box operator = (const dyrect_box& _src)
			{
				boxed<dyrect>::operator =(_src);
				return *this;
			}

			dyrect_box operator = (dyrect_box _src)
			{
				set_data(_src);
				return *this;
			}

			dyrect_box operator = (dyrect _src)
			{
				set_value(_src);
				return *this;
			}

			dyrect* operator->()
			{
				dyrect& t = boxed<dyrect>::get_data_ref();
				return &t;
			}

			operator dyrect& ()
			{
				dyrect& t = boxed<dyrect>::get_data_ref();
				return t;
			}

			dyrect value() const { return boxed<dyrect>::get_value(); }

		};

	}
}
