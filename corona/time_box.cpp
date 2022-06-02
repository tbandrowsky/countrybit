
#include "corona.h"

namespace corona
{
	namespace database
	{
		int compare(const time_box& a, const time_box& b)
		{
			return (const time_t&)a - (const time_t&)b;
		}

		int operator<(const time_box& a, const time_box& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const time_box& a, const time_box& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const time_box& a, const time_box& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const time_box& a, const time_box& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const time_box& a, const time_box& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const time_box& a, const time_box& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> int operator<(const time_box& a, T& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const time_box& a, T& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const time_box& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const time_box& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const time_box& a, T& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const time_box& a, T& b)
		{
			return compare(a, b) != 0;
		}

		std::ostream& operator <<(std::ostream& output, time_box& src)
		{
			output << (time_t)src;
			return output;
		}

	}
}