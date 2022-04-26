
#include "array_box.h"
#include <iostream>
#include "assert_if.h"

namespace countrybit
{
	namespace database
	{
		bool array_box_tests()
		{
			char box[8192];

			using int_array = array_box<int>;

			int_array ix = int_array::create(box, 4000);

			for (auto item : ix)
			{
				std::cout << __LINE__ << ": array_box iterator failed empty" << std::endl;
				return false;
			}

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

			iarray<int, 100> test_iarray;

			for (auto item : test_iarray) 
			{
				std::cout << __LINE__ << ": irray iterator failed empty" << std::endl;
				return false;
			}

			for (int i = 0; i < 10; i++)
				test_iarray.push_back(i);

			int iteration_count = 0;

			for (auto r : test_iarray)
			{
				if (r.item != r.location || test_iarray[r.location] != r.item) {
					std::cout << __LINE__ << ": iarray failed" << std::endl;
					return false;
				}
				iteration_count++;
			}
			assert_if([iteration_count]() { return iteration_count == 10; }, "Wrong count");

			auto count = test_iarray.count_if([](auto& t) { return t < 3;  });
			assert_if([count]() { return count == 3; }, "Wrong count");

			auto items = test_iarray.where([](auto& t) { return t < 3;  });

			count = 0;
			for (auto r : items)
			{
				auto rti = r.item;
				assert_if([rti]() { return rti < 3;  }, "Wrong values");
				count++;
			}
			assert_if([count]() { return count == 3; }, "Wrong count");


			return true;
		}
	}
}
