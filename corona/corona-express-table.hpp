
#pragma once

#ifndef CORONA_EXPRESS_TABLE_H
#define CORONA_EXPRESS_TABLE_H

const bool debug_xblock = false;

namespace corona
{

	enum xblock_types
	{
		xb_none = 0,
		xb_branch = 1,
		xb_leaf = 2,
		xb_record = 3
	};

	template <typename storable_type>
	concept xblock_storable = requires(storable_type a, size_t b, const char* src, size_t length)
	{
		{ a.size() == b };
		{ a.data() == src };
		{ storable_type(src, length) };
	};

	struct xblock_ref
	{
	public:

		xblock_types		block_type;
		relative_ptr_type	location;

		xblock_ref()
		{
			block_type = xblock_types::xb_none;
			location = -1;
		}

		xblock_ref(xblock_types _block_type, relative_ptr_type _location)
		{
			block_type = _block_type;
			location = _location;
		}

		xblock_ref(const char* _src, size_t _length)
		{
			xblock_ref* src = (xblock_ref*)_src;
			block_type = src->block_type;
			location = src->location;
		}

		size_t size() { return sizeof(this); }
		const char* data() { return (const char*)this; }
	};

	class xfor_each_result
	{
	public:
		bool is_any;
		bool is_all;
		int64_t count;
	};


	class xtable_interface
	{
	public:

		virtual relative_ptr_type get_location() = 0;
		virtual json get(json _object) = 0;
		virtual json get(int64_t _object_id) = 0;
		virtual void put(json _object, bool _save) = 0;
		virtual void put_array(json _object) = 0;
		virtual void erase(json _object, bool _save) = 0;
		virtual void erase(int64_t _object_id, bool _save) = 0;
		virtual void erase_array(json _object) = 0;
		virtual xfor_each_result for_each(json _object, std::function<relative_ptr_type(json& _item)> _process) = 0;
		virtual json select(json _object, std::function<json(json& _item)> _process) = 0;
		virtual void clear() = 0;
		virtual void save() = 0;

	};


	struct xstring 
	{
		field_types ft;
		int length;
		char data[1];

		int total_size()
		{
			return sizeof(xstring) + length;
		}
		static char* from(std::string _src)
		{
			int sz = sizeof(xstring) + _src.size() + 1;
			char* t = new char[sz];
			xstring* xt = (xstring*)t;
			xt->ft = field_types::ft_string;
			xt->length = _src.size() + 1;
			std::copy(_src.c_str(), _src.c_str() + xt->length, xt->data);
			return t;
		}
	};

	struct xdouble 
	{
		field_types ft;
		double data;

		int total_size()
		{
			return sizeof(xdouble);
		}
		static char* from(double _src)
		{
			int sz = sizeof(xdouble);
			char* t = new char[sz];
			xdouble* xt = (xdouble*)t;
			xt->ft = field_types::ft_double;
			xt->data = _src;
			return t;
		}
	};

	struct xdatetime 
	{
		field_types ft;
		date_time data;

		int total_size()
		{
			return sizeof(xdatetime);
		}
		static char* from(date_time _src)
		{
			int sz = sizeof(xdatetime);
			char* t = new char[sz];
			xdatetime* xt = (xdatetime*)t;
			xt->ft = field_types::ft_datetime;
			xt->data = _src;
			return t;
		}
	};

	struct xint64_t
	{
		field_types ft;
		int64_t data;

		int total_size()
		{
			return sizeof(xint64_t);
		}
		static char* from(int64_t _src)
		{
			int sz = sizeof(xint64_t);
			char* t = new char[sz];
			xint64_t* xt = (xint64_t*)t;
			xt->ft = field_types::ft_int64;
			xt->data = _src;
			return t;
		}
	};

	struct xplaceholder
	{
		field_types ft;

		int total_size()
		{
			return sizeof(xplaceholder);
		}
		static char* from()
		{
			int sz = sizeof(xplaceholder);
			char* t = new char[sz];
			xplaceholder* xt = (xplaceholder*)t;
			xt->ft = field_types::ft_placeholder;
			return t;
		}
	};

	template <typename typea, typename typeb> 
	bool xfn_lt(void *_itema, void *_itemb)
	{
		return ((typea *)(_itema))->data < ((typea*)(_itemb))->data;
	}

	template <typename typea, typename typeb>
	bool xfn_eq(void* _itema, void* _itemb)
	{
		return ((typea*)(_itema))->data == ((typea*)(_itemb))->data;
	}

	template <typename typea, typename typeb>
	bool xfn_neq(void* _itema, void* _itemb)
	{
		return ((typea*)(_itema))->data != ((typea*)(_itemb))->data;
	}

	template <typename typea, typename typeb>
	bool xfn_gt(void* _itema, void* _itemb)
	{
		return ((typea*)(_itema))->data > ((typea*)(_itemb))->data;
	}

	template <>
	bool xfn_lt<xstring, xstring>(void* _itema, void* _itemb)
	{
		return strcmp( ((xstring *)(_itema))->data, ((xstring*)(_itemb))->data) < 0;
	}

	template <>
	bool xfn_eq<xstring, xstring>(void* _itema, void* _itemb)
	{
		return strcmp(((xstring*)(_itema))->data, ((xstring*)(_itemb))->data) == 0;
	}

	template <>
	bool xfn_neq<xstring, xstring>(void* _itema, void* _itemb)
	{
		return strcmp(((xstring*)(_itema))->data, ((xstring*)(_itemb))->data) != 0;
	}

	template <>
	bool xfn_gt<xstring, xstring>(void* _itema, void* _itemb)
	{
		return strcmp(((xstring*)(_itema))->data, ((xstring*)(_itemb))->data) > 0;
	}

	template <bool value>
	bool xfn_fixed(void* _itema, void* _itemb)
	{
		return value;
	}

	using xbinary_fn = std::function<bool(void* _a, void* _b)>;

	class xoperation_table
	{
	public:
		xbinary_fn eq[field_type_size][field_type_size];
		xbinary_fn lt[field_type_size][field_type_size];
		xbinary_fn gt[field_type_size][field_type_size];
		xbinary_fn neq[field_type_size][field_type_size];

