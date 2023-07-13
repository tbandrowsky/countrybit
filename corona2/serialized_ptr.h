#pragma once

#include <vector>

namespace corona
{
	class serialized_ptr
	{
		long object_id;
	public:
	};

	class block_service 
	{
	public:

		virtual void store_value(store) = 0;
		virtual block_id_type get_value(block_id_type _block_id) = 0;
	};

}