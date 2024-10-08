
#pragma once

#ifndef CORONA_EXPRESS_TABLE_H
#define CORONA_EXPRESS_TABLE_H

namespace corona
{

	class xrecord_block;
	using xrecord_block_ptr = xrecord_block*;

	class express_table_interface
	{
	public:

		xrecord_block_ptr root;
		virtual void on_root_changed(xrecord_block_ptr _ptr) = 0;

		virtual json get(std::vector<std::string>& _key_members, json _object) = 0;
		virtual void put(std::vector<std::string>& _key_members, std::vector<std::string>& _data_members, json _object) = 0;
		virtual void erase(std::vector<std::string>& _key_members, json _object) = 0;
		virtual xfor_each_result for_each_object(json _object, std::function<relative_ptr_type>(json& _item)> _process) = 0;
		virtual std::vector<json> select_objects(json _object, std::function<json(json& _item)> _process) = 0;
		virtual void on_split(xrecord_block_ptr _left_root_block, xrecord_block_ptr _right_root_block) = 0;
	};

	class xblock
	{
	public:
		virtual char* before_read(int32_t size) = 0;
		virtual void after_read(char* _bytes) = 0;
		virtual char* before_write(int32_t* _size) = 0;
	};

	template <typename T> 
	class poco_xblock
	{
	public:

		T data;

		virtual char* before_read(int32_t size) override
		{
			char* io_bytes = (char*)&data;
			return io_bytes;
		}

		virtual void after_read(char* _bytes) override
		{

		}

		virtual char* before_write(int32_t* _size) override
		{
			*_size = sizeof(data);
			char* io_bytes = (char*)&data;
			return io_bytes;
		}

		bool operator < (const poco_block_serializable& _t)
		{
			return data < _t.data;
		}
	};

	using object_id_key = poco_xblock<int64_t>;

	class xfield
	{
	public:
		field_types data_type;
		int			data_length;
		char		data[];

		std::string get_string()
		{
			std::string t;
			if (data_type == field_types::ft_string)
			{
				t = &data[0];
			}
			return t;
		}

		double get_double()
		{
			double t;
			if (data_type == field_types::ft_double)
			{
				t = *((double *) &data[0]);
			}
			return t;
		}

		int64_t get_int64()
		{
			int64_t t;
			if (data_type == field_types::ft_int64)
			{
				t = *((int64_t*)&data[0]);
			}
			return t;
		}

		date_time get_datetime()
		{
			date_time t;
			if (data_type == field_types::ft_datetime)
			{
				t = *((date_time*)&data[0]);
			}
			return t;
		}
	
	};

	class xfield_holder
	{

		// bytes is where placement operator new holder for key may sometimes be allocated.
		// however bytes can be null while key is not.  If bytes is null and key is not, then
		// that means key came from bytes someone else knows.  So we leave bytes null so the destructor
		// won't try and free them.

		char* bytes;

		xfield* key;
		int total_size;

	public:

		xfield_holder()
		{
			bytes = nullptr;
			key = nullptr;
			total_size = 0;
		}

		xfield_holder(const xfield_holder& _src)
		{
			total_size = _src.total_size;
			bytes = new char[total_size];
			std::copy(_src.bytes, _src.bytes + total_size, bytes);
			key = new (bytes) xfield();
		}

		xfield_holder(xfield_holder&& _src)
		{
			std::swap(total_size, _src.total_size);
			std::swap(bytes, _src.bytes);
			std::swap(key, _src.key);
		}

		xfield_holder& operator =(const xfield_holder& _src)
		{
			total_size = _src.total_size;
			bytes = new char[total_size];
			std::copy(_src.bytes, _src.bytes + total_size, bytes);
			key = new (bytes) xfield();
			return *this;
		}

		xfield_holder& operator =(xfield_holder&& _src)
		{
			std::swap(total_size, _src.total_size);
			std::swap(bytes, _src.bytes);
			std::swap(key, _src.key);
			return *this;
		}

