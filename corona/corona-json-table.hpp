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

	struct allocation_index
	{
		int		index;
		int64_t size;
	};

	// root data block.  Everything is a block.  It's almost like minecraft, except, it makes money
	// and thus far, I do not.
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

		static allocation_index get_allocation_index(int64_t _size)
		{
			allocation_index ai = { };

			int64_t sz = 1;
			int idx = 0;
			while (sz < _size) {
				sz *= 2;
				idx++;
			}
			ai.index = idx;
			ai.size = sz;
			return ai;
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
				bytes = buffer(header.data_size);
				file_command_result data_result = _file->read(header.data_location, bytes.get_ptr(), header.data_size);

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

		virtual void on_write(int _offset, int _size)
		{
			;
		}

		relative_ptr_type write_piece(file* _file, int _offset, int _size)
		{
			if (header.block_location < 0)
			{
				throw std::invalid_argument("cannot append a partial write of a block");
			}

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "write piece", start_time, __FILE__, __LINE__);

			on_write(_offset, _size);

			file_command_result data_result = _file->write(header.data_location + _offset, bytes.get_ptr() + _offset, _size);

			if (data_result.success)
			{
				system_monitoring_interface::global_mon->log_block_stop("block", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return data_result.location;
			}
			system_monitoring_interface::global_mon->log_block_stop("block", "write failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return -1i64;
		}

		relative_ptr_type write(file* _file, 
			std::function<void(int64_t _location)> _deleter,
			std::function<int64_t(int64_t _size)> _allocator)
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

			if (size > header.data_capacity)
			{
				int64_t new_capacity = header.data_capacity;

				if (new_capacity < 1) 
					new_capacity = 1;

				while (new_capacity < size) {
					new_capacity *= 2;
				}

				// the header.data_length is the max size of the block.  Since there's stuff past that in the file, then,
				// there's not going to be a way we can write this, so we must have another block.
				if (_allocator and _deleter)
				{
					_deleter(header.data_location);
					header.data_size = size;
					header.data_capacity = new_capacity;
					header.data_location = _allocator(new_capacity);
				}
				else 
				{
					system_monitoring_interface::global_mon->log_warning("Block size too big to fit and an allocator was not provided.", __FILE__, __LINE__);
					return -size;
				}
			}
			else 
			{
				header.data_size = size;
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

		relative_ptr_type append(file* _file, std::function<int64_t(int64_t _size)> _allocator)
		{

			on_write();

			int size = bytes.get_size();
			auto size_ai = get_allocation_index(size);

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "append", start_time, __FILE__, __LINE__);

			header.block_location = _allocator(sizeof(header));
			header.block_type = block_id::table_id();
			header.data_size = bytes.get_size();
			header.data_capacity = size_ai.size;
			header.data_location = _allocator(size_ai.size);

			if (header.block_location < 0 or header.data_location < 0)
				return -1;

			auto hdr_status = _file->write(header.block_location, &header, sizeof(header));
			auto data_status = _file->write(header.data_location, bytes.get_ptr(), bytes.get_size());

			system_monitoring_interface::global_mon->log_block_stop("block", "append", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			if (not (hdr_status.success and data_status.success)) {
				return -1;
			}

			return header.block_location;
		}

		void erase(std::function<void(int64_t _location)> _deleter)
		{
			_deleter(header.data_location);
			_deleter(header.block_location);
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
			std::string x = data.to_json_typed();
			bytes = buffer(x.c_str());
		}

		virtual void on_read()
		{
			const char *contents = bytes.get_ptr();
			if (contents) {
				json_parser jp;
				if (contents[0] == '[') {
					data = jp.parse_array(contents);
				}
				else 
					data = jp.parse_object(contents);
			}
		}

		virtual void on_write()
		{
			std::string x = data.to_json_typed();
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
			poco_type* contents = (poco_type *)bytes.get_ptr();
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

	class json_table_header : public poco_node<index_header_struct>
	{
	public:

		int64_t write_free_list(file *_file, int _index)
		{
			auto offset = offsetof(index_header_struct, free_lists) + _index * sizeof(list_block_header);
			auto size = sizeof(list_block_header);
			memcpy(bytes.get_ptr() + offset, (const char*)&data + offset, size);
			auto r = write_piece(_file, offset, size);
			return r;
		}

		int64_t write_count(file* _file)
		{
			auto offset = offsetof(index_header_struct, count);
			auto size = sizeof(data.count);
			memcpy(bytes.get_ptr() + offset, (const char*)&data + offset, size);
			auto r = write_piece(_file, offset, size);
			return r;
		}

	};

	class json_tree_node : public poco_node<tree_block_header> 
	{
	public:
		json_tree_node() 
		{
			header = {};
			data = {};
		}

		json_tree_node(const json_tree_node& _src)
		{
			header = _src.header;
			data = _src.data;
		}

		json_tree_node &operator = (const json_tree_node& _src)
		{
			header = _src.header;
			data = _src.data;
			return *this;
		}

		relative_ptr_type write_child(file* _file, int _index)
		{
			if (header.block_location < 0)
			{
				throw std::invalid_argument("cannot append a partial write of a block");
			}

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "write child", start_time, __FILE__, __LINE__);

			int64_t child_base = offsetof(tree_block_header, children);
			int64_t offset = child_base + _index * sizeof(data.children[0]);

			file_command_result data_result = _file->write(header.data_location + offset, &data.children[_index], sizeof(data.children[0]));

			if (data_result.success)
			{
				system_monitoring_interface::global_mon->log_block_stop("block", "write child", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return data_result.location;
			}
			system_monitoring_interface::global_mon->log_block_stop("block", "write child", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return -1i64;
		}

	};

	class json_table
	{
	private:

		json_table_header				table_header;
		std::shared_ptr<file>			database_file;
		std::map<uint64_t, int64_t>		location_cache;

		using KEY = json;
		using VALUE = json_node;
		using UPDATE_VALUE = json;
		
		std::vector<std::string> key_fields;
		json header_key;

		relative_ptr_type create_header()
		{
			json_parser jp;
			table_header.append(database_file.get(), [this](int64_t _size) {
				return allocate(_size);
				});

			json_tree_node jtn;

			jtn.data = {};
			table_header.write(database_file.get(), nullptr, nullptr);

			return table_header.header.block_location;
		}

		json find_nodes(json _array)
		{
			json_parser jp;

			json_node jn;
			timer tx;

			if (key_fields.size() > 1) {
				comm_bus_interface::global_bus->log_warning("find_nodes doesn't work with multiple keys", __FILE__, __LINE__);
			}

			json results = jp.create_array();

			system_monitoring_interface::global_mon->log_put("find_node start", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			json grouped_by_hash = _array.group([this](json& _item)->std::string {
				json node_key = _item.extract(key_fields);
				hashbytes hash_bytes;
				uint64_t hash_code = get_hash_bytes(node_key, hash_bytes);
				return std::to_string(hash_code);
				});

			json_tree_node jtn;

			// if there is nothing in our tree, create a header

			auto objects_by_hash = grouped_by_hash.get_members();

			for (auto obj : objects_by_hash) {

				hashbytes hash_bytes;
				uint64_t hash_code = std::strtoull(obj.first.c_str(), nullptr, 10);
				get_hash_bytes(hash_code, hash_bytes);

				json_tree_node jtn;

				int64_t location = table_header.data.data_root_location;

				list_block_header* list_start = nullptr;

				if (location_cache.contains(hash_code)) {
					location = location_cache[hash_code];
					auto status = jtn.read(database_file.get(), location);
					list_start = &jtn.data.index_list;
					system_monitoring_interface::global_mon->log_put("cached", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				else {

					int level_index = 0;

					for (level_index = 0; level_index < 8; level_index++)
					{
						auto status = jtn.read(database_file.get(), location);
						byte local_hash_code = hash_bytes[level_index];
						location = jtn.data.children[local_hash_code];
					}
					auto status = jtn.read(database_file.get(), location);
					list_start = &jtn.data.index_list;

				//	location_cache.insert_or_assign(hash_code, jtn.header.block_location);
				}

				log_put("found hash", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);

				std::string key_name = key_fields[0];

				json grouped = obj.second.group([key_name](json& _item) -> std::string {
					return _item[key_name];
					});

				if (list_start->first_block)
				{
					system_monitoring_interface::global_mon->log_information("block scan:" + key_name + ":" + std::to_string(list_start->first_block));
					auto block_location = list_start->first_block;

					// see if our block is in here, if so, update it.

					while (block_location)
					{
						jn.read(database_file.get(), block_location);
						std::string key_value = jn.data[key_name];
						system_monitoring_interface::global_mon->log_information("check node:" + key_value);
						if (grouped.has_member(key_value))
						{
							results.push_back(jn.data);
						}
						block_location = jn.header.next_block;
					}
				};
			}
			return results;
		}

		relative_ptr_type find_node(json _key)
		{
			json_node jn;
			timer tx;

			system_monitoring_interface::global_mon->log_put("find_node start", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			hashbytes hash_bytes;

			uint64_t hash_code = get_hash_bytes(_key, hash_bytes);

			json_tree_node jtn;
			
			int64_t location = table_header.data.data_root_location;
			if (location <= 0)
				return null_row;

			list_block_header* list_start = nullptr;

			if (location_cache.contains(hash_code)) {
				location = location_cache[hash_code];
				auto status = jtn.read(database_file.get(), location);
				list_start = &jtn.data.index_list;
				system_monitoring_interface::global_mon->log_put("cached", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else {

				int level_index = 0;

				for (level_index = 0; level_index < 8; level_index++)
				{
					auto status = jtn.read(database_file.get(), location);
					byte local_hash_code = hash_bytes[level_index];
					location = jtn.data.children[local_hash_code];
					if (location == 0)
						return null_row;
				}
				auto status = jtn.read(database_file.get(), location);
				list_start = &jtn.data.index_list;
				//location_cache.insert_or_assign(hash_code, jtn.header.block_location);
			}

			log_put("found", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);

			if (list_start->first_block)
			{
				auto block_location = list_start->first_block;

				// see if our block is in here, if so, update it.

				while (block_location)
				{
					jn.read(database_file.get(), block_location);
					int c = _key.compare(jn.data);
					if (c == 0) {
						return block_location;
					}
					else if (c > 0)
					{
						block_location = jn.header.next_block;
					}
					else {
						return null_row;
					}
				}
			};

			return -1i64;
		}


		void log_put(std::string _place, json_tree_node& _jtn, double _elapsed_seconds, const char* _file = nullptr, int _line = 0)
		{
			std::string msg = std::format("{0:10} {1:<10}",
				_place,
				_jtn.header.block_location);

			system_monitoring_interface::global_mon->log_put(msg, _elapsed_seconds, _file, _line);

		}

		using hashbytes = std::vector<byte>;

		int64_t get_hash_bytes(uint64_t hash_code, hashbytes& _bytes)
		{
			int64_t thash_code = hash_code;

			_bytes.resize(8);

			for (int i = 7; i >= 0; i--) {
				_bytes[i] = thash_code % 256;
				thash_code >>= 8;
			}

			return hash_code;
		}

		int64_t get_hash_bytes(json& _node_key, hashbytes& _bytes)
		{
			uint64_t hash_code = _node_key.get_weak_ordered_hash(key_fields);
			int64_t thash_code = hash_code;

			_bytes.resize(8);

			for (int i = 7; i >= 0; i--) {
				_bytes[i] = thash_code % 256;
				thash_code >>= 8;
			}

			return hash_code;
		}

		void put_nodes(json _array)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json_table", "put_node", start_time, __FILE__, __LINE__);

			json grouped_by_hash = _array.group([this](json& _item)->std::string {
				json node_key = _item.extract(key_fields);
				hashbytes hash_bytes;
				uint64_t hash_code = get_hash_bytes(node_key, hash_bytes);
				return std::to_string(hash_code);
			});

			json_tree_node jtn;

			// if there is nothing in our tree, create a header

			auto objects_by_hash = grouped_by_hash.get_members();

			for (auto obj : objects_by_hash) {

				hashbytes hash_bytes;
				uint64_t hash_code = std::strtoull(obj.first.c_str(), nullptr, 10);
				get_hash_bytes(hash_code, hash_bytes);

				list_block_header* list_start = nullptr;

				int level_index = 0;
				int64_t location;

				log_put("start put", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);

				if (location_cache.contains(hash_code)) {
					location = location_cache[hash_code];
					auto status = jtn.read(database_file.get(), location);
					list_start = &jtn.data.index_list;
					system_monitoring_interface::global_mon->log_put("cached", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				else {

					if (table_header.data.data_root_location == 0) {
						json_tree_node jtn;
						jtn.data = {};
						jtn.header.next_block = 0;
						table_header.data.data_root_location = jtn.append(database_file.get(), [this](int64_t _size_t) {
							return allocate(_size_t);
							});
						table_header.write(database_file.get(), nullptr, nullptr);
						log_put("new root", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);
					};

					location = table_header.data.data_root_location;

					std::string path = "";

					for (level_index = 0; level_index < 8; level_index++)
					{
						auto status = jtn.read(database_file.get(), location);
						byte local_hash_code = hash_bytes[level_index];
						path = path + std::to_string(local_hash_code) + ".";
						location = jtn.data.children[local_hash_code];
						if (location == 0) {
							json_tree_node ntn;
							ntn.data = {};
							location = ntn.append(database_file.get(), [this](int64_t _size_t) {
								return allocate(_size_t);
								});
							jtn.data.children[local_hash_code] = location;
							jtn.write_child(database_file.get(), local_hash_code);
						}
					}

					auto status = jtn.read(database_file.get(), location);
					list_start = &jtn.data.index_list;

					path = path + "->" + std::to_string(jtn.header.block_location);
					system_monitoring_interface::global_mon->log_put(path, tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}

				if (location_cache.size() > 10000) {
					location_cache.clear();
				}
				//location_cache.insert_or_assign(hash_code, jtn.header.block_location);

				for (json _data : obj.second)
				{
					// walk the tree, to figure out which list to put it in
					json node_key = _data.extract(key_fields);

					// now that we have our list, find out where in the list it goes
					json_node new_node;
					int64_t previous_block_location = 0;

					if (list_start->first_block)
					{

						system_monitoring_interface::global_mon->log_put("block exists", tx.get_elapsed_seconds(), __FILE__, __LINE__);

						auto block_location = list_start->first_block;
						json_node current_node, previous_node;

						// first, check to see if this is larger than anything at the end of the block, then we'll just append it...
						int comparison = 0;

						current_node.read(database_file.get(), list_start->last_block);
						comparison = node_key.compare(current_node.data);
						if (comparison > 0) {
							system_monitoring_interface::global_mon->log_put("append new node", tx.get_elapsed_seconds(), __FILE__, __LINE__);
							new_node.data = _data;
							new_node.header.next_block = 0;
							new_node.append(database_file.get(), [this](int64_t _size) {
								return allocate(_size);
								});
							current_node.header.next_block = new_node.header.block_location;
							current_node.write(database_file.get(), nullptr, nullptr);
							list_start->last_block = new_node.header.block_location;
							jtn.write(database_file.get(), nullptr, nullptr);
							table_header.data.count++;
							continue;
						}

						// see if our block is in here, if so, update it.

						while (block_location)
						{
							current_node.read(database_file.get(), block_location);

							comparison = node_key.compare(current_node.data);

							if (comparison > 0) {
								previous_block_location = block_location;
								previous_node = current_node;
								block_location = current_node.header.next_block;
							}
							else {
								break;
							}
						}

						if (block_location > 0 and comparison < 0) {
							system_monitoring_interface::global_mon->log_put("insert into list", tx.get_elapsed_seconds(), __FILE__, __LINE__);

							new_node.data = _data;
							if (previous_block_location > 0) {
								new_node.header.next_block = current_node.header.block_location;
								new_node.append(database_file.get(), [this](int64_t _size) {
									return allocate(_size);
									});
								previous_node.header.next_block = new_node.header.block_location;
								previous_node.write(database_file.get(), nullptr, nullptr);
							}
							else {
								new_node.header.next_block = list_start->first_block;
								new_node.data = _data;
								new_node.append(database_file.get(), [this](int64_t _size) {
									return allocate(_size);
									});
								list_start->first_block = new_node.header.block_location;
								jtn.write(database_file.get(), nullptr, nullptr);
							}
							// this is an insert, so we do write the count
							table_header.data.count++;
							continue;
						}
						else if (block_location > 0 and comparison == 0) {
							system_monitoring_interface::global_mon->log_put("update existing", tx.get_elapsed_seconds(), __FILE__, __LINE__);
							current_node.data = _data;
							current_node.write(database_file.get(),
								[this](int64_t location) -> void {
									free(location);
								},
								[this](int64_t size) -> int64_t {
									return allocate(size);
								}
							);
							system_monitoring_interface::global_mon->log_json_stop("json_table", "put_node", tx.get_elapsed_seconds(), __FILE__, __LINE__);
							// this is an update, so we do not write the count
							continue;
						}
						else
						{
							system_monitoring_interface::global_mon->log_put("add to end", tx.get_elapsed_seconds(), __FILE__, __LINE__);
							new_node.data = _data;
							new_node.header.next_block = 0;
							new_node.append(database_file.get(), [this](int64_t _size) {
								return allocate(_size);
								});
							current_node.header.next_block = new_node.header.block_location;
							current_node.write(database_file.get(), nullptr, nullptr);
							list_start->last_block = new_node.header.block_location;
							jtn.write(database_file.get(), nullptr, nullptr);
							table_header.data.count++;
							continue;
						}

						system_monitoring_interface::global_mon->log_warning("Should not be here", __FILE__, __LINE__);
					}
					else
					{
						system_monitoring_interface::global_mon->log_put("new block", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						new_node.data = _data;
						new_node.header.next_block = 0;
						new_node.append(database_file.get(), [this](int64_t _size) -> int64_t {
							return allocate(_size);
							});
						list_start->first_block = list_start->last_block = new_node.header.block_location;
						jtn.write(database_file.get(), nullptr, nullptr);

						table_header.data.count++;
					}
				}
				table_header.write_count(database_file.get());

			}

			system_monitoring_interface::global_mon->log_json_stop("json_table", "put_node", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return;
		}

		json_node put_node(json _data)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json_table", "put_node", start_time, __FILE__, __LINE__);

			json node_key = _data.extract(key_fields);
			hashbytes hash_bytes;

			uint64_t hash_code = get_hash_bytes(node_key, hash_bytes);

			json_tree_node jtn;

			// if there is nothing in our tree, create a header

			list_block_header* list_start = nullptr;

			int level_index = 0;
			int64_t location;

			log_put("start put", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);

			if (location_cache.contains(hash_code)) {
				location = location_cache[hash_code];
				auto status = jtn.read(database_file.get(), location);
				list_start = &jtn.data.index_list;
				system_monitoring_interface::global_mon->log_put("cached", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else {

				if (table_header.data.data_root_location == 0) {
					json_tree_node jtn;
					jtn.data = {};
					jtn.header.next_block = 0;
					table_header.data.data_root_location = jtn.append(database_file.get(), [this](int64_t _size_t) {
						return allocate(_size_t);
						});
					table_header.write(database_file.get(), nullptr, nullptr);
					log_put("new root", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);
				};

				location = table_header.data.data_root_location;

				std::string path = "";

				for (level_index = 0; level_index < 8; level_index++)
				{
					path = path + std::to_string(location) + ".";
					auto status = jtn.read(database_file.get(), location);
					byte local_hash_code = hash_bytes[level_index];
					location = jtn.data.children[local_hash_code];
					if (location == 0) {
						json_tree_node ntn;
						ntn.data = {};
						location = ntn.append(database_file.get(), [this](int64_t _size_t) {
							return allocate(_size_t);
							});
						jtn.data.children[local_hash_code] = location;
						jtn.write_child(database_file.get(), local_hash_code);
					}
				}

				auto status = jtn.read(database_file.get(), location);
				list_start = &jtn.data.index_list;

				path = path + "->" + std::to_string(jtn.header.data_location);
				system_monitoring_interface::global_mon->log_put(path, tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			// walk the tree, to figure out which list to put it in

			// now that we have our list, find out where in the list it goes
			json_node new_node;
			int64_t previous_block_location = 0;

			if (list_start->first_block) 
			{
				if (location_cache.size() > 10000) {
					location_cache.clear();
				}
				//location_cache.insert_or_assign(hash_code, jtn.header.block_location);

				system_monitoring_interface::global_mon->log_put("block exists", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				auto block_location = list_start->first_block;
				json_node current_node, previous_node;

				// first, check to see if this is larger than anything at the end of the block, then we'll just append it...
				int comparison = 0;

				if (list_start->last_block) {
					current_node.read(database_file.get(), list_start->last_block);
					comparison = node_key.compare(current_node.data);
					if (comparison > 0) {
						system_monitoring_interface::global_mon->log_put("append new node", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						new_node.data = _data;
						new_node.header.next_block = 0;
						new_node.append(database_file.get(), [this](int64_t _size) {
							return allocate(_size);
							});
						current_node.header.next_block = new_node.header.block_location;
						current_node.write(database_file.get(), nullptr, nullptr);
						list_start->last_block = new_node.header.block_location;
						jtn.write(database_file.get(), nullptr, nullptr);
						table_header.data.count++;
						table_header.write_count(database_file.get());
						return new_node;
					}
				}

				// see if our block is in here, if so, update it.

				while (block_location)
				{
					current_node.read(database_file.get(), block_location);

					comparison = node_key.compare(current_node.data);

					if (comparison > 0) {
						previous_block_location = block_location;
						previous_node = current_node;
						block_location = current_node.header.next_block;
					}
					else {
						break;
					}
				}

				if (block_location > 0 and comparison < 0) {
					system_monitoring_interface::global_mon->log_put("insert into list", tx.get_elapsed_seconds(), __FILE__, __LINE__);

					new_node.data = _data;
					if (previous_block_location > 0) {
						new_node.header.next_block = current_node.header.block_location;
						new_node.append(database_file.get(), [this](int64_t _size) {
							return allocate(_size);
							});
						previous_node.header.next_block = new_node.header.block_location;
						previous_node.write(database_file.get(), nullptr, nullptr);
					}
					else {
						new_node.header.next_block = list_start->first_block;
						new_node.data = _data;
						new_node.append(database_file.get(), [this](int64_t _size) {
							return allocate(_size);
							});
						list_start->first_block = new_node.header.block_location;
						jtn.write(database_file.get(), nullptr, nullptr);
					}
					// this is an insert, so we do write the count
					table_header.data.count++;
					table_header.write_count(database_file.get());
					return new_node;
				}
				else if (block_location > 0 and comparison == 0) {
					system_monitoring_interface::global_mon->log_put("update existing", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					current_node.data = _data;
					current_node.write(database_file.get(),
						[this](int64_t location) -> void {
							free(location);
						}, 
						[this](int64_t size) -> int64_t {
							return allocate(size);
						}
					);
					system_monitoring_interface::global_mon->log_json_stop("json_table", "put_node", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					// this is an update, so we do not write the count
					return current_node;
				}
				else 
				{
					system_monitoring_interface::global_mon->log_put("add to end", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					new_node.data = _data;
					new_node.header.next_block = 0;
					new_node.append(database_file.get(), [this](int64_t _size) {
						return allocate(_size);
						});
					current_node.header.next_block = new_node.header.block_location;
					current_node.write(database_file.get(), nullptr, nullptr);
					list_start->last_block= new_node.header.block_location;
					jtn.write(database_file.get(), nullptr, nullptr);
					table_header.data.count++;
					table_header.write_count(database_file.get());
					return new_node;
				}

				system_monitoring_interface::global_mon->log_warning("Should not be here", __FILE__, __LINE__);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_put("new block", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				new_node.data = _data;
				new_node.header.next_block = 0;
				new_node.append(database_file.get(), [this](int64_t _size) -> int64_t {
					return allocate(_size);
					});
				list_start->first_block = list_start->last_block = new_node.header.block_location;
				jtn.write(database_file.get(), nullptr, nullptr);

				table_header.data.count++;
				table_header.write_count(database_file.get());
			}

			system_monitoring_interface::global_mon->log_json_stop("json_table", "put_node", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return new_node;
		}

		json_node get_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;
			node.read(_file, _node_location);
			return node;
		}

		void erase_node(file* _file, relative_ptr_type _node_location)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json_table", "erase_node", start_time, __FILE__, __LINE__);

			auto jn = get_node(_file, _node_location);

			json node_key = jn.data.extract(key_fields);

			system_monitoring_interface::global_mon->log_put("find_node start", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			hashbytes hash_bytes;

			get_hash_bytes(node_key, hash_bytes);

			json_tree_node jtn;

			int64_t location = table_header.data.data_root_location;
			if (location <= 0)
				return;

			list_block_header* list_start = nullptr;

			int level_index = 0;

			for (level_index = 0; level_index < 8; level_index++)
			{
				auto status = jtn.read(database_file.get(), location);
				byte local_hash_code = hash_bytes[level_index];
				location = jtn.data.children[local_hash_code];
				if (location == 0)
					return;
			}
			auto status = jtn.read(database_file.get(), location);
			list_start = &jtn.data.index_list;

			log_put("found", jtn, tx.get_elapsed_seconds(), __FILE__, __LINE__);

			if (list_start->first_block)
			{
				int64_t previous_block_location = 0;
				int64_t block_location = list_start->first_block;
				json_node current_node;

				// see if our block is in here, if so, update it.

				while (block_location)
				{
					current_node.read(database_file.get(), block_location);

					if (block_location == _node_location) {
						if (previous_block_location) {
							json_node previous_node;
							previous_node.read(database_file.get(), previous_block_location);
							previous_node.header.next_block = current_node.header.next_block;
							previous_node.write(database_file.get(),
								[this](int64_t _size) -> void {
									return free(_size);
								},
								[this](int64_t _size) -> int64_t {
									return allocate(_size);
								});
						}
						if (list_start->first_block == block_location)
							list_start->first_block = current_node.header.next_block;
						if (list_start->last_block == block_location)
							list_start->last_block == current_node.header.next_block;
						jtn.write(database_file.get(), nullptr, nullptr);
						current_node.erase([this](int64_t _location) {
							free(_location);
						});
						block_location = 0;
					}
					else 
					{
						previous_block_location = block_location;
						block_location = current_node.header.next_block;
					}
				}

				// and now we gotta put this node into our index
				table_header.data.count--;
				table_header.write_count(database_file.get());
			}

			system_monitoring_interface::global_mon->log_json_stop("json_table", "get_node", tx.get_elapsed_seconds(), __FILE__, __LINE__);
		}

		allocation_index get_allocation_index(int64_t _size)
		{
			allocation_index ai = data_block::get_allocation_index(_size);
			if (ai.index >= table_header.data.free_lists.capacity()) {
				std::string msg = std::format("{0} bytes is too big to allocate as a block.", _size);
				system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
				ai.index = table_header.data.free_lists.capacity() - 1;
			}
			return ai;
		}

		relative_ptr_type allocate(int64_t _size)
		{
			relative_ptr_type pt = 0;

			allocation_index ai = get_allocation_index(_size);

			auto& list_start = table_header.data.free_lists[ai.index];

			block_header_struct free_block = {};

			// get_allocation index always returns an index such that any block in that index
			// will have the right size.  so we know the first block is ok.
			if (list_start.first_block)
			{
				database_file.get()->read(list_start.first_block, &free_block, sizeof(free_block));
				// this
				if (list_start.last_block == list_start.first_block)
				{
					list_start.last_block = 0;
					list_start.first_block = 0;
				}
				else
				{
					list_start.first_block = free_block.next_block;
					free_block.next_block = 0;
				}
				table_header.write_free_list(database_file.get(), ai.index);
				return free_block.data_location;
			}

			int64_t total_size = sizeof(block_header_struct) + ai.size;
			int64_t base_space = database_file->add(sizeof(block_header_struct) + ai.size);
			if (base_space > 0) {
				free_block.block_type = block_id::allocated_space_id();
				free_block.block_location = base_space;
				free_block.data_location = base_space + sizeof(block_header_struct);
				free_block.next_block = 0;
				database_file.get()->write(base_space, &free_block, sizeof(free_block));
				return free_block.data_location;
			}

			return 0;
		}

		void free(int64_t _location)
		{
			relative_ptr_type block_start = _location - sizeof(block_header_struct);

			block_header_struct free_block = {};

			/* ah, the forgiveness of this can be evil.  Here we say, if we truly cannot verify this 
			_location can be freed, then do not free it.  Better to keep some old block around than it is 
			to stomp on a good one with a mistake.  */

			file_command_result fcr = database_file->read(block_start, &free_block, sizeof(free_block));
			
			// did we read the block
			if (fcr.success) {

				// is this actually an allocated space block, and, is it the block we are trying to free
				if (free_block.block_type.is_allocated_space() and free_block.data_location == _location) {

					// ok, now let's have a go and see where this is in our allocation index.
					allocation_index ai = get_allocation_index(free_block.data_capacity);

					// this check says, don't try to free the block if there is a mismatch between its 
					// allocation size, and the size it says it has.
					if (free_block.data_capacity == ai.size) 
					{
						// and now, we can look at the free list, and put ourselves at the 
						// end of it.  In this way, deletes and reallocates will tend to 
						// want to reuse the same space so handy for kill and fills.
						auto& list_start = table_header.data.free_lists[ai.index];

						// there is a last block, so we are at the end
						if (list_start.last_block) {
							relative_ptr_type old_last_block = list_start.last_block;
							block_header_struct last_free;
							auto fr = database_file->read(old_last_block, &last_free, sizeof(last_free));
							if (fr.success) {
								list_start.last_block = block_start;
								last_free.next_block = block_start;
								free_block.next_block = 0;
								database_file->write(old_last_block, &last_free, sizeof(last_free));
								database_file->write(block_start, &free_block, sizeof(free_block));

								// this basically says, just write out the list header for this index
								// not the whole header
								table_header.write_free_list(database_file.get(), ai.index);
							}
						}
						else 
						{
							// the list is empty and we add to it.
							free_block.next_block = 0;
							list_start.last_block = free_block.block_location;
							list_start.last_block = free_block.block_location;

							// this basically says, now save our block.
							database_file->write(block_start, &free_block, sizeof(free_block));

							// this basically says, just write out the list header for this index
							// not the whole header
							table_header.write_free_list(database_file.get(), ai.index);
						}
					}
				}
			}
		}

	public:

		json_table(std::shared_ptr<file> _database_file, std::vector<std::string> _key_fields) 
			: database_file(_database_file), 
				key_fields(_key_fields)
		{
			json_parser jp;
			header_key = jp.create_object();
			table_header = {};
		}

		json_table(const json_table& _src) 
			: key_fields(_src.key_fields)
		{
			table_header = _src.table_header;
			database_file = _src.database_file;
			header_key = _src.header_key;
		}

		json_table operator = (const json_table& _src)
		{
			table_header = _src.table_header;
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
			 table_header.read(database_file.get(), location);
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

		json get_list(json _array)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get_list", start_time, __FILE__, __LINE__);
			json object_list = find_nodes(_array);

			system_monitoring_interface::global_mon->log_table_stop("table", "get_list", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return object_list;
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
				// create an empty object.
				system_monitoring_interface::global_mon->log_warning("key for object.get could not be parsed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_warning(key);
				key = jp.create_object();
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
				if (not r.data.empty()) {
					result = r.data.extract(include_fields);
				}
			}
			system_monitoring_interface::global_mon->log_table_stop("table", "get", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		void put_array(json _array)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "put_array", start_time, __FILE__, __LINE__);

			put_nodes(_array);
			system_monitoring_interface::global_mon->log_table_stop("table", "put_array", tx.get_elapsed_seconds(), __FILE__, __LINE__);
		}

		relative_ptr_type put(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "put", start_time, __FILE__, __LINE__);

			json_node nd = put_node(value);
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
			if (jx.empty() or jx.is_member("ClassName", "SysParseError")) {
				return null_row;
			}
			json_node jn = put_node(jx);
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
				jnz.write(database_file.get(), 
				[this](int64_t _location) {
					return free(_location);
					},
				[this](int64_t _size) {
					return allocate(_size);	
				});
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

			int64_t node_location = find_node(key);

			bool deleted = false;

			if (node_location > 0) {
				erase_node(database_file.get(), node_location);
				deleted = true;
			}

			system_monitoring_interface::global_mon->log_table_stop("table", "erase failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return deleted;
		}

		class for_each_result {
		public:
			bool is_any;
			bool is_all;
			int64_t count;
		};

		void visit(int64_t _node, for_each_result& _result, json _key_fragment, std::function<relative_ptr_type(int _index, json_node& _item)>& _process_clause)
		{
			json_tree_node jtn;

			if (_node <= 0)
				return;

			if (jtn.read(database_file.get(), _node) <= 0) 
			{
				return;
			}

			for (int i = 0; i < jtn.data.children.capacity(); i++)
			{
				int64_t child = jtn.data.children[i];
				if (child) 
				{
					visit( child, _result, _key_fragment, _process_clause);
				}
			}

			auto list_start = jtn.data.index_list;
			auto block_location = list_start.first_block;

			// see if our block is in here, if so, update it.

			json_node jn;

			while (block_location)
			{
				jn.read(database_file.get(), block_location);
				if (_key_fragment.empty() or _key_fragment.compare(jn.data) == 0) {
					relative_ptr_type process_result = _process_clause(_result.count, jn);
					if (process_result > 0)
					{
						_result.is_any = true;
						_result.count++;
					}
					else
					{
						_result.is_all = false;
					}
				}
				block_location = jn.header.next_block;
			}

		}

		for_each_result for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json_node& _item)> _process_clause)
		{

			for_each_result result = {};

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "for_each", start_time, __FILE__, __LINE__);

			result.is_all = true;

			visit(table_header.data.data_root_location, result, _key_fragment, _process_clause);

			system_monitoring_interface::global_mon->log_table_stop("table", "for_each complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json_node& _item)> _process_clause)
		{
			json empty_key;
			return for_each( empty_key, _process_clause);
		}

		for_each_result for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			json empty_key;
			return for_each(empty_key, [_process_clause](int _index, json_node& _jn) {
				return _process_clause(_index, _jn.data);
				});
		}

		json get_first(json _key_fragment, std::function<bool(json& _src)> _fn)
		{
			json empty;
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "get_first", start_time, __FILE__, __LINE__);

			auto index_lists = 0;

			json_node jn;

			uint64_t hash_code = _key_fragment.get_weak_ordered_hash(key_fields);

			json_tree_node jtn;

			int64_t location = table_header.data.data_root_location;
			if (location <= 0)
				return empty;

			list_block_header* list_start = nullptr;

			int64_t last_node_location = 0;

			int level_index;

			for (level_index = 0; level_index < 8; level_index++)
			{
				auto status = jtn.read(database_file.get(), location);
				for (int i = 0; i < jtn.data.children.capacity(); i++) {
					location = jtn.data.children[i];
					if (location > 0)
						break;
				}
				if (location == 0)
					return empty;
				list_start = &jtn.data.index_list;
			}

			if (list_start == nullptr) {
				return empty;
			}

			if (list_start->first_block)
			{
				auto block_location = list_start->first_block;
				jn.read(database_file.get(), block_location);
				return jn.data;
			};

			system_monitoring_interface::global_mon->log_table_stop("table", "get_first complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return empty;
		}

		json select(std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);

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

			system_monitoring_interface::global_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return ja;
		}

		json select(json _key_fragment, std::function<json(int _index, json& _item)> _project)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "select", start_time, __FILE__, __LINE__);

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			json empty_key = jp.create_object();

			auto result = for_each(_key_fragment, [pja, _project](int _index, json_node& _data) ->relative_ptr_type
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

			system_monitoring_interface::global_mon->log_table_stop("table", "select complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return ja;
		}

		json update(json _key_fragment, 
			std::function<json(int _index, json& _item)> _project,
			json _update = "{ }"_jobject
		)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "update", start_time, __FILE__, __LINE__);

			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			auto result =  for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_node& _data) -> relative_ptr_type 
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data.data);
					if (not new_item.empty()&& !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count++;
						if (_update.object() and _update.size()>0) {
							_data.data.assign_update(_update);
							_data.write(database_file.get(), [this](int64_t _size)->size_t {
								return _size;
							},
							 [this](int64_t _size)->size_t {
									return _size;
							});
						}
					}
					return count;
			});

			system_monitoring_interface::global_mon->log_table_stop("table", "update complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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

			for_each_result fra =  for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_node& _jdata) -> int64_t
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
			system_monitoring_interface::global_mon->log_table_stop("table", "select_object complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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

	void test_object(json& _proof, std::shared_ptr<application> _app);
	void test_file(json& _proof, std::shared_ptr<application> _app);
	void test_data_block(json& _proof, std::shared_ptr<application> _app);
	void test_json_node(json& _proof, std::shared_ptr<application> _app);
	void test_json_table(json& _proof, std::shared_ptr<application> _app);

	void test_object(json& _proof, std::shared_ptr<application> _app)
	{
		corona::json_parser jp;
		corona::json		empty;

		json proof_assertions = jp.create_object();

		proof_assertions.put_member("test_name", "object");

		date_time start = date_time::now();
		timer tx;	

		system_monitoring_interface::global_mon->log_function_start("test object", "start", start, __FILE__, __LINE__);

		json parse_result = jp.create_object();

		corona::json test_negative = jp.parse_object(R"({ "name:"bill", "age":42 })");
		if (not test_negative.error())
			parse_result.put_member("not_object_parse", false);
		else
			parse_result.put_member("not_object_parse", true);

		corona::json test_eq1 = jp.parse_object(R"({ "name":"bill", "age":42 })");
		corona::json test_eq2 = jp.parse_object(R"({ "name":"bill", "age":42 })");

		if (test_eq1.empty() or test_eq2.empty())
			parse_result.put_member("object_parse", false);
		else 
			parse_result.put_member("object_parse", true);		

		parse_result.prove_member("is_true");
		proof_assertions.put_member("parse", parse_result);

		json member_result = jp.create_object();

		if (test_eq1.size() == 2 and test_eq2.size() == 2) {
			member_result.put_member("object_size", true);
		}
		else {
			system_monitoring_interface::global_mon->log_warning("object size test failed", __FILE__, __LINE__);
			member_result.put_member("object_size", false);
		}

		if (test_eq1.has_member("box"))
		{
			system_monitoring_interface::global_mon->log_warning("negative membership test failed", __FILE__, __LINE__);
			member_result.put_member("negative_membership", false);
		}
		else 
		{
			member_result.put_member("negative_membership", true);
		}

		if (test_eq1.has_member("bill"))
		{
			system_monitoring_interface::global_mon->log_warning("positive membership basic test failed", __FILE__, __LINE__);
			member_result.put_member("positive_membership", false);
		}
		else 
		{
			member_result.put_member("negative_membership", true);
		}

		if (not test_eq1.has_member("age"))
		{
			system_monitoring_interface::global_mon->log_warning("positive membership extent test failed", __FILE__, __LINE__);
			member_result.put_member("extent_membership", false);
		}
		else 
		{
			member_result.put_member("negative_membership", true);
		}

		member_result.prove_member("is_true");
		proof_assertions.put_member("member_access", member_result);

		json comparison_results = jp.create_object();

		if (empty.compare(test_eq1) >= 0)
		{
			system_monitoring_interface::global_mon->log_warning("empty < test_eq1 failed", __FILE__, __LINE__);
			comparison_results.put_member("< empty comparison", false);
		}
		else {
			comparison_results.put_member("< empty comparison", true);
		}

		if (test_eq1.compare(test_eq2) != 0)
		{
			system_monitoring_interface::global_mon->log_warning("test_eq1 == test_eq2 failed", __FILE__, __LINE__);
			comparison_results.put_member("== comparison", false);
		}
		else {
			comparison_results.put_member("== comparison", true);
		}

		test_eq1.set_compare_order({ "name", "age" });
		if (test_eq1.compare(test_eq2) != 0)
		{
			system_monitoring_interface::global_mon->log_warning("test_eq1 == test_eq2 failed", __FILE__, __LINE__);
			comparison_results.put_member("== comparison", false);
		}
		else {
			comparison_results.put_member("== comparison", true);
		}

		test_eq2.set_compare_order({ "name", "age" });
		if (test_eq2.compare(test_eq1) != 0)
		{
			system_monitoring_interface::global_mon->log_warning("test_eq1 == test_eq2 failed", __FILE__, __LINE__);
			comparison_results.put_member("== comparison", false);
		}
		else {
			comparison_results.put_member("== comparison", true);
		}

		json test_eq3 = test_eq2.clone();

		if (test_eq2.compare(test_eq3) != 0)
		{
			system_monitoring_interface::global_mon->log_warning("cloned object comparison failed failed", __FILE__, __LINE__);
			comparison_results.put_member("== clone comparison", false);
		}
		else {
			comparison_results.put_member("== clone comparison", true);
		}

		corona::json test_lt1 = jp.parse_object(R"({ "name":"zak", "age":34 })");
		corona::json test_lt2 = jp.parse_object(R"({ "name":"zak", "age":37 })");
		test_lt1.set_compare_order({ "name", "age" });

		if (test_lt1.compare(test_lt2) >= 0)
		{
			system_monitoring_interface::global_mon->log_warning("test_lt1 < test_lt2 failed", __FILE__, __LINE__);
			comparison_results.put_member("< weak_order, multiple keys", false);
		}
		else 
		{
			comparison_results.put_member("< weak_order, multiple keys", true);
		}

		test_lt2.set_compare_order({ "name", "age" });

		if (test_lt2.compare(test_lt1) < 0)
		{
			system_monitoring_interface::global_mon->log_warning("test_lt2 < test_lt1 failed", __FILE__, __LINE__);
			comparison_results.put_member(">= weak_order, multiple keys", false);
		}
		else {
			comparison_results.put_member("< weak_order, multiple keys", true);
		}

		comparison_results.prove_member("is_true");

		proof_assertions.put_member("comparison", comparison_results);

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

		if (test_array.size() != 12) {
			system_monitoring_interface::global_mon->log_warning("array parse size incorrect", __FILE__, __LINE__);
			array_results.put_member("array_size", false);
		} else 
			array_results.put_member("array_size", true);

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
		array_results.put_member("array_element", barray_test);

		if (xidx != 12) {
			system_monitoring_interface::global_mon->log_warning("array enumeration", __FILE__, __LINE__);
			array_results.put_member("array_enumeration", false);
		}
		else 
		{
			array_results.put_member("array_enumeration", true);
		}
		array_results.prove_member("is_true");

		if (not test_array.any([](json& _item) {
			return (double)_item["age"] > 35;
			}))
		{
			array_results.put_member("any", false);
			system_monitoring_interface::global_mon->log_warning(".any failed", __FILE__, __LINE__);
		}
		else {
			array_results.put_member("any", true);
		}

		if (not test_array.all([](json& _item) {
			return (double)_item["age"] > 17;
			}))
		{
			system_monitoring_interface::global_mon->log_warning(".all failed", __FILE__, __LINE__);
			array_results.put_member("all", false);
		}
		else {
			array_results.put_member("all", true);
		}

		array_results.prove_member("is_true");
		proof_assertions.put_member("array", array_results);

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

		comparison_results.put_member("weak_ordered_hash", weak_ordered_hashing_test);
		comparison_results.prove_member("is_true");

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

		if (test_array_group["teen"].array() and
			test_array_group["adult"].array() and
			test_array_group["middle"].array() and
			test_array_group["teen"].size() == 2 and
			test_array_group["adult"].size() == 5 and
			test_array_group["middle"].size() == 5 
			)
		{
			array_results.put_member("group", true);
		}
		else 
		{
			array_results.put_member("group", false);
			system_monitoring_interface::global_mon->log_warning("array group failed", __FILE__, __LINE__);
		}

		proof_assertions.prove_member("is_true");
		_proof.put_member("object", proof_assertions);

		system_monitoring_interface::global_mon->log_function_stop("test json", "stop", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_file(json& _proof, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("file proof", "start",  st, __FILE__, __LINE__);


		json_parser jp;
		json proof_assertions = jp.create_object();
		proof_assertions.put_member("test_name", "file");

		file dtest = _app->create_file(FOLDERID_Documents, "corona_data_block_test.ctb");

		char buffer_write[2048], buffer_read[2048];

		strcpy_s(buffer_write, R"({ "name": "test" })");
		int l = strlen(buffer_write) + 1;

		int64_t location = dtest.add(1000);
		int64_t sz = dtest.size();

		if (sz < 1000 or location < 0) {
			proof_assertions.put_member("append", false);
			system_monitoring_interface::global_mon->log_warning("append failed");
		}
		else {
			proof_assertions.put_member("append", true);
		}

		file_command_result tsk = dtest.write(location, (void *)buffer_write, l);

		if (tsk.success == false or l > tsk.bytes_transferred) {
			proof_assertions.put_member("write", false);
			system_monitoring_interface::global_mon->log_warning("write failed");
		}
		else 
		{
			proof_assertions.put_member("write", true);
		}

		file_command_result tsk2 = dtest.read(location, (void*)buffer_read, l);

		if (tsk2.success == false or l > tsk.bytes_transferred) {
			proof_assertions.put_member("read", false);
			system_monitoring_interface::global_mon->log_warning("read failed");
		}
		else
		{
			proof_assertions.put_member("read", true);
		}
		
		if (strcmp(buffer_write, buffer_read) != 0)
		{
			proof_assertions.put_member("roundtrip", false);
			system_monitoring_interface::global_mon->log_warning("roundtrip failed");
		}
		else
		{
			proof_assertions.put_member("roundtrip", true);
		}
		proof_assertions.prove_member("is_true");
		_proof.put_member("file", proof_assertions);

		system_monitoring_interface::global_mon->log_function_stop("file proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

	}

	void test_data_block(json& _proof, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("block proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_data_block_test.ctb");

		json_parser jp;
		json proof_assertion = jp.create_object();
		proof_assertion.put_member("test_name", "data_block");

		json dependencies = jp.parse_object(R"( 
{ 
	"assignment" : [ "object.parse" ],
    "read" : [ "file.read" ],
	"write" : [ "file.write" ],
	"append" : [ "file.append" ],
	"grow" : [ "object.parse", "file.write", "file.apend" ]
}
)");
		proof_assertion.put_member("dependencies", dependencies);

		json jx = jp.parse_object(R"({ "name":"bill", "age":42 })");

		data_block dfirst, dfirstread, dbounds;

		dfirst = jx;

		std::string dfs = dfirst.get_string();
		std::string jxs = jx.to_json_typed();
		if (dfs != jxs)
		{
			proof_assertion.put_member("assignment", false);
			system_monitoring_interface::global_mon->log_warning("assignment of data to data block failed.", __FILE__, __LINE__);
		}
		else
		{
			proof_assertion.put_member("assignment", true);
		}

		int64_t dfirstlocation = dfirst.append(dtest.get(), [dtest](int64_t _size)->int64_t {
			int64_t temp = dtest->add(_size);
			return temp;
			});

		dbounds.bytes = "test border string";
		std::string borders = dbounds.get_string();

		int64_t dboundslocation = dbounds.append(dtest.get(), [dtest](int64_t _size)->int64_t {
			int64_t temp = dtest->add(_size);
			return temp;
			});

		int64_t dfirstlocationread = dfirstread.read(dtest.get(), dfirstlocation);

		int64_t dboundslocationread = dbounds.read(dtest.get(), dboundslocation);

		std::string sc, sb;
		sc = dfirstread.get_string();
		sb = dfirst.get_string();
		if (sc != sb)
		{
			proof_assertion.put_member("append", false);
			proof_assertion.put_member("read", false);
			system_monitoring_interface::global_mon->log_warning("append and read failed", __FILE__, __LINE__);
		}
		else
		{
			proof_assertion.put_member("append", true);
			proof_assertion.put_member("read", true);
		}

		std::string borders_after;
		borders_after = dbounds.get_string();
		if (borders != borders_after)
		{
			proof_assertion.put_member("write", false);
			system_monitoring_interface::global_mon->log_warning("probably overrwrite", __FILE__, __LINE__);
		}
		else
		{
			proof_assertion.put_member("write", true);
		}

		// grow the middle block and see if it all works out.

		json jx2 = jp.parse_object(R"({ "name":"bill", 
			"age":42, 
			"weight":185, 
			"agency":"super alien and paranormal investigative unit", 
			"declaration" : "When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume among the powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation. We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.--That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, --That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness. Prudence, indeed, will dictate that Governments long established should not be changed for light and transient causes; and accordingly all experience hath shewn, that mankind are more disposed to suffer, while evils are sufferable, than to right themselves by abolishing the forms to which they are accustomed. But when a long train of abuses and usurpations, pursuing invariably the same Object evinces a design to reduce them under absolute Despotism, it is their right, it is their duty, to throw off such Government, and to provide new Guards for their future security.--Such has been the patient sufferance of these Colonies; and such is now the necessity which constrains them to alter their former Systems of Government. The history of the present King of Great Britain is a history of repeated injuries and usurpations, all having in direct object the establishment of an absolute Tyranny over these States. To prove this, let Facts be submitted to a candid world."
			 })");
		sc = jx2.to_json_typed();
			
		dfirst = jx2;
		std::string sbefore = dfirst.get_string();
		dfirst.write(dtest.get(), [dtest](int64_t _location) -> void {
				;
			},
			[dtest](int64_t _size_bytes) -> int64_t {
				return dtest->add(_size_bytes);
			}
		);
		
		json growth = jp.create_object();

		sb = dfirst.get_string();
		if (sc != sb)
		{
			growth.put_member("self", false);
			system_monitoring_interface::global_mon->log_warning("expand block failed", __FILE__, __LINE__);
		}
		else
		{
			growth.put_member("self", true);
		}

		dbounds.read(dtest.get(), dboundslocation);

		borders_after = dbounds.get_string();
		if (borders_after != borders)
		{
			growth.put_member("neighbor", false);
			system_monitoring_interface::global_mon->log_warning("expand block stomped neighbor", __FILE__, __LINE__);
		}
		else
		{
			growth.put_member("neighbor", true);
		}

		growth.prove_member("is_true");
		proof_assertion.put_member("grow", growth);
		proof_assertion.prove_member("is_true");
		_proof.put_member("block", proof_assertion);

		system_monitoring_interface::global_mon->log_function_stop("block proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_json_node(json& _proof, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("node proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_json_node_test.ctb");

		json_parser jp;
		json proof_assertion = jp.create_object();
		proof_assertion.put_member("test_name", "json_node");

		json dependencies = jp.parse_object(R"( 
{ 
    "read" : [ "block.read" ],
	"write" : [ "block.write" ],
	"append" : [ "block.append" ]
}
)");
		proof_assertion.put_member("dependencies", dependencies);

		json_node jnfirst, jnsecond;

		jnfirst.data = jp.create_array();
		for (double i = 0; i < 42; i++)
		{
			jnfirst.data.append_element( i );
		}

		bool append_worked = true;

		int64_t locfirst = jnfirst.append(dtest.get(), [dtest](int64_t size)->int64_t {
			return dtest->add(size);
			});

		if (locfirst < 0)
		{
			system_monitoring_interface::global_mon->log_warning("node append failed.", __FILE__, __LINE__);
			append_worked = false;
		}

		jnsecond.data = jp.parse_object(R"({ "Star" : "Aldeberran" })");

		int64_t locsecond = jnsecond.append(dtest.get(), [dtest](int64_t size)->int64_t {
			return dtest->add(size);
			});

		if (locsecond < 0)
		{
			system_monitoring_interface::global_mon->log_warning("node append failed.", __FILE__, __LINE__);
			append_worked = false;
		}

		proof_assertion.put_member("append", append_worked);

		jnfirst.read(dtest.get(), locfirst);

		bool read_success = true;

		if (jnfirst.data.size() != 42)
		{
			read_success = false;
			system_monitoring_interface::global_mon->log_warning("read array size incorrect.", __FILE__, __LINE__);
		}

		for (int i = 0; i < 42; i++) {
			int x = jnfirst.data.get_element(i);
			if (x != i) {
				read_success = false;
			}
			jnfirst.data.put_element(i, (double)(i * 10));
		}

		bool write_success = true;

		relative_ptr_type write_result = jnfirst.write(dtest.get(), [](int64_t _location)
			{

			},
			[dtest](int64_t _size) {
				return dtest->add(_size);
			}
		);

		if (write_result < 0) {
			system_monitoring_interface::global_mon->log_warning("write node failed.", __FILE__, __LINE__);
			write_success = false;
		}
		proof_assertion.put_member("write", write_success);
		proof_assertion.put_member("read", read_success);

		bool grow_success = true;
		jnfirst.data = jp.create_array();

		for (int i = 0; i < 10000; i++)
		{
			double t = i;
			jnfirst.data.push_back(t);
		}

		relative_ptr_type loc = jnfirst.write(dtest.get(),
			[](int64_t _location)
			{

			},
			[dtest](int64_t _size) {
				return dtest->add(_size);
			}
		);

		if (loc < 0) 
		{
			system_monitoring_interface::global_mon->log_warning("node write failed.", __FILE__, __LINE__);
		}

		auto rs = jnfirst.read(dtest.get(), locfirst);

		if (rs < 0)
		{
			grow_success = false;
			system_monitoring_interface::global_mon->log_warning("grow failed.", __FILE__, __LINE__);
		}
		else
		{
		}

		for (int i = 0; i < 10000; i++) {
			int x = jnfirst.data.get_element(i);
			if (x != i) {
				grow_success = false;
			}
			jnfirst.data.put_element(i, (double)(i * 10));
		}
		proof_assertion.put_member("grow", grow_success);
		proof_assertion.prove_member("is_true");

		_proof.put_member("node", proof_assertion);

		system_monitoring_interface::global_mon->log_function_stop("node proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_json_table(json& _proof, std::shared_ptr<application> _app)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("table proof", "start", st, __FILE__, __LINE__);

		using return_type = bool;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");

		json_parser jp;
		json proof_assertion = jp.create_object();
		proof_assertion.put_member("test_name", "json_table");

		json dependencies = jp.parse_object(R"( 
{ 
    "put" : [ "node.write", "node.append" ],
	"get" : [ "node.read" ],
	"create" : [ "node.read", "node.write" ],
	"erase" : [ "node.read", "node.write" ],
	"select" : [ "node.read" ],
	"for_each" : [ "node.read", "node.write" ],
	"group" : [ "object.group" ],
	"any" : [ "object.any" ],
	"all" : [ "object.all" ]
}
)");

		bool	put_success = true,
			get_success = true,
			create_success = true,
			erase_success = true,
			select_array_success = true,
			for_each_success = true,
			group_success = true,
			any_success = true,
			all_success = true;

		proof_assertion.put_member("dependencies", dependencies);

		json test_write = jp.create_object();
		test_write.put_member_i64("ObjectId", 5);
		test_write.put_member("Name", "Joe");
		json test_key = test_write.extract({ "ObjectId" });

		json_table test_table(f, {"ObjectId"});

		relative_ptr_type table_loc = test_table.create();

		if (table_loc < 0) {
			system_monitoring_interface::global_mon->log_warning("create table failed.", __FILE__, __LINE__);
			create_success = false;
		}

		relative_ptr_type rpt =  test_table.put(test_write);
		if (rpt < 0) {
			system_monitoring_interface::global_mon->log_warning("put failed.", __FILE__, __LINE__);
			put_success = false;
		}

		json test_read =  test_table.get(test_key);
		if (test_read.empty()) {
			system_monitoring_interface::global_mon->log_warning("get failed.", __FILE__, __LINE__);
			get_success = false;
		}
		else {

			test_read.set_compare_order({ "ObjectId" });

			if (test_read.compare(test_write))
			{
				system_monitoring_interface::global_mon->log_warning("get did not match put.", __FILE__, __LINE__);
				get_success = false;
			}
		}

		json db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		if (not db_contents.array()) {
			select_array_success = false;
			system_monitoring_interface::global_mon->log_warning("select did not return nodes.", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jack");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read = test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value.", __FILE__, __LINE__);
		}

		test_read = jp.create_object();
		test_read.put_member_i64("ObjectId", 5);
		test_read.set_natural_order();

		json joe = test_table.get(test_read);
		if (joe.empty()) {
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("joe disappeared.", __FILE__, __LINE__);
		}

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1 = db_contents.size();

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jill");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value.", __FILE__, __LINE__);
		}

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		int db_size1b = db_contents.size();

		try
		{
			test_key.put_member_i64("ObjectId", 6);
			json t5 =  test_table.get(test_key);
		}
		catch (std::exception exc)
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong null access", __FILE__, __LINE__);
		}

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
		});

		if (db_contents.size() != 2)
		{
			select_array_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong number of result elements", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 2);
		test_write.put_member("Name", "Sydney");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		test_read = jp.create_object();
		test_read.put_member_i64("ObjectId", 5);
		test_read.set_natural_order();

		joe = test_table.get(test_read);
		if (joe.empty()) {
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("sydney killed joe!", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Orwell");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Canada");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Roger");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Maraca");
		test_key = test_write.extract({ "ObjectId" });
		test_table.put(test_write);

		test_read =  test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			get_success = false;
			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		db_contents =  test_table.select([](int _index, json& item) {
			return item;
			});

		if (test_table.size() != 4)
		{
			select_array_success = false;

			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}

		if (db_contents.size() != 4)
		{
			select_array_success = false;

			system_monitoring_interface::global_mon->log_warning("wrong inserted value", __FILE__, __LINE__);
		}
		
		int64_t tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		json counts = jp.create_object();
		json *pcounts = &counts;

		auto fr = test_table.for_each([tests,pcounts](int _index, json& _item) -> bool {
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

		if ((not counts.object()) or (counts.size() != 4))
		{
			for_each_success = false;
		}

		std::string count_string = counts.to_json();

		db_contents =  test_table.select([tests](int _index, json& _item) -> json {
			int64_t temp = _item["ObjectId"];
			return (temp > 0i64) ? _item : json();
		}
		);

		if (not db_contents.array()) {
			select_array_success = false;
			system_monitoring_interface::global_mon->log_warning("select failed", __FILE__, __LINE__);
		}

		bool any_fails = db_contents.any([](json& _item)->bool {
			int64_t temp = _item["ObjectId"];
			return temp <= 0i64;
			});

		if (any_fails) {
			system_monitoring_interface::global_mon->log_warning("query failed", __FILE__, __LINE__);
		}

		auto summary = db_contents.array_to_object([](json& _item) {
			return (std::string)_item["ObjectId"];
			},
			[](json& _target) {
				return _target;
			});

		if (not (summary.has_member("1") and summary.has_member("2") and summary.has_member("5") and summary.has_member("7")))
		{
			for_each_success = false;
		}

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

		if (any_fails) {
			system_monitoring_interface::global_mon->log_warning("any failed.", __FILE__, __LINE__);
			any_success = false;
		}

		test_key.put_member_i64("ObjectId", 3);
		bool rdel3 =  test_table.erase(test_key);

		if (rdel3) {
			system_monitoring_interface::global_mon->log_warning("erase failed.", __FILE__, __LINE__);
			erase_success = false;
		}

		test_key.put_member_i64("ObjectId", 1);
		bool rdel1 =  test_table.erase(test_key);

		if (not rdel1) {
			erase_success = false;
			system_monitoring_interface::global_mon->log_warning("erase failed.", __FILE__, __LINE__);
			std::cout << "delete existing failed" << std::endl;
		}

		test_key.put_member_i64("ObjectId", 7);
		relative_ptr_type rdel7 =  test_table.erase(test_key);

		if (not rdel7) {
			system_monitoring_interface::global_mon->log_warning("erase failed.", __FILE__, __LINE__);
			std::cout << "delete existing failed" << std::endl;
		}

		json testi =  test_table.select([tests](int _index, json& item) -> json {
			int64_t object_id = item["ObjectId"];
			return object_id == 7 ? item : json();
			});

		if (testi.size() > 0) {
			erase_success = false;
			system_monitoring_interface::global_mon->log_warning("erase failed.", __FILE__, __LINE__);
		}
		 
		db_contents =  test_table.select([tests](int _index, json& item) {
			return item;
			}
		);

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item["ObjectId"];
			return  object_id != 2 and object_id != 5;
			});

		if (any_iteration_fails) {
			system_monitoring_interface::global_mon->log_warning("any fails.", __FILE__, __LINE__);
			any_success = false;
		}

		proof_assertion.put_member("put", put_success);
		proof_assertion.put_member("get", get_success);
		proof_assertion.put_member("create", create_success);
		proof_assertion.put_member("select_array", select_array_success);
		proof_assertion.put_member("for_each", for_each_success);
		proof_assertion.put_member("group", group_success);
		proof_assertion.put_member("any", any_success);
		proof_assertion.put_member("all", all_success);
		proof_assertion.put_member("erase", erase_success);

		bool general_success = put_success
			and get_success
			and create_success
			and select_array_success
			and for_each_success
			and group_success
			and any_success
			and all_success
			and erase_success;

		proof_assertion.put_member("is_true", general_success);

		_proof.put_member("table", proof_assertion);

		system_monitoring_interface::global_mon->log_function_stop("table proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}
}

#endif

