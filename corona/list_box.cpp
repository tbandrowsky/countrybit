
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

			for (auto itb = ix.rbegin(); itb != std::end(ix); itb--)
			{
				count--;
				if (*itb != count) {
					std::cout << __LINE__ << ": list_box reverse read data wrong" << std::endl;
					return false;
				}
			}

			int check_prev1 = ix.rbegin()[-1].get_value();
			int check_prev2 = ix.rbegin()[-2].get_value();

			if (check_prev1 != 9) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			if (check_prev2 != 8) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
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