		xfield_holder(std::vector<char>& _bytes, int _offset)
		{
			char *sb = &_bytes[_offset];
			key = new (sb) xfield();
			total_size = key->data_length + sizeof(xfield);
			bytes = nullptr;
		}

		xfield_holder(const std::string& _data)
		{
			total_size = _data.size() + sizeof(xfield);
			bytes = new char[total_size];
			key = new (bytes) xfield();
			key->data_length = _data.size();
			key->data_type = field_types::ft_string;
			std::copy(_data.c_str(), _data.c_str() + _data.size(), key->data);
		}

		xfield_holder(int64_t _data)
		{
			total_size = sizeof(_data) + sizeof(xfield);
			bytes = new char[total_size];
			key = new (bytes) xfield();
			key->data_length = sizeof(_data);
			key->data_type = field_types::ft_int64;
			int64_t* dest = (int64_t *)key->data;
			*dest = _data;
		}

		xfield_holder(double _data)
		{
			total_size = sizeof(_data) + sizeof(xfield);
			bytes = new char[total_size];
			key = new (bytes) xfield();
			key->data_length = sizeof(_data);
			key->data_type = field_types::ft_double;
			double* dest = (double*)key->data;
			*dest = _data;
		}

		xfield_holder(date_time _data)
		{
			total_size = sizeof(_data) + sizeof(xfield);
			bytes = new char[total_size];
			key = new (bytes) xfield();
			key->data_length = sizeof(_data);
			key->data_type = field_types::ft_datetime;
			date_time* dest = (date_time*)key->data;
			*dest = _data;
		}

		int get_total_size()
		{
			return total_size;
		}

		xfield* get_key()
		{
			return key;
		}

		operator bool()
		{
			return key != nullptr;
		}

		bool operator < (xfield_holder& _other)
		{
			if (*this and _other) {
				xfield* this_key = get_key();
				xfield* other_key = _other.get_key();
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_key()->data_type)
				{
				case field_types::ft_string:
					return this_key->get_string() < other_key->get_string();
				case field_types::ft_double:
					return this_key->get_double() < other_key->get_double();
				case field_types::ft_datetime:
					return this_key->get_datetime() < other_key->get_datetime();
				case field_types::ft_int64:
					return this_key->get_int64() < other_key->get_int64();
				}
			}
			else if (*this and not _other)
			{
				return false;
			}
			else
				return false;
		}

		bool operator == (xfield_holder& _other)
		{
			if (*this and _other) {
				xfield* this_key = get_key();
				xfield* other_key = _other.get_key();
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_key()->data_type)
				{
				case field_types::ft_string:
					return this_key->get_string() == other_key->get_string();
				case field_types::ft_double:
					return this_key->get_double() == other_key->get_double();
				case field_types::ft_datetime:
					return this_key->get_datetime() == other_key->get_datetime();
				case field_types::ft_int64:
					return this_key->get_int64() == other_key->get_int64();
				}
			}

