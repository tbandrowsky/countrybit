
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
			return measure(pct, measure_units::percent_container);
		}

		measure operator ""_psz(long double pct)
		{
			return measure(pct, measure_units::percent_size);
		}

		measure operator -(const measure& _src)
		{
			return measure(-_src.amount, _src.units);
		}
	}
}

