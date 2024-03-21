#ifndef CORONA_TIME_BOX
#define CORONA_TIME_BOX

#include "corona-windows-lite.h"
#include "corona-store_box.hpp"

#include <iostream>
#include <compare>

namespace corona 
{

	enum time_models
	{
		milliseconds = 0,
		seconds = 1,
		minutes = 2,
		hours = 3,
		days = 4,
		weeks = 5,
		months = 6,
		years = 7
	};

	int64_t time_model_nanos[8] =
	{
		10000i64,
		10000i64 * 1000i64,
		10000i64 * 1000i64 * 60i64,
		10000i64 * 1000i64 * 60i64 * 60i64,
		10000i64 * 1000i64 * 60i64 * 60i64 * 24i64,
		10000i64 * 1000i64 * 60i64 * 60i64 * 24i64 * 7i64,
		10000i64 * 1000i64 * 60i64 * 60i64 * 24i64 * 30i64,
		10000i64 * 1000i64 * 60i64 * 60i64 * 24i64 * 365i64
	};

	class time_span
	{
	public:

		time_models units;
		double value;

		time_span()
		{
			units = time_models::seconds;
			value = 0;
		}

		time_span(double _value, time_models _units)
		{
			value = _value;
			units = _units;
		}
	};

	class date_time
	{
		SYSTEMTIME system_time;

		date_time add(int _sign, time_span _span)
		{
			FILETIME ft;
			LARGE_INTEGER li;
			int elapsed_months;
			int elapsed_years;
			int64_t span_value = _sign * _span.value;

			switch (_span.units) {
			case time_models::milliseconds:
			case time_models::seconds:
			case time_models::minutes:
			case time_models::hours:
			case time_models::days:
			case time_models::weeks:
				::SystemTimeToFileTime(&system_time, &ft);
				li.LowPart = ft.dwLowDateTime;
				li.HighPart = ft.dwHighDateTime;
				li.QuadPart += span_value * time_model_nanos[_span.units];
				ft.dwLowDateTime = li.LowPart;
				ft.dwHighDateTime = li.HighPart;
				::FileTimeToSystemTime(&ft, &system_time);
				break;
			case time_models::months:
				elapsed_months = span_value + system_time.wMonth - 1;
				elapsed_years = system_time.wYear + elapsed_months / 12i64;
				system_time.wMonth = elapsed_months % 12 + 1;
				system_time.wYear = elapsed_years;
				break;
			case time_models::years:
				elapsed_months = 0;
				elapsed_years = span_value + system_time.wYear;
				system_time.wYear = elapsed_years;
				break;
			}

			date_time temp(system_time);
			return temp;
		}

	public:

		static date_time epoch()
		{
			date_time dt(1970, 1, 1);
			return dt;
		}

		static date_time utc_now()
		{
			date_time dt;
			::GetSystemTime(&dt.system_time);
			return dt;
		}

		static date_time now()
		{
			date_time dt;
			::GetLocalTime(&dt.system_time);
			return dt;
		}

		date_time()
		{
			system_time = {};
		}

		date_time(const date_time& _src) = default;

		date_time(SYSTEMTIME st)
		{
			system_time = st;
		}

		date_time(FILETIME ft)
		{
			::FileTimeToSystemTime(&ft, &system_time);
		}

		date_time(time_span ts)
		{
			FILETIME ft;
			LARGE_INTEGER li;

			li.QuadPart = ts.value * time_model_nanos[ts.units];
			ft.dwLowDateTime = li.LowPart;
			ft.dwHighDateTime = li.QuadPart;
			::FileTimeToSystemTime(&ft, &system_time);
		}

		date_time(time_t tt)
		{
			date_time start = epoch();

			FILETIME ft;
			LARGE_INTEGER li;

			::SystemTimeToFileTime(&start.system_time, &ft);
			li.LowPart = ft.dwLowDateTime;
			li.HighPart = ft.dwHighDateTime;
			li.QuadPart += tt * time_model_nanos[ time_models::seconds ];
			ft.dwLowDateTime = li.LowPart;
			ft.dwHighDateTime = li.HighPart;
			::FileTimeToSystemTime(&ft, &system_time);
		}

		date_time(int _year, int _month, int _day, int _hour = 0, int _minute = 0, int _second = 0, int _milliseconds = 0)
		{
			system_time = {};
			system_time.wYear = _year;
			system_time.wMonth = _month;
			system_time.wDay = _day;
			system_time.wHour = _hour;
			system_time.wMinute = _minute;
			system_time.wSecond = _second;
			system_time.wMilliseconds = _milliseconds;
		}

