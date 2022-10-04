#pragma once

namespace corona
{
	namespace database
	{


		class http_box : boxed<http_instance>
		{
			jobject* slice;
			jschema* schema;
			jclass* the_class;

		public:

			http_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx);
			http_box(http_box& _src);
			http_box operator = (const http_box& _src);
			http_box operator = (http_instance _src);
			operator http_instance& ();
			http_instance value() const;
		};

	}
}
