module;

#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <stdexcept>

export module corona.database_json;

export class json_value
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

	T& as<T>()
	{
		auto pas = dynamic_cast<T*>(this);
		if (!*pas)
			throw std::logic_error();
		T& temp = *pas;
		return temp;
	}

	const char* eat_white(const char* _src)
	{
		while (std::isspace(*_src))
			_src++;
		return _src;
	}
};

export class json_double : public json_value
{
public:
	double value;

};

export class json_string : public json_value
{
public:
	std::string value;

};

export class json_array : public json_value
{
public:
	std::vector<shared_ptr<json_value>> elements;

};

export class json_object : public json_value
{
public:
	std::map<std::string, std::shared_ptr<json_value>> members;

};

export class json_parser
{
public:

	int line_number = 1;

	class parse_message
	{
	public:
		int line;
		std::string topic;
		std::string error;
	};

	std::vector<parse_message> parse_error;

private:

	void check_line(const char* nl)
	{
		if (*nl == '\r')
			line_number++;
	}

	void error(std::string _topic, std::string _message)
	{
		parse_error_message new_message = { line_number, _topic, _message };
	}

	bool parse_string(std::string& _result, const char* _src, const char** _modified)
	{
		bool result = false;
		std::string temp = "";
		if (*src == '"')
		{
			result = true;
			bool parsing = false;
			while (*_src != '"')
			{
				check_line(_src);
				if (std::iscntrl(*_src))
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
			}
			if (*_src) 
			{
				_src++;
			}
		}
		if (_modified)
		{
			*_modified = _src;
			result = true;
		}
		return result;
	}

	bool parse_number(double& _result, const char* _src, const char** _modified)
	{
		bool result = false;
		_src = eat_white(_src);
		if (isdigit(*_src) || *src == '.')
		{
			std::string temp = "";
			result = true;
			while (isdigit(*_src) || *_src == '.' || *_src == '_')
			{
				check_line(_src);
				temp += *_src;
				_src++;
			}
			if (_modified)
			{
				*_modified = _src;
			}
			result = std::strtod(temp.c_str());
		}
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
			_value = std::make_shared<json_string>();
			js->value = new_string_value;
		}
		else if (parse_number(new_number_value, _src, &_src))
		{
			_value = std::make_shared<json_double>();
			js->value = new_number_value;
		}
		else if (parse_array(new_array_value, _src, &_src))
		{
			_value = _new_array_value;
		}
		else if (parse_object(new_object_value, _src, &_src))
		{
			_value = _new_object_value;
		}
		else
		{
			result = false;
			error("parse_value", "Invalid value.");
		}
		*_modified = _src;
		return result;
	}

	bool parse_array(std::shared_ptr<json_array>& _object, const char* _src, const char** _modified)
	{
		bool result = false;
		_src = eat_white(_src);
		std::string temp = "";
		if (*src == '[')
		{
			result = true;
			enum parse_object_states = {
				parse_name,
				parse_colon,
				parse_value
			};

			while (*_src && *_src != ']') {
				check_line(_src);
				std::shared_ptr<json_value> value;
				if (parse_value(value, _src, &_src)) {
					_object->elements.push_back(value);
				}
				_src = eat_white(_src);
				if (*_src != ',' && *_src != ']') {
					error("parse_array", "Comma or end of array expected.");
				}
				_src++;
			}
			_src++;
			if (_modified)
			{
				*_modified = _src;
			}
		}
		return result;
	}

	bool parse_object(std::shared_ptr<json_object>& _object, const char* _src, const char** _modified)
	{

		bool result = false;
		std::string temp = "";
		_src = eat_white(_src);
		if (*src == '{')
		{
			result = true;
			_object = std::make_shared<json_object>();
			std::string member_name;
			enum parse_object_states = {
				parse_name,
				parse_colon,
				parse_value
			};
			parse_object_state = parse_name;
			while (*_src && *_src != '}')
			{
				check_line(_src);
				if (parse_object_state == parse_name) {
					if (parse_string(member_name, _src, &_src)) {
						parse_object_state = parse_colon;
					}
					else
					{
						error("parse_member_name", "Invalid member name \"name\" : value pair.");
					}
					_src++;
				}
				else if (parse_object_state == parse_colon)
				{
					_src = eat_white(_src);
					if (*_src == ':') {
						parse_object_state = parse_value;
					}
					else 
					{
						error("parse_colon", "Invalid \":\" in \"name\" : value pair.");
					}
					_src++;
				}
				else if (parse_object_state == parse_value)
				{
					std::shared_ptr<json_value> member_value;
					if (parse_value(member_value, _src, &_src)) {
						object.members[member_name] = member_value;
					}
					else 
					{
						_src++;
					}
				}
			}
			_src++;
			if (_modified)
			{
				*_modified = _src;
			}
		}
		return result;
	}
};
