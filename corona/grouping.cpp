
#include "corona.h"

namespace corona
{
	namespace database
	{

		class animal
		{
		public:
			object_name animal_name;
			object_name species;
			color_box	color;
			rectangle_box position;
		};

		void query_tests()
		{
			dynamic_box box;
			box.init(1 << 21);


		}
	}
}
