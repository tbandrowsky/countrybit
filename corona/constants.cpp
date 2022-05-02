
#pragma once

#include "constants.h"
#include <cstring>

namespace countrybit
{
	namespace database
	{
		block_id block_id::box()
		{
			block_id box_id;
			strncpy_s(box_id.name, "box", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::sorted_index_node()
		{
			block_id box_id;
			strncpy_s(box_id.name, "slnode", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::sorted_index()
		{
			block_id box_id;
			strncpy_s(box_id.name, "sl", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::item_detail_table()
		{
			block_id box_id;
			strncpy_s(box_id.name, "idtble", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::table()
		{
			block_id box_id;
			strncpy_s(box_id.name, "tble", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::collection()
		{
			block_id box_id;
			strncpy_s(box_id.name, "corcol", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::database()
		{
			block_id box_id;
			strncpy_s(box_id.name, "cordb", sizeof(box_id.name));
			return box_id;
		}

		bool block_id::is_box()
		{
			block_id t = block_id::box();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_sorted_index_node()
		{
			block_id t = block_id::sorted_index_node();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_sorted_index()
		{
			block_id t = block_id::sorted_index();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_item_detail_table()
		{
			block_id t = block_id::item_detail_table();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_table()
		{
			block_id t = block_id::table();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_collection()
		{
			block_id t = block_id::collection();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_database()
		{
			block_id t = block_id::database();
			return strcmp(t.name, name) == 0;
		}

	}
}