		operator time_span() const
		{
			FILETIME ft;
			LARGE_INTEGER li;
			time_span ts;

			::SystemTimeToFileTime(&system_time, &ft);
			li.LowPart = ft.dwLowDateTime;
			li.HighPart = ft.dwHighDateTime;
			ts.units = time_models::seconds;
			ts.value = li.QuadPart / time_model_nanos[time_models::seconds];
			return ts;
		}

		time_t get_time_t() const
		{
			auto ts_base = (time_span)date_time::epoch();
			auto ts_now = (time_span)*this;

			time_t temp = ts_now.value - ts_base.value;
			return temp;
		}

		date_time& operator =(const date_time& _span) = default;
		date_time& operator =(date_time&& _span) = default;

		operator std::string() const
		{
			std::string temp = std::format("{0:04d}-{1:2d}-{2:2d}T{3}:{4:02d}:{5:02d}.{6:3d}Z",
				system_time.wYear,
				system_time.wMonth,
				system_time.wDay,
				system_time.wHour,
				system_time.wMinute,
				system_time.wSecond,
				system_time.wMilliseconds);

			return temp;
		}

		date_time& operator =(const std::string& _src)
		{
			system_time = {};

			int args = sscanf_s(_src.c_str(), "%04d-%2d-%2dT%d:%d:%d.%dZ",
				&system_time.wMonth,
				&system_time.wDay,
				&system_time.wYear,
				&system_time.wHour,
				&system_time.wMinute,
				&system_time.wSecond,
				&system_time.wMilliseconds
			);

			return *this;
		}

		date_time& operator =(const char *_src)
		{
			system_time = {};

			int args = sscanf_s(_src, "%d/%d/%d %d:%d:%d.%d",
				&system_time.wMonth,
				&system_time.wDay,
				&system_time.wYear,
				&system_time.wHour,
				&system_time.wMinute,
				&system_time.wSecond,
				&system_time.wMilliseconds
				);

			return *this;
		}

		date_time operator +(time_span _span)
		{
			date_time temp = *this;
			temp = temp.add(1, _span);
			return temp;
		}

		date_time operator -(time_span _span)
		{
			date_time temp = *this;
			temp = temp.add(-1, _span);
			return temp;
		}

		date_time operator +=(time_span _span)
		{
			date_time temp = add(1, _span);
			return temp;
		}

		date_time operator -=(time_span _span)
		{
			date_time temp = add(-1, _span);
			return temp;
		}
		
		int year()
		{
			return system_time.wYear;
		}

		int month()
		{
			return system_time.wMonth;
		}

		int day()
		{
			return system_time.wDay;
		}

		int hour()
		{
			return system_time.wHour;
		}

		int minute()
		{
			return system_time.wMinute;
		}

		int second()
		{
			return system_time.wSecond;
		}

		int millisecond()
		{
			return system_time.wMilliseconds;
		}

		int compare(const date_time& _src) const
		{
			int c = 0;
			const WORD* st_me = &system_time.wYear;
			const WORD* st_src = &_src.system_time.wYear;
			while (c < 8) 
			{
				int x = *st_me - *st_src;
				if (x) return x;
				c++;
				st_me++;
				st_src++;
			}
			return 0;
		}
	};

	std::ostream& operator <<(std::ostream& output, date_time& _src)
	{
		std::string temp = (std::string)_src;
		output << temp;
		return output;
	}

	time_span operator-(const date_time& a, const date_time& b)
	{
		time_span at = (time_span)a;
		time_span bt = (time_span)b;

		time_span result = at - bt;
		return result;
	}

	int operator<(const date_time& a, const date_time& b)
	{
		return a.compare(b) < 0;
	}

	int operator>(const date_time& a, const date_time& b)
	{
		return a.compare(b) > 0;
	}

	int operator>=(const date_time& a, const date_time& b)
	{
		return a.compare(b) == 0;
	}

	int operator<=(const date_time& a, const date_time& b)
	{
		return a.compare(b) <= 0;
	}

	int operator==(const date_time& a, const date_time& b)
	{
		return a.compare(b) == 0;
	}

	int operator!=(const date_time& a, const date_time& b)
	{
		return a.compare(b) != 0;
	}

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

		time_box operator = (DATE _src)
		{
			set_value(_src);
			return *this;
		}

		operator DATE& () { return *data; }
		DATE* get_data() { return data; }
		DATE value() const { return *data; }
	};

	std::ostream& operator <<(std::ostream& output, time_box& src);

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

#endif
