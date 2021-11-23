#pragma once

#include "store_box.h"
#include <ostream>

namespace countrybit
{
	namespace database
	{
		enum time_models
		{
			seconds,
			days,
			months,
			years
		};

		class time_span
		{
			time_models model;
		public:
			

		};

		class date_time 
		{
		public:
			static int year(time_t t);
			static int month(time_t t);
			static int day(time_t t);
			static int hour(time_t t);
			static int minute(time_t t);
			static int second(time_t t);

			time_t add_seconds(time_t seconds);
			time_t add_minutes(time_t minutes);
			time_t add_hours(time_t hours);
			time_t add_days(time_t days);
			time_t add_weeks(time_t weeks);
			time_t add_months(time_t months);
			time_t add_years(time_t years);
		};

		class time_box : basic_time_box
		{
		public:
			time_box(char* t) : basic_time_box(t)
			{
				;
			}

			time_box operator = (const time_box& _src)
			{
				data = _src.data;
				return *this;
			}

			time_box operator = (time_t _src)
			{
				*data = _src;
				return *this;
			}

			operator time_t& () { return *data; }
			time_t* get_data() { return data; }
		};

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

		template<typename T> std::ostream& operator <<(std::ostream& output, time_box& src)
		{
			output << (T)src;
			return output;
		}

	}
}
