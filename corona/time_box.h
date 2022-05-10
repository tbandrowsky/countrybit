#pragma once

#include "store_box.h"
#include <ostream>

namespace corona
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

		class time_box : public basic_time_box
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

			time_box operator = (time_t _src)
			{
				set_value(_src);
				return *this;
			}

			operator time_t& () { return *data; }
			time_t* get_data() { return data; }
			time_t value() const { return *data; }
		};

		std::ostream& operator <<(std::ostream& output, time_box& src);

	}
}
