
#include "corona.h"

namespace corona
{
	namespace database
	{

		gradientStop::gradientStop()
		{
			stop_position = 0;
			stop_color.r = 0;
			stop_color.g = 0;
			stop_color.b = 0;
			stop_color.a = 1.0;
		}

		gradientStop::gradientStop(color _color, float _position)
		{
			stop_position = _position;
			stop_color = _color;
		}

	}
}