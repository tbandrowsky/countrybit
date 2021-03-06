#pragma once

namespace corona
{
	namespace database
	{

		class sql_remote_box : public boxed<sql_remote_instance>
		{
			jobject* slice;
			jschema* schema;
			jclass* the_class;

		public:

			sql_remote_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx);
			sql_remote_box(sql_remote_box& _src);
			sql_remote_box operator = (const sql_remote_box& _src);
			sql_remote_box operator = (sql_remote_instance _src);
			operator sql_remote_instance& ();
			sql_remote_instance* operator->();
			sql_remote_instance value();
		};

	}
}
