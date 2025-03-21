/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
binary wrapper for a reference to midi object.

Notes

For Future Consideration
Implement midi chumpies here.
*/


#ifndef CORONA_MIDI_BOX_H
#define CORONA_MIDI_BOX_H

namespace corona {

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

#endif
