/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
A color box is a wrapper around a d2d color, with an 
eye towards binary serialization 
Notes

For Future Consideration
*/


#ifndef CORONA_COLOR_BOX_H
#define CORONA_COLOR_BOX_H


namespace corona
{

	class color_box : protected boxed<ccolor>
	{
	public:
		color_box(char* t) : boxed<ccolor>(t)
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

		color_box operator = (ccolor _src)
		{
			set_value(_src);
			return *this;
		}

		ccolor* operator->()
		{
			ccolor& t = boxed<ccolor>::get_data_ref();
			return &t;
		}

		operator ccolor& ()
		{
			ccolor& t = boxed<ccolor>::get_data_ref();
			return t;
		}

		ccolor value() const { return boxed<ccolor>::get_value(); }

	};

}

#endif
