#pragma once

namespace corona
{
	namespace database
	{

		using color = D3DCOLORVALUE;

		class color_box : protected boxed<color>
		{
		public:
			color_box(char* t) : boxed<color>(t)
			{
				;
			}

			color_box operator = (const color_box& _src)
			{
				boxed<color>::operator =(_src);
				return *this;
			}

			color_box operator = (color_box _src)
			{
				set_data(_src);
				return *this;
			}

			color_box operator = (color _src)
			{
				set_value(_src);
				return *this;
			}

			color* operator->()
			{
				color& t = boxed<color>::get_data_ref();
				return &t;
			}

			operator color& ()
			{
				color& t = boxed<color>::get_data_ref();
				return t;
			}

			color value() const { return boxed<color>::get_value(); }

		};

	}

}
