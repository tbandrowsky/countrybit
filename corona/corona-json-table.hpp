/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is a table in a json database, implemented with a skip list.

Notes

For Future Consideration
*/


#ifndef CORONA_JSON_TABLE_H
#define CORONA_JSON_TABLE_H

namespace corona 
{

	class json_node;

	const int debug_json_table = 0;

	// data blocks
	class data_block;

	// poco nodes
	template <typename T> class poco_node;

	// json nodes
	class json_node;

	// nesting of transactions

	class data_block
	{
	public:

		block_header_struct				header;
		buffer							bytes;

		data_block()
		{
			header = {};
			header.block_type = block_id::general_id();
		}

		data_block &operator = (json& _src)
		{
			std::string json_string = _src.to_json_typed();
			bytes = buffer(json_string.c_str());
			return *this;
		}

		std::string get_string()
		{
			std::string x = bytes.get_ptr();
			return x;
		}

		void init(int _size_bytes)
		{
			bytes.init(_size_bytes);
		}

		int64_t get_size()
		{
			int size = bytes.get_size();
			return size;
		}

		virtual void on_read()
		{
			;
		}

		relative_ptr_type read(file* _file, relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "read block", start_time, __FILE__, __LINE__);

			file_command_result header_result = _file->read(location, &header, sizeof(header));

			if (header_result.success) 
			{
				bytes = buffer(header.data_length);
				file_command_result data_result = _file->read(header.data_location, bytes.get_ptr(), header.data_length);

				if (data_result.success) 
				{
					on_read();
					system_monitoring_interface::global_mon->log_block_stop("block", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return header_result.location; // want to make this 0 or -1 if error
				}
				else 
				{
					system_monitoring_interface::global_mon->log_function_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}

			system_monitoring_interface::global_mon->log_block_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return -1i64;
		}

		virtual void on_write()
		{
			;
		}

		relative_ptr_type write(file* _file, std::function<block_header_struct(int64_t _size)> _allocator)
		{

			if (header.block_location < 0) 
			{
				throw std::invalid_argument("use append for new blocks");
			}

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "write block", start_time, __FILE__, __LINE__);

			on_write();

			int size = bytes.get_size();

			if (size > header.data_length)
			{
				if (_allocator) {
					auto temp_header = _allocator(size);
					header.data_length = temp_header.data_length;
					header.data_location = temp_header.data_location;
				}
				else {
					return -size;
				}
			}

			file_command_result data_result = _file->write(header.data_location, bytes.get_ptr(), size);

			if (data_result.success)
			{
				file_command_result header_result = _file->write(header.block_location, &header, sizeof(header));
				system_monitoring_interface::global_mon->log_block_stop("block", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return header_result.location;
			}
			system_monitoring_interface::global_mon->log_block_stop("block", "write failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return -1i64;
		}

		relative_ptr_type append(file* _file, std::function<block_header_struct(int64_t _size)> _allocator)
		{
			int size = bytes.get_size();

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "append", start_time, __FILE__, __LINE__);

			header = _allocator(size);
			write(_file, _allocator);

			system_monitoring_interface::global_mon->log_block_stop("block", "append", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return header.block_location;
		}
	};

	class json_node : public data_block
	{
	public:

		json				data;

		json_node()
		{
		}

		void clear()
		{
			json_parser jp;
			data = jp.create_object();
			std::string x = data.to_json_typed_string();
			bytes = buffer(x.c_str());
		}

		virtual void on_read()
		{
			const char *contents = bytes.get_ptr();
			if (contents) {
				json_parser jp;
				data = jp.parse_object(contents);
			}
		}

		virtual void on_write()
		{
			std::string x = data.to_json_typed_string();
			bytes = buffer(x.c_str());
		}

	};

	template <typename poco_type> class poco_node : public data_block
	{
	public:

		poco_node()
		{
			clear();
		}

		void clear()
		{
			data = {};
		}

		poco_type				data;

		virtual void on_read()
		{
			poco_type* contents = bytes.get_ptr();
			if (contents) {
				data = *contents;
			}
		}

		virtual void on_write()
		{
			bytes = data;
		}

		poco_node& operator = (const poco_type& _src)
		{
			data = _src;
			return *this;
		}

	};

	class json_table
	{
	private:

		poco_node<index_header_struct>	index_header;
		std::shared_ptr<file>			database_file;

		using KEY = json;
		using VALUE = json_node;
		using UPDATE_VALUE = json;

		const int SORT_ORDER = 1;
		
		std::vector<std::string> key_fields;
		json header_key;

		relative_ptr_type create_header()
		{
			json_parser jp;
			index_header.append(database_file.get(), [this](int64_t _size) {
				return allocate(block_id::json_id(), _size);
				});
			return index_header.header.block_location;
		}

		relative_ptr_type find_node(json _key)
		{
			int64_t hash_code = _key.get_hash_code();
			int64_t block_index = hash_code % index_header.data.index.length;
			json_node jn;

			auto& list_start = index_header.data.index[block_index];

			if (list_start.first_free_block)
			{
				auto block_location = list_start.first_free_block;

				// see if our block is in here, if so, update it.

				while (block_location)
				{
					jn.read(database_file.get(), block_location);
					if (_key.compare(jn.data) == 0) {
						return block_location;
					}
					else
					{
						block_location = jn.header.next_block;
					}
				}
			};

			return -1i64;
		}

		json_node put_node(json_node _data)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json_table", "put_node", start_time, __FILE__, __LINE__);
		
			json node_key = _data.data.extract(key_fields);
			int64_t hash_code = node_key.get_hash_code();
			int64_t block_index = hash_code % index_header.data.index.length;

			auto &list_start = index_header.data.index[ block_index ];
			
			if (list_start.first_free_block) 
			{
				json_node jn;
				auto block_location = list_start.first_free_block;

				// see if our block is in here, if so, update it.

				while (block_location) 
				{
					jn.read(database_file.get(), block_location);
					if (node_key.compare(jn.data) == 0) {
						jn.data = _data.data;
						jn.write(database_file.get(), [this](int64_t _size) -> block_header_struct {
							return allocate(block_id::json_id(), _size);
							});
						return jn;
					}
					else 
					{
						block_location = jn.header.next_block;
					}
				}

				// if not, then we must come up with one.
				auto old_last = list_start.last_free_block;		
				new_node.put_data(_data);
				new_node.write(database_file.get(), [this](int64_t _size) -> block_header_struct {
					return allocate(block_id::json_id(), _size);
					});
				json_node pn;
				pn.read(database_file.get(), old_last);
				pn.storage.header.next_block = new_node.storage.current_location;
				pn.write(database_file.get(), [this](int64_t _size) -> block_header_struct {
					return allocate(block_id::json_id(), _size);
					});
				list_start.last_free_block = new_node.storage.current_location;
				return new_node;
			}
			else 
			{
				new_node.put_data(_data);
				new_node.write(database_file.get(), [this](int64_t _size) -> block_header_struct {
					return allocate(block_id::json_id(), _size);
					});
			}	

			return new_node;
		}

		json_node get_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;

			node.read(_file, _node_location);
			return node;
		}

