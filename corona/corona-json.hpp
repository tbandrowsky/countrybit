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
		virtual std::string to_string()
		{
			return "";
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
		virtual std::string to_string()
		{
			return std::to_string(value);
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
		virtual std::string to_string()
		{
			return value;
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

		virtual std::string to_string()
		{
			return to_json();
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
				ret += el.second->to_json();
			}
			ret += " }";
			return ret;
		}

		virtual std::string to_string()
		{
			return to_json();
		}

	};

	class json_navigator
	{
		std::shared_ptr<json_value> value_base;
		std::shared_ptr<json_double> double_impl;
		std::shared_ptr<json_string> string_impl;
		std::shared_ptr<json_array> array_impl;
		std::shared_ptr<json_object> object_impl;

	public:

		json_navigator()
		{
		}

		json_navigator(std::shared_ptr<json_value> _value)
		{
			value_base = _value;
			double_impl = std::dynamic_pointer_cast<json_double>(_value);
			string_impl = std::dynamic_pointer_cast<json_string>(_value);
			array_impl = std::dynamic_pointer_cast<json_array>(_value);
			object_impl = std::dynamic_pointer_cast<json_object>(_value);
		}

		std::string to_json()
		{
			return value_base->to_json();
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

		operator bool() const
		{
			return value_base != nullptr;
		}

		explicit operator double& ()  const
		{
			return double_impl->value;
		}

		explicit operator std::string& () const
		{
			return string_impl->value;
		}

		operator double() const
		{
			if (double_impl)
				return double_impl->value;
			else if (string_impl)
				return std::stod(string_impl->value);
			else
				return 0.0;
		}

		operator std::string() const
		{
			if (double_impl)
				return std::format("{0}", double_impl->value);
			else if (string_impl)
				return string_impl->value;
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

		std::shared_ptr<json_object> operator ->()
		{
			return object_impl;
		}

		json_navigator operator[](int _index) const
		{
			json_navigator jn(array_impl->elements[_index]);
			return jn;
		}

		json_navigator operator[](std::string _key) const
		{
			json_navigator jn(object_impl->members[_key]);
			return jn;
		}

		bool has_member(std::string _key) const
		{
			bool has_value = object_impl->members.contains(_key);
			return has_value;
		}

		json_navigator get_element(int _index) const
		{
			json_navigator jn(array_impl->elements[_index]);
			return jn;
		}

		json_navigator get_member(std::string _key) const
		{
			json_navigator jn(object_impl->members[_key]);
			return jn;
		}

		json_navigator add_member(std::string _key, std::string _value)
		{
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json_navigator add_member(std::string _key, double _value)
		{
			auto new_member = std::make_shared<json_double>();
			new_member->value = _value;
			object_impl->members[_key] = new_member;
			return *this;
		}

		json_navigator add_member_array(std::string _key)
		{
			auto new_member = std::make_shared<json_array>();
			object_impl->members[_key] = new_member;
			return *this;
		}

		json_navigator add_member_object(std::string _key)
		{
			auto new_member = std::make_shared<json_object>();
			object_impl->members[_key] = new_member;
			return *this;
		}

		std::map<std::string, std::shared_ptr<json_value>> get_members()
		{
			return object_impl->members;
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

	public:

		class parse_message
		{
		public:
			int line;
			std::string topic;
			std::string error;
		};

		std::vector<parse_message> parse_errors;

		json_navigator parse_object(std::string _object)
		{
			line_number = 1;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			const char* start = _object.c_str();
			parse_object(jo, start, &start);
			json_navigator jn(jo);
			return jn;
		}

		json_navigator create_object()
		{
			line_number = 1;
			parse_errors.clear();
			auto jo = std::make_shared<json_object>();
			json_navigator jn(jo);
			return jn;
		}

		json_navigator parse_array(std::string _object)
		{
			line_number = 1;
			parse_errors.clear();
			auto jo = std::make_shared<json_array>();
			const char* start = _object.c_str();
			if (parse_array(jo, start, &start)) {
				json_navigator jn(jo);
				return jn;
			}
		}

		json_navigator create_array()
		{
			line_number = 1;
			parse_errors.clear();
			auto jo = std::make_shared<json_array>();
			json_navigator jn(jo);
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
			if (isdigit(*_src) || *_src == '.')
			{
				std::string temp = "";
				result = true;
				while (isdigit(*_src) || *_src == '.' || *_src == '_')
				{
					check_line(_src);
					temp += *_src;
					_src++;
				}
				_result = std::strtod(temp.c_str(), nullptr);
				result = true;
			}
			*_modified = _src;
			return result;
		}

		bool parse_value(std::shared_ptr<json_value>& _value, const char* _src, const char** _modified)
		{
			std::shared_ptr<json_array> new_array_value;
			std::shared_ptr<json_object> new_object_value;
			std::string new_string_value;
			double new_number_value;
			bool result = true;

			if (parse_string(new_string_value, _src, &_src))
			{
				auto js = std::make_shared<json_string>();
				js->value = new_string_value;
				_value = js;
			}
			else if (parse_number(new_number_value, _src, &_src))
			{
				auto js = std::make_shared<json_double>();
				js->value = new_number_value;
				_value = js;
			}
			else if (parse_array(new_array_value, _src, &_src))
			{
				_value = new_array_value;
			}
			else if (parse_object(new_object_value, _src, &_src))
			{
				_value = new_object_value;
			}
			else
			{
				result = false;
				error("parse_value", "Invalid value.");
			}
			*_modified = _src;
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
						}
					}
					else if (parse_object_state == parse_object_states::parsing_comma)
					{
						_src = eat_white(_src);
						if (*_src == ',') {
							_src++;
							parse_object_state = parse_object_states::parsing_name;
						}
						else
						{
							error("parse_value", std::format("Expected a comma\".", member_name));
						}
					}
				}
				_src++;
			}
			*_modified = _src;
			return result;
		}
	};


}

corona::json_navigator operator ""_json_array(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json_navigator operator ""_json_object(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

corona::json_navigator operator ""_json_array(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json_navigator operator ""_json_object(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

#endif
