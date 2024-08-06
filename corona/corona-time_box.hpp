/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_TIME_BOX
#define CORONA_TIME_BOX

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

		date_time(struct std::tm _tm)
		{
			system_time = {};
			system_time.wYear = _tm.tm_year + 1900;
			system_time.wMonth = _tm.tm_mon;
			system_time.wDay = _tm.tm_mday;
			system_time.wHour = _tm.tm_hour;
			system_time.wMinute = _tm.tm_min;
			system_time.wSecond = _tm.tm_sec;
			system_time.wMilliseconds = 0;
		}

		date_time& operator = (SYSTEMTIME st)
		{
			system_time = st;
			return *this;
		}

		date_time& operator = (FILETIME ft)
		{
			::FileTimeToSystemTime(&ft, &system_time);
			return *this;
		}

		date_time& operator = (time_span ts)
		{
			FILETIME ft;
			LARGE_INTEGER li;

			li.QuadPart = ts.value * time_model_nanos[ts.units];
			ft.dwLowDateTime = li.LowPart;
			ft.dwHighDateTime = li.QuadPart;
			::FileTimeToSystemTime(&ft, &system_time);
			return *this;
		}

		date_time& operator = (time_t tt)
		{
			date_time start = epoch();

			FILETIME ft;
			LARGE_INTEGER li;

			::SystemTimeToFileTime(&start.system_time, &ft);
			li.LowPart = ft.dwLowDateTime;
			li.HighPart = ft.dwHighDateTime;
			li.QuadPart += tt * time_model_nanos[time_models::seconds];
			ft.dwLowDateTime = li.LowPart;
			ft.dwHighDateTime = li.HighPart;
			::FileTimeToSystemTime(&ft, &system_time);
			return *this;
		}

		date_time &operator = (struct std::tm _tm)
		{
			system_time = {};
			system_time.wYear = _tm.tm_year + 1900;
			system_time.wMonth = _tm.tm_mon + 1;
			system_time.wDay = _tm.tm_mday;
			system_time.wHour = _tm.tm_hour;
			system_time.wMinute = _tm.tm_min;
			system_time.wSecond = _tm.tm_sec;
			system_time.wMilliseconds = 0;
			return *this;
		}

		date_time& operator = (date_time &_src)
		{
			system_time = _src.system_time;
			return *this;
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

		operator struct tm() const
		{
			struct tm tmx = {};

			if (system_time.wYear) 
			{
				tmx.tm_year = system_time.wYear - 1900;
				tmx.tm_mon = system_time.wMonth - 1;
				tmx.tm_mday = system_time.wDay;
				tmx.tm_hour = system_time.wHour;
				tmx.tm_min = system_time.wMinute;
				tmx.tm_sec = system_time.wSecond;
				tmx.tm_isdst = -1;
			}
			else {
				tmx.tm_year = 0;
				tmx.tm_mon = 0;
				tmx.tm_mday = 1;
				tmx.tm_hour = 0;
				tmx.tm_min = 0;
				tmx.tm_sec = 0;
				tmx.tm_isdst = 0;
			}

			return tmx;
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

		date_time parse(std::string _src)
		{
			std::string formats[8] = {
				"%m/%d/%Y",
				"%Y-%m-%d",
				"%m/%d/%Y %H:%M:%S",
				"%Y-%m-%d %H:%M:%S",
				"%m/%d/%Y %H:%M",
				"%Y-%m-%d %H:%M",
				"%Y-%m-%dT%H:%M:%S",
				"%Y-%m-%dT%H:%M:%SZ"
			};

			for (auto format : formats) {
				std::istringstream ss(_src);
				std::tm tm = {};
				ss >> std::get_time(&tm, format.c_str());
				if (!ss.fail())
				{
					*this = tm;
					break;
				}
			}

			return *this;
		}

		std::string short_date() const
		{
			std::string temp;

			temp = std::format("{0}/{1}/{2}", system_time.wMonth, system_time.wDay, system_time.wYear);

			return temp;
		}

		operator std::string() const
		{
			char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
			struct tm tmx = (struct tm)(*this);
			std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", &tmx);
			std::string temp = timeString;
			return temp;
		}

		std::string format(std::string _format)
		{
			char time_buff[256] = {};
			strncpy_s(time_buff, _format.c_str(), _TRUNCATE);
			struct tm tmx = (struct tm)(*this);
			std::strftime(time_buff, std::size(time_buff), _format.c_str(), &tmx);
			std::string temp = time_buff;
			return temp;
		}

		date_time& operator =(const std::string& _src)
		{
			parse(_src);
			return *this;
		}

		date_time& operator =(const char *_src)
		{
			parse(_src);
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

		operator SYSTEMTIME& ()
		{
			return system_time;
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

		double temp = at.units - bt.units;
		time_span result(temp, time_models::seconds);
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

	template <typename T, int size_items> class ring_buffer
	{
		T items[size_items];
		volatile unsigned int counter;

		int get_next_id()
		{
			::InterlockedIncrement(&counter);
			return counter % size_items;
		}

	public:

		ring_buffer() : counter(0)
		{
			;
		}

		int get_capacity()
		{
			return size_items;
		}

		int get_size()
		{
			if (counter < size_items)
				return counter;
			else
				return size_items;
		}

		auto& get_items()
		{
			return items;
		}

		void put(T& item)
		{
			int newidx = get_next_id();
			items[newidx] = item;
		}

		T& get(unsigned int _idx)
		{
			return items[_idx % size_items];
		}

	};

	class timer
	{
		LARGE_INTEGER last_time;
		LARGE_INTEGER frequency;
	public:
		
		timer()
		{
			::QueryPerformanceFrequency(&frequency);
			::QueryPerformanceCounter(&last_time);
		}

		double get_elapsed_seconds()
		{
			LARGE_INTEGER current_time;
			::QueryPerformanceCounter(&current_time);
			double elapsed = (double)(current_time.QuadPart - last_time.QuadPart) / (double)frequency.QuadPart;
			last_time = current_time;
			return elapsed;
		}

		bool check(double _elapsed_seconds)
		{
			LARGE_INTEGER current_time;
			::QueryPerformanceCounter(&current_time);
			double elapsed = (double)(current_time.QuadPart - last_time.QuadPart) / (double)frequency.QuadPart;
			if (elapsed > _elapsed_seconds) {
				last_time = current_time;
				return true;
			}
			return false;
		}
	};

}

#endif