		void erase_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;
		}

		int get_allocation_index(int64_t _size)
		{
			int64_t sz = 1;
			int idx = 0;
			while (sz < _size) {
				sz *= 2;
				idx++;
			}
			return idx;
		}

		block_header_struct allocate(block_id _block_id, int64_t _size)
		{
			relative_ptr_type pt = 0;

			int index = get_allocation_index(_size);
			int max_index = index_header.data.free_lists.length - 1;

			if (index > max_index)
				index = max_index;

			auto& list_start = index_header.data.free_lists[index];

			block_header_struct bhs;

			if (list_start.first_free_block) {

				database_file.get()->read(list_start.first_free_block, &bhs, sizeof(bhs));

				if (list_start.last_free_block == list_start.first_free_block)
				{
					list_start.last_free_block = 0;
					list_start.first_free_block = 0;
				}
				else
				{
					list_start.first_free_block = bhs.next_block;
					bhs.next_block = 0;
				}

				database_file.get()->write(bhs.block_location, &bhs, sizeof(bhs));
				index_header.write(database_file.get());
			}
			else
			{
				bhs.block_type = _block_id;
				bhs.block_location = database_file->add(sizeof(block_header_struct));
				bhs.data_length = 1i64 << index;
				bhs.data_location = database_file->add(bhs.data_length);
				bhs.next_block = 0;

				database_file.get()->write(bhs.block_location, &bhs, sizeof(bhs));
				index_header.write(database_file.get());
			}
			return bhs;
		}

