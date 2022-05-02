
#include "query_box.h"
#include "jdatabase.h"
#include "jfield.h"
#include "navigator.h"

namespace countrybit
{
	namespace database
	{

		class query_navigator : public navigator
		{

			jslice		dest_slice;
			jlist		target;
			jcollection collection;

			using filter_details_type = item_details_table<relative_ptr_type, filter_element>;
			filter_details_type filters;

			virtual void on_node(jslice& _slice)
			{
				dest_slice.update(_slice);
			}

			virtual void on_tail(jslice& _slice)
			{
				dest_slice = target.append_slice();
			}


		public:

			query_navigator()
			{
				;
			}

			void run(jschema* _schema, jclass* _class, jslice* _slice, int _field_index)
			{
				filters.clear();

				auto& fldref = _class->detail(_field_index);

				collection_id_type collection_id;
				init_collection_id(collection_id);

				query_definition_type query_copy = _schema->get_query_definition(fldref.field_id);

			}
		};


		query_box::query_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_index) :
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

		void query_box::run()
		{
			query_navigator runner;

			runner.run(schema, the_class, slice, field_index);
		}
	}
}
