#ifndef CORONA_JSON_H
#define CORONA_JSON_H

namespace corona 
{

	class json_value
	{
	public:
		int comparison_index = 0;

		json_value()
		{
			;
		}
		virtual ~json_value()
		{
			;
		}

		virtual std::string to_key()
		{
			return "";
		}
		virtual std::string to_json()
		{
			return "";
		}
		virtual std::string to_json_typed()
		{
			return "";
		}
		virtual std::string to_string()
		{
			return "";
		}
		virtual std::string get_type_prefix()
		{
			return "";
		}
		virtual std::string get_type_name()
		{
			std::string temp;

			temp = get_type_prefix();
			if (temp.size()) {
				temp = temp.substr(1, temp.size() - 1);
			}

			return temp;
		}
		virtual std::string get_ctype_name()
		{
			return typeid(*this).name();
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto jv = std::make_shared<json_value>(*this);
			jv->comparison_index = comparison_index;
			return jv;
		}
	};

	class json_double : public json_value
	{
	public:
		double value;

		virtual std::string to_key()
		{
			return std::to_string(value);
		}
		virtual std::string to_json()
		{
			return std::to_string(value);
		}
		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::string to_string()
		{
			return std::format("{}", value);
		}
		virtual std::string get_type_prefix()
		{
			return "$double";
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_double>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_datetime : public json_value
	{
	public:
		date_time value;

		virtual std::string to_key()
		{
			std::string v = value;
			return value;
		}
		virtual std::string to_json()
		{
			return "\"" + to_string() + "\"";
		}
		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::string to_string()
		{
			return value;
		}
		virtual std::string get_type_prefix()
		{
			return "$datetime";
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_datetime>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_blob : public json_value
	{
	public:
		std::vector<char> value;

		virtual std::string to_json()
		{
			return "\"" + to_string() + "\"";
		}

		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}

		virtual std::string to_string()
		{
			std::string st;
			for (auto ea : value) 
			{
				auto c = toHex(ea);
				st += c.str;
			}
			return st;
		}

		virtual void from_string(std::string st)
		{
			value.clear();
			for (int i = 0; i < st.size(); i += 2)
			{
				int item = toInt2(st, i);
				value.push_back(item);
			}
		}

		virtual std::string get_type_prefix()
		{
			return "$blob";
		}

		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_blob>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_int64 : public json_value
	{
	public:
		int64_t value;

		virtual std::string to_key()
		{
			return std::to_string(value);
		}

		virtual std::string to_json()
		{
			return std::to_string(value);
		}

		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}

		virtual std::string to_string()
		{
			return std::format("{}", value);
		}

		virtual std::string get_type_prefix()
		{
			return "$int64";
		}

		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_int64>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_string : public json_value
	{
	public:
		std::string value;

		virtual std::string to_key()
		{
			return value;
		}
		virtual std::string to_json()
		{
			return "\"" + value + "\"";
		}
		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::string to_string()
		{
			return value;
		}
		virtual std::string get_type_prefix()
		{
			return "$string";
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_string>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_array : public json_value
	{
	public:
		std::vector<std::shared_ptr<json_value>> elements;

		virtual std::string to_key()
		{
			std::string ret = "";
			std::string comma = "";
			for (auto el : elements) {
				ret += comma;
				comma = "\t";
				ret += el->to_key();
			}
			return ret;
		}

		virtual std::string to_json()
		{
			std::string ret = "[ ";
			std::string comma = "";
			for (auto el : elements) {
				ret += comma;
				comma = ", ";
				ret += el->to_json();
			}
			ret += " ]";
			return ret;
		}

		virtual std::string to_json_typed()
		{
			return to_json();
		}

		virtual std::string to_string()
		{
			return to_json();
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_array>();
			for (auto element : elements) {
				auto c = element->clone();
				t->elements.push_back(c);
			}
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json_object : public json_value
	{
	public:
		std::map<std::string, std::shared_ptr<json_value>> members;

		virtual std::string to_key()
		{
			std::string ret = "";
			std::string comma = "";
			for (auto el : members) {
				ret += comma;
				comma = "-";
				if (el.second) {
					ret += el.second->to_key();
				}
			}
			return ret;
		}

		virtual std::string to_json()
		{
			std::string ret = "{ ";
			std::string comma = "";
			for (auto el : members) {
				ret += comma;
				comma = ", ";
				ret += "\"" + el.first + "\"";
				ret += ":";
				if (el.second) {
					ret += el.second->to_json();
				}
			}
			ret += " }";
			return ret;
		}

		virtual std::string to_json_typed()
		{
			return to_json();
		}

		virtual std::string to_string()
		{
			return to_json();
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_object>();
			for (auto member : members) {
				if (member.second) {
					auto c = member.second->clone();
					t->members[member.first] = c;
				}
				else {
					t->members[member.first] = nullptr;
				}
			}
			t->comparison_index = comparison_index;
			return t;
		}
	};

	class json;

	using json_function_function = std::function<json(json parent, json params)>;

	class json_function : public json_value
	{
	public:

		json* function_this = {};
		json_function_function fn;

		virtual std::string to_key();
		virtual std::string to_json();
		virtual std::string to_json_typed();
		virtual std::string to_string();
		virtual std::string get_type_prefix();
		virtual json get_json();
		virtual std::shared_ptr<json_value> clone();

	};


	class json
	{
		using compared_item = std::tuple<int, std::string>;

		std::vector<compared_item> comparison_fields;

		std::shared_ptr<json_value> value_base;
		std::shared_ptr<json_double> double_impl;
		std::shared_ptr<json_string> string_impl;
		std::shared_ptr<json_array> array_impl;
		std::shared_ptr<json_int64> int64_impl;
		std::shared_ptr<json_datetime> datetime_impl;
		std::shared_ptr<json_blob> blob_impl;
		std::shared_ptr<json_object> object_impl;
		std::shared_ptr<json_function> function_impl;

	public:

		json()
		{
		}

		json(std::shared_ptr<json_value> _value)
		{
			set(_value);
		}

		void set(std::shared_ptr<json_value> _value)
		{
			value_base = _value;
			double_impl = std::dynamic_pointer_cast<json_double>(_value);
			string_impl = std::dynamic_pointer_cast<json_string>(_value);
			array_impl = std::dynamic_pointer_cast<json_array>(_value);
			object_impl = std::dynamic_pointer_cast<json_object>(_value);
			int64_impl = std::dynamic_pointer_cast<json_int64>(_value);
			datetime_impl = std::dynamic_pointer_cast<json_datetime>(_value);
			blob_impl = std::dynamic_pointer_cast<json_blob>(_value);
			object_impl = std::dynamic_pointer_cast<json_object>(_value);
			function_impl = std::dynamic_pointer_cast<json_function>(_value);
		}

		json clone()
		{
			json result(value_base->clone());
			return result;
		}

		std::string to_key()
		{
			return value_base->to_key();
		}

		std::string to_json()
		{
			return value_base ? value_base->to_json() : "";
		}

		std::string to_json_typed()
		{
			return value_base->to_json_typed();
		}

		std::string to_json_typed_string()
		{
			std::string json_str = to_json_typed();
			std::string escaped_json_str = "";

			for (auto in : json_str)
			{
				switch (in) {
				case '\\':
				case '"':
					escaped_json_str += '\\';
					escaped_json_str += in;
					break;
				case '\n':
					escaped_json_str += '\\';
					escaped_json_str += 'n';
					break;
				default:
					escaped_json_str += in;
					break;
				}
			}
			return escaped_json_str;
		}

		std::string to_json_string()
		{
			std::string json_str = to_json();
			std::string escaped_json_str = "";

			for (auto in : json_str)
			{
				switch (in) {
				case '\\':
				case '"':
					escaped_json_str += '\\';
					escaped_json_str += in;
					break;
				case '\n':
					escaped_json_str += '\\';
					escaped_json_str += 'n';
					break;
				default:
					escaped_json_str += in;
					break;
				}
			}
			return escaped_json_str;
		}

		bool is_int64() const
		{
			return (bool)int64_impl;
		}

		bool is_datetime() const
		{
			return (bool)datetime_impl;
		}

		bool is_blob() const
		{
			return (bool)blob_impl;
		}

		bool is_double() const
		{
			return (bool)double_impl;
		}

		bool is_string() const
		{
			return (bool)string_impl;
		}

		bool is_array() const
		{
			return (bool)array_impl;
		}

		bool is_object() const
		{
			return (bool)object_impl;
		}

		bool is_function() const
		{
			return (bool)function_impl;
		}

		bool is_empty() const
		{
			return value_base == nullptr;
		}


		int64_t get_int64s()  const
		{
			if (double_impl)
				return double_impl->value;
			else if (int64_impl)
				return int64_impl->value;
			else if (datetime_impl)
				return datetime_impl->value.get_time_t();
			else if (string_impl)
				return std::stod(string_impl->value);
			else
				return 0;
		}

		int64_t& get_int64()  const
		{
			return int64_impl->value;
		}

		date_time& get_time()  const
		{
			return datetime_impl->value;
		}

		double& get_double()  const
		{
			return double_impl->value;
		}

		std::string& get_string() const
		{
			return string_impl->value;
		}

		operator double() const
		{
			try
			{
				if (double_impl)
					return double_impl->value;
				else if (int64_impl)
					return int64_impl->value;
				else if (string_impl)
					return std::strtod(string_impl->value.c_str(), nullptr);
				else if (datetime_impl)
					return datetime_impl->value.get_time_t();
				else
					return 0.0;
			}
			catch (std::exception)
			{
				return 0.0;
			}
		}

		operator int() const
		{
			try
			{

				if (double_impl)
					return double_impl->value;
				else if (int64_impl)
					return int64_impl->value;
				else if (string_impl)
				{
					if (string_impl->value == "true")
						return 1;
					else
						return std::strtod(string_impl->value.c_str(), nullptr);
				}
				else if (datetime_impl)
					return 0;
				else
					return 0;
			}
			catch (std::exception)
			{
				return 0;
			}
		}

		operator date_time() const
		{
			if (datetime_impl)
			{
				return datetime_impl->value;
			}
			else
			{
				date_time temp;
				return temp;
			}
		}

		operator int64_t() const
		{
			if (double_impl)
				return double_impl->value;
			else if (int64_impl)
				return int64_impl->value;
			else if (datetime_impl)
				return datetime_impl->value.get_time_t();
			else if (string_impl)
				return std::stod(string_impl->value);
			else
				return 0;
		}

		operator std::string() const
		{
			if (double_impl)
				return std::format("{0}", double_impl->value);
			else if (int64_impl)
				return int64_impl->to_string();
			else if (datetime_impl)
				return datetime_impl->to_string();
			else if (string_impl)
				return string_impl->value;
			else if (blob_impl)
				return blob_impl->to_string();
			else
				return "";
		}


		explicit operator bool() const
		{
			bool value = false;
			if (is_double()) {
				value = double_impl->value != 0.0;
			}
			else if (is_int64()) {
				value = int64_impl->value != 0.0;
			}
			else if (is_string()) {
				value = string_impl->value == "true";
			}
			return value;
		}

		operator std::shared_ptr<json_array>& ()
		{
			return array_impl;
		}

		operator std::shared_ptr<json_object>& ()
		{
			return object_impl;
		}

		operator std::shared_ptr<json_blob>& ()
		{
			return blob_impl;
		}

		operator std::shared_ptr<json_function>& ()
		{
			return function_impl;
		}
		
		operator buffer()
		{
			if (!is_empty()) 
			{
				std::string temp_s = to_json();
				int sz = temp_s.size();
				buffer temp(sz + 1);
				char* dest = temp.get_ptr();
				const char* src_begin = temp_s.c_str();
				const char* src_end = temp_s.c_str() + sz + 1;
				std::copy(src_begin, src_end, dest);
				return temp;
			}
			buffer empty;
			return empty;
		}

		std::shared_ptr<json_value> operator ->()
		{
			return value_base;
		}

		json operator[](const std::string& _key) const
		{
			if (object_impl && object_impl->members.contains(_key)) {
				json jn(object_impl->members[_key]);
				return jn;
			}
			json empty;
			return empty;
		}

		json operator[](const char *_key) const
		{
			if (object_impl && object_impl->members.contains(_key)) {
				json jn(object_impl->members[_key]);
				return jn;
			}
			json empty;
			return empty;
		}

		void assign_update(json _member)
		{

			if (_member.is_empty())
			{
				return;
			}

			if (is_empty())
			{
				set(_member.value_base);
				return;
			}

			if (_member.is_object() && is_object()) 
			{
				auto members = _member.get_members_raw();

				for (auto src : members)
				{
					put_member_value(src.first, src.second);
				}
			}
		}

		void assign_replace(json _member)
		{
			set(_member.value_base);
		}

		bool has_member(std::string _key) const
		{
			bool has_value = !_key.empty() && object_impl && object_impl->members.contains(_key);
			return has_value;
		}

		bool is_member(std::string _key, const char *_value) const
		{
			bool has_value = object_impl && object_impl->members.contains(_key);
			if (has_value) {
				std::string svalue = object_impl->members[_key]->to_string();
				std::string xvalue = _value;
				has_value = svalue == xvalue;
			}
			return has_value;
		}

		bool is_member(std::string _key, std::string _value) const
		{
			bool has_value = object_impl && object_impl->members.contains(_key);
			if (has_value) {
				std::string svalue = object_impl->members[_key]->to_string();
				has_value = svalue == _value;
			}
			return has_value;
		}

		bool is_member(std::string _key, int64_t _value) const
		{
			bool has_value = object_impl && object_impl->members.contains(_key);
			if (has_value) {
				int64_t svalue = get_member(_key);
				has_value = svalue == _value;
			}
			return has_value;
		}

		bool is_member(std::string _key, bool _value) const
		{
			bool has_value = object_impl && object_impl->members.contains(_key);
			if (has_value) {
				bool svalue = (bool)get_member(_key);
				has_value = svalue == _value;
			}
			return has_value;
		}

		json get_first_element() const
		{
			return get_element(0);
		}

		json get_last_element() const
		{
			return get_element(array_impl->elements.size() - 1);
		}

		json get_element(int _index) const
		{
			json jn(array_impl->elements[_index]);
			return jn;
		}

		json get_member(std::string _key) const
		{
			json jn, jx;

			if (!object_impl) {
				throw std::logic_error("Not an object");
			}

			jn = json(object_impl->members[_key]);
			return jn;
		}

		int get_member_int(std::string _key) const
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			json jn(object_impl->members[_key]);
			double jnd = (double)jn;
			return jnd;
		}

		json copy_member(std::string _key, json& _source)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			std::string search_key = _key;
			if (_source.has_member(search_key)) {
				json member = _source.get_member(search_key);
				put_member(_key, member);
			}
			return *this;
		}

		void put_members(json& _member)
		{
			if (!object_impl) {
				throw std::logic_error("Target is not an object");
			}

			if (!_member.object_impl) {
				throw std::logic_error("Source is not an object");
			}

			auto members = _member.get_members_raw();

			for (auto src : members) 
			{
				put_member_value(src.first, src.second);
			}
		}

		void change_member_type(std::string _key, std::string _new_type)
		{
			if (!object_impl) 
			{
				throw std::logic_error("Not an object");
			}

			if (!has_member(_key))
			{
				throw std::logic_error("Changing type on a non-existent member");
			}

			if (_new_type == "object") {
				put_member_object(_key);
			}
			else if (_new_type == "array") {
				put_member_array(_key);
			}
			else if (_new_type == "string") {
				std::string s = get_member(_key);
				put_member(_key, s);
			}
			else if (_new_type == "int64") {
				int64_t i64 = get_member(_key).get_int64s();
				put_member_i64(_key, i64);
			}
			else if (_new_type == "double" || _new_type == "number") {
				double d = get_member(_key);
				put_member(_key, d);
			}
			else if (_new_type == "datetime") {
				date_time dt = get_member(_key);
				put_member(_key, dt);
			}
			else if (_new_type == "bool") {
				bool b = (bool)get_member(_key);
				put_member(_key, b);
			}
		}

		json put_member(std::string _key, json& _member)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			if (_member.is_array()) {
				put_member_array(_key, _member);
			}
			else if (_member.is_double()) {
				double d = _member;
				put_member(_key, d);
			}
			else if (_member.is_int64()) {
				int64_t d = _member;
				put_member_i64(_key, d);
			}
			else if (_member.is_datetime()) {
				date_time d = _member;
				put_member(_key, d);
			}
			else if (_member.is_object()) {
				put_member_object(_key, _member);
			}
			else if (_member.is_string()) {
				std::string d = _member;
				put_member(_key, d);
			}
			else if (_member.is_blob()) {
				std::string d = _member;
				put_member(_key, d);
			}
			else if (_member.is_function()) {
				put_member_function(_key, _member);
			}
			return *this;
		}

		json put_member(std::string _key, std::string _value)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member_i64(std::string _key,  int64_t _value)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_int64>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member(std::string _key, double _value)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_double>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member(std::string _key, date_time _value)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_datetime>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member_array(std::string _key)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_array>();
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member_object(std::string _key)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_object>();
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member_blob(std::string _key)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_object>();
			object_impl->members[_key] = new_member;
			return *this;
		}

		json put_member_array(std::string _key, json& _array)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_array> existing_array = _array;
			auto new_array = existing_array->clone();
			object_impl->members[_key] = new_array;
			return *this;
		}

		json put_member_value(std::string _key, std::shared_ptr<json_value> _obj)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			auto new_object = _obj->clone();
			object_impl->members[_key] = new_object;
			return *this;
		}

		json put_member_object(std::string _key, json& _object)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_object> existing_object = _object;
			auto new_object = existing_object->clone();
			object_impl->members[_key] = new_object;
			return *this;
		}

		json put_member_function(std::string _key, json& _object)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_function> existing_object = _object;
			auto new_object = existing_object->clone();
			object_impl->members[_key] = new_object;
			return *this;
		}

