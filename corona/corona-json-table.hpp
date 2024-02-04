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

/*

namespace std
{
	struct coroutine_traits<corona::json_node, corona::json_table*>
	{
		struct promise_type
		{
			corona::json_node value;

			result_holder<T> get_return_object() const noexcept
			{
				return holder;
			}

			void return_value(T const& v) const
			{
				holder.set_result(v);
			}

			void unhandled_exception() const noexcept
			{
				holder.set_exception(std::current_exception());
			}

			suspend_never initial_suspend() const noexcept
			{
				return{};
			}

			suspend_never final_suspend() const noexcept
			{
				return{};
			}
		};
	};
}

*/

namespace corona 
{

	const int SortedIndexMaxNumberOfLevels = 32;
	const int SortedIndexMaxLevel = SortedIndexMaxNumberOfLevels - 1;

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
			std::string json_string = _src.to_json_typed_string();
			bytes = buffer(json_string.c_str());
			return *this;
		}

		async_io_task<int64_t> read(file* _file, int64_t location)
		{
			current_location = location;

			auto header_result = co_await _file->read(location, &header, sizeof(header));

			if (header_result.success) 
			{
				bytes = buffer(header.data_length);
				auto data_result = co_await _file->read(header.data_location, bytes.get_ptr(), header.data_length);

				if (data_result.success) 
				{
					co_return data_result.bytes_transferred; // want to make this 0 or -1 if error
				}
			}

			co_return -1i64;
		}

		async_io_task<int64_t> write(file* _file)
		{
			if (current_location < 0) 
			{
				return append(_file);
			}

			int size = bytes.get_size();

			if (size > header.data_length)
			{
				header.next_free_block = header.data_location;
				header.data_location = _file->add(size);
			}

			auto data_result = co_await _file->write(header.data_location, bytes.get_ptr(), size);

			if (data_result.success)
			{
				auto header_result = co_await _file->write(current_location, &header, sizeof(header));
				co_return header_result.bytes_transferred;
			}

			co_return -1i64;
		}

		async_io_task<int64_t> append(file* _file)
		{
			int size = bytes.get_size();

			current_location = _file->add(sizeof(header));

			header.block_type = block_id::general_id();
			header.next_free_block = 0;
			header.data_location = _file->add(size);

			auto data_result = co_await _file->write(header.data_location, bytes.get_ptr(), size);

			if (data_result.success)
			{
				auto header_result = co_await _file->write(current_location, &header, sizeof(header));

				co_return current_location;
			}

			co_return -1i64;
		}
	};

	class json_node
	{
	public:

		int64_t					object_id;
		std::vector<int64_t>	forward;
		json					data;
		data_block				storage;

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
				fow.put_element(-1, pt);
			}
			payload.put_member("Forward", fow);
			return payload;
		}

		json_node& put_json(json& _src)
		{
			object_id = _src["ObjectId"];
			data = _src["Data"];
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

		sync<int64_t> read(file* _file, int64_t location)
		{
			sync<int64_t> sw;

			int64_t status = co_await storage.read(_file, location);

			if (status > -1) 
			{
				char *c = storage.bytes.get_ptr();
				json_parser jp;
				json payload = jp.parse_object(c);
				put_json(payload);
			}
			sw.configure(status);
			co_return sw;
		}

		sync<int64_t> write(file* _file)
		{
			sync<int64_t> sw;

			auto json_payload = get_json();

			storage = json_payload;

			int64_t result = co_await storage.write(_file);
			sw.configure(result);

			co_return sw;
		}

		sync<int64_t> append(file* _file)
		{
			sync<int64_t> sw;

			auto json_payload = get_json();

			storage = json_payload;

			int64_t result = co_await storage.append(_file);
			sw.configure(result);

			co_return sw;
		}
	};

	template <typename poco_type> class poco_node
	{
	public:
		int64_t					object_id;
		std::vector<int64_t>	forward;
		poco_type				data;
		data_block				storage;

		void clear()
		{
			object_id = 0;
			forward.clear();
			data = {}
		}

		poco_node& operator = (const poco_type& _src)
		{
			data = _src;
			return *this;
		}

		sync<int64_t> read(file* _file, int64_t location)
		{
			sync<int64_t> sw;

			storage = buffer(sizeof(poco_type));

			int64_t status = co_await storage.read(_file, location);

			if (status > -1)
			{
				poco_type* c = (poco_type *)storage.bytes.get_ptr();
				data = *c;
			}

			co_return status;
		}

		sync<int64_t> write(file* _file)
		{
			sync<int64_t> sw;

			storage = buffer(sizeof(poco_type));
			poco_type* c = (poco_type*)storage.bytes.get_ptr();
			*c = data;

			int64_t status = co_await storage.write(_file);
			sw.configure(status);

			co_return sw;
		}

		sync<int64_t> append(file* _file)
		{
			sync<int64_t> sw;

			auto json_payload = get_json();

			storage = json_payload;

			int64_t status = co_await storage.write(_file);
			sw.configure(status);

			co_return sw;
		}
	};

	struct index_header_struct
	{
	public:
		int64_t	header_node_location;
		int		count;
		int		level;
	};

	class json_table
	{
		poco_node<index_header_struct> index_header;
		file *database_file;

		const int SORT_ORDER = 1;

	public:

		void create_header()
		{
			index_header.data.header_node_location = index_header.append(database_file);
			index_header.data.count = 0;
			index_header.data.level = SortedIndexMaxNumberOfLevels;
			auto header_node = create_node(SortedIndexMaxNumberOfLevels);
			index_header.data.header_node_location = header_node.result.storage.current_location;
			index_header.write(database_file);
		}

		sync<json_node> get_header()
		{
			json_node in;

			int64_t result = in.read(database_file, index_header.data.header_node_location);

			if (result < 0) 
			{
				throw std::logic_error("Couldn't read table header.");
			}

			co_return in;
		}

		sync<json_node> create_node(int _max_level)
		{
			json_node new_node;

			int level_bounds = _max_level + 1;

			for (int i = 0; i < level_bounds; i++)
			{
				relative_ptr_type rit = null_row;
				new_node.forward.push_back(rit);
			}

			new_node.append(database_file);
			co_return new_node;
		}

		sync<json_node> get_node(file* _file, relative_ptr_type _node_id) const
		{
			json_node node;

			co_await node.read(_file, _node_id);
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

		bool pop_front()
		{
			bool result = false;

			json_node q = get_header();

			relative_ptr_type qr;

			qr = q.forward[0];

			if (qr != null_row)
			{
				q.read(database_file, qr);
				result = true;
				remove_node(q.object_id);
			}
			return result;
		}

		json_node get_at(relative_ptr_type offset)
		{
			return get_node(database_file, offset);
		}

		bool erase(int64_t key)
		{
			return this->remove_node(key);
		}

		json_node operator[](const int64_t key) 
		{
			relative_ptr_type n = find_node(key);
			if (n == null_row) {
				throw std::invalid_argument("bad key");
			}
			return get_node(database_file, n);
		}

		bool contains(const int64_t key) const
		{
			return this->find_node(key) != null_row;
		}

		json_node get(const int64_t key)
		{
			auto n = this->find_node(key);
			return get_node(database_file, n);
		}

		bool contains(const int64_t key)
		{
			auto n = this->find_node(key);
			return n != null_row;
		}

		void insert_or_assign(data_pair& kvp)
		{
			relative_ptr_type modified_node = this->update_node(kvp, [kvp](VALUE& dest) { dest = kvp.second; });
		}

		void insert_or_assign(KEY key, VALUE value)
		{
			data_pair kvp(key, value);
			insert_or_assign(kvp);
		}

		void put(data_pair& kvp)
		{
			relative_ptr_type modified_node = this->update_node(kvp, [kvp](VALUE& dest) { dest = kvp.second; });
		}

		void put(KEY key, VALUE value)
		{
			data_pair kvp(key, value);
			insert_or_assign(kvp);
		}

		void put(const KEY& key, VALUE& _default_value, std::function<void(VALUE& existing_value)> predicate)
		{
			data_pair kvp(key, _default_value);
			relative_ptr_type modified_node = this->update_node(kvp, predicate);
		}

	private:

		int randomLevel()
		{
			double r = ((double)rand() / (RAND_MAX));
			int level = (int)(log(1. - r) / log(1. - .5));
			if (level < 1) level = 0;
			else if (level >= SortedIndexMaxLevel) level = SortedIndexMaxLevel;
			return level;
		}

	public:

		inline int size() { return index_header.data.count; }

		bool contains(int64_t _object_id)
		{
			bool all = true;
			if (find_node(_object_id) == null_row)
			{
				all = false;
			}
			return all;
		}

	private:

		// compare a node to a key for equality

		sync<int> compare(relative_ptr_type _node, int64_t _id) const
		{
			if (_node != null_row)
			{
				json_node nd = get_node(database_file, _node);
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

		relative_ptr_type find_node(relative_ptr_type* update, int64_t _key) const
		{
			relative_ptr_type found = null_row, p, q;

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = index_header.data.header_node_location;
				json_node jn = get_node(database_file, p);
				q = jn.forward[k];
				auto comp = compare(q, _key);
				while (comp < 0)
				{
					p = q;
					json_node jn = get_node(database_file, p);
					q = jn.forward[k];
					comp = compare(q, _key);
				}
				if (comp == 0)
					found = q;
				update[k] = p;
			}

			return found;
		}

		relative_ptr_type find_first(relative_ptr_type* update, int64_t _key)
		{
			relative_ptr_type found = null_row, p, q, last_link;

			for (int k = index_header.data.level; k >= 0; k--)
			{
				p = index_header.data.header_node_location;
				json_node jn = get_node(database_file, p);
				q = jn.forward[k];
				last_link = q;
				auto comp = compare(q, _key);
				while (comp < 0)
				{
					p = q;
					last_link = q;
					json_node jn = get_node(database_file, p);
					q = jn.forward[k];
					comp = compare(q, _key);
				}
				if (comp == 0)
					found = q;
				else if (comp < 0)
					found = last_link;
				update[k] = p;
			}

			return found;
		}

		relative_ptr_type update_node(data_pair& kvp, std::function<void(VALUE& existing_value)> predicate)
		{
			int k;
			relative_ptr_type update[SortedIndexMaxNumberOfLevels];
			relative_ptr_type q = find_node(update, kvp.first);
			json_node qnd;

			if (q != null_row)
			{
				qnd = get_node(datebase_file, q);
				predicate(qnd.item().second);
				return q;
			}

			k = randomLevel();
			if (k > get_index_header()->level)
			{
				k = ++(get_index_header()->level);
				update[k] = get_index_header()->header_id;
			}

			qnd = create_node(k);

			qnd.item() = kvp;
			predicate(qnd.item().second);
			get_index_header()->count++;

			do {
				auto pnd = get_node(update[k]);
				qnd.detail(k) = pnd.detail(k);
				pnd.detail(k) = qnd.row_id();
			} while (--k >= 0);

			return qnd.row_id();
		}

		bool remove_node(const KEY& key)
		{
			int k;
			relative_ptr_type update[SortedIndexMaxNumberOfLevels], p;
			index_node qnd, pnd;

			relative_ptr_type q = find_node(update, key);

			mapper_dirty = true;

			if (q != null_row)
			{
				k = 0;
				p = update[k];
				qnd = get_node(q);
				pnd = get_node(p);
				int m = get_index_header()->level;
				while (k <= m && pnd.detail(k) == q)
				{
					pnd.detail(k) = qnd.detail(k);
					k++;
					if (k <= m) {
						p = update[k];
						pnd = get_node(p);
					}
				}
				get_index_header()->count--;
				while (get_header().detail(m) == null_row && m > 0) {
					m--;
				}
				get_index_header()->level = m;
				return true;
			}
			else
			{
				return false;
			}
		}

		relative_ptr_type find_node(const KEY& key) const
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[SortedIndexMaxNumberOfLevels];
			return find_node(update, key);
		}

		relative_ptr_type find_first_node(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			relative_ptr_type update[SortedIndexMaxNumberOfLevels];
			return find_first(update, key);
		}

		relative_ptr_type first_node()
		{
			return get_header().detail(0);
		}

		relative_ptr_type next_node(relative_ptr_type _node)
		{
			if (_node == null_row)
				return _node;

			auto nd = get_node(_node);
			_node = nd.detail(0);
			return _node;
		}
	};

	template <typename KEY, typename VALUE, int SORT_ORDER> std::ostream& operator <<(std::ostream& output, sorted_index<KEY, VALUE, SORT_ORDER>& src)
	{
		bool space = false;
		for (auto& l : src)
		{
			if (space)
				output << "  ";
			space = true;
			output << l.second;
		}
		return output;
	}

	bool test_index();


	bool test_index()
	{

		std::shared_ptr<static_box<20000>> box =
			std::make_shared<static_box<20000>>();

		using test_sorted_index_type = sorted_index<int, istring<30>, 1>;

		test_sorted_index_type test;

		relative_ptr_type test_location;
		test = test_sorted_index_type::create_sorted_index(box, test_location);

		test.insert_or_assign(5, "hello");
		auto t1 = test[5];
		if (t1.get_key() != 5 || t1.get_value() != "hello")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}
		test.insert_or_assign(7, "goodbye");
		auto t2 = test[7];
		if (t2.get_key() != 7 || t2.get_value() != "goodbye" || t2.get_value() != "goodbye")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}
		test.insert_or_assign(7, "something");
		auto t3 = test[7];
		if (t3.get_key() != 7 || t3.get_value() != "something")
		{
			std::cout << __LINE__ << " fail: wrong updated value." << std::endl;
			return false;
		}
		auto t4 = test[7];
		if (t4.get_key() != 7 || t4.get_value() != "something")
		{
			std::cout << __LINE__ << " fail: wrong [] access." << std::endl;
			return false;
		}
		try
		{
			auto t5 = test[6];
			std::cout << __LINE__ << " fail: wrong null access." << std::endl;
			return false;
		}
		catch (std::exception exc)
		{
			;
		}

		int count = 0;

		for (auto t : test)
		{
			count++;
		}

		if (count != 2)
		{
			std::cout << __LINE__ << " fail: wrong number of iterations." << std::endl;
			return false;
		}

		test.put(2, "hello super");
		auto t6 = test[2];
		if (t6.get_key() != 2 || t6.get_value() != "hello super")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test.put(1, "first");
		auto t7 = test[1];
		if (t7.get_key() != 1 || t7.get_value() != "first")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		test.put(1, "second");
		t7 = test[1];
		if (t7.get_key() != 1 || t7.get_value() != "second")
		{
			std::cout << __LINE__ << " fail: wrong inserted value." << std::endl;
			return false;
		}

		count = std::count_if(test.begin(), test.end(), [](auto _item) {
			return true;
			});

		if (count != 4)
		{
			std::cout << __LINE__ << " fail: wrong number of iterations." << std::endl;
			return false;
		}

		int tests[4] = { 1, 2, 5, 7 };
		int k = 0;
		for (auto item : test)
		{
			if (tests[k] != item.first) {
				std::cout << __LINE__ << " loop failed" << std::endl;
				return false;
			}
			k++;
		}

		int tests2[3] = { 2, 5, 7 };

		k = 0;
		for (auto item : test.where([](auto& kvpi) { return kvpi.first > 1; }))
		{
			if (tests2[k] != item.first) {
				std::cout << __LINE__ << " starting from key failed" << std::endl;
				return false;
			}
			k++;
		}

		k = 0;
		for (auto item : test.where([](auto& kvpi) { return kvpi.first > 1; }))
		{
			if (tests2[k] != item.first) {
				std::cout << __LINE__ << " loop failed" << std::endl;
				return false;
			}
			k++;
		}

		test.erase(1);
		test.erase(7);

		auto testi = test
			.where([](auto& a) { return a.first == 7; });

		if (testi.exists()) {
			std::cout << __LINE__ << " existence failed" << std::endl;
			return false;
		}

		if (!testi.eoi()) {
			std::cout << __LINE__ << " eoi failed" << std::endl;
			return false;
		}

		int tests3[2] = { 2, 5 };

		k = 0;

		for (auto item : test)
		{
			if (tests3[k] != item.first) {
				std::cout << __LINE__ << " erasing iterator failed" << std::endl;
				return false;
			}
			k++;
		}

		return true;
	}

}

#endif