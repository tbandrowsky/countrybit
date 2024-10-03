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


#ifndef CORONA_JSON_TABLE_H
#define CORONA_JSON_TABLE_H

namespace corona 
{

	const int debug_json_table = 0;

	class json_key_block : public data_block
	{
	public:

		uint64_t									hash_code;
		int64_t										json_location;
		iarray<int64_t, JsonTableMaxNumberOfLevels> foward;

		json_key_block()
		{
			clear();
		}

		json_key_block(const json_key_block& _src) = default;
		json_key_block(json_key_block&& _src) = default;

		virtual ~json_key_block()
		{
			;
		}

		json_key_block& operator = (const json_key_block& _src) = default;
		json_key_block& operator = (json_key_block&& _src) = default;

		void clear()
		{
			foward.clear();
			json_location = 0;
			hash_code = 0;
		}

		virtual char* before_read(int32_t size) override
		{
			char* io_bytes = (char*)&hash_code;
			return io_bytes;
		}

		virtual void after_read(char* _bytes) override
		{
		}


		virtual char* before_write(int32_t* _size) override
		{
			*_size =
				sizeof(json_location) +
				sizeof(hash_code) +
				foward.get_io_write_size();
			char* io_bytes = (char*)&hash_code;
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


	class json_table_header : protected poco_node<table_header_struct>
	{
		shared_lockable header_lock;

	public:

		json_table_header()
		{
			data.count = 0;
			data.level = JsonTableMaxLevel;
			data.data_root_location = -1;
		}

		int get_level()
		{
			return data.level;
		}

		int32_t add_level( )
		{
			write_scope_lock locko(header_lock);
			data.level++;
			return data.level;
		}

		int32_t sub_level()
		{
			write_scope_lock locko(header_lock);
			data.level--;
			return data.level;
		}

		int32_t set_level(int32_t _new_level)
		{
			write_scope_lock locko(header_lock);
			data.level = _new_level;
			return data.level;
		}

		int64_t add_count()
		{
			write_scope_lock locko(header_lock);
			data.count++;
			return data.count;
		}

		int64_t sub_count()
		{
			write_scope_lock locko(header_lock);
			data.count--;
			return data.count;
		}

		int64_t get_count()
		{
			read_scope_lock locko(header_lock);
			return data.count;
		}

		int64_t get_data_root_location()
		{
			read_scope_lock me(header_lock);
			return data.data_root_location;
		}

		int64_t set_data_root_location(int64_t _root)
		{
			read_scope_lock me(header_lock);
			data.data_root_location = _root;
			return data.data_root_location;
		}

		int64_t get_location()
		{
			read_scope_lock me(header_lock);
			return header.block_location;
		}

		void open(file_block* fb, int64_t _location)
		{
			write_scope_lock me(header_lock);
			data_block::read(fb, _location);
		}

		void create(file_block *fb)
		{
			write_scope_lock me(header_lock);
			data_block::append(fb);
		}

		void save(file_block* fb)
		{
			write_scope_lock me(header_lock);
			data_block::write(fb);
		}
	};

	class json_table 
	{
	private:

		shared_lockable table_lock;

		std::shared_ptr<json_table_header>	table_header;

		using KEY = json;
		using VALUE = json_node;
		using UPDATE_VALUE = json;
		const int SORT_ORDER = 1;

		file_block* fb;
		
		std::vector<std::string> key_fields;

		std::shared_ptr<json_table_header> create_header()
		{
			json_parser jp;

			json_key_block header = create_node(JsonTableMaxLevel);
			header.hash_code = 0;
			header.write(fb);

			table_header->set_data_root_location(header.header.block_location);
			table_header->save(fb);

			return table_header;
		}

		json_key_block get_header()
		{
			json_key_block in;

			auto p = table_header->get_data_root_location();
			if (p < 0) {
				throw std::exception("table header node location not set");
			}

			int64_t result = in.read(fb, p);

			if (result < 0)
			{
				throw std::logic_error("Couldn't read table header.");
			}

			return in;
		}

		json_key_block create_node(int _max_level)
		{
			json_key_block new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.foward.push_back(rit);
			}

			new_node.append(fb);

			return new_node;
		}

		json_key_block get_key_node(relative_ptr_type _key_node_location)
		{
			json_key_block node;

			node.read(fb, _key_node_location);
			return node;
		}

		void free_node(relative_ptr_type _key_node_location)
		{
			json_key_block node;
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

		relative_ptr_type compare_node(json_key_block& _nd, KEY _id_key, uint64_t _key_hash)
		{
			if (_nd.header.block_location == table_header->get_data_root_location())
				return -1;

			if (_nd.hash_code < _key_hash)
				return -SORT_ORDER;
			else if (_nd.hash_code > _key_hash)
				return SORT_ORDER;

			auto jd = _nd.get_node(fb);

			KEY ndkey = get_key(jd.data);
			int k = -_id_key.compare(ndkey); // the - is here because the comparison is actually backwards. 
			if (k < 0)
				return -SORT_ORDER;
			else if (k > 0)
				return SORT_ORDER;
			else
				return 0;
		}

		relative_ptr_type find_advance_gt(json_key_block& _node, json_key_block& _peek, int _level, KEY _key, uint64_t _key_hash,  int64_t* _found)
		{
			auto t = _node.foward[_level];
			if (t != null_row) {
				_peek.read(fb, t);
				int comp = compare_node(_peek, _key, _key_hash);
				if (comp < 0)
					return t;
				else if (comp >= 0) {
					*_found = t;
				}
			}
			return -1;
		}

		relative_ptr_type find_advance(json_key_block& _node, json_key_block& _peek, int _level, KEY _key, uint64_t _key_hash, int64_t *_found)
		{
			auto t = _node.foward[_level];
			if (t != null_row) {
				_peek.read(fb, t);
				int comp = compare_node(_peek, _key, _key_hash);
				if (comp < 0)
					return t;
				else if (comp == 0) {
					*_found = t;
				}
			}
			return -1;
		}

		relative_ptr_type find_node(relative_ptr_type* update, KEY _key, json_key_block& _found_node)
		{
			relative_ptr_type found = null_row;

			uint64_t key_hash = _key.get_weak_ordered_hash(key_fields);	
			
			json_key_block x = get_header();

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance(x, _found_node, k, _key, key_hash, &found); // TODO Found node could be here.
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance( x, _found_node, k, _key, key_hash, &found);
				}
				update[k] = x.header.block_location;
			}

			return found;
		}