		void free(block_header_struct _bhs)
		{
			relative_ptr_type pt = 0;

			int index = get_allocation_index(_bhs.data_length);
			int max_index = index_header.data.free_lists.length - 1;

			if (index > max_index)
				index = max_index;

			auto& list_start = index_header.data.free_lists[index];

			if (list_start.last_free_block) {
				block_header_struct last_block;
				database_file->read(list_start.last_free_block, &last_block, sizeof(last_block));
				last_block.next_block = _bhs.block_location;
				list_start.last_free_block = _bhs.block_location;
				database_file->write(last_block.block_location, &last_block, sizeof(block_header_struct));
				database_file->write(_bhs.block_location, &_bhs, sizeof(block_header_struct));
				index_header.write(database_file.get());
			}
			else
			{
				list_start.first_free_block = _bhs.block_location;
				list_start.last_free_block = _bhs.block_location;
				_bhs.next_block = 0;
				database_file->write(_bhs.block_location, &_bhs, sizeof(block_header_struct));
				index_header.write(database_file.get());
			}
		}

		int get_hash_index(json& _src_key, int _hash_code)
		{
			int64_t hash_code = _src_key.get_hash_code();
			int64_t idx = hash_code % _hash_code;
		}

	public:

		json_table(std::shared_ptr<file> _database_file, std::vector<std::string> _key_fields) 
			: database_file(_database_file), 
				key_fields(_key_fields)
		{
			json_parser jp;
			header_key = jp.create_object();
		}