			return false;
		}

		bool operator > (xfield_holder& _other)
		{
			if (*this and _other) {
				xfield* this_key = get_key();
				xfield* other_key = _other.get_key();
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_key()->data_type)
				{
				case field_types::ft_string:
					return this_key->get_string() > other_key->get_string();
				case field_types::ft_double:
					return this_key->get_double() > other_key->get_double();
				case field_types::ft_datetime:
					return this_key->get_datetime() > other_key->get_datetime();
				case field_types::ft_int64:
					return this_key->get_int64() > other_key->get_int64();
				}
			}
			else if (*this and not _other)
			{
				return true;
			}

			return false;
		}

		~xfield_holder()
		{
			if (bytes) 
			{
				delete[] bytes;
			}
		}

	};

	class xrecord : public xblock
	{
		std::vector<char> key;


	public:

		xrecord()
		{

		}

		xrecord(std::vector<std::string>& _keys, json _src)
		{
			put_json(_keys, _src);
		}

		xrecord(char* _src, int _length)
		{
			key.insert(key.end(), _src, _src + _length);
		}

		xrecord(const xrecord& _src)
		{
			key = _src.key;
		}

		xrecord& operator =(const xrecord& _src)
		{
			key = _src.key;
			return *this;
		}

		xrecord(xrecord&& _src)
		{
			key = std::move(_src.key);
		}

		xrecord& operator = (xrecord&& _src)
		{
			key = std::move(_src.key);
			return *this;
		}

		void put_json(std::vector<std::string>& _keys, json _j)
		{
			key.clear();
			int index = 0;

			for (auto& fld : _keys)
			{
				std::string field_name = fld;
				auto m = _j[field_name];
				xfield_holder new_key;
				switch (m.get_field_type())
				{
				case field_types::ft_string:
					new_key = std::move(xfield_holder(m.get_string()));
					break;
				case field_types::ft_double:
					new_key = std::move(xfield_holder(m.get_double()));
					break;
				case field_types::ft_datetime:
					new_key = std::move(xfield_holder(m.get_datetime()));
					break;
				case field_types::ft_int64:
					new_key = std::move(xfield_holder(m.get_int64()));
					break;
				default:
					throw std::logic_error("Only use string, double, datetime and int64 for index keys");
					break;
				}
				key.insert(key.end(), new_key.get_key(), new_key.get_key() + new_key.get_total_size());
			}
		}

		void get_json(json& _dest, std::vector<std::string>& _keys)
		{
			int index = 0;

			int this_offset = 0;
			xfield_holder this_key;

			this_key = std::move(get_key(this_offset, &this_offset));
			while (this_key and index < _keys.size());
			{
				std::string field_name = _keys[index];
				switch (this_key.get_key()->data_type)
				{
				case field_types::ft_string:
					_dest.put_member(field_name, this_key.get_key()->get_string());
					break;
				case field_types::ft_double:
					_dest.put_member(field_name, this_key.get_key()->get_double());
					break;
				case field_types::ft_datetime:
					_dest.put_member(field_name, this_key.get_key()->get_datetime());
					break;
				case field_types::ft_int64:
					_dest.put_member(field_name, this_key.get_key()->get_int64());
					break;
				default:
					throw std::logic_error("Only use string, double, datetime and int64 for index keys");
					break;
				}
				this_key = std::move(get_key(this_offset, &this_offset));
				index++;
			}
		}

		relative_ptr_type get_location()
		{
			int next_offset;
			xfield_holder xfkey;
			xfkey = get_key(0, &next_offset);
			if (xfkey and xfkey.get_key()->data_type == field_types::ft_int64) {
				return xfkey.get_key()->get_int64();
			}
			return null_row;
		}

		void put_location(data_block& db)
		{
			clear();
			add( db.header.block_location );
		}

		void clear()
		{
			key.clear();
		}

		xrecord& add(double _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), new_key.get_key(), new_key.get_key() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(std::string _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), new_key.get_key(), new_key.get_key() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(date_time _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), new_key.get_key(), new_key.get_key() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(int64_t _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), new_key.get_key(), new_key.get_key() + new_key.get_total_size());
			return *this;
		}

		size_t size() const
		{
			return key.size();
		}

		bool is_empty()
		{
			return key.empty();
		}

		virtual char* before_read(int32_t _size)  override
		{
			key.resize(_size);
			return (char*)key.data();
		}

		virtual void after_read(char* _bytes)  override
		{

		}

		virtual char* before_write(int32_t* _size) override
		{
			*_size = key.size();
			return (char*)key.data();
		}

		xfield_holder get_key(int _offset, int* _next_offset)
		{
			xfield_holder t;
			if (_offset < key.size()) {
				t = xfield_holder(key, _offset);
				*_next_offset = t.get_total_size() + _offset;
			}
			return t;
		}


		bool operator == (xrecord& _other)
		{
			int this_offset = 0;
			int other_offset = 0;
			xfield_holder this_key;
			xfield_holder other_key;

			other_key = std::move(_other.get_key(other_offset, &other_offset));
			this_key = std::move(get_key(this_offset, &this_offset));
			while (this_key and other_key);
			{
				if (not (this_key == other_key))
				{
					return false;
				}
				other_key = std::move(_other.get_key(other_offset, &other_offset));
				this_key = std::move(get_key(this_offset, &this_offset));
			}

			return true;
		}

		bool operator < (xrecord& _other)
		{
			int this_offset = 0;
			int other_offset = 0;
			xfield_holder this_key;
			xfield_holder other_key;

			do {
				other_key = std::move(_other.get_key(other_offset, &other_offset));
				this_key = std::move(get_key(this_offset, &this_offset));
				if (this_offset and other_offset)
				{
					if (not (this_key < other_key))
					{
						return false;
					}
				}
			} while (this_offset and other_offset);

			return true;
		}

	};

	class xfor_each_result 
	{
	public:
		bool is_any;
		bool is_all;
		int64_t count;
	};

	class xrecord_block : public data_block
	{
	protected:

		int										capacity;
		bool									dirty;

		std::map<xrecord, xrecord>				records;
		std::vector<char>						bytes;

		struct xblock_record_list
		{
			int count;
			struct xblock_ref
			{
				int key_offset;
				int key_size;
				int value_offset;
				int value_size;
			} offsets[];
		};

	public:

		xrecord_block(int _capacity)
		{
			capacity = _capacity;
			dirty = false;
		}

		virtual int64_t get_location() 
		{
			return data_block::header.block_location;
		}

		bool get_dirty()
		{
			return dirty;
		}

		void set_dirty(bool _dirty)
		{
			dirty = false;
		}

		virtual xrecord get_start_key()
		{
			xrecord key;
			auto max_key = records.begin();
			if (max_key != records.end())
			{
				key = max_key->first;
			}
			return key;
		}

		virtual xrecord get_end_key()
		{
			xrecord key;
			auto max_key = records.crbegin();
			if (max_key != records.rend())
			{
				key = max_key->first;
			}
			return key;
		}

		virtual void put(const xrecord& key, xrecord& value)
		{
			dirty = true;
			records.insert_or_assign(key, value);

			// then, we check to see if we have to split the block

			if (records.size() >= capacity)
			{
				on_full();
			}
		}

		virtual xrecord get(const xrecord& key)
		{
			xrecord temp;
			records.find(key);
			records.erase(key);
		}

		virtual void erase(const xrecord& key)
		{
			records.erase(key);
		}

		virtual xfor_each_result for_each(xrecord _key, std::function<relative_ptr_type(int _index, xrecord& _item)> _process)
		{
			xfor_each_result result;
			auto it = records.lower_bound(key);
			int index = 0;
			while (it != records.end() and key < it->get_key()) {
				relative_ptr_type t = _process( index, it->second );
				if (t != null_row) {
					result.is_any = true;
					index++;
				}
				it++;
			}
			result.is_all = index == records.size();
			result.count = index;
			return result;
		}

		virtual std::vector<xrecord> select(xrecord key, std::function<xrecord(int _index, xrecord& _item)> _process)
		{
			std::vector<xrecord> results;
			auto it = records.lower_bound(key);
			int index = 0;
			while (it != records.end()) {
				xrecord t = _process(index, it->second);
				if (not t.is_empty())
				{
					if (t == key) {
						results.push_back(t);
						index++;
					}
					else if (key < t) 
					{
						break;
					}
				}
				it++;
			}
			return results;
		}

		virtual char* before_read(int32_t _size)  override
		{
			bytes.resize(_size);
			return (char*)bytes.data();
		}

		virtual void after_read(char* _bytes) override
		{
			records.clear();
			xblock_record_list* header = (xblock_record_list*)_bytes;
			for (int i = 0; i < header->count; i++)
			{
				xblock_record_list::xblock_ref* rl = &header->offsets[i];
				xrecord k(bytes.data() + rl->key_offset, rl->key_size); // just deserializing the records.
				xrecord v(bytes.data() + rl->value_offset, rl->value_size); // just deserializing the records.
				records.insert_or_assign(k, v);
			}
			dirty = false;
		}

		virtual void finished_io(char* _bytes) override
		{
			;
		}

		virtual char* before_write(int32_t* _size) override
		{
			int32_t record_bytes = 0;
			int32_t total_bytes = 0;
			int32_t header_bytes = 0;
			int32_t count;

			for (auto& r : records)
			{
				record_bytes += r.first.size();
				record_bytes += r.second.size();
				total_bytes = record_bytes;
				count++;
			}
			header_bytes = sizeof(xblock_record_list) + sizeof(xblock_record_list::xblock_ref) * count;
			total_bytes += header_bytes;
			*_size = total_bytes;
			bytes.resize(total_bytes);

			char* base = bytes.data();
			char* current = base;
			xblock_record_list* header = (xblock_record_list*)current;
			current += header_bytes;

			int i = 0;
			for (auto& r : records)
			{
				auto* rl = &header->offsets[i];
				xrecord rkey = r.first;
				xrecord& skey = r.second;
				rl->key_size = rkey.size();
				rl->key_offset = current - base;
				int size_actual;
				xrecord* dest;
				dest = (xrecord*)rkey.before_write(&size_actual);
				std::copy(dest, dest + size_actual, current);
				current += rl->key_size;

				rl->value_size = skey.size();
				rl->value_offset = current - base;
				dest = (xrecord*)skey.before_write(&size_actual);
				std::copy(dest, dest + size_actual, current);
				current += size_actual;
			}
			return base;
		}

		virtual void after_write(char* _t) override
		{

		}


		virtual void on_full()
		{
			xrecord_block* new_xb = get_new_block();
			new_xb->set_dirty(true);

			int rsz = records.size() / 2;

			// time to split the block
			std::vector<xrecord> keys_to_delete;
			keys_to_delete.resize(rsz);

			int count = 0;

			for (auto& kv : records)
			{
				if (count > rsz) {
					keys_to_delete[count] = kv.first;
					new_xb->put(kv.first, kv.second);
				}
				count++;
			}

			for (auto& kv : keys_to_delete)
			{
				records.erase(kv);
			}

			notify_split(this, new_xb);
		}

		virtual void notify_split(xrecord_block_ptr _split_block, xrecord_block_ptr _new_block) = 0;
		virtual xrecord_block_ptr get_new_block() = 0;
	};

	class xblock_branch : public xrecord_block
	{
	public:

		express_table_interface		*table;
		xblock_branch				*parent;

		xblock_branch(express_table_interface* _table, xblock_branch *_parent, int _capacity) :
			table(_table),
			parent(_parent),
			xrecord_block(_capacity)
		{
			
		}

		virtual void put_block(xrecord_block_ptr _block)
		{
			auto new_key = _block->get_start_key();
			int64_t location = _block->get_location(); // remember this is not written yet.
			xrecord xlocation;
			xlocation.add(location);
			put(new_key, xlocation);
		}

		virtual xrecord_block_ptr get_new_block()
		{
			return new xblock_branch(table, parent, capacity);
		}

		virtual void on_split(xrecord_block_ptr _left_root_block, xrecord_block_ptr _right_root_block)
		{
		}

		virtual void notify_split(xrecord_block_ptr _split_block, xrecord_block_ptr _new_block)
		{
			if (parent) {
				parent->on_split(_split_block, _new_block);
			}
			else if (table)
			{
				table->on_split(_split_block, _new_block);
			}
		}
	};

	class xblock_leaf: public xrecord_block
	{
	public:

		xblock_branch* parent;

		xblock_leaf(xblock_branch*_parent, int _capacity) : xrecord_block(_capacity), parent(nullptr)
		{
			;
		}

		virtual void notify_split(xrecord_block_ptr _split_block, xrecord_block_ptr _new_block)
		{
			if (parent) 
			{
				parent->on_split(_split_block, _new_block);
			}
		}

		virtual xrecord_block_ptr get_new_block() 
		{
			;
		}
	};

	class express_table_header : public data_block
	{
	public:
		std::string data;

		relative_ptr_type self_location;
		relative_ptr_type root_location;
		xrecord_block_ptr root;

		std::vector<std::string> key_members;
		std::vector<std::string> object_members;

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			_dest.put_member_i64("self_location", self_location);
			_dest.put_member_i64("root_location", root_location);
			json kms = jp.create_array(key_members);
			_dest.put_member("key_members", kms);
			json oms = jp.create_array(object_members);
			_dest.put_member("object_members", kms); 
			_dest.put_member("keys", kms);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			self_location = _src["self_location"];
			root_location = _src["root_location"];
			json kms = _src["key_members"];
			key_members = kms.to_string_array();
			json oms = _src["object_members"];
			object_members = oms.to_string_array();
		}

		virtual char* before_read(int32_t _size)  override
		{
			data.resize(_size);
			return (char*)data.c_str();
		}

		virtual void after_read(char* _bytes) override
		{
			json_parser parser;
			json temp = parser.parse_object(data);
			put_json(temp);
		}

		virtual void finished_io(char* _bytes) override
		{
			;
		}

		virtual char* before_write(int32_t* _size) override
		{
			json_parser jp;
			json temp = jp.create_object();
			get_json(temp);
			data = temp.to_json_typed();
			if (data.size() > (1 << 30))
				throw std::logic_error("Block too big");
			*_size = (int32_t)data.size();
			char *r = data.data();
			return r;
		}

		virtual void after_write(char* _t) override
		{

		}

	};

	class express_table : public express_table_interface
	{

	public:
		std::string								data;
		file_block*								fb;
		std::shared_ptr<express_table_header>	table_header;
		std::shared_ptr<xblock_branch>			root;

		express_table(file_block* _fb, std::shared_ptr<express_table_header> _header) :
			fb(_fb),
			table_header(_header)
		{
			if (_header->self_location < 0)
			{
				create();
			}
			else
			{
				open();
			}
		}

		std::shared_ptr<express_table_header> create_header()
		{
			json_parser jp;

			table_header = std::make_shared<express_table_header>();
			root = std::make_shared<xblock_branch>();			
			table_header->root_location = root->append(fb);
			relative_ptr_type table_header_location = table_header->append(fb);
			table_header->self_location = table_header_location;
			table_header->write(fb);

			return table_header;
		}

		std::shared_ptr<express_table_header> create()
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

		virtual json get(json _object)
		{
			json_parser jp;
			json jresult;
			xrecord key(table_header->key_members, _object);
			xrecord result = root->get(key);
			if (not result.is_empty()) {
				jresult = jp.create_object();
				key.get_json(jresult, table_header->key_members);
				result.get_json(jresult, table_header->object_members);
				return jresult;
			}
			return jresult;
		}

		virtual void put(json _object)
		{
			xrecord key(table_header->key_members, _object);
			xrecord data(table_header->object_members, _object);
			root->put(key, data);
		}

		virtual void erase(json _object)
		{
			xrecord key(table_header->key_members, _object);
			root->erase(key);
		}

		virtual xfor_each_result for_each(json _object, std::function<json(json& _item)> _process)
		{
			xrecord key(table_header->key_members, _object);
			root->for_each(key, [this](int _index, xrecord& _src)->xrecord {
				;
				});
		}

		virtual std::vector<xrecord> select(json _object, std::function<json(json& _item)> _process)
		{
			;
		}

		virtual void on_split(xrecord_block_ptr _left_root_block, xrecord_block_ptr _right_root_block)
		{
			;
		}
	};
}

#endif