		relative_ptr_type find_first_gte(KEY _key, json_key_block& _found_node)
		{
			relative_ptr_type found = null_row, last_link;
			json_parser jp;

			json table_key = jp.create_object();

			for (auto key : key_fields) {
				if (_key.has_member(key)) {
					table_key.copy_member(key, _key);
				}
				else {
					table_key = jp.create_object();
				}
			}

			uint64_t key_hash = table_key.get_weak_ordered_hash(key_fields);

			json_key_block x = get_header();

			if (table_key.empty() or table_key.size() == 0) {
				return x.foward[0];
			}

			for (int k = table_header->get_level(); k >= 0; k--)
			{
				int comp = -1;
				relative_ptr_type nl = find_advance_gt( x, _found_node, k, table_key, key_hash, &found);
				while (nl != null_row)
				{
					x = _found_node;
					nl = find_advance_gt( x, _found_node, k, table_key, key_hash, &found);
				}
			}

			return found;
		}

		relative_ptr_type update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			uint64_t key_hash = _key.get_weak_ordered_hash(key_fields);

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			json_key_block header = get_header();

			relative_ptr_type location = header.foward[0];

			json_key_block jkn;

			relative_ptr_type q = find_node(update, _key, jkn);

			if (q != null_row)
			{
				json_data_block qnd;
				qnd = jkn.get_node(fb);
				predicate(qnd.data);
				qnd.write(fb);
				return q;
			}

			json_key_block qnd;

			k = randomLevel();
			if (k > table_header->get_level())
			{
				k = table_header->add_level();
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

			json_key_block key_node = create_node(k);

			json_data_block value_node;
			value_node.data = initial_value;
			key_node.json_location = value_node.append(fb);
			key_node.hash_code = key_hash;

			do
			{
				json_key_block pnd = get_key_node(update[k]);
				key_node.foward[k] = pnd.foward[k];
				pnd.foward[k] = key_node.header.block_location;
				pnd.write(fb);

			} while (--k >= 0);

			key_node.write(fb);

			table_header->add_count();

			return key_node.header.block_location;
		}

		relative_ptr_type find_node(const KEY& key, json_key_block& _found_node)
		{
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = find_node(update, key, _found_node);
			return value;
		}

		json_key_block first_node()
		{
			json_key_block jn;
			auto header = get_header();
			if (header.foward[0] != null_row) {
				jn.read(fb, header.foward[0]);
			}
			return jn;
		}

		json_key_block next_node(json_key_block _node)
		{
			json_key_block jn;

			if (_node.foward.size() == 0)
				return jn;

			json_key_block nd = get_key_node(_node.foward[0]);
			return nd;
		}

		std::shared_ptr<json_table_header> create()
		{
			date_time start_time = date_time::now();
			timer tx;

			create_header();

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "create", start_time, __FILE__, __LINE__);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "create complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return table_header;
		}

