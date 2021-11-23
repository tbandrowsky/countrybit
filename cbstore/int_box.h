#pragma once

#include "store_box.h"
#include <ostream>

namespace countrybit
{
	namespace database
	{
		class int_math
		{
		public:

		};

		template <typename integer> 
		requires (std::numeric_limits<integer>::is_integer)
		class integer_box : boxed<integer>
		{
		public:
			integer_box(char* t) : boxed(t)
			{
				;
			}

			integer_box operator = (const integer_box & _src)
			{
				boxed::operator =(_src);
				return *this;
			}

			integer_box operator = (integer _src)
			{
				boxed::operator =(_src);
				return *this;
			}

			operator integer& () { return boxed::operator(); }

			static integer random()
			{
				::random();
			}
		};

		template<typename T> int compare(const integer_box<T>& a, const integer_box<T>& b)
		{
			return (T)a <=> (T)b;
		}

		template<typename T> int operator<(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const integer_box<T>& a, const integer_box<T>& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> int compare(const integer_box<T>& a, T& b)
		{
			return compare(a, boxed(b));
		}

		template<typename T> int operator<(const integer_box<T>& a, T& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const integer_box<T>& a, T& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const integer_box<T>& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const integer_box<T>& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const integer_box<T>& a, T& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const integer_box<T>& a, T& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> std::ostream& operator <<(std::ostream& output, integer_box<T>& src)
		{
			output << (T)src;
			return output;
		}

		using int8_box = integer_box<int8_t>;
		using int16_box = integer_box<int16_t>;
		using int32_box = integer_box<int32_t>;
		using int64_box = integer_box<int64_t>;

	}
}