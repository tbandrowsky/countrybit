#pragma once

#include "store_box.h"
#include <ostream>
#include <cmath>
#include <numbers>

#undef max
#undef min

namespace countrybit
{
	namespace database
	{

		template <class floating_point>
		class float_math
		{
		public:

			static floating_point abs(floating_point d)
			{
				return std::abs(d);
			}
			static floating_point mod(floating_point q, floating_point d)
			{
				return std::fmod(q,d);
			}
			static floating_point rem(floating_point q, floating_point d)
			{
				return std::remainder(q, d);
			}
			static floating_point fma(floating_point m1, floating_point m2, floating_point a)
			{
				return std::fma(m1, m2, a);
			}
			static floating_point max(floating_point m1, floating_point m2)
			{
				return std::max(m1, m2);
			}
			static floating_point min(floating_point m1, floating_point m2)
			{
				return std::min(m1, m2);
			}
			static floating_point dim(floating_point m1, floating_point m2)
			{
				return std::fdim(m1, m2);
			}

			static floating_point is_nan(floating_point m1)
			{
				return std::isnan(m1);
			}
			static floating_point is_normal(floating_point m1)
			{
				return std::isnormal(m1);
			}
			static floating_point is_unordered(floating_point m1)
			{
				return std::isunordered(m1);
			}

			static floating_point exp(floating_point m1)
			{
				return std::exp(m1);
			}

			static floating_point exp2(floating_point m1)
			{
				return std::exp2(m1);
			}

			static floating_point expm1(floating_point m1)
			{
				return std::expm1(m1);
			}

			static floating_point log(floating_point m1)
			{
				return std::log(m1);
			}

			static floating_point log2(floating_point m1)
			{
				return std::log2(m1);
			}

			static floating_point log10(floating_point m1)
			{
				return std::log10(m1);
			}

			static floating_point loglp(floating_point m1)
			{
				return std::log1p(m1);
			}

			static floating_point pow(floating_point m1, floating_point m2)
			{
				return std::pow(m1, m2);
			}

			static floating_point sqrt(floating_point m1)
			{
				return std::sqrt(m1);
			}

			static floating_point cbrt(floating_point m1)
			{
				return std::cbrt(m1);
			}

			static floating_point hypot(floating_point m1, floating_point m2)
			{
				return std::hypot(m1, m2);
			}

			static floating_point hypot(floating_point m1, floating_point m2, floating_point m3)
			{
				return std::hypot(m1, m2, m3);
			}

			static floating_point sin(floating_point m1)
			{
				return std::sin(m1);
			}

			static floating_point cos(floating_point m1)
			{
				return std::cos(m1);
			}

			static floating_point tan(floating_point m1)
			{
				return std::tan(m1);
			}

			static floating_point asin(floating_point m1)
			{
				return std::asin(m1);
			}

			static floating_point acos(floating_point m1)
			{
				return std::acos(m1);
			}

			static floating_point atan(floating_point m1)
			{
				return std::atan(m1);
			}

			static floating_point atan2(floating_point m1)
			{
				return std::atan2(m1);
			}

			static floating_point sinh(floating_point m1)
			{
				return std::sinh(m1);
			}

			static floating_point cosh(floating_point m1)
			{
				return std::cosh(m1);
			}

			static floating_point tanh(floating_point m1)
			{
				return std::tanh(m1);
			}

			static floating_point asinh(floating_point m1)
			{
				return std::asinh(m1);
			}

			static floating_point acosh(floating_point m1)
			{
				return std::acosh(m1);
			}

			static floating_point atanh(floating_point m1)
			{
				return std::atanh(m1);
			}

			static floating_point tgamma(floating_point m1)
			{
				return std::tgamma(m1);
			}

			static floating_point lgamma(floating_point m1)
			{
				return std::lgamma(m1);
			}

			static floating_point ceil(floating_point m1)
			{
				return std::ceil(m1);
			}

			static floating_point floor(floating_point m1)
			{
				return std::floor(m1);
			}

			static floating_point trunc(floating_point m1)
			{
				return std::trunc(m1);
			}

			static floating_point round(floating_point m1)
			{
				return std::round(m1);
			}

			static floating_point nearbyint(floating_point m1)
			{
				return std::nearbyint(m1);
			}

			static floating_point rint(floating_point m1)
			{
				return std::rint(m1);
			}

			const double pi = std::numbers::pi;
			const double inv_pi = std::numbers::inv_pi;
			const double e = std::numbers::e;
			const double egamma = std::numbers::egamma;

		};

