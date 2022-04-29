
#include <iostream>
#include "list_box.h"
#include "assert_if.h"

namespace countrybit
{
	namespace database
	{
		bool list_box_tests()
		{
			dynamic_box box;

			box.init(100000);

			using int_list = list_box<int>;

			int_list ix = int_list::create(&box);

			for (auto item : ix)
			{
				std::cout << __LINE__ << ": list_box iterator failed empty" << std::endl;
				return false;
			}

			for (int i = 0; i < 10; i++)
			{
				ix.push_back(i);
			}

			int count = 0;
			for (auto its : ix)
			{
				if (its != count) {
					std::cout << __LINE__ << ": list_box data wrong" << std::endl;
					return false;
				}
				count++;
			}

			count = ix.count_if([](auto& t) { return t < 3;  });
			assert_if([count]() { return count == 3; }, "Wrong count");

			auto items = ix.where([](auto& t) { return t < 3;  });

			count = 0;
			for (auto r : items)
			{
				assert_if([r]() { return r < 3;  }, "Wrong values");
				count++;
			}
			assert_if([count]() { return count == 3; }, "Wrong count");


			return true;

		}
	}
}