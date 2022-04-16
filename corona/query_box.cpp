
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

			using filter_details_type = item_details_table<row_id_type, filter_element>;
			filter_details_type filters;

			virtual void on_node(jslice& _slice)
			{
				dest_slice.copy(_slice);
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
				auto& fld = _schema->get_field(fldref.field_id);

				collection_id_type collection_id;
				init_collection_id(collection_id);

				query_definition_type query_copy = _schema->get_query_definition(fld.query_properties.properties_id);

				jclass_header* root_hdr;
				jslice root_slice;

				root_hdr = _class->pitem();

				while (root_hdr && root_hdr->class_id != query_copy.source_path.root.model_id)
				{
					root_model = _slice->get_parent_model();
					if (root_model.is_empty()) {
						return;
					}
					root_slice = root_model.get_model_slice();
					root_hdr = root_slice.get_class().pitem();
				}

				if (!root_hdr)
					return;

				auto result_field = _schema->get_field(query_copy.result_field_id);
				auto result_class = _schema->get_class(query_copy.result_class_id);

				int64_t size_bytes = root_hdr->class_size_bytes * 5 / 2;
				int64_t estimated_rows = root_hdr->class_size_bytes / result_class.pitem()->class_size_bytes;

				data.init(size_bytes);

				collection = _schema->create_collection(&data, collection_id, 2, root_hdr->class_size_bytes);
				target = collection.create_list(query_copy.result_class_id, estimated_rows);

			    visit(*_slice, root_slice, query_copy);
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
