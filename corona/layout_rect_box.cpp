
#include "corona.h"

namespace corona
{
	namespace database
	{
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


	}
}

