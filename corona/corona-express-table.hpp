
#pragma once

#ifndef CORONA_EXPRESS_TABLE_H
#define CORONA_EXPRESS_TABLE_H

namespace corona
{


	class xrecord_block;
	using xrecord_block_ptr = xrecord_block*;

	struct xblock_ref
	{
	public:
		xblock_types		block_type;
		relative_ptr_type	location;
	};

	class xfor_each_result
	{
	public:
		bool is_any;
		bool is_all;
		int64_t count;
	};

	class express_table_interface
	{
	public:

		xrecord_block_ptr root;
		virtual void on_root_changed(xrecord_block_ptr _ptr) = 0;

		virtual json get(json _object) = 0;
		virtual void put(json _object) = 0;
		virtual void erase(json _object) = 0;
		virtual xfor_each_result for_each(json _object, std::function<relative_ptr_type(json& _item)> _process) = 0;
		virtual json select(json _object, std::function<json(json& _item)> _process) = 0;
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

		bool operator < (const poco_xblock& _t)
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
		char		data[0];

		xfield()
		{
			// don't change a thing.
		}

		std::string get_string() const
		{
			std::string t;
			if (data_type == field_types::ft_string)
			{
				t = &data[0];
			}
			return t;
		}

		double get_double() const
		{
			double t;
			if (data_type == field_types::ft_double)
			{
				t = *((double *) &data[0]);
			}
			return t;
		}

		int64_t get_int64() const
		{
			int64_t t;
			if (data_type == field_types::ft_int64)
			{
				t = *((int64_t*)&data[0]);
			}
			return t;
		}

		date_time get_datetime() const
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
			if (_src.bytes) {
				bytes = new char[total_size];
				std::copy(_src.bytes, _src.bytes + total_size, bytes);
				key = new (bytes) xfield();
			}
			else {
				bytes = nullptr;
				key = _src.key;
			}
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

		xfield_holder(char *_bytes, int _offset)
		{
			char *sb = &_bytes[_offset];
			key = new (sb) xfield();
			total_size = key->data_length + sizeof(xfield);
			bytes = nullptr;
		}

