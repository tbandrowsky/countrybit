
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

		measure operator ""_pcc(long double pcc)
		{
			return measure(pcc, measure_units::percent_container);
		}

		measure operator ""_pca(long double pct)
		{
			return measure(pct, measure_units::percent_aspect);
		}

		measure operator ""_fnt(long double fnt)
		{
			return measure(fnt, measure_units::font);
		}

		measure operator ""_fntgr(long double fnt)
		{
			return measure(fnt, measure_units::font_golden_ratio);
		}

		measure operator -(const measure& _src)
		{
			return measure(-_src.amount, _src.units);
		}


	}
}

