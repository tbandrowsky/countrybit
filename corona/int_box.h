#pragma once

#include "store_box.h"
#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		template <typename integer>
		class int_math
		{
		public:
			static bool has_single_bit(integer i)
			{
				return std::has_single_bit(i);
			}
			static integer bit_ceil(integer i)
			{
				return std::bit_ceil(i);
			}
			static integer bit_floor(integer i)
			{
				return std::bit_floor(i);
			}
			static integer bit_width(integer i)
			{
				return std::bit_width(i);
			}
			static integer rotl(integer i)
			{
				return std::rotl(i);
			}
			static integer rotr(integer i)
			{
				return std::rotr(i);
			}
			static integer countl_zero(integer i)
			{
				return std::countl_zero(i);
			}
			static integer countl_one(integer i)
			{
				return std::countl_one(i);
			}
			static integer countr_zero(integer i)
			{
				return std::countr_zero(i);
			}
			static integer countr_one(integer i)
			{
				return std::countr_one(i);
			}
			static integer pop_count(integer i)
			{
				return std::popcount;
			}
			static integer gcd(integer i, integer j)
			{
				return std::gcd(i, j);
			}
			static integer lcm(integer i, integer j)
			{
				return std::lcm(i, j);
			}
			static integer midpoint(integer i, integer j)
			{
				return std::midpoint(i, j);
			}
			static integer lerp(integer i, integer j, integer k)
			{
				return std::lerp(i, j, k);
			}
		};

		template <typename integer> 
		requires (std::numeric_limits<integer>::is_integer)
		class integer_box : public boxed<integer>
		{
		public:
			integer_box(char* t) : boxed<integer>(t)
			{
				;
			}

			integer_box operator = (const integer_box & _src)
			{
				boxed<integer>::operator =(_src);
				return *this;
			}

			integer_box operator = (integer _src)
			{
				boxed<integer>::operator =(_src);
				return *this;
			}

			static integer random()
			{
				return ::random();
			}

			bool has_single_bit()
			{
				return int_math::has_single_bit(*this);
			}
			integer bit_ceil()
			{
				return int_math::bit_ceil(*this);
			}
			integer bit_floor()
			{
				return int_math::bit_floor(*this);
			}
			integer bit_width()
			{
				return int_math::bit_width(*this);
			}
			integer rotl()
			{
				return int_math::rotl(*this);
			}
			integer rotr()
			{
				return int_math::rotr(*this);
			}
			integer countl_zero()
			{
				return int_math::countl_zero(*this);
			}
			integer countl_one()
			{
				return int_math::countl_one(*this);
			}
			integer countr_zero()
			{
				return int_math::countr_zero(*this);
			}
			integer countr_one()
			{
				return int_math::countr_one(*this);
			}
			integer pop_count()
			{
				return int_math::pop_count(*this);
			}

			operator integer& ()
			{
				integer& t = boxed<integer>::get_data_ref();
				return t;
			}

			operator const integer& () const
			{
				const integer& t = boxed<integer>::get_data_ref();
				return t;
			}

			integer value() const { return boxed<integer>::get_value(); }
		};

		using int8_box = integer_box<int8_t>;
		using int16_box = integer_box<int16_t>;
		using int32_box = integer_box<int32_t>;
		using int64_box = integer_box<int64_t>;

		template<typename T> std::ostream& operator <<(std::ostream& output, integer_box<T>& src)
		{
			output << (T)src;
			return output;
		}
	}
} 
