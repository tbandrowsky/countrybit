
#include "query_box.h"
#include "jdatabase.h"

namespace countrybit
{
	namespace database
	{

		template <typename q>
		requires query_path<q>
		class query_math
		{
		public:

			jcollection execute(jdatabase* db, q query_body)
			{

			}
		};

	}
}
