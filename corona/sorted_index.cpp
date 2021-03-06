
#include "corona.h"

namespace corona
{
	namespace database
	{

		bool test_index()
		{

			static_box<20000> box;

			using test_sorted_index_type = sorted_index<int, istring<30>, 1>;

			test_sorted_index_type test;

			relative_ptr_type test_location;
			test = test_sorted_index_type::create_sorted_index( &box, test_location );

			test.insert_or_assign(5, "hello");
			auto t1 = test[5];
			if (t1.get_key() != 5 || t1.get_value() != "hello")
			{
				std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
				return false;
			}
			test.insert_or_assign(7, "goodbye");
			auto t2 = test[7];
			if (t2.get_key() != 7 || t2.get_value() != "goodbye" || t2.get_value() != "goodbye")
			{
				std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
				return false;
			}
			test.insert_or_assign(7, "something");
			auto t3 = test[7];
			if (t3.get_key() != 7 || t3.get_value() != "something")
			{
				std::cout << __LINE__ << " fail: wrong updated value." << std::endl;
				return false;
			}
			auto t4 = test[7];
			if (t4.get_key() != 7 || t4.get_value() != "something")
			{
				std::cout << __LINE__ << " fail: wrong [] access." << std::endl;
				return false;
			}
			try 
			{
				auto t5 = test[6];
				std::cout << __LINE__ << " fail: wrong null access." << std::endl;
				return false;
			}
			catch (std::exception exc)
			{
				;
			}

			int count = 0;

			for (auto t : test)
			{
				count++;
			}

			if (count != 2)
			{
				std::cout << __LINE__ << " fail: wrong number of iterations." << std::endl;
				return false;
			}

			test.put(2, "hello super");
			auto t6 = test[2];
			if (t6.get_key() != 2 || t6.get_value() != "hello super")
			{
				std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
				return false;
			}

			test.put(1, "first");
			auto t7 = test[1];
			if (t7.get_key() != 1 || t7.get_value() != "first")
			{
				std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
				return false;
			}

			test.put(1, "second");
			t7 = test[1];
			if (t7.get_key() != 1 || t7.get_value() != "second")
			{
				std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
				return false;
			}

			count = std::count_if(test.begin(), test.end(), [](auto) {
				return true;
				});

			if (count != 4)
			{
				std::cout << __LINE__ << " fail: wrong number of iterations." << std::endl;
				return false;
			}

			int tests[4] = { 1, 2, 5, 7 };
			int k = 0;
			for (auto item : test)
			{
				if (tests[k] != item.first) {
					std::cout << __LINE__ << " loop failed" << std::endl;
					return false;
				}
				k++;
			}

			int tests2[3] = { 2, 5, 7 };

			k = 0;
			for (auto item : test.where( [](auto& kvpi) { return kvpi.first>1; } ) )
			{
				if (tests2[k] != item.first) {
					std::cout << __LINE__ << " starting from key failed" << std::endl;
					return false;
				}
				k++;
			}

			k = 0;
			for (auto item : test.where([](auto& kvpi) { return kvpi.first > 1; }))
			{
				if (tests2[k] != item.first) {
					std::cout << __LINE__ << " loop failed" << std::endl;
					return false;
				}
				k++;
			}

			test.erase(1);
			test.erase(7);

			auto testi = test
				.where([](auto& a) { return a.first == 7; });

			if (testi.exists()) {
				std::cout << __LINE__ << " existence failed" << std::endl;
				return false;
			}

			if (!testi.eoi()) {
				std::cout << __LINE__ << " eoi failed" << std::endl;
				return false;
			}

			int tests3[2] = { 2, 5 };

			k = 0;

			for (auto item : test)
			{
				if (tests3[k] != item.first) {
					std::cout << __LINE__ << " erasing iterator failed" << std::endl;
					return false;
				}
				k++;
			}

			return true;
		}

	}

}