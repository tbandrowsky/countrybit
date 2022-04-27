#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include "messages.h"
#include "store_box.h"
#include "string_box.h"
#include "time_box.h"
#include "sorted_index.h"
#include "color_box.h"
#include "image_box.h"
#include "point_box.h"
#include "rectangle_box.h"
#include "wave_box.h"
#include "midi_box.h"
#include "object_id_box.h"
#include "collection_id_box.h"

#include <functional>

namespace countrybit 
{
	namespace system 
	{

		class pobject;
		class parray;
		class pvalue;

		class pvalue
		{
		public:

			int line;
			int index;

			enum class pvalue_types
			{
				string_value,
				double_value,
				time_value,
				array_value,
				object_value
			};

			pvalue_types pvalue_type;

			const char* string_value;
			double double_value;
			time_t time_value;
			const pobject* object_value;
			const parray* array_value;

			int x, y, z;

			const char* as_string() const
			{
				return (pvalue_type == pvalue_types::string_value) ? string_value : nullptr;
			}

			const double* as_double() const
			{
				return (pvalue_type == pvalue_types::double_value) ? &double_value : nullptr;
			}

			const pobject* as_object() const
			{
				return (pvalue_type == pvalue_types::object_value) ? object_value : nullptr;
			}

			const parray* as_array() const
			{
				return (pvalue_type == pvalue_types::array_value) ? array_value : nullptr;
			}

			void set_value(database::string_box& dest) const;
			void set_value(database::double_box& dest) const;
			void set_value(database::float_box& dest) const;
			void set_value(database::int8_box& dest) const;
			void set_value(database::int16_box& dest) const;
			void set_value(database::int32_box& dest) const;
			void set_value(database::int64_box& dest) const;
			void set_value(database::time_box& dest) const;
			void set_value(database::object_id_box& dest) const;
			void set_value(database::collection_id_box& dest) const;
			void set_value(database::color_box& dest) const;
			void set_value(database::point_box& dest) const;
			void set_value(database::rectangle_box& dest) const;
			void set_value(database::image_box& dest) const;
			void set_value(database::wave_box& dest) const;
			void set_value(database::midi_box& dest) const;

			bool is_container() const
			{
				return (pvalue_type == pvalue_types::array_value || pvalue_type == pvalue_types::object_value);
			}

			bool is_scalar() const
			{
				return !is_container();
			}

			pvalue* next;
		};

		class parray
		{
		public:
			int line;
			int index;

			int num_elements;
			pvalue* first, *last;

			void add(pvalue* pv)
			{
				num_elements++;
				if (!first) {
					first = last = pv;
				}
				else {
					last->next = pv;
					last = pv;
				}
			}
		};

		class pmember
		{
		public:
			int line;
			int index;

			const char* name;
			const pvalue* value;
			pmember* next;

			int get_type_code() const
			{
				int code = 17;
				for (const char* c = name; *c; c++)
				{
					code = code * 23 + *c;
				}
				return code;
			}
		};

		class pobject
		{
		public:
			int line;
			int index;

			int num_members;
			pmember* first, *last;
			pmember* type_member;

			pmember* get_member(const char* _name) const
			{
				for (pmember* mb = first; mb; mb = mb->next)
				{
					if (_stricmp(mb->name,_name) == 0)
						return mb;
				}
				return nullptr;
			}

			void add(pmember* pv)
			{
				num_members++;
				if (!first) {
					first = last = pv;
				}
				else {
					last->next = pv;
					last = pv;
				}
			}

		};


	}
}


