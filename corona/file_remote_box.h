#pragma once

namespace corona
{
	namespace database
	{


		class file_remote_box : public boxed<file_remote_instance>
		{
			jslice* slice;
			jschema* schema;
			jclass* the_class;

		public:

			file_remote_box(char* t, jschema* _schema, jclass* _class, jslice* _slice, int _field_idx);
			file_remote_box(file_remote_box& _src);
			file_remote_box operator = (const file_remote_box& _src);
			file_remote_box operator = (file_remote_instance _src);
			operator file_remote_instance& ();

			file_remote_instance value() const;
		};
	}
}
