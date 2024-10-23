
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

	const int packed_field_type_size = 1;

	struct xstring 
	{
		field_types ft;
		const char*	data;
		size_t size_bytes;

		xstring(const char *_src, size_t _offset)
		{
			ft = (field_types)_src[_offset];
			_offset++;
			data = &_src[_offset];
			size_bytes = strlen(data) + 1 + packed_field_type_size;
		}

		xstring &operator = (const xstring& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xstring(const xstring& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
		}

		xstring &operator = (xstring&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xstring(xstring&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
		}

		int size() const
		{
			return size_bytes;
		}


		std::string to_string() const
		{
			return data;
		}

		static bool get(const std::vector<char>& _src, size_t* _offset, xstring& _dest)
		{
			size_t ofs = *_offset;
			if (ofs >= _src.size())
				return false;
			_dest = xstring(_src.data(), ofs);
			auto sz = _dest.size();
			*_offset += sz;
			return true;
		}

		static void emplace(const std::string& _src, std::vector<char>& _dest)
		{
			char ft = (char)field_types::ft_string;
			_dest.push_back(ft);
			_dest.insert(_dest.end(), _src.c_str(), _src.c_str()+_src.size()+1);
		}
	};

	template <typename data_type, field_types field_type> 
	class xpoco
	{
	public:
		field_types ft;
		data_type data;
		size_t size_bytes;

		xpoco()
		{
			ft = field_types::ft_none;
			data = {};
			size_bytes = packed_field_type_size + sizeof(data_type);
		}

		xpoco(const char* _src, size_t _offset)
		{
			ft = (field_types)_src[_offset];
			_offset++;
			// simply assiging the pointer can be bad due to alignment,
			// and would be less portable anyway
			char* s = (char *)&data;
			for (int i = 0; i < sizeof(data_type); i++)
			{
				s = data[_offset];
				s++;
				_offset++;
			}
			size_bytes = packed_field_type_size + sizeof(data_type) ;
		}

		xpoco& operator = (const xpoco& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xpoco(const xpoco& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
		}

		xpoco& operator = (xpoco&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xpoco(xpoco&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			size_bytes = _src.size_bytes;
		}

		std::string to_string()
		{
			return std::format("{0}", data);
		}

		int size() const
		{
			return size_bytes;
		}

		static bool get(const std::vector<char>& _src, size_t* _offset, xpoco& _dest)
		{
			size_t ofs = *_offset;
			if (ofs >= _src.size())
				return false;
			_dest = xpoco(_src.data(), ofs);
			auto sz = _dest.size();
			*_offset += sz;
			return true;
		}

		static void emplace(const data_type& _src, std::vector<char>& _dest)
		{
			char ft = (char)field_type;
			_dest.push_back(ft);
			const char* sd = (const char*)&_src;
			const char* se = sd + sizeof(data_type);
			_dest.insert(_dest.end(), sd, se);
		}
	};

	using xint64_t = xpoco<int64_t, field_types::ft_int64>;
	using xdouble = xpoco<double, field_types::ft_double>;
	using xdatetime = xpoco<date_time, field_types::ft_datetime>;
	using xplaceholder = xpoco<char, field_types::ft_placeholder>;

	// base templates

	enum class fn_operators {
		op_lt = 0,
		op_eq = 1,
		op_neq = 2,
		op_gt = 3
	};

	template <fn_operators op, typename typea, typename typeb>
	bool xcompare(const typea& _itema, const typeb& _itemb)
	{
		return true;
	}

	template <typename typea, typename typeb> 
	bool xcompare<fn_operators::op_lt>(const typea& _itema, const typeb& _itemb)
	{
		return (_item.data < _itemb.data);
	}

	template <typename typea, typename typeb>
	bool xfn_eq(const typea& _itema, const typeb& _itemb)
	{
		return (_item.data == _itemb.data);
	}

	template <typename typea, typename typeb>
	bool xfn_neq(const typea& _itema, const typeb& _itemb)
	{
		return (_item.data != _itemb.data);
	}

	template <typename typea, typename typeb>
	bool xfn_gt(const typea& _itema, const typeb& _itemb)
	{
		return (_item.data > _itemb.data);
	}

	// string to string

	template <>
	bool xcompare<fn_operators::op_lt,xstring, xstring>(const xstring& _itema, const xstring& _itemb)
	{
		return strcmp( _itema.data, _itemb.data) < 0;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xstring, xstring>(const xstring& _itema, const xstring& _itemb)
	{
		return strcmp(_itema.data, _itemb.data) == 0;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xstring, xstring>(const xstring& _itema, const  xstring& _itemb)
	{
		return strcmp(_itema.data, _itemb.data) != 0;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xstring, xstring>(const xstring& _itema, const  xstring& _itemb)
	{
		return strcmp(_itema.data, _itemb.data) > 0;
	}

	// placeholders are always equal.

	template <typename typeb>
	bool xcompare<fn_operators::op_lt, xplaceholder, typeb>(xplaceholder& _itema, typeb& _itemb)
	{
		return false;
	}

	template <typename typeb>
	bool xcompare<fn_operators::op_eq, xplaceholder, typeb>(xplaceholder& _itema, typeb& _itemb)
	{
		return true;
	}

	template <typename typeb>
	bool xcompare<fn_operators::op_neq, xplaceholder, typeb>(xplaceholder& _itema, typeb& _itemb)
	{
		return false;
	}

	template <typename typeb>
	bool xcompare<fn_operators::op_gt, xplaceholder, typeb>(xplaceholder& _itema, typeb& _itemb)
	{
		return false;
	}


	template <typename typea>
	bool xcompare<fn_operators::op_lt, typea, xplaceholder>(typea& _itema, xplaceholder& _itemb)
	{
		return false;
	}

	template <typename typea>
	bool xcompare<fn_operators::op_eq, typea, xplaceholder>(typea& _itema, xplaceholder& _itemb)
	{
		return true;
	}

	template <typename typea>
	bool xcompare<fn_operators::op_neq, typea, xplaceholder>(typea& _itema, xplaceholder& _itemb)
	{
		return false;
	}

	template <typename typea>
	bool xcompare<fn_operators::op_gt, typea, xplaceholder>(typea& _itema, xplaceholder& _itemb)
	{
		return false;
	}

	// conversions for strings and datetimes

	template <>
	bool xcompare<fn_operators::op_lt, xstring, xdatetime>(const xstring& _itema, const xdatetime& _itemb)
	{
		date_time dp;
		dp.parse(_itema.data);
		return dp < _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xstring, xdatetime>(const xstring& _itema, const xdatetime& _itemb)
	{
		date_time dp;
		dp.parse(_itema.data);
		return dp == _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xstring, xdatetime>(const xstring& _itema, const  xdatetime& _itemb)
	{
		date_time dp;
		dp.parse(_itema.data);
		return dp != _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xstring, xdatetime>(const xstring& _itema, const  xdatetime& _itemb)
	{
		date_time dp;
		dp.parse(_itema.data);
		return dp != _itemb.data;
	}


	template <>
	bool xcompare<fn_operators::op_lt, xdatetime, xstring>(const xdatetime& _itema, const xstring& _itemb)
	{
		date_time dp;
		dp.parse(_itemb.data);
		return dp < _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xdatetime, xstring>(const xdatetime& _itema, const xstring& _itemb)
	{
		date_time dp;
		dp.parse(_itemb.data);
		return dp == _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xdatetime, xstring>(const xdatetime& _itema, const xstring& _itemb)
	{
		date_time dp;
		dp.parse(_itemb.data);
		return dp != _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xdatetime, xstring>(const xdatetime& _itema, const xstring& _itemb)
	{
		date_time dp;
		dp.parse(_itemb.data);
		return dp > _itema.data;
	}

	// conversions for strings and doubles

	template <>
	bool xcompare<fn_operators::op_lt, xstring, xdouble>(const xstring& _itema, const xdouble& _itemb)
	{
		double dp;
		dp = strtod(_itema.data, nullptr);
		return dp < _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xstring, xdouble>(const xstring& _itema, const xdouble& _itemb)
	{
		double dp;
		dp = strtod(_itema.data, nullptr);
		return dp == _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xstring, xdouble>(const xstring& _itema, const  xdouble& _itemb)
	{
		double dp;
		dp = strtod(_itema.data, nullptr);
		return dp != _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xstring, xdouble>(const xstring& _itema, const  xdouble& _itemb)
	{
		double dp;
		dp = strtod(_itema.data, nullptr);
		return dp > _itemb.data;
	}


	template <>
	bool xcompare<fn_operators::op_lt, xdouble, xstring>(const xdouble& _itema, const xstring& _itemb)
	{
		double dp;
		dp = strtod(_itemb.data, nullptr);
		return dp < _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xdouble, xstring>(const xdouble& _itema, const xstring& _itemb)
	{
		double dp;
		dp = strtod(_itemb.data, nullptr);
		return dp == _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xdouble, xstring>(const xdouble& _itema, const xstring& _itemb)
	{
		double dp;
		dp = strtod(_itemb.data, nullptr);
		return dp != _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xdouble, xstring>(const xdouble& _itema, const xstring& _itemb)
	{
		double dp;
		dp = strtod(_itemb.data, nullptr);
		return dp > _itema.data;
	}

	// conversions for strings and int64_t

	template <>
	bool xcompare<fn_operators::op_lt, xstring, xint64_t>(const xstring& _itema, const xint64_t& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itema.data, nullptr, 10);
		return dp < _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xstring, xint64_t>(const xstring& _itema, const xint64_t& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itema.data, nullptr, 10);
		return dp == _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xstring, xint64_t>(const xstring& _itema, const  xint64_t& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itema.data, nullptr, 10);
		return dp != _itemb.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xstring, xint64_t>(const xstring& _itema, const  xint64_t& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itema.data, nullptr, 10);
		return dp > _itemb.data;
	}


	template <>
	bool xcompare<fn_operators::op_lt, xint64_t, xstring>(const xint64_t& _itema, const xstring& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itemb.data, nullptr, 10);
		return dp < _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_eq, xint64_t, xstring>(const xint64_t& _itema, const xstring& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itemb.data, nullptr, 10);
		return dp == _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_neq, xint64_t, xstring>(const xint64_t& _itema, const xstring& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itemb.data, nullptr, 10);
		return dp != _itema.data;
	}

	template <>
	bool xcompare<fn_operators::op_gt, xint64_t, xstring>(const xint64_t& _itema, const xstring& _itemb)
	{
		int64_t dp;
		dp = strtoll(_itemb.data, nullptr, 10);
		return dp > _itema.data;
	}

	// thus armed, we can directly implement xrecord


	class xrecord
	{
		std::vector<char> record_bytes;

		template <typename xtype> bool on_to_string(std::string& _dest, size_t* _offset) const
		{
			xtype temp;
			bool success = xtype::get(record_bytes, _offset, temp);
			if (success) {
				_dest = temp.to_string();
			}
			return success;
		}

		template <typename xtype> bool on_get_json(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xtype temp;
			bool success = xtype::get(record_bytes, _offset, temp);
			if (success) {
				_dest.put_member(_key, temp.data);
			}
			return success;
		}

		template <> bool on_get_json<xint64_t>(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xint64_t temp;
			bool success = xint64_t::get(record_bytes, _offset, temp);
			if (success) {
				_dest.put_member_i64(_key, temp.data);
			}
			return success;
		}

		template <> bool on_get_json<xplaceholder>(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xplaceholder temp;
			bool success = xplaceholder::get(record_bytes, _offset, temp);
			return success;
		}

		template <typename xtype, typename native_type> void emplace(json& _src)
		{
			native_type temp = (native_type)(_src);
			xtype::emplace(temp, record_bytes);
		}

		template <> void emplace<xplaceholder, int>(json& _src)
		{
			int temp = 0;
			xplaceholder::emplace(0, record_bytes);
		}

		template <fn_operators op, typename xtype1, typename xtype2> bool compare_field(size_t* _src_offset, const xrecord& _other, size_t* _other_offset) const
		{
			bool truth = false;
			xtype1 temp1;
			bool success1 = xtype1::get(record_bytes, _offset, temp1);

			xtype1 temp2;
			bool success2 = xtype2::get(_other.record_bytes, _other_offset, temp2);

			if (success1 and success2) 
			{
				truth = xcompare<op>(&temp1, &temp2);
			}

			return truth;
		}

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
			record_bytes.insert(record_bytes.end(), _src, _src + _length);
		}

		size_t size() const
		{
			return record_bytes.size();
		}

		const char* data() const
		{
			return record_bytes.data();
		}

		// end implements xblock storable

		xrecord(const xrecord& _src)
		{
			record_bytes = _src.record_bytes;
		}

		xrecord& operator =(const xrecord& _src)
		{
			record_bytes = _src.record_bytes;
			return *this;
		}

		xrecord(xrecord&& _src)
		{
			record_bytes = std::move(_src.record_bytes);
		}

		xrecord& operator = (xrecord&& _src)
		{
			record_bytes = std::move(_src.record_bytes);
			return *this;
		}

		bool empty()
		{
			return record_bytes.size() == 0;
		}
		
		void put_json(std::vector<std::string>& _keys, json _j)
		{
			record_bytes.clear();
			int index = 0;

			for (auto& fld : _keys)
			{
				std::string field_name = fld;
				auto m = _j[field_name];

				switch (m.get_field_type())
				{
				case field_types::ft_string:
					emplace<xstring, std::string>(m);
					break;
				case field_types::ft_double:
					emplace<xdouble, double>(m);
					break;
				case field_types::ft_datetime:
					emplace<xdatetime, date_time>(m);
					break;
				case field_types::ft_int64:
					emplace<xint64_t, int64_t>(m);
					break;
				default:
					emplace<xplaceholder, int>(m);
					break;
				} // end switch
			}
		}

		std::string to_string() const
		{
			std::stringstream output;

			size_t this_offset = 0;
			bool remaining = this_offset < size();
			std::string separator = "";
			field_types ft;
			
			while (remaining)
			{
				std::string temp;
				ft = (field_types)record_bytes[this_offset];
				switch (ft) {
				case field_types::ft_string:
					on_to_string<xstring>(temp, &this_offset);
					break;
				case field_types::ft_int64:
					on_to_string<xint64_t>(temp, &this_offset);
					break;
				case field_types::ft_double:
					on_to_string<xdouble>(temp, &this_offset);
					break;
				case field_types::ft_datetime:
					on_to_string<xdatetime>(temp, &this_offset);
					break;
				case field_types::ft_placeholder:
					on_to_string<xplaceholder>(temp, &this_offset);
					break;
				}
				output << temp;
				remaining = this_offset < size();
				separator = ".";
			}

			std::string results = output.str();
			return results;
		}
		
		void get_json(json& _dest, std::vector<std::string>& _keys) const
		{
			size_t this_offset = 0;
			bool remaining = this_offset < size();
			std::string separator = "";
			field_types ft;
			auto ki = _keys.begin();

			while (remaining)
			{
				ft = (field_types)record_bytes[this_offset];
				switch (ft) {
				case field_types::ft_string:
					on_get_json<xstring>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_int64:
					on_get_json<xint64_t>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_double:
					on_get_json<xdouble>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_datetime:
					on_get_json<xdatetime>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_placeholder:
					on_get_json<xplaceholder>(_dest, *ki, &this_offset);
					break;
				}
				remaining = this_offset < size();
			}
		}
		
		void clear()
		{
			record_bytes.clear();
		}
		
		xrecord& add(double _value)
		{
			xdouble::emplace(_value, record_bytes);
			return *this;
		}

		xrecord& add(std::string _value)
		{
			xstring::emplace(_value, record_bytes);
			return *this;
		}

		xrecord& add(date_time _value)
		{
			xdatetime::emplace(_value, record_bytes);
			return *this;
		}

		xrecord& add(int64_t _value)
		{
			xint64_t::emplace(_value, record_bytes);
			return *this;
		}

		xrecord& add()
		{
			xplaceholder::emplace(0, record_bytes);
			return *this;
		}

		bool is_empty()
		{
			return record_bytes.empty();
		}

		template <fn_operators compare_op> bool compare(const xrecord& _other) const
		{
			bool comp_result;
			size_t this_offset = 0;
			size_t other_offset = 0;
			field_types this_ft;
			field_types other_ft;
			bool this_remaining = this_offset < size();
			bool other_remaining = other_offset < _other.size();

			while (this_remaining and other_remaining)
			{
				this_ft = (field_types)record_bytes[this_offset];
				other_ft = (field_types)_other.record_bytes[other_offset];

				if (this_ft == field_types::ft_int64)
				{
					if (other_ft == field_types::ft_int64)
					{
						comp_result = compare_field<compare_op, xint64_t, xint64_t>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						comp_result = compare_field<compare_op, xint64_t, xstring>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						comp_result = compare_field<compare_op, xint64_t, xdouble>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						comp_result = compare_field<compare_op, xint64_t, xdatetime>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_placeholder)
					{
						comp_result = compare_field<compare_op, xint64_t, xplaceholder> (compare_function, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_string)
				{ 
					if (other_ft == field_types::ft_int64)
					{
						comp_result = compare_field<compare_op, xstring, xint64_t>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						comp_result = compare_field<compare_op, xstring, xstring>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						comp_result = compare_field<compare_op, xstring, xdouble>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						comp_result = compare_field<compare_op, xstring, xdatetime>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_placeholder)
					{
						comp_result = compare_field<compare_op, xstring, xplaceholder>(compare_function, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_double)
				{
					if (other_ft == field_types::ft_int64)
					{
						comp_result = compare_field<compare_op, xdouble, xint64_t>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						comp_result = compare_field<compare_op, xdouble, xstring>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						comp_result = compare_field<compare_op, xdouble, xdouble>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						comp_result = compare_field<compare_op, xdouble, xdatetime>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_placeholder)
					{
						comp_result = compare_field<compare_op, xdouble, xplaceholder>(compare_function, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_datetime)
				{
					if (other_ft == field_types::ft_int64)
					{
						comp_result = compare_field<compare_op, xdatetime, xint64_t>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						comp_result = compare_field<compare_op, xdatetime, xstring>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						comp_result = compare_field<compare_op, xdatetime, xdouble>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						comp_result = compare_field<compare_op, xdatetime, xdatetime>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_placeholder)
					{
						comp_result = compare_field<compare_op, xdatetime, xplaceholder>(compare_function, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_placeholder)
				{
					if (other_ft == field_types::ft_int64)
					{
						comp_result = compare_field<compare_op, xplaceholder, xint64_t>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						comp_result = compare_field<compare_op, xplaceholder, xstring>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						comp_result = compare_field<compare_op, xplaceholder, xdouble>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						comp_result = compare_fieldcompare_op, <xplaceholder, xdatetime>(compare_function, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_placeholder)
					{
						comp_result = compare_field<compare_op, xplaceholder, xplaceholder>(compare_function, &this_offset, _other, &other_offset);
					}
				}

				if (constexpr compare_op == fn_operators::op_eq)
				{
					if (not comp_result) return false;
				}
				else if (constexpr compare_op == fn_operators::op_neq)
				{
					if (comp_result) return true;
				}
				else if (constexpr compare_op == fn_operators::op_lt)
				{
					if (comp_result) return true;
				}
				else if (constexpr compare_op == fn_operators::op_gt)
				{
					if (comp_result) return true;
				}

				this_remaining = not comp_result.end_this;
				other_remaining = not comp_result.end_other;
			}
		}

		bool all_equal(const xrecord& _other) const
		{

			return true;
		}

		bool operator == (const xrecord& _other) const
		{
			return compare<fn_operators::op_eq>( _other );
		}
		bool operator < (const xrecord& _other) const
		{
			return compare<fn_operators::op_lt> (_other);
			return false;
		}
		bool operator != (const xrecord& _other) const
		{
			return compare<fn_operators::op_neq>(_other);
			return false;
		}
		bool operator > (const xrecord & _other) const
		{
			return compare<fn_operators::op_gt>(_other);
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

