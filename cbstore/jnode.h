#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include "skip_list.h"
#include <thread>
#include <atomic>

namespace countrybit
{
	namespace database
	{

		enum jtype
		{
			type_unknown = 0,
			type_int8 = 1,
			type_int16 = 2,
			type_int32 = 3,
			type_int64 = 4,
			type_float32 = 5,
			type_float64 = 6,
			type_datetime = 7,
			type_string = 8,
			type_object = 9
		};

		enum junit
		{
			unit_other = 0,

			// string units
			unit_fullname = 1,
			unit_firstname = 2,
			unit_sublastname = 3,
			unit_lastname = 4,
			unit_middlename = 5,
			unit_ssn = 6,
			unit_email = 7,
			unit_title = 8,
			unit_street = 9,
			unit_substreet = 10,
			unit_city = 11,
			unit_state = 12,
			unit_postal = 13,
			unit_countryname = 21,
			unit_countrycode = 22,
			unit_longname = 23,
			unit_shortname = 24,
			unit_unit = 25,
			unit_code_symbol = 26,
			unit_code_operator = 27,

			// date units
			unit_birthday = 10001,
			unit_scheduled = 10002,

			// int units
			unit_count = 15001,

			// float units
			unit_money = 20001,
			unit_quantity = 20002,
			unit_latitude = 20003,
			unit_longitude = 20004,
			unit_meters = 20005,
			unit_feet = 20006,
			unit_kilograms = 20007,
			unit_pounds = 20008,
			unit_seconds = 20009,
			unit_minutes = 20010,
			unit_hours = 20011,
			unit_amperes = 20012,
			unit_kelvin = 20013,
			unit_mole = 20014,
			unit_candela = 20015
		};

		class jbase
		{
		public:
			std::string name;
			junit node_unit;
			jbase* parent;
		};

		class jobject : public jbase
		{
			void delete_children()
			{
				for (auto child : children)
				{
					if (child.second) {
						delete child.second;
						child.second = nullptr;
					}
				}
				children.clear();
			}

			countrybit::collections::skip_list<std::string, jbase*> children;

		public:

			jobject()
			{
				;
			}

			~jobject()
			{
				delete_children();
			}
		};

		class jarray : public jbase
		{
		public:
			std::vector<jbase*> values;
		};

		template <typename value_type> class jvalue : public jnode
		{
		public:
			value_type value;
		};
	}
}
