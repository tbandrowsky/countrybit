
#include "corona.h"

namespace corona
{
	namespace database
	{
		measure operator ""_px(long double px)
		{
			return measure(px, measure_units::pixels);
		}

		measure operator ""_pct(long double pct)
		{
			return measure(pct, measure_units::percent_remaining);
		}
		
		measure operator ""_pcw(long double pct)
		{
			return measure(pct, measure_units::percent_width);
		}

		measure operator ""_pch(long double pct)
		{
			return measure(pct, measure_units::percent_height);
		}

		measure operator -(const measure& _src)
		{
			return measure(-_src.amount, _src.units);
		}
	}
}

