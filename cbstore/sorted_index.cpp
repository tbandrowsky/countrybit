
#include "sorted_index.h"
#include "string_box.h"

namespace countrybit
{
	namespace database
	{

		bool test_index()
		{

			static_box<20000> box;

			using test_sorted_index_type = sorted_index<int, istring<30>, 1>;

			test_sorted_index_type test;

			row_id_type test_location;
			test = test_sorted_index_type::create_sorted_index( &box, 100, test_location );

			auto t1 = test.insert_or_assign(5, "hello");
			if (t1.get_key() != 5 || t1.get_value() != "hello" || t1->second != "hello")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}
			auto t2 = test.insert_or_assign(7, "goodbye");
			if (t2.get_key() != 7 || t2.get_value() != "goodbye" || t2->second != "goodbye")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}
			auto t3 = test.insert_or_assign(7, "something");
			if (t3.get_key() != 7 || t3->second != "something")
			{
				std::cout << "fail: wrong updated value." << std::endl;
				return false;
			}
			auto t4 = test[7];
			if (t4.get_key() != 7 || t4->second != "something")
			{
				std::cout << "fail: wrong [] access." << std::endl;
				return false;
			}
			auto t5 = test[6];
			if (t5 != test.end())
			{
				std::cout << "fail: wrong null access." << std::endl;
				return false;
			}

			int count = 0;

			for (auto t : test)
			{
				count++;
			}

			if (count != 2)
			{
				std::cout << "fail: wrong number of iterations." << std::endl;
				return false;
			}

			auto t6 = test.put(2, "hello super");
			if (t6.get_key() != 2 || t6.get_value() != "hello super")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			auto t7 = test.put(1, "first");
			if (t7.get_key() != 1 || t7->second != "first")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			t7 = test.put(1, "second");
			if (t7.get_key() != 1 || t7->second != "second")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			count = std::count_if(test.begin(), test.end(), [](auto) {
				return true;
				});

			if (count != 4)
			{
				std::cout << "fail: wrong number of iterations." << std::endl;
				return false;
			}

			int tests[4] = { 1, 2, 5, 7 };
			int k = 0;
			for (auto& item : test)
			{
				if (tests[k] != item.first) {
					std::cout << "loop failed" << std::endl;
					return false;
				}
				k++;
			}

			int tests2[3] = { 2, 5, 7 };

			k = 0; 
			for (auto& item : test[2])
			{
				if (tests2[k] != item.first) {
					std::cout << "starting from key failed" << std::endl;
					return false;
				}
				k++;
			}

			auto x = test[1];
			test.erase(x);
			test.erase(7);

			int tests3[2] = { 2, 5 };

			k = 0;

			for (auto& item : test)
			{
				if (tests3[k] != item.first) {
					std::cout << "erasing iterator failed" << std::endl;
					return false;
				}
				k++;
			}

			return true;
		}

	}

}