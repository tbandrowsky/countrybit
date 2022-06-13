#pragma once

namespace corona
{
	namespace database
	{


		class http_remote_box : boxed<http_remote_instance>
		{
			jobject* slice;
			jschema* schema;
			jclass* the_class;

		public:

			http_remote_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx);
			http_remote_box(http_remote_box& _src);
			http_remote_box operator = (const http_remote_box& _src);
			http_remote_box operator = (http_remote_instance _src);
			operator http_remote_instance& ();
			http_remote_instance value() const;
		};

	}
}