		void open()
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "open", start_time, __FILE__, __LINE__);
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		int64_t			table_class_id;

	public:

		json_table(std::shared_ptr<json_table_header> _header, int64_t _table_class_id, file_block* _fb, std::vector<std::string> _key_fields)
			:	table_header(_header),
				table_class_id(_table_class_id),
				fb(_fb),
				key_fields(_key_fields)
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

		json get_key(json& _object) 
		{		
			json key = _object.extract(key_fields);
			return key;
		}

		bool key_covered(json& _key)
		{
			bool r = _key.keys_compatible(key_fields);
			return r;
		}

		void clear()
		{
			// fill this out at some point;
		}

		bool contains(const KEY key)
		{
			read_scope_lock me(table_lock);
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "contains", start_time, __FILE__, __LINE__);
			}
			json_key_block found_node;

			relative_ptr_type result =  find_node(key, found_node);
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "contains complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return  result != null_row;
		}

		json get(std::string _key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}
			read_scope_lock me(table_lock);

			json_parser jp;
			json key = jp.parse_object(_key);

			key.set_natural_order();

			if (key.is_member("ClassName", parse_error_class)) {
				// create an empty object.
				system_monitoring_interface::global_mon->log_warning("key for object.get could not be parsed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_warning(key);
				key = jp.create_object();
				return key;
			}

			json result;
			json_key_block found_node;

			auto hash = key.get_weak_ordered_hash(key_fields);

			relative_ptr_type n =  find_node(key, found_node);
			if (n != null_row) {
				json_data_block dn = found_node.get_node(fb);
				result = dn.data;
			}
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return result;
		}

		json get(KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}
			read_scope_lock me(table_lock);

			auto hash = key.get_weak_ordered_hash(key_fields);

			json result;
			json_key_block found_node;
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

		json get( KEY key, std::initializer_list<std::string> include_fields)
		{
			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);
			}
			read_scope_lock me(table_lock);

			auto hash = key.get_weak_ordered_hash(key_fields);

			json result;
			json_key_block found_node;
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
			write_scope_lock me(table_lock);

			auto key = get_key(value);
			auto hash = key.get_weak_ordered_hash(key_fields);

			relative_ptr_type modified_node = this->update_node(key,
				[value](UPDATE_VALUE& dest) {
					dest.assign_update(value);
				}
			);

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			table_header->save(fb);

			return modified_node;
		}

		relative_ptr_type put(std::string _json)
		{
			write_scope_lock me(table_lock);

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
			auto hash = key.get_weak_ordered_hash(key_fields);

			relative_ptr_type modified_node = this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_update(jx); });
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			return modified_node;
		}

		relative_ptr_type replace(json value)
		{
			write_scope_lock me(table_lock);

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);
			}

			auto key = get_key(value);
			auto hash = key.get_weak_ordered_hash(key_fields);

			json_key_block found_node;
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

		bool erase(KEY& key)
		{
			write_scope_lock me(table_lock);

			date_time start_time = date_time::now();
			timer tx;
			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::global_mon->log_table_start("table", "erase", start_time, __FILE__, __LINE__);
			}

			auto hash = key.get_weak_ordered_hash(key_fields);

			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_key_block qnd, pnd;

			json_key_block found_node;
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

				json_key_block header = get_header();
				while (header.foward[m] == null_row && m > 0) {
					m--;
				}
				header.write(fb);
				free_node(q);

				table_header->set_level(m);
				table_header->sub_count();
				table_header->save(fb);
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
			read_scope_lock me(table_lock);

			result.is_all = true;

			json_key_block jkn;
			relative_ptr_type location = find_first_gte(_key_fragment, jkn);
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
			return for_each( empty_key, _process_clause);
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
			read_scope_lock me(table_lock);


			auto index_lists = 0;

			int64_t index = 0;
			json result_data;
			json_key_block node;

			relative_ptr_type location = find_first_gte(_key_fragment, node);

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

			auto result = for_each( [pja, _project](int _index, json& _data) ->relative_ptr_type
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

			auto result =  for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_data_block& _data) -> relative_ptr_type 
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data.data);
					if (not new_item.empty()&& !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count++;
						if (_update.object() and _update.size()>0) {
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

			for_each_result fra =  for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_data_block& _jdata) -> int64_t
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

	std::ostream& operator <<(std::ostream& output, json_table & src)
	{
		std::cout << "[json_table]" << std::endl;
		return output;
	}

	void test_object(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app);
	void test_file_block(std::shared_ptr<test_set> _tests,  std::shared_ptr<application> _app);
	void test_file(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app);
	void test_data_block(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app);
	void test_json_node(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app);
	void test_json_table(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app);

	void test_file_block(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{

		date_time start = date_time::now();
		timer tx;

		system_monitoring_interface::global_mon->log_function_start("test file_block", "start", start, __FILE__, __LINE__);

		corona::json_parser jp;
		corona::json		empty;

		json proof_assertions = jp.create_object();

		proof_assertions.put_member("test_name", "file_block");

		std::shared_ptr<file> fp = _app->create_file_ptr("test_file_block.dat");

		const char* pattern1 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int pattern1_length = strlen(pattern1);
		const char* pattern2 = "XXXXXXXXXXXXXXXXX";
		int pattern2_length = strlen(pattern2);
		const char* pattern3 = "YYYY";
		int pattern3_length = strlen(pattern3);

		file_block fb(fp);

		const int size = 1 << 17;

		for (int i = 0; i < size; i += pattern1_length)
		{
			fb.append((void*)pattern1, pattern1_length);
		}

		bool result;
		result = fb.buffer_count() == 0;
		_tests->test({ "append_continuity", result, __FILE__, __LINE__ });

		fb.commit();
		fb.wait();

		char check_buffer[size];
		fb.read(0, check_buffer, size);
		std::string temp;

		bool file_contents_check = true;
		const char* c = &check_buffer[0];
		int l = 0;
		int i = 0;

		while (i < size)
		{
			if (l == pattern1_length) {
				if (temp != pattern1) {
					file_contents_check = false;
					break;
				}
				l = 0;
				temp = "";
			}

			temp += *c;

			i++;
			l++;
			c++;
		}

		result = file_contents_check;
		_tests->test({ "file_contents", result, __FILE__, __LINE__ });

		fb.clear();
		result = fb.buffer_count() == 0;
		_tests->test({ "clear_buffers", result, __FILE__, __LINE__ });

		// test lone block read.

		memset(check_buffer, 0, sizeof(check_buffer));
		fb.read(pattern1_length, check_buffer, pattern1_length);
		result = strcmp(pattern1, check_buffer) == 0;
		_tests->test({ "checklone", result, __FILE__, __LINE__ });

		// test inner write
		// we cleared the buffers, and now there are none. 
		// writing should create a buffer.

		fb.write(pattern1_length + 4, (void*)pattern3, pattern3_length);

		memset(check_buffer, 0, sizeof(check_buffer));
		fb.read(pattern1_length + 4, check_buffer, pattern3_length);

		result = strcmp(pattern3, check_buffer) == 0 and fb.buffer_count() == 1;
		_tests->test({ "inner_write", result, __FILE__, __LINE__ });

		// test tail write
		// this should create another buffer

		fb.write(pattern1_length * 2 + 4, (void*)pattern3, pattern3_length);

		memset(check_buffer, 0, sizeof(check_buffer));
		fb.read(pattern1_length * 2 + 4, check_buffer, pattern3_length);

		result = strcmp(pattern3, check_buffer) == 0 and fb.buffer_count() == 1;
		_tests->test({ "tail write", result, __FILE__, __LINE__ });

		// test spanning read
		// expect this should take us back down to one buffer,
		// with the read being correct.

		memset(check_buffer, 0, sizeof(check_buffer));
		fb.read(0, check_buffer, pattern1_length * 3);

		const char* test_pattern = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123YYYY89ABCDEFGHIJKLMNOPQRSTUVWXYZ0123YYYY89ABCDEFGHIJKLMNOPQRSTUVWXYZ";

		result = strcmp(test_pattern, check_buffer) == 0 and fb.buffer_count() == 1;
		_tests->test({ "read_span", result, __FILE__, __LINE__ });

		fb.commit();
		fb.clear();

		// test fragmented writes, with a spanning read.

		for (int i = 0; i < pattern1_length * 3; i += pattern1_length)
		{
			char write_it[10];
			strcpy_s(write_it, "!");
			fb.write(i + 10, (void*)write_it, 1);
			memset(check_buffer, 0, 10);
			fb.read(i + 10, check_buffer, 1);
			check_buffer[7] = 0;
			if (strcmp(check_buffer, "!") != 0) {
				proof_assertions.put_member("read_frags", false);
				system_monitoring_interface::global_mon->log_warning("read_frags failed", __FILE__, __LINE__);
				break;
			}
		}

		// and read back in.
		memset(check_buffer, 0, sizeof(check_buffer));
		fb.read(0, check_buffer, pattern1_length * 3);

		const char* test_pattern2 = "0123456789!BCDEFGHIJKLMNOPQRSTUVWXYZ0123YYYY89!BCDEFGHIJKLMNOPQRSTUVWXYZ0123YYYY89!BCDEFGHIJKLMNOPQRSTUVWXYZ";

		result = strcmp(test_pattern2, check_buffer) == 0 and fb.buffer_count() == 1;
		_tests->test({ "read_frags", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("test file_block", "stop", tx.get_elapsed_seconds(), __FILE__, __LINE__);

	}

	void test_object(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		corona::json_parser jp;
		corona::json		empty;

		date_time start = date_time::now();
		timer tx;	

		system_monitoring_interface::global_mon->log_function_start("test object", "start", start, __FILE__, __LINE__);

		json parse_result = jp.create_object();
		bool result;

		corona::json test_negative = jp.parse_object(R"({ "name:"bill", "age":42 })");
		result = not test_negative.error();
		_tests->test({ "negative_parse", result, __FILE__, __LINE__ });

		corona::json test_eq1 = jp.parse_object(R"({ "name":"bill", "age":42 })");
		corona::json test_eq2 = jp.parse_object(R"({ "name":"bill", "age":42 })");

		result = test_eq1.object() and test_eq2.object();
		_tests->test({ "parse", result, __FILE__, __LINE__ });

		json member_result = jp.create_object();

		result = test_eq1.size() == 2 and test_eq2.size() == 2;
		_tests->test({ "object_size", result, __FILE__, __LINE__ });

		result = not test_eq1.has_member("box");
		_tests->test({ "negative_member", result, __FILE__, __LINE__ });
		
		result = test_eq1.has_member("name");
		_tests->test({ "member1", result, __FILE__, __LINE__ });

		result = test_eq1.has_member("age");
		_tests->test({ "member2", result, __FILE__, __LINE__ });

		json comparison_results = jp.create_object();

		result = empty.compare(test_eq1) < 0;
		_tests->test({ "compare <", result, __FILE__, __LINE__ });

		result = test_eq1.compare(test_eq2) == 0;
		_tests->test({ "compare = natural", result, __FILE__, __LINE__ });

		test_eq1.set_compare_order({ "name", "age" });
		test_eq2.set_compare_order({ "name", "age" });

		result = test_eq2.compare(test_eq1) == 0;
		_tests->test({ "compare = ordered", result, __FILE__, __LINE__ });

		json test_eq3 = test_eq2.clone();

		result = test_eq2.compare(test_eq3) == 0;
		_tests->test({ "clone compare = ", result, __FILE__, __LINE__ });

		corona::json test_lt1 = jp.parse_object(R"({ "name":"zak", "age":34 })");
		corona::json test_lt2 = jp.parse_object(R"({ "name":"zak", "age":37 })");
		test_lt1.set_compare_order({ "name", "age" });

		result = test_lt1.compare(test_lt2) < 0;
		_tests->test({ "< weak_order", result, __FILE__, __LINE__ });

		test_lt2.set_compare_order({ "name", "age" });

		result = test_lt2.compare(test_lt1) >= 0;
		_tests->test({ ">= weak_order", result, __FILE__, __LINE__ });

		json array_results = jp.create_object();

		corona::json test_array = jp.parse_array(R"(
[
{ "name":"holly", "age":37, "sex":"female" },
{ "name":"susan", "age":22, "sex":"female" },
{ "name":"tina", "age":19, "sex":"female" },
{ "name":"kirsten", "age":19, "sex":"female" },

{ "name":"cheri", "age":22, "sex":"female" },
{ "name":"dorothy", "age":22, "sex":"female" },
{ "name":"leila", "age":25, "sex":"female" },
{ "name":"dennis", "age":40, "sex":"male" },

{ "name":"kevin", "age":44, "sex":"male" },
{ "name":"kirk", "age":42, "sex":"male" },
{ "name":"dan", "age":25, "sex":"male" },
{ "name":"peter", "age":33, "sex":"male" }
])");

		result = test_array.size() == 12;
		_tests->test({ "array_size", result, __FILE__, __LINE__ });

		std::vector<std::string> names = { "holly", "susan", "tina", "kirsten", "cheri", "dorothy", "leila", "dennis", "kevin", "kirk", "dan", "peter" };

		int xidx = 0;
		bool barray_test = true;
		for (auto arr : test_array) {
			std::string name = arr["name"];
			if (name != names[xidx]) {
				system_monitoring_interface::global_mon->log_warning("array name not loaded correctly", __FILE__, __LINE__);
				barray_test = false;
				break;
			}
			xidx++;
		}

		_tests->test({ "array_element", barray_test, __FILE__, __LINE__ });

		result = (xidx == 12);
		_tests->test({ "array_enumerationn", result, __FILE__, __LINE__ });

		result = test_array.any([](json& _item) {
			return (double)_item["age"] > 35;
			});
		_tests->test({ "array_any", result, __FILE__, __LINE__ });

		result = test_array.all([](json& _item) {
			return (double)_item["age"] > 17;
			});
		_tests->test({ "array_all", result, __FILE__, __LINE__ });

		corona::json test_woh1 = jp.parse_object(R"({ "name":"bill", "age":42 })");
		corona::json test_woh2 = jp.parse_object(R"({ "name":"bill", "age":42 })");
		corona::json test_woh3 = jp.parse_object(R"({ "name":"bob", "age":12 })");
		corona::json test_woh4 = jp.parse_object(R"({ "name":"bob", "age":42 })");
		corona::json test_woh5 = jp.parse_object(R"({ "name":"greg", "age":12 })");
		corona::json test_woh6 = jp.parse_object(R"({ "name":"greg", "age":14 })");

		bool weak_ordered_hashing_test = true;

		uint64_t hash_woh1 = test_woh1.get_weak_ordered_hash({ "name","age" });
		uint64_t hash_woh2 = test_woh2.get_weak_ordered_hash({ "name","age" });
		uint64_t hash_woh3 = test_woh3.get_weak_ordered_hash({ "name","age" });
		uint64_t hash_woh4 = test_woh4.get_weak_ordered_hash({ "name","age" });
		uint64_t hash_woh5 = test_woh5.get_weak_ordered_hash({ "name","age" });
		uint64_t hash_woh6 = test_woh6.get_weak_ordered_hash({ "name","age" });

		uint64_t hash_woh1s = test_woh1.get_weak_ordered_hash({ "name" });
		uint64_t hash_woh2s = test_woh2.get_weak_ordered_hash({ "name" });
		uint64_t hash_woh3s = test_woh3.get_weak_ordered_hash({ "name" });
		uint64_t hash_woh4s = test_woh4.get_weak_ordered_hash({ "name" });
		uint64_t hash_woh5s = test_woh5.get_weak_ordered_hash({ "name" });
		uint64_t hash_woh6s = test_woh6.get_weak_ordered_hash({ "name" });

		if (hash_woh1 != hash_woh2) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash == failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh2 > hash_woh3) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh3 > hash_woh4) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh4 > hash_woh5) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh5 > hash_woh6) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}

		if (hash_woh1s != hash_woh2s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash == failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh2s > hash_woh3s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh3s != hash_woh4s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh5s > hash_woh6s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh1s > hash_woh3s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}
		if (hash_woh4s > hash_woh5s) {
			system_monitoring_interface::global_mon->log_warning("weak ordered hash < failed", __FILE__, __LINE__);
			weak_ordered_hashing_test = false;
		}

		_tests->test({ "weak_ordered_hash", weak_ordered_hashing_test, __FILE__, __LINE__ });

		json test_array_group = test_array.array_to_object(
			[](json& _item)->std::string {
				double age = _item["age"];
				if (age < 21)
				{
					return "teen";
				}
				else if (age < 30)
				{
					return "adult";
				}
				else
				{
					return "middle";
				}
			},
			[](json& _item)->json {
				return _item;
			}
		);

		result = (test_array_group["teen"].array() and
			test_array_group["adult"].array() and
			test_array_group["middle"].array() and
			test_array_group["teen"].size() == 2 and
			test_array_group["adult"].size() == 5 and
			test_array_group["middle"].size() == 5
			);
		_tests->test({ "group", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("test json", "stop", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_file(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("file proof", "start",  st, __FILE__, __LINE__);


		json_parser jp;

		std::shared_ptr<file> dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_data_block_test.ctb");

		file_block fb(dtest);

		char buffer_write[2048], buffer_read[2048];

		strcpy_s(buffer_write, R"({ "name": "test" })");
		int l = strlen(buffer_write) + 1;

		int64_t location = fb.add(1000);
		int64_t sz = fb.size();

		bool result = sz >= 1000 and location >= 0;
		_tests->test({ "append", result, __FILE__, __LINE__ });

		file_command_result tsk = fb.write(location, (void *)buffer_write, l);

		result = tsk.success == true and l == tsk.bytes_transferred;
		_tests->test({ "write", result, __FILE__, __LINE__ });

		file_command_result tsk2 = fb.read(location, (void*)buffer_read, l);

		result = tsk2.success == true and l == tsk2.bytes_transferred;
		_tests->test({ "read", result, __FILE__, __LINE__ });

		result = strcmp(buffer_write, buffer_read) == 0;
		_tests->test({ "round_trip", result, __FILE__, __LINE__ });

		fb.commit();
		fb.clear();

		file_command_result tsk3 = fb.read(location, (void*)buffer_read, l);

		result = (tsk3.success == true and l == tsk3.bytes_transferred);
		_tests->test({ "physical read 1", result, __FILE__, __LINE__ });

		result = strcmp(buffer_write, buffer_read) == 0;
		_tests->test({ "physical read 2", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("file proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

	}

	void test_data_block(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("block proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_data_block_test.ctb");

		file_block fb(dtest);

		json_parser jp;

		json jx = jp.parse_object(R"({ "name":"bill", "age":42 })");

		string_block dfirst, dfirstread, dbounds;

		dfirst.data = jx.to_json_typed();

		std::string dfs = dfirst.data;
		std::string jxs = jx.to_json_typed();

		bool result;

		result = (dfs == jxs);
		_tests->test({ "assignment", result, __FILE__, __LINE__ });
		
		int64_t dfirstlocation = dfirst.append(&fb);

		dbounds.data = "test border string";
		std::string borders = dbounds.data;

		int64_t dboundslocation = dbounds.append(&fb);

		int64_t dfirstlocationread = dfirstread.read(&fb, dfirstlocation);

		int64_t dboundslocationread = dbounds.read(&fb, dboundslocation);

		std::string sc, sb;
		sc = dfirstread.data;		
		sb = dfirst.data;

		result = sc == sb;
		_tests->test({ "append and read", result, __FILE__, __LINE__ });

		std::string borders_after;
		borders_after = dbounds.data;

		result = (borders == borders_after);
		_tests->test({ "write", result, __FILE__, __LINE__ });

		json jx2 = jp.parse_object(R"({ "name":"bill", 
			"age":42, 
			"weight":185, 
			"agency":"super alien and paranormal investigative unit", 
			"declaration" : "When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation. We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.--That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, --That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness. Prudence, indeed, will dictate that Governments long established should not be changed for light and transient causes; and accordingly all experience hath shewn, that mankind are more disposed to suffer, while evils are sufferable, than to right themselves by abolishing the forms to which they are accustomed. But when a long train of abuses and usurpations, pursuing invariably the same Object evinces a design to reduce them under absolute Despotism, it is their right, it is their duty, to throw off such Government, and to provide new Guards for their future security.--Such has been the patient sufferance of these Colonies; and such is now the necessity which constrains them to alter their former Systems of Government. The history of the present King of Great Britain is a history of repeated injuries and usurpations, all having in direct object the establishment of an absolute Tyranny over these States. To prove this, let Facts be submitted to a candid world."
			 })");
		sc = jx2.to_json_typed();

		dfirst.data = sc;
		std::string sbefore = dfirst.data;
		dfirst.write(&fb);
		
		sb = dfirst.data;
		result = (sc == sb);
		_tests->test({ "grow 1", result, __FILE__, __LINE__ });
		dbounds.read(&fb, dboundslocation);

		borders_after = dbounds.data;
		result = (borders_after == borders);
		_tests->test({ "neighbor", result, __FILE__, __LINE__ });

		fb.commit();
		fb.clear();

		dbounds.read(&fb, dboundslocation);

		borders_after = dbounds.data;

		result = (borders_after == borders);
		_tests->test({ "persistent", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("block proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_json_node(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("node proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_json_node_test.ctb");

		file_block fb(dtest);

		json_parser jp;

		json_data_block jnfirst, jnsecond;

		jnfirst.data = jp.create_array();
		for (double i = 0; i < 42; i++)
		{
			jnfirst.data.append_element( i );
		}

		bool append_worked = true;

		int64_t locfirst = jnfirst.append(&fb);

		if (locfirst < 0)
		{
			system_monitoring_interface::global_mon->log_warning("node append failed.", __FILE__, __LINE__);
			append_worked = false;
		}

		jnsecond.data = jp.parse_object(R"({ "Star" : "Aldeberran" })");

		int64_t locsecond = jnsecond.append(&fb);

		bool result;

		result = (locsecond >= 0);
		_tests->test({ "append", result, __FILE__, __LINE__ });

		jnfirst.read(&fb, locfirst);

		bool read_success = true;

		result = jnfirst.data.size() == 42;
		_tests->test({ "read array", result, __FILE__, __LINE__ });

		for (int i = 0; i < 42; i++) {
			int x = jnfirst.data.get_element(i);
			if (x != i) {
				read_success = false;
			}
			jnfirst.data.put_element(i, (double)(i * 10));
		}

		bool write_success = true;

		relative_ptr_type write_result = jnfirst.write(&fb);

		result = write_result >= 0;
		_tests->test({ "write array", result, __FILE__, __LINE__ });

		bool grow_success = true;
		jnfirst.data = jp.create_array();

		for (int i = 0; i < 10000; i++)
		{
			double t = i;
			jnfirst.data.push_back(t);
		}

		relative_ptr_type loc = jnfirst.write(&fb);

		result = (loc >= 0);
		_tests->test({ "write big", result, __FILE__, __LINE__ });

		auto rs = jnfirst.read(&fb, locfirst);

		result = rs >= 0;
		_tests->test({ "read big", result, __FILE__, __LINE__ });

		for (int i = 0; i < 10000; i++) {
			int x = jnfirst.data.get_element(i);
			if (x != i) {
				grow_success = false;
			}
			jnfirst.data.put_element(i, (double)(i * 10));
		}

		fb.commit();
		fb.clear();

		json_key_block jkn;

		jkn.hash_code = 42;
		for (int i = 0; i < 16; i++)
		{
			jkn.foward.push_back(i);
		}

		int64_t key_location = jkn.append(&fb);

		json_key_block read_back;
		read_back.read(&fb, key_location);

		bool key_success = true;

		result = (jkn.hash_code == read_back.hash_code and jkn.foward.size() == read_back.foward.size());
		_tests->test({ "key basics", result, __FILE__, __LINE__ });

		for (int i = 0; i < read_back.foward.size(); i++)
		{
			if (read_back.foward[i] != jkn.foward[i]) {

				system_monitoring_interface::global_mon->log_warning("foward ptrs failed.", __FILE__, __LINE__);
				key_success = false;
				break;
			}
		}
		result = key_success;
		_tests->test({ "key all", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("node proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_json_table(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("table proof", "start", st, __FILE__, __LINE__);

		using return_type = bool;
		json_parser jp;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");
		file_block fp(f);

		std::shared_ptr<json_table_header> header = std::make_shared<json_table_header>();
		header->create(&fp);

		json test_write = jp.create_object();
		test_write.put_member_i64(object_id_field, 5);
		test_write.put_member("Name", "Joe");
		json test_key = test_write.extract({ object_id_field });

		json_table test_table(header, 0, &fp, {object_id_field});

		auto read_header = test_table.get_table_header();

		bool result;

		result = (bool)read_header;
		_tests->test({ "create table", result, __FILE__, __LINE__ });

		relative_ptr_type rpt =  test_table.put(test_write);
		result = (rpt >= 0);
		_tests->test({ "put", result, __FILE__, __LINE__ });

		json test_read =  test_table.get(test_key);
		result = not test_read.empty();
		_tests->test({ "read 1", result, __FILE__, __LINE__ });

		test_read.set_compare_order({ object_id_field });
		result = test_read.compare(test_write) == 0;
		_tests->test({ "check read 1", result, __FILE__, __LINE__ });

		json db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		result = db_contents.array();
		_tests->test({ "array result", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Jack");
		test_key = test_write.extract({ object_id_field });
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

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1 = db_contents.size();

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Jill");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);
		test_read.set_natural_order();

		result = test_read.compare(test_write) == 0;
		_tests->test({ "read 3", result, __FILE__, __LINE__ });

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1b = db_contents.size();

		try
		{
			test_key.put_member_i64(object_id_field, 6);
			json t5 =  test_table.get(test_key);
			_tests->test({ "null access", true, __FILE__, __LINE__ });
		}
		catch (std::exception exc)
		{
			_tests->test({ "null access", false, __FILE__, __LINE__ });
		}

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
		});

		result = (db_contents.size() == 2);
		_tests->test({ "table results size", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 2);
		test_write.put_member("Name", "Sydney");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert", result, __FILE__, __LINE__ });

		test_read = jp.create_object();
		test_read.put_member_i64(object_id_field, 5);
		test_read.set_natural_order();

		joe = test_table.get(test_read);
		result = not joe.empty();
		_tests->test({ "insert front", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Orwell");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "update", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 1);
		test_write.put_member("Name", "Canada");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert first 2", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 7);
		test_write.put_member("Name", "Roger");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "update 2", result, __FILE__, __LINE__ });

		test_write.put_member_i64(object_id_field, 1);
		test_write.put_member("Name", "Maraca");
		test_key = test_write.extract({ object_id_field });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		result = test_read.compare(test_write) == 0;
		_tests->test({ "insert first 2", result, __FILE__, __LINE__ });

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		result = test_table.size() == 4;
		_tests->test({ "table size", result, __FILE__, __LINE__ });
		result = db_contents.size() == 4;
		_tests->test({ "select size", result, __FILE__, __LINE__ });

		int64_t tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		json counts = jp.create_object();
		json *pcounts = &counts;

		auto fr = test_table.for_each([tests,pcounts](int _index, json& _item) -> bool {
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

		db_contents =  test_table.select([tests](int _index, json& _item) -> json {
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

		db_contents =  test_table.select(search_key, [tests](int _index, json& _item) -> json {
			return _item;
			}
		);

		any_fails = db_contents.any([](json& _item)->bool {
			std::string temp = _item["Name"];
			return temp != "Zeus";
			});

		result = not any_fails;
		_tests->test({ "any 2", result, __FILE__, __LINE__ });

		test_key.put_member_i64(object_id_field, 3);
		result =  not test_table.erase(test_key);
		_tests->test({ "erase negative", result, __FILE__, __LINE__ });

		test_key.put_member_i64(object_id_field, 1);
		result =  test_table.erase(test_key);
		_tests->test({ "erase", result, __FILE__, __LINE__ });

		test_key.put_member_i64(object_id_field, 7);
		result =  test_table.erase(test_key);
		_tests->test({ "erase tail", result, __FILE__, __LINE__ });

		json testi =  test_table.select([tests](int _index, json& item) -> json {
			int64_t object_id = item[object_id_field];
			return object_id == 7 ? item : json();
			});

		result = (testi.size() == 0);
		_tests->test({ "table after erase", result, __FILE__, __LINE__ });

		db_contents =  test_table.select([tests](int _index, json& item) {
			return item;
			}
		);

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item[object_id_field];
			return  object_id != 2 and object_id != 5;
			});

		result = not any_iteration_fails;
		_tests->test({ "table correct", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("table proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif

