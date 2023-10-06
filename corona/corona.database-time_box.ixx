module;

#include <iostream>
#include <compare>

export module corona.database:time_box;
import "corona.database-windows-lite.h";

import :store_box;

export enum time_models
		{
			seconds,
			days,
			months,
			years
		};

export class time_span
		{
			time_models model;
		public:


		};

export class date_time
		{
		public:
			static int year(DATE t);
			static int month(DATE t);
			static int day(DATE t);
			static int hour(DATE t);
			static int minute(DATE t);
			static int second(DATE t);

			DATE add_seconds(DATE seconds);
			DATE add_minutes(DATE minutes);
			DATE add_hours(DATE hours);
			DATE add_days(DATE days);
			DATE add_weeks(DATE weeks);
			DATE add_months(time_t months);
			DATE add_years(time_t years);
		};

export class time_box : public basic_time_box
		{
		public:
			time_box(char* t) : basic_time_box(t)
			{
				;
			}

			time_box operator = (const time_box& _src)
			{
				set_data(_src);
				return *this;
			}

			time_box operator = (DATE _src)
			{
				set_value(_src);
				return *this;
			}

			operator DATE& () { return *data; }
			DATE* get_data() { return data; }
			DATE value() const { return *data; }
		};

export std::ostream& operator <<(std::ostream& output, time_box& src);

		export int compare(const time_box& a, const time_box& b)
		{
			return (const time_t&)a - (const time_t&)b;
		}

		export int operator<(const time_box& a, const time_box& b)
		{
			return compare(a, b) < 0;
		}

		export int operator>(const time_box& a, const time_box& b)
		{
			return compare(a, b) > 0;
		}

		export int operator>=(const time_box& a, const time_box& b)
		{
			return compare(a, b) >= 0;
		}

		export int operator<=(const time_box& a, const time_box& b)
		{
			return compare(a, b) <= 0;
		}

		export int operator==(const time_box& a, const time_box& b)
		{
			return compare(a, b) == 0;
		}

		export int operator!=(const time_box& a, const time_box& b)
		{
			return compare(a, b) != 0;
		}

		export template<typename T> int operator<(const time_box& a, T& b)
		{
			return compare(a, b) < 0;
		}

		export template<typename T> int operator>(const time_box& a, T& b)
		{
			return compare(a, b) > 0;
		}

		export template<typename T> int operator>=(const time_box& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		export template<typename T> int operator<=(const time_box& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		export template<typename T> int operator==(const time_box& a, T& b)
		{
			return compare(a, b) == 0;
		}

		export template<typename T> int operator!=(const time_box& a, T& b)
		{
			return compare(a, b) != 0;
		}

		export std::ostream& operator <<(std::ostream& output, time_box& src)
		{
			output << (time_t)src;
			return output;
		}