		json_table(const json_table& _src) 
			: key_fields(_src.key_fields)
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
			header_key = _src.header_key;
		}

		json_table operator = (const json_table& _src)
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
			key_fields = _src.key_fields;
			header_key = _src.header_key;
			return *this;
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

		relative_ptr_type create()
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "create", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  create_header();
			system_monitoring_interface::global_mon->log_table_stop("table", "create complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return location;
		}

		relative_ptr_type open(relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "open", start_time, __FILE__, __LINE__);
			 index_header.read(database_file.get(), location);
			system_monitoring_interface::global_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return location;
		}

		bool contains(const KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "contains", start_time, __FILE__, __LINE__);

			relative_ptr_type result =  find_node(key);
			system_monitoring_interface::global_mon->log_table_stop("table", "contains complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return  result != null_row;
		}

		json get(std::string _key)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);

			json_parser jp;
			json key = jp.parse_object(_key);

			key.set_natural_order();

			if (key.is_member("ClassName", "SysParseError")) {
				std::cout << key.to_json() << std::endl;
				return key;
			}

			json result;
			relative_ptr_type n =  find_node(key);
			if (n != null_row) {
				json_node r =  get_node(database_file.get(), n);
				result = r.data;
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		json get(const KEY key)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);

			json result;
			relative_ptr_type n =  find_node(key);
			if (n != null_row) {
				json_node r =  get_node(database_file.get(), n);
				result = r.data;
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		json get(const KEY key, std::initializer_list<std::string> include_fields)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get", start_time, __FILE__, __LINE__);

			json result;
			relative_ptr_type n =  find_node(key);
			if (n != null_row) {
				json_node r =  get_node(database_file.get(), n);
				if (!r.data.empty()) {
					result = r.data.extract(include_fields);
				}
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		relative_ptr_type put(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);

			json_node jn;
			jn.data = value;

			json_node nd = put_node(jn);
			system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return nd.header.block_location;
		}

		relative_ptr_type put(std::string _json)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);

			json_parser jp;
			json jx = jp.parse_object(_json);
			if (jx.empty() || jx.is_member("ClassName", "SysParseError")) {
				std::cout << jx.to_json() << std::endl;
				return null_row;
			}
			json_node jn;
			jn.data = jx;
			jn = put_node(jn);
			system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return jn.header.block_location;
		}

		relative_ptr_type replace(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);

			auto key = get_key(value);

			auto node_location = find_node(key);

			if (node_location > -1) {
				json_node jnz;
				jnz.read(database_file.get(), node_location);
				jnz.data.assign_replace(value);
				jnz.write(database_file.get(), [this](int64_t _size) {
					return allocate(block_id::json_id(), _size);
					})
			}

			system_monitoring_interface::global_mon->log_table_stop("table", "replace", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return node_location;
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
			system_monitoring_interface::global_mon->log_table_start("table", "erase", start_time, __FILE__, __LINE__);


			system_monitoring_interface::global_mon->log_table_stop("table", "erase failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return false;
		}

		int64_t co_for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json_node& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "co_for_each", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;

			while (location != null_row) 
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0) 
				{
					relative_ptr_type process_result =  _process_clause(index, node);
					if (process_result > 0)
					{
						index++;
					}
				}
				location = node.forward[0];
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "co_for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return index;
		}

		int64_t for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0)
				{
					relative_ptr_type process_result = _process_clause(index, node.data);
					if (process_result > 0)
					{
						index++;
					}
				}
				location = node.forward[0];
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return index;
		}

		json get_first(json _key_fragment)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;
			json result_data;

			while (location != null_row)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0)
				{
					result_data = node.data;
					location = null_row;
				} 
				else {
					location = node.forward[0];
				}
			}

			system_monitoring_interface::global_mon->log_table_stop("table", "get_first complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result_data;
		}

		bool any(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "any", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;
			bool is_any = false;

			while (location != null_row && !is_any)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0)
				{
					relative_ptr_type process_result = _process_clause(index, node.data);
					if (process_result > 0)
					{
						index++;
						is_any = true;
					}
					location = node.forward[0];
				}
				else
				{
					location = null_row;
				}
			}

			system_monitoring_interface::global_mon->log_table_stop("table", "is_any complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return is_any;
		}

		bool all(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "all", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;
			bool is_all = true;

			while (location != null_row && is_all)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0)
				{
					relative_ptr_type process_result = _process_clause(index, node.data);
					if (process_result > 0)
					{
						index++;
						is_all = false;
					}
					location = node.forward[0];
				}
				else
				{
					location = null_row;
				}
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "is_all complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return is_all;
		}

		int64_t co_for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "co_for_each", start_time, __FILE__, __LINE__);

			auto header =  get_header();

			relative_ptr_type location = header.forward[0];
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				relative_ptr_type process_result =  _process_clause(index, node.data);
				if (process_result > 0)
				{
					index++;
				}
				location = node.forward[0];
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "co_for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return index;
		}

		int64_t for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);

			auto header =  get_header();

			relative_ptr_type location = header.forward[0];
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				relative_ptr_type process_result = _process_clause(index, node.data);
				if (process_result > 0)
				{
					index++;
				}
				location = node.forward[0];
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return index;
		}

		json get_first(json _key_fragment, std::function<bool(json& _src)> _fn)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);

			relative_ptr_type location =  find_first_node_gte(_key_fragment);
			int64_t index = 0;
			json result_data;

			while (location != null_row)
			{
				json_node node;
				node =  get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0 && _fn(node.data))
				{
					result_data = node.data;
					location = null_row;
				}
				else 
				{
					location = node.forward[0];
				}
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "get_first complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result_data;
		}

		json select_array(std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "select_array", start_time, __FILE__, __LINE__);

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			int64_t count =  co_for_each([pja, _project](int _index, json& _data) ->relative_ptr_type
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data);
					if (!new_item.empty() && !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count = 1;
					}
					return count;
				});

			system_monitoring_interface::global_mon->log_table_stop("table", "select_array complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return ja;
		}

		json select_array(json _key_fragment, 
			std::function<json(int _index, json& _item)> _project,
			json _update = "{ }"_jobject
		)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "select_array", start_time, __FILE__, __LINE__);

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			int64_t count =  co_for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_node& _data) -> relative_ptr_type 
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data.data);
					if (!new_item.empty()&& !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count++;
						if (_update.object() && _update.size()>0) {
							auto key = get_key(_data.data);
							update_node(key, [_update](UPDATE_VALUE& dest) { dest.assign_update(_update); });							
						}
					}
					return count;
				});

			system_monitoring_interface::global_mon->log_table_stop("table", "select_array complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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
			system_monitoring_interface::global_mon->log_table_start("table", "select_object", start_time, __FILE__, __LINE__);

			int64_t count =  co_for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_node& _jdata) -> int64_t
				{
					json _data = _jdata.data;
					json new_item = _project(_index, _data);
					if (!new_item.empty()) {
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
			system_monitoring_interface::global_mon->log_table_stop("table", "select_object complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return _destination;
		}


		inline int size() { return index_header.data.count; }


	private:

		int randomLevel()
		{
			double r = ((double)rand() / (RAND_MAX));
			int level = (int)(log(1. - r) / log(1. - .5));
			if (level < 1) level = 0;
			else if (level >= JsonTableMaxLevel) level = JsonTableMaxLevel;
			return level;
		}

	private:

		// compare a node to a key for equality
		// return -1 if the node < key
		// return 1 if the node > key
		// return 0 if the node == key

	
	};

	std::ostream& operator <<(std::ostream& output, json_table & src)
	{
		std::cout << "[json_table]" << std::endl;
		return output;
	}

	bool test_json_table(std::shared_ptr<application> _app);
	relative_ptr_type test_file(std::shared_ptr<application> _app);
	int64_t test_data_block(std::shared_ptr<application> _app);
	int64_t test_json_node(std::shared_ptr<application> _app);

	relative_ptr_type test_file(std::shared_ptr<application> _app)
	{

		std::cout << "\ntest_file: entry, thread:" << ::GetCurrentThreadId() << std::endl;

		file dtest = _app->create_file(FOLDERID_Documents, "corona_data_block_test.ctb");

		char buffer_write[2048], buffer_read[2048];

		strcpy_s(buffer_write, R"({ "name": "test" })");
		int l = strlen(buffer_write) + 1;

		dtest.add(1000);

		std::cout << "\ntest_file:  write, thread:" << ::GetCurrentThreadId() << std::endl;
		file_command_result tsk = dtest.write(0, (void *)buffer_write, l);

		std::cout << "\ntest_file:  read, thread:" << ::GetCurrentThreadId() << std::endl;
		file_command_result tsk2 = dtest.read(0, (void*)buffer_read, l);

		std::cout << "\ntest_file:  read, thread:" << ::GetCurrentThreadId() << std::endl;
		
		if (!strcmp(buffer_write, buffer_read))
		{
			std::cout << "\ntest_file: return success " << 42 << ", thread:" << ::GetCurrentThreadId() << std::endl;
			return 42;
		}

		std::cout << "\ntest_file: return fail, thread:" << ::GetCurrentThreadId() << std::endl;
		return 0;
	}

	int64_t test_data_block(std::shared_ptr<application> _app)
	{
		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_data_block_test.ctb");

		std::cout << "test_data_block, thread:" << ::GetCurrentThreadId() << std::endl;

		json_parser jp;
		json jx = jp.parse_object(R"({ "name":"bill", "age":42 })");
		data_block db, dc;
		db = jx;
		std::cout << "test_data_block, write, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r1 =  db.append(dtest.get());
		
		std::cout << "test_data_block, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r2 =  dc.read(dtest.get(), db.current_location);

		std::cout << "test_data_block_nested, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = dc.get_string();
		std::cout << x << std::endl;
		return 32;
	}

	int64_t test_json_node(std::shared_ptr<application> _app)
	{
		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_json_node_test.ctb");

		std::cout << "test_json_node, thread:" << ::GetCurrentThreadId() << std::endl;

		json_node jnwrite, jnread;
		json_parser jp;

		jnwrite.data = jp.create_array();
		for (double i = 0; i < 42; i++)
		{
			jnwrite.data.append_element( i );
		}

		std::cout << "test_json_node, write, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t location =  jnwrite.append(dtest.get());

		std::cout << "test_json_node, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t bytes_ex =  jnread.read(dtest.get(), location);
		
		std::cout << "test_json_node, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = jnread.data.to_json_string();
		std::cout << x << std::endl;
		return 1;
	}

	bool test_json_table(std::shared_ptr<application> _app)
	{
		using return_type = bool;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");

		json_parser jp;

		json test_write = jp.create_object();
		test_write.put_member_i64("ObjectId", 5);
		test_write.put_member("Name", "Joe");
		json test_key = test_write.extract({ "ObjectId" });

		json_table test_table(f, {"ObjectId"});

		 test_table.create();

		relative_ptr_type rpt =  test_table.put(test_write);
		json test_read =  test_table.get(test_key);
		test_read.set_compare_order( {"ObjectId"});

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}
		json db_contents =  test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jack");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		db_contents =  test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jill");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		db_contents =  test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		try
		{
			test_key.put_member_i64("ObjectId", 6);
			json t5 =  test_table.get(test_key);
		}
		catch (std::exception exc)
		{
			std::cout << __LINE__ << " fail: wrong null access." << std::endl;
			return false;
		}

		db_contents =  test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		if (db_contents.size() != 2)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			return false;
		}

		test_write.put_member_i64("ObjectId", 2);
		test_write.put_member("Name", "Sydney");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Zeus");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Canada");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Zeus");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Maraca");
		test_key = test_write.extract({ "ObjectId" });
		 test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		db_contents =  test_table.select_array([](int _index, json& item) {
			return item;
			});

		if (test_table.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements in test." << std::endl;
			return false;
		}

		if (db_contents.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			return false;
		}
		
		int64_t tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		json counts = jp.create_object();
		json *pcounts = &counts;

		relative_ptr_type count =  test_table.for_each([tests,pcounts](int _index, json& _item) -> bool {

			int64_t test_index = tests[_index];
			std::string member_names = std::format("item{0}", (int64_t)_item["ObjectId"]);
			int64_t counto = 0;
			if (pcounts->has_member(member_names)) {
				counto = pcounts->get_member(member_names);
			}
			counto++;
			pcounts->put_member_i64(member_names, counto);
			return 1;
			});

		std::string count_string = counts.to_json();

		std::cout << count_string << std::endl;

		db_contents =  test_table.select_array([tests](int _index, json& _item) -> json {
			int64_t temp = _item["ObjectId"];
			return (temp > 0i64) ? _item : json();
			}
		);

		std::cout << "..." << std::endl;
		std::cout << "Check negative objects." << std::endl;

		bool any_fails = db_contents.any([](json& _item)->bool {
			int64_t temp = _item["ObjectId"];
			return temp <= 0i64;
			});

		if (any_fails) {
			std::cout << __LINE__ << " query failed" << std::endl;
			return false;
		}

		std::cout << db_contents.to_json() << std::endl;

		// and break it up
		std::cout << "..." << std::endl;
		std::cout << "Create summary object." << std::endl;

		auto summary = db_contents.array_to_object([](json& _item) {
			return (std::string)_item["ObjectId"];
			},
			[](json& _target) {
				return _target;
			});

		std::cout << summary.to_json() << std::endl;

		// and, see, now we can explore partial keys

		std::cout << "..." << std::endl;
		std::cout << "Extract only those that have the name Zeus." << std::endl;

		json search_key = jp.create_object("Name", "Zeus");
		search_key.set_compare_order({ "Name" });

		db_contents =  test_table.select_array(search_key, [tests](int _index, json& _item) -> json {
			return _item;
			}
		);

		any_fails = db_contents.any([](json& _item)->bool {
			std::string temp = _item["Name"];
			return temp != "Zeus";
			});

		if (any_fails) {
			std::cout << __LINE__ << " find bad key query failed" << std::endl;
			return false;
		}

		std::cout << db_contents.to_json() << std::endl;

		test_key.put_member_i64("ObjectId", 3);
		bool rdel3 =  test_table.erase(test_key);

		if (rdel3) {
			std::cout << "delete non-existing failed" << std::endl;
		}

		test_key.put_member_i64("ObjectId", 1);
		bool rdel1 =  test_table.erase(test_key);

		if (!rdel1) {
			std::cout << "delete existing failed" << std::endl;
		}

		test_key.put_member_i64("ObjectId", 7);
		relative_ptr_type rdel7 =  test_table.erase(test_key);

		if (!rdel7) {
			std::cout << "delete existing failed" << std::endl;
		}

		json testi =  test_table.select_array([tests](int _index, json& item) -> json {
			int64_t object_id = item["ObjectId"];
			return object_id == 7 ? item : json();
			});

		if (testi.size() > 0) {
			std::cout << __LINE__ << " retrieved a deleted item" << std::endl;
			return false;
		}
		 
		db_contents =  test_table.select_array([tests](int _index, json& item) {
			return item;
			}
		);

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item["ObjectId"];
			return  object_id != 2 && object_id != 5;
			});

		if (any_iteration_fails) {
			std::cout << __LINE__ << " iteration after delete failed." << std::endl;
			return false;
		}

		return true;
	}

}

#endif