		template <class floating_point> 
		requires (std::numeric_limits<floating_point>::is_floating_point)
		class fp_box : boxed<floating_point>
		{
		public:
			fp_box(char* t) : boxed(t)
			{
				;
			}

			fp_box operator = (const fp_box & _src)
			{
				boxed::operator=(_src);
				return *this;
			}

			fp_box operator = (floating_point _src)
			{
				boxed::operator=(_src);
				return *this;
			}

			floating_point abs()
			{
				return std::abs(*this);
			}
			floating_point mod(floating_point d)
			{
				return std::fmod(*this, d);
			}
			floating_point rem(floating_point d)
			{
				return std::remainder(*this, d);
			}
			floating_point fma(floating_point m2, floating_point a)
			{
				return std::fma(*this, m2, a);
			}
			floating_point max(floating_point m2)
			{
				return std::max(*this, m2);
			}
			floating_point min(floating_point m2)
			{
				return std::min(*this, m2);
			}
			floating_point dim(floating_point m2)
			{
				return std::fdim(*this, m2);
			}

			floating_point is_nan()
			{
				return std::isnan(*this);
			}
			floating_point is_normal()
			{
				return std::isnormal(*this);
			}
			floating_point is_unordered()
			{
				return std::isunordered(*this);
			}

			floating_point exp()
			{
				return std::exp(*this);
			}

			floating_point exp2()
			{
				return std::exp2(*this);
			}

			floating_point expm1()
			{
				return std::expm1(*this);
			}

			floating_point log()
			{
				return std::log(*this);
			}

			floating_point log2()
			{
				return std::log2(*this);
			}

			floating_point log10()
			{
				return std::log10(*this);
			}

			floating_point loglp()
			{
				return std::log1p(*this);
			}

			floating_point pow(floating_point m2)
			{
				return std::pow(*this, m2);
			}

			floating_point sqrt()
			{
				return std::sqrt(*this);
			}

			floating_point cbrt()
			{
				return std::cbrt(*this);
			}

			floating_point hypot(floating_point m2)
			{
				return std::hypot(*this, m2);
			}

			floating_point hypot(floating_point m2, floating_point m3)
			{
				return std::hypot(*this, m2, m3);
			}

			floating_point sin()
			{
				return std::sin(*this);
			}

			floating_point cos()
			{
				return std::cos(*this);
			}

			floating_point tan()
			{
				return std::tan(*this);
			}

			floating_point asin()
			{
				return std::asin(*this);
			}

			floating_point acos()
			{
				return std::acos(*this);
			}

			floating_point atan()
			{
				return std::atan(*this);
			}

			floating_point atan2()
			{
				return std::atan2(*this);
			}

			floating_point sinh()
			{
				return std::sinh(*this);
			}

			floating_point cosh()
			{
				return std::cosh(*this);
			}

			floating_point tanh()
			{
				return std::tanh(*this);
			}

			floating_point asinh()
			{
				return std::asinh(*this);
			}

			floating_point acosh()
			{
				return std::acosh(*this);
			}

			floating_point atanh()
			{
				return std::atanh(*this);
			}

			floating_point tgamma()
			{
				return std::tgamma(*this);
			}

			floating_point lgamma()
			{
				return std::lgamma(*this);
			}

			floating_point ceil()
			{
				return std::ceil(*this);
			}

			floating_point floor()
			{
				return std::floor(*this);
			}

			floating_point trunc()
			{
				return std::trunc(*this);
			}

			floating_point round()
			{
				return std::round(*this);
			}

			floating_point nearbyint()
			{
				return std::nearbyint(*this);
			}

			floating_point rint()
			{
				return std::rint(*this);
			}

			operator floating_point& () { return boxed::operator(); }
		};

		template<typename T> int compare(const fp_box<T>& a, const fp_box<T>& b)
		{
			return (T)a <=> (T)b;
		}

		template<typename T> int operator<(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const fp_box<T>& a, const fp_box<T>& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> int compare(const fp_box<T>& a, T& b)
		{
			return compare(a, boxed(b));
		}

		template<typename T> int operator<(const fp_box<T>& a, T& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const fp_box<T>& a, T& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const fp_box<T>& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const fp_box<T>& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const fp_box<T>& a, T& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const fp_box<T>& a, T& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> std::ostream& operator <<(std::ostream& output, fp_box<T>& src)
		{
			output << (T)src;
			return output;
		}

		using float_box = fp_box<float>;
		using double_box = fp_box<double>;

	}
}
