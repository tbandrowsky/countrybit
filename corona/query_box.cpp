
#include "corona.h"

namespace corona
{
	namespace database
	{


		query_box::query_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_index) :
			boxed<query_status>(t),
			schema(_schema),
			the_class(_class),
			slice(_slice),
			field_index(_field_index)
		{
			;
		}

		query_box::query_box(query_box& _src) : boxed<query_status>(_src)
		{
			;
		}

		query_box query_box::operator = (const query_box& _src)
		{
			set_data(_src);
			return *this;
		}

		query_box query_box::operator = (query_status _src)
		{
			set_value(_src);
			return *this;
		}

		query_box::operator query_status& ()
		{
			query_status& t = get_data_ref();
			return t;
		}

		query_status* query_box::operator->()
		{
			query_status& t = get_data_ref();
			return &t;
		}

		query_status query_box::value() const 
		{ 
			return get_value(); 
		}


		void query_box::run(jcollection *_collection)
		{
			auto class_field = the_class->detail(field_index);
			auto definition = schema->get_query_definition(class_field.field_id);

		}
	}
}
