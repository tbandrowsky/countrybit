#pragma once

namespace corona
{
	namespace database
	{

		class sql_box : public boxed<sql_instance>
		{
			jobject* slice;
			jschema* schema;
			jclass* the_class;

		public:

			sql_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx);
			sql_box(sql_box& _src);
			sql_box operator = (const sql_box& _src);
			sql_box operator = (sql_instance _src);
			operator sql_instance& ();
			sql_instance* operator->();
			sql_instance value();
		};

	}
}
