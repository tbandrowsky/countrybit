#pragma once

namespace corona
{
	namespace database
	{

		class jcollection;
		class jdatabase;
		class jschema;
		class jobject;

		class query_box : public boxed<query_instance>
		{
		
			jobject* slice;
			jschema* schema;
			jclass* the_class;
			int field_index;

			relative_ptr_type put_class(query_mapping_type& qd, object_name obj);

		public:

			query_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_index);
			query_box(query_box& _src);
			query_box operator = (const query_box& _src);
			query_box operator = (query_instance _src);
			operator query_instance& ();
			query_instance* operator->();
			query_instance value() const;

			void run(jcollection *_collection);

		};

	}
}
