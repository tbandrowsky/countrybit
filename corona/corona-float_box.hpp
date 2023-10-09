#ifndef CORONA_FLOAT_BOX_H
#define CORONA_FLOAT_BOX_H

#include "corona-store_box.hpp"

#include <cmath>
#include <numbers>
#include <iostream>
#include <compare>

namespace corona
{

	template <typename floating_point>
		requires (std::is_floating_point<floating_point>::value)
	class float_math
	{
	public:

		static floating_point abs(floating_point d)
		{
			return std::abs(d);
		}

		static floating_point mod(floating_point q, floating_point d)
		{
			return std::fmod(q, d);
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

		static floating_point distance(floating_point m1, floating_point m2)
		{
			return std::abs(m1 - m2);
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

	template <typename floating_point>
		requires (std::is_floating_point<floating_point>::value)
	class fp_box : public boxed<floating_point>
	{
	public:
		fp_box(char* t) : boxed<floating_point>(t)
		{
			;
		}

		fp_box operator = (const fp_box& _src)
		{
			boxed<floating_point>::set_data(_src);
			return *this;
		}

		fp_box operator = (floating_point _src)
		{
			boxed<floating_point>::set_value(_src);
			return *this;
		}

		floating_point abs() const
		{
			return std::abs(*this);
		}

		floating_point is_nan() const
		{
			return std::isnan(*this);
		}

		floating_point is_normal() const
		{
			return std::isnormal(*this);
		}

		floating_point is_unordered() const
		{
			return std::isunordered(*this);
		}

		floating_point exp() const
		{
			return std::exp(*this);
		}

		floating_point exp2() const
		{
			return std::exp2(*this);
		}

		floating_point expm1() const
		{
			return std::expm1(*this);
		}

		floating_point log() const
		{
			return std::log(*this);
		}

		floating_point log2() const
		{
			return std::log2(*this);
		}

		floating_point log10() const
		{
			return std::log10(*this);
		}

		floating_point loglp() const
		{
			return std::log1p(*this);
		}

		floating_point pow(floating_point m2) const
		{
			return std::pow(*this, m2);
		}

		floating_point sqrt() const
		{
			return std::sqrt(*this);
		}

		floating_point cbrt() const
		{
			return std::cbrt(*this);
		}

		floating_point sin() const
		{
			return std::sin(*this);
		}

		floating_point cos() const
		{
			return std::cos(*this);
		}

		floating_point tan() const
		{
			return std::tan(*this);
		}

		floating_point asin() const
		{
			return std::asin(*this);
		}

		floating_point acos() const
		{
			return std::acos(*this);
		}

		floating_point atan() const
		{
			return std::atan(*this);
		}

		floating_point atan2() const
		{
			return std::atan2(*this);
		}

		floating_point sinh() const
		{
			return std::sinh(*this);
		}

		floating_point cosh() const
		{
			return std::cosh(*this);
		}

		floating_point tanh() const
		{
			return std::tanh(*this);
		}

		floating_point asinh() const
		{
			return std::asinh(*this);
		}

		floating_point acosh() const
		{
			return std::acosh(*this);
		}

		floating_point atanh() const
		{
			return std::atanh(*this);
		}

		floating_point tgamma() const
		{
			return std::tgamma(*this);
		}

		floating_point lgamma() const
		{
			return std::lgamma(*this);
		}

		floating_point ceil() const
		{
			return std::ceil(*this);
		}

		floating_point floor() const
		{
			return std::floor(*this);
		}

		floating_point trunc() const
		{
			return std::trunc(*this);
		}

		floating_point round() const
		{
			return std::round(*this);
		}

		floating_point nearbyint() const
		{
			return std::nearbyint(*this);
		}

		floating_point rint() const
		{
			return std::rint(*this);
		}

		operator floating_point& ()
		{
			floating_point& t = boxed<floating_point>::get_data_ref();
			return t;
		}

		operator const floating_point& () const
		{
			floating_point& t = boxed<floating_point>::get_data_ref();
			return t;
		}

		floating_point value() const
		{
			return boxed<floating_point>::get_value();
		}


	};

	template<typename T> std::ostream& operator <<(std::ostream& output, fp_box<T>& src)
	{
		output << (T)src;
		return output;
	}

	using float_box = fp_box<float>;
	using double_box = fp_box<double>;

	double distance(double a, double b)
	{
		return std::abs(a - b);
	}

	int compare(double a, double b)
	{
		return (a <=> b)._Value;
	}

	bool test_doubles()
	{
		char test_buffer64[sizeof(double)];
		char test_buffer32[sizeof(float)];
		char test_buffer32b[sizeof(float)];

		double_box box64(test_buffer64);
		float_box box32(test_buffer32);
		float_box box32b(test_buffer32b);

		box64 = 1024.0;
		box32 = 2048.0;
		box32b = 2048.0;

		if (box64 > box32) return false;
		if (box64 >= box32) return false;
		if (box32 < box64) return false;
		if (box32 <= box64) return false;
		if (box32 == box64) return false;
		if (box32b != box32) return false;
		if (box32b < box32) return false;
		if (box32b > box32) return false;
		if (!(box32b == box32)) return false;
		if (!(box32b >= box32)) return false;
		if (!(box32b <= box32)) return false;

		auto d = distance(box64, box32);
		if (d != 1024) return false;

		box32 = 512;
		box32b = box32;
		if (box32b != box32) return false;

		return true;
	}
}

#endif
