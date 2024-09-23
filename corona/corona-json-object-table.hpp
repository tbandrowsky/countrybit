#pragma once
/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This is a table in a json database, implemented with a skip list.

Notes

For Future Consideration
*/


#ifndef CORONA_JSON_OBJECT_TABLE_H
#define CORONA_JSON_OBJECT_TABLE_H

namespace corona
{
	const int debug_json_object_table = 0;

	class json_object_key_node : public data_block
	{
	public:

		int64_t										object_id;
		iarray<int64_t, JsonTableMaxNumberOfLevels> foward;
		int64_t										json_location;

		json_object_key_node() = default;
		json_object_key_node(const json_object_key_node& _src) = default;
		json_object_key_node(json_object_key_node&& _src) = default;

		virtual ~json_object_key_node()
		{
			;
		}

		json_object_key_node& operator = (const json_object_key_node& _src) = default;
		json_object_key_node& operator = (json_object_key_node&& _src) = default;

		void clear()
		{
			foward.clear();
			json_location = 0;
			object_id = 0;
		}

		virtual void on_read()
		{
			clear();
			char* base = bytes.get_ptr();
			int16_t* foward_count = (int16_t*)base;
			base += sizeof(int16_t);
			int64_t* ptr = (int64_t*)base;
			for (int i = 0; i < *foward_count; i++) {
				foward.push_back(*ptr);
				ptr++;
			}
			json_location = *ptr;
			ptr++;
			object_id = *ptr;
		}

		virtual void on_write()
		{
			int32_t size_bytes =
				sizeof(int16_t) +
				sizeof(int64_t) * foward.size() +
				sizeof(json_location) +
				sizeof(object_id);

			bytes.init(size_bytes);
			char* base = bytes.get_ptr();
			int16_t* foward_count = (int16_t*)base;
			*foward_count = foward.size();
			base += sizeof(int16_t);
			int64_t* ptr = (int64_t*)base;
			for (int i = 0; i < foward.size(); i++) {
				*ptr = foward[i];
				ptr++;
			}
			*ptr = json_location;
			ptr++;
			*ptr = object_id;
		}

		json_data_node get_node(file_block* _file)
		{
			json_data_node node_to_read;
			node_to_read.read(_file, json_location);
			return node_to_read;
		}

	};

	class json_object_table
	{
	private:

		lockable						space_lock;
		json_table_header				table_header;

		using KEY = int64_t;
		using VALUE = json_node;
		using UPDATE_VALUE = json;
		const int SORT_ORDER = 1;

		file_block* fb;

		relative_ptr_type create_header()
		{
			json_parser jp;
			table_header.append(fb);

			json_object_key_node header = create_node(JsonTableMaxLevel);
			table_header.data.data_root_location = header.header.block_location;
			table_header.data.count = 0;
			table_header.data.level = JsonTableMaxLevel;
			table_header.write(fb);
			header.object_id = 0;
			header.write(fb);
			return table_header.header.block_location;
		}

		json_object_key_node get_header()
		{
			json_object_key_node in;

			auto p = table_header.data.data_root_location;
			if (p <= 0) {
				throw std::exception("table header node location not set");
			}

			int64_t result = in.read(fb, table_header.data.data_root_location);

			if (result < 0)
			{
				throw std::logic_error("Couldn't read table header.");
			}

			return in;
		}

		json_object_key_node create_node(int _max_level)
		{
			json_object_key_node new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.foward.push_back(rit);
			}

			new_node.append(fb);

			return new_node;
		}

		json_object_key_node get_key_node(relative_ptr_type _key_node_location)
		{
			json_object_key_node node;

			node.read(fb, _key_node_location);
			return node;
		}

		void free_node(relative_ptr_type _key_node_location)
		{
			json_object_key_node node;
			node.read(fb, _key_node_location);
			node.erase(fb);

			json_data_node json_data;
			json_data.read(fb, node.json_location);
			json_data.erase(fb);
		}

		// compare a node to a key for equality
		// return -1 if the node < key
		// return 1 if the node > key
		// return 0 if the node == key

		relative_ptr_type compare_node(json_object_key_node& _nd, KEY _id_key)
		{

			if (_nd.header.block_location == table_header.data.data_root_location)
				return -1;

			if (_nd.object_id < _id_key)
				return -SORT_ORDER;
			else if (_nd.object_id > _id_key)
				return SORT_ORDER;
			else
				return 0;
		}

		relative_ptr_type find_advance(json_object_key_node& _node, json_object_key_node& _peek, int _level, KEY _key, int64_t* _found)
		{
			auto t = _node.foward[_level];
			if (t != null_row) {
				_peek.read(fb, t);
				int comp = compare_node(_peek, _key);
				if (comp < 0)
					return t;
				else if (comp == 0) {
					*_found = t;
				}
			}
			return -1;
		}

		relative_ptr_type find_node(relative_ptr_type* update, KEY _key, json_object_key_node& _found_node)
		{
			relative_ptr_type found = null_row;

			json_object_key_node x = get_header();

			for (int k = table_header.data.level; k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance(x, _found_node, k, _key, &found); // TODO Found node could be here.
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance(x, _found_node, k, _key, &found);
				}
				update[k] = x.header.block_location;
			}

			return found;
		}

		relative_ptr_type find_first_gte(KEY _key, json_object_key_node& _found_node)
		{
			relative_ptr_type found = null_row, last_link;
			json_parser jp;

			json_object_key_node x = get_header();

			for (int k = table_header.data.level; k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance(x, _found_node, k, _key, &found);
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance(x, _found_node, k, _key, &found);
				}
			}

			return found;
		}

		relative_ptr_type update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			json_object_key_node header = get_header();

			relative_ptr_type location = header.foward[0];

			json_object_key_node jkn;

			relative_ptr_type q = find_node(update, _key, jkn);

			if (q != null_row)
			{
				json_data_node qnd;
				qnd = jkn.get_node(fb);
				predicate(qnd.data);
				qnd.write(fb);
				return q;
			}

			json_object_key_node qnd;

			k = randomLevel();
			if (k > table_header.data.level)
			{
				table_header.data.level++;
				k = table_header.data.level;
				update[k] = header.header.block_location;
			}

			UPDATE_VALUE initial_value;
			try {
				predicate(initial_value);
			}
			catch (std::exception exc)
			{
				std::cout << __FILE__ << " " << __LINE__ << ":Initialization of new object failed when inserting node into table." << std::endl;
				return -1;
			}

			json_object_key_node key_node = create_node(k);

			json_data_node value_node;
			value_node.data = initial_value;
			key_node.json_location = value_node.append(fb);
			key_node.object_id = _key;

			table_header.data.count++;

			do
			{
				json_object_key_node pnd = get_key_node(update[k]);
				key_node.foward[k] = pnd.foward[k];
				pnd.foward[k] = key_node.header.block_location;
				pnd.write(fb);

			} while (--k >= 0);

			key_node.write(fb);
			table_header.write(fb);

			return key_node.header.block_location;
		}

		relative_ptr_type find_first_gte(relative_ptr_type* update, KEY _key)
		{
			relative_ptr_type found = null_row, p, q, last_link;

			json_object_key_node header = get_header();

			for (int k = table_header.data.level; k >= 0; k--)
			{
				p = header.foward[k];
				json_object_key_node jn = get_key_node(p);
				q = jn.foward[k];
				last_link = q;
				json_object_key_node qnd;
				int comp = 1;
				if (q != null_row) {
					qnd.read(fb, q);
					comp = compare_node(qnd, _key);
				}
				while (comp < 0)
				{
					p = q;
					last_link = q;
					json_object_key_node jn = qnd;
					q = jn.foward[k];
					comp = 1;
					if (q != null_row) {
						qnd.read(fb, q);
						comp = compare_node(qnd, _key);
					}
				}
				if (comp == 0)
					found = q;
				else if (comp < 0)
					found = last_link;
				update[k] = p;
			}

			return found;
		}

		relative_ptr_type find_node(const KEY& key, json_object_key_node& _found_node)
		{
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = find_node(update, key, _found_node);
			return value;
		}

		json_object_key_node first_node()
		{
			json_object_key_node jn;
			auto header = get_header();
			if (header.foward[0] != null_row) {
				jn.read(fb, header.foward[0]);
			}
			return jn;
		}

		json_object_key_node next_node(json_object_key_node _node)
		{
			json_object_key_node jn;

			if (_node.foward.size() == 0)
				return jn;

			json_object_key_node nd = get_key_node(_node.foward[0]);
			return nd;
		}

	public:

		json_object_table(file_block* _fb): fb(_fb)
		{
			json_parser jp;
			table_header = {};
		}

		KEY get_key(json& _object)
		{
			KEY key = (KEY)_object[object_id_field];
			return key;
		}

		relative_ptr_type create()
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "create", start_time, __FILE__, __LINE__);
			}

			relative_ptr_type location = create_header();
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "create complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return location;
		}

		relative_ptr_type open(relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "open", start_time, __FILE__, __LINE__);
			}
			table_header.read(fb, location);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return location;
		}

		void clear()
		{
			// fill this out at some point;
		}

		bool contains(const KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "contains", start_time, __FILE__, __LINE__);
			}

			json_object_key_node found_node;

			relative_ptr_type result = find_node(key, found_node);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "contains complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return  result != null_row;
		}

		json get(const KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}

			json result;
			json_object_key_node found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_node dn = found_node.get_node(fb);
				result = dn.data;
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		json get(const KEY key, std::initializer_list<std::string> include_fields)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}

			json result;
			json_object_key_node found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_node dn = found_node.get_node(fb);
				result = dn.data;
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		void put_array(json _array)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "put_array", start_time, __FILE__, __LINE__);
			}

			if (_array.array()) {
				for (auto item : _array) {
					put(item);
				}
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "put_array", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		relative_ptr_type put(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);
			}

			auto key = get_key(value);
			relative_ptr_type modified_node = this->update_node(key,
				[value](UPDATE_VALUE& dest) {
					dest.assign_update(value);
				}
			);

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return modified_node;
		}

		relative_ptr_type put(std::string _json)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json jx = jp.parse_object(_json);
			if (jx.empty() or jx.is_member("ClassName", parse_error_class)) {
				return null_row;
			}
			auto key = get_key(jx);
			relative_ptr_type modified_node = this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_update(jx); });
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return modified_node;
		}

		relative_ptr_type replace(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);
			}

			auto key = get_key(value);

			json_object_key_node found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_node dn = found_node.get_node(fb);
				dn.data.assign_replace(value);
				dn.write(fb);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "replace", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return n;
		}

		relative_ptr_type replace(std::string _json)
		{
			json_parser jp;
			json jx = jp.parse_object(_json);
			return replace(jx);
		}

		bool erase(const KEY& key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "erase", start_time, __FILE__, __LINE__);
			}

			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_object_key_node qnd, pnd;

			json_object_key_node found_node;
			relative_ptr_type q = find_node(update, key, found_node);

			if (q != null_row)
			{
				k = 0;
				p = update[k];
				qnd = found_node;
				pnd = get_key_node(p);
				int m = table_header.data.level;
				while (k <= m && pnd.foward[k] == q)
				{
					pnd.foward[k] = qnd.foward[k];
					pnd.write(fb);
					k++;
					if (k <= m) {
						p = update[k];
						pnd = get_key_node(p);
					}
				}

				table_header.data.count--;

				json_object_key_node header = get_header();

				while (header.foward[m] == null_row && m > 0) {
					m--;
				}

				free_node(q);

				table_header.data.level = m;
				header.write(fb);
				table_header.write(fb);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::global_mon->log_table_stop("table", "erase complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				return true;
			}
			else
			{
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::global_mon->log_table_stop("table", "erase failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				return false;
			}
		}

		class for_each_result {
		public:
			bool is_any;
			bool is_all;
			int64_t count;
		};

		for_each_result for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json_data_node& _item)> _process_clause)
		{

			for_each_result result = {};

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);
			}

			result.is_all = true;

			KEY key = get_key(_key_fragment);

			json_object_key_node jkn;
			relative_ptr_type location = find_first_gte(key, jkn);
			int64_t index = 0;

			while (location != null_row)
			{
				jkn = get_key_node(location);
				json_data_node node = jkn.get_node(fb);
				int comparison;

				if (_key_fragment.empty())
					comparison = 0;
				else
					comparison = _key_fragment.compare(node.data);

				if (comparison == 0)
				{
					relative_ptr_type process_result = _process_clause(index, node);
					if (process_result > 0)
					{
						result.is_any = true;
						index++;
					}
					else {
						result.is_all = false;
					}
				}
				location = jkn.foward[0];
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json_data_node& _item)> _process_clause)
		{
			json empty_key;
			return for_each(empty_key, _process_clause);
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			json empty_key;
			return for_each(empty_key, [_process_clause](int _index, json_data_node& _jn) {
				return _process_clause(_index, _jn.data);
				});
		}

		json get_first(json _key_fragment, std::function<bool(json& _src)> _fn)
		{
			json empty;
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);
			}

			auto index_lists = 0;

			int64_t index = 0;
			json result_data;
			json_object_key_node node;
			KEY key = get_key(_key_fragment);

			relative_ptr_type location = find_first_gte(key, node);

			while (location != null_row)
			{
				node = get_key_node(location);
				json_data_node data = node.get_node(fb);
				int comparison = _key_fragment.compare(data.data);
				if (comparison == 0 && _fn(data.data))
				{
					result_data = data.data;
					location = null_row;
				}
				else
				{
					location = node.foward[0];
				}
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "get_first complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return empty;
		}

		json select(std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			json empty_key = jp.create_object();

			auto result = for_each([pja, _project](int _index, json& _data) ->relative_ptr_type
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data);
					if (not new_item.empty() and
						not new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count = 1;
					}
					return count;
				});

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return ja;
		}

		json select(json _key_fragment, std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			json empty_key = jp.create_object();

			auto result = for_each(_key_fragment, [pja, _project](int _index, json_data_node& _data) ->relative_ptr_type
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data.data);
					if (not new_item.empty() and
						not new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count = 1;
					}
					return count;
				});

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return ja;
		}

		json update(json _key_fragment,
			std::function<json(int _index, json& _item)> _project,
			json _update = "{ }"_jobject
		)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "update", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			auto result = for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_data_node& _data) -> relative_ptr_type
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data.data);
					if (not new_item.empty() && !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count++;
						if (_update.object() and _update.size() > 0) {
							_data.data.assign_update(_update);
							_data.write(fb);
						}
					}
					return count;
				});

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "update complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return ja;
		}

		json select_object(json& _destination,
			json _key_fragment,
			std::function<json(int _index, json& _item)> _project,
			std::function<json(int _index, json& _item)> _get_child_key,
			std::initializer_list<std::string> _group_by
		)
		{
			json* pdestination = &_destination;

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "select_object", start_time, __FILE__, __LINE__);
			}

			for_each_result fra = for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_data_node& _jdata) -> int64_t
				{
					json _data = _jdata.data;
					json new_item = _project(_index, _data);
					if (not new_item.empty()) {
						json group_key = new_item.extract(_group_by);
						std::string member_name = group_key.to_key();
						if (pdestination->has_member(member_name))
						{
							json jx = pdestination->get_member(member_name);
							if (jx.array())
							{
								jx.append_element(new_item);
							}
						}
						else
						{
							json_parser jp;
							json item_array = jp.create_array();
							item_array.append_element(new_item);
							pdestination->put_member_array(member_name, item_array);

							json child_key = _get_child_key(_index, _data);
							if (child_key.object()) {
								select_object(*pdestination, child_key, _project, _get_child_key, _group_by);
							}
						}
					}
					return true;
				});
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "select_object complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return _destination;
		}


		inline int size() { return table_header.data.count; }


	private:

		int randomLevel()
		{
			double r = ((double)rand() / (RAND_MAX));
			int level = (int)(log(1. - r) / log(1. - .5));
			if (level < 1) level = 0;
			else if (level >= JsonTableMaxLevel) level = JsonTableMaxLevel;
			return level;
		}

	};

	std::ostream& operator <<(std::ostream& output, json_object_table& src)
	{
		std::cout << "[json_table]" << std::endl;
		return output;
	}
}

#endif

