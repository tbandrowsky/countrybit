

#ifndef CORONA_EXPRESS_TABLE_H
#define CORONA_EXPRESS_TABLE_H

namespace corona
{

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

		bool operator < (xfield_holder& _other)
		{
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

			return true;
		}

		~xfield_holder()
		{
			if (bytes) {
				delete[] bytes;
			}
		}

	};

	class xrecord : public xblock
	{
		std::vector<char> key;

		void from_json(json _j)
		{
			key.clear();
			int copy_dest = 0;

			if (_j.object()) 
			{
				auto& ord = _j.get_compare_order();
				for (auto& fld : ord)
				{
					std::string field_name = std::get<1>(fld);
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
					key.insert(key.end(), new_key.get_key(), new_key.get_key()+new_key.get_total_size());
				}
			}
		}

	public:

		xrecord()
		{
		}

		xrecord(json _src)
		{
			from_json(_src);
		}

		xrecord(char* _src, int _length)
		{
			key.insert(key.end(), _src, _src + _length);
		}

		xrecord& operator = (json _key)
		{
			from_json(_key);
			return *this;
		}

		xrecord(const xrecord &_src)
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

	class xfor_each_result {
	public:
		bool is_any;
		bool is_all;
		int64_t count;
	};

	class xrecord_block : public data_block
	{
	public:

		virtual int64_t get_location() {
			return data_block::header.block_location;
		}

		virtual xrecord get_start_key()
		{
			;
		}

		virtual xrecord get_end_key()
		{
			;
		}

		xrecord_block_ptr parent_tree;
		express_table_ptr parent_table;

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

		int										capacity;
		bool									dirty;

		std::map<xrecord, xrecord>				records;
		std::vector<char>						bytes;

		xrecord_block(int _capacity)
		{
			capacity = _capacity;
			dirty = false;
		}

		virtual void erase(const xrecord& key)
		{
			;
		}

		virtual xrecord get(const xrecord& key)
		{
			;
		}

		virtual xfor_each_result for_each(const xrecord& key, std::function<xrecord(xrecord& _item)> _process)
		{
			;
		}

		virtual std::vector<xrecord> select(const xrecord& key, std::function<xrecord(xrecord& _item)> _process)
		{
			;
		}

		virtual void put(const xrecord& key, xrecord& value) 
		{
			dirty = true;
			records.insert_or_assign(key, value);

			// then, we check to see if we have to split the block

			int rsz = records.size() / 2;
			if (records.size() >= capacity)
			{
				xrecord_block* new_xb = new xrecord_block(capacity);
				new_xb->dirty = true;

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
			}
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

		virtual void on_split(xrecord_block_ptr* _split_block, xrecord_block_ptr* _new_block)
		{
			;
		}
	};

	using xrecord_block_ptr = xrecord_block *;

	class express_table_interface 
	{
	public:
		xrecord_block_ptr root;
		virtual void on_root_changed(xrecord_block_ptr _ptr) = 0;

		virtual json get(std::vector<std::string>& _key_members, json _object) = 0;
		virtual void put(std::vector<std::string>& _key_members, std::vector<std::string>& _data_members, json _object) = 0;
		virtual void erase(std::vector<std::string>& _key_members, json _object) = 0;
		virtual xfor_each_result for_each(std::vector<std::string>& _key, json _object, std::function<json(json& _item)> _process) = 0;
		virtual std::vector<xrecord> select(std::vector<std::string>& _key, json _object, std::function<json(json& _item)> _process) = 0;

		virtual void on_created(xrecord_block_ptr* _new_block) = 0;
	};

	using express_table_ptr = express_table_interface*;

	class xrecord_tree_block : public xrecord_block
	{
	public:

		virtual xrecord_block_ptr get(xrecord& _key) = 0;
		virtual void put(xrecord_block_ptr _src) = 0;
		virtual void erase(xrecord_block_ptr _src) = 0;
		virtual xfor_each_result for_each(xrecord& _key, std::function<xrecord_block_ptr(xrecord_block_ptr& _item)> _process) = 0;
		virtual std::vector<xrecord> select(xrecord& _key, std::function<xrecord_block_ptr(xrecord_block_ptr& _item)> _process) = 0;
	};

	class xrecord_json_block : public xrecord_block
	{
	public:
		virtual json get(std::vector<std::string>& _key_members, json _object)
		{

		};
		virtual void put(std::vector<std::string>& _key_members, std::vector<std::string>& _data_members, json _object)
		{
			// first, we create our new record
			json key_json = _object.extract(_key_members);
			json data_json = _object.extract(_data_members);

			xrecord key(key_json);
			xrecord data(data_json);

			xrecord_block::put(key, data);
		}
		virtual void erase(std::vector<std::string>& _key_members, json _object)
		{
			;
		}
		virtual xfor_each_result for_each(std::vector<std::string>& _key, json _object, std::function<json(json& _item)> _process)
		{
			;
		}
		virtual std::vector<xrecord> select(std::vector<std::string>& _key, json _object, std::function<json(json& _item)> _process)
		{
			;
		}
	};

}

#endif

