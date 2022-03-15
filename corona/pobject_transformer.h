#pragma once

#include "pobject.h"
#include "store_box.h"

namespace countrybit
{
	namespace system
	{
		class transformer 
		{
		public:

			database::dynamic_box& data;

			transformer(database::dynamic_box& _data) : data(_data)
			{
				;
			}

			transformer() = delete;

			pobject* array_to_object_template(parray* _arr)
			{
				pobject *ret = data.allocate<pobject>(1);
				int column_index = 0;
				for (auto pv = _arr->first; pv; pv = pv->next)
				{
					const char *s = pv->as_string();
					const double* d = pv->as_double();
					char* r = nullptr;
					if (s) 
					{
						r = data.copy(s, 0);
					}
					else if (d)
					{
						database::istring<512> temp;
						temp = *d;
						r = data.copy(temp.c_str(), 0);
					}
					if (r) 
					{
						pmember* member = data.allocate<pmember>(1);
						member->name = r;
						pvalue* new_pv = data.allocate<pvalue>(1);
						new_pv->pvalue_type = pvalue::pvalue_types::double_value;
						new_pv->double_value = column_index;
						member->value = new_pv;
						ret->add(member);
					}
					column_index++;
				}

				return ret;
			}

			pobject* array_to_object(pobject* _template, parray* _arr)
			{
				int column_index = 0;
				pmember* member = _template->first;
				pvalue* pv = _arr->first;
				pobject* ret = data.allocate<pobject>(1);

				while (pv && member) 
				{
					while (member && (column_index > member->value->double_value))
					{
						member = member->next;
					}

					if (member && (column_index == member->value->double_value))
					{
						pmember* new_member = data.allocate<pmember>(1);
						new_member->name = member->name;
						new_member->value = pv;
						ret->add(new_member);
					}

					pv = pv->next;
					column_index++;
				}
				return ret;
			}
		};
	}
}

