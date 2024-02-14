#ifndef CORONA_JSON_TABLE_H
#define CORONA_JSON_TABLE_H

#include "corona-constants.hpp"
#include "corona-string_box.hpp"
#include "corona-iarray.hpp"
#include "corona-json.hpp"
#include "corona-file.hpp"
#include "corona-function.hpp"

#include <memory>
#include <functional>
#include <algorithm>
#include <iterator>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <compare>
#include <conio.h>

namespace corona 
{

	class json_node;

	const int debug_json_table = 0;

	static const int JsonTableMaxNumberOfLevels = 40;
	static const int JsonTableMaxLevel = JsonTableMaxNumberOfLevels - 1;

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

		block_header_struct header;
		buffer				bytes;
		int64_t				current_location;

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

		template <typename calling_awaitable> file_batch<calling_awaitable> read(file* _file, int64_t location)
		{
			current_location = location;

			debug_json_table && std::cout << "datablock:read begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			file_task_result header_result = co_await _file->read<calling_awaitable>(location, &header, sizeof(header));

			if (header_result.success) 
			{
				debug_json_table && std::cout << "datablock:read data begin:" << *this << " " << GetCurrentThreadId() << std::endl;

				bytes = buffer(header.data_length);
				file_task_result  data_result = co_await _file->read<calling_awaitable>(header.data_location, bytes.get_ptr(), header.data_length);

				if (data_result.success) 
				{
					debug_json_table && std::cout << "datablock:read data worked:" << *this << " " << GetCurrentThreadId() << std::endl;
					co_return data_result.bytes_transferred; // want to make this 0 or -1 if error
				}
			}

			debug_json_table && std::cout << "header read failed:" << *this << " " << GetCurrentThreadId() << std::endl;

			co_return -1i64;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> write(file* _file)
		{
			debug_json_table&& std::cout << "datablock:write begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (current_location < 0) 
			{
				throw std::invalid_argument("use append for new blocks");
			}

			int size = bytes.get_size();

			if (size > header.data_length)
			{
				header.data_length = size;
				header.next_free_block = header.data_location;
				header.data_location = _file->add(size);
			}

			debug_json_table&& std::cout << "datablock:write header start:" << *this << " " << GetCurrentThreadId() << std::endl;

			file_task_result data_result = co_await _file->write<calling_awaitable>(header.data_location, bytes.get_ptr(), size);

			debug_json_table && std::cout << "datablock:write header end:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (data_result.success)
			{
				file_task_result header_result = co_await _file->write<calling_awaitable>(current_location, &header, sizeof(header));
				co_return header_result.bytes_transferred;
			}

			co_return -1i64;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> append(file* _file)
		{
			int size = bytes.get_size();

			debug_json_table&& std::cout << "append begin:" << *this << " " << GetCurrentThreadId() << std::endl;

			current_location = _file->add(sizeof(header));

			header.block_type = block_id::general_id();
			header.next_free_block = 0;
			header.data_location = _file->add(size);
			header.data_length = size;

			file_task_result data_result = co_await _file->write<calling_awaitable>( header.data_location, bytes.get_ptr(), size);

			debug_json_table&& std::cout << "append write header:" << *this << " " << GetCurrentThreadId() << std::endl;

			if (data_result.success)
			{
				debug_json_table&& std::cout << "append write data:" << *this << " " << GetCurrentThreadId() << std::endl;
				file_task_result header_result = co_await _file->write<calling_awaitable>(current_location, &header, sizeof(header));

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

		int64_t					object_id;
		std::vector<int64_t>	forward;
		json					data;
		data_block				storage;

		json_node()
		{
			object_id = -1;
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
			object_id = 0;
		}

		json get_json()
		{
			json_parser jp;
			json payload = jp.create_object();
			json fow = jp.create_array();
			payload.put_member("ObjectId", object_id);
			payload.put_member("Data", data);
			for (auto pt : forward) {
				fow.append_element( pt);
			}
			payload.put_member("Forward", fow);
			return payload;
		}

		json_node& put_json(json& _src)
		{
			object_id = _src["ObjectId"];
			data = _src["Data"];
			forward.clear();
			auto forwardjson = _src["Forward"];
			forwardjson.for_each([this](json& _item) {
				int64_t ptr = _item;
				forward.push_back(ptr);
				});
			return *this;
		}

		json_node& operator = (json& _src)
		{
			return put_json(_src);
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> read(file* _file, int64_t location)
		{
			debug_json_table&& std::cout << "read:" << *this << std::endl;
			int64_t status = co_await storage.read<calling_awaitable>(_file, location);

			if (status > -1)
			{
				char* c = storage.bytes.get_ptr();
				json_parser jp;
				json payload = jp.parse_object(c);
				put_json(payload);
			}
			co_return status;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> write(file* _file)
		{
			debug_json_table&& std::cout << "write:" << *this << std::endl;
			auto json_payload = get_json();

			storage = json_payload;

			int64_t result = co_await storage.write<calling_awaitable>(_file);
			co_return result;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> append(file* _file)
		{
			debug_json_table&& std::cout << "append:" << *this << std::endl;
			auto json_payload = get_json();

			storage = json_payload;

			int64_t result = co_await storage.append<calling_awaitable>(_file);

			co_return result;
		}
	};

	std::ostream& operator <<(std::ostream& output, json_node& src)
	{
		output << "json_node:" << src.object_id << " bytes";
		return output;
	}

	template <typename poco_type> class poco_node
	{
	public:
		int64_t					object_id;
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

		template <typename calling_awaitable> file_batch<calling_awaitable> read(file* _file, int64_t location)
		{
			storage.init(sizeof(poco_type));

			debug_json_table&& std::cout << "read:" << *this << std::endl;

			int64_t status = co_await storage.read(_file, location);

			if (status > -1)
			{
				poco_type* c = (poco_type *)storage.bytes.get_ptr();
				data = *c;
			}

			co_return status;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> write(file* _file)
		{
			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			debug_json_table&& std::cout << "write:" << *this << std::endl;

			int64_t status = co_await storage.write(_file);

			co_return status;
		}

		template <typename calling_awaitable> file_batch<calling_awaitable> append(file* _file)
		{
			storage.init(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			debug_json_table&& std::cout << "append:" << *this << std::endl;

			int64_t status = co_await storage.write(_file);

			co_return status;
		}
	};

	template<typename T> std::ostream& operator <<(std::ostream& output, poco_node<T>& src)
	{
		output << "json_node:" << src.object_id << " bytes";
		return output;
	}

	struct index_header_struct
	{
	public:
		int64_t	header_node_location;
		int64_t count;
		long	level;
		int64_t forward[JsonTableMaxNumberOfLevels];
	};

	class json_table
	{
		poco_node<index_header_struct> index_header;
		file* database_file;

		const int SORT_ORDER = 1;
		
	public:

		using KEY = int64_t;
		using VALUE = json_node;

		template <typename calling_awaitable> file_transaction<int64_t, calling_awaitable> create_header()
		{
			index_header.data.header_node_location = co_await index_header.append(database_file);
			index_header.data.count = 0;
			for (int i = 0; i < JsonTableMaxNumberOfLevels; i++)
			{
				index_header.data.forward[i] = null_row;
			}
			index_header.data.level = JsonTableMaxNumberOfLevels;
			co_await index_header.write(database_file);
			co_return index_header.data.header_node_location;
		}

		template <typename calling_awaitable> file_transaction<json_node, calling_awaitable> get_header()
		{
			json_node in;

			int64_t result = co_await in.read(database_file, index_header.data.header_node_location);

			if (result < 0) 
			{
				throw std::logic_error("Couldn't read table header.");
			}

			co_return in;
		}

		template <typename calling_awaitable> file_transaction<json_node, calling_awaitable> create_node(int _max_level)
		{
			json_node new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.forward.push_back(rit);
			}

			co_await new_node.append(database_file);
			co_return new_node;
		}

		template <typename calling_awaitable> file_transaction<json_node, calling_awaitable> get_node(file* _file, relative_ptr_type _node_location) const
		{
			json_node node;

			co_await node.read(_file, _node_location);
			co_return node;
		}

	public:

		json_table(file* _database_file) : database_file(_database_file)
		{

		}

		json_table(const json_table& _src) 
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
		}

		json_table operator = (const json_table& _src)
		{
			index_header = _src.index_header;
			database_file = _src.database_file;
			return *this;
		}

		template <typename calling_awaitable> table_transaction<bool, calling_awaitable> contains(const KEY key) const
		{
			relative_ptr_type result = co_await this->find_node<table_transaction<bool>>(key);
			co_return  result != null_row;
		}

		template <typename calling_awaitable> table_transaction<json_node, calling_awaitable> get(const KEY key)
		{
			auto n = this->find_node(key);
			json_node r = co_await get_node<table_transaction<json_node>>(database_file, n);
			co_return r;
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable>
		put(KEY key, VALUE value)
		{
			relative_ptr_type modified_node = co_await this->update_node(key, [value](VALUE& dest) { dest.data = value.data; });
			co_return modified_node;
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable>
		put(KEY key, std::string _json)
		{
			json_parser jp;
			json jx = jp.parse_object(_json);
			relative_ptr_type modified_node = co_await this->update_node(key, [jx](VALUE& dest) { dest.data = jx; });
			co_return modified_node;
		}

		template <typename calling_awaitable> table_transaction<bool, calling_awaitable> 
		erase(const KEY& key)
		{
			int k;
			relative_ptr_type update[JsonTableMaxNumberOfLevels], p;
			json_node qnd, pnd;

			relative_ptr_type q = find_node(update, key);

			if (q != null_row)
			{
				k = 0;
				p = update[k];
				qnd = co_await get_node<table_transaction<bool>>(database_file, q);
				pnd = co_await get_node<table_transaction<bool>>(database_file, p);
				int m = index_header.data.level;
				while (k <= m && pnd.forward[k] == q)
				{
					pnd.forward[k] = qnd.forward[k];
					co_await pnd.write<table_transaction<bool>>(database_file);
					k++;
					if (k <= m) {
						p = update[k];
						pnd = co_await get_node<table_transaction<bool>>(database_file, p);
					}
				}

				::InterlockedDecrement64(&index_header.data.count);
				while (index_header.data.forward[m] == null_row && m > 0) {
					m--;
				}
				index_header.data.level = m;
				co_await index_header.write<table_transaction<bool>>(database_file);
				co_return true;
			}
			else
			{
				co_return false;
			}
		}

		template <typename calling_awaitable> table_transaction<json, calling_awaitable>
		query(std::function<bool(int _index, json_node& _predicate)> _predicate,
			std::function<void(json& _dest_array, int _index, json_node& _predicate)> _transform = nullptr)
		{
			json_parser jp;
			json ja = jp.create_array();

			json_node jn;
			int index = 0;
			for (jn = co_await first_node(); !jn.is_empty(); jn = co_await next_node(jn)) {
				if (_predicate(index, jn)) {
					if (_transform) 
					{
						_transform(ja, index, jn);
					}
					else 
					{
						ja.put_element(-1, jn.data);
					}
					index++;
				}
			}

			co_return ja;
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

		template <typename calling_awaitable> table_transaction<int, calling_awaitable> 
		compare(relative_ptr_type _node, int64_t _id) const
		{
			if (_node != null_row)
			{
				json_node nd = co_await get_node(database_file, _node);
				auto ndkey = nd.object_id;

				if (ndkey < _id)
					co_return -SORT_ORDER;
				else if (ndkey > _id)
					co_return SORT_ORDER;
				else
					co_return 0;
			}
			else
			{
				co_return 1;
			}
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable> 
		find_node(relative_ptr_type* update, int64_t _key) const
		{
			relative_ptr_type found = null_row, p, q;

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = index_header.data.forward[k];
				json_node jn = co_await get_node<calling_awaitable>(database_file, p);
				q = jn.forward[k];
				int comp = compare(q, _key);
				while (comp < 0)
				{
					p = q;
					json_node jn = co_await get_node<calling_awaitable>(database_file, p);
					q = jn.forward[k];
					comp = co_await compare(q, _key);
				}
				if (comp == 0)
					found = q;
				update[k] = p;
			}

			return found;
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable> 
		find_first_gte(relative_ptr_type* update, int64_t _key)
		{
			relative_ptr_type found = null_row, p, q, last_link;

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = index_header.data.forward[k];
				json_node jn = co_await get_node(database_file, p);
				q = jn.forward[k];
				last_link = q;
				int comp = compare(q, _key);
				while (comp < 0)
				{
					p = q;
					last_link = q;
					json_node jn = co_await get_node(database_file, p);
					q = jn.forward[k];
					comp = co_await compare(q, _key);
				}
				if (comp == 0)
					found = q;
				else if (comp < 0)
					found = last_link;
				update[k] = p;
			}

			co_return found;
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable>
		update_node(KEY _key, std::function<void(VALUE& existing_value)> predicate)
		{
			int k;

			debug_json_table&& std::cout << "json_table update_node" << std::endl;

			relative_ptr_type update[JsonTableMaxNumberOfLevels];

			relative_ptr_type q = co_await find_node(update, _key);
			json_node qnd;


			if (q != null_row)
			{
				qnd = co_await get_node(database_file, q);
				predicate(qnd);
				co_await qnd.write(database_file);
				co_return qnd.storage.current_location;
			}

			k = randomLevel();
			if (k > index_header.data.level)
			{
				::InterlockedIncrement(&index_header.data.level);
				k = index_header.data.level;
				update[k] = index_header.data.forward[k];
			}

			qnd = co_await create_node(k);
			qnd.object_id = _key;
			predicate(qnd);
			::InterlockedIncrement64(&index_header.data.count);

			if (update[k] == null_row) {
				update[k] = qnd.storage.current_location;
				index_header.data.forward[k] = index_header.data.forward[k];
			}

			do 
			{
				json_node pnd = co_await get_node(database_file, update[k]);
				qnd.forward[k] = pnd.forward[k];
				pnd.forward[k] = qnd.storage.current_location;

				co_await qnd.write(database_file);
				co_await pnd.write(database_file);

			} while (--k >= 0);

			co_await index_header.write(database_file);

			return qnd.storage.current_location;
		}


		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable>
		find_node(const KEY& key) const
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type value = co_await find_node(update, key);
			return value;
		}

		template <typename calling_awaitable> table_transaction<relative_ptr_type, calling_awaitable>
		find_first_node_gte(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[JsonTableMaxNumberOfLevels];
			relative_ptr_type fn = find_first_gte(update, key);
			return fn;
		}

		template <typename calling_awaitable> table_transaction<json_node, calling_awaitable> first_node()
		{
			json_node jn;
			if (index_header.data.forward[0] != null_row) {
				jn = get_node(database_file, index_header.data.forward[0]);
			}
			return jn;
		}

		template <typename calling_awaitable> table_transaction<json_node, calling_awaitable> next_node(json_node _node)
		{
			if (_node.is_empty())
				return _node;

			json_node nd = co_await get_node(database_file, _node.forward[0]);
			co_return nd;
		}
	};

	std::ostream& operator <<(std::ostream& output, json_table& src)
	{
		bool space = false;
		auto awaiter = src.query<json>([](int _index, json_node& nd) -> bool { return true; });
		json j = awaiter.wait();
		std::string temp = j.to_json_typed_string();
		output << temp;
		return output;
	}

	user_transaction<bool> test_json_table(corona::application& _app);

	file_batch<> test_file(corona::application& _app);
	file_transaction<int64_t> test_data_block(corona::application& _app);
	file_transaction<int64_t> test_json_node(corona::application& _app);

	file_batch<> test_file(corona::application& _app)
	{

		std::cout << "\ntest_file: entry, thread:" << ::GetCurrentThreadId() << std::endl;

		file dtest = _app.create_file(FOLDERID_Documents, "corona_data_block_test.ctb");

		char buffer_write[2048], buffer_read[2048];

		strcpy_s(buffer_write, R"({ "name": "test" })");
		int l = strlen(buffer_write) + 1;

		dtest.add(1000);

		std::cout << "\ntest_file: co_await write, thread:" << ::GetCurrentThreadId() << std::endl;
		file_task_result tsk = co_await dtest.write<file_batch<>>(0, (void *)buffer_write, l);

		std::cout << "\ntest_file: co_await read, thread:" << ::GetCurrentThreadId() << std::endl;
		file_task_result tsk2 = co_await dtest.read<file_batch<>>(0, (void*)buffer_read, l);

		std::cout << "\ntest_file: co_await read, thread:" << ::GetCurrentThreadId() << std::endl;
		
		if (!strcmp(buffer_write, buffer_read))
		{
			std::cout << "\ntest_file: co_return success " << 42 << ", thread:" << ::GetCurrentThreadId() << std::endl;
			co_return 42;
		}

		std::cout << "\ntest_file: co_return fail, thread:" << ::GetCurrentThreadId() << std::endl;
		co_return 0;
	}

	file_transaction<int64_t> test_data_block(corona::application& _app)
	{
		file dtest = _app.create_file(FOLDERID_Documents, "corona_data_block_test.ctb");

		std::cout << "test_data_block, thread:" << ::GetCurrentThreadId() << std::endl;

		json_parser jp;
		json jx = jp.parse_object(R"({ "name":"bill", "age":42 })");
		data_block db, dc;
		db = jx;
		std::cout << "test_data_block, write, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r1 = co_await db.append<file_transaction<int64_t>>(&dtest);
		
		std::cout << "test_data_block, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t r2 = co_await dc.read<file_transaction<int64_t>>(&dtest, db.current_location);

		std::cout << "test_data_block_nested, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = dc.get_string();
		std::cout << x << std::endl;
		co_return 32;
	}

	file_transaction<int64_t> test_json_node(corona::application& _app)
	{
		file dtest = _app.create_file(FOLDERID_Documents, "corona_json_node_test.ctb");

		std::cout << "test_json_node, thread:" << ::GetCurrentThreadId() << std::endl;

		json_node jnwrite, jnread;
		json_parser jp;

		jnwrite.data = jp.create_array();
		for (double i = 0; i < 42; i++)
		{
			jnwrite.data.append_element( i);
		}

		std::cout << "test_json_node, write, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t location = co_await jnwrite.append<file_transaction<int64_t>>(&dtest);

		std::cout << "test_json_node, read, thread:" << ::GetCurrentThreadId() << std::endl;
		int64_t bytes_ex = co_await jnread.read<file_transaction<int64_t>>(&dtest, location);
		
		std::cout << "test_json_node, check, thread:" << ::GetCurrentThreadId() << std::endl;
		std::string x = jnread.data.to_json_string();
		std::cout << x << std::endl;
		co_return 1;
	}

	user_transaction<bool> test_json_table(corona::application& _app)
	{
		long object_id = 100;

		using return_type = user_transaction<bool>;

		file f = _app.create_file(FOLDERID_Documents, "corona_table.ctb");

		object_id = 5;
		json_table test(&f);
		test.put<return_type>(object_id, R"({ "Name" : "Joe" })");
		json_node t1 = co_await test.get<return_type>(object_id);

		if (t1.object_id != object_id || t1.data["Name"].get_string() != "Joe")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		object_id = 7;
		test.put<return_type>(object_id, R"({ "Name" : "Jack" })");
		json_node t2 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Jack")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		co_await test.put<return_type>(object_id, R"({ "Name" : "Jill" })");
		json_node t3 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Jill")
		{
			std::cout << __LINE__ << " fail: wrong updated value." << std::endl;
			co_return false;
		}

		try
		{
			json_node t5 = co_await test.get<return_type>(6);
			std::cout << __LINE__ << " fail: wrong null access." << std::endl;
			co_return false;
		}
		catch (std::exception exc)
		{
			;
		}

		auto db_contents = co_await test.query<return_type>([](int _index, json_node& item) {
			return true;
			});

		if (db_contents.size() != 2)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			co_return false;
		}

		object_id = 2;
		co_await test.put<return_type>(object_id, R"({ "Name" : "Sidney" })");
		t2 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Sidney")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		object_id = 7;
		test.put<return_type>(object_id, R"({ "Name" : "Zeus" })");
		t2 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Zeus")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		object_id = 1;
		test.put<return_type>(object_id, R"({ "Name" : "Canada" })");
		t2 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Canada")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			co_return false;
		}

		object_id = 1;
		test.put<return_type>(object_id, R"({ "Name" : "Maraca" })");
		t2 = co_await test.get<return_type>(object_id);
		if (t2.object_id != object_id || t2.data["Name"].get_string() != "Maraca")
		{
			std::cout << __LINE__ << " fail: wrong updated value." << std::endl;
			co_return false;
		}

		db_contents = test.query<return_type>([](int _index, json_node& item) {
			return true;
			});

		if (test.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements in test." << std::endl;
			co_return false;
		}

		if (db_contents.size() != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of result elements." << std::endl;
			co_return false;
		}

		int tests[4] = { 1, 2, 5, 7 };
		int k = 0;

		db_contents = test.query<return_type>([tests](int _index, json_node& item) {
			if (tests[_index] != item.object_id) {
				std::cout << __LINE__ << " order failed" << std::endl;
				return false;
			}
			});

		db_contents = test.query<return_type>([tests](int _index, json_node& item) {
			return (item.object_id > 1);
			},
			[](json& _dest_array, int _index, json_node& _item) {
				json new_json = _item.data;
				new_json.put_member_i64("ObjectId", _item.object_id);
				_dest_array.put_element(-1, new_json);
			});

		bool any_fails = db_contents.any([](json& _item)->bool {
			return _item["ObjectId"].get_int64() <= 1;
			});

		if (any_fails) {
			std::cout << __LINE__ << " query failed" << std::endl;
			co_return false;
		}

		co_await test.erase<return_type>(1);
		co_await test.erase<return_type>(7);

		auto testi = co_await test.query<return_type>([tests](int _index, json_node& item) -> bool {
			return (item.object_id == 7);
			});

		if (testi.size() > 0) {
			std::cout << __LINE__ << " retrieved a deleted item" << std::endl;
			co_return false;
		}
		 
		db_contents = test.query<return_type>([tests](int _index, json_node& item) {
			return true;
			},
			[](json& _dest_array, int _index, json_node& _item) {
				json new_json = _item.data;
				new_json.put_member_i64("ObjectId", _item.object_id);
				_dest_array.put_element(-1, new_json);
			});

		bool any_iteration_fails = db_contents.any([](json& _item)->bool {
			int64_t object_id = _item["ObjectId"].get_int64();
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
