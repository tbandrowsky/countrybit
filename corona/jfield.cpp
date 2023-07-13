

#include "corona.h"

namespace corona
{
	namespace database
	{

		bool dimensions_type::increment(dimensions_type& _constraint)
		{
			x++;
			if (x >= _constraint.x)
			{
				x = 0;
				y++;
				if (y >= _constraint.y)
				{
					z++;
					y = 0;
					if (z >= _constraint.z)
					{
						z = 0;
						return false;
					}
				}
			}
			return true;
		}


	}
}