		xoperation_table()
		{
			// eq
			for (int i = 0; i < field_type_size; i++)
			{
				for (int j = 0; j < field_type_size; j++)
				{
					eq[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
				}
			}
			for (int i = 0; i < field_type_size; i++)
			{
				eq[to_underlying(field_types::ft_placeholder)][i] = [](void* a, void* b)-> bool { return xfn_fixed<true>(a, b); };
				eq[i][to_underlying(field_types::ft_placeholder)] = [](void* a, void* b)-> bool { return xfn_fixed<true>(a, b); };
			}
			eq[to_underlying(field_types::ft_double)][to_underlying(field_types::ft_double)] = [](void* a, void* b)-> bool { return xfn_eq<xdouble, xdouble>(a, b); };
			eq[to_underlying(field_types::ft_int64)][to_underlying(field_types::ft_int64)] = [](void* a, void* b)-> bool { return xfn_eq<xint64_t, xint64_t>(a, b); };
			eq[to_underlying(field_types::ft_datetime)][to_underlying(field_types::ft_datetime)] = [](void* a, void* b)-> bool { return xfn_eq<xdatetime, xdatetime>(a, b); };
			eq[to_underlying(field_types::ft_string)][to_underlying(field_types::ft_string)] = [](void* a, void* b)-> bool { return xfn_eq<xstring, xstring>(a, b); };


			// neq
			for (int i = 0; i < field_type_size; i++)
			{
				for (int j = 0; j < field_type_size; j++)
				{
					neq[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<true>(a, b); };
				}
			}
			for (int i = 0; i < field_type_size; i++)
			{
				neq[to_underlying(field_types::ft_placeholder)][i] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
				neq[i][to_underlying(field_types::ft_placeholder)] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
			}
			neq[to_underlying(field_types::ft_double)][to_underlying(field_types::ft_double)] = [](void* a, void* b)-> bool { return xfn_neq<xdouble, xdouble>(a, b); };
			neq[to_underlying(field_types::ft_int64)][to_underlying(field_types::ft_int64)] = [](void* a, void* b)-> bool { return xfn_neq<xint64_t, xint64_t>(a, b); };
			neq[to_underlying(field_types::ft_datetime)][to_underlying(field_types::ft_datetime)] = [](void* a, void* b)-> bool { return xfn_neq<xdatetime, xdatetime>(a, b); };
			neq[to_underlying(field_types::ft_string)][to_underlying(field_types::ft_string)] = [](void* a, void* b)-> bool { return xfn_neq<xstring, xstring>(a, b); };

			// lt
			for (int i = 0; i < field_type_size; i++)
			{
				for (int j = 0; j < field_type_size; j++)
				{
					if (i < j) {
						lt[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<true>(a, b); };
					}
					else if (j > i)
					{
						lt[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
					}
				}
			}

			for (int i = 0; i < field_type_size; i++)
			{
				lt[to_underlying(field_types::ft_placeholder)][i] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
				lt[i] [to_underlying(field_types::ft_placeholder)] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
			}
			lt[to_underlying(field_types::ft_double)][to_underlying(field_types::ft_double)] = [](void* a, void* b)-> bool { return xfn_lt<xdouble, xdouble>(a, b); };
			lt[to_underlying(field_types::ft_int64)][to_underlying(field_types::ft_int64)] = [](void* a, void* b)-> bool { return xfn_lt<xint64_t, xint64_t>(a, b); };
			lt[to_underlying(field_types::ft_datetime)][to_underlying(field_types::ft_datetime)] = [](void* a, void* b)-> bool { return xfn_lt<xdatetime, xdatetime>(a, b); };
			lt[to_underlying(field_types::ft_string)][to_underlying(field_types::ft_string)] = [](void* a, void* b)-> bool { return xfn_lt<xstring, xstring>(a, b); };

			// gt
			for (int i = 0; i < field_type_size; i++)
			{
				for (int j = 0; j < field_type_size; j++)
				{
					if (i < j) {
						gt[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
					}
					else if (j > i)
					{
						gt[i][j] = [](void* a, void* b)-> bool { return xfn_fixed<true>(a, b); };
					}
				}
			}

			for (int i = 0; i < field_type_size; i++)
			{
				gt[to_underlying(field_types::ft_placeholder)][i] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
				gt[i][to_underlying(field_types::ft_placeholder)] = [](void* a, void* b)-> bool { return xfn_fixed<false>(a, b); };
			}
			gt[to_underlying(field_types::ft_double)][to_underlying(field_types::ft_double)] = [](void* a, void* b)-> bool { return xfn_gt<xdouble, xdouble>(a, b); };
			gt[to_underlying(field_types::ft_int64)][to_underlying(field_types::ft_int64)] = [](void* a, void* b)-> bool { return xfn_gt<xint64_t, xint64_t>(a, b); };
			gt[to_underlying(field_types::ft_datetime)][to_underlying(field_types::ft_datetime)] = [](void* a, void* b)-> bool { return xfn_gt<xdatetime, xdatetime>(a, b); };
			gt[to_underlying(field_types::ft_string)][to_underlying(field_types::ft_string)] = [](void* a, void* b)-> bool { return xfn_gt<xstring, xstring>(a, b); };
		}
	};

	class xfield_holder
	{

		// bytes is where placement operator new holder for key may sometimes be allocated.
		// however bytes can be null while key is not.  If bytes is null and key is not, then
		// that means key came from bytes someone else knows.  So we leave bytes null so the destructor
		// won't try and free them.

		char		*bytes;
		bool		free_bytes;
		int			size_bytes;

		field_types get_field_type() { return (field_types)(*bytes); }
		xstring* as_string() const { return (xstring*)bytes; }
		xdouble* as_double() const { return (xdouble*)bytes; }
		xint64_t* as_int64_t() const { return (xint64_t*)bytes; }
		xdatetime* as_datetime() const { return (xdatetime*)bytes; }
		xplaceholder* as_placeholder() const { return (xplaceholder*)bytes; }
	
		void from_bytes()
		{
			switch (get_field_type()) {
			case field_types::ft_string:
				size_bytes = as_string()->total_size();
				break;
			case field_types::ft_double:
				size_bytes = as_double()->total_size();
				break;
			case field_types::ft_datetime:
				size_bytes = as_datetime()->total_size();
				break;
			case field_types::ft_int64:
				size_bytes = as_int64_t()->total_size();
				break;
			case field_types::ft_placeholder:
				size_bytes = as_placeholder()->total_size();
				break;
			}
		}

		static xoperation_table* xops;

	public:

		xfield_holder()
		{
			size_bytes = 0;
			bytes = nullptr;
			free_bytes = false;
			if (xops == nullptr) {
				xops = new xoperation_table();
			}
		}

		xfield_holder(const xfield_holder& _src)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}
			if (_src.bytes) {
				size_bytes = _src.size_bytes;
				bytes = new char[size_bytes];
				free_bytes = true;
				std::copy(_src.bytes, _src.bytes + size_bytes, bytes);
			}
			else 
			{
				bytes = nullptr;
			}
		}

		xfield_holder(xfield_holder&& _src)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}
			bytes = _src.bytes;
			size_bytes = _src.size_bytes;
			free_bytes = _src.free_bytes;
			_src.free_bytes = false;
		}

		xfield_holder& operator =(const xfield_holder& _src)
		{
			if (free_bytes)
				delete[] bytes;
			bytes = nullptr;

			if (_src.bytes) {
				size_bytes = _src.size_bytes;
				bytes = new char[size_bytes];
				free_bytes = true;
				std::copy(_src.bytes, _src.bytes + size_bytes, bytes);
			}
			else
			{
				bytes = nullptr;
			}
			return *this;
		}

		xfield_holder& operator =(xfield_holder&& _src)
		{
			bytes = _src.bytes;
			size_bytes = _src.size_bytes;
			free_bytes = _src.free_bytes;
			_src.free_bytes = false;
			return *this;
		}

		xfield_holder(const char *_bytes, int _offset)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			free_bytes = false;
			bytes = (char *) &_bytes[_offset];

			field_types ft_data_type = get_data_type();

