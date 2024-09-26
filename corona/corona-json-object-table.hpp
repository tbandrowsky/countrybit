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

		std::shared_ptr<json_table_header>	table_header;

		using KEY = int64_t;
		using VALUE = json_node;
		using UPDATE_VALUE = json;
		const int SORT_ORDER = 1;

		file_block* fb;

		std::shared_ptr<json_table_header> create_header()
		{
			json_parser jp;

			json_object_key_block header = create_node(JsonTableMaxLevel);
			header.object_id = 0;
			header.write(fb);

			table_header->set_data_root_location(header.header.block_location);
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

		relative_ptr_type find_advance(lock_owner& _transaction_lock, json_object_key_block& _node, json_object_key_block& _peek, int _level, KEY _key, int64_t* _found)
		{
			auto t = _node.foward[_level];
			if (t != null_row) {
				_peek.read(fb, t);
				lock_chumpy->add_lock(_transaction_lock, { object_lock_types::lock_object, table_class_id, _peek.object_id });
				int comp = compare_node(_peek, _key);
				if (comp < 0)
					return t;
				else if (comp == 0) {
					*_found = t;
				}
			}
			return -1;
		}

		relative_ptr_type find_node(lock_owner& _transaction_lock, relative_ptr_type* update, KEY _key, json_object_key_block& _found_node)
		{
			relative_ptr_type found = null_row;

			json_object_key_block x = get_header();

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance(_transaction_lock, x, _found_node, k, _key, &found); // TODO Found node could be here.
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance(_transaction_lock, x, _found_node, k, _key, &found);
				}
				update[k] = x.header.block_location;
			}

			return found;
		}

		relative_ptr_type find_first_gte(lock_owner& _transaction_lock, KEY _key, json_object_key_block& _found_node)
		{
			relative_ptr_type found = null_row, last_link;
			json_parser jp;

			json_object_key_block x = get_header();

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance(_transaction_lock, x, _found_node, k, _key, &found);
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance(_transaction_lock, x, _found_node, k, _key, &found);
				}
			}

			return found;
		}

		relative_ptr_type update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			json_object_key_block jkn;

			auto transaction_lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, _key });

			relative_ptr_type q = find_node(transaction_lock, update, _key, jkn);

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
				auto transaction_lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, _key });
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
				lock_chumpy->add_lock(transaction_lock, { object_lock_types::lock_object, table_class_id, pnd.object_id });
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
			auto locks = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = find_node(locks, update, key, _found_node);
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

		object_locker*  lock_chumpy;
		int64_t			table_class_id;

	public:

		json_object_table(std::shared_ptr<json_table_header> _header, int64_t _table_class_id, object_locker* _lock_chumpy, file_block* _fb)
			:	table_header(_header),
				table_class_id(_table_class_id),
				lock_chumpy(_lock_chumpy),
				fb(_fb)
		{
			json_parser jp;
			table_header = {};
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

		std::shared_ptr<json_table_header> create()
		{
			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "create", start_time, __FILE__, __LINE__);
			}

			create_header();

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "create complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return table_header;
		}

		void open()
		{
			auto lock = lock_chumpy->lock({ object_lock_types::lock_table, table_class_id, 0 });

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "open", start_time, __FILE__, __LINE__);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		void clear()
		{
			// fill this out at some point;
		}

		bool contains(const KEY key)
		{
			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "contains", start_time, __FILE__, __LINE__);
			}

			json_object_key_block found_node;

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

			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });
			json result;
			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
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

			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });

			json result;
			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
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
			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });
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
			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });

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
			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });

			json_object_key_block found_node;
			relative_ptr_type n = find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
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
			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });

			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_object_key_block qnd, pnd;

			json_object_key_block found_node;
			relative_ptr_type q = find_node(lock, update, key, found_node);

			if (q != null_row)
			{
				k = 0;				
				p = update[k];
				qnd = found_node;
				pnd = get_key_node(p);
				lock_chumpy->add_lock(lock, { object_lock_types::lock_object, table_class_id, pnd.object_id });
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

				lock_chumpy->add_lock(lock, { object_lock_types::lock_object, table_class_id, 0 });
				json_object_key_block header = get_header();

				free_node(q);

				while (header.foward[m] == null_row && m > 0) {
					m--;
				}

				table_header->set_level(m);
				table_header->sub_count();
				header.write(fb);
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

		for_each_result for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json_data_block& _item)> _process_clause)
		{

			for_each_result result = {};

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);
			}

			result.is_all = true;

			KEY key = get_key(_key_fragment);

			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });

			json_object_key_block jkn;
			relative_ptr_type location = find_first_gte(lock, key, jkn);
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
				system_monitoring_interface::global_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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
				system_monitoring_interface::global_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);
			}

			auto index_lists = 0;

			int64_t index = 0;
			json result_data;
			json_object_key_block node;
			KEY key = get_key(_key_fragment);

			auto lock = lock_chumpy->lock({ object_lock_types::lock_object, table_class_id, key });


			relative_ptr_type location = find_first_gte(lock, key, node);

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
				system_monitoring_interface::global_mon->log_table_stop("table", "select_object complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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
}

#endif