		xfield_holder(const std::string& _data)
		{
			total_size = _data.size() + sizeof(xfield) + 1;
			bytes = new char[total_size];
			key = new (bytes) xfield();
			key->data_length = _data.size() + 1; // +1 are for the nullptrs
			key->data_type = field_types::ft_string;
			std::copy(_data.c_str(), _data.c_str() + _data.size()+1, key->data);
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

		int get_total_size() const
		{
			return total_size;
		}

		xfield* get_field() const
		{
			return key;
		}

		char* get_bytes() const
		{
			return (char*)key;
		}

		operator bool() const
		{
			return key != nullptr;
		}

		bool operator < (const xfield_holder& _other) const
		{
			xfield* this_key = get_field();
			xfield* other_key = _other.get_field();
			if (this_key and other_key) {
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (this_key->data_type)
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
			else if (this_key and not other_key)
			{
				return false;
			}
			else if (other_key and not this_key)
			{
				return true;
			}
			else
				return false;
		}

		bool operator == (const xfield_holder& _other) const
		{
			xfield* this_key = get_field();
			xfield* other_key = _other.get_field();

			if (this_key and other_key) {
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_field()->data_type)
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

		bool operator > (const xfield_holder& _other) const
		{
			if (*this and _other) {
				xfield* this_key = get_field();
				xfield* other_key = _other.get_field();
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_field()->data_type)
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

		bool operator != (const xfield_holder& _other) const
		{
			if (*this and _other) {
				xfield* this_key = get_field();
				xfield* other_key = _other.get_field();
				if (this_key->data_type != other_key->data_type)
					return this_key->data_type < other_key->data_type;

				switch (get_field()->data_type)
				{
				case field_types::ft_string:
					return this_key->get_string() != other_key->get_string();
				case field_types::ft_double:
					return this_key->get_double() != other_key->get_double();
				case field_types::ft_datetime:
					return this_key->get_datetime() != other_key->get_datetime();
				case field_types::ft_int64:
					return this_key->get_int64() != other_key->get_int64();
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
				bytes = nullptr;
			}
		}
	
	};

	void test_xfield(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xfield", "start", start, __FILE__, __LINE__);

		date_time testa = date_time(2024, 10, 8);
		date_time testb = date_time(2024, 11, 8);

		xfield_holder test_datetimea(testa);
		xfield_holder test_doublea(42.0);
		xfield_holder test_int64a(12i64);
		xfield_holder test_stringa("testa");

		xfield_holder test_datetimeb(testb);
		xfield_holder test_doubleb(52.0);
		xfield_holder test_int64b(22i64);
		xfield_holder test_stringb("testb");

		bool result;

		// check values

		result = test_datetimea.get_field()->get_datetime() == testa;
		_tests->test({ "data dt", result, __FILE__, __LINE__ });

		result = test_doublea.get_field()->get_double() == 42.0;
		_tests->test({ "data dble", result, __FILE__, __LINE__ });

		result = test_int64a.get_field()->get_int64() == 12;
		_tests->test({ "data i64", result, __FILE__, __LINE__ });

		result = test_stringa.get_field()->get_string() == "testa";
		_tests->test({ "data i64", result, __FILE__, __LINE__ });

		result = test_datetimeb.get_field()->get_datetime() == testb;
		_tests->test({ "data dt 2", result, __FILE__, __LINE__ });

		result = test_doubleb.get_field()->get_double() == 52.0;
		_tests->test({ "data dble 2", result, __FILE__, __LINE__ });

		result = test_int64b.get_field()->get_int64() == 22;
		_tests->test({ "data i64 2", result, __FILE__, __LINE__ });

		result = test_stringb.get_field()->get_string() == "testb";
		_tests->test({ "data i64 2", result, __FILE__, __LINE__ });

		int l;
		char* c;
		char* test_copy;

		// these tests shouldn't leak

		//datetime
		l = test_datetimeb.get_total_size();
		c = (char*)test_datetimeb.get_field();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copydt(c, l);
		result = copydt.get_field()->get_string() == test_int64b.get_field()->get_string();
		_tests->test({ "copy dt", result, __FILE__, __LINE__ });
		delete test_copy;

		//double
		l = test_doubleb.get_total_size();
		c = test_doubleb.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copydb(c, 0);
		result = copydb.get_field()->get_double() == test_doubleb.get_field()->get_double();
		_tests->test({ "copy dbl", result, __FILE__, __LINE__ });
		delete test_copy;

		//int64
		l = test_int64b.get_total_size();
		c = test_int64b.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copyi(c, 0);
		result = copyi.get_field()->get_int64() == test_int64b.get_field()->get_int64();
		_tests->test({ "copy i64", result, __FILE__, __LINE__ });
		delete test_copy;

		// string
		l = test_stringb.get_total_size();
		c = test_stringb.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copys(c, l);
		result = copys.get_field()->get_string() == test_int64b.get_field()->get_string();
		_tests->test({ "copy str", result, __FILE__, __LINE__ });
		delete test_copy;

		// comparisons, three sets, these first should all be true

		result = test_datetimea < test_datetimeb;
		_tests->test({ "< dt", result, __FILE__, __LINE__ });

		result = test_doublea < test_doubleb;
		_tests->test({ "< dbl", result, __FILE__, __LINE__ });

		result = test_int64a < test_int64b;
		_tests->test({ "< i64", result, __FILE__, __LINE__ });

		result = test_stringa < test_stringb;
		_tests->test({ "< string", result, __FILE__, __LINE__ });

		// these are the opposites and we expect them to be false
		result = not test_datetimeb < test_datetimea;
		_tests->test({ "< dt 2", result, __FILE__, __LINE__ });

		result = not test_doubleb < test_doublea;
		_tests->test({ "< dbl 2", result, __FILE__, __LINE__ });

		result = not test_int64b < test_int64a;
		_tests->test({ "< i64 2", result, __FILE__, __LINE__ });

		result = not test_stringb < test_stringa;
		_tests->test({ "< string 2", result, __FILE__, __LINE__ });

		// these are the equals and we expect them to be false
		// and so if all are good we should have decent strong weak ordering
		result = not test_datetimeb < test_datetimeb;
		_tests->test({ "< dt 3", result, __FILE__, __LINE__ });

		result = not test_doubleb < test_doubleb;
		_tests->test({ "< dbl 3", result, __FILE__, __LINE__ });

		result = not test_int64b < test_int64b;
		_tests->test({ "< i64 3", result, __FILE__, __LINE__ });

		result = not test_stringb < test_stringb;
		_tests->test({ "< string 3", result, __FILE__, __LINE__ });

		// and we want to test these operators out

		result = test_datetimeb > test_datetimea;
		_tests->test({ "> dt 2", result, __FILE__, __LINE__ });

		result = test_doubleb > test_doublea;
		_tests->test({ "> dbl 2", result, __FILE__, __LINE__ });

		result = test_int64b > test_int64a;
		_tests->test({ "> i64 2", result, __FILE__, __LINE__ });

		result = test_stringb > test_stringa;
		_tests->test({ "> string 2", result, __FILE__, __LINE__ });


		// and this one

		result = test_datetimea != test_datetimeb;
		_tests->test({ "!= dt 2", result, __FILE__, __LINE__ });

		result = test_doublea != test_doubleb;
		_tests->test({ "!= dbl 2", result, __FILE__, __LINE__ });

		result = test_int64a != test_int64b;
		_tests->test({ "!= i64 2", result, __FILE__, __LINE__ });

		result = test_stringa != test_stringb;
		_tests->test({ "!= string 2", result, __FILE__, __LINE__ });

		// and this one

		result = test_datetimea == test_datetimea;
		_tests->test({ "== dt 2", result, __FILE__, __LINE__ });

		result = test_doublea == test_doublea;
		_tests->test({ "== dbl 2", result, __FILE__, __LINE__ });

		result = test_int64a == test_int64a;
		_tests->test({ "== i64 2", result, __FILE__, __LINE__ });

		result = test_stringa == test_stringa;
		_tests->test({ "== string 2", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("xfield", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	enum xblock_types 
	{
		xb_none = 0,
		xb_branch = 1,
		xb_leaf = 2
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

		xrecord(const char* _src, int _length)
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
			std::swap(key, _src.key);
		}

		xrecord& operator = (xrecord&& _src)
		{
			std::swap(key, _src.key);
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
					new_key = xfield_holder(m.get_string());
					break;
				case field_types::ft_double:
					new_key = xfield_holder(m.get_double());
					break;
				case field_types::ft_datetime:
					new_key = xfield_holder(m.get_datetime());
					break;
				case field_types::ft_int64:
					new_key = xfield_holder(m.get_int64());
					break;
				default:
					throw std::logic_error("Only use string, double, datetime and int64 for index keys");
					break;
				}
				key.insert(key.end(), (char *)new_key.get_field(), (char*)new_key.get_field() + new_key.get_total_size());
			}
		}
		
		void get_json(json& _dest, std::vector<std::string>& _keys)
		{
			int index = 0;

			int this_offset = 0;
			xfield_holder this_key;

			this_key = get_field(this_offset, &this_offset);
			while (this_key and index < _keys.size())
			{
				std::string field_name = _keys[index];
				switch (this_key.get_field()->data_type)
				{
				case field_types::ft_string:
					_dest.put_member(field_name, this_key.get_field()->get_string());
					break;
				case field_types::ft_double:
					_dest.put_member(field_name, this_key.get_field()->get_double());
					break;
				case field_types::ft_datetime:
					_dest.put_member(field_name, this_key.get_field()->get_datetime());
					break;
				case field_types::ft_int64:
					_dest.put_member_i64(field_name, this_key.get_field()->get_int64());
					break;
				default:
					throw std::logic_error("Only use string, double, datetime and int64 for index keys");
					break;
				}
				this_key = get_field(this_offset, &this_offset);
				index++;
			}
		}
		
		xblock_ref get_xblock_ref()
		{
			xblock_ref result = { xblock_types::xb_none, null_row };
			int next_offset;
			xfield_holder xfkey;
			xfkey = get_field(0, &next_offset);
			if (xfkey and xfkey.get_field()->data_type == field_types::ft_int64) {
				result.location = xfkey.get_field()->get_int64();
			}
			xfkey = get_field(next_offset, &next_offset);
			if (xfkey and xfkey.get_field()->data_type == field_types::ft_int64) {
				result.block_type = (xblock_types)xfkey.get_field()->get_int64();
			}
			return result;
		}

		void put_xblock_ref(xblock_types _table_type, data_block& db)
		{
			clear();
			add( db.header.block_location );
			add( (int64_t)_table_type );
		}

		void clear()
		{
			key.clear();
		}
		
		xrecord& add(double _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char *)new_key.get_field(), (char*)new_key.get_field() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(std::string _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_field(), (char*)new_key.get_field() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(date_time _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_field(), (char*)new_key.get_field() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(int64_t _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_field(), (char*)new_key.get_field() + new_key.get_total_size());
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

		xfield_holder get_field(int _offset, int* _next_offset) const
		{
			xfield_holder t = {};
			if (_offset < key.size()) {
				t = xfield_holder((char *)key.data(), _offset);
				*_next_offset = t.get_total_size() + _offset;
			}
			return t;
		}

		bool operator == (const xrecord& _other) const
		{
			int this_offset = 0;
			int other_offset = 0;
			xfield_holder this_key;
			xfield_holder other_key;

			other_key = _other.get_field(other_offset, &other_offset);
			this_key = get_field(this_offset, &this_offset);
			while (this_key and other_key)
			{
				if (this_key != other_key)
				{
					return false;
				}
				other_key = _other.get_field(other_offset, &other_offset);
				this_key = get_field(this_offset, &this_offset);
			}

			return true;
		}

		bool operator < (const xrecord& _other) const
		{
			int this_offset = 0;
			int other_offset = 0;
			xfield_holder this_key;
			xfield_holder other_key;
			other_key = _other.get_field(other_offset, &other_offset);
			this_key = get_field(this_offset, &this_offset);

			while (this_key and other_key)
			{
				if (this_key < other_key)
				{
					return true;
				}
				other_key = _other.get_field(other_offset, &other_offset);
				this_key = get_field(this_offset, &this_offset);
			}

			return false;
		}
	};

	void test_xrecord(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		xrecord comp1, comp2, comp3;
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xrecord", "start", start, __FILE__, __LINE__);

		comp1.add(4.0);
		comp1.add("hello");
		comp1.add(42i64);

		comp2.add(4.0);
		comp2.add("hello");
		comp2.add(42i64);

		bool result;
		result = comp1 == comp2;
		_tests->test({ "xr =", result, __FILE__, __LINE__ });

		comp3.add(2.0);
		result = comp3 < comp1;
		_tests->test({ "xr <", result, __FILE__, __LINE__ });

		// partial keys are equal.
		comp3.clear();
		comp3.add(4.0);
		result = comp3 == comp1;
		_tests->test({ "xr == key 1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = comp3 == comp1;
		_tests->test({ "xr == key 2", result, __FILE__, __LINE__ });

		comp3.add(42i64);
		result = comp3 == comp1;
		_tests->test({ "xr == key 3", result, __FILE__, __LINE__ });

		comp3.clear();
		comp3.add(2.0);
		result = comp3 < comp1;
		_tests->test({ "xr < key 1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 2", result, __FILE__, __LINE__ });

		comp3.add(42i64);
		result = comp3 < comp1;
		_tests->test({ "xr < key 3", result, __FILE__, __LINE__ });

		comp3.clear();
		comp3.add(4.0);
		result = not (comp3 < comp1);
		_tests->test({ "xr < key 2.1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = not(comp3 < comp1);
		_tests->test({ "xr < key 2.2", result, __FILE__, __LINE__ });

		comp3.add(43i64);
		result = not(comp3 < comp1);
		_tests->test({ "xr < key 2.3", result, __FILE__, __LINE__ });

		comp3.clear();
		comp3.add(4.0);
		comp3.add("ahello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 3", result, __FILE__, __LINE__ });

		// keys, more partial tests

		// partial keys are equal.
		comp3.clear();
		comp3.add(4.0);
		result = comp3 == comp1;
		_tests->test({ "xr == key 1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = comp3 == comp1;
		_tests->test({ "xr == key 2", result, __FILE__, __LINE__ });

		comp3.add(42i64);
		result = comp3 == comp1;
		_tests->test({ "xr == key 3", result, __FILE__, __LINE__ });

		comp3.clear();
		comp3.add(2.0);
		result = not (comp3 == comp1);
		_tests->test({ "xr == key 1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = not (comp3 == comp1);
		_tests->test({ "xr == key 2", result, __FILE__, __LINE__ });

		comp3.add(42i64);
		result = not (comp3 == comp1);
		_tests->test({ "xr == key 3", result, __FILE__, __LINE__ });

		// and now json tests.
		json_parser jp;
		json jsrc = jp.create_object();
		jsrc.put_member("Name", "Bill");
		jsrc.put_member("Age", "41");
		jsrc.put_member_i64("Atoms", 124);
		jsrc.put_member("Today", date_time::now());

		std::vector<std::string> keys = { "Name", "Age", "Atoms", "Today" };

		xrecord compj(keys, jsrc);
		json jdst = jp.create_object();
		compj.get_json(jdst, keys);
		
		result = (std::string)jsrc["Name"] == (std::string)jdst["Name"];
		_tests->test({ "rt name", result, __FILE__, __LINE__ });

		result = (double)jsrc["Age"] == (double)jdst["Age"];
		_tests->test({ "rt age", result, __FILE__, __LINE__ });

		result = (_int64)jsrc["Atoms"] == (_int64)jdst["Atoms"];
		_tests->test({ "rt atoms", result, __FILE__, __LINE__ });

		result = (date_time)jsrc["Today"] == (date_time)jdst["Today"];
		_tests->test({ "rt today", result, __FILE__, __LINE__ });

		xblock_ref tst_ref = { xblock_types::xb_none, null_row };
		xrecord copy, readin;
		compj.clear();
		poco_node<rectangle> block;
		block.header.block_location = 122;
		compj.put_xblock_ref(xblock_types::xb_branch, block);
		copy = compj;
		tst_ref = compj.get_xblock_ref();
		result = (tst_ref.location == 122);
		_tests->test({ "rt loc", result, __FILE__, __LINE__ });
		result = (tst_ref.block_type == xblock_types::xb_branch);
		_tests->test({ "rt type", result, __FILE__, __LINE__ });
		tst_ref = copy.get_xblock_ref();
		result = (tst_ref.location == 122);
		_tests->test({ "rt loc cp", result, __FILE__, __LINE__ });
		result = (tst_ref.block_type == xblock_types::xb_branch);
		_tests->test({ "rt type cp", result, __FILE__, __LINE__ });


		// write the copy back to readin and see if the round trip works
		char* src;
		char* dest;
		int length;

		dest = copy.before_write(&length);
		src = readin.before_read(length);
		std::copy(dest, dest + length, src);
		tst_ref = readin.get_xblock_ref();
		result = (tst_ref.location == 122);
		_tests->test({ "rt locs", result, __FILE__, __LINE__ });
		result = (tst_ref.block_type == xblock_types::xb_branch);
		_tests->test({ "rt types", result, __FILE__, __LINE__ });
		tst_ref = copy.get_xblock_ref();
		result = (tst_ref.location == 122);
		_tests->test({ "rt locs cp", result, __FILE__, __LINE__ });
		result = (tst_ref.block_type == xblock_types::xb_branch);
		_tests->test({ "rt types cp", result, __FILE__, __LINE__ });

		// and with the json
		jdst = jp.create_object();
		xrecord xsrc(keys, jsrc);
		dest = xsrc.before_write(&length);
		src = readin.before_read(length);
		std::copy(dest, dest + length, src);
		readin.get_json(jdst, keys);

		// and finally, checking our matches after the round trip
		result = (std::string)jsrc["Name"] == (std::string)jdst["Name"];
		_tests->test({ "rts name", result, __FILE__, __LINE__ });

		result = (double)jsrc["Age"] == (double)jdst["Age"];
		_tests->test({ "rts age", result, __FILE__, __LINE__ });

		result = (_int64)jsrc["Atoms"] == (_int64)jdst["Atoms"];
		_tests->test({ "rts atoms", result, __FILE__, __LINE__ });

		result = (date_time)jsrc["Today"] == (date_time)jdst["Today"];
		_tests->test({ "rts today", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("xrecord", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	struct xrecord_block_header
	{
		xblock_types							type;
		int										capacity;
		bool									dirty;
	};

	class xrecord_block : public data_block
	{
	protected:

		xrecord_block_header								header;
		std::map<xrecord, xrecord>							records;
		std::vector<char>									bytes;
		std::map<int64_t, std::shared_ptr<xrecord_block>>	child_cache;
		file_block		*									fb;

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

		xrecord_block(file_block *_fb, xrecord_block_header& _src)
		{
			fb = _fb;
			header.capacity = _src.capacity;
			header.dirty = _src.dirty;
			header.type = _src.type;
		}

		xblock_ref get_reference()
		{
			xblock_ref ref;
			ref.block_type = header.type;
			ref.location = data_block::header.block_location;
			return ref;
		}

		bool get_dirty()
		{
			return header.dirty;
		}

		void set_dirty(bool _dirty)
		{
			header.dirty = false;
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
			header.dirty = true;
			records.insert_or_assign(key, value);

			// then, we check to see if we have to split the block

			if (records.size() >= header.capacity)
			{
				on_full();
			}
		}

		virtual xrecord get(const xrecord& key)
		{
			xrecord temp;
			auto ri = records.find(key);
			if (ri != records.end()) {
				temp = ri->second;
			}
			return temp;
		}

		virtual void erase(const xrecord& key)
		{
			records.erase(key);
		}

		virtual xfor_each_result for_each(xrecord _key, std::function<relative_ptr_type(int _index, xrecord& _key, xrecord& _value)> _process)
		{
			xfor_each_result result;
			auto it = records.lower_bound(_key);
			int index = 0;
			while (it != records.end() and _key == it->first) {
				xrecord it_temp = it->first;
				relative_ptr_type t = _process(index, it_temp, it->second);
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

		virtual std::vector<xrecord> select(xrecord _key, std::function<xrecord(int _index, xrecord& _key, xrecord& _value)> _process)
		{
			std::vector<xrecord> results;
			auto it = records.lower_bound(_key);
			int index = 0;
			while (it != records.end() and _key == it->first) {
				xrecord it_temp = it->first;
				xrecord t = _process(index, it_temp, it->second);
				if (not t.is_empty())
				{
					results.push_back(t);
					index++;
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
			header = *((xrecord_block_header*)_bytes);
			_bytes += sizeof(xrecord_block_header);
			xblock_record_list* record_list = (xblock_record_list*)_bytes;
			for (int i = 0; i < record_list->count; i++)
			{
				xblock_record_list::xblock_ref* rl = &record_list->offsets[i];
				char* pdata = bytes.data();
				xrecord k(pdata + rl->key_offset, rl->key_size); // just deserializing the records.
				xrecord v(pdata + rl->value_offset, rl->value_size); // just deserializing the records.
				records.insert_or_assign(k, v);
			}
			header.dirty = false;
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
			header_bytes = sizeof(xrecord_block_header) + sizeof(xblock_record_list) + sizeof(xblock_record_list::xblock_ref) * count;
			total_bytes += header_bytes;
			*_size = total_bytes;
			bytes.resize(total_bytes);

			char* base = bytes.data();
			char* current = base;
			xrecord_block_header* hdr = (xrecord_block_header*)current;
			*hdr = header;
			current += sizeof(xrecord_block_header);
			xblock_record_list* record_list = (xblock_record_list*)current;
			current += header_bytes;

			int i = 0;
			for (auto& r : records)
			{
				auto* rl = &record_list->offsets[i];
				xrecord rkey = r.first;
				xrecord& skey = r.second;
				rl->key_size = rkey.size();
				rl->key_offset = current - base;
				int size_actual;
				char *rsrc = rkey.before_write(&size_actual);
				std::copy(rsrc, rsrc + size_actual, current);
				current += rl->key_size;

				rl->value_size = skey.size();
				rl->value_offset = current - base;
				char *vsrc = skey.before_write(&size_actual);
				std::copy(vsrc, vsrc + size_actual, current);
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

			int count = 0;

			for (auto& kv : records)
			{
				if (count > rsz) {
					keys_to_delete.push_back( kv.first );
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

		xblock_branch(file_block *_fb, xrecord_block_header& _header, express_table_interface* _table) :
			table(_table),
			parent(nullptr),
			xrecord_block(_fb, _header)
		{
			;
		}

		xblock_branch(file_block *_fb, xrecord_block_header& _header, xblock_branch *_parent) :
			table(nullptr),
			parent(_parent),
			xrecord_block(_fb, _header)
		{
			;
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
			;
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

		std::shared_ptr<xrecord_block> create_block(xrecord_block_header& _header);
		std::shared_ptr<xrecord_block> get_block(xblock_ref& _ref);
		std::shared_ptr<xrecord_block> find_block(const xrecord& key);

		virtual void put(const xrecord& key, xrecord& value)
		{
			
		}

		virtual xrecord get(const xrecord& key)
		{
			xrecord temp;
			return temp;
		}

		virtual void erase(const xrecord& key)
		{
			records.erase(key);
		}

		virtual xfor_each_result for_each(xrecord _key, std::function<relative_ptr_type(int _index, xrecord& _key, xrecord& _value)> _process)
		{
			xfor_each_result result;
			auto it = records.lower_bound(_key);
			int index = 0;
			while (it != records.end() and _key == it->first) {
				xrecord it_temp = it->first;
				relative_ptr_type t = _process(index, it_temp, it->second);
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

		virtual std::vector<xrecord> select(xrecord _key, std::function<xrecord(int _index, xrecord& _key, xrecord& _value)> _process)
		{
			std::vector<xrecord> results;
			auto it = records.lower_bound(_key);
			int index = 0;
			while (it != records.end() and _key == it->first) {
				xrecord it_temp = it->first;
				xrecord t = _process(index, it_temp, it->second);
				if (not t.is_empty())
				{
					results.push_back(t);
					index++;
				}
				it++;
			}
			return results;
		}

	};

	class xblock_leaf: public xrecord_block
	{
	public:

		xblock_branch* parent;

		xblock_leaf(file_block *_fb, xrecord_block_header& _header, xblock_branch* _parent) 
			: xrecord_block(_fb, _header), 
			parent(_parent)
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
		int capacity;

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			_dest.put_member_i64("self_location", self_location);
			_dest.put_member_i64("root_location", root_location);
			_dest.put_member_i64("capacity", capacity);
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
			capacity = (int64_t)_src["capacity"];
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
		const int								block_capacity = 1000;
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
			table_header->capacity = block_capacity;
			root = std::make_shared<xblock_branch>(this, nullptr, table_header->capacity);
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

		virtual xfor_each_result for_each(json _object, std::function<relative_ptr_type(json& _item)> _process) override
		{
			xrecord key(table_header->key_members, _object);
			root->for_each(key, [_process, this](int _index, xrecord& _key, xrecord& _data)->relative_ptr_type {
				json_parser jp;
				json obj = jp.create_object();
				_key.get_json(obj, table_header->key_members);
				_data.get_json(obj, table_header->object_members);
				return _process(obj);
				});
		}

		virtual json select(json _object, std::function<json(json& _item)> _process) override
		{
			xrecord key(table_header->key_members, _object);
			json_parser jp;
			json target = jp.create_array();
			root->select(key, [_process, this, &target](int _index, xrecord& _key, xrecord& _data)->xrecord {
				json_parser jp;
				json obj = jp.create_object();
				_key.get_json(obj, table_header->key_members);
				_data.get_json(obj, table_header->object_members);
				json jresult = _process(obj);
				if (jresult.object()) {
					target.push_back(jresult);
				}
				return _key;
				});
			return target;
		}

		virtual void on_split(xrecord_block_ptr _left_root_block, xrecord_block_ptr _right_root_block)
		{
			;
		}
	};

	std::shared_ptr<xrecord_block> xblock_branch::create_block(xrecord_block_header& _header)
	{
		std::shared_ptr<xrecord_block> result;
		switch (_header.type) {
		case xblock_types::xb_branch:
			result = std::make_shared<xblock_branch>(table, parent, _header.capacity);
			break;
		case xblock_types::xb_leaf:
			result = std::make_shared<xblock_leaf>(table, parent, _header.capacity);
			break;
		}
		return result;
	}

	// you're getting an xblock ref and you have to read an a result.
	std::shared_ptr<xrecord_block> xblock_branch::get_block(xblock_ref& _ref)
	{
		std::shared_ptr<xrecord_block> result;
		auto itrcached = child_cache.find(_ref.location);
		if (itrcached != child_cache.end()) {
			return itrcached->second;
		}

		xrecord_block_header hbr;
		hbr.capacity = header.capacity;
		hbr.dirty = false;
		hbr.type = _ref.block_type;
		switch (_ref.block_type) {
		case xblock_types::xb_branch:			
			result = std::make_shared<xblock_branch>(fb, hbr, parent);
			break;
		case xblock_types::xb_leaf:
			result = std::make_shared<xblock_leaf>(fb, hbr, parent);
			break;
		}
		if (_ref.location != null_row) {
			result->read(fb, _ref.location);
		}
		else {
			result->append(fb);
		}
		child_cache.insert_or_assign(_ref.location, result);
		return result;
	}

	std::shared_ptr<xrecord_block> xblock_branch::find_block(const xrecord& key)
	{

		std::shared_ptr<xrecord_block> return_block;

		auto ifirst = records.lower_bound(key);
		if (ifirst != std::end(records)) {
			auto& iftable = ifirst->second;
			return_block
		}
		auto irecord = records.rbegin();
		if (irecord != std::rend(records)) {
			auto& iftable = ifirst->second;
		}

		xblock_ref new_table;

		xrecord new_table;

	}

}

#endif

