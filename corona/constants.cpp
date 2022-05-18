
#pragma once

#include "pch.h"

namespace corona
{
	namespace database
	{
		block_id block_id::box_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "box", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::sorted_index_node_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "slnode", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::sorted_index_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "sl", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::list_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "lst", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::item_detail_table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "idtble", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::group_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "group", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::table_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "tble", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::expression_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "expr", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::expression_term_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "exprt", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::collection_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "corcol", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::database_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "cordb", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::operator_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "oper", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::value_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "value", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::identifier_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "ident", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::string_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "string", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::number_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "number", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::color_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "color", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::wave_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "wave", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::audio_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "audio", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::rectangle_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "rect", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::point_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "point", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::dimension_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dim", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::datetime_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "dtm", sizeof(box_id.name));
			return box_id;
		}

		block_id block_id::path_id()
		{
			block_id box_id;
			strncpy_s(box_id.name, "path", sizeof(box_id.name));
			return box_id;
		}

		bool block_id::is_box()
		{
			block_id t = block_id::box_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_sorted_index_node()
		{
			block_id t = block_id::sorted_index_node_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_sorted_index()
		{
			block_id t = block_id::sorted_index_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_list()
		{
			block_id t = block_id::list_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_item_detail_table()
		{
			block_id t = block_id::item_detail_table_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_table()
		{
			block_id t = block_id::table_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_group()
		{
			block_id t = block_id::group_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_expression()
		{
			block_id t = block_id::expression_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_collection()
		{
			block_id t = block_id::collection_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_database()
		{
			block_id t = block_id::database_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_operator()
		{
			block_id t = block_id::operator_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_value()
		{
			block_id t = block_id::value_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_identifer()
		{
			block_id t = block_id::identifier_id();
			return strcmp(t.name, name) == 0;
		;
		}

		bool block_id::is_string()
		{
			block_id t = block_id::string_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_number()
		{
			block_id t = block_id::number_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_color()
		{
			block_id t = block_id::color_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_wave()
		{
			block_id t = block_id::wave_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_audio()
		{
			block_id t = block_id::audio_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_rectangle()
		{
			block_id t = block_id::rectangle_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_point()
		{
			block_id t = block_id::point_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_dimension()
		{
			block_id t = block_id::dimension_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_datetime()
		{
			block_id t = block_id::datetime_id();
			return strcmp(t.name, name) == 0;
		}

		bool block_id::is_path()
		{
			block_id t = block_id::path_id();
			return strcmp(t.name, name) == 0;
		}

	}
}
