
#include "corona.h"

namespace corona
{
	namespace database
	{


		query_box::query_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_index) :
			boxed<query_instance>(t),
			schema(_schema),
			the_class(_class),
			slice(_slice),
			field_index(_field_index)
		{
			;
		}

		query_box::query_box(query_box& _src) : boxed<query_instance>(_src)
		{
			;
		}

		query_box query_box::operator = (const query_box& _src)
		{
			set_data(_src);
			return *this;
		}

		query_box query_box::operator = (query_instance _src)
		{
			set_value(_src);
			return *this;
		}

		query_box::operator query_instance& ()
		{
			query_instance& t = get_data_ref();
			return t;
		}

		query_instance* query_box::operator->()
		{
			query_instance& t = get_data_ref();
			return &t;
		}

		query_instance query_box::value() const 
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
