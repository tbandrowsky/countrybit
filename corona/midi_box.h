#pragma once

namespace corona
{
	namespace database
	{
		struct midi_instance
		{
			double	start_seconds,
					stop_seconds;
			double	pitch_adjust,
					volume_adjust;
			bool	playing;
		};

		class midi_box : protected boxed<midi_instance>
		{
		public:
			midi_box(char* t) : boxed<midi_instance>(t)
			{
				;
			}

			midi_box operator = (const midi_box& _src)
			{
				boxed<midi_instance>::set_data(_src);
				return *this;
			}

			midi_box operator = (midi_instance _src)
			{
				boxed<midi_instance>::set_value(_src);
				return *this;
			}

			operator midi_instance& ()
			{
				midi_instance& t = boxed<midi_instance>::get_data_ref();
				return t;
			}

			midi_instance* operator->()
			{
				midi_instance& t = boxed<midi_instance>::get_data_ref();
				return &t;
			}

			midi_instance value() const { return boxed<midi_instance>::get_value(); }

		};

	}
}
