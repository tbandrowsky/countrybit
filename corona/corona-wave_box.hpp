#ifndef CORONA_WAVE_BOX
#define CORONA_WAVE_BOX

namespace corona {

	struct wave_instance
	{
		double	start_seconds,
			stop_seconds;
		double	pitch_adjust,
			volume_adjust;
		bool	playing;
	};
	/// <summary>
	/// 
	/// </summary>
	class wave_box : public boxed<wave_instance>
	{
	public:
		wave_box(char* t) : boxed<wave_instance>(t)
		{
			;
		}

		wave_box operator = (const wave_box& _src)
		{
			set_data(_src);
			return *this;
		}

		wave_box operator = (wave_instance _src)
		{
			set_value(_src);
			return *this;
		}

		operator wave_instance& ()
		{
			wave_instance& t = boxed<wave_instance>::get_data_ref();
			return t;
		}

		wave_instance* operator->()
		{
			wave_instance& t = boxed<wave_instance>::get_data_ref();
			return &t;
		}

		wave_instance value() const { return boxed<wave_instance>::get_value(); }

	};

}

#endif
