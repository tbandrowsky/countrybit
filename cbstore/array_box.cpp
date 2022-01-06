
#include "array_box.h"
#include <iostream>

namespace countrybit
{
	namespace database
	{
		bool array_box_tests()
		{
			char box[8192];

			using int_array = array_box<int>;

			int_array ix = int_array::create(box, 4000);

			for (int k = 4000; k < sizeof(box); k++) 
			{
				box[k] = 0;
			}

			for (int i = 0; i < ix.max_size(); i++)
			{
				ix.push_back(i);
			}

			for (int i = 0; i < ix.size(); i++)
			{
				if (ix[i] != i) {
					std::cout << __LINE__ << ": array box failed" << std::endl;
					return false;
				}
			}

			for (int k = 4000; k < sizeof(box); k++)
			{
				if (box[k] != 0) {
					std::cout << __LINE__ << ": array box failed" << std::endl;
					return false;
				}
			}

			return true;
		}
	}
}
