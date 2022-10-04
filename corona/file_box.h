#pragma once

namespace corona
{
	namespace database
	{


		class file_box : public boxed<file_instance>
		{
			jobject* slice;
			jschema* schema;
			jclass* the_class;

		public:

			file_box(char* t, jschema* _schema, jclass* _class, jobject* _slice, int _field_idx);
			file_box(file_box& _src);
			file_box operator = (const file_box& _src);
			file_box operator = (file_instance _src);
			operator file_instance& ();

			file_instance value() const;
		};
	}
}
