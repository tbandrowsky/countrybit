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

			bool set_value(database::string_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::string_value:
					dest = string_value;
					break;
				case pvalue_types::double_value:
					dest = double_value;
					break;
				}
			}

			void set_value(database::double_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = double_value;
					break;
				default:
					dest = 0.0;
					break;
				}
			}

			void set_value(database::float_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int8_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int8_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int16_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int16_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int32_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int32_t)double_value;
					break;
				default:
					dest = 0;
					break;
				}
			}

			void set_value(database::int64_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::double_value:
					dest = (int64_t)double_value;
					break;
				}
			}

			void set_value(database::time_box& dest) const
			{
				switch (pvalue_type) {
				case pvalue_types::time_value:
					dest = (int64_t)time_value;
					break;
				}
			}

			const pvalue* next;
		};

		class parray
		{
		public:
			int line;
			int index;

			int num_elements;
			const pvalue* first;
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
			pmember* first;

			int get_type_code() const
			{
				int type_code = 0;
				for (pmember* mb = first; mb; mb = mb->next)
				{
					int mtc = mb->get_type_code();
					type_code = type_code ^ mtc;
				}
				return type_code;
			}
		};

		class pstore
		{
			database::dynamic_box data;

		public:

			pstore(int _size)
			{
				data.init(_size);
			}

			template <typename T> T* allocate()
			{
				char* t = data.place<T>();
				T* d = new (t) T();
				return d;
			}

			const char* allocate(const char* src)
			{
				char* t = data.allocate_extracted(src, 0);
				return t;
			}

			const char* allocate(const char* src, int start, int stop)
			{
				char* t = data.allocate_extracted(src, start, stop, true);
				return t;
			}

		};

	}
}


