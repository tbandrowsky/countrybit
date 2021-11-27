
#include "time_box.h"

namespace countrybit
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

		int compare(const time_box& a, time_t& b)
		{
			char data[sizeof(time_t) * 2];
			time_box tb(data);
			tb = b;
			return compare(a, b);
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
			output << src;
			return output;
		}

	}
}