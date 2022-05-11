#pragma once

namespace corona 
{
	namespace database
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

			pvalue* next_link;
		};

		class parray
		{
		public:
			int line;
			int index;

			int num_elements;
			pvalue* first_link, *last_link;

			void add(pvalue* pv)
			{
				num_elements++;
				if (!first_link) {
					first_link = last_link = pv;
				}
				else {
					last_link->next_link = pv;
					last_link = pv;
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
			pmember* next_link;

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
			pmember* first_link, *last_link;
			pmember* type_member;

			pmember* get_member(const char* _name) const
			{
				for (pmember* mb = first_link; mb; mb = mb->next_link)
				{
					if (_stricmp(mb->name,_name) == 0)
						return mb;
				}
				return nullptr;
			}

			void add(pmember* pv)
			{
				num_members++;
				if (!first_link) {
					first_link = last_link = pv;
				}
				else {
					last_link->next_link = pv;
					last_link = pv;
				}
			}

		};


	}
}


