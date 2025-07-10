/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This is a json parser and json object representation

Notes

For Future Consideration
*/


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
		virtual std::string format(std::string _format)
		{
			return "";
		}
		virtual std::string to_string()
		{
			return "";
		}
		virtual bool is_empty()
		{
			return true;
		}
		virtual void from_string(const std::string_view& _src)
		{
			;
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

		virtual field_types get_field_type()
		{
			return field_types::ft_none;
		}

		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			return _src;
		}

		virtual std::shared_ptr<json_value> clone()
		{
			auto jv = std::make_shared<json_value>(*this);
			jv->comparison_index = comparison_index;
			return jv;
		}

		virtual int64_t to_int64()
		{
			return 0;
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			return dt;
		}
		virtual bool to_bool()
		{
			return false;
		}
		virtual double to_double()
		{
			return 0.0;
		}

	};

	enum base_scales {
		plain = 0,
		us_length = 1,
		us_mass = 2,
		metric_length = 3,
		metric_mass = 4,
		quantity = 5
	} base_scale;

	struct base_scale_item 
	{
		double threshold;
		std::string name;
	};

	std::vector<std::vector<base_scale_item>> scales = {
		{ { 1, "" } },
		{ { 1, "16ths" }, { 4, "quarters" }, { 2, "half" }, { 2, "inches" }, { 12, "feet" }, { 3, "yards" }, { 5.5, "rods" }, { 4, "chains" }, { 80, "miles" } },
		{ { 1, "ounces" }, { 16, "pounds" }, { 2000, "tons" } },
		{ { 1, "millimeter" }, { 10, "centimeter" }, { 100, "meter" }, { 1000, "kilometer" } },
		{ { 1, "grams" }, { 1000, "kilograms" } },
		{ { 1, "" }, { 1000, "thousand" }, { 1000, "million" }, { 1000, "billion" }, { 1000, "trillion" } }
	};

	std::string get_scale(double _amount, base_scales _scales)
	{
		auto& scale_v = scales[_scales];
		std::vector<std::string> temp_items;

		double scale_boy = 1;
		auto iscale = scale_v.begin();
		auto llast = iscale;

		while (iscale != std::end(scale_v)) 
		{
			double next_scale = scale_boy * iscale->threshold;
			if (next_scale > _amount) {
				while (llast != std::begin(scale_v)) {
					double step_amount = std::floor(_amount / scale_boy);
					_amount = std::fmod(_amount, scale_boy);
					if (llast->name.size()) {
						std::string temp = std::format("{0} {1}", step_amount, llast->name);
						temp_items.push_back(temp);
					}
					else
					{
						std::string temp = std::format("{0}", step_amount);
						temp_items.push_back(temp);
					}
					llast--;
				}
				std::string result = join(temp_items, ", ");
				return result;
			}
		}
		return std::to_string(_amount);
	}

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
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;

		}
		virtual bool is_empty()
		{
			return false;
		}
		virtual void from_string(const std::string_view& _src)
		{
			std::string temp(_src);
			value = std::stod(temp);
		}
		virtual std::string format(std::string _format)
		{
			std::vector<std::string> options = corona::split(_format, ',');

			enum base_formats {
				currency,
				number,
				scales,
				plain
			} base_format;

			base_format = base_formats::plain;

			char decimal_sep[3] = ".";
			char thousands_sep[3] = ",";
			char currency_symbol[3] = "$";

			bool insurance = false;
			int decimals = 0;

			for (auto option : options) 
			{
				if (option.starts_with("currency"))
				{
					base_format = base_formats::currency;					
				}
				else if (option == "number") 
				{
					base_format = base_formats::number;
				}
				else if (option.starts_with("scale"))
				{
					std::vector<std::string> scales = corona::split(_format, '-');
					if (scales.size() > 1) {
						std::string dscale = scales[1];
						if (dscale == "us_length") {
							base_scale = base_scales::us_length;
						}
						else if (dscale == "us_mass") {
							base_scale = base_scales::us_mass;
						}
						else if (dscale == "metric_length") {
							base_scale = base_scales::metric_length;
						}
						else if (dscale == "metric_mass") {
							base_scale = base_scales::metric_mass;
						}
						else
							base_scale = base_scales::plain;
					}
				}
				else if (option.starts_with("decimals"))
				{
					std::vector<std::string> decimalssize = corona::split(_format, '-');
					if (decimalssize.size() > 1) {
						std::string dsize = decimalssize[1];
						decimals = std::strtol(dsize.c_str(), nullptr, 10);
					}
				}
				else if (option == "plain")
				{
					base_format = base_formats::plain;
				}
			}

			std::string base_result_number = std::format("{0}", value);
			std::string formatted_number;

			switch (base_format) {
			case base_formats::number:
				{
					char buffer[256];
					NUMBERFMTA numformat = {};
					numformat.Grouping = 3;
					numformat.LeadingZero = 0;
					numformat.lpDecimalSep = decimal_sep;
					numformat.lpThousandSep = thousands_sep;
					numformat.NegativeOrder = 0;
					numformat.NumDigits = decimals;

					bool success = ::GetNumberFormatA(LOCALE_USER_DEFAULT, 0, base_result_number.c_str(), &numformat, buffer, std::size(buffer));
					if (success) {
						formatted_number = buffer;
					}
				}
				break;
			case base_formats::scales:
				{
					formatted_number = get_scale(value, base_scale);
				}
				break;
			case base_formats::plain:
				{
					formatted_number = base_result_number;
				}
				break;
			case base_formats::currency:
				{
					char buffer[256];
					CURRENCYFMTA currencyformat = {};
					currencyformat.Grouping = 3;
					currencyformat.LeadingZero = 0;
					currencyformat.lpDecimalSep = decimal_sep;
					currencyformat.lpThousandSep = thousands_sep;
					currencyformat.NegativeOrder = 0;
					currencyformat.NumDigits = decimals;
					currencyformat.lpCurrencySymbol = currency_symbol;

					bool success = ::GetCurrencyFormatA(LOCALE_USER_DEFAULT, 0, base_result_number.c_str(), &currencyformat, buffer, std::size(buffer));
					if (success) {
						formatted_number = buffer;
					}
				}
				break;
			}

			return formatted_number;
		}
		virtual field_types get_field_type()
		{
			return field_types::ft_double;
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

		virtual int64_t to_int64()
		{
			return value;
		}
		virtual date_time to_datetime()
		{
			date_time dt((time_t)value);
			return dt;
		}
		virtual bool to_bool()
		{
			return value != 0.0;
		}
		virtual double to_double()
		{
			return value;
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
		virtual bool is_empty()
		{
			return value.is_empty();
		}
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;

		}
		virtual void from_string(const std::string_view& _src)
		{
			value = std::string(_src);
		}
		virtual std::string format(std::string _format)
		{
			return value.format(_format);
		}
		virtual field_types get_field_type()
		{
			return field_types::ft_datetime;
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

		virtual int64_t to_int64()
		{
			return value.get_time_t();
		}
		virtual date_time to_datetime()
		{
			return value;
		}
		virtual bool to_bool()
		{
			return true;
		}
		virtual double to_double()
		{
			return value.get_time_t();
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

		virtual bool is_empty()
		{
			return value.size() == 0;
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
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;
		}

		virtual void from_string(const std::string_view& st)
		{
			value.clear();
			for (int i = 0; i < st.size(); i += 2)
			{
				int item = toInt2(st, i);
				value.push_back(item);
			}
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_blob;
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


		virtual int64_t to_int64()
		{
			return 0;
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			return dt;
		}
		virtual bool to_bool()
		{
			return true;
		}
		virtual double to_double()
		{
			return 0.0;
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
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;

		}

		virtual bool is_empty()
		{
			return false;
		}

		virtual std::string format(std::string _format)
		{
			return std::to_string(value);
		}

		virtual std::string to_string()
		{
			return std::format("{}", value);
		}

		virtual void from_string(const std::string_view& _src)
		{
			std::string temp(_src);
			value = std::stoll(temp);
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_int64;
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

		virtual int64_t to_int64()
		{
			return value;
		}
		virtual date_time to_datetime()
		{
			date_time dt(value);
			return dt;
		}
		virtual bool to_bool()
		{
			return value != 0;
		}
		virtual double to_double()
		{
			return value;
		}

	};

	class json_reference : public json_value
	{
	public:
		object_reference_type value;

		virtual std::string to_key()
		{
			return std::format("{0}:{1}", value.class_name, value.object_id);
		}

		virtual std::string to_json()
		{
			return std::format("{0}:{1}", value.class_name, value.object_id);
		}

		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;

		}

		virtual bool is_empty()
		{
			return false;
		}

		virtual std::string format(std::string _format)
		{
			return std::format("{0}", to_key());
		}

		virtual std::string to_string()
		{
			return std::format("{}", to_key());
		}

		virtual void from_string(const std::string_view& _src)
		{
			std::string temp(_src);
			auto arr = split(temp, ':');
			if (arr.size() > 0)
				value.class_name = arr[0];
			if (arr.size() > 1)
				value.object_id = strtol(arr[1].c_str(), nullptr, 10);
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_reference;
		}

		virtual std::string get_type_prefix()
		{
			return "$reference";
		}

		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_reference>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}

		virtual int64_t to_int64()
		{
			return value.object_id;
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			return dt;
		}
		virtual bool to_bool()
		{
			return value.object_id != 0 and not value.class_name.empty();
		}
		virtual double to_double()
		{
			return value.object_id;
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
			char oldChar = '"';
			// TODO: recognize the escape character when parsing!!!
			std::string temp;
			temp += '"';
			for (auto c : value) {
				if (c == oldChar) {
					temp.push_back('\\');
				}
				temp.push_back(c);
			}
			temp += '"';
			return temp;
		}

		virtual std::string to_json_typed()
		{
			return get_type_prefix() + " " + to_json();
		}
		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << to_json_typed();
			return _src;

		}

		virtual std::string to_string()
		{
			return value;
		}
		virtual void from_string(const std::string_view& _src)
		{
			value = _src;
		}

		virtual bool is_empty()
		{
			return value.empty();
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_string;
		}

		virtual std::string get_type_prefix()
		{
			return "$string";
		}
		virtual std::string format(std::string _format)
		{
			return value;
		}
		virtual std::shared_ptr<json_value> clone()
		{
			auto t = std::make_shared<json_string>();
			t->value = value;
			t->comparison_index = comparison_index;
			return t;
		}

		virtual int64_t to_int64()
		{
			return std::strtoll(value.c_str(), nullptr, 10);
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			dt.parse(value);
			return dt;
		}
		virtual bool to_bool()
		{
			return not value.empty();
		}
		virtual double to_double()
		{
			return std::strtod(value.c_str(), nullptr);
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

		virtual bool is_empty()
		{
			return elements.empty();
		}

		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << "[ ";
			std::string comma = "";
			for (auto el : elements) {
				_src << comma;
				comma = ", ";
				el->serialize(_src);
			}
			_src <<  " ]";
			return _src;
		}

		virtual std::string to_json_typed()
		{
			return to_json();
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_array;
		}


		virtual std::string format(std::string _format)
		{
			return to_json();
		}

		virtual std::string to_string()
		{
			return to_json();
		}
		virtual void from_string(const std::string_view& _src)
		{
			elements.clear();
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

		virtual int64_t to_int64()
		{
			return 0;
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			return dt;
		}
		virtual bool to_bool()
		{
			return true;
		}
		virtual double to_double()
		{
			return 0.0;
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
			std::string ret = "{ ";
			std::string comma = "";
			for (auto el : members) {
				ret += comma;
				comma = ", ";
				ret += "\"" + el.first + "\"";
				ret += ":";
				if (el.second) {
					ret += el.second->to_json_typed();
				}
			}
			ret += " }";
			return ret;
		}

		virtual std::stringstream& serialize(std::stringstream& _src)
		{
			_src << "{ ";
			std::string comma = "";
			for (auto el : members) {
				_src << comma;
				comma = ", ";
				_src << "\"" << el.first << "\"" << ":";
				if (el.second) {
					el.second->serialize(_src);
				}
			}
			_src << " }";
			return _src;
		}

		virtual bool is_empty()
		{
			return members.empty();
		}

		virtual field_types get_field_type()
		{
			return field_types::ft_object;
		}

		virtual std::string to_string()
		{
			return to_json();
		}

		virtual void from_string(const std::string_view& _src)
		{
			members.clear();
		}

		virtual std::string format(std::string _format)
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

		virtual int64_t to_int64()
		{
			return 0;
		}
		virtual date_time to_datetime()
		{
			date_time dt;
			return dt;
		}
		virtual bool to_bool()
		{
			return true;
		}
		virtual double to_double()
		{
			return 0.0;
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
		virtual std::string format(std::string _format)
		{
			return to_json();
		}


	};

	class json
	{
	public:
		using compared_item = std::tuple<int, std::string>;

	private:
		std::vector<compared_item> comparison_fields;

		std::shared_ptr<json_value> value_base;

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
		}

		std::shared_ptr<json_double> double_impl() const {
			return std::dynamic_pointer_cast<json_double>(value_base);
		}

		std::shared_ptr<json_string> string_impl() const {
			return std::dynamic_pointer_cast<json_string>(value_base);
		}

		std::shared_ptr<json_array> array_impl() const {
			return std::dynamic_pointer_cast<json_array>(value_base);
		}

		std::shared_ptr<json_object> object_impl()   const {
			return std::dynamic_pointer_cast<json_object>(value_base);
		}

		std::shared_ptr<json_int64> int64_impl() const {
			return std::dynamic_pointer_cast<json_int64>(value_base);
		}

		std::shared_ptr<json_reference> reference_impl() const {
			return std::dynamic_pointer_cast<json_reference>(value_base);
		}

		std::shared_ptr<json_datetime> datetime_impl()  const {
			return std::dynamic_pointer_cast<json_datetime>(value_base);
		}

		std::shared_ptr<json_blob> blob_impl() const {
			return std::dynamic_pointer_cast<json_blob>(value_base);
		}

		std::shared_ptr<json_function> function_impl() const {
			return std::dynamic_pointer_cast<json_function>(value_base);
		}

		virtual field_types get_field_type()
		{
			if (value_base) {
				return value_base->get_field_type();
			}
			return field_types::ft_none;
		}

		json query(std::string_view _path);

		json clone()
		{
			json result;
			if (value_base) {
				result = json(value_base->clone());
			}
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

		std::stringstream& serialize(std::stringstream& _dest)
		{
			if (value_base) {
				value_base->serialize(_dest);
			}
			return _dest;
		}

		bool has_members(const std::vector<std::string>& _src)
		{
			if (not object())
				return false;

			for (auto s : _src) {
				if (not has_member(s))
					return false;
				auto member = object_impl()->members[s];
				if (member->is_empty()) {
					return false;
				}
			}
			return true;
		}

		bool has_members(std::vector<std::string>& _missing, const std::vector<std::string>& _src)
		{
			if (not object())
				return false;

			bool good = true;

			for (auto s : _src) {
				if (not has_member(s)) {
					good = false;
					_missing.push_back(s);
				}
				else
				{
					auto member = object_impl()->members[s];
					if (member->is_empty()) {
						good = false;
						_missing.push_back(s);
					}
				}
			}
			return good;
		}

		std::string to_json_typed_escaped()
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

		std::string to_json_escaped()
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

		int getMaxBitIndex(unsigned long long value) {
			unsigned long index_lists;
			if (_BitScanReverse64(&index_lists, value)) {
				return index_lists;
			}
			return -1; // No bits set
		}

		uint64_t get_weak_ordered_hash1(std::vector<std::string> _keys)
		{
			// single key case

			uint64_t hash = 0;

			if (not object())
				return 0;

			for (int i = 0; i < 1; i++)
			{
				auto ikey = _keys[i];
				if (has_member(ikey)) {
					auto m = get_member(ikey);
					if (m.is_datetime())
					{
						date_time dt = (date_time)(m);
						hash = dt.get_time_t();
					}
					else if (m.is_double())
					{
						double v = (double)m;
						if (v < 0) v = 0;
						hash = static_cast<short>(std::log10(v) * 1000);
					}
					else if (m.is_int64())
					{
						hash = (int64_t)m;
					}
					else
					{
						std::string temp = (std::string)m;
						int sz = temp.size();
						for (int i = 0; i < 8; i++) {
							if (i < sz) {
								hash += temp[i];
							}
							hash *= 256;
						}
					}
				}
			}

			return hash;
		}

		uint64_t get_weak_ordered_hash2(std::vector<std::string> _keys)
		{
			// single key case

			uint64_t hash = 0;

			if (not object())
				return 0;

			bool flippo;

			for (int i = 0; i < 2; i++)
			{
				auto ikey = _keys[i];
				hash <<= 32;
				uint32_t temp = 0;

				if (has_member(ikey)) {
					auto m = get_member(ikey);
					if (m.is_datetime())
					{
						date_time dt = (date_time)(m);
						LARGE_INTEGER li;
						li.QuadPart = dt.get_time_t();
						temp = li.HighPart;
					}
					else if (m.is_double())
					{
						double v = (double)m;
						if (v < 0) v = 0;
						double x = static_cast<short>(std::log10(v) * 1000);

						LARGE_INTEGER li;
						li.QuadPart = x;
						temp = li.HighPart;
					}
					else if (m.is_int64())
					{
						LARGE_INTEGER li;
						li.QuadPart = m;
						temp = li.HighPart;
					}
					else
					{
						std::string temps = (std::string)m;
						int sz = temps.size();
						for (int i = 0; i < 4; i++) {
							if (i < sz) {
								temp += temps[i];
							}
							temp *= 256;
						}
					}
				}
				hash |= temp;
			}

			return hash;
		}

		uint64_t get_weak_ordered_hash3(std::vector<std::string> _keys)
		{
			// single key case

			uint64_t hash = 0;

			if (not object())
				return 0;

			bool flippo;

			for (int i = 0; i < 3; i++)
			{
				auto ikey = _keys[i];
				hash <<= 16;
				uint16_t temp = 0;

				if (has_member(ikey)) {
					auto m = get_member(ikey);
					if (m.is_datetime())
					{
						date_time dt = (date_time)(m);
						LARGE_INTEGER li;
						li.QuadPart = dt.get_time_t();
						temp = HIWORD(li.HighPart);
					}
					else if (m.is_double())
					{
						double v = (double)m;
						if (v < 0) v = 0;
						double x = static_cast<short>(std::log10(v) * 1000);

						LARGE_INTEGER li;
						li.QuadPart = x;
						temp = HIWORD(li.HighPart);
					}
					else if (m.is_int64())
					{
						LARGE_INTEGER li;
						li.QuadPart = m;
						temp = HIWORD(li.HighPart);
					}
					else
					{
						std::string temps = (std::string)m;
						int sz = temps.size();
						for (int i = 0; i < 2; i++) {
							if (i < sz) {
								temp += temps[i];
							}
							temp *= 256;
						}
					}
				}
				hash |= temp;
			}

			return hash;
		}

		uint64_t get_weak_ordered_hash(std::vector<std::string> _keys)
		{
			switch (_keys.size()) {
			case 0:
				return 0;
			case 1:
				return get_weak_ordered_hash1(_keys);
			case 2:
				return get_weak_ordered_hash2(_keys);
			case 3:
				return get_weak_ordered_hash2(_keys);
			}
			return 0;
		}

		json make_path(std::string_view _path, std::string& _name, json _value);


		bool is_int64() const
		{
			return int64_impl() != nullptr;
		}

		bool is_datetime()  const
		{
			return datetime_impl() != nullptr;
		}

		bool is_blob()  const
		{
			return blob_impl() != nullptr;
		}

		bool is_double()  const
		{
			return double_impl() != nullptr;
		}

		bool is_string()  const
		{
			return string_impl() != nullptr;
		}

		bool reference()  const
		{
			return reference_impl() != nullptr;
		}

		bool array()  const
		{
			return array_impl() != nullptr;
		}

		bool object()  const
		{
			return object_impl() != nullptr;
		}

		bool function()  const
		{
			return function_impl() != nullptr;
		}

		bool scalar()  const
		{
			return not array() and not object() and not empty();
		}

		bool empty() const
		{
			return value_base == nullptr;
		}

		bool error()
		{
			return has_member(class_name_field) and (std::string)get_member(class_name_field) == parse_error_class;
		}

		int64_t& get_int64()   const
		{
			return int64_impl()->value;
		}

		date_time& get_datetime()   const
		{
			return datetime_impl()->value;
		}

		double& get_double() const
		{
			return double_impl()->value;
		}

		std::string& get_string()  const
		{
			return string_impl()->value;
		}

		operator int() const
		{
			if (value_base)
				return value_base->to_int64();
			else
				return 0;
		}

		operator double() const
		{
			if (value_base)
				return value_base->to_double();
			else
				return 0.0;
		}

		operator int64_t() const
		{
			if (value_base)
				return value_base->to_int64();
			else
				return 0;
		}

		operator date_time() const
		{
			if (value_base)
				return value_base->to_datetime();
			else
				return 0;
		}

		operator std::string() const
		{
			if (value_base)
				return value_base->to_string();
			else
				return "";
		}

		operator object_reference_type() const
		{
			auto value_ref = reference_impl();
			if (value_ref)
				return value_ref->value;
			else
				return object_reference_type();
		}

		explicit operator bool() const
		{
			if (value_base)
				return value_base->to_bool();
			else
				return false;
		}

		std::string format_string(std::string _format)
		{
			return value_base->format(_format);
		}

		std::string format_member(std::string _member, std::string _format)
		{
			if (not object())
			{
				return value_base->format(_format);
			}
		}

		bool import_member(std::string _member, const std::string_view& _value_to_import)
		{
			bool stuffed = false;
			if (object_impl() and object_impl()->members.contains(_member)) {
				object_impl()->members[_member]->from_string(_value_to_import);
				stuffed = true;
			}
			return stuffed;
		}


		bool contains_text(std::string _text);


		buffer to_buffer()
		{
			if (not empty())
			{
				std::string temp_s = to_json(); // this is probably wrong for some applications, but right for others
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

		std::shared_ptr<json_value> get_member_value(const std::string& _key)
		{
			if (object_impl()) {
				auto vlist = object_impl()->members.find(_key);
				if (vlist != std::end(object_impl()->members)) {
					return vlist->second;
				}
			}
			return nullptr;
		}

		std::shared_ptr<json_array> get_member_array(const std::string& _key)
		{
			if (object_impl()) {
				auto vlist = object_impl()->members.find(_key);
				if (vlist != std::end(object_impl()->members)) {
					return std::dynamic_pointer_cast<json_array>(vlist->second);
				}
			}
			return nullptr;
		}

		json operator[](const std::string_view& _key) const
		{
			std::string temp(_key);
			if (object_impl() and object_impl()->members.contains(temp)) {
				json jn(object_impl()->members[temp]);
				return jn;
			}
			json empty;
			return empty;
		}

		json operator[](const std::string& _key) const
		{
			if (object_impl() and object_impl()->members.contains(_key)) {
				json jn(object_impl()->members[_key]);
				return jn;
			}
			json empty;
			return empty;
		}

		json operator[](const char* _key)  const
		{
			if (object_impl() and object_impl()->members.contains(_key)) {
				json jn(object_impl()->members[_key]);
				return jn;
			}
			json empty;
			return empty;
		}

		bool gt(json _value);
		bool lt(json _value);
		bool eq(json _value);

		void assign_update(json _member)
		{

			if (_member.empty())
			{
				return;
			}

			if (empty())
			{
				set(_member.value_base);
				return;
			}

			if (_member.object() and object())
			{
				auto members = _member.get_members_raw();

				for (auto src : members)
				{
					put_member_value(src.first, src.second);
				}
			}
		}

		void append_array(json _src)
		{
			if (array_impl()) {
				if (_src.array()) {
					for (json item : _src) {
						array_impl()->elements.push_back(item.value_base->clone());
					}
				}
				else
				{
					array_impl()->elements.push_back(_src.value_base->clone());
				}
			}
		}

		void assign_replace(json _member)
		{
			set(_member.value_base);
		}

		bool has_member(std::string _key)
		{
			bool has_value = !_key.empty() and object_impl() and object_impl()->members.contains(_key);
			return has_value;
		}

		bool is_member(std::string _key, const char* _value)
		{
			bool has_value = object_impl() and object_impl()->members.contains(_key);
			if (has_value) {
				std::string svalue = object_impl()->members[_key]->to_string();
				std::string xvalue = _value;
				has_value = svalue == xvalue;
			}
			return has_value;
		}

		bool is_member(std::string _key, std::string _value)
		{
			bool has_value = object_impl() and object_impl()->members.contains(_key);
			if (has_value) {
				std::string svalue = object_impl()->members[_key]->to_string();
				has_value = svalue == _value;
			}
			return has_value;
		}

		bool is_member(std::string _key, int64_t _value)
		{
			bool has_value = object_impl() and object_impl()->members.contains(_key);
			if (has_value) {
				int64_t svalue = get_member(_key);
				has_value = svalue == _value;
			}
			return has_value;
		}

		bool is_member(std::string _key, bool _value)
		{
			bool has_value = object_impl() and object_impl()->members.contains(_key);
			if (has_value) {
				bool svalue = (bool)get_member(_key);
				has_value = svalue == _value;
			}
			return has_value;
		}

		json get_first_element()
		{
			json result;
			if (array_impl() and array_impl()->elements.size() > 0) {
				result = get_element(0);
			}
			return result;
		}

		json get_last_element()
		{
			return get_element(array_impl()->elements.size() - 1);
		}

		json get_element(int _index)
		{
			json jn(array_impl()->elements[_index]);
			return jn;
		}

		json get_member(std::string _key)
		{
			json jn, jx;

			if (not object_impl()) {
				return jn;
			}

			jn = json(object_impl()->members[_key]);
			return jn;
		}

		int get_member_int(std::string _key)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			json jn(object_impl()->members[_key]);
			double jnd = (double)jn;
			return jnd;
		}

		json copy_member(std::string _key, json& _source)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			std::string search_key = _key;
			if (_source.object() and _source.has_member(search_key)) {
				json member = _source.get_member(search_key);
				put_member(_key, member);
			}
			return *this;
		}

		void put_members(json& _member)
		{
			if (not object_impl()) {
				throw std::logic_error("Target is not an object");
			}

			if (not _member.object_impl()) {
				throw std::logic_error("Source is not an object");
			}

			auto members = _member.get_members_raw();

			for (auto src : members)
			{
				put_member_value(src.first, src.second);
			}
		}

		void change_member_type(std::string _key, field_types _new_type)
		{
			if (not object_impl())
			{
				throw std::logic_error("Not an object");
			}

			if (not has_member(_key))
			{
				throw std::logic_error("Changing type on a non-existent member");
			}

			if (_new_type == field_types::ft_object) {
				put_member_object(_key);
			}
			else if (_new_type == field_types::ft_array) {
				put_member_array(_key);
			}
			else if (_new_type == field_types::ft_string) {
				std::string s = (std::string)get_member(_key);
				put_member(_key, s);
			}
			else if (_new_type == field_types::ft_int64) {
				int64_t i64 = (int64_t)get_member(_key);
				put_member_i64(_key, i64);
			}
			else if (_new_type == field_types::ft_double) {
				double d = (double)get_member(_key);
				put_member(_key, d);
			}
			else if (_new_type == field_types::ft_datetime) {
				date_time dt = get_member(_key);
				put_member(_key, dt);
			}
			else if (_new_type == field_types::ft_bool) {
				bool b = (bool)get_member(_key);
				put_member(_key, b);
			}
		}

		json build_member(std::string _key);

		json build_member(std::string _key, json& _member)
		{
			std::string tail;
			json obj = make_path(_key, tail, _member);
			return obj;
		}

		json build_member(std::string _key, std::string _value)
		{
			std::string tail;
			json jv = json(std::make_shared<json_string>());
			jv.string_impl()->value = _value;
			json obj = make_path(_key, tail, jv);
			return obj;
		}

		json build_member(std::string _key, double _value)
		{
			std::string tail;
			json jv = json(std::make_shared<json_double>());
			jv.double_impl()->value = _value;
			json obj = make_path(_key, tail, jv);
			return obj;
		}

		json build_member_int64(std::string _key, int64_t _value)
		{
			std::string tail;
			json jv = json(std::make_shared<json_int64>());
			jv.int64_impl()->value = _value;
			json obj = make_path(_key, tail, jv);
			return obj;
		}

		json put_member(std::string _key, json& _member)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			if (_member.array()) {
				put_member_array(_key, _member);
			}
			else if (_member.empty())
			{
				put_member_object(_key, _member);
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
			else if (_member.object()) {
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
			else if (_member.function()) {
				put_member_function(_key, _member);
			}
			return *this;
		}

		json put_member(std::string _key, std::shared_ptr<json_array> _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			object_impl()->members[_key] = _value;
			return *this;
		}

		json put_member(std::string _key, std::shared_ptr<json_object> _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			object_impl()->members[_key] = _value;
			return *this;
		}

		json put_member(std::string _key, object_reference_type _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_reference>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member(std::string _key, std::string_view _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}


		json put_member_string(std::string _key, std::string _value)
		{
			return put_member(_key, _value);
		}

		json put_member(std::string _key, std::string _value)
		{

			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_i64(std::string _key,  int64_t _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_int64>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_reference(std::string _key, object_reference_type _ref)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_reference>();
			new_member->value = _ref;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_double(std::string _key, double _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_double>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member(std::string _key, double _value)
		{
			return put_member_double(_key, _value);
		}

		json put_member(std::string _key, date_time _value)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_datetime>();
			new_member->value = _value;
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_array(std::string _key)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_array>();
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_object(std::string _key, json& _object)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_object> existing_object = _object.object_impl();
			if (existing_object) {
				auto new_object = existing_object->clone();
				object_impl()->members[_key] = new_object;
			}
			else 
			{
				auto new_object = std::make_shared<json_object>();
				object_impl()->members[_key] = new_object;
			}
			return *this;
		}

		json put_member_object(std::string _key)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_object>();
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_blob(std::string _key)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_member = std::make_shared<json_object>();
			object_impl()->members[_key] = new_member;
			return *this;
		}

		json put_member_array(std::string _key, json& _array)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_array> existing_array = _array.array_impl();

			if (existing_array) {
				auto new_array = existing_array->clone();
				object_impl()->members[_key] = new_array;
			}
			else 
			{
				auto new_array = std::make_shared<json_array>();
				object_impl()->members[_key] = new_array;
			}
			return *this;
		}

		json put_member_value(std::string _key, std::shared_ptr<json_value> _obj)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			auto new_object = _obj->clone();
			object_impl()->members[_key] = new_object;
			return *this;
		}

		json merge(json& _object);

		json put_member_function(std::string _key, json& _object)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_function> existing_object = _object.function_impl();
			auto new_object = existing_object->clone();
			object_impl()->members[_key] = new_object;
			return *this;
		}

		json put_member_function(std::string _key, json_function_function fn)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			std::shared_ptr<json_function> new_object = std::make_shared<json_function>();
			object_impl()->members[_key] = new_object;
			return *this;
		}

		json erase_member(std::string _key)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			object_impl()->members.erase(_key);
			return *this;
		}

		json operator()(json _params)
		{
			if (not function_impl()) {
				throw std::logic_error("Not a function");
			}
			return function_impl()->fn(*function_impl()->function_this, _params);
		}

		json set_natural_order()
		{
			comparison_fields.clear();

			int comparison_index = 1;
			for (auto m : object_impl()->members) {
				if (m.second) {
					m.second->comparison_index = comparison_index++;
					compared_item sort_tuple = std::make_tuple(m.second->comparison_index, m.first);
					comparison_fields.push_back(sort_tuple);
				}
			}

			std::sort(comparison_fields.begin(), comparison_fields.end());

			return *this;
		}

		std::vector<compared_item>& get_compare_order()
		{
			return comparison_fields;
		}

		json set_compare_order(std::vector<std::string> _fields)
		{
			comparison_fields.clear();

			int comparison_index = 1;
			for (auto m : object_impl()->members) {
				if (m.second) m.second->comparison_index = 0;
			}
			for (auto f : _fields)
			{
				if (object_impl()->members.contains(f)) {
					auto fn = object_impl()->members[f];
					fn->comparison_index = ++comparison_index;
				}
			}

			for (auto m : object_impl()->members)
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
			if (not object_impl())
			{
				throw std::logic_error("Not an object");
			}
			if (not comparison_fields.size())
			{
				return false;
			}
			for (auto k : keys) 
			{				
				bool found = false;
				for (compared_item& c : comparison_fields)
				{
					std::string cname = std::get<1>(c);
					if (cname == k) {
						found = true;
						break;
					}
				}
				if (not found) {
					return false;
				}
			}
			return true;
		}

		json extract(std::vector<std::string> _fields)
		{
			if (not object_impl())
			{
				throw std::logic_error("Not an object");
			}

			json jn(std::make_shared<json_object>());

			int comparison_index = 1;
			for (auto f : _fields)
			{
				if (object_impl()->members.contains(f)) {
					auto fn = object_impl()->members[f];
					fn->comparison_index = ++comparison_index;
					jn.put_member_value(f, fn);
				}
			}

			jn.set_compare_order(_fields);

			return jn;
		}

		std::map<std::string, std::shared_ptr<json_value>> get_members_raw()
		{
			if (not object_impl()) {
				throw std::logic_error("Not an object");
			}
			return object_impl()->members;
		}

		std::map<std::string, json> get_members();

		template <typename element_type> json append_element(element_type et)
		{
			return put_element( -1, et);
		}

		json push_back(double et)
		{
			return put_element(-1, et);
		}

		json push_back(int64_t et)
		{
			return put_element(-1, et);
		}

		json push_back(std::string et)
		{
			return put_element(-1, et);
		}

		json push_back(date_time et)
		{
			return put_element(-1, et);
		}

		json push_back(json et)
		{
			return put_element(-1, et);
		}

		json put_element(int _index, json &_element)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}

			if (_element.array()) {
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
			else if (_element.object()) {
				put_element_object(_index, _element);
			}
			else if (_element.is_string()) {
				std::string d = _element;
				put_element(_index, d);
			}
			return *this;
		}

		json put_element(int _index, object_reference_type _value)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_reference>();
			new_member->value = _value;

			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, std::string _value)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_string>();
			new_member->value = _value;

			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, int64_t _value)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_int64>();
			new_member->value = _value;
			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, date_time _value)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_datetime>();
			new_member->value = _value;
			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element(int _index, double _value)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_double>();
			new_member->value = _value;
			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_array(int _index)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_array>();
			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_object(int _index)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			auto new_member = std::make_shared<json_object>();
			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_member);
			}
			else {
				array_impl()->elements[_index] = new_member;
			}
			return *this;
		}

		json put_element_array(int _index, json& _array)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			std::shared_ptr<json_array> existing_array = _array.array_impl();
			auto new_array = existing_array->clone();

			if (_index < 0 or _index >= array_impl()->elements.size())
			{
				array_impl()->elements.push_back(new_array);
			}
			else 
			{
				array_impl()->elements[_index] = new_array;
			}
			return *this;
		}

		json put_element_object(int _index, json& _object)
		{
			if (not array_impl()) {
				throw std::logic_error("Not an array");
			}
			std::shared_ptr<json_object> existing_object = _object.object_impl();
			auto new_object = existing_object->clone();

			if (_index < 0 or _index >= array_impl()->elements.size()) {
				array_impl()->elements.push_back(new_object);
			}
			else 
			{
				array_impl()->elements[_index] = new_object;
			}
			return *this;
		}

		std::vector<std::string> to_string_array()
		{
			std::vector<std::string> temp;
			if (array_impl())
			{
				for (auto& array_item : array_impl()->elements)
				{
					std::string sitem = array_item->to_string();
					temp.push_back(sitem);
				}
			}
			return temp;
		}

		int compare_field(const std::string& key, json& _item)
		{
			int comparison_result = 0;
			auto source_value = get_member_value(key);
			auto dest_value = _item.get_member_value(key);

			if (not dest_value or dest_value->get_field_type() != source_value->get_field_type())
			{
				comparison_result = 1;
			}
			else if (not source_value)
			{
				comparison_result = -1;
			}
			else switch (source_value->get_field_type())
			{
			case field_types::ft_none:
			{
				comparison_result = 0;
			}
			break;

			case field_types::ft_object:
			{
				comparison_result = 0;
			}
			break;

			case field_types::ft_array:
			{
				comparison_result = 0;
			}
			break;

			case field_types::ft_double:
			{
				auto vsource = std::dynamic_pointer_cast<json_double>(source_value);
				auto vdest = std::dynamic_pointer_cast<json_double>(dest_value);
				if (vsource->value < vdest->value) {
					comparison_result = -1;
				}
				else if (vsource->value > vdest->value) {
					comparison_result = 1;
				}
			}
			break;

			case field_types::ft_int64:
			{
				auto vsource = std::dynamic_pointer_cast<json_int64>(source_value);
				auto vdest = std::dynamic_pointer_cast<json_int64>(dest_value);
				if (vsource->value < vdest->value) {
					comparison_result = -1;
				}
				else if (vsource->value > vdest->value) {
					comparison_result = 1;
				}

			}
			break;

			case field_types::ft_string:
			{
				auto vsource = std::dynamic_pointer_cast<json_string>(source_value);
				auto vdest = std::dynamic_pointer_cast<json_string>(dest_value);
				if (vsource->value < vdest->value) {
					comparison_result = -1;
				}
				else if (vsource->value > vdest->value) {
					comparison_result = 1;
				}
			}
			break;

			case field_types::ft_bool:
				break;

			case field_types::ft_datetime:
			{
				auto vsource = std::dynamic_pointer_cast<json_datetime>(source_value);
				auto vdest = std::dynamic_pointer_cast<json_datetime>(dest_value);
				if (vsource->value < vdest->value) {
					comparison_result = -1;
				}
				else if (vsource->value > vdest->value) {
					comparison_result = 1;
				}
			}
			break;

			case field_types::ft_query:
			{

				return 0;

			}
			break;

			case field_types::ft_blob:
			{
				return 0;

			}
			break;
			}
			return comparison_result;
		}

		int compare(json& _item)
		{
			if (object() and !_item.object())
			{
				return 1;
			}

			if (not object() and _item.object())
			{
				return -1;
			}

			if (not object() and !_item.object()) 
			{
				throw std::logic_error("At least one of the being compared must be json_object (not array or value, yet)");
			}

			int comparison_result = 0;

			if (comparison_fields.size() > 0)
			{
				for (auto m : comparison_fields)
				{
					std::string key = std::get<1>(m);

					comparison_result = compare_field(key, _item);

					if (comparison_result)
						return comparison_result;
				}
			}
			else 
			{
				auto members = get_members();
				for (auto member : members)
				{
					comparison_result = compare_field(member.first, _item);
					if (comparison_result)
						return comparison_result;
				}
			}

			return comparison_result;
		}

		json for_each_member(std::function<void(const std::string& _key_name, json _member)> _transform)
		{
			if (not object_impl()) {
				throw std::logic_error("Not an array");
			}
			for (auto m : object_impl()->members)
			{
				_transform(m.first, m.second);
			}
			return *this;
		}

		json for_each_element(std::function<void(json& _item)> _transform)
		{
			if (not array_impl()) {
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
			if (object_impl()) {
				auto members = get_members();
				for (auto m : members)
				{
					if (_where_clause(m.second)) {
						return true;
					}
				}
			}
			else if (array_impl())
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
			if (object_impl()) {
				auto members = get_members();
				for (auto m : members)
				{
					if (not _where_clause(m.second)) {
						return false;
					}
				}
			}
			else if (array_impl())
			{
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (not _where_clause(element)) {
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

			if (object_impl()) {
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
			else if (array_impl())
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

			if (array_impl())
			{
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					if (element.object()) {
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

			if (object_impl())
			{
				auto members = get_members();
				for (auto member: members)
				{
					json new_object = _get_payload(member.first, member.second);
					if (not new_object.empty()) {
						result_item.append_element(new_object);
					}
				}
			}
			else {
				throw std::exception("Item is not array");
			}
			return result_item;
		}

		json slice(int _start_index, int _length)
		{
			json result_item;
			if (array_impl()) {
				result_item = json(std::make_shared<json_array>());
				int start = _start_index;
				int stop = _start_index + _length;
				if (start >= size()) {
					return result_item;
				}
				if (stop >= size()) {
					stop = size();
				}
				for (int i = start; i < stop; i++)
				{
					json temp = get_element(i);
					result_item.push_back(temp);
				}
			}
			return result_item;
		}

		json map(std::function<json(std::string _member, int _index, json& _item)> _transform)
		{
			json result_item;

			if (array_impl())
			{
				result_item = json(std::make_shared<json_array>());
				for (int i = 0; i < size(); i++)
				{
					auto element = get_element(i);
					auto new_element = _transform("", i, element);
					if (new_element.array()) {
						for (int i = 0; i < new_element.size(); i++) {
							json j = new_element.get_element(i);
							result_item.append_element( j);
						}
					}
					else if (not new_element.empty())
					{
						result_item.append_element(new_element);
					}
				}
			}
			else if (object_impl())
			{
				result_item = json(std::make_shared<json_object>());
				auto members = get_members();
				for (auto member : members)
				{
					auto new_member = _transform(member.first, 0, member.second);
					if (new_member.object()) {
						auto members = new_member.get_members();
						for (auto member : members) {
							if (not result_item.has_member(member.first)) {
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

		std::string apply_template(std::string _src)
		{
			if (object())
			{
				for (auto member : object_impl()->members) {
					_src = replace(_src, member.first, member.second->to_string());
				}
			}
			return _src;
		}

		void apply_abbreviations(json _abbreviations)
		{
			if (not _abbreviations.object())
				return;

			auto abbrev = _abbreviations.object_impl();

			if (object())
			{
				for (auto& member : object_impl()->members)
				{
					if (member.second->get_field_type() == field_types::ft_string)
					{
						std::shared_ptr<json_string> js = std::dynamic_pointer_cast<json_string>(member.second);
						if (js)
						{
							auto ifound = abbrev->members.find(js->value);
							if (ifound != std::end(abbrev->members)) {
								member.second = ifound->second;
							}
						}
					}
					else if (member.second->get_field_type() == field_types::ft_object)
					{
						json j = member.second;
						j.apply_abbreviations(_abbreviations);
					}
					else if (member.second->get_field_type() == field_types::ft_array)
					{
						std::shared_ptr<json_array> js = std::dynamic_pointer_cast<json_array>(member.second);
						if (js)
						{
							for (auto item : js->elements)
							{
								if (item->get_field_type() == field_types::ft_object)
								{
									json obj = item;
									obj.apply_abbreviations(_abbreviations);
								}
							}
						}
					}
				}
			}
			else if (array())
			{
				for (auto element : array_impl()->elements)
				{
					json temp(element);
					temp.apply_abbreviations(_abbreviations);
				}
			}
		}

		json update(std::function<json&(json& _item)> _transform)
		{
			if (not array_impl()) {
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
			if (not array_impl()) {
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

		json group(std::function<std::string(json& _item)> _get_group)
		{
			if (not array_impl())
			{
				throw std::logic_error("Not an array");
			}

			json new_object(std::make_shared<json_object>());

			for (int i = 0; i < size(); i++)
			{
				auto element = get_element(i);
				std::string key = _get_group(element);
				if (not new_object.has_member(key)) {
					new_object.put_member_array(key);
				}
				new_object[key.c_str()].put_element(-1, element);
			}

			return new_object;
		}

		int size() 
		{
			if (object_impl())
				return object_impl()->members.size();
			else if (array_impl())
				return array_impl()->elements.size();
			else
				return 1;
		}

		class json_iterator
		{
			json* base;
			int index_lists;

		public:

			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = json;
			using pointer = json*;  // or also value_type*
			using reference = json&;  // or also value_type&

			json_iterator(json* _base, int _index) :
				base(_base),
				index_lists(_index)
			{
			}

			json_iterator() : base(nullptr), index_lists(0)
			{

			}

			json_iterator(const json_iterator& _src) : base(_src.base), index_lists(_src.index_lists)
			{

			}

			json_iterator& operator = (const json_iterator& _src)
			{
				base = _src.base;
				index_lists = _src.index_lists;
				return *this;
			}

			json operator *();

			inline json_iterator begin() const
			{
				return json_iterator(base, index_lists);
			}

			inline json_iterator end() const
			{
				return json_iterator(base, base->size());
			}

			inline json_iterator operator++()
			{
				if (index_lists < base->size()) {
					index_lists++;
				}
				return json_iterator(base, index_lists);
			}

			inline json_iterator operator++(int)
			{
				json_iterator tmp(*this);
				operator++();
				return tmp;
			}

			bool operator == (const json_iterator& _src) const
			{
				return (base == _src.base and _src.index_lists == index_lists);
			}

			bool operator != (const json_iterator& _src)
			{
				bool r = (*this == _src);
				return !r;
			}

		};

		json_iterator begin()
		{
			return json_iterator(this, 0);
		}

		json_iterator end()
		{
			return json_iterator(this, size());
		}
	};

	json json::json_iterator::operator *()
	{
		json r;

		if (base and index_lists < base->size() and index_lists >= 0) {
			r = base->get_element(index_lists);
		}
		return r;
	}

	std::map<std::string, json> json::get_members()
	{
		if (not object_impl()) {
			throw std::logic_error("Not an object");
		}
		std::map<std::string, json> mp;
		for (auto m : object_impl()->members)
		{
			json jx(m.second);
			mp.insert_or_assign(m.first, jx);
		}
		return mp;
	}

	class parser_base
	{
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

		int line_number = 1;
		int char_index = 0;

		bool has_errors()
		{
			return parse_errors.size() > 0;
		}

		json get_errors()
		{
			json error_root(std::make_shared<json_object>());

			error_root.put_member(class_name_field, parse_error_class);
			error_root.put_member(success_field, has_errors());

			json error_array(std::make_shared<json_array>());

			for (auto parse_error : parse_errors)
			{
				json err(std::make_shared<json_object>());
				err.put_member("line", parse_error.line);
				err.put_member("char", parse_error.char_index);
				err.put_member("topic", parse_error.topic);
				err.put_member("error", parse_error.error);
				error_array.append_element(err);
			}

			error_root.put_member("errors", error_array);
			return error_root;
		}
		
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
			if (not _src)
				return _src;
			while (std::isspace(*_src))
				_src++;
			return _src;
		}

		bool parse_symbol(std::string& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (std::isalpha(*_src))
			{
				result = true;
				while (isalnum(*_src) || *_src == '.' || *_src == '_')
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

		bool parse_member_type(std::string& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			std::string temp = "";
			_src = eat_white(_src);
			if (*_src == '$')
			{
				_src++;
				result = true;
				while (isalnum(*_src))
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
					if (*_src < 0 or std::iscntrl(*_src))
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

		bool parse_reference(object_reference_type& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isalnum(*_src))
			{
				_result.class_name = "";
				result = true;
				while (isalnum(*_src) or *_src == '_')
				{
					check_line(_src);
					if (*_src != '_') {
						_result.class_name += *_src;
					}
					_src++;
				}
				_src = eat_white(_src);
				if (*_src == ':')
					_src++;
				_src = eat_white(_src);
				result = parse_int64(_result.object_id, _src, &_src);
			}
			*_modified = _src;
			return result;
		}


		bool parse_number(double& _result, const char* _src, const char** _modified)
		{
			bool result = false;
			_src = eat_white(_src);
			if (isdigit(*_src) or *_src == '.' or *_src == '-')
			{
				std::string temp = "";
				result = true;
				while (isdigit(*_src) or *_src == '.' or *_src == '_' or *_src == '-')
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
			if (isdigit(*_src) or *_src == '-')
			{
				std::string temp = "";
				result = true;
				while (isdigit(*_src) or *_src == '_' or *_src == '-')
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
				while (isalnum(*_src) or *_src == '_')
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
				while (isalnum(*_src) or *_src == '_')
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


	};

	class json_parser : public parser_base
	{

	public:


		json from_double(double _d)
		{
			std::shared_ptr<json_double> dd = std::make_shared<json_double>();
			dd->value = _d;
			json result(dd);
			return result;
		}

		json from_integer(int64_t _d)
		{
			std::shared_ptr<json_int64> dd = std::make_shared<json_int64>();
			dd->value = _d;
			json result(dd);
			return result;
		}

		json from_datetime(date_time _d)
		{
			std::shared_ptr<json_datetime> dd = std::make_shared<json_datetime>();
			dd->value = _d;
			json result(dd);
			return result;
		}

		json from_string(std::string _d)
		{
			std::shared_ptr<json_string> dd = std::make_shared<json_string>();
			dd->value = _d;
			json result(dd);
			return result;
		}

		json parse_query(std::string _path)
		{
			json_parser jp;
			json j = jp.create_object();

			std::vector<std::string> path_components;
			path_components = split(_path, ':');
			if (path_components.size() == 2)
			{
				j.put_member("source_name", path_components[0]);
				j.put_member("query_path", path_components[1]);
			}
			else if (_path.find(':') != std::string::npos) {
				j.put_member("source_name", path_components[0]);
			}
			else {
				j.put_member("source_name", path_components[0]);
			}

			return j;
		}

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

		json create_array(std::vector<std::string>& _sarray)
		{
			line_number = 1;
			char_index = 0;
			parse_errors.clear();
			auto jo = std::make_shared<json_array>();
			json jn(jo);
			for (auto s : _sarray) {
				jn.push_back(s);
			}
			return jn;
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

	public:

		bool parse_delimited_string(json _dest_template, json& _column_map, const std::string& _src, char _delimiter)
		{
			bool r = false;

			std::string_view line;
			line = _src;

			if (_src.ends_with("\n"))
			{
				line = line.substr(0, line.size() - 1);
			}

			std::vector<std::string_view> pieces = split(line, _delimiter);

			for (int i = 0; i < pieces.size(); i++)
			{
				std::string index_key = std::to_string(i);
				std::string column_name = _column_map[index_key];
				bool t = _dest_template.import_member(column_name, pieces[i]);
				if (t) r = t;
			}

			return r;
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
				_src = new_src;
				if (member_type == "double")
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
				else if (member_type == "datetime")
				{
					if (parse_string(new_string_value, _src, &new_src))
					{
						auto js = std::make_shared<json_datetime>();
						js->value.parse(new_string_value);
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "int64")
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
				else if (member_type == "blob")
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
				else if (member_type == "string")
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
				else if (member_type == "reference")
				{
					object_reference_type ort;
					if (parse_reference(ort, _src, &new_src))
					{
						auto js = std::make_shared<json_reference>();
						js->value = ort;
						_value = js;
						*_modified = new_src;
						return result;
					}
				}
				else if (member_type == "function")
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

				if (not _array) {
					_array = std::make_shared<json_array>();
				}
				_src++;
				while (*_src and *_src != ']') {
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
				while (*_src and *_src != '}')
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
						_src = eat_white(_src);
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
				if (*_src) _src++;
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
		return result.to_json_typed_escaped();
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

corona::json operator ""_jarray(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json operator ""_jobject(const char* _src, size_t _length)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

corona::json operator ""_jarray(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_array(_src);
	return result;
}

corona::json operator ""_jobject(const char* _src)
{
	corona::json_parser parser;
	auto result = parser.parse_object(_src);
	return result;
}

namespace corona 
{


	class json_serializable
	{
	public:
		virtual void get_json(json& _dest) 
		{
			;
		}
		virtual void put_json(json& _src)
		{
			;
		}
	};

	bool json::contains_text(std::string _text)
	{
		if (empty())
			return false;
		if (object_impl())
		{
			auto members = get_members();
			for (auto m : members)
			{
				if (m.second.contains_text(_text))
				{
					return true;
				}
			}
		}
		else if (array_impl())
		{
			for (auto e : *this)
			{
				if (e.contains_text(_text))
				{
					return true;
				}
			}
		}
		else {
			std::string temp = (std::string)*this;
			std::transform(temp.begin(), temp.end(), temp.begin(),
				[](unsigned char c) { return std::tolower(c); });
			std::transform(_text.begin(), _text.end(), _text.begin(),
				[](unsigned char c) { return std::tolower(c); });
			if (temp.find(_text) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	bool json::gt(json _value)
	{
		if (empty() or _value.empty())
			return false;
		if (is_datetime() and _value.is_datetime()) {
			date_time dtme = datetime_impl()->value;
			date_time dtv = _value.datetime_impl()->value;
			return dtme > dtv;
		}
		else if (is_double() and _value.is_double()) {
			double dme = double_impl()->value;
			double dv = _value.double_impl()->value;
			return dme > dv;
		}
		else if (is_int64() and _value.is_int64()) {
			int64_t ime = int64_impl()->value;
			int64_t iv = _value.int64_impl()->value;
			return ime > iv;
		}
		else if (_value.is_int64()) {
			int64_t ime = (int64_t)*this;
			int64_t iv = _value.int64_impl()->value;
			return ime > iv;
		}
		else if (_value.is_double()) {
			double dme = (double)*this;
			double dv = _value.double_impl()->value;
			return dme > dv;
		}
		else 
		{
			std::string sme = (std::string)(*this);
			std::string sv = (std::string)(_value);
			return sme > sv;
		}
	}

	bool json::lt(json _value)
	{
		if (empty() or _value.empty())
			return false;
		if (is_datetime() and _value.is_datetime()) {
			date_time dtme = datetime_impl()->value;
			date_time dtv = _value.datetime_impl()->value;
			return dtme < dtv;
		}
		else if (is_double() and _value.is_double()) {
			double dme = double_impl()->value;
			double dv = _value.double_impl()->value;
			return dme < dv;
		}
		else if (is_int64() and _value.is_int64()) {
			int64_t ime = int64_impl()->value;
			int64_t iv = _value.int64_impl()->value;
			return ime < iv;
		}
		else if (_value.is_int64()) {
			int64_t ime = (int64_t)*this;
			int64_t iv = _value.int64_impl()->value;
			return ime < iv;
		}
		else if (_value.is_double()) {
			double dme = (double)*this;
			double dv = _value.double_impl()->value;
			return dme < dv;
		}
		else
		{
			std::string sme = (std::string)(*this);
			std::string sv = (std::string)(_value);
			return sme < sv;
		}
	}

	bool json::eq(json _value)
	{
		if (empty() or _value.empty())
			return false;
		if (is_datetime() and _value.is_datetime()) {
			date_time dtme = datetime_impl()->value;
			date_time dtv = _value.datetime_impl()->value;
			return dtme == dtv;
		}
		else if (is_double() and _value.is_double()) {
			double dme = double_impl()->value;
			double dv = _value.double_impl()->value;
			return dme == dv;
		}
		else if (is_int64() and _value.is_int64()) {
			int64_t ime = int64_impl()->value;
			int64_t iv = _value.int64_impl()->value;
			return ime == iv;
		}
		else if (_value.is_int64()) {
			int64_t ime = (int64_t)*this;
			int64_t iv = _value.int64_impl()->value;
			return ime == iv;
		}
		else if (_value.is_double()) {
			double dme = (double)*this;
			double dv = _value.double_impl()->value;
			return dme == dv;
		}
		else
		{
			std::string sme = (std::string)(*this);
			std::string sv = (std::string)(_value);
			return sme == sv;
		}
	}

	json json::merge(json& _object)
	{
		json_parser jp;

		if (not object_impl())
		{
			throw std::logic_error("Not an object");
		}

		if (_object.object())
		{
			auto members = _object.get_members();
			for (auto member : members)
			{
				put_member(member.first, member.second);
			}
		}

		return *this;
	}

	json operator + (const json& _srcA, const json& _srcB)
	{
		json_parser jp;
		json result;
		if (_srcA.is_double() and _srcB.is_double())
		{
			double d1 = _srcA.get_double();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 + d2);
		}
		else if (_srcA.is_int64() and _srcB.is_int64())
		{
			int64_t d1 = _srcA.get_int64();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_integer(d1 + d2);
		}
		else if (_srcA.is_double() and _srcB.is_int64())
		{
			double d1 = _srcA.get_double();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_double(d1 + d2);
		}
		else if (_srcA.is_int64() and _srcB.is_double())
		{
			int64_t d1 = _srcA.get_int64();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 + d2);
		}
		else if (_srcA.is_string() or _srcB.is_string())
		{
			std::string d1 = (std::string)_srcA;
			std::string d2 = (std::string)_srcB;
			std::string r = d1 + d2;
			result = jp.from_string(r);
		}
		else if (_srcA.is_datetime() and _srcB.is_double())
		{
			date_time d1 = _srcA.get_datetime();
			time_t start_seconds = d1.get_time_t();
			int64_t elapsed_seconds = (int64_t)_srcB.get_double();
			time_t new_time = start_seconds + elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_datetime() or _srcB.is_int64())
		{
			date_time d1 = _srcA.get_datetime();
			time_t start_seconds = d1.get_time_t();
			int64_t elapsed_seconds = (int64_t)_srcB.get_int64();
			time_t new_time = start_seconds + elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_double() and _srcB.is_datetime())
		{
			date_time d1 = _srcB.get_datetime();
			int64_t start_seconds = (int64_t)_srcA.get_double();
			time_t  elapsed_seconds = d1.get_time_t();
			time_t new_time = start_seconds + elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_int64() or _srcB.is_datetime())
		{
			date_time d1 = _srcB.get_datetime();
			int64_t start_seconds = (int64_t)_srcA.get_int64();
			time_t  elapsed_seconds = d1.get_time_t();
			time_t new_time = start_seconds + elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		return result;
	}

	json operator - (const json& _srcA, const json& _srcB)
	{
		json_parser jp;
		json result;
		if (_srcA.is_double() and _srcB.is_double())
		{
			double d1 = _srcA.get_double();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 - d2);
		}
		else if (_srcA.is_int64() and _srcB.is_int64())
		{
			int64_t d1 = _srcA.get_int64();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_integer(d1 - d2);
		}
		else if (_srcA.is_double() and _srcB.is_int64())
		{
			double d1 = _srcA.get_double();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_double(d1 - d2);
		}
		else if (_srcA.is_int64() and _srcB.is_double())
		{
			int64_t d1 = _srcA.get_int64();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 - d2);
		}
		else if (_srcA.is_string() or _srcB.is_string())
		{
			std::string d1 = (std::string)_srcA;
			std::string d2 = (std::string)_srcB;

			size_t pos = d1.find(d2);
			while (pos != std::string::npos)
			{
				d1.replace(pos, d2.length(), "");
				pos = d1.find(d2, pos + d2.length());
			}
			result = jp.from_string(d1);
		}
		else if (_srcA.is_datetime() and _srcB.is_double())
		{
			date_time d1 = _srcA.get_datetime();
			time_t start_seconds = d1.get_time_t();
			int64_t elapsed_seconds = (int64_t)_srcB.get_double();
			time_t new_time = start_seconds - elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_datetime() or _srcB.is_int64())
		{
			date_time d1 = _srcA.get_datetime();
			time_t start_seconds = d1.get_time_t();
			int64_t elapsed_seconds = (int64_t)_srcB.get_int64();
			time_t new_time = start_seconds - elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_double() and _srcB.is_datetime())
		{
			date_time d1 = _srcB.get_datetime();
			int64_t start_seconds = (int64_t)_srcA.get_double();
			time_t  elapsed_seconds = d1.get_time_t();
			time_t new_time = start_seconds - elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		else if (_srcA.is_int64() or _srcB.is_datetime())
		{
			date_time d1 = _srcB.get_datetime();
			int64_t start_seconds = (int64_t)_srcA.get_int64();
			time_t  elapsed_seconds = d1.get_time_t();
			time_t new_time = start_seconds - elapsed_seconds;
			d1 = date_time(new_time);
			result = jp.from_datetime(d1);
		}
		return result;
	}

	json operator / (const json& _srcA, const json& _srcB)
	{
		json_parser jp;
		json result;
		if (_srcA.is_double() and _srcB.is_double())
		{
			double d1 = _srcA.get_double();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 / d2);
		}
		else if (_srcA.is_int64() and _srcB.is_int64())
		{
			int64_t d1 = _srcA.get_int64();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_integer(d1 / d2);
		}
		else if (_srcA.is_double() and _srcB.is_int64())
		{
			double d1 = _srcA.get_double();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_double(d1 / d2);
		}

		return result;
	}

	json operator * (const json& _srcA, const json& _srcB)
	{
		json_parser jp;
		json result;
		if (_srcA.is_double() and _srcB.is_double())
		{
			double d1 = _srcA.get_double();
			double d2 = _srcB.get_double();
			result = jp.from_double(d1 * d2);
		}
		else if (_srcA.is_int64() and _srcB.is_int64())
		{
			int64_t d1 = _srcA.get_int64();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_integer(d1 * d2);
		}
		else if (_srcA.is_double() and _srcB.is_int64())
		{
			double d1 = _srcA.get_double();
			int64_t d2 = _srcB.get_int64();
			result = jp.from_double(d1 * d2);
		}

		return result;
	}

	json negate(json& _src)
	{
		json_parser jp;
		json j = _src.clone();
		j = jp.from_double(0.0) - j;
		return j;
	}

	json json::build_member(std::string _key)
	{
		std::string key;
		json_parser jp;
		json empty = jp.create_object();
		json obj = make_path(_key, key, empty);
		return obj;
	}

	json json::make_path(std::string_view _path, std::string& _name, json _new_object)
	{
		json_parser jp;
		std::vector<std::string_view> items = split(_path, '.');
		json start = *this;

		int sz = items.size();
		int sze = sz - 1;

		for (int i = 0; i < sz; i++) {
			auto item = items[i];
			std::string member_name(item);
			_name = member_name;

			json child = start[member_name];

			if (child.object())
			{
				start = child;
			}
			else if (i < sze or _new_object.empty())
			{
				child = jp.create_object();				
				start.put_member(member_name, child);
				start = start[member_name];
			}
			else
			{
				child = _new_object;
				start.put_member(member_name, child);
				start = start[member_name];
			}
		}

		return start;
	}

	json json::query(std::string_view _path)
	{
		json_parser jp;
		json result = jp.create_object();
		std::vector<std::string_view> items = split(_path, '.');
		json start = *this;

		result.put_member("path", _path);
		result.put_member("value", *this);

		for (auto item : items)
		{
			result.put_member("name", item);
			if (start.object())
			{
				result.put_member("target", start);
				result.put_member("object", start);
				json new_start = start[item];
				start = new_start;
				result.put_member("value", start);
			}
			else if (start.array())
			{
				result.put_member("target", start);
				result.put_member("array", start);
				std::string sitem(item);
				if (is_number(sitem)) {
					int idx = std::strtol(sitem.c_str(), nullptr, 10);
					if (idx < 0 or idx >= start.size()) {
						return result;
					}
					start = start.get_element(idx);
					result.put_member("value", start);
				}
				else {
					std::vector<std::string_view> item_ops = split(item, '-');
					if (item_ops.size() < 2) {
						return result;
					}
					std::string_view &operation = item_ops[0];
					std::string_view &member_name = item_ops[1];
					if (item == "last")
					{
						int index_lists = start.size() - 1;
						if (index_lists < 0)
							return result;
						else {
							json v = start.get_element(index_lists);
							result.put_member("value", v);
							return v;;
						}
					}
					else if (item == "first")
					{
						int index_lists = start.size() - 1;
						if (index_lists < 0)
							return result;
						else {
							json v = start.get_element(index_lists);
							result.put_member("value", v);
							return v;;
						}
					}
					else if (operation == "count")
					{
						int count = 0;
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								count++;
							}
						}
						result.put_member("value", count);
						return result;
					}
					else if (operation == "min")
					{
						json mvalue;
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								if (mvalue.empty()) {
									mvalue = ch[member_name];
								}
								else {
									json tvalue = ch[member_name];
									if (tvalue.lt(mvalue)) {
										mvalue = tvalue;
									}
								}
							}
						}
						result.put_member("value", mvalue);
						return result;
					}
					else if (operation == "max")
					{
						json mvalue;
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								if (mvalue.empty()) {
									mvalue = ch[member_name];
								}
								else {
									json tvalue = ch[member_name];
									if (tvalue.gt(mvalue)) {
										mvalue = tvalue;
									}
								}
							}
						}
						result.put_member("value", mvalue);
						return result;
					}
					else if (operation == "sum")
					{
						double rsum = 0;
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								double t = (double)ch[member_name];
								t += rsum;
							}
						}
						result.put_member("value", rsum);
						return result;
					}
					else if (operation == "avg")
					{
						double rsum = 0;
						double count = 0;
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								double t = (double)ch[member_name];
								t += rsum;
								count += 1.0;
							}
						}
						double vvalue = 0;
						if (count > 0.0) {
							vvalue = rsum / count;
						}
						else {
							vvalue = 0.0;
						}
						result.put_member("value", rsum);
						return result;
					}
					else if (operation == "cat")
					{
						std::string sresult = "";
						std::string comma = "";
						for (auto ch : start) {
							if (ch.has_member(std::string(member_name))) {
								sresult += comma;
								sresult += (std::string)ch[member_name];
								comma = ", ";
							}
						}
						result.put_member("value", sresult);
						return result;
					}
				}
			}
		}
		return result;
	}

	class validation_error
	{
	public:
		std::string class_name;
		std::string field_name;
		std::string message;
		std::string filename;
		int			line_number;

		validation_error() = default;
		validation_error(const validation_error& _src) = default;
		validation_error(validation_error&& _src) = default;
		validation_error& operator = (const validation_error& _src) = default;
		validation_error& operator = (validation_error&& _src) = default;

		validation_error(std::string _field_name, std::string _message, std::string _filename, int _line_number)
		{
			class_name = "N/A";
			field_name = _field_name;
			message = _message;
			filename = _filename;
			line_number = _line_number;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member(class_name_field, class_name);
			_dest.put_member("field_name", field_name);
			_dest.put_member(message_field, message);
			_dest.put_member("filename", filename);
			_dest.put_member("line_number", line_number);
		}

		virtual void put_json(json& _src)
		{
			class_name = _src[class_name_field];
			field_name = _src["field_name"];
			message = _src[message_field];
			filename = _src["filename"];
			line_number = _src["line_number"];
		}
	};

}

/* todo, unit tests for the json parser, in particular, compare */

#endif
