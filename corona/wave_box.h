#pragma once

#include "store_box.h"
#include "float_box.h"

#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		struct wave_instance
		{
			double	start_seconds,
					stop_seconds;
			double	pitch_adjust,
					volume_adjust;
			bool	playing;
		};

		class wave_box : protected boxed<wave_instance>
		{
		public:
			wave_box(char* t) : boxed<wave_instance>(t)
			{
				;
			}

			wave_box operator = (const wave_box& _src)
			{
				boxed<wave_instance>::operator =(_src);
				return *this;
			}

			wave_box operator = (wave_instance _src)
			{
				boxed<wave_instance>::operator =(_src);
				return *this;
			}

			operator wave_instance& ()
			{
				wave_instance& t = boxed<wave_instance>::get_data_ref();
				return t;
			}

			wave_instance value() const { return boxed<wave_instance>::get_value(); }

		};

	}
}
