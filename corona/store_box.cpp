
#include "pch.h"

namespace corona 
{
	namespace database 
	{


		bool test_box(serialized_box_container *b)
		{
			struct test_struct {
				int i1,
					i2,
					i3;
				double d;
				char s[8];
			};

			test_struct tests[3] = { { 1, 2, 3, 4.5, "alpha"}, { 10, 20, 30, 45.5, "beta"}, { 100, 200, 300, 455.5, "gamma"} };
			int locations[3] = { -1, -1, -1 };
			int l = 0;
			int c = 0;

			// pack the box until it is full

			while (l >= 0) 
			{
				int i = c % 3;
				l = b->pack(tests[i]);
				if (l >= 0) {
					locations[i] = l;
					c++;
				}
			}

			while (c >= 0)
			{
				int i = c % 3;
				test_struct *ts = b->unpack<test_struct>(locations[i]);
				test_struct& item = tests[i];
				if (ts->i1 != item.i1 ||
					ts->i2 != item.i2 ||
					ts->i3 != item.i3 ||
					ts->d != item.d ||
					strcmp(ts->s, item.s) )
				{
					std::cout << __LINE__ << ": pack " << c << " did not match unpack" << std::endl;
					return false;
				}
				c--;
			}

			return true;
		}

		bool box_tests()
		{
			dynamic_box dbox;
			dbox.init(500);
			if (!test_box(&dbox))
			{
				std::cout << __LINE__ << ": dynamic box failed" << std::endl;
				return false;
			}

			static_box<500> sbox;
			if (!test_box(&sbox))
			{
				std::cout << __LINE__ << ": static box failed" << std::endl;
				return false;
			}
			return true;
		}

	}
}
