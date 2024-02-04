#ifndef CORONA_JSON_H
#define CORONA_JSON_H

#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <stdexcept>
#include <memory>
#include <format>

namespace corona 
{

	class json_value
	{
	public:

		json_value()
		{
			;
		}
		virtual ~json_value()
		{
			;
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
		virtual std::shared_ptr<json_value> clone()
		{
			return std::make_shared<json_value>();
		}
	};

	class json_double : public json_value
	{
	public:
		double value;

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
			return t;
		}
	};

	class json_datetime : public json_value
	{
	public:
		LARGE_INTEGER value;

		virtual std::string to_json()
		{
			return std::to_string(value.QuadPart);
		}
		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::string to_string()
		{
			return std::format("{}", value.QuadPart);
		}
		virtual std::string get_type_prefix()
		{
			return "$datetime";
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_datetime>();
			t->value = value;
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
			return t;
		}
	};

	class json_int64 : public json_value
	{
	public:
		int64_t value;

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
			return t;
		}
	};

	class json_string : public json_value
	{
	public:
		std::string value;

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
			return t;
		}
	};

	class json_array : public json_value
	{
	public:
		std::vector<std::shared_ptr<json_value>> elements;

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
			return t;
		}
	};

	class json_object : public json_value
	{
	public:
		std::map<std::string, std::shared_ptr<json_value>> members;

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
			return t;
		}
	};

	class json
	{
		std::shared_ptr<json_value> value_base;
		std::shared_ptr<json_double> double_impl;
		std::shared_ptr<json_string> string_impl;
		std::shared_ptr<json_array> array_impl;
		std::shared_ptr<json_int64> int64_impl;
		std::shared_ptr<json_datetime> datetime_impl;
		std::shared_ptr<json_blob> blob_impl;
		std::shared_ptr<json_object> object_impl;

	public:

		json()
		{
		}

		json(std::shared_ptr<json_value> _value)
		{
			value_base = _value;
			double_impl = std::dynamic_pointer_cast<json_double>(_value);
			string_impl = std::dynamic_pointer_cast<json_string>(_value);
			array_impl = std::dynamic_pointer_cast<json_array>(_value);
			object_impl = std::dynamic_pointer_cast<json_object>(_value);
			int64_impl = std::dynamic_pointer_cast<json_int64>(_value);
			datetime_impl = std::dynamic_pointer_cast<json_datetime>(_value);
			blob_impl = std::dynamic_pointer_cast<json_blob>(_value);
		}

		std::string to_json()
		{
			return value_base->to_json();
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

		bool is_empty() const
		{
			return value_base != nullptr;
		}

		operator bool() const
		{
			bool value = false;
			if (is_double()) {
				value = double_impl->value != 0.0;
			}
			else if (is_string()) {
				value = string_impl->value == "true";
			}
			return value;
		}

		int64_t& get_int64()  const
		{
			return int64_impl->value;
		}

		LARGE_INTEGER& get_time()  const
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
			if (double_impl)
				return double_impl->value;
			else if (string_impl)
				return std::stod(string_impl->value);
			else if (datetime_impl)
				return datetime_impl->value.QuadPart;
			else
				return 0.0;
		}

		operator LARGE_INTEGER() const
		{
			if (datetime_impl)
			{
				return datetime_impl->value;
			}
			else 
			{
				LARGE_INTEGER t;
				t.QuadPart = (int64_t)*this;
				return t;
			}
		}

		operator int64_t() const
		{
			if (double_impl)
				return double_impl->value;
			else if (int64_impl)
				return int64_impl->value;
			else if (datetime_impl)
				return datetime_impl->value.QuadPart;
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

		std::shared_ptr<json_object> operator ->()
		{
			return object_impl;
		}

		json operator[](int _index) const
		{
			json jn(array_impl->elements[_index]);
			return jn;
		}

		json operator[](const std::string& _key) const
		{
			json jn(object_impl->members[_key]);
			return jn;
		}

		json operator[](const char *_key) const
		{
			json jn(object_impl->members[_key]);
			return jn;
		}

		bool has_member(std::string _key) const
		{
			bool has_value = object_impl && object_impl->members.contains(_key);
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
				put_member(_key, d);
			}
			else if (_member.is_datetime()) {
				int64_t d = _member;
				put_member(_key, d);
			}
			else if (_member.is_object()) {
				put_member_object(_key, _member);
			}
			else if (_member.is_string()) {
				std::string d = _member;
				put_member(_key, d);
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

		std::map<std::string, std::shared_ptr<json_value>> get_members()
		{
			if (!object_impl) {
				throw std::logic_error("Not an object");
			}
			return object_impl->members;
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
				LARGE_INTEGER timex = _element;
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

		json put_element(int _index, LARGE_INTEGER _value)
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

		json for_each(std::function<void(json& _item)> _transform)
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
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			json new_array(std::make_shared<json_array>());
			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				if (_where_clause(element)) {
					return true;
				}
			}
			return false;
		}

		json filter(std::function<bool(json& _item)> _where_clause)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			json new_array( std::make_shared<json_array>() );
			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				if (_where_clause(element)) {
					json jnew = element->clone();
					new_array.put_element(-1, jnew );
				}
			}
			return new_array;
		}

		json map(std::function<json(json& _item)> _transform)
		{
			if (!array_impl) {
				throw std::logic_error("Not an array");
			}
			json new_array(std::make_shared<json_array>());
			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				auto new_element = _transform(element);
				new_array.put_element(-1, new_element);
			}
			return new_array;
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
			std::function<std::string(json& _item)> _get_key,
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
				std::string key = _get_key(left_item);
				if (!join_items.contains(key)) {
					match_set m;
					join_items[key] = m;
				}
				join_items[key].left.push_back(i);
			}

			for (j = 0; j < _right.size(); j++)
			{
				json right_item = _right.get_element(j);
				std::string key = _get_key(right_item);
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
						new_array.put_element(-1, new_item);
					}
				}
			}

			return new_array;
		}

		json group(std::function<std::string(json& _item)> _get_group)
		{
			if (!array_impl) {
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

	class json_parser
	{
	private:

		int line_number = 1;

		json get_errors()
		{
			json error_root(std::make_shared<json_object>());

			error_root.put_member("success", false);

			json error_array(std::make_shared<json_array>());

			for (auto parse_error : parse_errors)
			{
				json err(std::make_shared<json_object>());
				err.put_member("line", parse_error.line);
				err.put_member("topic", parse_error.topic);
				err.put_member("error", parse_error.error);
				error_array.put_element(-1, err);
			}

			error_root.put_member("errors", error_array);
			return error_root;
		}

	public:

		class parse_message
		{
		public:
			int line;
			std::string topic;
			std::string error;
		};

		std::vector<parse_message> parse_errors;

		json parse_object(std::string _object)
		{
			line_number = 1;
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

		json create_object()
		{
			line_number = 1;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json jn(jo);
			return jn;
		}

		json parse_array(std::string _object)
		{
			line_number = 1;
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
			if (*nl == '\r')
				line_number++;
		}

		void error(std::string _topic, std::string _message)
		{
			parse_message new_message = { line_number, _topic, _message };
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

			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (*_src == '{')
			{
				_src++;
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
						else
						{
							error("parse_value", std::format("Expected a comma\".", member_name));
							return false;
						}
						parse_object_state = parse_object_states::parsing_name;
					}
				}
				_src++;
			}
			*_modified = _src;
			return result;
		}
	};


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

#endif
