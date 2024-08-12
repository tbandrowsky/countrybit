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

	using db_object_id_type = int64_t;

	// data blocks
	class data_block;
	std::ostream& operator <<(std::ostream& output, data_block& src);

	// poco nodes
	template <typename T> class poco_node;
	template <typename T> std::ostream& operator <<(std::ostream& output, poco_node<T>& src);

	// json nodes
	class json_node;
	std::ostream& operator <<(std::ostream& output, json_node& src);

	// nesting of transactions

	class data_block
	{
	public:

		block_header_struct				header;
		buffer							bytes;
		relative_ptr_type				current_location;

		data_block()
		{
			header = {};
			header.block_type = block_id::general_id();
			current_location = -1;
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

		file_batch read(file* _file, relative_ptr_type location)
		{
			current_location = location;

			debug_json_table && std::cout << "datablock:read begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			file_task_result header_result = co_await _file->read(location, &header, sizeof(header));

			if (header_result.success) 
			{
				debug_json_table && std::cout << "datablock:read data begin:" << *this << " " << GetCurrentThreadId() << std::endl;

				bytes = buffer(header.data_length);
				file_task_result  data_result = co_await _file->read(header.data_location, bytes.get_ptr(), header.data_length);

				if (data_result.success) 
				{
					debug_json_table && std::cout << "datablock:read data worked:" << *this << " " << GetCurrentThreadId() << std::endl;
					co_return data_result.bytes_transferred; // want to make this 0 or -1 if error
				}
			}

			debug_json_table && std::cout << "header read failed:" << *this << " " << GetCurrentThreadId() << std::endl;

			co_return -1i64;
		}

		file_batch write(file* _file)
		{
			debug_json_table&& std::cout << "datablock:write begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (current_location < 0) 
			{
				throw std::invalid_argument("use append for new blocks");
			}

			int size = bytes.get_size();

			if (size > header.data_length)
			{
				header.data_length = 1;
				while (header.data_length < size)
					header.data_length *= 2;
				header.next_free_block = header.data_location;
				header.data_location = _file->add(header.data_length);
			}

			debug_json_table&& std::cout << "datablock:write header start:" << *this << " " << GetCurrentThreadId() << std::endl;

			file_task_result data_result = co_await _file->write(header.data_location, bytes.get_ptr(), size);

			debug_json_table && std::cout << "datablock:write header end:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (data_result.success)
			{
				file_task_result header_result = co_await _file->write(current_location, &header, sizeof(header));
				co_return header_result.bytes_transferred;
			}

			co_return -1i64;
		}

		file_batch append(file* _file)
		{
			int size = bytes.get_size();

			debug_json_table&& std::cout << "append begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			current_location = _file->add(sizeof(header) + size);

			header.block_type = block_id::general_id();
			header.next_free_block = 0;
			header.data_location = current_location + sizeof(header);
			header.data_length = size;

			file_task_result header_result = co_await _file->write(current_location, &header, sizeof(header));
			debug_json_table&& std::cout << "append write header:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (header_result.success)
			{
				debug_json_table&& std::cout << "append write data:" << *this << " " << GetCurrentThreadId() << std::endl;
				file_task_result data_result = co_await _file->write(header.data_location, bytes.get_ptr(), size);

				debug_json_table&& std::cout << "append write data finished:" << *this << " " << GetCurrentThreadId() << std::endl;
				co_return current_location;
			}

			co_return -1i64;
		}

	};

	std::ostream& operator <<(std::ostream& output, data_block& src)
	{
		output << "datablock:" << src.current_location << ", " << src.bytes.get_size() << " bytes";
		return output;
	}

	class json_node
	{
	public:

		std::vector<relative_ptr_type>	forward;
		json							data;
		data_block						storage;

		json_node()
		{
		}

		bool is_empty()
		{
			return forward.size()==0;
		}

		void clear()
		{
			json_parser jp;
			data = jp.create_object();
			forward.clear();
		}

		json get_json()
		{
			json_parser jp;
			json payload = jp.create_object();
			json fow = jp.create_array();
			payload.put_member("Data", data);
			for (auto pt : forward) {
				fow.append_element( pt);
			}
			payload.put_member("Forward", fow);
			return payload;
		}

		json_node& put_json(json& _src)
		{
			data = _src["Data"];
			forward.clear();
			auto forwardjson = _src["Forward"];
			forwardjson.for_each_element([this](json& _item) {
				relative_ptr_type ptr = _item;
				forward.push_back(ptr);
				});
			return *this;
		}

		json_node& operator = (json& _src)
		{
			return put_json(_src);
		}

		file_batch read(file* _file, relative_ptr_type location)
		{
			debug_json_table&& std::cout << "json_node read:" << location << std::endl;
			relative_ptr_type status = co_await storage.read(_file, location);

			if (status > -1)
			{
				char* c = storage.bytes.get_ptr();
				json_parser jp;
				json payload = jp.parse_object(c);
				if (payload.is_member("ClassName", "SysParseErrors")) {
					std::string temp = "Could not parse json node on retrieval:";
					temp += c;
					system_monitoring_interface::global_mon->log_warning(temp, __FILE__, __LINE__);
				}
				put_json(payload);
			}
			co_return status;
		}

		file_batch write(file* _file)
		{
			debug_json_table&& std::cout << "json_node write:" << storage.current_location << std::endl;
			auto json_payload = get_json();

			storage = json_payload;

			relative_ptr_type result = co_await storage.write(_file);
			co_return result;
		}

		file_batch append(file* _file)
		{
			debug_json_table&& std::cout << "json_node append:" << std::endl;
			auto json_payload = get_json();

			storage = json_payload;

			relative_ptr_type result = co_await storage.append(_file);

			storage.current_location = result;

			debug_json_table&& std::cout << "json_node append location:" << result << std::endl;

			co_return result;
		}
	};

	std::ostream& operator <<(std::ostream& output, json_node& src)
	{
		output << "json_node:" << src.data.to_json();
		return output;
	}

	template <typename poco_type> class poco_node
	{
	public:
		db_object_id_type		object_id;
		poco_type				data;
		data_block				storage;

		void clear()
		{
			object_id = 0;
			data = {};
		}

		poco_node& operator = (const poco_type& _src)
		{
			data = _src;
			return *this;
		}

		file_batch read(file* _file, relative_ptr_type location)
		{
			storage.init(sizeof(poco_type));

			debug_json_table&& std::cout << "read poco:" << location << std::endl;

			relative_ptr_type status = co_await storage.read(_file, location);

			if (status > -1)
			{
				poco_type* c = (poco_type *)storage.bytes.get_ptr();
				data = *c;
			}

			co_return status;
		}

		file_batch write(file* _file)
		{
			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			debug_json_table&& std::cout << "write poco:" << storage.current_location << std::endl;

			relative_ptr_type status = co_await storage.write(_file);

			co_return status;
		}

		file_batch append(file* _file)
		{
			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			debug_json_table&& std::cout << "append poco:" << storage.current_location << std::endl;

			relative_ptr_type status = co_await storage.append(_file);

			co_return status;
		}
	};

	template<typename T> std::ostream& operator <<(std::ostream& output, poco_node<T>& src)
	{
		output << "json_node:" << src.object_id << " bytes";
		return output;
	}

	class json_table
	{
	private:

		relative_ptr_type header_location;
		poco_node<index_header_struct> index_header;
		std::shared_ptr<file> database_file;

		using KEY = json;
		using VALUE = json_node;
		using UPDATE_VALUE = json;

		const int SORT_ORDER = 1;
		
		table_transaction<relative_ptr_type> create_header()
		{
			json_parser jp;
			header_location = co_await index_header.append(database_file.get());
			json_node header = co_await create_node(JsonTableMaxLevel, header_key);
			index_header.data.header_node_location = header.storage.current_location;
			index_header.data.count = 0;
			index_header.data.level = JsonTableMaxLevel;
			co_await index_header.write(database_file.get());
			co_await header.write(database_file.get());
			co_return header_location;
		}

		table_transaction<json_node> get_header()
		{

			json_node in;

			co_await index_header.read(database_file.get(), header_location);

			int64_t result = co_await in.read(database_file.get(), index_header.data.header_node_location);

			if (result < 0) 
			{
				throw std::logic_error("Couldn't read table header.");
			}

			co_return in;
		}

		file_transaction<json_node> create_node(int _max_level, json _data)
		{
			json_node new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.forward.push_back(rit);
			}

			new_node.data = _data;

			co_await new_node.append(database_file.get());
			co_return new_node;
		}

		file_transaction<json_node> get_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;

			co_await node.read(_file, _node_location);
			co_return node;
		}

		std::vector<std::string> key_fields;
		json header_key;

	public:

		json_table(std::shared_ptr<file> _database_file, std::vector<std::string> _key_fields) 
			: database_file(_database_file), 
				key_fields(_key_fields),
			header_location(0)
		{

		}

		json_table(const json_table& _src) 
			: key_fields(_src.key_fields)
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
			header_key = _src.header_key;
			header_location = 0;
		}

		json_table operator = (const json_table& _src)
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
			key_fields = _src.key_fields;
			header_key = _src.header_key;
			header_location = _src.header_location;
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

		table_transaction<relative_ptr_type> create()
		{
			relative_ptr_type location = co_await create_header();
			co_return location;
		}

		table_transaction<relative_ptr_type> open(relative_ptr_type location)
		{
			header_location = location;
			co_await index_header.read(database_file.get(), header_location);
			co_return header_location;
		}

		table_transaction<bool> contains(const KEY key)
		{
			relative_ptr_type result = co_await find_node(key);
			co_return  result != null_row;
		}

		table_transaction<json> get(std::string _key)
		{
			json_parser jp;
			json key = jp.parse_object(_key);

			key.set_natural_order();

			if (key.is_member("ClassName", "SysParseError")) {
				std::cout << key.to_json() << std::endl;
				co_return key;
			}

			json result;
			relative_ptr_type n = co_await find_node(key);
			if (n != null_row) {
				json_node r = co_await get_node(database_file.get(), n);
				result = r.data;
			}
			co_return result;
		}

		table_transaction<json> get(const KEY key)
		{
			json result;
			relative_ptr_type n = co_await find_node(key);
			if (n != null_row) {
				json_node r = co_await get_node(database_file.get(), n);
				result = r.data;
			}
			co_return result;
		}

		table_transaction<json> get(const KEY key, std::initializer_list<std::string> include_fields)
		{
			json result;
			relative_ptr_type n = co_await find_node(key);
			if (n != null_row) {
				json_node r = co_await get_node(database_file.get(), n);
				if (!r.data.empty()) {
					result = r.data.extract(include_fields);
				}
			}
			co_return result;
		}

		table_transaction<relative_ptr_type>
		put(json value)
		{
			auto key = get_key(value);
			relative_ptr_type modified_node = co_await this->update_node(key, 
				[value](UPDATE_VALUE& dest) { 
					dest.assign_update(value); 
				}
			);
			co_return modified_node;
		}

		table_transaction<relative_ptr_type>
		put(std::string _json)
		{
			json_parser jp;
			json jx = jp.parse_object(_json);
			if (jx.empty() || jx.is_member("ClassName", "SysParseError")) {
				std::cout << jx.to_json() << std::endl;
				co_return null_row;
			}
			auto key = get_key(jx);
			relative_ptr_type modified_node = co_await this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_update(jx); });
			co_return modified_node;
		}

		table_transaction<relative_ptr_type>
			replace(json value)
		{
			auto key = get_key(value);
			relative_ptr_type modified_node = co_await this->update_node(key, [value](UPDATE_VALUE& dest) { dest.assign_replace(value); });
			co_return modified_node;
		}

		table_transaction<relative_ptr_type>
			replace(std::string _json)
		{
			json_parser jp;
			json jx = jp.parse_object(_json);
			auto key = get_key(jx);
			relative_ptr_type modified_node = co_await this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_replace(jx); });
			co_return modified_node;
		}

		table_transaction<bool>
		erase(const KEY& key)
		{
			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_node qnd, pnd;

			relative_ptr_type q = co_await find_node(update, key);

			if (q != null_row)
			{
				k = 0;
				p = update[k];
				qnd = co_await get_node(database_file.get(), q);
				pnd = co_await get_node(database_file.get(), p);
				int m = index_header.data.level;
				while (k <= m && pnd.forward[k] == q)
				{
					pnd.forward[k] = qnd.forward[k];
					co_await pnd.write(database_file.get());
					k++;
					if (k <= m) {
						p = update[k];
						pnd = co_await get_node(database_file.get(), p);
					}
				}

				::InterlockedDecrement64(&index_header.data.count);

				auto header_task = get_header();
				json_node header = header_task.wait();

				while (header.forward[m] == null_row && m > 0) {
					m--;
				}
				index_header.data.level = m;
				co_await header.write(database_file.get());
				co_await index_header.write(database_file.get());
				co_return true;
			}
			else
			{
				co_return false;
			}
		}

		table_group_transaction<int64_t> 
			co_for_each(json _key_fragment, std::function<table_transaction<relative_ptr_type>(int _index, json_node& _item)> _process_clause)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;

			while (location != null_row) 
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
				int comparison = _key_fragment.compare(node.data);
				if (comparison == 0) 
				{
					relative_ptr_type process_result = co_await _process_clause(index, node);
					if (process_result > 0)
					{
						index++;
					}
				}
				location = node.forward[0];
			}

			co_return index;
		}

		table_group_transaction<int64_t>
			for_each(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
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

			co_return index;
		}

		table_transaction<json>
			get_first(json _key_fragment)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;
			json result_data;

			while (location != null_row)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
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

			co_return result_data;
		}

		table_group_transaction<bool> any(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;
			bool is_any = false;

			while (location != null_row && !is_any)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
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

			co_return is_any;
		}

		table_group_transaction<bool> all(json _key_fragment, std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;
			bool is_all = true;

			while (location != null_row && is_all)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
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

			co_return is_all;
		}

		table_group_transaction<int64_t>
			co_for_each(std::function<table_method_transaction<relative_ptr_type>(int _index, json& _item)> _process_clause)
		{
			auto header = co_await get_header();

			relative_ptr_type location = header.forward[0];
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
				relative_ptr_type process_result = co_await _process_clause(index, node.data);
				if (process_result > 0)
				{
					index++;
				}
				location = node.forward[0];
			}

			co_return index;
		}

		table_group_transaction<int64_t>
			for_each(std::function<relative_ptr_type(int _index, json& _item)> _process_clause)
		{
			auto header = co_await get_header();

			relative_ptr_type location = header.forward[0];
			int64_t index = 0;

			while (location != null_row)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
				relative_ptr_type process_result = _process_clause(index, node.data);
				if (process_result > 0)
				{
					index++;
				}
				location = node.forward[0];
			}

			co_return index;
		}

		table_transaction<json> get_first(json _key_fragment, std::function<bool(json& _src)> _fn)
		{
			relative_ptr_type location = co_await find_first_node_gte(_key_fragment);
			int64_t index = 0;
			json result_data;

			while (location != null_row)
			{
				json_node node;
				node = co_await get_node(database_file.get(), location);
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

			co_return result_data;
		}

		table_array_transaction<json>
		select_array(std::function<json(int _index, json& _item)> _project)
		{
			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			int64_t count = co_await co_for_each([pja, _project](int _index, json& _data) ->table_method_transaction<relative_ptr_type>
				{
					relative_ptr_type count = 0;
					json new_item = _project(_index, _data);
					if (!new_item.empty() && !new_item.is_member("Skip", "this")) {
						pja->append_element(new_item);
						count = 1;
					}
					co_return count;
				});

			co_return ja;
		}

		table_array_transaction<json>
		select_array(json _key_fragment, 
			std::function<json(int _index, json& _item)> _project,
			json _update = "{ }"_jobject
		)
		{
			json_parser jp;
			json ja = jp.create_array();
			json* pja = &ja;

			int64_t count = co_await co_for_each(_key_fragment, [this, pja, _project, &_update](int _index, json_node& _data) -> table_transaction<relative_ptr_type> 
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
					co_return count;
				});

			co_return ja;
		}

		table_array_transaction<json>
		select_object(json& _destination,
				json _key_fragment,
				std::function<json(int _index, json& _item)> _project,
				std::function<json(int _index, json& _item)> _get_child_key,
				std::initializer_list<std::string> _group_by
			)
		{
			json* pdestination = &_destination;

			int64_t count = co_await co_for_each(_key_fragment, [this, _group_by, pdestination, _project, _get_child_key](int _index, json_node& _jdata) -> table_transaction<int64_t>
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
								auto children_task = select_object(*pdestination, child_key, _project, _get_child_key, _group_by);
								children_task.wait();
							}
						}
					}
					co_return true;
				});

			co_return _destination;
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

		relative_ptr_type compare_node(json_node _nd, KEY _id_key)
		{
			KEY ndkey = get_key(_nd.data);
			int k = -_id_key.compare(ndkey); // the - is here because the comparison is actually backwards. 
			if (k < 0)
				return -SORT_ORDER;
			else if (k > 0)
				return SORT_ORDER;
			else
				return 0;
		}

		table_method_transaction<relative_ptr_type>
		find_node(relative_ptr_type* update, KEY _key)
		{
			relative_ptr_type found = null_row, p, q;
			auto hdr_task = get_header();
			json_node hdr = hdr_task.wait();

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = index_header.data.header_node_location;
				if (p <= 0) {
					throw std::exception("table header node location not set");
				}
				json_node jn = hdr;
				q = jn.forward[k];
				int comp = 1;
				json_node qnd;
				if (q != null_row) {
					qnd = co_await get_node(database_file.get(), q);
					comp = compare_node(qnd, _key);
				}
				while (comp < 0)
				{
					p = q;
					jn = qnd;
					q = jn.forward[k];
					comp = 1;
					if (q != null_row) {
						qnd = co_await get_node(database_file.get(), q);
						comp = compare_node(qnd, _key);
					}
				}
				if (comp == 0)
					found = q;
				update[k] = p;
			}

			co_return found;
		}

		table_method_transaction<relative_ptr_type>
		find_first_gte(relative_ptr_type* update, KEY _key)
		{
			relative_ptr_type found = null_row, p, q, last_link;

			auto get_header_task = get_header();
			json_node header = get_header_task.wait();

			if (!_key.keys_compatible(key_fields)) {
				co_return header.forward[0];
			}

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = header.forward[k];
				json_node jn = co_await get_node(database_file.get(), p);
				q = jn.forward[k];
				last_link = q;
				json_node qnd;
				int comp = 1;
				if (q != null_row) {
					qnd = co_await get_node(database_file.get(), q);
					comp = compare_node(qnd, _key);
				}
				while (comp < 0)
				{
					p = q;
					last_link = q;
					json_node jn = qnd;
					q = jn.forward[k];
					comp = 1;
					if (q != null_row) {
						qnd = co_await get_node(database_file.get(), q);
						comp = compare_node(qnd, _key);
					}
				}
				if (comp == 0)
					found = q;
				else if (comp < 0)
					found = last_link;
				update[k] = p;
			}

			co_return found;
		}

		table_method_transaction<relative_ptr_type>
		update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			debug_json_table&& std::cout << "json_table update_node" << std::endl;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			auto get_header_task = get_header();
			json_node header = get_header_task.wait();

			relative_ptr_type location = header.forward[0];

			relative_ptr_type q = co_await find_node(update, _key);
			json_node qnd;

			if (q != null_row)
			{
				qnd = co_await get_node(database_file.get(), q);
				predicate(qnd.data);
				co_await qnd.write(database_file.get());
				co_return qnd.storage.current_location;
			}

			k = randomLevel();
			if (k > index_header.data.level)
			{
				::InterlockedIncrement(&index_header.data.level);
				k = index_header.data.level;
				update[k] = header.storage.current_location;
			}

			UPDATE_VALUE initial_value;
			try {
				predicate(initial_value);
			}
			catch (std::exception exc)
			{
				std::cout << __FILE__ << " " << __LINE__ << ":Initialization of new object failed when inserting node into table." << std::endl;
				co_return -1;
			}

			qnd = co_await create_node(k, initial_value);
			::InterlockedIncrement64(&index_header.data.count);

			do 
			{
				json_node pnd = co_await get_node(database_file.get(), update[k]);
				qnd.forward[k] = pnd.forward[k];
				pnd.forward[k] = qnd.storage.current_location;

				co_await qnd.write(database_file.get());
				co_await pnd.write(database_file.get());

			} while (--k >= 0);

			co_await index_header.write(database_file.get());

			co_return qnd.storage.current_location;
		}


		table_method_transaction<relative_ptr_type>
		find_node(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = co_await find_node(update, key);
			co_return value;
		}

		table_method_transaction<relative_ptr_type>
		find_first_node_gte(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type fn = co_await find_first_gte(update, key);
			co_return fn;
		}

		table_method_transaction<json_node> first_node()
		{
			json_node jn;
			auto header_task = get_header();
			auto header = header_task.wait();
			if (header.forward[0] != null_row) {
				jn = get_node(database_file.get(), header.forward[0]);
			}
			co_return jn;
		}

		table_method_transaction<json_node> next_node(json_node _node)
		{
			if (_node.is_empty())
				co_return _node;

			json_node nd = co_await get_node(database_file.get(), _node.forward[0]);
			co_return nd;
		}
	};

	std::ostream& operator <<(std::ostream& output, json_table & src)
	{
		std::cout << "[json_table]" << std::endl;
		return output;
	}

	user_transaction<bool> test_json_table(std::shared_ptr<application> _app);
	file_batch test_file(std::shared_ptr<application> _app);
	file_transaction<int64_t> test_data_block(std::shared_ptr<application> _app);
	file_transaction<int64_t> test_json_node(std::shared_ptr<application> _app);

	file_batch test_file(std::shared_ptr<application> _app)
	{

		std::cout << "\ntest_file: entry, thread:" << ::GetCurrentThreadId() << std::endl;

		file dtest = _app->create_file(FOLDERID_Documents, "corona_data_block_test.ctb");

		char buffer_write[2048], buffer_read[2048];

		strcpy_s(buffer_write, R"({ "name": "test" })");
		int l = strlen(buffer_write) + 1;

		dtest.add(1000);

		std::cout << "\ntest_file: co_await write, thread:" << ::GetCurrentThreadId() << std::endl;
		file_task_result tsk = co_await dtest.write(0, (void *)buffer_write, l);

		std::cout << "\ntest_file: co_await read, thread:" << ::GetCurrentThreadId() << std::endl;
		file_task_result tsk2 = co_await dtest.read(0, (void*)buffer_read, l);

		std::cout << "\ntest_file: co_await read, thread:" << ::GetCurrentThreadId() << std::endl;
		
		if (!strcmp(buffer_write, buffer_read))
		{
			std::cout << "\ntest_file: co_return success " << 42 << ", thread:" << ::GetCurrentThreadId() << std::endl;
			co_return 42;
		}

		std::cout << "\ntest_file: co_return fail, thread:" << ::GetCurrentThreadId() << std::endl;
		co_return 0;
	}

	file_transaction<int64_t> test_data_block(std::shared_ptr<application> _app)
	{
		std::shared_ptr<file>  dtest = _app->create_file_ptr(FOLDERID_Documents, "corona_data_block_test.ctb");

		std::cout << "test_data_block, thread:" << ::GetCurrentThreadId() << std::endl;

		json_parser jp;
		json jx = jp.parse_object(R"({ "name":"bill", "age":42 })");
		data_block db, dc;
		db = jx;
		std::cout << "test_data_block, write, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r1 = co_await db.append(dtest.get());
		
		std::cout << "test_data_block, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r2 = co_await dc.read(dtest.get(), db.current_location);

		std::cout << "test_data_block_nested, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = dc.get_string();
		std::cout << x << std::endl;
		co_return 32;
	}

	file_transaction<int64_t> test_json_node(std::shared_ptr<application> _app)
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
		int64_t location = co_await jnwrite.append(dtest.get());

		std::cout << "test_json_node, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t bytes_ex = co_await jnread.read(dtest.get(), location);
		
		std::cout << "test_json_node, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = jnread.data.to_json_string();
		std::cout << x << std::endl;
		co_return 1;
	}

	user_transaction<bool> test_json_table(std::shared_ptr<application> _app)
	{
		using return_type = user_transaction<bool>;

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_table.ctb");

		json_parser jp;

		json test_write = jp.create_object();
		test_write.put_member_i64("ObjectId", 5);
		test_write.put_member("Name", "Joe");
		json test_key = test_write.extract({ "ObjectId" });

		json_table test_table(f, {"ObjectId"});

		co_await test_table.create();

		relative_ptr_type rpt = co_await test_table.put(test_write);
		json test_read = co_await test_table.get(test_key);
		test_read.set_compare_order( {"ObjectId"});

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}
		json db_contents = co_await test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jack");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		db_contents = co_await test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Jill");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);
		test_read.set_natural_order();

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		db_contents = co_await test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		try
		{
			test_key.put_member_i64("ObjectId", 6);
			json t5 = co_await test_table.get(test_key);
		}
		catch (std::exception exc)
		{
			std::cout << __LINE__ << " fail: wrong null access." << std::endl;
			co_return false;
		}

		db_contents = co_await test_table.select_array([](int _index, json& item) {
			return item;
			});

		std::cout << db_contents.to_json_string() << std::endl;

		if (db_contents.size() != 2)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			co_return false;
		}

		test_write.put_member_i64("ObjectId", 2);
		test_write.put_member("Name", "Sydney");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Zeus");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Canada");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		test_write.put_member_i64("ObjectId", 7);
		test_write.put_member("Name", "Zeus");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		test_write.put_member_i64("ObjectId", 1);
		test_write.put_member("Name", "Maraca");
		test_key = test_write.extract({ "ObjectId" });
		co_await test_table.put(test_write);

		test_read = co_await test_table.get(test_key);

		if (test_read.compare(test_write))
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		db_contents = co_await test_table.select_array([](int _index, json& item) {
			return item;
			});

		if (test_table.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements in test." << std::endl;
			co_return false;
		}

		if (db_contents.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			co_return false;
		}
		
		int64_t tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		json counts = jp.create_object();
		json *pcounts = &counts;

		relative_ptr_type count = co_await test_table.for_each([tests,pcounts](int _index, json& _item) -> bool {

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

		db_contents = co_await test_table.select_array([tests](int _index, json& _item) -> json {
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
			co_return false;
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

		db_contents = co_await test_table.select_array(search_key, [tests](int _index, json& _item) -> json {
			return _item;
			}
		);

		any_fails = db_contents.any([](json& _item)->bool {
			std::string temp = _item["Name"];
			return temp != "Zeus";
			});

		if (any_fails) {
			std::cout << __LINE__ << " find bad key query failed" << std::endl;
			co_return false;
		}

		std::cout << db_contents.to_json() << std::endl;

		test_key.put_member_i64("ObjectId", 3);
		bool rdel3 = co_await test_table.erase(test_key);

		if (rdel3) {
			std::cout << "delete non-existing failed" << std::endl;
		}

		test_key.put_member_i64("ObjectId", 1);
		bool rdel1 = co_await test_table.erase(test_key);

		if (!rdel1) {
			std::cout << "delete existing failed" << std::endl;
		}

		test_key.put_member_i64("ObjectId", 7);
		relative_ptr_type rdel7 = co_await test_table.erase(test_key);

		if (!rdel7) {
			std::cout << "delete existing failed" << std::endl;
		}

		json testi = co_await test_table.select_array([tests](int _index, json& item) -> json {
			int64_t object_id = item["ObjectId"];
			return object_id == 7 ? item : json();
			});

		if (testi.size() > 0) {
			std::cout << __LINE__ << " retrieved a deleted item" << std::endl;
			co_return false;
		}
		 
		db_contents = co_await test_table.select_array([tests](int _index, json& item) {
			return item;
			}
		);

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item["ObjectId"];
			return  object_id != 2 && object_id != 5;
			});

		if (any_iteration_fails) {
			std::cout << __LINE__ << " iteration after delete failed." << std::endl;
			co_return false;
		}

		co_return true;
	}

}

#endif