			switch (ft_data_type)
			{
			case field_types::ft_datetime:
				size_bytes = as_datetime()->total_size();
				break;
			case field_types::ft_int64:
				size_bytes = as_int64_t()->total_size();
				break;
			case field_types::ft_double:
				size_bytes = as_double()->total_size();
				break;
			case field_types::ft_placeholder:
				size_bytes = as_placeholder()->total_size();
				break;
			case field_types::ft_string:
				size_bytes = as_string()->total_size();
				break;
			default:
				size_bytes = 1;
				break;
			}
		}

		xfield_holder(const std::string& _data)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			bytes = xstring::from(_data);
			size_bytes = as_string()->total_size();
		}

		xfield_holder(int64_t _data)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			bytes = xint64_t::from(_data);
			size_bytes = as_int64_t()->total_size();
		}

		xfield_holder(double _data)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			bytes = xdouble::from(_data);
			size_bytes = as_double()->total_size();
		}

		xfield_holder(date_time _data)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			bytes = xdatetime::from(_data);
			size_bytes = as_datetime()->total_size();
		}

		xfield_holder(void* _dummy)
		{
			if (xops == nullptr) {
				xops = new xoperation_table();
			}

			bytes = xplaceholder::from();
			size_bytes = as_placeholder()->total_size();
		}

		field_types get_data_type() const
		{
			return (field_types)*bytes;
		}

		int get_total_size() const
		{
			return size_bytes;
		}

		inline double get_double() const
		{
			return as_double()->data;
		}

		inline int64_t get_int64() const
		{
			return as_int64_t()->data;
		}

		inline date_time get_datetime() const
		{
			return as_datetime()->data;
		}

		std::string get_string() const
		{
			return as_string()->data;
		}

		char *get_cstring() const
		{
			return as_string()->data;
		}

		char* get_bytes() const
		{
			return (char*)bytes;
		}

		operator bool() const
		{
			return bytes != nullptr;
		}

		bool operator < (const xfield_holder& _other) const
		{
			field_types ft_this, ft_other;
			ft_this = get_data_type();
			ft_other = _other.get_data_type();
			auto fn = xops->lt[to_underlying(ft_this)][to_underlying(ft_other)];
			return fn(bytes, _other.bytes);
		}

		bool operator == (const xfield_holder& _other) const
		{
			field_types ft_this, ft_other;
			ft_this = get_data_type();
			ft_other = _other.get_data_type();
			auto fn = xops->eq[to_underlying(ft_this)][to_underlying(ft_other)];
			return fn(bytes, _other.bytes);
		}

		bool operator > (const xfield_holder& _other) const
		{
			field_types ft_this, ft_other;
			ft_this = get_data_type();
			ft_other = _other.get_data_type();
			auto fn = xops->gt[to_underlying(ft_this)][to_underlying(ft_other)];
			return fn(bytes, _other.bytes);
		}

		bool operator != (const xfield_holder& _other) const
		{
			field_types ft_this, ft_other;
			ft_this = get_data_type();
			ft_other = _other.get_data_type();
			auto fn = xops->neq[to_underlying(ft_this)][to_underlying(ft_other)];
			return fn(bytes, _other.bytes);
		}

		~xfield_holder()
		{
			if (bytes and free_bytes) 
			{
				delete[] bytes;
				bytes = nullptr;
			}
		}
	
	};

	xoperation_table *xfield_holder::xops;

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

		result = test_datetimea.get_datetime() == testa;
		_tests->test({ "data dt", result, __FILE__, __LINE__ });

		result = test_doublea.get_double() == 42.0;
		_tests->test({ "data dble", result, __FILE__, __LINE__ });

		result = test_int64a.get_int64() == 12;
		_tests->test({ "data i64", result, __FILE__, __LINE__ });

		result = test_stringa.get_string() == "testa";
		_tests->test({ "data str", result, __FILE__, __LINE__ });

		result = test_datetimeb.get_datetime() == testb;
		_tests->test({ "data dt 2", result, __FILE__, __LINE__ });

		result = test_doubleb.get_double() == 52.0;
		_tests->test({ "data dble 2", result, __FILE__, __LINE__ });

		result = test_int64b.get_int64() == 22;
		_tests->test({ "data i64 2", result, __FILE__, __LINE__ });

		result = test_stringb.get_string() == "testb";
		_tests->test({ "data i64 2", result, __FILE__, __LINE__ });

		int l;
		char* c;
		char* test_copy;

		// these tests shouldn't leak

		//datetime
		l = test_datetimeb.get_total_size();
		c = (char*)test_datetimeb.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copydt(test_copy, 0);
		result = copydt.get_datetime() == test_datetimeb.get_datetime();
		_tests->test({ "copy dt", result, __FILE__, __LINE__ });
		delete test_copy;

		//double
		l = test_doubleb.get_total_size();
		c = test_doubleb.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copydb(test_copy, 0);
		result = copydb.get_double() == test_doubleb.get_double();
		_tests->test({ "copy dbl", result, __FILE__, __LINE__ });
		delete test_copy;

		//int64
		l = test_int64b.get_total_size();
		c = test_int64b.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copyi(test_copy, 0);
		result = copyi.get_int64() == test_int64b.get_int64();
		_tests->test({ "copy i64", result, __FILE__, __LINE__ });
		delete test_copy;

		// string
		l = test_stringb.get_total_size();
		c = test_stringb.get_bytes();
		test_copy = new char[l];
		std::copy(c, c + l, test_copy);

		xfield_holder copys(test_copy, 0);
		result = copys.get_string() == test_stringb.get_string();
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

	class xrecord
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

		// to satisfy xblock_storable

		xrecord(const char* _src, size_t _length)
		{
			key.insert(key.end(), _src, _src + _length);
		}

		size_t size() const
		{
			return key.size();
		}

		const char* data() const
		{
			return key.data();
		}

		// end implements xblock storable

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

		bool empty()
		{
			return key.size() == 0;
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
					new_key = xfield_holder((std::string)m);
					break;
				case field_types::ft_double:
					new_key = xfield_holder((double)m);
					break;
				case field_types::ft_datetime:
					new_key = xfield_holder((date_time)m);
					break;
				case field_types::ft_int64:
					new_key = xfield_holder((int64_t)m);
					break;
				default:
					new_key = xfield_holder(nullptr);
					break;
				}
				key.insert(key.end(), (char *)new_key.get_bytes(), (char*)new_key.get_bytes() + new_key.get_total_size());
			}
		}

		std::string to_string() const
		{
			std::stringstream output;

			int this_offset = 0;
			xfield_holder this_key;
			std::string separator = "";
			this_key = get_field(this_offset, &this_offset);
			while (this_key)
			{
				output << separator;
				switch (this_key.get_data_type())
				{
				case field_types::ft_string:
					output << this_key.get_string();
					break;
				case field_types::ft_double:
					output << this_key.get_double();
					break;
				case field_types::ft_datetime:
					output << (std::string)this_key.get_datetime();
					break;
				case field_types::ft_int64:
					output << this_key.get_int64();
					break;
				case field_types::ft_placeholder:
					output << "[null]";
				default:
					break;
				}
				this_key = get_field(this_offset, &this_offset);
				separator = ".";
			}

			std::string results = output.str();
			return results;
		}
		
		void get_json(json& _dest, std::vector<std::string>& _keys) const
		{
			int index = 0;

			int this_offset = 0;
			xfield_holder this_key;

			this_key = get_field(this_offset, &this_offset);
			while (this_key and index < _keys.size())
			{
				std::string field_name = _keys[index];
				switch (this_key.get_data_type())
				{
				case field_types::ft_string:
					_dest.put_member(field_name, this_key.get_string());
					break;
				case field_types::ft_double:
					_dest.put_member(field_name, this_key.get_double());
					break;
				case field_types::ft_datetime:
					_dest.put_member(field_name, this_key.get_datetime());
					break;
				case field_types::ft_int64:
					_dest.put_member_i64(field_name, this_key.get_int64());
					break;
				default:
					break;
				}
				this_key = get_field(this_offset, &this_offset);
				index++;
			}
		}
		
		xblock_ref get_xblock_ref()
		{
			xblock_ref result( xblock_types::xb_none, null_row );
			int next_offset;
			xfield_holder xfkey;
			xfkey = get_field(0, &next_offset);
			if (xfkey and xfkey.get_data_type() == field_types::ft_int64) {
				result.location = xfkey.get_int64();
			}
			xfkey = get_field(next_offset, &next_offset);
			if (xfkey and xfkey.get_data_type() == field_types::ft_int64) {
				result.block_type = (xblock_types)xfkey.get_int64();
			}
			return result;
		}

		void put_xblock_ref(const xblock_ref& ref)
		{
			clear();
			add( ref.location );
			add( (int64_t)ref.block_type );
		}

		void clear()
		{
			key.clear();
		}
		
		xrecord& add(double _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char *)new_key.get_bytes(), (char*)new_key.get_bytes() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(std::string _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_bytes(), (char*)new_key.get_bytes() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(date_time _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_bytes(), (char*)new_key.get_bytes() + new_key.get_total_size());
			return *this;
		}

		xrecord& add(int64_t _value)
		{
			xfield_holder new_key(_value);
			key.insert(key.end(), (char*)new_key.get_bytes(), (char*)new_key.get_bytes() + new_key.get_total_size());
			return *this;
		}
		
		bool is_empty()
		{
			return key.empty();
		}


		xfield_holder get_field(int _offset, int* _next_offset) const
		{
			xfield_holder t = {};
			if (_offset < key.size()) {
				const char* f = key.data();
				t = xfield_holder(f, _offset);
				*_next_offset = t.get_total_size() + _offset;
			}
			return t;
		}

		bool all_equal(const xrecord& _other) const
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

			if (not this_key and other_key)
				return false;
			else if (this_key and not other_key)
				return false;

			return true;
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
				else if (this_key > other_key) // this is necessary to ensure a < b and b < a are properly transitive.
				{
					return false;
				}
				other_key = _other.get_field(other_offset, &other_offset);
				this_key = get_field(this_offset, &this_offset);
			}

			if (this_key and not other_key)
				return false;

			if (not this_key and other_key)
				return true;

			return false;
		}
	};

	void test_xrecord(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xrecord", "start", start, __FILE__, __LINE__);

		xrecord comp1, comp2, comp3;

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
		result = comp3 < comp1;
		_tests->test({ "xr < key 2.1", result, __FILE__, __LINE__ });

		comp3.add("hello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 2.2", result, __FILE__, __LINE__ });

		comp3.add(43i64);
		result = not(comp3 < comp1);
		_tests->test({ "xr < key 2.3", result, __FILE__, __LINE__ });

		comp3.clear();
		comp3.add(4.0);
		comp3.add("ahello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 3.1", result, __FILE__, __LINE__ });

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

		xblock_ref tst_ref( xblock_types::xb_none, null_row );
		xrecord copy, readin;
		compj.clear();
		xblock_ref ref;
		ref.location = 122;
		ref.block_type = xblock_types::xb_branch;
		compj.put_xblock_ref(ref);
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

		readin = xrecord(copy.data(), copy.size());
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
		readin = xrecord(xsrc.data(), xsrc.size());
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

	static const int xrecords_per_block = 100;

	struct xblock_location
	{
		int32_t key_offset;
		int32_t key_size;
		int32_t value_offset;
		int32_t value_size;
	};

	struct xrecord_block_header
	{
		xblock_types							type;
		xblock_types							content_type;
		int										count;
		xblock_location							records[xrecords_per_block];

		xrecord_block_header()
		{
			count = 0;
		}

		xrecord_block_header(const char *_src, size_t _length)
		{
			set(_src, _length);
		}

		xrecord_block_header(xrecord_block_header&& _src) = default;
		xrecord_block_header(const xrecord_block_header& _src) = default;
		xrecord_block_header& operator = (xrecord_block_header&& _src) = default;
		xrecord_block_header& operator = (const xrecord_block_header& _src) = default;

		void set(const char* _src, size_t _length)
		{
			xrecord_block_header* src = (xrecord_block_header*)(_src);
			type = src->type;
			content_type = src->content_type;
			count = src->count;
			if (count > xrecords_per_block)
				throw std::logic_error("bad src count for header");
			std::copy(&src->records[0], &src->records[count], records);
		}

		size_t size() const
		{
			return sizeof(xrecord_block_header);
		}

		const char* data() const
		{
			return (const char*)this;
		}

	};

	template <typename data_type>
	requires xblock_storable<data_type>
	class xrecord_block : protected data_block
	{
	protected:

		shared_lockable locker;

		xrecord_block_header								xheader;
		std::map<xrecord, data_type>						records;
		file_block		*									fb;

		friend class xbranch_block;
		friend class xleaf_block;

		virtual xrecord get_start_key_nl()
		{
			xrecord key;
			auto max_key = records.begin();
			if (max_key != records.end())
			{
				key = max_key->first;
			}
			return key;
		}

		virtual xrecord get_end_key_nl()
		{
			xrecord key;
			auto max_key = records.crbegin();
			if (max_key != records.rend())
			{
				key = max_key->first;
			}
			return key;
		}

		void save_nl()
		{
			if (data_block::header.block_location >= 0) {
				write(fb);
			}
			else
			{
				append(fb);
			}
		}

	public:

		xrecord_block(file_block *_fb, xrecord_block_header& _src)
		{
			fb = _fb;
			xheader = _src;
			save_nl();
		}

		xrecord_block(file_block* _fb, int64_t _location)
		{
			if (_location == null_row) {
				throw std::invalid_argument("Invalid location");
			}
			fb = _fb;
			read(fb, _location);
		}

		virtual ~xrecord_block()
		{
		}

		xblock_ref get_reference()
		{
			read_scope_lock lockit(locker);

			xblock_ref ref;
			ref.block_type = xheader.type;
			ref.location = data_block::header.block_location;
			return ref;
		}


		xrecord get_start_key()
		{
			read_scope_lock lockit(locker);

			return get_start_key_nl();
		}

		xrecord get_end_key()
		{
			read_scope_lock lockit(locker);

			return get_end_key_nl();
		}

		virtual void release()
		{
			write_scope_lock lockit(locker);

			data_block::erase(fb);
		}


		bool is_full()
		{
			read_scope_lock lockit(locker);

			return xrecords_per_block <= records.size() + 1;
		}

		bool is_empty()
		{
			read_scope_lock lockit(locker);

			return records.size() == 0;
		}

		virtual void clear()
		{
			write_scope_lock lockit(locker);

			records.clear();
		}

		void save()
		{
			write_scope_lock lockit(locker);
			save_nl();
		}


	protected:

		virtual char* before_read(int32_t _size)  override
		{
			char *bytes = new char[_size + 10];
			return bytes;
		}

		virtual void after_read(char* _bytes) override
		{
			records.clear();
			xheader.set(_bytes, 0);
			for (int i = 0; i < xheader.count; i++)
			{
				auto& rl = xheader.records[i];
				xrecord k(_bytes + rl.key_offset, rl.key_size); // just deserializing the records.
				data_type v(_bytes + rl.value_offset, rl.value_size); // just deserializing the records.
				records.insert_or_assign(k, v);
			}
		}

		virtual char* before_write(int32_t* _size) override
		{
			size_t total_bytes = 0;
			size_t header_bytes = 0;
			int32_t count = 0;

			for (auto& r : records)
			{
				total_bytes += r.first.size();
				total_bytes += r.second.size();
				count++;
			}

			header_bytes = xheader.size();
			total_bytes += header_bytes;
			*_size = total_bytes;
			char *bytes = new char[total_bytes + 10];
			std::copy(xheader.data(), xheader.data() + header_bytes, bytes);

			char* current = bytes + header_bytes;

			int i = 0;
			for (auto& r : records)
			{
				auto& rl = xheader.records[i];
				int size_actual;

				rl.key_offset = current - bytes;
				rl.key_size = r.first.size();
				const char *rsrc = r.first.data();
				std::copy(rsrc, rsrc + rl.key_size, current);
				current += rl.key_size;

				rl.value_offset = current - bytes;
				rl.value_size = r.second.size();
				const char *vsrc = r.second.data();
				std::copy(vsrc, vsrc + rl.value_size, current);
				current += rl.value_size;

				i++;
			}

			return bytes;
		}

		virtual void after_write(char* _t) override
		{

		}

		virtual void finished_io(char* _bytes) override
		{
			if (_bytes)
				delete[] _bytes;
		}

	};

	class xleaf_block : public xrecord_block<xrecord>
	{
	public:

		xleaf_block(file_block* _fb, xrecord_block_header& _header)
			: xrecord_block(_fb, _header)
		{
			;
		}

		xleaf_block(file_block* _fb, xblock_ref _ref) : xrecord_block(_fb, _ref.location)
		{
			;
		}

		void put(int _indent, const xrecord& key, const xrecord& value)
		{
			write_scope_lock lockit(locker);

			if constexpr (debug_xblock) {
				std::string indent(_indent * 4, ' ');
				std::string message = std::format("{2} leaf:{0}, key:{1}", get_reference().location, key.to_string(), indent);
				system_monitoring_interface::global_mon->log_information(message, __FILE__, __LINE__);
			}

			records.insert_or_assign(key, value);
			save_nl();
		}

		xrecord get(const xrecord& key)
		{
			read_scope_lock lockit(locker);

			xrecord temp;
			auto ri = records.find(key);
			if (ri != records.end()) {
				temp = ri->second;
			}
			return temp;
		}

		void erase(const xrecord& key)
		{
			write_scope_lock lockit(locker);

			records.erase(key);
		}

		virtual xfor_each_result for_each(xrecord _key, std::function<relative_ptr_type(const xrecord& _key, const xrecord& _value)> _process)
		{
			xfor_each_result result;
			result.is_all = true;
			result.is_any = false;
			for (auto item : records) 
			{
				if (item.first == _key) {
					if (_process(item.first, item.second) != null_row)
					{
						result.count++;
						result.is_any = true;
					}
				} 
				else 
				{
					result.is_all = false;
				}
			}
			return result;
		}

		virtual std::vector<xrecord> select(xrecord _key, std::function<xrecord(const xrecord& _key, const xrecord& _value)> _process)
		{
			std::vector<xrecord> result = {};
			for (auto item : records)
			{
				if (item.first == _key) {
					xrecord temp = _process(item.first, item.second);
					if (not temp.is_empty())
					{
						result.push_back(temp);
					}
				}
			}
			return result;
		}

		json get_info()
		{
			json_parser jp;
			json result = jp.create_object();
			auto start_key = get_start_key();
			auto end_key = get_end_key();
			result.put_member("type", "leaf");
			result.put_member_i64("location", get_reference().location);
			result.put_member_i64("count", records.size());
			std::string starts = start_key.to_string();
			std::string ends = end_key.to_string();
			result.put_member("start", starts);
			result.put_member("end", ends);
			return result;
		}

	};

	class xbranch_block : public xrecord_block<xblock_ref>
	{
	private:
		std::shared_ptr<xleaf_block> create_leaf_block();
		std::shared_ptr<xbranch_block> create_branch_block(xblock_types _content_type);
		std::shared_ptr<xleaf_block> open_leaf_block(xblock_ref& _header);
		std::shared_ptr<xbranch_block> open_branch_block(xblock_ref& _header);
		xblock_ref find_block(const xrecord& key);


		virtual std::shared_ptr<xbranch_block> split_branch(std::shared_ptr<xbranch_block> _block, int _indent)
		{

			/************************************************

			split into children from this:

			Block A - Full [ 0........n ]

			to this

			Block A - [ 0...1/2n ]        Block B - [ 1/2n...n ]

			***********************************************/


			auto new_xb = create_branch_block(_block->xheader.content_type);

			int64_t rsz = records.size() / 2;

			// time to split the block
			std::vector<xrecord> keys_to_delete;

			int64_t count = 0;

			for (auto& kv : _block->records)
			{
				if (count > rsz) {
					keys_to_delete.push_back(kv.first);
					new_xb->records.insert_or_assign(kv.first, kv.second);
				}
				count++;
			}

			for (auto& kv : keys_to_delete)
			{
				_block->records.erase(kv);
			}

			new_xb->save();

			return new_xb;
		}


		virtual std::shared_ptr<xleaf_block> split_leaf(std::shared_ptr<xleaf_block> _block, int _indent)
		{

			/************************************************

			split into children from this:

			Block A - Full [ 0........n ]

			to this

			Block A - [ 0...1/2n ]        Block B - [ 1/2n...n ]

			***********************************************/


			auto new_xb = create_leaf_block();

			int64_t rsz = records.size() / 2;

			// time to split the block
			std::vector<xrecord> keys_to_delete;

			int64_t count = 0;

			for (auto& kv : _block->records)
			{
				if (count > rsz) {
					keys_to_delete.push_back(kv.first);
					new_xb->put(_indent, kv.first, kv.second);
				}
				count++;
			}

			for (auto& kv : keys_to_delete)
			{
				_block->records.erase(kv);
			}

			new_xb->save();

			return new_xb;
		}

	public:

		xbranch_block(file_block *_fb, xrecord_block_header& _header) :
			xrecord_block(_fb, _header)
		{
			;
		}

		xbranch_block(file_block* _fb, xblock_ref _ref) : xrecord_block(_fb, _ref.location)
		{
			;
		}

		void split_root(int _indent)
		{
			write_scope_lock lock_me(locker);
			/************************************************

			split into children from this:

			Block A - Full [ 0........n ]

			to this

				  Block A - [ Block B, Block C ]
						//                  \\
					   //                    \\
			Block B - [ 0...3/4n ]        Block C - [ 3/4n...n ]

			***********************************************/

			auto new_child1 = create_branch_block(xheader.content_type);
			auto new_child2 = create_branch_block(xheader.content_type);

			int64_t rsz = records.size() / 2i64;
			int64_t count = 0;

			for (auto r : records)
			{
				if (count < rsz)
				{
					new_child1->records.insert_or_assign(r.first, r.second);
				}
				else
				{
					new_child2->records.insert_or_assign(r.first, r.second);
				}
				count++;
			}

			new_child1->save();
			new_child2->save();

			xblock_ref child1_ref = new_child1->get_reference();
			xblock_ref child2_ref = new_child2->get_reference();

			// we find the keys for the new children so that we can insert
			// then right things into our map
			xrecord child1_key = new_child1->get_end_key_nl();
			xrecord child2_key = new_child2->get_end_key_nl();

			xheader.content_type = xblock_types::xb_branch;

			records.clear();
			records.insert_or_assign(child1_key, child1_ref);
			records.insert_or_assign(child2_key, child2_ref);
		}

		virtual void put(int _indent, const xrecord& _key, const xrecord& _value) 
		{
			write_scope_lock lock_me(locker);

			if constexpr (debug_xblock) {
				std::string indent(_indent * 4, ' ');
				std::string message = std::format("{2} branch:{0}, key:{1}", get_reference().location, _key.to_string(), indent);
				system_monitoring_interface::global_mon->log_information(message, __FILE__, __LINE__);
			}

			xblock_ref found_block = find_block(_key);

			if (found_block.block_type == xblock_types::xb_branch)
			{
				auto branch_block = open_branch_block(found_block);
				auto old_key = branch_block->get_end_key();
				branch_block->put(_indent, _key, _value);
				if (branch_block->is_full()) {
					auto new_branch = split_branch(branch_block, _indent);
					auto new_branch_key = new_branch->get_end_key();
					auto new_branch_ref = new_branch->get_reference();
					records.insert_or_assign(new_branch_key, new_branch_ref);
				}
				auto new_key = branch_block->get_end_key();
				if (old_key != new_key) {
					records.erase(old_key);
					records.insert_or_assign(new_key, found_block);
				}
			}
			else if (found_block.block_type == xblock_types::xb_leaf)
			{
				auto leaf_block = open_leaf_block(found_block);
				auto old_key = leaf_block->get_end_key();
				leaf_block->put(_indent, _key, _value);
				if (leaf_block->is_full()) {
					auto new_leaf = split_leaf(leaf_block, _indent);
					auto new_leaf_key = new_leaf->get_end_key();
					auto new_leaf_ref = new_leaf->get_reference();
					records.insert_or_assign(new_leaf_key, new_leaf_ref);
				}
				auto new_key = leaf_block->get_end_key();
				if (old_key != new_key) {
					records.erase(old_key);
					records.insert_or_assign(new_key, found_block);
				}
			}
			else if (found_block.block_type == xblock_types::xb_none)
			{
				xblock_ref new_ref;
				if (xheader.content_type == xblock_types::xb_leaf)
				{
					auto new_leaf = create_leaf_block();
					new_leaf->put(_indent + 1, _key, _value);
					new_ref = new_leaf->get_reference();
				}
				else if (xheader.content_type == xblock_types::xb_branch)
				{
					auto new_branch = create_branch_block(xblock_types::xb_branch);
					new_branch->put(_indent + 1, _key, _value);
					new_ref = new_branch->get_reference();
				}
				records.insert_or_assign(_key, new_ref);
			}

			save_nl();
		}

		virtual xrecord get(const xrecord& _key)
		{
			read_scope_lock lock_me(locker);

			xrecord result;
			xblock_ref found_block = find_block(_key);

			if (found_block.block_type == xblock_types::xb_branch)
			{
				auto branch_block = open_branch_block(found_block);
				result = branch_block->get(_key);
			}
			else if (found_block.block_type == xblock_types::xb_leaf)
			{
				auto leaf_block = open_leaf_block(found_block);
				result = leaf_block->get(_key);
			}
			return result;
		}

		virtual void erase(const xrecord& _key)
		{
			write_scope_lock lock_me(locker);

			xblock_ref found_block = find_block(_key);

			if (found_block.block_type == xblock_types::xb_branch)
			{
				auto branch_block = open_branch_block(found_block);
				branch_block->erase(_key);
			}
			else if (found_block.block_type == xblock_types::xb_leaf)
			{
				auto leaf_block = open_leaf_block(found_block);
				leaf_block->erase(_key);
			}
		}

		virtual void clear()
		{
			write_scope_lock lock_me(locker);

			for (auto item : records) {
				auto found_block = item.second;
				if (found_block.block_type == xblock_types::xb_branch)
				{
					auto branch_block = open_branch_block(found_block);
					branch_block->clear();
					branch_block->release();
				}
				else if (found_block.block_type == xblock_types::xb_leaf)
				{
					auto leaf_block = open_leaf_block(found_block);
					leaf_block->clear();
					leaf_block->release();
				}
			}
			records.clear();
			save_nl();
		}

		virtual xfor_each_result for_each(xrecord _key, std::function<relative_ptr_type(const xrecord& _key, const xrecord& _value)> _process)
		{
			read_scope_lock lock_me(locker);

			xfor_each_result result;
			result.is_all = false;
			result.is_any = false;
			for (auto item : records) {
				if (item.first == _key) {
					xfor_each_result temp;
					auto& found_block = item.second;
					if (found_block.block_type == xblock_types::xb_branch)
					{
						auto branch_block = open_branch_block(found_block);
						temp = branch_block->for_each(_key, _process);
					}
					else if (found_block.block_type == xblock_types::xb_leaf)
					{
						auto leaf_block = open_leaf_block(found_block);
						temp = leaf_block->for_each(_key, _process);
					}
					else
						temp = {};

					result.count += temp.count;
					if (temp.is_any)
						result.is_any = true;
					if (not temp.is_all)
						result.is_all = false;
				}
			}
			return result;
		}

		virtual std::vector<xrecord> select(xrecord _key, std::function<xrecord(const xrecord& _key, const xrecord& _value)> _process)
		{
			read_scope_lock lock_me(locker);

			std::vector<xrecord> result = {};
			for (auto item : records) {
				if (item.first == _key) {
					std::vector<xrecord> temp;
					auto& found_block = item.second;
					if (found_block.block_type == xblock_types::xb_branch)
					{
						auto branch_block = open_branch_block(found_block);
						temp = branch_block->select(_key, _process);
					}
					else if (found_block.block_type == xblock_types::xb_leaf)
					{
						auto leaf_block = open_leaf_block(found_block);
						temp = leaf_block->select(_key, _process);
					}
					else
						temp = {};

					result.insert(result.end(), temp.begin(), temp.end());
				}
			}
			return result;
		}

		json get_info()
		{
			read_scope_lock lockit(locker);

			json_parser jp;
			json result = jp.create_object();
			auto start_key = get_start_key();
			auto end_key = get_end_key();
			result.put_member("type", "branch");
			switch (xheader.content_type) {
			case xblock_types::xb_branch:
				result.put_member("content", "branch");
				break;
			case xblock_types::xb_leaf:
				result.put_member("content", "leaf");
				break;
			}
			result.put_member_i64("location", get_reference().location);
			result.put_member_i64("count", records.size());
			std::string starts = start_key.to_string();
			std::string ends = end_key.to_string();
			result.put_member("start", starts);
			result.put_member("end", ends);

			json children = jp.create_array();
			for (auto r : records) 
			{
				if (r.second.block_type == xblock_types::xb_leaf)
				{
					auto block = open_leaf_block(r.second);
					json info = block->get_info();
					children.push_back(info);
				}
				else if (r.second.block_type == xblock_types::xb_branch)
				{
					auto block = open_branch_block(r.second);
					json info = block->get_info();
					children.push_back(info);
				}
			}

			return result;
		}

	};


	class xtable_header : public data_block
	{
		std::string		  data;

	public:

		xblock_ref		  root_block;

		std::shared_ptr<xbranch_block> root;
		std::vector<std::string> key_members;
		std::vector<std::string> object_members;
		int64_t count;

		relative_ptr_type get_location()
		{
			return header.block_location;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			_dest.put_member_i64("root_type", root_block.block_type);
			_dest.put_member_i64("root_location", root_block.location);
			json kms = jp.create_array(key_members);
			_dest.put_member("key_members", kms);
			json oms = jp.create_array(object_members);
			_dest.put_member("object_members", oms); 
			_dest.put_member_i64("count", count);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			root_block.location = _src["root_location"];
			root_block.block_type = (xblock_types)((int64_t)_src["root_type"]);
			json kms = _src["key_members"];
			key_members = kms.to_string_array();
			json oms = _src["object_members"];
			object_members = oms.to_string_array();
			count = (int64_t)_src["count"];
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

	class xtable : public xtable_interface
	{

	public:
		std::string								data;
		file_block*								fb;
		std::shared_ptr<xtable_header>			table_header;

		shared_lockable							locker;

		xtable(file_block* _fb, std::shared_ptr<xtable_header> _header) :
			fb(_fb),
			table_header(_header)
		{
			xrecord_block_header new_header;
			new_header.content_type = xblock_types::xb_leaf;
			new_header.type = xblock_types::xb_branch;
			table_header->root = std::make_shared<xbranch_block>(fb, new_header);
			table_header->root_block = table_header->root->get_reference();
			table_header->append(fb);
		}

		xtable(file_block* _fb, int64_t _location) :
			fb(_fb)
		{
			if (_location == null_row) {
				throw std::invalid_argument("Invalid location");
			}
			table_header = std::make_shared<xtable_header>();
			table_header->read(fb, _location);

			table_header->root = std::make_shared<xbranch_block>(fb, table_header->root_block);
		}

		relative_ptr_type get_location() override
		{
			read_scope_lock lockme(locker);
			return table_header->get_location();
		}

		void save() override
		{
			write_scope_lock lockit(locker);
			table_header->root->save();
			table_header->write(fb);
		}

		virtual json get(int64_t _key) override
		{
			json_parser jp;
			json jresult;
			xrecord key;
			key.add(_key);
			xrecord result = table_header->root->get(key);
			if (not result.is_empty()) {
				jresult = jp.create_object();
				key.get_json(jresult, table_header->key_members);
				result.get_json(jresult, table_header->object_members);
				return jresult;
			}
			return jresult;
		}

		virtual json get(json _object) override
		{
			json_parser jp;
			json jresult;
			xrecord key(table_header->key_members, _object);
			xrecord result = table_header->root->get(key);
			if (not result.is_empty()) {
				jresult = jp.create_object();
				key.get_json(jresult, table_header->key_members);
				result.get_json(jresult, table_header->object_members);
				return jresult;
			}
			return jresult;
		}

		virtual void put(json _object, bool _save = true) override
		{
			xrecord key(table_header->key_members, _object);
			xrecord data(table_header->object_members, _object);			 
			table_header->root->put(0, key, data);
			::InterlockedIncrement64(&table_header->count);

			if (table_header->root->is_full()) {
				table_header->root->split_root(0);
			}
			if (_save) {
				save();
			}
		}

		virtual void put_array(json _array) override
		{
			if (_array.array()) {
				for (auto item : _array) {
					put(item, false);
				}
			}
			save();
		}

		virtual void erase(int64_t _id, bool _save = true) override
		{
			xrecord key;
			key.add(_id);
			table_header->root->erase(key);
			::InterlockedDecrement64(&table_header->count);
			if (_save) {
				save();
			}
		}

		virtual void erase(json _object, bool _save = true) override
		{
			xrecord key(table_header->key_members, _object);
			::InterlockedDecrement64(&table_header->count);
			table_header->root->erase(key);
			if (_save) {
				save();
			}
		}

		virtual void erase_array(json _array) override
		{
			if (_array.array()) {
				for (auto item : _array) {
					erase(item);
				}
			}
			save();
		}

		int64_t get_count()
		{
			return table_header->count;
		}
		
		json get_info()
		{
			return table_header->root->get_info();
		}

		virtual xfor_each_result for_each(json _object, std::function<relative_ptr_type(json& _item)> _process) override
		{
			
			xrecord key(table_header->key_members, _object);
			xfor_each_result result;
			result = table_header->root->for_each(key, [_process, this](const xrecord& _key, const xrecord& _data)->relative_ptr_type {
				json_parser jp;
				json obj = jp.create_object();
				_key.get_json(obj, table_header->key_members);
				_data.get_json(obj, table_header->object_members);
				return _process(obj);
				});
			return result;
		}

		virtual json select(json _object, std::function<json(json& _item)> _process) override
		{
			xrecord key(table_header->key_members, _object);
			json_parser jp;
			json target = jp.create_array();
			table_header->root->select(key, [_process, this, &target](const xrecord& _key, const xrecord& _data)->xrecord {
				json_parser jp;
				json obj = jp.create_object();
				xrecord empty;
				_key.get_json(obj, table_header->key_members);
				_data.get_json(obj, table_header->object_members);
				json jresult = _process(obj);
				if (jresult.object()) {
					target.push_back(jresult);
					return _key;
				}
				return empty;
				});
			return target;
		}

		virtual void clear() override
		{
			table_header->root->clear();
			save();
		}
	};

	std::shared_ptr<xleaf_block> xbranch_block::open_leaf_block(xblock_ref& _header)
	{
		return std::make_shared<xleaf_block>(fb, _header);
	}

	std::shared_ptr<xbranch_block> xbranch_block::open_branch_block(xblock_ref& _header)
	{
		return std::make_shared<xbranch_block>(fb, _header);
	}

	std::shared_ptr<xbranch_block> xbranch_block::create_branch_block(xblock_types _content_type)
	{
		xrecord_block_header header;

		header.type = xblock_types::xb_leaf;
		header.content_type = _content_type;

		auto result = std::make_shared<xbranch_block>(fb, header);
		return result;
	}

	std::shared_ptr<xleaf_block> xbranch_block::create_leaf_block()
	{
		xrecord_block_header header;

		header.type = xblock_types::xb_leaf;
		header.content_type = xblock_types::xb_record;

		auto result = std::make_shared<xleaf_block>(fb, header);
		return result;
	}

	xblock_ref xbranch_block::find_block(const xrecord& key)
	{
		read_scope_lock lockit(locker);

		xblock_ref found_block;
		found_block.block_type == xblock_types::xb_none;

		std::shared_ptr<xrecord_block> return_block;

		// taking the left end
		auto ifirst = records.upper_bound(key);
		if (ifirst != std::end(records)) {
			found_block = ifirst->second;
			return found_block;
		}

		return found_block;
	}

	void test_xleaf(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xleaf", "start", start, __FILE__, __LINE__);

		std::shared_ptr<file> fp = _app->open_file_ptr("test.cxdb", file_open_types::create_always);
		file_block fb(fp);

		xrecord_block_header xheader;
		xheader.type = xblock_types::xb_leaf;
		xheader.content_type = xblock_types::xb_record;
		std::shared_ptr<xleaf_block> pleaf = std::make_shared<xleaf_block>(&fb, xheader);

		json_parser jp;

		for (int64_t i = 1; i <= 250; i++)
		{
			xrecord key, value;
			key.add(i);
			value.add(10 + i % 50);
			value.add(100 + (i % 4) * 50);
			pleaf->put(0, key, value);
		}

		pleaf->save();
		_tests->test({ "put_survived", true, __FILE__, __LINE__ });

		auto ref = pleaf->get_reference();
		pleaf = std::make_shared<xleaf_block>(&fb, ref);

		_tests->test({ "read_survived", true, __FILE__, __LINE__ });

		bool round_trip_success = true;
		for (int64_t i = 1; i <= 50; i++)
		{
			xrecord key, value, valueread;
			key.add(i);
			value.add(10 + i % 50);
			value.add(100 + (i % 4) * 50);
			valueread = pleaf->get(key);
			
			if constexpr (debug_xblock) {
				std::string message;
				message = key.to_string();
				message += " correct:";
				message += value.to_string();
				message += " read:";
				message += valueread.to_string();
				system_monitoring_interface::global_mon->log_information(message, __FILE__, __LINE__);
			}

			if (not valueread.all_equal(value)) {
				round_trip_success = false;
				break;
			}
		}
		_tests->test({ "round_trip", round_trip_success, __FILE__, __LINE__ });

		json info = pleaf->get_info();
		system_monitoring_interface::global_mon->log_json(info);

		system_monitoring_interface::global_mon->log_function_stop("xleaf", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_xbranch(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xbranch", "start", start, __FILE__, __LINE__);

		std::shared_ptr<file> fp = _app->open_file_ptr("test.cxdb", file_open_types::create_always);
		file_block fb(fp);

		xrecord_block_header xheader;
		xheader.type = xblock_types::xb_branch;
		xheader.content_type = xblock_types::xb_leaf;
		std::shared_ptr<xbranch_block> pleaf = std::make_shared<xbranch_block>(&fb, xheader);

		json_parser jp;

		for (int64_t i = 1; i <= 50; i++)
		{
			xrecord key, value;
			key.add(i);
			value.add(10 + i % 50);
			value.add(100 + (i % 4) * 50);
			pleaf->put(0, key, value);
		}

		pleaf->save();
		_tests->test({ "put_survived", true, __FILE__, __LINE__ });

		auto ref = pleaf->get_reference();
		pleaf = std::make_shared<xbranch_block>(&fb, ref);

		_tests->test({ "read_survived", true, __FILE__, __LINE__ });

		bool round_trip_success = true;
		for (int64_t i = 1; i <= 50; i++)
		{
			xrecord key, value, valueread;
			key.add(i);
			value.add(10 + i % 50);
			value.add(100 + (i % 4) * 50);
			valueread = pleaf->get(key);

			if constexpr (debug_xblock) {
				std::string message;
				message = key.to_string();
				message += " correct:";
				message += value.to_string();
				message += " read:";
				message += valueread.to_string();
				system_monitoring_interface::global_mon->log_information(message, __FILE__, __LINE__);
			}

			if (not valueread.all_equal(value)) {
				round_trip_success = false;
				break;
			}
		}
		_tests->test({ "round_trip", round_trip_success, __FILE__, __LINE__ });

		json info = pleaf->get_info();
		system_monitoring_interface::global_mon->log_json(info);

		system_monitoring_interface::global_mon->log_function_stop("xbranch", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_xtable(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::global_mon->log_function_start("xtable", "start", start, __FILE__, __LINE__);


		std::shared_ptr<file> fp = _app->open_file_ptr("test.cxdb", file_open_types::create_always);
		file_block fb(fp);

		std::shared_ptr<xtable_header> header = std::make_shared<xtable_header>();
		header->key_members = { object_id_field };
		header->object_members = { "name", "age", "weight" };

		std::shared_ptr<xtable> ptable;
		ptable = std::make_shared<xtable>(&fb, header);

		json_parser jp;

		for (int i = 1; i <= 2000; i++)
		{
			json obj = jp.create_object();
			obj.put_member_i64(object_id_field, i);
			obj.put_member("age", 10 + i % 50);
			obj.put_member("weight", 100 + (i % 4) * 50);
			ptable->put(obj);
		}

		_tests->test({ "put_survived", true, __FILE__, __LINE__ });

		ptable->save();
		_tests->test({ "save_survived", true, __FILE__, __LINE__ });

		int count52 = 0;
		std::vector<std::string> keys = { object_id_field, "age", "weight"};
		bool round_trip_success = true;
		for (int i = 1; i <= 2000; i++)
		{
			json key = jp.create_object();
			key.put_member_i64(object_id_field, i);
			json obj = jp.create_object();
			obj.put_member_i64(object_id_field, i);
			int age = 10 + i % 50;
			if (age == 52)
				count52++;
			obj.put_member("age", 10 + i % 50);
			obj.put_member("weight", 100 + (i % 4) * 50);
			obj.set_compare_order(keys);
			json objd;
			objd = ptable->get(key);
			if (objd.empty())
			{
				round_trip_success = false;
				break;
			}
			objd.set_compare_order(keys);
			if (objd.compare(obj) != 0) {
				round_trip_success = false;
				break;
			}
		}
		_tests->test({ "round_trip", round_trip_success, __FILE__, __LINE__ });

		json object_key = jp.create_object();
		object_key.put_member_i64(object_id_field, 42);
		json select_key_results = ptable->select(object_key, [&object_key](json& _target)-> json {
			if (object_key.compare(_target) == 0) {
				return _target;
			}
			});

		object_key = jp.create_object();
		object_key.put_member("age", 52);
		json select_match_results = ptable->select(object_key, [&object_key](json& _target)-> json {
			if (object_key.compare(_target) == 0) {
				return _target;
			}
			});

		bool range_key_success = select_match_results.size() == count52;
		_tests->test({ "range_key_success", range_key_success, __FILE__, __LINE__ });

		std::map<int, bool> erased_keys;
		bool erase_success = true;
		for (int i = 1; i <= 2000; i+=10)
		{
			erased_keys.insert_or_assign(i, true);
			ptable->erase(i);
		}

		for (int i = 1; i <= 2000; i++)
		{
			json result = ptable->get(i);
			if (result.object() and erased_keys.contains(i)) {
				erase_success = false;
				break;
			}
			if (result.empty() and not erased_keys.contains(i)) {
				erase_success = false;
				break;
			}
		}
		_tests->test({ "erase", erase_success, __FILE__, __LINE__ });

		ptable->clear();
		ptable->save();

		bool clear_success = true;

		for (int i = 1; i <= 2000; i++)
		{
			json key = jp.create_object();
			key.put_member_i64(object_id_field, i);
			json obj = jp.create_object();
			obj.put_member_i64(object_id_field, i);
			obj.put_member("age", 10 + i % 50);
			obj.put_member("weight", 100 + (i % 4) * 50);
			obj.set_compare_order(keys);
			json objd;
			objd = ptable->get(key);
			if (not objd.empty())
			{
				clear_success = false;
				break;
			}
		}

		_tests->test({ "clear", clear_success, __FILE__, __LINE__ });

		json info = ptable->get_info();
		system_monitoring_interface::global_mon->log_json(info);

		system_monitoring_interface::global_mon->log_function_stop("xtable", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif

