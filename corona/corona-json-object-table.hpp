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

	class json_object_key_block : public data_block
	{
	public:

		int64_t										object_id;
		int64_t										json_location;
		iarray<int64_t, JsonTableMaxNumberOfLevels> foward;

		json_object_key_block() = default;
		json_object_key_block(const json_object_key_block& _src) = default;
		json_object_key_block(json_object_key_block&& _src) = default;

		virtual ~json_object_key_block()
		{
			;
		}

		json_object_key_block& operator = (const json_object_key_block& _src) = default;
		json_object_key_block& operator = (json_object_key_block&& _src) = default;

		void clear()
		{
			foward.clear();
			json_location = 0;
			object_id = 0;
		}

		virtual char* before_read(int32_t size) override
		{
			char* io_bytes = (char*)&object_id;
			return io_bytes;
		}

		virtual void after_read(char* _bytes) override
		{
		}


		virtual char* before_write(int32_t* _size) override
		{
			*_size =
				sizeof(object_id) +
				sizeof(json_location) +
				foward.get_io_write_size();
			char* io_bytes = (char*)&object_id;
			return io_bytes;
		}

		virtual void after_write(char* _bytes) override
		{
			;
		}

		virtual void finished_io(char* _bytes)  override
		{

		}
		json_data_block get_node(file_block* _file)
		{
			json_data_block node_to_read;
			node_to_read.read(_file, json_location);
			return node_to_read;
		}

	};

	/* 
	
	the json object table is created by the database for each class,
	and it is keyed off an object id alone, as compared to the more general
	case of the json_table, which can take any key.
	
	*/

	class json_object_table
	{
	private:

		shared_lockable table_lock;

		std::shared_ptr<json_table_header>	table_header;

		using KEY = int64_t;
		using VALUE = json_node;
		using UPDATE_VALUE = json;
		const int SORT_ORDER = 1;

		file_block* fb;

		std::shared_ptr<json_table_header> create_header()
		{
			json_parser jp;

			json_object_key_block node_header = create_node(JsonTableMaxLevel);
			node_header.object_id = 0;
			node_header.write(fb);

			table_header->set_data_root_location(node_header.header.block_location);
			table_header->save(fb);

			return table_header;
		}

		json_object_key_block get_header()
		{
			json_object_key_block in;

			auto p = table_header->get_data_root_location();
			if (p <= 0) {
				throw std::exception("table header node location not set");
			}

			int64_t result = in.read(fb, p);

			if (result < 0)
			{
				throw std::logic_error("Couldn't read table header.");
			}

			return in;
		}

		json_object_key_block create_node(int _max_level)
		{
			json_object_key_block new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.foward.push_back(rit);
			}

			new_node.append(fb);

			return new_node;
		}

		json_object_key_block get_key_node(relative_ptr_type _key_node_location)
		{
			json_object_key_block node;

			node.read(fb, _key_node_location);
			return node;
		}

		void free_node(relative_ptr_type _key_node_location)
		{
			json_object_key_block node;
			node.read(fb, _key_node_location);
			node.erase(fb);

			json_data_block json_data;
			json_data.read(fb, node.json_location);
			json_data.erase(fb);
		}

		// compare a node to a key for equality
		// return -1 if the node < key
		// return 1 if the node > key
		// return 0 if the node == key

		relative_ptr_type compare_node(json_object_key_block& _nd, KEY _id_key)
		{

			if (_nd.header.block_location == table_header->get_data_root_location())
				return -1;

			if (_nd.object_id < _id_key)
				return -SORT_ORDER;
			else if (_nd.object_id > _id_key)
				return SORT_ORDER;
			else
				return 0;
		}

		relative_ptr_type find_advance(json_object_key_block& _node, json_object_key_block& _peek, int _level, KEY _key, int64_t* _found)
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

		relative_ptr_type find_advance_gt(json_object_key_block& _node, json_object_key_block& _peek, int _level, KEY _key, int64_t* _found)
		{
			auto t = _node.foward[_level];
			if (t != null_row) {
				_peek.read(fb, t);
				int comp = compare_node(_peek, _key);
				if (comp < 0)
					return t;
				else if (comp >= 0) {
					*_found = t;
				}
			}
			return -1;
		}

		relative_ptr_type find_node(relative_ptr_type* update, KEY _key, json_object_key_block& _found_node)
		{
			relative_ptr_type found = null_row;

			json_object_key_block x = get_header();

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				x = get_header();
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

		relative_ptr_type find_first_gte(KEY _key, json_object_key_block& _found_node)
		{
			relative_ptr_type found = null_row, last_link;
			json_parser jp;

			json_object_key_block x = get_header();

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance_gt(x, _found_node, k, _key, &found);
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance_gt(x, _found_node, k, _key, &found);
				}
			}

			return found;
		}

		relative_ptr_type update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			json_object_key_block jkn;

			relative_ptr_type q = find_node(update, _key, jkn);

			if (q != null_row)
			{
				json_data_block qnd;
				qnd = jkn.get_node(fb);
				predicate(qnd.data);
				qnd.write(fb);
				return q;
			}

			json_object_key_block qnd;

			k = randomLevel();

			json_object_key_block header = get_header();
			relative_ptr_type location = header.foward[0];

			if (k > table_header->get_level())
			{
				k = table_header->add_level();
			}

			update[k] = header.header.block_location;
			UPDATE_VALUE initial_value;
			try {
				predicate(initial_value);
			}
			catch (std::exception exc)
			{
				std::cout << __FILE__ << " " << __LINE__ << ":Initialization of new object failed when inserting node into table." << std::endl;
				return -1;
			}

			json_object_key_block key_node = create_node(k);

			json_data_block value_node;
			value_node.data = initial_value;
			key_node.json_location = value_node.append(fb);
			key_node.object_id = _key;

			do
			{
				json_object_key_block pnd = get_key_node(update[k]);
				key_node.foward[k] = pnd.foward[k];
				pnd.foward[k] = key_node.header.block_location;
				pnd.write(fb);

			} while (--k >= 0);

			table_header->add_count();
			key_node.write(fb);

			return key_node.header.block_location;
		}

		relative_ptr_type find_node(const KEY& key, json_object_key_block& _found_node)
		{
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = find_node(update, key, _found_node);
			return value;
		}

		json_object_key_block first_node()
		{
			json_object_key_block jn;
			auto header = get_header();
			if (header.foward[0] != null_row) {
				jn.read(fb, header.foward[0]);
			}
			return jn;
		}

		json_object_key_block next_node(json_object_key_block _node)
		{
			json_object_key_block jn;

			if (_node.foward.size() == 0)
				return jn;

			json_object_key_block nd = get_key_node(_node.foward[0]);
			return nd;
		}

		int64_t			table_class_id;


		std::shared_ptr<json_table_header> create()
		{

			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "create", start_time, __FILE__, __LINE__);
			}

			create_header();

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "create complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return table_header;
		}

		void open()
		{

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "open", start_time, __FILE__, __LINE__);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

	public:

		json_object_table(std::shared_ptr<json_table_header> _header, int64_t _table_class_id, file_block* _fb)
			:	table_header(_header),
				table_class_id(_table_class_id),
				fb(_fb)
		{
			if (_header->get_data_root_location() < 0)
			{
				create();
			}
			else
			{
				open();
			}
		}

		std::shared_ptr<json_table_header> get_table_header()
		{
			return table_header;
		}

		KEY get_key(json& _object)
		{
			KEY key = (KEY)_object[object_id_field];
			return key;
		}

		void clear()
		{
			// fill this out at some point;
		}

		bool contains(const KEY key)
		{
			read_scope_lock lock(table_lock);
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "contains", start_time, __FILE__, __LINE__);
			}

			json_object_key_block found_node;

			relative_ptr_type result = find_node(key, found_node);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "contains complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return  result != null_row;
		}

		json get(const KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}

			read_scope_lock lock(table_lock);
			json result;
			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
				result = dn.data;
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		json get(const KEY key, std::initializer_list<std::string> include_fields)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}

			read_scope_lock lock(table_lock);

			json result;
			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
				result = dn.data;
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		void put_array(json _array)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "put_array", start_time, __FILE__, __LINE__);
			}
			if (_array.array()) {
				for (auto item : _array) {
					put(item);
				}
			}
			table_header->save(fb);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "put_array", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		relative_ptr_type put(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);
			}
			write_scope_lock lock(table_lock);
			auto key = get_key(value);
			relative_ptr_type modified_node = this->update_node(key,
				[value](UPDATE_VALUE& dest) {
					dest.assign_update(value);
				}
			);
			table_header->save(fb);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return modified_node;
		}

		relative_ptr_type put(std::string _json)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json jx = jp.parse_object(_json);
			if (jx.empty() or jx.is_member("ClassName", parse_error_class)) {
				return null_row;
			}
			auto key = get_key(jx);
			write_scope_lock lock(table_lock);

			relative_ptr_type modified_node = this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_update(jx); });
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return modified_node;
		}

		relative_ptr_type replace(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);
			}
			write_scope_lock lock(table_lock);

			auto key = get_key(value);

			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
				dn.data.assign_replace(value);
				dn.write(fb);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_stop("table", "replace", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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
				system_monitoring_interface::active_mon->log_table_start("table", "erase", start_time, __FILE__, __LINE__);
			}
			write_scope_lock lock(table_lock);

			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_object_key_block qnd, pnd;

			json_object_key_block found_node;
			relative_ptr_type q = find_node(update, key, found_node);

			if (q != null_row)
			{
				k = 0;				
				p = update[k];
				qnd = found_node;
				pnd = get_key_node(p);
				int m = table_header->get_level();
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

				json_object_key_block header = get_header();

				free_node(q);

				while (header.foward[m] == null_row && m > 0) {
					m--;
				}

				table_header->set_level(m);
				table_header->sub_count();
				header.write(fb);
				table_header->save(fb);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_table_stop("table", "erase complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				return true;
			}
			else
			{
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_table_stop("table", "erase failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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

		for_each_result for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json_data_block& _item)> _process_clause)
		{

			for_each_result result = {};

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);
			}

			result.is_all = true;

			KEY key = get_key(_key_fragment);

			read_scope_lock lock(table_lock);

			json_object_key_block jkn;
			relative_ptr_type location = find_first_gte(key, jkn);
			int64_t index = 0;

			while (location != null_row)
			{
				jkn = get_key_node(location);
				json_data_block node = jkn.get_node(fb);
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
				system_monitoring_interface::active_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json_data_block& _item)> _process_clause)
		{
			json empty_key;
			return for_each(empty_key, _process_clause);
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			json empty_key;
			return for_each(empty_key, [_process_clause](int _index, json_data_block& _jn) {
				return _process_clause(_index, _jn.data);
				});
		}

		json get_first(json _key_fragment, std::function<bool(json& _src)> _fn)
		{
			json empty;
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);
			}

			auto index_lists = 0;

			int64_t index = 0;
			json result_data;
			json_object_key_block node;
			KEY key = get_key(_key_fragment);
			read_scope_lock lock(table_lock);
			
			relative_ptr_type location = find_first_gte( key, node);

			while (location != null_row)
			{
				node = get_key_node(location);
				json_data_block data = node.get_node(fb);
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
				system_monitoring_interface::active_mon->log_table_stop("table", "get_first complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return empty;
		}

		json select(std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);
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
				system_monitoring_interface::active_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return ja;
		}

		json select(json _key_fragment, std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			json empty_key = jp.create_object();

			auto result = for_each(_key_fragment, [pja, _project](int _index, json_data_block& _data) ->relative_ptr_type
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
				system_monitoring_interface::active_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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
				system_monitoring_interface::active_mon->log_table_start("table", "update", start_time, __FILE__, __LINE__);
			}

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			auto result = for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_data_block& _data) -> relative_ptr_type
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
				system_monitoring_interface::active_mon->log_table_stop("table", "update complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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
				system_monitoring_interface::active_mon->log_table_start("table", "select_object", start_time, __FILE__, __LINE__);
			}

			for_each_result fra = for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_data_block& _jdata) -> int64_t
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
				system_monitoring_interface::active_mon->log_table_stop("table", "select_object complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return _destination;
		}


		inline int size() { return table_header->get_count(); }


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

	void test_json_object_table(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::active_mon->log_function_start("object table proof", "start", st, __FILE__, __LINE__);
		using namespace std::literals;

		using return_type = bool;
		json_parser jp;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");
		file_block fp(f);

		std::shared_ptr<json_table_header> header = std::make_shared<json_table_header>();
		header->create(&fp);

		json test_write = jp.create_object();
		test_write.put_member_i64(object_id_field, 5);
		test_write.put_member("Name", "Joe"sv);
		int64_t test_key = (int64_t)test_write[object_id_field];

		json_object_table test_table(header, 0, &fp);

		auto read_header = test_table.get_table_header();

		bool result;

		result = (bool)read_header;
		_tests->test({ "create table", result, __FILE__, __LINE__ });

		relative_ptr_type rpt = test_table.put(test_write);
		result = (rpt >= 0);
		_tests->test({ "put", result, __FILE__, __LINE__ });

		json test_read = test_table.get(test_key);
		result = not test_read.empty();
		_tests->test({ "read 1", result, __FILE__, __LINE__ });

		test_read.set_compare_order({ object_id_field });
		result = test_read.compare(test_write) == 0;
		_tests->test({ "check read 1", result, __FILE__, __LINE__ });

		json db_contents = test_table.select([](int _index, json& item) {
			return item;
			});

		result = db_contents.array();
		_tests->test({ "array result", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Jack"sv);
		test_key = (int64_t)test_write[object_id_field];

		test_table.put(test_write);

		test_read = test_table.get(test_key);
		test_read.set_natural_order();

		result = test_read.compare(test_write) == 0;
		_tests->test({ "read 2", result, __FILE__, __LINE__ });

		test_read = jp.create_object();
		test_read.put_member_i64(object_id_field, 5);
		test_read.set_natural_order();

		json joe = test_table.get(test_read);
		result = not joe.empty();
		_tests->test({ "out of order", result, __FILE__, __LINE__ });

		db_contents = test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1 = db_contents.size();

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Jill"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);
		test_read.set_natural_order();

		result = test_read.compare(test_write) == 0;
		_tests->test({ "read 3", result, __FILE__, __LINE__ });

		db_contents = test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1b = db_contents.size();

		try
		{
			json t5 = test_table.get(6);
			_tests->test({ "null access", true, __FILE__, __LINE__ });
		}
		catch (std::exception exc)
		{
			_tests->test({ "null access", false, __FILE__, __LINE__ });
		}

		db_contents = test_table.select([](int _index, json& item) {
			return item;
			});

		result = (db_contents.size() == 2);
		_tests->test({ "table results size", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 2);
		test_write.put_member("Name", "Sydney"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert", result, __FILE__, __LINE__ });

		test_read = jp.create_object();
		test_read.put_member_i64(object_id_field, 5);
		test_read.set_natural_order();

		joe = test_table.get(test_read);
		result = not joe.empty();
		_tests->test({ "insert front", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Orwell"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "update", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 1);
		test_write.put_member("Name", "Canada"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert first 2", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Roger"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "update 2", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 1);
		test_write.put_member("Name", "Maraca"sv);
		test_key = (int64_t)test_write[object_id_field];
		test_table.put(test_write);

		test_read = test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert first 2", result, __FILE__, __LINE__ });

		db_contents = test_table.select([](int _index, json& item) {
			return item;
			});

		result = test_table.size() == 4;
		_tests->test({ "table size", result, __FILE__, __LINE__ });
		result = db_contents.size() == 4;
		_tests->test({ "select size", result, __FILE__, __LINE__ });

		int64_t tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		json counts = jp.create_object();
		json* pcounts = &counts;

		auto fr = test_table.for_each([tests, pcounts](int _index, json& _item) -> bool {
			int64_t test_index = tests[_index];
			std::string member_names = std::format("item{0}", (int64_t)_item[object_id_field]);
			int64_t counto = 0;
			if (pcounts->has_member(member_names)) {
				counto = pcounts->get_member(member_names);
			}
			counto++;
			pcounts->put_member_i64(member_names, counto);
			return 1;
			});

		result = counts.object() and counts.size() == 4;
		_tests->test({ "for_each", result, __FILE__, __LINE__ });

		std::string count_string = counts.to_json();

		db_contents = test_table.select([tests](int _index, json& _item) -> json {
			int64_t temp = _item[object_id_field];
			return (temp > 0i64) ? _item : json();
			}
		);

		result = db_contents.array();
		_tests->test({ "select", result, __FILE__, __LINE__ });

		bool any_fails = db_contents.any([](json& _item)->bool {
			int64_t temp = _item[object_id_field];
			return temp <= 0i64;
			});

		result = not any_fails;
		_tests->test({ "any", result, __FILE__, __LINE__ });

		auto summary = db_contents.array_to_object([](json& _item) {
			return (std::string)_item[object_id_field];
			},
			[](json& _target) {
				return _target;
			});

		result = summary.has_member("1") and summary.has_member("2") and summary.has_member("5") and summary.has_member("7");
		_tests->test({ "array_to_object", result, __FILE__, __LINE__ });

		json search_key = jp.create_object("Name", "Zeus");
		search_key.set_compare_order({ "Name" });

		db_contents = test_table.select(search_key, [tests](int _index, json& _item) -> json {
			return _item;
			}
		);

		any_fails = db_contents.any([](json& _item)->bool {
			std::string temp = _item["Name"];
			return temp != "Zeus";
			});

		result = not any_fails;
		_tests->test({ "any 2", result, __FILE__, __LINE__ });

		result = not test_table.erase(3);
		_tests->test({ "erase negative", result, __FILE__, __LINE__ });

		result = test_table.erase(1);
		_tests->test({ "erase", result, __FILE__, __LINE__ });

		result = test_table.erase(7);
		_tests->test({ "erase tail", result, __FILE__, __LINE__ });

		json testi = test_table.select([tests](int _index, json& item) -> json {
			int64_t object_id = item[object_id_field];
			return object_id == 7 ? item : json();
			});

		result = (testi.size() == 0);
		_tests->test({ "table after erase", result, __FILE__, __LINE__ });

		db_contents = test_table.select([tests](int _index, json& item) {
			return item;
			}
		);

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item[object_id_field];
			return  object_id != 2 and object_id != 5;
			});

		result = not any_iteration_fails;
		_tests->test({ "table correct", result, __FILE__, __LINE__ });

		system_monitoring_interface::active_mon->log_function_stop("object table proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_json_object_table_load(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::active_mon->log_function_start("object table load proof", "start", st, __FILE__, __LINE__);

		using return_type = bool;
		json_parser jp;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");
		file_block fp(f);

		std::shared_ptr<json_table_header> header = std::make_shared<json_table_header>();
		header->create(&fp);


		json_object_table test_table(header, 0, &fp);

		for (int i = 1; i < 1000; i++)
		{
			json test_write = jp.create_object();
			test_write.put_member_i64(object_id_field, i);
			test_write.put_member("Name", "Joe " + std::to_string(i));
			test_table.put(test_write);
		}

		_tests->test({ "write 1000", true, __FILE__, __LINE__ });

		for (int i = 1; i < 1000; i++)
		{
			json obj = test_table.get(i);
			if (obj.empty()) {
				_tests->test({ "read 1000", false, __FILE__, __LINE__ });;
				return;
			}
		}

		_tests->test({ "read 1000", true, __FILE__, __LINE__ });

		system_monitoring_interface::active_mon->log_function_stop("object table load proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif

