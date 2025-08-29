#pragma once

#ifndef CORONA_EXPRESS_RECORD_HPP
#define CORONA_EXPRESS_RECORD_HPP

/// <summary>
/// 
/// </summary>
namespace corona
{


	const int packed_field_type_size = 1;

	struct xptr
	{
	public:
		int64_t length;
		char* bytes;
	};

	class xfield
	{
	public:
		field_types field_type;
		int64_t		record_offset;

		virtual xptr get_ptr() = 0;
	};

	struct xrecord2 
	{
	public:
        std::vector<xfield> record_bytes;

		xrecord2() = default;
		virtual ~xrecord2() = default;
	};

	struct xstring
	{
		field_types ft;
		int32_t length;
		const char* data;
		size_t size_bytes;

		xstring()
		{
			ft = field_types::ft_string;
			data = nullptr;
			length = 0;
			size_bytes = 0;
		}

		xstring(const char* _src, size_t _offset)
		{
			ft = (field_types)_src[_offset];
			_offset++;
			int32_t* l = (int *)(&_src[_offset]);
			length = *l;
			_offset += sizeof(int32_t);
			data = &_src[_offset];
			_offset += length;
			size_bytes = packed_field_type_size + sizeof(int32_t) + length;
		}

		xstring(const xstring& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
		}

		xstring& operator = (const xstring& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xstring(xstring&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
		}

		xstring& operator = (xstring&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
			return *this;
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
			auto sz = _dest.size_bytes;
			*_offset += sz;
			return true;
		}

		static int emplace(const std::string& _src, std::vector<char>& _dest)
		{
			char ft = (char)field_types::ft_string;
			_dest.push_back(ft);

			int32_t l = _src.size() + 1;
			char* x = (char *)&l;
			_dest.insert(_dest.end(), x, x + sizeof(l));

			_dest.insert(_dest.end(), _src.c_str(), _src.c_str() + l);
			int return_value;
			return_value = std::distance(_dest.begin(), _dest.end());
			return return_value;
		}

		static int emplace_as(field_types _ft, const std::string& _src, std::vector<char>& _dest)
		{
			char ft = (char)_ft;
			_dest.push_back(ft);

			int32_t l = _src.size() + 1;
			char* x = (char*)&l;
			_dest.insert(_dest.end(), x, x + sizeof(l));

			_dest.insert(_dest.end(), _src.c_str(), _src.c_str() + l);
			int return_value;
			return_value = std::distance(_dest.begin(), _dest.end());
			return return_value;
		}

	};

	struct xreference
	{
		field_types ft;
		int64_t object_id;
		int32_t length;
		const char* data;
		size_t size_bytes;

		xreference()
		{
			ft = field_types::ft_reference;
			data = nullptr;
			length = 0;
			size_bytes = 0;
		}

		xreference(const char* _src, size_t _offset)
		{
			ft = (field_types)_src[_offset];
			_offset++;
			int64_t* oi = (int64_t*)(&_src[_offset]);
			_offset += sizeof(int64_t);
			int32_t* l = (int*)(&_src[_offset]);
			length = *l;
			_offset += sizeof(int32_t);
			data = &_src[_offset];
			_offset += length;
			size_bytes = packed_field_type_size + sizeof(int32_t) + length;
		}

		xreference(const xreference& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
		}

		xreference& operator = (const xreference& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
			return *this;
		}

		xreference(xreference&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
		}

		xreference& operator = (xreference&& _src)
		{
			ft = _src.ft;
			data = _src.data;
			length = _src.length;
			size_bytes = _src.size_bytes;
			return *this;
		}

		int size() const
		{
			return size_bytes;
		}


		std::string to_string() const
		{
			return data;
		}

		operator object_reference_type()
		{
			object_reference_type ort;
			ort.class_name = data;
			ort.object_id = object_id;
			return ort;
		}

		static bool get(const std::vector<char>& _src, size_t* _offset, xreference& _dest)
		{
			size_t ofs = *_offset;
			if (ofs >= _src.size())
				return false;
			const char* s = _src.data() + ofs;
			field_types ft = *((field_types*)s);
			s++;
			_dest.object_id = *((int64_t*)s);
			s+=sizeof(int64_t);
			int32_t sz = *((int32_t*)s);
			s += sizeof(int32_t);
			_dest.data = s;
			*_offset += sz;
			return true;
		}

		static int emplace(const object_reference_type& _src, std::vector<char>& _dest)
		{
			char ft = (char)field_types::ft_reference;
			_dest.push_back(ft);

			int64_t i = _src.object_id;
			char* x = (char*)&i;
			_dest.insert(_dest.end(), x, x + sizeof(i));

			int32_t l = _src.class_name.size() + 1;
			x = (char*)&l;
			_dest.insert(_dest.end(), x, x + sizeof(l));

			_dest.insert(_dest.end(), _src.class_name.begin(), _src.class_name.end());
			int return_value;
			return_value = std::distance(_dest.begin(), _dest.end());
			return return_value;
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
			size_bytes = packed_field_type_size + sizeof(data_type);
		}

		xpoco(const char* _src, size_t _offset)
		{
			ft = (field_types)_src[_offset];
			_offset++;
			// simply assiging the pointer can be bad due to alignment,
			// and would be less portable anyway
			_src = _src + _offset;
			char* s = (char*)&data;
			for (int i = 0; i < sizeof(data_type); i++)
			{
				*s = *_src;
				s++;
				_src++;
			}
			size_bytes = packed_field_type_size + sizeof(data_type);
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

		static int emplace(const data_type& _src, std::vector<char>& _dest)
		{
			char ft = (char)field_type;
			_dest.push_back(ft);
			int return_value;
			return_value = std::distance(_dest.begin(), _dest.end());
			const char* sd = (const char*)&_src;
			const char* se = sd + sizeof(data_type);
			_dest.insert(_dest.end(), sd, se);
			return return_value;
		}

		bool eq(const xpoco& _other) const
		{
			return data == _other.data;
		}

		bool lt(const xpoco& _other) const
		{
			return data < _other.data;
		}

		bool gt(const xpoco& _other) const
		{
			return data > _other.data;
		}

		bool neq(const xpoco& _other) const
		{
			return data != _other.data;
		}
	};

	class xint64_t : public xpoco<int64_t, field_types::ft_int64>
	{
	public:

	};

	class xdouble : public xpoco<double, field_types::ft_double>
	{
	public:

	};

	class xdatetime : public xpoco<date_time, field_types::ft_datetime>
	{
	public:
	};

	class xwildcard : public xpoco<char, field_types::ft_wildcard>
	{

	};

	// base templates

	enum class fn_operators {
		op_lt = 0,
		op_eq = 1,
		op_neq = 2,
		op_gt = 3
	};

	class fn_op_lt {

	};

	class fn_op_eq {

	};

	class fn_op_neq {

	};

	class fn_op_gt {

	};

	// strings 

	std::weak_ordering xcompare(const xstring& _itema, const xstring& _itemb)
	{
		int k = strcmp(_itema.data, _itemb.data);
		if (k < 0)
			return std::weak_ordering::less;
		else if (k == 0)
			return std::weak_ordering::equivalent;
		else if (k > 0)
			return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xstring& _itema, const xdatetime& _itemb)
	{
		date_time dp;
		dp.parse(_itema.data);
		return dp <=> _itemb.data;
	}

	std::weak_ordering xcompare(const xstring& _itema, const xdouble& _itemb)
	{
		double dp = strtod(_itema.data, nullptr);
		if (dp < _itemb.data)
			return std::weak_ordering::less;
		else if (dp == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	// conversions for strings and int64_t

	std::weak_ordering xcompare(const xstring& _itema, const xint64_t& _itemb)
	{
		int64_t dp = strtoll(_itema.data, nullptr, 10);
		if (dp < _itemb.data)
			return std::weak_ordering::less;
		else if (dp == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xstring& _itemb, const xreference& _itema)
	{
		int64_t dp = strtoll(_itemb.data, nullptr, 10);
		if (_itema.object_id > dp)
			return std::weak_ordering::greater;
		else if (_itema.object_id == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::less;
	}


	// xint64_t
	std::weak_ordering xcompare(const xint64_t& _itema, const xint64_t& _itemb)
	{
		if (_itema.data < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.data == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xint64_t& _itema, const xstring& _itemb)
	{
		int64_t dp = strtoll(_itemb.data, nullptr, 10);
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xint64_t& _itema, const xdouble& _itemb)
	{
		if (_itema.data < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.data == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xint64_t& _itema, const xdatetime& _itemb)
	{
		int64_t dp = _itemb.data.get_time_t();
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xint64_t& _itemb, const xreference& _itema)
	{
		if (_itema.object_id > _itemb.data)
			return std::weak_ordering::greater;
		else if (_itema.object_id == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::less;
	}



	// xdouble
	std::weak_ordering xcompare(const xdouble& _itema, const xdouble& _itemb)
	{
		if (_itema.data < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.data == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;

	}

	std::weak_ordering xcompare(const xdouble& _itema, const xstring& _itemb)
	{
		double dp = strtod(_itemb.data, nullptr);
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xdouble& _itema, const xint64_t& _itemb)
	{
		if (_itema.data < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.data == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xdouble& _itema, const xdatetime& _itemb)
	{
		int64_t dp = _itemb.data.get_time_t();
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xdouble& _itemb, const xreference& _itema)
	{
		if (_itema.object_id > _itemb.data)
			return std::weak_ordering::greater;
		else if (_itema.object_id == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::less;
	}

	// xdatetime
	std::weak_ordering  xcompare(const xdatetime& _itema, const xdatetime& _itemb)
	{
		if (_itema.data < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.data == _itemb.data)
			return std::weak_ordering::equivalent;
	}


	std::weak_ordering xcompare(const xdatetime& _itema, const xstring& _itemb)
	{
		date_time dp;
		dp.parse(_itemb.data);
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xdatetime& _itema, const xdouble& _itemb)
	{
		int64_t dp = _itema.data.get_time_t();
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;

	}

	std::weak_ordering xcompare(const xdatetime& _itema, const xint64_t& _itemb)
	{
		int64_t dp = _itema.data.get_time_t();
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xdatetime& _itema, const xreference& _itemb)
	{
		int64_t dp = _itema.data.get_time_t();
		if (_itema.data < dp)
			return std::weak_ordering::less;
		else if (_itema.data == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	// references

	std::weak_ordering xcompare(const xreference& _itema, const xint64_t& _itemb)
	{
		if (_itema.object_id < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.object_id == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xreference& _itema, const xstring& _itemb)
	{
		int64_t dp = strtoll(_itemb.data, nullptr, 10);
		if (_itema.object_id < dp)
			return std::weak_ordering::less;
		else if (_itema.object_id == dp)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xreference& _itema, const xdouble& _itemb)
	{
		if (_itema.object_id < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.object_id == _itemb.data)
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xreference& _itema, const xdatetime& _itemb)
	{
		int64_t dp = _itemb.data.get_time_t();
		if (_itema.object_id < _itemb.data)
			return std::weak_ordering::less;
		else if (_itema.object_id == _itemb.data.get_time_t())
			return std::weak_ordering::equivalent;
		else return std::weak_ordering::greater;
	}

	std::weak_ordering xcompare(const xreference& _itema, const xreference& _itemb)
	{
		if (_itema.object_id < _itemb.object_id) {
			return std::weak_ordering::less;
		}
		else if (_itema.object_id == _itemb.object_id)
		{
			// I'll get away with this for now because I know that there is one object_id_counter
			return std::weak_ordering::equivalent;
		}
		else return std::weak_ordering::greater;
	}


	// wildcards are always equal.

	std::weak_ordering xcompare(const xwildcard& _itema, const xwildcard& _itemb)
	{
		return std::weak_ordering::equivalent;
	}

	template <typename typeb>
	std::weak_ordering xcompare(const xwildcard& _itema, const typeb& _itemb)
	{
		return std::weak_ordering::equivalent;
	}

	template <typename typea>
	std::weak_ordering xcompare(const typea& _itema, const xwildcard& _itemb)
	{
		return std::weak_ordering::equivalent;
	}


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

		template <> bool on_get_json<xstring>(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xstring temp;
			bool success = xstring::get(record_bytes, _offset, temp);
			if (success) {
				_dest.put_member(_key, std::string(temp.data));
			}
			return success;
		}

		bool on_get_json_object(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xstring temp;
			bool success = xstring::get(record_bytes, _offset, temp);
			if (success) {
				std::string temp_obj = std::string(temp.data);
				json_parser jp;
				json obj = jp.parse_object(temp_obj);
				_dest.share_member(_key, obj);
			}
			return success;
		}

		bool on_get_json_array(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xstring temp;
			bool success = xstring::get(record_bytes, _offset, temp);
			if (success) {
				std::string temp_obj = std::string(temp.data);
				json_parser jp;
				json obj = jp.parse_array(temp_obj);
				_dest.share_member(_key, obj);
			}
			return success;
		}

		template <> bool on_get_json<xreference>(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xreference temp;
			bool success = xreference::get(record_bytes, _offset, temp);
			if (success) {
				object_reference_type ort;
				ort.class_name = temp.data;
				ort.object_id = temp.object_id;
				_dest.put_member(_key, ort);
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

		template <> bool on_get_json<xwildcard>(json& _dest, const std::string& _key, size_t* _offset) const
		{
			xwildcard temp;
			bool success = xwildcard::get(record_bytes, _offset, temp);
			return success;
		}

		void emplace_json(json& _src)
		{
			std::string temp = _src.to_json_typed();
			xstring::emplace_as(_src.get_field_type(), temp, record_bytes);
		}

		void emplace(const object_reference_type& _src)
		{
			xreference temp;
			temp.object_id = _src.object_id;
			temp.length = _src.class_name.length() + 1;
			temp.data = _src.class_name.c_str();
			xreference::emplace(temp, record_bytes);
		}

		template <typename xtype, typename native_type> void emplace(json& _src)
		{
			native_type temp = (native_type)(_src);
			xtype::emplace(temp, record_bytes);
		}

		template <> void emplace<xreference,object_reference_type>(json& _src)
		{
			object_reference_type temp = (object_reference_type)(_src);
			xreference::emplace(temp, record_bytes);
		}

		template <> void emplace<xwildcard, int>(json& _src)
		{
			int temp = 0;
			xwildcard::emplace(0, record_bytes);
		}

		template <typename comparefn, typename xtype1, typename xtype2>
		class field_comparer
		{
		public:
			std::weak_ordering compare_field(const std::vector<char>& _src, size_t* _src_offset, const xrecord& _other, size_t* _other_offset) const
			{
				xtype1 temp1;
				bool success1 = xtype1::get(_src, _src_offset, temp1);

				xtype2 temp2;
				bool success2 = xtype2::get(_other.record_bytes, _other_offset, temp2);

				if (success1 and success2)
				{
					std::weak_ordering truth;
					truth = xcompare(temp1, temp2);
					return truth;
				}

				throw std::logic_error("comparison missing operands");
			}
		};

		template <typename compare_fn>
		bool compare_record(const xrecord& _other) const
		{
			std::weak_ordering comp_result;
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
						field_comparer<compare_fn, xint64_t, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						field_comparer<compare_fn, xint64_t, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xint64_t, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xint64_t, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xint64_t, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xint64_t, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_string)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xstring, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						field_comparer<compare_fn, xstring, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xstring, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xstring, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xstring, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xstring, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_double)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xdouble, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						field_comparer<compare_fn, xdouble, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xdouble, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xdouble, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xdouble, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xdouble, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_datetime)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xdatetime, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						field_comparer<compare_fn, xdatetime, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xdatetime, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xdatetime, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xdatetime, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xdatetime, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_reference)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xreference, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string)
					{
						field_comparer<compare_fn, xreference, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xreference, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xreference, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xreference, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xreference, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					}
				else if (this_ft == field_types::ft_wildcard)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xwildcard, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string or other_ft == field_types::ft_object or other_ft == field_types::ft_array)
					{
						field_comparer<compare_fn, xwildcard, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xwildcard, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xwildcard, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xwildcard, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xwildcard, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
				}
				else if (this_ft == field_types::ft_object or this_ft == field_types::ft_array)
				{
					if (other_ft == field_types::ft_int64)
					{
						field_comparer<compare_fn, xstring, xint64_t> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_string or other_ft == field_types::ft_object or other_ft == field_types::ft_array)
					{
						field_comparer<compare_fn, xstring, xstring> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_double)
					{
						field_comparer<compare_fn, xstring, xdouble> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_datetime)
					{
						field_comparer<compare_fn, xstring, xdatetime> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_reference)
					{
						field_comparer<compare_fn, xstring, xreference> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}
					else if (other_ft == field_types::ft_wildcard)
					{
						field_comparer<compare_fn, xstring, xwildcard> comparer;
						comp_result = comparer.compare_field(record_bytes, &this_offset, _other, &other_offset);
					}

				}
				if constexpr (std::is_same<compare_fn, fn_op_eq>::value)
				{
					if (comp_result != std::weak_ordering::equivalent) return false;
				}
				if constexpr (std::is_same<compare_fn, fn_op_neq>::value)
				{
					if (comp_result == std::weak_ordering::equivalent) return true;
				}
				if constexpr (std::is_same<compare_fn, fn_op_lt>::value)
				{
					// ok, this has to be not comp_result immediate bail
					// to ensure strict weak ordering.
					// strict weak ordering is dead simple.
					// first, if you have a < b is false, then b < a must be false, as b is either equal or > a to be false.
					// second, if you have a < b, and b < c, then a < c must be true.
					// do these things, and you can even database records sorted for while in memory stl:maps or maybe even 
					// a fast implementation.
					// but here is the twister,
					// say we are comparing two sets of records, and ours and another, we walk through this in steps, 1..n
					// discussing using lower case letters for whatever pops into my head such as variables
					// 1.  ours - get our current key a, or our a, and likewise their key b.
					// 2.  compare truth = ours < theirs
					// think about it, each of our numbers and strings are already strict weakly ordered,
					// our own records strict weak ordering would work like that,
					// and the special rule is thus. If you are already false, you are done.  It can never be true,
					// once it is false, otherwise, we're breaking our own strict weak ordering and we'd never 
					// be ordered at all.  if you are bouncing strict weak ordering off of a map, the map
					// will throw an exception and it will put you right where the comparison is so you 
					// back up and step through it and check it.
					// Instead of letters, think of this string as where the letters are labels for something, 
					// so that, whatever it is will be strictly weakly ordered, if all our field types are,
					// as this makes us do this:
					// A*123 < B*123 < C*123
					// A1* < A2*
					// A2* < A3*
					// NOT A2* < A1*
					// NOT A3* < A2*
					// A1* < A3*
					// A1*9 < A*3
					// A23* < A23*45
					// everything is ordered like a string gets sorted in a way that makes sense.
					// the star is where you don't know.
					if (comp_result == std::weak_ordering::less)
						return true;
					else if (comp_result == std::weak_ordering::greater)
						return false;
				}
				if constexpr (std::is_same<compare_fn, fn_op_gt>::value)
				{
					if (comp_result == std::weak_ordering::less)
						return false;
					else if (comp_result == std::weak_ordering::greater)
						return true;
				}

				this_remaining = this_offset < record_bytes.size();
				other_remaining = other_offset < _other.record_bytes.size();
			}

			if constexpr (std::is_same<compare_fn, fn_op_eq>::value)
			{
				return not (this_remaining and other_remaining);
			}
			if constexpr (std::is_same<compare_fn, fn_op_neq>::value)
			{
				return this_remaining or other_remaining;
			}
			if constexpr (std::is_same<compare_fn, fn_op_lt>::value)
			{
				return not this_remaining and other_remaining;
			}
			if constexpr (std::is_same<compare_fn, fn_op_gt>::value)
			{
				return this_remaining and not other_remaining;
			}

			return false;
		}

		bool has_wildcard;

	public:

		xrecord()
		{
			has_wildcard = false;
		}

		bool is_wildcard() const
		{
			return has_wildcard;
		}

		xrecord(std::vector<std::string>& _keys, json _src)
		{
			has_wildcard = false;
			put_json(_keys, _src);
		}

		// to satisfy xblock_storable

		xrecord(const char* _src, size_t _length)
		{
			has_wildcard = false;
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
			has_wildcard = _src.has_wildcard;
		}

		xrecord& operator =(const xrecord& _src)
		{
			record_bytes = _src.record_bytes;
			has_wildcard = _src.has_wildcard;
			return *this;
		}

		xrecord(xrecord&& _src)
		{
			record_bytes = std::move(_src.record_bytes);
			has_wildcard = _src.has_wildcard;

		}

		xrecord& operator = (xrecord&& _src)
		{
			record_bytes = std::move(_src.record_bytes);
			has_wildcard = _src.has_wildcard;

			return *this;
		}

		bool empty()
		{
			return record_bytes.size() == 0;
		}

		void put_json(std::vector<std::string>& _keys, json _j)
		{
			has_wildcard = false;
			record_bytes.clear();
			int index = 0;

			for (auto& fld : _keys)
			{
				std::string field_name = fld;
				std::string temp;
				auto m = _j[field_name];

				switch (m.get_field_type())
				{
				case field_types::ft_string:
					emplace<xstring, std::string>(m);
					break;
				case field_types::ft_array:
					emplace_json(m);
					break;
				case field_types::ft_object:
					emplace_json(m);
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
				case field_types::ft_reference:
					emplace<xreference, object_reference_type>(m);
					break;
				default:
					has_wildcard = true;
					emplace<xwildcard, int>(m);
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
				case field_types::ft_wildcard:
					on_to_string<xwildcard>(temp, &this_offset);
					break;
				case field_types::ft_reference:
					on_to_string<xreference>(temp, &this_offset);
					break;
				case field_types::ft_array:
					on_to_string<xstring>(temp, &this_offset);
					break;
				case field_types::ft_object:
					on_to_string<xstring>(temp, &this_offset);
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

			while (remaining and ki != _keys.end())
			{
				ft = (field_types)record_bytes[this_offset];
				switch (ft) {
				case field_types::ft_string:
					on_get_json<xstring>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_object:
					on_get_json_object(_dest, *ki, &this_offset);
					break;
				case field_types::ft_array:
					on_get_json_array(_dest, *ki, &this_offset);
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
				case field_types::ft_reference:
					on_get_json<xreference>(_dest, *ki, &this_offset);
					break;
				case field_types::ft_wildcard:
					on_get_json<xwildcard>(_dest, *ki, &this_offset);
					break;
				}
				remaining = this_offset < size();
				ki++;
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
			xwildcard::emplace(0, record_bytes);
			return *this;
		}

		int bind(double _value)
		{
			return xdouble::emplace(_value, record_bytes);
		}

		int bind(std::string _value)
		{
			return xstring::emplace(_value, record_bytes);
		}

		int bind(date_time _value)
		{
			return xdatetime::emplace(_value, record_bytes);
		}

		int bind(int64_t _value)
		{
			return xint64_t::emplace(_value, record_bytes);
		}

		char* get_ptr(int _binding_offset)
		{
			return &record_bytes[_binding_offset];
		}

		bool is_empty()
		{
			return record_bytes.empty();
		}

		bool exact_equal(const xrecord& _other) const
		{
			if (_other.size() != size())
				return false;

			for (int i = 0; i < size(); i++)
			{
				if (_other.record_bytes[i] != record_bytes[i])
					return false;
			}

			return true;
		}

		bool operator == (const xrecord& _other) const
		{
			return compare_record<fn_op_eq>(_other);
		}
		bool operator < (const xrecord& _other) const
		{
			return compare_record<fn_op_lt>(_other);
		}
		bool operator <= (const xrecord& _other) const
		{
			return compare_record<fn_op_lt>(_other) or compare_record<fn_op_eq>(_other);
		}
		bool operator != (const xrecord& _other) const
		{
			return compare_record<fn_op_neq>(_other);
		}
		bool operator > (const xrecord& _other) const
		{
			return compare_record<fn_op_gt>(_other);
		}
		bool operator >= (const xrecord& _other) const
		{
			return compare_record<fn_op_gt>(_other) or compare_record<fn_op_eq>(_other);
		}

	};

	void test_xrecord(std::shared_ptr<test_set> _tests, std::shared_ptr<application> _app)
	{
		timer tx;
		date_time start = date_time::now();

		system_monitoring_interface::active_mon->log_function_start("xrecord", "start", start, __FILE__, __LINE__);
		using namespace std::literals;

		xrecord comp1, comp2, comp3, test_in, test_out;

		json_parser jp;
        json test_obj = jp.create_object();
        json test_inner_obj = jp.create_object();
		json test_inner_array = jp.create_array();
		test_inner_obj.put_member("string_value", std::string("hello"));
		test_inner_obj.put_member("int64_value", 42i64);
		test_inner_array.push_back(std::string("one"));
		test_inner_array.push_back(2i64);
		test_inner_array.push_back(3.0);
		test_obj.put_member("obj_value", test_inner_obj);
		test_obj.put_member("arr_value", test_inner_array);
		test_obj.put_member("amount", 4.0);
		test_obj.put_member("prompt", std::string("hello"));
        test_obj.put_member("object_id", 42i64);
		
        std::vector<std::string> keysc = { "amount", "prompt", "object_id", "obj_value", "arr_value" };
		test_in.put_json(keysc, test_obj);
		json jtest_out = jp.create_object();
        test_in.get_json(jtest_out, keysc);

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

		result = not (comp1 < comp3);
		_tests->test({ "xr < swo", result, __FILE__, __LINE__ });

		result = comp1 > comp3;
		_tests->test({ "xr >", result, __FILE__, __LINE__ });

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

		result = comp1 >= comp3;
		_tests->test({ "xr >= key 3", result, __FILE__, __LINE__ });

		result = not (comp1 < comp3);
		_tests->test({ "xr < key 3, swo", result, __FILE__, __LINE__ });


		comp3.add(42i64);
		result = comp3 < comp1;
		_tests->test({ "xr < key 3", result, __FILE__, __LINE__ });

		result = not (comp1 < comp3);
		_tests->test({ "xr < key 3, swo", result, __FILE__, __LINE__ });

		// implied equality
		comp3.clear();
		comp3.add(4.0);
		result = comp3 < comp1;
		_tests->test({ "xr < key 2.1", result, __FILE__, __LINE__ });

		result = not (comp1 < comp3);
		_tests->test({ "xr < key 2.1 swo", result, __FILE__, __LINE__ });


		comp3.add("hello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 2.2", result, __FILE__, __LINE__ });

		result = not (comp1 < comp3);
		_tests->test({ "xr < key 2.2 swo", result, __FILE__, __LINE__ });


		comp3.add(43i64);
		result = not(comp3 < comp1);
		_tests->test({ "xr < key 2.3", result, __FILE__, __LINE__ });

		result = (comp1 < comp3);
		_tests->test({ "xr > key 2.3 swo", result, __FILE__, __LINE__ });


		comp3.clear();
		comp3.add(4.0);
		comp3.add("ahello");
		result = comp3 < comp1;
		_tests->test({ "xr < key 3.1", result, __FILE__, __LINE__ });
		result = not(comp1 < comp3);
		_tests->test({ "xr > key 3.1", result, __FILE__, __LINE__ });

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
		json jsrc = jp.create_object();
		jsrc.put_member("Name", "Bill"sv);
		jsrc.put_member("Age", "41"sv);
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

		// and with the json
		jdst = jp.create_object();
		xrecord xsrc(keys, jsrc);
		xrecord readin = xrecord(xsrc.data(), xsrc.size());
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

		// and now, unit tests that simulate our indexes.
		comp1.clear();
		comp1.add("Roger");
		comp1.add(13i64);

		comp2.clear();
		comp2.add("Sam");
		comp2.add(14i64);

		comp3.clear();
		comp3.add("Xavier");
		comp3.add(15i64);

		result = comp1 < comp2;
		_tests->test({ "index <", result, __FILE__, __LINE__ });

		result = not (comp2 < comp1);
		_tests->test({ "index < swo", result, __FILE__, __LINE__ });

		result = comp2 < comp3;
		_tests->test({ "index 2 <", result, __FILE__, __LINE__ });

		result = not (comp3 < comp2);
		_tests->test({ "index 2 < swo", result, __FILE__, __LINE__ });

		result = comp1 < comp3;
		_tests->test({ "index 3 <", result, __FILE__, __LINE__ });

		result = not (comp3 < comp1);
		_tests->test({ "index 3 < swo", result, __FILE__, __LINE__ });

		system_monitoring_interface::active_mon->log_function_stop("xrecord", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif
