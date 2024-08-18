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

		relative_ptr_type read(file* _file, relative_ptr_type location)
		{
			current_location = location;

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
					system_monitoring_interface::global_mon->log_block_stop("block", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return data_result.bytes_transferred; // want to make this 0 or -1 if error
				}
				else {
					system_monitoring_interface::global_mon->log_function_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}

			system_monitoring_interface::global_mon->log_block_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return -1i64;
		}

		relative_ptr_type write(file* _file)
		{

			if (current_location < 0) 
			{
				throw std::invalid_argument("use append for new blocks");
			}

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "write block", start_time, __FILE__, __LINE__);

			int size = bytes.get_size();

			if (size > header.data_length)
			{
				header.data_length = 1;
				while (header.data_length < size)
					header.data_length *= 2;
				header.next_free_block = header.data_location;
				header.data_location = _file->add(header.data_length);
			}

			file_command_result data_result = _file->write(header.data_location, bytes.get_ptr(), size);

			if (data_result.success)
			{
				file_command_result header_result = _file->write(current_location, &header, sizeof(header));
				system_monitoring_interface::global_mon->log_block_stop("block", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return header_result.bytes_transferred;
			}
			system_monitoring_interface::global_mon->log_block_stop("block", "write failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return -1i64;
		}

		relative_ptr_type append(file* _file)
		{
			int size = bytes.get_size();

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_block_start("block", "append", start_time, __FILE__, __LINE__);

			current_location = _file->add(sizeof(header) + size);

			header.block_type = block_id::general_id();
			header.next_free_block = 0;
			header.data_location = current_location + sizeof(header);
			header.data_length = size;

			file_command_result header_result = _file->write(current_location, &header, sizeof(header));

			if (header_result.success)
			{
				file_command_result data_result = _file->write(header.data_location, bytes.get_ptr(), size);

				system_monitoring_interface::global_mon->log_block_stop("block", "append complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return current_location;
			}
			else 
			{
				system_monitoring_interface::global_mon->log_block_stop("block", "append failed ", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return -1i64;
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

		relative_ptr_type read(file* _file, relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json", "read", start_time, __FILE__, __LINE__);

			relative_ptr_type status =  storage.read(_file, location);

			if (status > -1)
			{
				char* c = storage.bytes.get_ptr();
				json_parser jp;
				json payload;
				if (c) {
					payload = jp.parse_object(c);
				}
				else {
					std::exception exc("Fatal Read Error");
					system_monitoring_interface::global_mon->log_exception(exc, __FILE__, __LINE__);
					system_monitoring_interface::global_mon->log_json_stop("json", "read failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				}
				if (payload.is_member("ClassName", "SysParseErrors")) {
					std::string temp = "Could not parse json node on retrieval:";
					temp += c;
					system_monitoring_interface::global_mon->log_warning(temp, __FILE__, __LINE__);
					system_monitoring_interface::global_mon->log_json_stop("json", "read failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				put_json(payload);
			}
			system_monitoring_interface::global_mon->log_json_stop("json", "read complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return status;
		}

		relative_ptr_type write(file* _file)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json", "write", start_time, __FILE__, __LINE__);

			auto json_payload = get_json();

			storage = json_payload;

			relative_ptr_type result =  storage.write(_file);
			system_monitoring_interface::global_mon->log_json_stop("json", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		relative_ptr_type append(file* _file)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_json_start("json", "append", start_time, __FILE__, __LINE__);

			auto json_payload = get_json();

			storage = json_payload;

			relative_ptr_type result =  storage.append(_file);

			storage.current_location = result;
			system_monitoring_interface::global_mon->log_json_stop("json", "append complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
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

		relative_ptr_type read(file* _file, relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_poco_start("poco", "read", start_time, __FILE__, __LINE__);

			storage.init(sizeof(poco_type));


			relative_ptr_type status =  storage.read(_file, location);

			if (status > -1)
			{
				poco_type* c = (poco_type *)storage.bytes.get_ptr();
				data = *c;
			}

			system_monitoring_interface::global_mon->log_poco_stop("poco", "read complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return status;
		}

		relative_ptr_type write(file* _file)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_poco_start("poco", "write", start_time, __FILE__, __LINE__);

			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;


			relative_ptr_type status =  storage.write(_file);
			system_monitoring_interface::global_mon->log_poco_stop("poco", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return status;
		}

		relative_ptr_type append(file* _file)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_poco_start("poco", "append", start_time, __FILE__, __LINE__);

			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			relative_ptr_type status =  storage.append(_file);


			system_monitoring_interface::global_mon->log_poco_stop("poco", "append complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);


			return status;
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
		
		relative_ptr_type create_header()
		{
			json_parser jp;
			header_location =  index_header.append(database_file.get());
			json_node header =  create_node(JsonTableMaxLevel, header_key);
			index_header.data.header_node_location = header.storage.current_location;
			index_header.data.count = 0;
			index_header.data.level = JsonTableMaxLevel;
			 index_header.write(database_file.get());
			 header.write(database_file.get());
			return header_location;
		}

		json_node get_header()
		{

			json_node in;

			 index_header.read(database_file.get(), header_location);

			int64_t result =  in.read(database_file.get(), index_header.data.header_node_location);

			if (result < 0) 
			{
				throw std::logic_error("Couldn't read table header.");
			}

			return in;
		}

		json_node create_node(int _max_level, json _data)
		{
			json_node new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.forward.push_back(rit);
			}

			new_node.data = _data;

			 new_node.append(database_file.get());
			return new_node;
		}

		json_node get_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;

			 node.read(_file, _node_location);
			return node;
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
			header_location = location;
			 index_header.read(database_file.get(), header_location);
			system_monitoring_interface::global_mon->log_table_stop("table", "open complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return header_location;
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

			auto key = get_key(value);
			relative_ptr_type modified_node =  this->update_node(key, 
				[value](UPDATE_VALUE& dest) { 
					dest.assign_update(value); 
				}
			);
			system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return modified_node;
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
			auto key = get_key(jx);
			relative_ptr_type modified_node =  this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_update(jx); });
			system_monitoring_interface::global_mon->log_table_stop("table", "put", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return modified_node;
		}

		relative_ptr_type replace(json value)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);

			auto key = get_key(value);
			relative_ptr_type modified_node =  this->update_node(key, [value](UPDATE_VALUE& dest) { dest.assign_replace(value); });
			system_monitoring_interface::global_mon->log_table_stop("table", "replace", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return modified_node;
		}

		relative_ptr_type replace(std::string _json)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "replace", start_time, __FILE__, __LINE__);

			json_parser jp;
			json jx = jp.parse_object(_json);
			auto key = get_key(jx);
			relative_ptr_type modified_node =  this->update_node(key, [jx](UPDATE_VALUE& dest) { dest.assign_replace(jx); });
			return modified_node;
		}

		bool erase(const KEY& key)
		{
			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_table_start("table", "erase", start_time, __FILE__, __LINE__);

			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_node qnd, pnd;

			relative_ptr_type q =  find_node(update, key);

			if (q != null_row)
			{
				k = 0;
				p = update[k];
				qnd =  get_node(database_file.get(), q);
				pnd =  get_node(database_file.get(), p);
				int m = index_header.data.level;
				while (k <= m && pnd.forward[k] == q)
				{
					pnd.forward[k] = qnd.forward[k];
					 pnd.write(database_file.get());
					k++;
					if (k <= m) {
						p = update[k];
						pnd =  get_node(database_file.get(), p);
					}
				}

				::InterlockedDecrement64(&index_header.data.count);

				json_node header = get_header();

				while (header.forward[m] == null_row && m > 0) {
					m--;
				}
				index_header.data.level = m;
				 header.write(database_file.get());
				 index_header.write(database_file.get());
				system_monitoring_interface::global_mon->log_table_stop("table", "erase complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return true;
			}
			else
			{
				system_monitoring_interface::global_mon->log_table_stop("table", "erase failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return false;
			}
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

		relative_ptr_type
		find_node(relative_ptr_type* update, KEY _key)
		{
			relative_ptr_type found = null_row, p, q;
			json_node hdr = get_header();

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
					qnd =  get_node(database_file.get(), q);
					comp = compare_node(qnd, _key);
				}
				while (comp < 0)
				{
					p = q;
					jn = qnd;
					q = jn.forward[k];
					comp = 1;
					if (q != null_row) {
						qnd =  get_node(database_file.get(), q);
						comp = compare_node(qnd, _key);
					}
				}
				if (comp == 0)
					found = q;
				update[k] = p;
			}

			return found;
		}

		relative_ptr_type
		find_first_gte(relative_ptr_type* update, KEY _key)
		{
			relative_ptr_type found = null_row, p, q, last_link;

			json_node header = get_header();

			if (!_key.keys_compatible(key_fields)) {
				return header.forward[0];
			}

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = header.forward[k];
				json_node jn =  get_node(database_file.get(), p);
				q = jn.forward[k];
				last_link = q;
				json_node qnd;
				int comp = 1;
				if (q != null_row) {
					qnd =  get_node(database_file.get(), q);
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
						qnd =  get_node(database_file.get(), q);
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

		relative_ptr_type
		update_node(KEY _key, std::function<void(UPDATE_VALUE& existing_value)> predicate)
		{
			int k;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			json_node header = get_header();

			relative_ptr_type location = header.forward[0];

			relative_ptr_type q =  find_node(update, _key);
			json_node qnd;

			if (q != null_row)
			{
				qnd =  get_node(database_file.get(), q);
				predicate(qnd.data);
				 qnd.write(database_file.get());
				return qnd.storage.current_location;
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
				return -1;
			}

			qnd =  create_node(k, initial_value);
			::InterlockedIncrement64(&index_header.data.count);

			do 
			{
				json_node pnd =  get_node(database_file.get(), update[k]);
				qnd.forward[k] = pnd.forward[k];
				pnd.forward[k] = qnd.storage.current_location;

				 qnd.write(database_file.get());
				 pnd.write(database_file.get());

			} while (--k >= 0);

			 index_header.write(database_file.get());

			return qnd.storage.current_location;
		}


		relative_ptr_type
		find_node(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value =  find_node(update, key);
			return value;
		}

		relative_ptr_type
		find_first_node_gte(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type fn =  find_first_gte(update, key);
			return fn;
		}

		json_node first_node()
		{
			json_node jn;
			auto header = get_header();
			if (header.forward[0] != null_row) {
				jn = get_node(database_file.get(), header.forward[0]);
			}
			return jn;
		}

		json_node next_node(json_node _node)
		{
			if (_node.is_empty())
				return _node;

			json_node nd =  get_node(database_file.get(), _node.forward[0]);
			return nd;
		}
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