		json put_member_function(std::string _key, json_function_function fn)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_function> new_object = std::make_shared<json_function>();
			object_impl->members[_key] = new_object;
			return *this;
		}

		json erase_member(std::string _key)
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			object_impl->members.erase(_key);
			return *this;
		}

		json operator()(json _params)
		{
			if (!function_impl) {
				throw std::logic_error("Not a function");
			}
			return function_impl->fn(*function_impl->function_this, _params);
		}

		json set_natural_order()
		{
			comparison_fields.clear();

			int comparison_index = 1;
			for (auto m : object_impl->members) {
				if (m.second) {
					m.second->comparison_index = comparison_index++;
					compared_item sort_tuple = std::make_tuple(m.second->comparison_index, m.first);
					comparison_fields.push_back(sort_tuple);
				}
			}

			std::sort(comparison_fields.begin(), comparison_fields.end());

			return *this;
		}

		json set_compare_order(std::vector<std::string> _fields)
		{
			comparison_fields.clear();

			int comparison_index = 1;
			for (auto m : object_impl->members) {
				if (m.second) m.second->comparison_index = 0;
			}
			for (auto f : _fields)
			{
				if (object_impl->members.contains(f)) {
					auto fn = object_impl->members[f];
					fn->comparison_index = ++comparison_index;
				}
			}

			for (auto m : object_impl->members)
			{
				if (m.second->comparison_index) {
					compared_item sort_tuple = std::make_tuple(m.second->comparison_index, m.first);
					comparison_fields.push_back(sort_tuple);
				}
			}

			std::sort(comparison_fields.begin(), comparison_fields.end());

			return *this;
		}

		bool keys_compatible(std::vector<std::string> keys)
		{
			if (!object_impl)
			{
				throw std::logic_error("Not an object");
			}

			auto	index_keys = keys.begin();
			auto				   my_keys = comparison_fields.begin();

			int chk = 0;
			while (index_keys != std::end(keys))
			{
				if (my_keys != std::end(comparison_fields))
					return false;

				std::string ifx = std::get<1>(*my_keys);
				std::string ify = *index_keys;

				if (ifx != ify)
					return false;

				index_keys++;
				my_keys++;
			}
			
			return true;
		}

		json extract(std::vector<std::string> _fields)
		{
			if (!object_impl)
			{
				throw std::logic_error("Not an object");
			}

			json jn(std::make_shared<json_object>());

			int comparison_index = 1;
			for (auto f : _fields)
			{
				if (object_impl->members.contains(f)) {
					auto fn = object_impl->members[f];
					fn->comparison_index = ++comparison_index;
					jn.put_member_value(f, fn);
				}
			}

			jn.set_compare_order(_fields);

			return jn;
		}

		std::map<std::string, std::shared_ptr<json_value>> get_members_raw()
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			return object_impl->members;
		}

		std::map<std::string, json> get_members();

		template <typename element_type> json append_element(element_type et)
		{
			return put_element( -1, et);
		}

		json put_element(int _index, json &_element)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}

			if (_element.is_array()) {
				put_element_array(_index, _element);
			}
			else if (_element.is_double()) {
				double d = _element;
				put_element(_index, d);
			}
			else if (_element.is_int64()) {
				int64_t d = _element;
				put_element(_index, d);
			}
			else if (_element.is_datetime()) {
				date_time timex = _element;
				put_element(_index, timex);
			}
			else if (_element.is_object()) {
				put_element_object(_index, _element);
			}
			else if (_element.is_string()) {
				std::string d = _element;
				put_element(_index, d);
			}
			return *this;
		}

		json put_element(int _index, std::string _value)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;

			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, int64_t _value)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_int64>();
			new_member->value = _value;
			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, date_time _value)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_datetime>();
			new_member->value = _value;
			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, double _value)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_double>();
			new_member->value = _value;
			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_array(int _index)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_array>();
			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_object(int _index)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_object>();
			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_member);
			}
			else {
				array_impl->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_array(int _index, json& _array)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			std::shared_ptr<json_array> existing_array = _array;
			auto new_array = existing_array->clone();

			if (_index < 0 || _index >= array_impl->elements.size()) 
			{
				array_impl->elements.push_back(new_array);
			}
			else 
			{
				array_impl->elements[_index] = new_array;
			}
			return *this;
		}

		json put_element_object(int _index, json& _object)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			std::shared_ptr<json_object> existing_object = _object;
			auto new_object = existing_object->clone();

			if (_index < 0 || _index >= array_impl->elements.size()) {
				array_impl->elements.push_back(new_object);
			}
			else 
			{
				array_impl->elements[_index] = new_object;
			}
			return *this;
		}

		int compare(json& _item)
		{
			int comparison = 0;

			if (is_object() && !_item.is_object())
			{
				return 1;
			}

			if (!is_object() && _item.is_object())
			{
				return -1;
			}

			if (!is_object() && !_item.is_object()) 
			{
				throw std::logic_error("At least one of the being compared must be json_object (not array or value, yet)");
			}


			for (auto m : comparison_fields)
			{
				std::string key = std::get<1>(m);
				auto member_value = object_impl->members[key];

				if (!_item.has_member(key)) 
				{
					return 1;
				}				

				auto member_dest = _item[key];
				auto member_src = json(member_value);

				if (member_src.is_string()) 
				{
					std::string tst_src, tst_dst;
					tst_src = member_src;
					tst_dst = member_dest;

					comparison = _stricmp(tst_src.c_str(), tst_dst.c_str());
				}
				else if (member_src.is_int64())
				{
					int64_t itst_src, itst_dst;
					itst_src = (int64_t)member_src;
					itst_dst = (int64_t)member_dest;

					if (itst_src < itst_dst) {
						comparison = -1;
					}
					else if (itst_src > itst_dst) {
						comparison = 1;
					}
					else if (itst_src == itst_dst) {
						comparison = 0;
					}
				}
				else if (member_src.is_double())
				{
					double dtst_src, dtst_dst;
					dtst_src = (double)member_src;
					dtst_dst = (double)member_dest;

					if (dtst_src < dtst_dst) {
						comparison = -1;
					}
					else if (dtst_src > dtst_dst) {
						comparison = 1;
					}
					else if (dtst_src == dtst_dst) {
						comparison = 0;
					}
				}
				else if (member_src.is_datetime())
				{
					date_time dtst_src = (date_time)member_src;
					date_time dtst_dst = (date_time)member_dest;

					comparison = dtst_src.compare(dtst_dst);
				}
				else if (member_src.is_object() || member_src.is_array() || member_src.is_function())
				{
					json dtst_src, dtst_dst;
					dtst_src = member_src;
					dtst_dst = member_dest;
					comparison = dtst_src.compare(dtst_dst);
				}

				if (comparison) {
					return comparison;
				}
			}

			return 0;
		}

		json for_each_member(std::function<void(const std::string& _key_name)> _transform)
		{
			if (!object_impl) {
				throw std::logic_error("Not an array");
			}
			for (auto m : object_impl->members)
			{
				_transform(m.first);
			}
			return *this;
		}

		json for_each_element(std::function<void(json& _item)> _transform)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				_transform(element);
			}
			return *this;
		}

		bool any(std::function<bool(json& _item)> _where_clause)
		{
			if (object_impl) {
				auto members = get_members();
				for (auto m : members)
				{
					if (_where_clause(m.second)) {
						return true;
					}
				}
			}
			else if (array_impl) 
			{
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (_where_clause(element)) {
						return true;
					}
				}
			}
			else
			{
				throw std::logic_error("Not an array or an object");
			}
			return false;
		}

		bool all(std::function<bool(json& _item)> _where_clause)
		{
			if (object_impl) {
				auto members = get_members();
				for (auto m : members)
				{
					if (!_where_clause(m.second)) {
						return false;
					}
				}
			}
			else if (array_impl)
			{
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (!_where_clause(element)) {
						return false;
					}
				}
			}
			else
			{
				throw std::logic_error("Not an array or an object");
			}
			return true;
		}

		json filter(std::function<bool(json& _item)> _where_clause)
		{
			json result_item;

			if (object_impl) {
				result_item = json(std::make_shared<json_object>());
				auto members = get_members();
				for (auto m : members)
				{
					if (_where_clause(m.second)) {
						json j = m.second.clone();
						result_item.put_member(m.first, j);
					}
				}
			}
			else if (array_impl)
			{
				result_item = json(std::make_shared<json_array>());
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (_where_clause(element)) {
						json jnew = element->clone();
						result_item.put_element(-1, jnew);
					}
				}
			}
			else
			{
				throw std::logic_error("Not an array or an object");
			}

			return result_item;
		}

		json array_to_object(std::function<std::string(json& _item)> _get_key,
			std::function<json(json& _target)> _get_payload)
		{
			json result_item;
			result_item = json(std::make_shared<json_object>());

			if (array_impl)
			{
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (element.is_object()) {
						std::string key = _get_key(element);
						json new_item = _get_payload(element);
						if (result_item.has_member(key)) 
						{
							result_item[key].append_element(new_item);
						}
						else 
						{
							result_item.put_member_array(key);
							result_item[key].append_element(new_item);
						}
					}
				}
			}
			else {
				throw std::exception("Item is not array");
			}
			return result_item;
		}

		json object_to_array(std::function<json(std::string _member_name, json& _src)> _get_payload)
		{
			json result_item;
			result_item = json(std::make_shared<json_array>());

			if (object_impl)
			{
				auto members = get_members();
				for (auto member: members)
				{
					json new_object = _get_payload(member.first, member.second);
					if (!new_object.is_empty()) {
						result_item.append_element(new_object);
					}
				}
			}
			else {
				throw std::exception("Item is not array");
			}
			return result_item;
		}


		json map(std::function<json(std::string _member, int _index, json& _item)> _transform)
		{
			json result_item;

			if (array_impl) 
			{
				result_item = json(std::make_shared<json_array>());
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					auto new_element = _transform("", i, element);
					if (new_element.is_array()) {
						for (int i = 0; i < new_element.size(); i++) {
							json j = new_element.get_element(i);
							result_item.append_element( j);
						}
					}
					else 
					{
						result_item.append_element(new_element);
					}
				}
			}
			else if (object_impl)
			{
				result_item = json(std::make_shared<json_object>());
				auto members = get_members();
				for (auto member : members)
				{
					auto new_member = _transform(member.first, 0, member.second);
					if (new_member.is_object()) {
						auto members = new_member.get_members();
						for (auto member : members) {
							if (!result_item.has_member(member.first)) {
								result_item.put_member_array(member.first);
							}
							result_item[member.first].append_element(member.second);
						}
					}
					else
					{
						result_item.put_member(member.first, new_member);
					}
				}
			}

			return result_item;
		}

		json update(std::function<json&(json& _item)> _transform)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				auto new_element = _transform(element);
				put_element(1, new_element);
			}
			return *this;
		}

		json join(json& _right, 
			std::function<bool(json&_item1, json&_item2)> _compare,
			std::function<json(json& _item1, json& _item2)> _compose
			)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			json new_array(std::make_shared<json_array>());

			for (int i = 0; i < size(); i++)
			{
				json left_item = get_element(i);
				for (int j = 0; j < _right.size(); j++)
				{
					json right_item = _right.get_element(j);
					if (_compare(left_item, right_item)) {
						json new_item = _compose(left_item, right_item);
						new_array.put_element(-1, new_item);
					}
				}
			}

			return new_array;
		}

		json join(json& _right,
			std::function<std::string(json& _item)> _get_key_left,
			std::function<std::string(json& _item)> _get_key_right,
			std::function<json(json& _item1, json& _item2)> _compose
		)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}

			int i, j;

			class match_set
			{
			public:
				std::vector<int> left;
				std::vector<int> right;
			};

			std::map<std::string, match_set> join_items;

			json new_array(std::make_shared<json_array>());

			for (i = 0; i < size(); i++)
			{
				json left_item = get_element(i);				
				std::string key = _get_key_left(left_item);
				if (!join_items.contains(key)) {
					match_set m;
					join_items[key] = m;
				}
				join_items[key].left.push_back(i);
			}

			for (j = 0; j < _right.size(); j++)
			{
				json right_item = _right.get_element(j);
				std::string key = _get_key_right(right_item);
				if (!join_items.contains(key)) {
					match_set m;
					join_items[key] = m;
				}
				join_items[key].left.push_back(j);
			}

			for (auto join_item : join_items)
			{
				for (i = 0; i < join_item.second.left.size(); i++) 
				{
					auto left_id = join_item.second.left[i];
					auto left_item = get_element(i);

					for (j = 0; i < join_item.second.right.size(); j++)
					{
						auto right_id = join_item.second.right[i];
						auto right_item = get_element(i);
						auto new_item = _compose(left_item, right_item);
						new_array.append_element(new_item);
					}
				}
			}

			return new_array;
		}

		json group(std::function<std::string(json& _item)> _get_group)
		{
			if (!array_impl) 
			{
				throw std::logic_error("Not an array");
			}

			json new_object(std::make_shared<json_object>());

			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				std::string key = _get_group(element);
				if (!new_object.has_member(key)) {
					new_object.put_member_array(key);
				}
				new_object[key.c_str()].put_element(-1, element);
			}

			return new_object;
		}

		int size() const
		{
			if (object_impl)
				return object_impl->members.size();
			else if (array_impl)
				return array_impl->elements.size();
			else
				return 1;
		}

	};

	std::map<std::string, json> json::get_members()
	{
		if (!object_impl) {
			throw std::logic_error("Not an object");
		}
		std::map<std::string, json> mp;
		for (auto m : object_impl->members)
		{
			json jx(m.second);
			mp.insert_or_assign(m.first, jx);
		}
		return mp;
	}

	class json_parser
	{
	private:

		int line_number = 1;
		int char_index = 0;

		json get_errors()
		{
			json error_root(std::make_shared<json_object>());

			error_root.put_member("ClassName", "SysParseError");
			error_root.put_member("Success", false);

			json error_array(std::make_shared<json_array>());

			for (auto parse_error : parse_errors)
			{
				json err(std::make_shared<json_object>());
				err.put_member("line", parse_error.line);
				err.put_member("char", parse_error.char_index);
				err.put_member("topic", parse_error.topic);
				err.put_member("error", parse_error.error);
				error_array.append_element( err);
			}

			error_root.put_member("errors", error_array);
			return error_root;
		}

	public:

		class parse_message
		{
		public:
			int line;
			int char_index;
			std::string topic;
			std::string error;
		};

		std::vector<parse_message> parse_errors;

		json parse_object(std::string _object)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			const char* start = _object.c_str();
			if (parse_object(jo, start, &start)) {
				json jn(jo);
				return jn;
			}
			else {
				return get_errors();
			}
		}

		json parse_object(http_response _response)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			if (_response.content_type != "application/json") {
				error("http_response", "content type is not json");
				return get_errors();
			}
			if (_response.response_body.get_size() == 0) {
				error("http_response", "response had no body");
				return get_errors();
			}

			if (_response.response_body.is_safe_string()) {
				json temp = parse_object(_response.response_body.get_ptr());
				return temp;
			}
			else {
				error("http_response", "response was not a valid string");
				return get_errors();
			}
		}

		json create_object()
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json jn(jo);
			return jn;
		}

		json create_object(std::string _name, std::string _value)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json jn(jo);
			jn.put_member(_name, _value);
			return jn;
		}

		json create_object(std::string _name, double _value)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json jn(jo);
			jn.put_member(_name, _value);
			return jn;
		}

		json create_object(std::string _name, int64_t _value)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json jn(jo);
			jn.put_member(_name, _value);
			return jn;
		}

		json parse_array(std::string _object)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_array>();
			const char* start = _object.c_str();
			if (parse_array(jo, start, &start)) {
				json jn(jo);
				return jn;
			}
			else {
				return get_errors();
			}
		}

		json create_array()
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_array>();
			json jn(jo);
			return jn;
		}

		std::shared_ptr<json_value> parse_value(std::shared_ptr<json_object> _object, std::string _name, std::string _value)
		{
			const char* start = _value.c_str();
			auto existing = _object->members.contains(_name);
			std::shared_ptr<json_value> modified_value;
			if (parse_value(modified_value, start, &start)) {
				_object->members[_name] = modified_value;
			}
			return modified_value;
		}

	private:

		void check_line(const char* nl)
		{
			char_index++;
			if (*nl == '\r') {
				line_number++;
				char_index = 0;
			}
		}

		void error(std::string _topic, std::string _message)
		{
			parse_message new_message = { line_number, char_index, _topic, _message };
			parse_errors.push_back(new_message);
		}

		const char* eat_white(const char* _src)
		{
			while (std::isspace(*_src))
				_src++;
			return _src;
		}

		bool parse_member_type(std::string& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (*_src == '$')
			{
				_src++;
				result = true;
				while (isalpha(*_src))
				{
					check_line(_src);
					temp += *_src;
					_src++;
				}
				*_modified = _src;
				_result = temp;
			}
			return result;
		}

		bool parse_string(std::string& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (*_src == '"')
			{
				_src++;
				result = true;
				bool parsing = false;
				while (*_src != '"')
				{
					check_line(_src);
					if (*_src < 0 || std::iscntrl(*_src))
					{
						_src++;
						continue;
					}
					else if (*_src == '\\')
					{
						_src++;
						switch (*_src)
						{
						case '"':
							temp += '"';
							break;
						case '\\':
							temp += '\\';
							break;
						case '/':
							temp += '/';
							break;
						case 'b':
							temp += 0x8;
							break;
						case 'f':
							temp += 12;
							break;
						case 'n':
							temp += 10;
							break;
						case 'r':
							temp += 13;
							break;
						case 't':
							temp += 9;
							break;
						case 'u':
							break;
						}
					}
					else
					{
						temp += *_src;
					}
					_src++;
				}
				_result = temp;
				_src++;
			}
			*_modified = _src;
			return result;
		}

		bool parse_number(double& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isdigit(*_src) || *_src == '.' || *_src == '-')
			{
				std::string temp = "";
				result = true;
				while (isdigit(*_src) || *_src == '.' || *_src == '_' || *_src == '-')
				{
					check_line(_src);
					if (*_src != '_') {
						temp += *_src;
					}
					_src++;
				}
				_result = std::strtod(temp.c_str(), nullptr);
				result = true;
			}
			*_modified = _src;
			return result;
		}

		bool parse_int64(int64_t& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isdigit(*_src) || *_src == '-')
			{
				std::string temp = "";
				result = true;
				while (isdigit(*_src) || *_src == '_' || *_src == '-')
				{
					check_line(_src);
					if (*_src != '_') {
						temp += *_src;
					}
					_src++;
				}
				_result = std::strtoll(temp.c_str(), nullptr, 10);
				result = true;
			}
			*_modified = _src;
			return result;
		}

		bool parse_boolean(double& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isalpha(*_src))
			{
				std::string temp = "";
				result = true;
				while (isalnum(*_src) || *_src == '_')
				{
					check_line(_src);
					temp += *_src;
					_src++;
				}
				if (temp == "true") 
				{
					_result = 1.0;
					result = true;
				}
				else if (temp == "false") 
				{
					_result = 0.0;
					result = true;
				}
				else
				{
					result = false;
				}
				
			}
			*_modified = _src;
			return result;
		}

		bool parse_null(const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isalpha(*_src))
			{
				std::string temp = "";
				result = true;
				while (isalnum(*_src) || *_src == '_')
				{
					check_line(_src);
					temp += *_src;
					_src++;
				}
				if (temp == "null")
				{
					result = true;
				}
				else
				{
					result = false;
				}
			}
			*_modified = _src;
			return result;
		}

		bool parse_value(std::shared_ptr<json_value>& _value, const char* _src, const char** _modified)
		{
			std::shared_ptr<json_array> new_array_value;
			std::shared_ptr<json_object> new_object_value;
			std::string new_string_value;
			int64_t new_int64_value;
			double new_number_value;
			bool result = true;

			const char* new_src = _src;

			std::string member_type;
			if (parse_member_type(member_type, _src, &new_src))
			{
				if (member_type == "$double")
				{
					if (parse_number(new_number_value, _src, &new_src))
					{
						auto js = std::make_shared<json_double>();
						js->value = new_number_value;
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "$datetime")
				{
					if (parse_int64(new_int64_value, _src, &new_src))
					{
						auto js = std::make_shared<json_int64>();
						js->value = new_int64_value;
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "$int64")
				{
					if (parse_int64(new_int64_value, _src, &new_src))
					{
						auto js = std::make_shared<json_int64>();
						js->value = new_int64_value;
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "$blob")
				{
					if (parse_string(new_string_value, _src, &new_src))
					{
						auto js = std::make_shared<json_blob>();
						js->from_string(new_string_value);
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "$string")
				{
					if (parse_string(new_string_value, _src, &new_src))
					{
						auto js = std::make_shared<json_string>();
						js->value = new_string_value;
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "$function")
				{
					if (parse_string(new_string_value, _src, &new_src))
					{
						auto js = std::make_shared<json_function>();
						*_modified = new_src;
						return result;
					}
				}

				result = false;
				member_type += " invalid";
				error("parse_value", member_type);
				*_modified = new_src;
				return result;
			}

			if (parse_string(new_string_value, _src, &new_src))
			{
				auto js = std::make_shared<json_string>();
				js->value = new_string_value;
				_value = js;
			}
			else if (parse_number(new_number_value, _src, &new_src))
			{
				auto js = std::make_shared<json_double>();
				js->value = new_number_value;
				_value = js;
			}
			else if (parse_array(new_array_value, _src, &new_src))
			{
				_value = new_array_value;
			}
			else if (parse_object(new_object_value, _src, &new_src))
			{
				_value = new_object_value;
			}
			else if (parse_null(_src, &new_src))
			{
				// don't put anything into it... it's null!
			}
			else if (parse_boolean(new_number_value, _src, &new_src))
			{
				auto js = std::make_shared<json_double>();
				js->value = new_number_value;
				_value = js;
			}
			else
			{
				result = false;
				error("parse_value", "Invalid value.");
			}
			*_modified = new_src;
			return result;
		}

		bool parse_array(std::shared_ptr<json_array>& _array, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			std::string temp = "";

			if (*_src == '[')
			{
				result = true;

				if (!_array) {
					_array = std::make_shared<json_array>();
				}
				_src++;
				while (*_src && *_src != ']') {
					check_line(_src);
					std::shared_ptr<json_value> value;
					if (parse_value(value, _src, &_src)) {
						_array->elements.push_back(value);
					}
					_src = eat_white(_src);
					if (*_src == ',') {
						_src++;
						_src = eat_white(_src);
					}
					else if (*_src == ']')
					{
					}
					else if (*_src)
					{
						_src++;
						_src = eat_white(_src);
					}
					else 
					{
						error("parse_array", "Comma or end of array expected.");
						return false;
					}
				}
				_src++;
			}
			*_modified = _src;
			return result;
		}

		bool parse_object(std::shared_ptr<json_object>& _object, const char* _src, const char** _modified)
		{
			int comparison_index = 0;
			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (*_src == '{')
			{
				_src++;
				_src = eat_white(_src);
				result = true;
				_object = std::make_shared<json_object>();
				std::string member_name;
				enum parse_object_states {
					parsing_name,
					parsing_colon,
					parsing_value,
					parsing_comma
				};
				parse_object_states parse_object_state = parse_object_states::parsing_name;
				while (*_src && *_src != '}')
				{
					check_line(_src);
					if (parse_object_state == parse_object_states::parsing_name) {
						if (parse_string(member_name, _src, &_src)) {
							parse_object_state = parse_object_states::parsing_colon;
						}
						else
						{
							error("parse_member_name", "Invalid member name.");
							return false;
						}
					}
					else if (parse_object_state == parse_object_states::parsing_colon)
					{
						_src = eat_white(_src);
						if (*_src == ':') {
							parse_object_state = parse_object_states::parsing_value;
							_src++;
						}
						else
						{
							error("parse_colon", std::format("Expected colon for  \"{0}\".", member_name));
							return false;
						}
					}
					else if (parse_object_state == parse_object_states::parsing_value)
					{
						std::shared_ptr<json_value> member_value;
						if (parse_value(member_value, _src, &_src)) {
							parse_object_state = parse_object_states::parsing_comma;
							member_value->comparison_index = ++comparison_index;
							_object->members[member_name] = member_value;
						}
						else
						{
							error("parse_value", std::format("Invalid value for \"{0}\".", member_name));
							return false;
						}
					}
					else if (parse_object_state == parse_object_states::parsing_comma)
					{
						_src = eat_white(_src);
						if (*_src == ',') {
							_src++;
						}
						else if (*_src == '}')
						{
							continue;
						}
						else
						{
							error("parse_value", std::format("Expected a comma\".", member_name));
							return false;
						}
						parse_object_state = parse_object_states::parsing_name;
					}
					_src = eat_white(_src);
				}
				_src++;
			}
			*_modified = _src;
			return result;
		}
	};

	json json_function::get_json()
	{
		json default_params;
		json result = fn(*function_this, default_params);
		return result;
	}

	std::string json_function::to_key()
	{
		json default_params;
		json result = fn(*function_this, default_params);
		return result.to_key();
	}
	std::string json_function::to_json()
	{
		json default_params;
		json result = fn(*function_this, default_params);
		return result.to_json();
	}
	std::string json_function::to_json_typed()
	{
		json default_params;
		json result = fn(*function_this, default_params);
		return result.to_json_typed();
	}

	std::string json_function::to_string()
	{
		json default_params;
		json result = fn(*function_this, default_params);
		return result.to_json_typed_string();
	}

	std::string json_function::get_type_prefix()
	{
		return "$function";
	}

	std::shared_ptr<json_value> json_function::clone()
	{
		auto t = std::make_shared<json_function>();
		t->fn = fn;
		return t;
	}
}

corona::json operator ""_json_array(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json operator ""_json_object(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

corona::json operator ""_json_array(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json operator ""_json_object(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

namespace corona 
{

	bool test_json_parser()
	{
		bool success = true;

		corona::json_parser jp;

		corona::json empty;

		corona::json test_eq1 = jp.parse_object(R"({ "name":"bill", "age":42 })");
		corona::json test_eq2 = jp.parse_object(R"({ "name":"bill", "age":42 })");

		if (test_eq1.has_member("box"))
		{
			std::cout << "negative membership test failed" << std::endl;
			success = false;
		}

		if (test_eq1.has_member("bill"))
		{
			std::cout << "positive membership basic test failed" << std::endl;
			success = false;
		}

		if (!test_eq1.has_member("age"))
		{
			std::cout << "positive membership extent test failed" << std::endl;
			success = false;
		}

		if (empty.compare(test_eq1) >= 0) 
		{
			std::cout << "empty < test_eq1 failed" << std::endl;
			success = false;
		}

		if (test_eq1.compare(test_eq2) != 0)
		{
			std::cout << "test_eq1 == test_eq1 failed" << std::endl;
			success = false;
		}

		test_eq1.set_compare_order({ "name", "age" });
		if (test_eq1.compare(test_eq2) != 0)
		{
			std::cout << "test_eq1 == test_eq1 failed" << std::endl;
			success = false;
		}

		test_eq2.set_compare_order({ "name", "age" });
		if (test_eq2.compare(test_eq1) != 0)
		{
			std::cout << "test_eq1 == test_eq1 failed" << std::endl;
			success = false;
		}

		corona::json test_lt1 = jp.parse_object(R"({ "name":"zak", "age":34 })");
		corona::json test_lt2 = jp.parse_object(R"({ "name":"zak", "age":37 })");
		test_lt1.set_compare_order({ "name", "age" });

		if (test_lt1.compare(test_lt2) >= 0)
		{
			std::cout << "test_lt1 < test_lt2 failed" << std::endl;
			success = false;
		}

		test_lt2.set_compare_order({ "name", "age" });

		if (test_lt2.compare(test_lt1) < 0)
		{
			std::cout << "test_lt2 < test_lt1 failed" << std::endl;
			success = false;
		}

		corona::json test_array = jp.parse_array(R"(
[
{ "name":"holly", "age":37, "sex":"female" },
{ "name":"susan", "age":22, "sex":"female" },
{ "name":"tina", "age":19, "sex":"female" },
{ "name":"kirsten", "age":19, "sex":"female" },
{ "name":"cheri", "age":22, "sex":"female" },
{ "name":"dorothy", "age":22, "sex":"female" },
{ "name":"leila", "age":25, "sex":"female" },
{ "name":"dennis", "age":40, "sex":"male" },
{ "name":"kevin", "age":44, "sex":"male" },
{ "name":"kirk", "age":42, "sex":"male" },
{ "name":"dan", "age":25, "sex":"male" },
{ "name":"peter", "age":33, "sex":"male" }
])");

		if (!test_array.any([](json& _item) {
			return (double)_item["age"] > 35;
			}))
		{
			std::cout << "any failed" << std::endl;
		}

		if (!test_array.all([](json& _item) {
			return (double)_item["age"] > 17;
			}))
		{
			std::cout << "all failed" << std::endl;
		}

		json test_array_group = test_array.array_to_object(
			[](json& _item)->std::string {
				double age = _item["age"];
				if (age < 21) 
				{
					return "teen";
				}
				else if (age < 30) 
				{
					return "adult";
				}
				else 
				{
					return "middle";
				}
			},
			[](json& _item)->json {
				return _item;
			}
			);

		std::cout << test_array_group.to_json() << std::endl;

		return success;
	}

}

/* todo, unit tests for the json parser, in particular, compare */

#endif
