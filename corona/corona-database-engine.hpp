/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the core database engine for the corona database server.  

*/


#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

/*********************************************** 

Journal

3/14/2024 - Objectives
Create and list teams and users, 
Team permission checks for all objects,
Create 

3/15/2024 - log
Create and edit 

3/28/2024 - 
API close to complete, rounding out for testing.

4/6/2024 -
A good chunk of this is coded, but I am dealing 
with getting the headers straightened out from 
the great module disaster.

9/18/2024 - rewrote most of the above because the design was stupid.

***********************************************/


namespace corona
{

	class class_definition_interface
	{
	public:
		virtual std::string get_class_name() = 0;
		virtual std::map<std::string, bool>& get_descendants() = 0;
		virtual std::map<std::string, bool>& get_ancestors() = 0;
	};

	class corona_database_interface 
	{
	public:

		virtual json create_database() = 0;
		virtual relative_ptr_type open_database(relative_ptr_type _header_location) = 0;

		virtual void apply_config(json _config) = 0;
		virtual json apply_schema(json _schema) = 0;

		virtual std::string get_random_code() = 0;

		virtual json create_user(json create_user_request) = 0;
		virtual json login_user(json _login_request) = 0;
		virtual json get_classes(json get_classes_request) = 0;
		virtual json get_class(json get_class_request) = 0;
		virtual json put_class(json put_class_request) = 0;
		virtual std::shared_ptr<class_definition_interface> get_class_interface(std::string _class_name) = 0;

		virtual json edit_object(json _edit_object_request) = 0;
		virtual json create_object(json create_object_request) = 0;
		virtual json put_object(json put_object_request) = 0;
		virtual json get_object(json get_object_request) = 0;
		virtual json delete_object(json delete_object_request) = 0;

		virtual json copy_object(json copy_request) = 0;
		virtual json query(json query_request) = 0;

		// these two are for internal use only
		virtual json select_object(json _key, bool _include_children) = 0;
		virtual json select_single_object(json _key, bool _include_children) = 0;

	};

	class corona_db_header_struct
	{
	public:
		int64_t								object_id;
		relative_ptr_type					classes_location;
		relative_ptr_type					indexes_location;
		iarray<list_block_header, 100>		free_lists;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
		}
	};

	using class_method_key = std::tuple<std::string, std::string>;

	class corona_db_header : public poco_node<corona_db_header_struct>
	{
	public:

		int64_t write_free_list(file_block* _file, int _index)
		{
			auto offset = offsetof(corona_db_header_struct, free_lists) + _index * sizeof(list_block_header);
			auto size = sizeof(list_block_header);
			memcpy(bytes.get_ptr() + offset, (const char*)&data + offset, size);
			auto r = write_piece(_file, offset, size);
			return r;
		}
	};

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
			field_name = _src["field_anme"];
			message = _src[message_field];
			filename = _src["filename"];
			line_number = _src["line_number"];
		}
	};

	class field_options_base
	{
	public:
		bool required;

		field_options_base() = default;
		field_options_base(const field_options_base& _src) = default;
		field_options_base(field_options_base&& _src) = default;
		field_options_base& operator = (const field_options_base& _src) = default;
		field_options_base& operator = (field_options_base&& _src) = default;
		virtual ~field_options_base() = default;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("required", required);
		}

		virtual void put_json(json& _src)
		{
			required = (bool)_src["required"];
		}

		virtual void init_validation(corona_database_interface* _db)
		{
			;
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, json& _object)
		{
			json empty;
			return empty;
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			bool is_empty = _object_to_test.empty();
			if (required and is_empty) {
				validation_error ve;
				ve.class_name = _class_name;
				ve.field_name = _field_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "required field";
				_validation_errors.push_back(ve);
				return false;
			};
		}
	};

	class array_field_options : public field_options_base
	{
	public:
		std::map<std::string, bool> allowed_classes;
		std::map<std::string, bool> all_allowed_classes;

		array_field_options() = default;
		array_field_options(const array_field_options& _src) = default;
		array_field_options(array_field_options&& _src) = default;
		array_field_options& operator = (const array_field_options& _src) = default;
		array_field_options& operator = (array_field_options&& _src) = default;
		virtual ~array_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);

			json_parser jp;
			json jallowed = jp.create_object();
			json jall_allowed = jp.create_object();

			for (auto ac : allowed_classes) {
				jallowed.put_member(ac.first, true);
			}

			_dest.put_member("allowed_classes", jallowed);

			for (auto ac : all_allowed_classes) {
				jall_allowed.put_member(ac.first, true);
			}

			_dest.put_member("all_allowed_classes", jall_allowed);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);

			json jallowed = _src["allowed_classes"];
			auto members = jallowed.get_members();
			allowed_classes.clear();
			for (auto member : members) {
				allowed_classes.insert_or_assign(member.first, true);
			}
		}

		virtual void init_validation(corona_database_interface* _db)
		{
			all_allowed_classes.clear();
			for (auto class_name_pair : allowed_classes) {
				auto ci = _db->get_class_interface(class_name_pair.first);
				if (ci) {
					auto descendants = ci->get_descendants();
					for (auto descendant : descendants) {
						all_allowed_classes.insert_or_assign(descendant.first, true);
					}
				}
			}
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (_object_to_test.array()) 
				{
					for (auto obj : _object_to_test) 
					{
						std::string object_class_name;
						if (obj.object()) {
							object_class_name = _object_to_test[class_name_field];
						}
						else if (obj.is_datetime())
						{
							object_class_name = "datetime";
						}
						else if (obj.is_double())
						{
							object_class_name = "double";
						}
						else if (obj.is_int64())
						{
							object_class_name = "int";
						}
						else if (obj.is_string())
						{
							object_class_name = "string";
						}
						// TODO: make this include descendants
						if (all_allowed_classes.contains(object_class_name))
						{
							is_legit = true;
						}
						else 
						{
							is_legit = false;
						}
					}
				}
				else 
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "Value must be an array of correct types.";
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}
	};

	class object_field_options : public field_options_base
	{
	public:
		std::map<std::string, bool> allowed_classes;
		std::map<std::string, bool> all_allowed_classes;

		object_field_options() = default;
		object_field_options(const object_field_options& _src) = default;
		object_field_options(object_field_options&& _src) = default;
		object_field_options& operator = (const object_field_options& _src) = default;
		object_field_options& operator = (object_field_options&& _src) = default;
		virtual ~object_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);

			json_parser jp;
			json jallowed = jp.create_object();

			for (auto ac : allowed_classes) {
				jallowed.put_member(ac.first, true);
			}

			_dest.put_member("allowed_classes", jallowed);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);

			json jallowed = _src["allowed_classes"];
			auto members = jallowed.get_members();
			allowed_classes.clear();
			for (auto member : members) {
				allowed_classes.insert_or_assign(member.first, true);
			}
		}

		virtual void init_validation(corona_database_interface* _db)
		{
			all_allowed_classes.clear();
			for (auto class_name_pair : allowed_classes) {
				auto ci = _db->get_class_interface(class_name_pair.first);
				if (ci) {
					auto descendants = ci->get_descendants();
					for (auto descendant : descendants) {
						all_allowed_classes.insert_or_assign(descendant.first, true);
					}
				}
			}
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (_object_to_test.object())
				{
					std::string object_class_name;
					if (_object_to_test.object()) {
						object_class_name = _object_to_test[class_name_field];
					}
					if (all_allowed_classes.contains(object_class_name))
					{
						is_legit = true;
					}
					else {
						is_legit = false;
					}
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value must be an object of correct type.";
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}

	};

	class string_field_options : public field_options_base
	{
	public:
		int maximum_length;
		int minimum_length;
		std::string match_pattern;

		string_field_options() = default;
		string_field_options(const string_field_options& _src) = default;
		string_field_options(string_field_options&& _src) = default;
		string_field_options& operator = (const string_field_options& _src) = default;
		string_field_options& operator = (string_field_options&& _src) = default;
		virtual ~string_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member_i64("max_length", maximum_length);
			_dest.put_member_i64("min_length", minimum_length);
			_dest.put_member("match_pattern", match_pattern);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			minimum_length = _src["min_length"];
			maximum_length = _src["max_length"];
			match_pattern = _src["match_pattern"];
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				std::string chumpy = (std::string)_object_to_test;

				if (chumpy.size() >= minimum_length and chumpy.size() <= maximum_length)
				{
					if (not match_pattern.empty()) {
						std::regex rgx(match_pattern);
						if (std::regex_match(chumpy, rgx)) {
							is_legit = true;
						}
						else {
							is_legit = false;
						}
					}
					else {
						is_legit = true;
					}
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format( "Value '{0}' must be between {1} and {2} characters long", chumpy, minimum_length, maximum_length);
					if (match_pattern.size() > 0) {
						ve.message += std::format(" and must match '{0}'", match_pattern);
					}
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}

	};

	class int64_field_options : public field_options_base
	{
	public:
		int64_t min_value;
		int64_t max_value;

		int64_field_options() = default;
		int64_field_options(const int64_field_options& _src) = default;
		int64_field_options(int64_field_options&& _src) = default;
		int64_field_options& operator = (const int64_field_options& _src) = default;
		int64_field_options& operator = (int64_field_options&& _src) = default;
		virtual ~int64_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member_i64("min_value", min_value);
			_dest.put_member_i64("max_value", max_value);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			min_value = (int64_t)_src["min_value"];
			max_value = (int64_t)_src["max_value"];
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				int64_t chumpy = (int64_t)_object_to_test;

				if (chumpy >= min_value and chumpy <= max_value)
				{
					is_legit = true;
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format("Value '{0}' must be between {1} and {2}", chumpy, min_value, max_value);
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}

	};

	template <typename scalar_type> 
	class general_field_options : public field_options_base
	{
	public:
		scalar_type min_value;
		scalar_type max_value;

		general_field_options() = default;
		general_field_options(const general_field_options& _src) = default;
		general_field_options(general_field_options&& _src) = default;
		general_field_options& operator = (const general_field_options& _src) = default;
		general_field_options& operator = (general_field_options&& _src) = default;
		virtual ~general_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("min_value", min_value);
			_dest.put_member("max_value", max_value);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			min_value = (scalar_type)_src["min_value"];
			max_value = (scalar_type)_src["max_value"];
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				scalar_type chumpy = (scalar_type)_object_to_test;

				if (chumpy >= min_value and chumpy <= max_value)
				{
					is_legit = true;
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value out of range";
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}

	};

	class choice_field_options : public field_options_base
	{
	public:
		json		filter;
		json		options;

		std::string	id_field_name;
		std::string	description_field_name;

		choice_field_options() = default;
		choice_field_options(const choice_field_options& _src) = default;
		choice_field_options(choice_field_options&& _src) = default;
		choice_field_options& operator = (const choice_field_options& _src) = default;
		choice_field_options& operator = (choice_field_options&& _src) = default;
		virtual ~choice_field_options() = default;

		virtual void init_validation(corona_database_interface* _db)
		{
			json_parser jp;
			json key = jp.create_object();
			options = _db->select_object(filter, false);
		}

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("filter", filter);
			_dest.put_member("id_field_name", id_field_name);
			_dest.put_member("description_field_name", description_field_name);
			_dest.put_member("options", options);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			field_options_base::put_json(_src);
			filter = _src["filter"];
			id_field_name = _src["id_field_name"];
			description_field_name = _src["description_field_name"];
			options = jp.create_array();
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_empty = _object_to_test.empty();
				if (is_empty) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value is empty";
					_validation_errors.push_back(ve);
					return false;
				}
				else if (options.empty()) 
				{
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "validation list not loaded";
					_validation_errors.push_back(ve);
					return false;
				}
				else 
				{
					std::string object_test_id;
					if (_object_to_test.object())
					{
						object_test_id = _object_to_test[id_field_name];
					}
					else if (_object_to_test.is_string())
					{
						object_test_id = _object_to_test;
					}
					else
						return false;

					for (auto option : options)
					{
						std::string id_field = option[id_field_name];
						if (id_field == object_test_id)
							return true;
					}
				}
				return true;
			}
			return false;
		}

	};

	class query_field_options : public field_options_base
	{
	public:
		json		query_body;

		query_field_options() = default;
		query_field_options(const query_field_options& _src) = default;
		query_field_options(query_field_options&& _src) = default;
		query_field_options& operator = (const query_field_options& _src) = default;
		query_field_options& operator = (query_field_options&& _src) = default;
		virtual ~query_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("query", query_body);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			field_options_base::put_json(_src);
			query_body = _src["query"];
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, json& _object)
		{
			json_parser jp;
			json this_query_body = query_body.clone();
			json froms = query_body["from"];
			if (froms.array()) {
				json new_from = jp.create_object();
				new_from.put_member(data_field, _object);
				new_from.copy_member(class_name_field, _object);
				new_from.put_member("name", "this");
			}
			this_query_body.put_member(token_field, _token);
			json query_results, query_data_results;
			query_results = _db->query(this_query_body);
			if (query_results[success_field]) {
				query_data_results = query_results[data_field];
			}
			else 
			{
				query_data_results = jp.create_array();
			}
			return query_data_results;
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			return false;
		}

	};

	class field_definition {
	public:
		std::string field_name;
		std::string field_type;
		std::shared_ptr<field_options_base> options;

		field_definition() = default;
		field_definition(const field_definition& _src) = default;
		field_definition(field_definition&& _src) = default;
		field_definition& operator = (const field_definition& _src) = default;
		field_definition& operator = (field_definition&& _src) = default;
		virtual ~field_definition() = default;

		virtual void init_validation(corona_database_interface* _db)
		{
			if (options) options->init_validation(_db);
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (options) {
				return options->accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test);
			}
			return true;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			json joptions = jp.create_object();

			_dest.put_member("field_name", field_name);
			_dest.put_member("field_type", field_type);

			if (options) {
				options->get_json(_dest);
			}
		}

		virtual void put_json(json& _src)
		{
			field_type = _src["field_type"];
			field_name = _src["field_name"];

			if (field_type == "object") 
			{
				options = std::make_shared<object_field_options>();
				options->put_json(_src);
			}
			else if (field_type == "array")
			{
				options = std::make_shared<array_field_options>();
				options->put_json(_src);
			}
			else if (field_type == "number" || field_type == "double")
			{
				options = std::make_shared<general_field_options<double>>();
				options->put_json(_src);
			}
			else if (field_type == "int64")
			{
				options = std::make_shared<int64_field_options>();
				options->put_json(_src);
			}
			else if (field_type == "string")
			{
				options = std::make_shared<string_field_options>();
				options->put_json(_src);
			}
			else if (field_type == "bool")
			{
				options = std::make_shared<field_options_base>();
				options->put_json(_src);
			}
			else if (field_type == "datetime")
			{
				options = std::make_shared<general_field_options<date_time>>();
				options->put_json(_src);
			}
			else if (field_type == "query")
			{
				options = std::make_shared<query_field_options>();
				options->put_json(_src);
			}
			else if (field_type == "function")
			{
				;
			}
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, json& _object)
		{
			json results;
			if (options) {
				results = options->run_queries(_db, _token, _object);
			}
			return results;
		}
	};

	class index_definition 
	{
	public:
		std::string index_name;
		std::vector<std::string> index_keys;
		int64_t index_location;

		index_definition() = default;
		index_definition(const index_definition& _src) = default;
		index_definition(index_definition&& _src) = default;
		index_definition& operator = (const index_definition& _src) = default;
		index_definition& operator = (index_definition&& _src) = default;

		std::string index_key_string()
		{
			return join(index_keys, ".");
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			_dest.put_member("index_name", index_name);

			json jindex_keys = jp.create_array();
			for (auto ikey : index_keys) {
				jindex_keys.push_back(ikey);
			}
			_dest.put_member("index_keys", jindex_keys);
			_dest.put_member_i64("index_location", index_location);
		}

		virtual void put_json(json& _src)
		{			
			index_name = _src["index_name"];

			json jindex_keys = _src["index_keys"];
			if (jindex_keys.array())
			{
				index_keys.clear();
				for (auto key : jindex_keys) {
					std::string key_name = key;
					index_keys.push_back(key);
				}
			}
			index_location = (int64_t)_src["index_location"];
		}
	};

	class class_definition : public class_definition_interface
	{
	public:
		std::string class_name;
		std::string class_description;
		std::string base_class_name;
		int64_t		table_location;
		std::map<std::string, std::shared_ptr<field_definition>> fields;
		std::map<std::string, std::shared_ptr<index_definition>> indexes;
		std::map<std::string, bool> ancestors;
		std::map<std::string, bool> descendants;

		class_definition() = default;
		class_definition(const class_definition& _src) = default;
		class_definition(class_definition&& _src) = default;
		class_definition& operator = (const class_definition& _src) = default;
		class_definition& operator = (class_definition&& _src) = default;

		virtual std::string get_class_name() 
		{
			return class_name;
		}

		virtual std::map<std::string, bool>& get_descendants()
		{
			return descendants;
		}

		virtual std::map<std::string, bool>& get_ancestors()
		{
			return ancestors;
		}

		bool empty()
		{
			return class_name.empty();
		}

		virtual void init_validation(corona_database_interface* _db)
		{
			for (auto& fld : fields) {
				fld.second->init_validation(_db);
			}
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member(class_name_field, class_name);
			_dest.put_member("class_description", class_description);
			_dest.put_member("base_class_name", base_class_name);
			_dest.put_member_i64("table_location", table_location);

			if (fields.size() > 0) {
				json jfield_object = jp.create_object();
				for (auto field : fields) {
					json jfield_definition = jp.create_object();
					field.second->get_json(jfield_definition);
					jfield_object.put_member(field.first, jfield_definition);
				
				}
				_dest.put_member("fields", jfield_object);
			}

			if (indexes.size() > 0) {
				json jindex_object = jp.create_object();
				for (auto index : indexes) {
					json jindex_definition = jp.create_object();
					index.second->get_json(jindex_definition);
					jindex_object.put_member(index.first, jindex_definition);
				}
				_dest.put_member("indexes", jindex_object);
			}

			if (ancestors.size() > 0) {
				json jancestor_array = jp.create_array();
				for (auto class_ancestor : ancestors) {
					jancestor_array.push_back(class_ancestor.first);
				}
				_dest.put_member("ancestors", jancestor_array);
			}

			if (descendants.size() > 0) {
				json jdescendants_array = jp.create_array();
				for (auto class_descendant : descendants) {
					jdescendants_array.push_back(class_descendant.first);
				}
				_dest.put_member("descendants", jdescendants_array);
			}

		}

		virtual void put_json(json& _src)
		{
			json jfields, jindexes, jancestors, jdescendants;

			class_name = _src[class_name_field];
			class_description = _src["class_description"];
			base_class_name = _src["base_class_name"];
			table_location = (int64_t)_src["table_location"];

			ancestors.clear();
			jancestors = _src["ancestors"];
			if (jancestors.array())
			{
				for (auto jancestor : jancestors)
				{
					std::string ancestor = jancestor;
					ancestors.insert_or_assign(ancestor, true);
				}
			}

			descendants.clear();
			jdescendants = _src["descendants"];
			if (jdescendants.array())
			{
				for (auto jancestor : jancestors)
				{
					std::string ancestor = jancestor;
					ancestors.insert_or_assign(ancestor, true);
				}
			}

			fields.clear();
			jfields = _src["fields"];

			if (jfields.object()) {
				auto jfield_members = jfields.get_members();
				for (auto jfield : jfield_members) {
					std::shared_ptr<field_definition> field = std::make_shared<field_definition>();
					field->field_name = jfield.first;
					if (jfield.second.object()) 
					{
						field->put_json(jfield.second);
					}
					else if (jfield.second.is_string()) 
					{
						field->field_type = jfield.second;
					}
					if (allowed_field_types.contains(field->field_type)) {
						fields.insert_or_assign(field->field_name, field);
					}
				}
			}

			indexes.clear();
			jindexes = _src["indexes"];
			if (jindexes.object()) {
				auto jindex_members = jindexes.get_members();
				for (auto jindex : jindex_members) {
					std::shared_ptr<index_definition> index = std::make_shared<index_definition>();
					index->put_json(jindex.second);
					index->index_name = jindex.first;
					indexes.insert_or_assign(index->index_name, index);
				}
			}
		}

		virtual void clear_queries(json& _target)
		{
			json_parser jp;
			for (auto fldpair : fields) {
				auto query_field = fldpair.second;
				if (query_field->field_type == "query") {
					json empty_array = jp.create_array();
					_target.put_member(query_field->field_name, empty_array);
				}
			}
		}

		virtual void run_queries(corona_database_interface* _db, std::string& _token, json& _target)
		{
			for (auto fldpair : fields) {
				auto query_field = fldpair.second;
				if (query_field->field_type == "query") {
					json objects = query_field->run_queries(_db, _token, _target);
					_target.put_member(query_field->field_name, objects);
				}
			}
		}
	};

	class corona_database : public file_block, public corona_database_interface
	{
		corona_db_header header;

		json schema;

		std::map<class_method_key, json_function_function> functions;

		crypto crypter;

		std::string send_grid_api_key;
		bool watch_polling;

		const std::string auth_general = "auth-general";
		const std::string auth_system = "auth-system";
		
		/*
		* authorizations in tokens, methods and progressions
		* 
		create_user->login_user
		login_user->send_confirmation_code, send_password_reset_code
		send_login_confirmation_code->receive_login_confirmation_code
		receive_login_confirmation_code->user - connected
		send_password_reset_code->receive_reset_password_code
		receive_reset_password_code->user - connected

		connected can:
		edit_object
		get_classes
		get_class
		put_class
		query_class
		create_object
		put_object
		get_object
		copy_object
		delete_object
		*/

		lockable classes_rw_lock;
		lockable objects_rw_lock;
		lockable header_rw_lock;

		std::shared_ptr<json_table> classes;
		std::shared_ptr<json_table> indexes;

	public:

		bool trace_check_class = false;

		allocation_index get_allocation_index(int64_t _size)
		{
			allocation_index ai = data_block::get_allocation_index(_size);
			if (ai.index >= header.data.free_lists.capacity()) {
				std::string msg = std::format("{0} bytes is too big to allocate as a block.", _size);
				system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
				ai.index = header.data.free_lists.capacity() - 1;
			}
			return ai;
		}

		virtual relative_ptr_type allocate_space(int64_t _size, int64_t *_actual_size) override
		{
			relative_ptr_type pt = 0;

			allocation_index ai = get_allocation_index(_size);

			auto& list_start = header.data.free_lists[ai.index];

			allocation_block_struct free_block = {};

			// get_allocation index always returns an index such that any block in that index
			// will have the right size.  so we know the first block is ok.
			if (list_start.first_block)
			{
				read(list_start.first_block, &free_block, sizeof(free_block));
				// this
				if (list_start.last_block == list_start.first_block)
				{
					list_start.last_block = 0;
					list_start.first_block = 0;
				}
				else
				{
					list_start.first_block = free_block.next_block;
					free_block.next_block = 0;
				}
				header.write_free_list(this, ai.index);
				return free_block.data_location;
			}

			int64_t total_size = sizeof(allocation_block_struct) + ai.size;
			int64_t base_space = add(sizeof(allocation_block_struct) + ai.size);
			if (base_space > 0) {
				free_block.data_location = base_space + sizeof(allocation_block_struct);
				free_block.next_block = 0;
				write(base_space, &free_block, sizeof(free_block));
				return free_block.data_location;
			}

			return 0;
		}


		virtual void free_space(int64_t _location) override
		{

			relative_ptr_type block_start = _location - sizeof(allocation_block_struct);

			allocation_block_struct free_block = {};

			/* ah, the forgiveness of this can be evil.  Here we say, if we truly cannot verify this
			_location can be freed, then do not free it.  Better to keep some old block around than it is
			to stomp on a good one with a mistake.  */

			file_command_result fcr = read(block_start, &free_block, sizeof(free_block));

			// did we read the block
			if (fcr.success) {

				// is this actually an allocated space block, and, is it the block we are trying to free
				if (free_block.data_location == _location) {

					// ok, now let's have a go and see where this is in our allocation index.
					allocation_index ai = get_allocation_index(free_block.data_capacity);

					// this check says, don't try to free the block if there is a mismatch between its 
					// allocation size, and the size it says it has.
					if (free_block.data_capacity == ai.size)
					{
						// and now, we can look at the free list, and put ourselves at the 
						// end of it.  In this way, deletes and reallocates will tend to 
						// want to reuse the same space so handy for kill and fills.
						auto& list_start = header.data.free_lists[ai.index];

						// there is a last block, so we are at the end
						if (list_start.last_block) {
							relative_ptr_type old_last_block = list_start.last_block;
							allocation_block_struct last_free;
							auto fr = read(old_last_block, &last_free, sizeof(last_free));
							if (fr.success) {
								list_start.last_block = block_start;
								last_free.next_block = block_start;
								free_block.next_block = 0;
								write(old_last_block, &last_free, sizeof(last_free));
								write(block_start, &free_block, sizeof(free_block));

								// this basically says, just write out the list header for this index
								// not the whole header
								header.write_free_list(this, ai.index);
							}
						}
						else
						{
							// the list is empty and we add to it.
							free_block.next_block = 0;
							list_start.last_block = free_block.data_location - sizeof(allocation_block_struct);
							list_start.last_block = free_block.data_location - sizeof(allocation_block_struct);

							// this basically says, now save our block.
							write(block_start, &free_block, sizeof(free_block));

							// this basically says, just write out the list header for this index
							// not the whole header
							header.write_free_list(this, ai.index);
						}
					}
				}
			}
		}

		json create_database()
		{
			json result;
			timer method_timer;

			json created_classes;
			relative_ptr_type class_location;
			relative_ptr_type header_location;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_job_start("create_database", "start", start_time, __FILE__, __LINE__);
			
			scope_lock lock_one(classes_rw_lock);
			scope_lock lock_two(objects_rw_lock);

			header.data.object_id = 1;
			header_location = header.append(this);

			header.data.object_id = 1;
			header.data.classes_location =  classes->create();
			header.data.indexes_location = indexes->create();

			created_classes = jp.create_object();

			header.write(this);
			commit_check();
	
			json response =  create_class(R"(
{	
	"class_name" : "sys_object",
	"class_description" : "Base of all objects",
	"fields" : {			
			"object_id" : "int64",
			"class_name" : "string",
			"created" : "datetime",
			"created_by" : "string",
			"updated": "datetime",
 			"updated_by" :"string" 
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json test =  classes->get(R"({"class_name":"sys_object"})");
			if (test.empty() or test.is_member("class_name", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysObject after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_object", true);

			response = create_class(R"(
{
	"class_name" : "sys_parent_child",
	"base_class_name" : "sys_object",
	"class_description" : "parent_child relationship",
	"fields" : {
		"parent_id" : "int64",
		"parent_member" : "string",
		"child_class" : "string",
		"child_id" : "int64"
	},
	"indexes" : {
		"parent_child_index" : {
			"index_keys" : [ "parent_id", "parent_member" ]
		}
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test = classes->get(R"({"class_name":"sys_object"})");
			if (test.empty() or test.is_member("class_name", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysObject after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_object", true);



			response =  create_class(R"(
{
	"class_name" : "sys_schemas",
	"base_class_name" : "sys_object",
	"class_description" : "Database script changes",
	"fields" : [			
			"schema_name" : "string",
			"schema_description" : "string",
			"schema_version" : "string",
			"schema_authors" : "array",
			"classes" : "array",
			"users" : "array",
			"datasets" : "array" 
		]
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"class_name":"sys_schemas"})");
			if (test.empty() or test.is_member("class_name", "SysParseErrors")) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_schemas after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			created_classes.put_member("sys_schemas", true);

			response =  create_class(R"(
{
	"class_name" : "sys_datasets",
	"base_class_name" : "sys_object",
	"class_description" : "Database script changes",
	"fields" : {
			"dataset_name" : "string",
			"dataset_description" : "string",
			"dataset_version" : "string",
			"dataset_authors" : "array",
			"run_on_change": "bool",
			"objects": "array",
			"import" : "object"
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_datasets put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"class_name":"sys_datasets"})");
			if (test.empty() or test.is_member("class_name", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_datasets after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_datasets", true);

			response =  create_class(R"(
{	
	"base_class_name" : "sys_object",
	"class_name" : "sys_users",
	"class_description" : "A user",
	"fields" : {			
			"class_name" : "string",
			"first_name" : "string",
			"last_name" : "string",
			"user_name" : "string",
			"email" : "string",
			"mobile" : "string",
			"street1" : "string",
			"street2" : "string",
			"city" : "string",
			"state" : "string",
			"zip" : "string",
			"password" : "string"
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_users put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"class_name":"sys_users"})");
			if (test.empty() or test.is_member("class_name", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_users after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_users", true);

			json gc = jp.create_object();
			json gcr = create_system_request( gc );

			json classes_array_response =  get_classes(gcr);
			json classes_array = classes_array_response[data_field];
			json classes_grouped = classes_array.group([](json& _item) -> std::string {
				return (std::string)_item[class_name_field];
				});

			bool missing = classes_array.any([classes_grouped](json& _item) {
				return !classes_grouped.has_member(_item[class_name_field]);
				});

			if (missing) {
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				std::cout << __FILE__ << " " << __LINE__ << ":Class list returned from database missed some classes." << std::endl;
				return result;
			}

			json new_user_request;
			json new_user_data;

			new_user_data = jp.create_object();
			new_user_data.put_member(class_name_field, "sys_users");
			new_user_data.put_member(user_name_field, default_user);
			new_user_data.put_member(user_email_field, default_email_address);
			new_user_data.put_member("password1", default_password);
			new_user_data.put_member("password2", default_password);

			new_user_request = create_system_request(new_user_data);
			json new_user_result =  create_user(new_user_request);
			json new_user = new_user_result[data_field];
			json user_return = create_response(new_user_request, true, "Ok", new_user, method_timer.get_elapsed_seconds());
			response = create_response(new_user_request, true, "Database Created", user_return, method_timer.get_elapsed_seconds());

			commit_check();

			system_monitoring_interface::global_mon->log_job_stop("create_database", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

private:

		virtual void commit_check()
		{
			time_t current;
			time(&current);
			if ((current - last_commit) > 1) {
				commit();
			}
		}

		virtual void extract_child_objects(std::shared_ptr<class_definition>& _cdef, json& _child_objects, json& _src_list)
		{
			json_parser jp;
			for (auto _src_obj : _src_list)
			{
				int64_t parent_object_id = (int64_t)_src_obj[object_id_field];
				for (auto& fpair : _cdef->fields) {
					auto& fld = fpair.second;
					if (fld->field_type == "array")
					{
						json fld_array = _src_obj[fld->field_name];
						if (fld_array.array()) {
							json new_array = jp.create_array();
							for (auto item : fld_array) {
								if (item.object() and item.has_member(class_name_field)) {

									int64_t child_object_id;
									std::string child_class_name = item[class_name_field];

									if (not item.has_member(object_id_field)) {
										child_object_id = get_next_object_id();
										item.put_member_i64(object_id_field, child_object_id);
									}
									else {
										child_object_id = item[object_id_field];
									}
									_child_objects.push_back(item);

									json parent_child = jp.create_object();
									parent_child.put_member(class_name_field, "sys_parent_child");
									parent_child.put_member_i64("parent_id", parent_object_id);
									parent_child.put_member("parent_member", fld->field_name);
									parent_child.put_member("child_class", child_class_name);
									parent_child.put_member("child_id", child_object_id);
									json existing_child = select_object(parent_child, false);
									if (existing_child.array() and existing_child.size() == 0)
									{
										_child_objects.push_back(parent_child);
									}
								}
								else
								{
									new_array.push_back(item);
								}
							}
							_src_obj.put_member(fld->field_name, new_array);
						}
					}
					else if (fld->field_type == "object")
					{
						json item = _src_obj[fld->field_name];
						if (item.object() and item.has_member(class_name_field)) {
							int64_t child_object_id;
							std::string child_class_name = item[class_name_field];

							if (not item.has_member(object_id_field)) {
								child_object_id = get_next_object_id();
								item.put_member_i64(object_id_field, child_object_id);
							}
							else {
								child_object_id = item[object_id_field];
							}
							_child_objects.push_back(item);

							json parent_child = jp.create_object();
							parent_child.put_member(class_name_field, "sys_parent_child");
							parent_child.put_member_i64("parent_id", parent_object_id);
							parent_child.put_member("parent_member", fld->field_name);
							parent_child.put_member("child_class", child_class_name);
							parent_child.put_member("child_id", child_object_id);
							json existing_child = select_object(parent_child, false);
							if (existing_child.array() and existing_child.size() == 0)
							{
								_child_objects.push_back(parent_child);
							}
							json empty = jp.create_object();
							_src_obj.put_member(fld->field_name, empty);
						}
					}
				}
			}
		}

		virtual void select_child_objects(std::shared_ptr<class_definition>& _cdef, json& _src_list)
		{
			json_parser jp;
			for (auto _src_obj : _src_list)
			{
				json key = jp.create_object();
				int64_t object_id = (int64_t)_src_obj[object_id_field];
				key.put_member(class_name_field, "sys_parent_child");
				key.put_member_i64("parent_id", object_id);
				for (auto& fpair : _cdef->fields) {
					auto& fld = fpair.second;
					key.put_member("parent_member", fld->field_name);
					json link_objects = select_object(key, true);

					if (fld->field_type == "array")
					{
						json existing = _src_obj[fld->field_name];
						if (not existing.array()) {
							json new_array = jp.create_array();
							_src_obj.put_member(fld->field_name, new_array);
							existing = _src_obj[fld->field_name];
						}
						for (auto link_object : link_objects)
						{
							json child_select = jp.create_object();
							std::string child_class = link_object["child_class"];
							int64_t child_object_id = (int64_t)link_object["child_id"];
							child_select.put_member(class_name_field, child_class);
							child_select.put_member_i64(object_id_field, child_object_id);
							auto child_objects = select_object(child_select, true);
							for (auto child : child_objects) {
								existing.push_back(child);
							}
						}
					}
					else if (fld->field_type == "object")
					{
						for (auto link_object : link_objects)
						{
							json child_select = jp.create_object();
							std::string child_class = link_object["child_class"];
							int64_t child_object_id = (int64_t)link_object["child_id"];
							child_select.put_member(class_name_field, child_class);
							child_select.put_member_i64(object_id_field, child_object_id);
							auto child_objects = select_object(child_select, true);
							for (auto child : child_objects) {
								_src_obj.put_member(fld->field_name, child);
							}
						}
					}
				}
			}
		}

		json create_class(std::string _text)
		{
			json_parser jp;

			json jclass_def = jp.parse_object(_text);

			json sys_request = create_system_request(jclass_def);

			json response = put_class(sys_request);

			return response;
		}

		json check_object(std::string _user_name, json object_load)
		{
			timer method_timer;
			json_parser jp;
			date_time current_date = date_time::now();

			json response,
				 result;

			response = jp.create_object();

			json object_definition,
				object_list,
				result_list;

			if (object_load.array()) {
				object_list = object_load;
			}
			else {
				object_list = jp.create_array();
				object_list.push_back(object_load);
			}

			result_list = jp.create_array();

			json classes_group = object_list.group([](json _item) -> std::string {
				return _item[class_name_field];
				});

			auto class_list = classes_group.get_members();

			std::vector<validation_error> validation_errors;

			bool all_objects_good = true;

			for (auto class_pair : class_list)
			{
				auto cd = load_class(class_pair.first);
				cd->init_validation(this);
			}

			for (auto object_definition : object_list)
			{
				if (not object_definition.object())
				{
					result_list.push_back(result);
				}

				if (not object_definition.has_member(class_name_field))
				{
					result_list.push_back(result);
				}

				db_object_id_type object_id = -1;

				if (object_definition.has_member(object_id_field))
				{
					object_id = object_definition[object_id_field].get_int64s();
					object_definition.put_member("updated", current_date);
					object_definition.put_member("updated_by", _user_name);
				}
				else
				{
					object_id =  get_next_object_id();
					object_definition.put_member_i64(object_id_field, object_id);
					object_definition.put_member("created", current_date);
					object_definition.put_member("created_by", _user_name);
				}

				json warnings = jp.create_array();

				std::string class_name = object_definition[class_name_field];

				auto class_data = load_class(class_name);

				if (class_data)
				{
					bool permission = has_class_permission(_user_name, class_name, "Put");

					if (not permission) {
						response.put_member(success_field, false);
						response.put_member(message_field, "denied");
						return result;
					}

					// if the user was a stooser and saved the query results with the object,
					// blank that out here because we will run that on load
					class_data->clear_queries(object_definition);

					// check the object against the class definition for correctness
					// first we see which fields are in the class not in the object

					for (auto kv : class_data->fields) {
						if (object_definition.has_member(kv.first)) {
							std::string obj_type = object_definition[kv.first]->get_type_name();
							std::string member_type = kv.second->field_type;
							if (member_type != obj_type) {
								object_definition.change_member_type(kv.first, member_type);
							}
						}						
					}

					// then we see which fields are in the object that are not 
					// in the class definition.
					auto object_members = object_definition.get_members();
					for (auto om : object_members) {
						if (class_data->fields.contains(om.first)) {
							auto& fld = class_data->fields[om.first];
							fld->accepts(this, validation_errors, class_name, om.first, om.second);
						}
						else 
						{
							json warning = jp.create_object();
							validation_error ve;
							ve.class_name = class_name;
							ve.field_name = om.first;
							ve.filename = __FILE__;
							ve.line_number = __LINE__;
							ve.message = "Field not found in class definition";
							validation_errors.push_back(ve);
						}
					}
					result = jp.create_object();
					if (validation_errors.size() > 0) {
						std::string msg = std::format("Object '{0}' has problems", class_name);
						for (auto& ve : validation_errors) {
							json jve = jp.create_object();
							ve.get_json(jve);
							warnings.push_back(jve);
						}
						result.put_member(message_field, msg);
						result.put_member(success_field, 0);
						result.put_member("Errors", warnings);
						result.put_member(data_field, object_definition);
						all_objects_good = false;
					}
					else {
						result.put_member(message_field, "Ok");
						result.put_member(success_field, 1);
						result.put_member(data_field, object_definition);
					}
				}
				else
				{
					std::string msg = std::format("'{0}' is not valid class_name", class_name);
					result.put_member(message_field, msg);
					result.put_member(success_field, 0);
					result.put_member(data_field, object_definition);
					all_objects_good = false;
				}
				result_list.push_back(result);
			}

			response.put_member(success_field, all_objects_good);
			response.put_member(message_field, "Objects processed");
			response.put_member(data_field, result_list);
			return response;
		}

		lockable header_lock;

		std::string get_pass_phrase()
		{
			return "This is a test pass phrase";
		}

		std::string get_iv()
		{
			return "This is a test iv";
		}

		json create_response(std::string _user_name, std::string _authorization, bool _success, std::string _message, json _data, double _seconds)
		{
			json_parser jp;

			json payload = jp.create_object();
			json token = jp.create_object();
			token.put_member(user_name_field, _user_name);
			token.put_member(authorization_field, _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string );

			if (_success) {
				payload.put_member(token_field, base64_token_string);
			}
			payload.put_member(success_field, _success);
			payload.put_member(message_field, _message);
			payload.put_member(data_field, _data);
			payload.put_member(seconds_field, _seconds);

			return payload;
		}

		json create_response(json _request, bool _success, std::string _message, json _data, double _seconds)
		{
			json_parser jp;
			json payload = jp.create_object();
			json token = jp.create_object();
			json src_token = _request[token_field];
			
			token.copy_member(user_name_field, src_token);
			token.copy_member(authorization_field, src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			if (_success) {
				payload.put_member(token_field, base64_token_string);
			}
			payload.put_member(success_field, _success);
			payload.put_member(message_field, _message);
			payload.put_member(data_field, _data);
			payload.put_member_double(seconds_field, _seconds);
			return payload;
		}

		json check_token(std::string _encoded_token, std::vector<std::string> _authorizations)
		{
			json_parser jp;
			json empty;
			std::string decoded_token = base64_decode(_encoded_token);
			json token = jp.parse_object(decoded_token);

			if (not token.object())
			{
				return empty;
			}

			if (not token.has_member(signature_field))
			{
				return empty;
			}

			std::string signature = token[signature_field];
			token.erase_member(signature_field);

			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());

			if (cipher_text != signature) {
				return empty;
			}

			token.put_member(signature_field, signature);

			date_time current = date_time::utc_now();
			date_time expiration = (date_time)token[token_expires_field];

			if (current > expiration)
			{
				return empty;
			}

			if (cipher_text != signature)
			{
				return empty;
			}

			if (not token.has_member(authorization_field))
			{
				return empty;
			}

			std::string authorization = token[authorization_field];
			std::string user = token[user_name_field];

			if (authorization == auth_system and user == default_user)
			{
				return token;
			}

			for (auto _authorization : _authorizations)
			{
				if (_authorization == authorization)
					return token;
			}

			return empty;
		}

		bool check_message(json& _message, std::vector<std::string> _authorizations, std::string& _user_name)
		{
			std::string token = _message[token_field];

			json result = check_token(token, _authorizations);
			bool is_ok = not result.empty();

			if (is_ok) {
				_user_name = result[user_name_field];
			}

			return is_ok;
		}

		json acquire_object(json _object_key)
		{
			json_parser jp;
			json obj;


			std::string class_name = _object_key[class_name_field];
			auto classd = load_class(class_name);

			if (classd) 
			{
				json_table class_data(this, { object_id_field });
				class_data.open(classd->table_location);
				obj = class_data.get(_object_key);
				return obj;
			}

			return obj;
		}

		json select_object(json _key, bool _children)
		{
			json_parser jp;
			json obj = jp.create_array();

			_key.set_natural_order();

			std::string class_name = _key[class_name_field];

			auto classd = load_class(class_name);

			if (classd)
			{
				relative_ptr_type rpt = classd->table_location;
				json_table class_data(this, { object_id_field });
				class_data.open(rpt);

				// Now, if there is an index set specified, let's go see if we can find one and use it 
				// rather than scanning the table

				if (classd->indexes.size() > 0)
				{
					std::shared_ptr<index_definition> matched_index;
					int max_matched_key_count = 0;

					// go through each index

					for (auto index_pair : classd->indexes)
					{
						// and here, let's go see if all the keys except the object_id, hit the filter.
						// if they do, we can use this index,
						// but, we want to use the index that matches the most keys
						//
						int matched_key_count = 0;

						for (auto ikey : index_pair.second->index_keys)
						{
							if (not _key.has_member(ikey))
							{
								matched_key_count = 0;
								break;
							}
							else
							{
								matched_key_count++;
							}
						}

						if (matched_key_count > max_matched_key_count)
						{
							matched_index = index_pair.second;
							max_matched_key_count = matched_key_count;
						}
					}

					// so now, if we have an index, we can use it.
					if (matched_index)
					{
						json_table index_table(this, matched_index->index_keys);
						index_table.open(matched_index->index_location);

						json object_key = jp.create_object();
						object_key.put_member(class_name_field, class_name);

						obj = index_table.select(_key, [&object_key, &class_data](int _idx, json& _item) -> json {
							object_key.copy_member(object_id_field, _item);
							json objfound = class_data.get(object_key);
							return objfound;
						});
					}
					else 
					{
						obj = class_data.select([&_key](int _index, json& _j)
							{
								json result;
								if (_key.compare(_j) == 0)
									result = _j;
								return result;

							});
					}
				}
				else 
				{
					obj = class_data.select([&_key](int _index, json& _j)
						{
							json result;
							if (_key.compare(_j) == 0)
								result = _j;
							return result;

						});
				}

				if (_children) {
					select_child_objects(classd, obj);
				}
			}

			return obj;
		}

		json select_single_object(json _key, bool _children)
		{
			json result = select_object(_key, _children);
			result = result.get_first_element();
			return result;
		}

		json get_user(std::string _user_name)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member(class_name_field, "sys_users");
			obj.put_member(user_name_field, _user_name);
			return select_single_object(obj, true);
		}

		json get_schema(std::string schema_name, std::string schema_version)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member(class_name_field, "sys_schemas");
			obj.put_member("schema_name", schema_name);
			obj.put_member("schema_version", schema_version);
			return select_single_object(obj, true);
		}

		json get_dataset(std::string dataset_name, std::string dataset_version)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member(class_name_field, "sys_datasets");
			obj.put_member("dataset_name", dataset_name);
			obj.put_member("dataset_version", dataset_version);
			return select_single_object(obj, true);
		}

		bool has_class_permission(
			std::string _user_name,
			std::string _class_name,
			std::string _permission)
		{

			bool granted = false;

			// extract the user key from the token and get the user object
			if (_user_name == default_user) 
			{
				return true;
			}

			json user =  get_user(_user_name);

			if (user.empty()) 
			{
				return false;
			}
			else 
			{
				granted = true;
			}
			
			return granted;
		}

		db_object_id_type get_next_object_id()
		{
			InterlockedIncrement64(&header.data.object_id);
			return header.data.object_id;
		}

		time_t last_commit;

	public:

		std::string default_user;
		std::string default_password;
		std::string default_email_address;
		std::string default_guest_team;
		time_span token_life;

		// constructing and opening a database

		corona_database(std::shared_ptr<file> _database_file) :
			file_block(_database_file)
		{
			time(&last_commit);
			std::vector<std::string> class_key_fields({ class_name_field });
			std::vector<std::string> indexes_key_fields({ class_name_field });
			classes = std::make_shared<json_table>(this, class_key_fields);
			indexes = std::make_shared<json_table>(this, indexes_key_fields);
			token_life = time_span(1, time_models::hours);	
		}

		virtual ~corona_database()
		{
			commit();
			wait();
		}

		void apply_config(json _config)
		{
			date_time start;
			start = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("apply_config", "start", start, __FILE__, __LINE__);

			if (_config.has_member("SendGrid"))
			{
				json send_grid = _config["SendGrid"];
				send_grid_api_key = send_grid["ApiKey"];
			}
			if (_config.has_member("Server"))
			{
				json server = _config["Server"];
				default_user = server[sys_user_name_field];
				default_password = server[sys_user_password_field];
				default_email_address = server[sys_user_email_field];
				default_guest_team = server[sys_default_team_field];
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_config", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		}


		virtual std::string get_random_code()
		{
			std::string s_confirmation_code = "";
			int confirmation_code_digits = 6;
			char confirmation_code[32] = {};

			int rc = 0;
			int lc = 0;
			int i = 0;
			while (i < confirmation_code_digits and i < sizeof(confirmation_code))
			{
				do
				{
					rc = rand() % 26 + 'A';
					confirmation_code[i] = rc;
				} while (rc == lc);
				lc = rc;
				i++;
			}
			confirmation_code[i] = 0;
			s_confirmation_code = confirmation_code;
			return s_confirmation_code;
		}

		std::map<std::string, std::shared_ptr<class_definition>> class_cache;

		virtual std::shared_ptr<class_definition> load_class(std::string _class_name)
		{
			std::shared_ptr<class_definition> cd;
			if (class_cache.contains(_class_name))
			{
				cd = class_cache[_class_name];
			}
			else 
			{
				json_parser jp;
				json key = jp.create_object();
				key.put_member(class_name_field, _class_name);
				json class_def = classes->get(key);
				if (class_def.object()) {
					cd = std::make_shared<class_definition>();
					cd->put_json(class_def);
					class_cache.insert_or_assign(_class_name, cd);
				}
			}
			return cd;
		}

		virtual json save_class(class_definition& _class_to_save)
		{
			std::shared_ptr<class_definition> cd;

			if (class_cache.contains(_class_to_save.class_name))
			{
				cd = class_cache[_class_to_save.class_name];
			}
			else
			{
				cd = std::make_shared<class_definition>(_class_to_save);
				class_cache.insert_or_assign(cd->class_name, cd);
			}
			json_parser jp;
			json class_def = jp.create_object();
			cd->get_json(class_def);
			classes->put(class_def);
			return class_def;
		}

		virtual json apply_schema(json _schema)
		{
			date_time start_schema = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("apply_schema", "Applying schema file", start_schema, __FILE__, __LINE__);

			bool new_database = true;

			std::map<std::string, bool> changed_classes;

			if (not _schema.has_member("schema_name"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema name");
			}

			if (not _schema.has_member("schema_version"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema version");
			}

			if (not _schema.has_member("schema_authors"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema author");
			}

			json_parser jp;

			json schema_key = jp.create_object();
			schema_key.copy_member("schema_name", _schema);
			schema_key.copy_member("schema_version", _schema);
			schema_key.put_member(class_name_field, "sys_schemas");
			schema_key.set_compare_order({ "schema_name", "schema_version" });

			json schema_test =  select_object(schema_key, false);

			if (schema_test.object()) 
			{
				new_database = false;
			}

			if (_schema.has_member("classes"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Classes", start_section, __FILE__, __LINE__);

				json class_array = _schema["classes"];
				if (class_array.array())
				{
					for (int i = 0; i < class_array.size(); i++)
					{
						date_time start_class = date_time::now();
						timer txc;

						json class_definition = class_array.get_element(i);
						system_monitoring_interface::global_mon->log_function_start("put class", class_definition[class_name_field], start_class, __FILE__, __LINE__);

						try {

							json put_class_request = create_system_request(class_definition);
							json class_result =  put_class(put_class_request);

							if (class_result.error()) 
							{
								system_monitoring_interface::global_mon->log_warning(class_result, __FILE__, __LINE__);
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::global_mon->log_exception(exc);
						}
						system_monitoring_interface::global_mon->log_function_stop("put class", class_definition[class_name_field], txc.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Classes", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else
			{
				system_monitoring_interface::global_mon->log_warning("classes not found in schema");
			}

			if (_schema.has_member("users"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Users", start_section, __FILE__, __LINE__);
				json user_array = _schema["users"];
				if (user_array.array())
				{
					for (int i = 0; i < user_array.size(); i++)
					{
						date_time start_user = date_time::now();
						timer txu;

						json user_definition = user_array.get_element(i);
						system_monitoring_interface::global_mon->log_function_start("put user", user_definition[user_name_field], start_user, __FILE__, __LINE__);
						json put_user_request = create_system_request(user_definition);
						create_user(put_user_request);
					    system_monitoring_interface::global_mon->log_function_stop("put user", user_definition[user_name_field], txu.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Users", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
	
			if (_schema.has_member("datasets"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Datasets", start_section, __FILE__, __LINE__);

				json script_array = _schema["datasets"];
				if (script_array.array())
				{
					for (int i = 0; i < script_array.size(); i++)
					{
						date_time start_dataset = date_time::now();
						timer txs;

						json script_definition = script_array.get_element(i);

						script_definition.put_member(class_name_field, "sys_datasets");
						std::string dataset_name = script_definition["dataset_name"];
						std::string dataset_version = script_definition["dataset_version"];

						system_monitoring_interface::global_mon->log_job_section_start("DataSet", dataset_name + " Start", start_dataset, __FILE__, __LINE__);

						bool script_run = (bool)script_definition["run_on_change"];
						json existing_script = get_dataset(dataset_name, dataset_version);
						bool run_script = false;
						if (existing_script.empty() or script_run)
							run_script = true;

						if (existing_script.empty())
						{
							// in corona, creating an object doesn't actually persist anything 
							// but a change in identifier.  It's a clean way of just getting the 
							// "new chumpy" item for ya.  Or you can just shove it in there.
							json put_script_request = create_system_request(script_definition);
							json created_object = put_script_request[data_field];
							json save_result = put_object(put_script_request);
							if (not save_result[success_field]) {
								system_monitoring_interface::global_mon->log_warning(save_result[message_field]);
								system_monitoring_interface::global_mon->log_json<json>(save_result);
								existing_script = save_result[data_field];
							}
							else
								system_monitoring_interface::global_mon->log_information(save_result[message_field]);
						}

						if (run_script and script_definition.has_member("import"))
						{
							json import_spec = script_definition["import"];
							std::vector<std::string> missing;

							if (not import_spec.has_members(missing, { "target_class", "type" })) {
								system_monitoring_interface::global_mon->log_warning("Import missing:");
								std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
									system_monitoring_interface::global_mon->log_warning(s);
									});
								system_monitoring_interface::global_mon->log_information("the source json is:");
								system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
								continue;
							}

							std::string target_class = import_spec["target_class"];
							std::string import_type = import_spec["type"];

							if (import_type == "csv") {

								if (not import_spec.has_members(missing, { "filename", "delimiter" })) {
									system_monitoring_interface::global_mon->log_warning("Import CSV missing:");
									std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
										system_monitoring_interface::global_mon->log_warning(s);
										});
									system_monitoring_interface::global_mon->log_information("the source json is:");
									system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
									continue;
								}

								std::string filename = import_spec["filename"];
								std::string delimiter = import_spec["delimiter"];
								if (filename.empty() or delimiter.empty()) {
									system_monitoring_interface::global_mon->log_warning("filename and delimiter can't be blank.");
								}

								json column_map = import_spec["column_map"];

								FILE* fp = nullptr;
								int error_code = fopen_s(&fp, filename.c_str(), "rS");

								if (fp) {
									// Buffer to store each line of the file.
									char line[8182];
									json datomatic = jp.create_array();

									// create template object
									json codata = jp.create_object();
									codata.put_member(class_name_field, target_class);
									json cor = create_system_request(codata);
									json new_object_response =  create_object(cor);

									if (new_object_response[success_field]) {
										json new_object_template = new_object_response[data_field];

										// Read each line from the file and store it in the 'line' buffer.
										int64_t total_row_count = 0;
										while (fgets(line, sizeof(line), fp)) {
											// Print each line to the standard output.
											json new_object = new_object_template.clone();
											new_object.erase_member(object_id_field);
											jp.parse_delimited_string(new_object, column_map, line, delimiter[0]);
											datomatic.push_back(new_object);
											if (datomatic.size() > 1000) {
												timer tx;
												json cor = create_system_request(datomatic);
												json put_result =  put_object(cor);
												if (put_result[success_field]) {
													double e = tx.get_elapsed_seconds();
													total_row_count += datomatic.size();
													std::string msg = std::format("import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
													system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
													datomatic = jp.create_array();
												}
												else {
													std::string msg = std::format("Error saving object {0}", (std::string)put_result[message_field]);
													system_monitoring_interface::global_mon->log_warning(msg);
													system_monitoring_interface::global_mon->log_information("Return result");
													system_monitoring_interface::global_mon->log_json(put_result);
													system_monitoring_interface::global_mon->log_information("Object that failed.");
													system_monitoring_interface::global_mon->log_json(new_object);
													break;
												}
											}
										}

										if (datomatic.size() > 0) {
											timer tx;
											json cor = create_system_request(datomatic);
											json put_result = put_object(cor);
											if (put_result[success_field]) {
												double e = tx.get_elapsed_seconds();
												total_row_count += datomatic.size();
												std::string msg = std::format("import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
												system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
												datomatic = jp.create_array();
											}
											else {
												std::string msg = std::format("Error saving object {0}", (std::string)put_result[message_field]);
												system_monitoring_interface::global_mon->log_warning(msg);
												system_monitoring_interface::global_mon->log_information("Return result");
												system_monitoring_interface::global_mon->log_json(put_result);
												system_monitoring_interface::global_mon->log_information("Object that failed.");
												break;
											}
										}

									}

									// Close the file stream once all lines have been read.
									fclose(fp);
								}
								else {
									char error_buffer[256] = {};
									strerror_s(
										error_buffer,
										std::size(error_buffer),
										error_code
									);
									std::string msg = std::format("could not open file {0}:{1}", filename, error_buffer);
									system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
									char directory_name[MAX_PATH] = {};
									char *result = _getcwd(directory_name, std::size(directory_name));
									if (result) {
										msg = std::format("cwd is {0}", result);
										system_monitoring_interface::global_mon->log_information(msg, __FILE__, __LINE__);
									}
								}
							}
						}

						if (run_script and script_definition.has_member("objects")) {
							json object_list = script_definition["objects"];
							if (object_list.array()) {
								for (int j = 0; j < object_list.size(); j++) {
									json object_definition = object_list.get_element(i);
									json put_object_request = create_system_request(object_definition);
									// in corona, creating an object doesn't actually persist anything 
									// but a change in identifier.  It's a clean way of just getting the 
									// "new chumpy" item for ya.  
									json create_result =  create_object(put_object_request);
									if (create_result[success_field]) {
										json created_object = put_object_request[data_field];
										json save_result =  put_object(put_object_request);
										if (not save_result[success_field]) {
											system_monitoring_interface::global_mon->log_warning(save_result[message_field]);
											system_monitoring_interface::global_mon->log_json<json>(save_result);
										}
										else
											system_monitoring_interface::global_mon->log_information(save_result[message_field]);
										object_definition.copy_member(success_field, create_result);
										object_definition.copy_member(message_field, create_result);
									}
									else 
									{
										object_definition.copy_member(success_field, create_result);
										object_definition.copy_member(message_field, create_result);
										system_monitoring_interface::global_mon->log_warning(create_result[message_field], __FILE__, __LINE__);
										system_monitoring_interface::global_mon->log_json<json>(create_result);
									}
								}
							}

							date_time completed_date = date_time::now();
							script_definition.put_member("Completed", completed_date);
							json put_script_request = create_system_request(script_definition);
							json save_script_result =  put_object(put_script_request);
							if (not save_script_result[success_field]) {
								system_monitoring_interface::global_mon->log_warning(save_script_result[message_field]);
								system_monitoring_interface::global_mon->log_json<json>(save_script_result);
							}
							else
								system_monitoring_interface::global_mon->log_information(save_script_result[message_field]);
						}

						system_monitoring_interface::global_mon->log_job_section_stop("DataSet", dataset_name + " Finished", txs.get_elapsed_seconds(), __FILE__, __LINE__);
					}
					system_monitoring_interface::global_mon->log_job_section_stop("DataSets", "", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}

			_schema.put_member(class_name_field, "sys_schemas");

			json put_schema_request = create_system_request(_schema);
			// in corona, creating an object doesn't actually persist anything 
			// but a change in identifier.  It's a clean way of just getting the 
			// "new chumpy" item for ya.  
			json create_schema_result =  create_object(put_schema_request);
			if (create_schema_result[success_field]) {
				json created_object = put_schema_request[data_field];
				json save_schema_result =  put_object(put_schema_request);
				if (not save_schema_result[success_field]) {
					system_monitoring_interface::global_mon->log_warning(save_schema_result[message_field]);
					system_monitoring_interface::global_mon->log_json<json>(save_schema_result);
				}
				else
					system_monitoring_interface::global_mon->log_information(save_schema_result[message_field]);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_warning(create_schema_result[message_field], __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(create_schema_result);
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_schema", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			json temp = R"({ "success" : true, "message" : "Everything Ok, situation normal."})"_jobject;

			return temp;
		}

		virtual relative_ptr_type open_database(relative_ptr_type _header_location)
		{
			timer method_timer;
			date_time start_schema = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("open_database", "Open database", start_schema, __FILE__, __LINE__);

			scope_lock lock_one(header_rw_lock);

			relative_ptr_type header_location =  header.read(this, _header_location);

			relative_ptr_type result = classes->open(header.data.classes_location);
			result = indexes->open(header.data.indexes_location);

			system_monitoring_interface::global_mon->log_job_stop("open_database", "Open database", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return header_location;
		}

		virtual json create_user(json create_user_request)
		{
			timer method_timer;
			json_parser jp;
			json response;


			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_user", "start", start_time, __FILE__, __LINE__);

			json data = create_user_request[data_field];

			std::string user_name = data[user_name_field];
			std::string user_password1 = data["password1"];
			std::string user_password2 = data["password2"];
			std::string user_class = "sys_users";

			if (user_password1 != user_password2) 
			{
				system_monitoring_interface::global_mon->log_function_stop("create_user", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				response = create_response(create_user_request, false, "Passwords don't match", data, method_timer.get_elapsed_seconds());
				return response;
			}

			bool user_exists = true;
			int attempt_count = 0;

			do 
			{
				scope_lock lock_one(objects_rw_lock);

				json existing_user_link = get_user(user_name);

				if (existing_user_link.object()) 
				{
					attempt_count++;
					char buff[128];
					buff[0] = ('0' + rand() % 10);
					buff[1] = 0;
					user_name = user_name + buff;
				}
				else 
				{
					user_exists = false;
				}

			} while (user_exists);

			std::string hashed_pw = crypter.hash(user_password1);

			json create_user_params = jp.create_object();
			create_user_params.put_member(class_name_field, user_class);
			create_user_params.put_member(user_name_field, user_name);
			create_user_params.put_member(user_password_field, hashed_pw);
			create_user_params.copy_member(user_first_name_field, data);
			create_user_params.copy_member(user_last_name_field, data);
			create_user_params.copy_member(user_email_field, data);
			create_user_params.copy_member(user_mobile_field, data);
			create_user_params.copy_member(user_street1_field, data);
			create_user_params.copy_member(user_street2_field, data);
			create_user_params.copy_member(user_state_field, data);
			create_user_params.copy_member(user_zip_field, data);

			json create_object_request = create_request(user_name, auth_general, create_user_params);
			json user_result =  put_object(create_object_request);
			if (user_result[success_field]) {
				json new_user_wrapper = user_result[data_field];
				response = create_response(user_name, auth_general, true, "User created", data, method_timer.get_elapsed_seconds());
			}
			else
			{
				response = create_response(create_user_request, false, "User not created", data, method_timer.get_elapsed_seconds());
			}

			commit_check();

			system_monitoring_interface::global_mon->log_function_stop("create_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return response;
		}

		// this starts a login attempt
		virtual json login_user(json _login_request)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("login_user", "start", start_time, __FILE__, __LINE__);

			json data = _login_request;
			std::string user_name = data[user_name_field];
			std::string user_password = data["password"];
			std::string hashed_user_password;

			std::string hashed_pw = crypter.hash(user_password);

			json user = get_user(user_name);
			std::string pw = user["password"];

			if (pw == hashed_pw)
			{
				if (user_name == default_user and default_user.size() > 0) {
					response = create_response(user_name, auth_system, true, "Ok", data, method_timer.get_elapsed_seconds());
				}
				else {
					response = create_response(user_name, auth_general, true, "Ok", data, method_timer.get_elapsed_seconds());
				}
			}
			else
			{
				response = create_response(_login_request, false, "Failed", jp.create_object(), method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("login_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			commit_check();

			return response;
		}

		virtual json edit_object(json _edit_object_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;
			std::string user_name;


			bool include_children = (bool)_edit_object_request["include_children"];

			if (not check_message(_edit_object_request, { auth_general }, user_name))
			{
				result = create_response(_edit_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			system_monitoring_interface::global_mon->log_function_start("edit_object", "start", start_time, __FILE__, __LINE__);

			json token = _edit_object_request[token_field];
			json key = _edit_object_request.extract({ class_name_field, object_id_field });
			int64_t object_id = (int64_t)key[object_id_field];
			std::string class_name = key[class_name_field];

			auto edit_class = load_class(class_name);
			if (edit_class) 
			{
				edit_class->init_validation(this);
				json jedit_object = select_single_object(key, include_children);
				if (not jedit_object.empty() and include_children) 
				{
					std::string token = _edit_object_request[token_field];
					edit_class->run_queries(this, token, jedit_object);
				}
				json jedit_class = jp.create_object();
				edit_class->put_json(jedit_class);
				result = jp.create_object();
				result.put_member("class", jedit_class);
				result.put_member("object", jedit_object);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return create_response(_edit_object_request, false, "Invalid class.", jp.create_object(), method_timer.get_elapsed_seconds());
			}

			return create_response(_edit_object_request, true, "Ok", result, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("edit_object", "success", tx.get_elapsed_seconds(), __FILE__, __LINE__);
		}

		virtual json get_classes(json get_classes_request)
		{
			timer method_timer;
			json_parser jp;

			json result;
			json result_list;
			
			scope_lock lock_one(classes_rw_lock);

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_classes", "start", start_time, __FILE__, __LINE__);

			std::string user_name;

			if (not check_message(get_classes_request, { auth_general }, user_name))
			{
				result = create_response(get_classes_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return result;
			}

			result_list =  classes->select([this, user_name](int _index, json& _item) {
				bool has_permission = has_class_permission(user_name, _item[class_name_field], "Describe");
				json_parser jp;

				if (has_permission) 
				{
					return _item;
				}
				else 
				{
					json empty = jp.create_object("Skip", "this");
					return empty;
				}
			});

			system_monitoring_interface::global_mon->log_function_stop("get_classes", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			result = create_response(get_classes_request, true, "Ok", result_list, method_timer.get_elapsed_seconds());

			return result;
		}

		virtual json get_class(json get_class_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_class", "start", start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;

			if (not get_class_request.has_members(missing_elements, { token_field, class_name_field })) {
				std::string error_message;
				error_message = "get_class missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				json response = create_response(get_class_request, false, error_message, jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string user_name;

			if (not check_message(get_class_request, { auth_general }, user_name))
			{
				result = create_response(get_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			std::string class_name = get_class_request[class_name_field];

			bool can_get_class =  has_class_permission( user_name, class_name, "Describe");

			json key = jp.create_object(class_name_field, class_name);
			key.set_natural_order();

			{
				scope_lock lock_one(classes_rw_lock);
				result =  classes->get(key);
			}

			result = create_response(get_class_request, true, "Ok", result, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("get_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		virtual json put_class(json put_class_request)
		{
			timer method_timer;
			json result;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("put_class", "start", start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;
			if (not put_class_request.has_members(missing_elements, { token_field })) {
				std::string error_message;
				error_message = "create_class missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				json response = create_response(put_class_request, false, error_message, jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string user_name;

			if (not check_message(put_class_request, { auth_general }, user_name))
			{
				result = create_response(put_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			json token = put_class_request[token_field];
			json jclass_definition = put_class_request[data_field];
			std::string class_name = jclass_definition[class_name_field];

			bool can_put_class =  has_class_permission(
				user_name,
				class_name,
				"Alter");

			if (not can_put_class) {
				result = create_response(put_class_request, false, "Denied", jclass_definition, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			auto existing_class = load_class(class_name);

			class_definition class_def;
			class_def.put_json(jclass_definition);

			if (trace_check_class) {
				system_monitoring_interface::global_mon->log_activity("check_class", start_time);
			}

			result = create_response(put_class_request, true, "Ok", jclass_definition, method_timer.get_elapsed_seconds());

			if (class_def.class_name.empty())
			{
				result = create_response(put_class_request, false, "Class must have a name", jclass_definition, method_timer.get_elapsed_seconds());
			}

			if (class_def.class_description.empty())
			{
				result = create_response(put_class_request, false, "Class must have a description", jclass_definition, method_timer.get_elapsed_seconds());
			}

			class_name = class_def.class_name;

			// here we are going to grab the ancestor chain for this class.

			if (not class_def.base_class_name.empty())
			{
				std::string base_class_name = class_def.base_class_name;

				auto base_class = load_class(base_class_name);

				if (not base_class)
				{
					result = create_response(put_class_request, false, "Base class not found", jclass_definition, method_timer.get_elapsed_seconds());
				}

				class_def.ancestors = base_class->ancestors;
				class_def.ancestors.insert_or_assign(base_class_name, true);
				base_class->descendants.insert_or_assign(class_name, true);
				save_class(*base_class.get());
				if (existing_class) {
					class_def.descendants = existing_class->descendants;
				}
				else {
					class_def.descendants.insert_or_assign(class_name, true);
				}

				for (auto temp_field : base_class->fields)
				{
					class_def.fields.insert_or_assign(temp_field.first, temp_field.second);
				}

				for (auto descendant : class_def.descendants)
				{
					auto desc_class = load_class(descendant.first);
					if (desc_class) {
						desc_class->ancestors.insert_or_assign(class_name, true);
						save_class(*desc_class);
					}
				}
			}

			if (existing_class) {
				class_def.table_location = existing_class->table_location;
			}

			json_table class_data(this, { object_id_field });
			relative_ptr_type rpt;

			if (class_def.table_location == 0) {
				rpt = class_data.create();
				class_def.table_location = rpt;
			}
			else
			{
				rpt = class_def.table_location;
				class_data.open(rpt);
			}

			// The object id always part of the index key
			// and I check it here, before looping through the existing indexes.

			for (auto& new_index : class_def.indexes)
			{
				auto first = std::find_if(new_index.second->index_keys.begin(), new_index.second->index_keys.begin(), [](std::string& _key) {
							if (_key == object_id_field) {
								return true;
							}
							return false;
						});
				if (first == new_index.second->index_keys.end()) {
					new_index.second->index_keys.push_back(object_id_field);
				}
			}

			// loop through existing indexes,
			// dropping old ones that don't match
			if (existing_class) {
				for (auto old_index : existing_class->indexes)
				{
					auto index = old_index.second;
					auto existing = class_def.indexes.find(index->index_name);
					if (existing == class_def.indexes.end() or
						existing->second->index_key_string() != index->index_key_string()) {
						json_table index_table(this, index->index_keys);
						if (index->index_location)
						{
							index_table.open(index->index_location);
							index_table.clear();
							index->index_location = 0;
						}
					}
				}
			}

			// reindex tables list

			std::vector<std::shared_ptr<index_definition>> indexes_to_create;

			// and once again through the indexes

			for (auto& new_index : class_def.indexes)
			{
				// don't trust the inbounnd table locations
				new_index.second->index_location = 0;

				if (existing_class) 
				{
					auto existing = existing_class->indexes.find(new_index.first);

					if (existing != existing_class->indexes.end())
					{
						new_index.second->index_location = existing->second->index_location;
					}
				}

				if (new_index.second->index_location == 0) 
				{
					json_table table(this, new_index.second->index_keys);
					new_index.second->index_location = table.create();
					indexes_to_create.push_back(new_index.second);
				}
			}

			// and populate the new indexes with any data that might exist
			for (auto idc : indexes_to_create) {
				json_table table(this, idc->index_keys);
				table.open(idc->index_location);
				class_data.for_each([idc, this, &table](int _idx, json& _item) -> relative_ptr_type {
					json index_item = _item.extract(idc->index_keys);
					table.put(index_item);
					return 1;
				});
			}

			json saved_class = save_class(class_def);	
			result = create_response(put_class_request, true, "Ok", saved_class, method_timer.get_elapsed_seconds());

			commit_check();
			system_monitoring_interface::global_mon->log_function_stop("put_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		virtual std::shared_ptr<class_definition_interface> get_class_interface(std::string _class_name)
		{
			auto cd = load_class(_class_name);
			return cd;
		}

		private:

		json query_class(std::string _user_name, json query_details, json update_json)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("update", "start", start_time, __FILE__, __LINE__);

			bool include_children = (bool)query_details["include_children"];

			json base_class_name = query_details[class_name_field];
			if (base_class_name.empty()) {
				response = create_response(_user_name, auth_general, false, "class_name not specified", query_details, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			bool class_granted =  has_class_permission(_user_name, base_class_name, "Get");
			if (not class_granted)
			{
				response = create_response(_user_name, auth_general, false, "denied", query_details, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto class_def = load_class(base_class_name);

			json object_list = jp.create_array();

			json filter = query_details["filter"];
			if (filter.empty()) {
				filter = jp.create_object();
			}

			for (auto class_pair : class_def->descendants)
			{
				json class_key;
				class_key = filter.clone();
				class_key.put_member(class_name_field, class_pair.first);
				json objects = select_object(class_key, include_children);
				if (objects.array()) {
					for (auto obj : objects) {
						object_list.push_back(obj);
					}
				}
			}

			response = create_response(_user_name, auth_general, true, "completed", object_list, method_timer.get_elapsed_seconds());

			system_monitoring_interface::global_mon->log_function_stop("update", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		public:

		virtual json query(json query_request)
		{
			timer tx;

			json_parser jp;
			json jx; // not creating an object, leaving it empty.  should work with empty objects
			// or with an object that has no members.
			json response;

			std::string user_name;

			if (not check_message(query_request, { auth_general }, user_name))
			{
				response = create_response(query_request, false, "Denied", jp.create_object(), tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("query", "denied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			query_context context;

			context.froms_preloaded = true;

			json from_classes = query_request["from"];
			if (from_classes.array())
			{
				json stages = query_request["stages"];

				if (not stages.array() or stages.size() == 0)
				{
					response = create_response(query_request, false, "query has no stages", jp.create_object(), tx.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("query", "query has no stages", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return response;
				}

				for (auto from_class : from_classes)
				{
					std::string class_name = from_class[class_name_field];
					std::string from_name = from_class["name"];
					if (from_name.empty())
					{
						response = create_response(query_request, false, "from with no name", jp.create_object(), tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from with no name", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
					if (class_name.empty())
					{
						response = create_response(query_request, false, "from with no class", jp.create_object(), tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from with no class", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
					auto cd = load_class(class_name);
					if (not cd) {
						response = create_response(query_request, false, "from class not found", jp.create_object(), tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from class not found", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
				}

				// these will all run in parallel once I get this to work
				for (auto from_class : from_classes)
				{
					std::string class_name = from_class[class_name_field];
					std::string from_name = from_class["name"];
					json data = from_class[data_field];
					json objects;

					// allow the query to have inline objects
					if (data.object())
					{
						objects = jp.create_array();
						objects.push_back(data);
					}
					else if (data.array())
					{
						objects = data;
					}
					else if (data.empty()) {
						json query_class_response = query_class(user_name, from_class, jx);
						objects = query_class_response[data_field];
						if (objects.array()) {
							bool include_children = (bool)from_class["include_children"];
							if (include_children)
							{
								auto edit_class = load_class(class_name);
								std::string token = query_request[token_field];
								for (auto obj : objects) {
									edit_class->run_queries(this, token, obj);
								}
							}
						}
					}
					else {
						response = create_response(query_request, true, "query data is not an object or an array", jp.create_object(), tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "bad query data", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
					context.set_data_source(from_name, objects);
				}

				// so now, we've loaded up our context, we can extract the stages
				context.put_json(query_request);

				// and, we can run the thing.
				json query_results = context.run();

				response = create_response(query_request, true, "query ran", query_results, tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("query", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else 
			{
				response = create_response(query_request, true, "query has no froms", jp.create_object(), tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("query", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return response;
		}

		virtual json create_object(json create_object_request)
		{
			timer method_timer;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_object", "start", start_time, __FILE__, __LINE__);

			json token = create_object_request[token_field];
			json data = create_object_request[data_field];
			std::string class_name = data[class_name_field];
			json response;

			std::string user_name;

			if (not check_message(create_object_request, { auth_general }, user_name))
			{
				response = create_response(create_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			bool permission =  has_class_permission(user_name, class_name, "Create");
			if (not permission) {
				json result = create_response(create_object_request, false, "Denied", data, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			auto class_def = load_class(class_name);

			if (not class_def->empty()) {

				json new_object = jp.create_object();
				new_object.put_member(class_name_field, class_name);

				for (auto& member : class_def->fields)
				{
					if (member.first == class_name_field)
						continue;

					auto &field = member.second;
					std::string &field_type = field->field_type;

					if (field_type == "object") 
					{
						new_object.put_member_object(member.first);
					}
					else if (field_type == "array" or field_type == "query")
					{
						new_object.put_member_array(member.first);
					}
					else if (field_type == "number") 
					{
						new_object.put_member(member.first, 0.0);
					}
					else if (field_type == "string") 
					{
						new_object.put_member(member.first, "");
					}
					else if (field_type == "int64") 
					{
						new_object.put_member_i64(member.first, 0);
					}
					else if (field_type == "datetime")
					{
						date_time dt;
						new_object.put_member(member.first, dt);
					}
					else if (field_type == "function")
					{
						auto key = std::make_tuple(class_name, member.first);
						if (functions.contains(key)) {
							new_object.put_member_function(member.first, functions[key]);
						}
						else 
						{
							std::string err_message = std::format("function {0} {1} not defined", class_name, member.first);
							new_object.put_member(member.first, err_message);
						}
					}
				}
				response = create_response(create_object_request, true, "Object created", new_object, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else {
				std::string msg = std::format("create_object failed because the class '{0}' was never found.", class_name);
				system_monitoring_interface::global_mon->log_warning(msg);
				response = create_response(create_object_request, false, "Couldn't find class", create_object_request, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			commit_check();

			return response;
			
		}

		virtual json put_object(json put_object_request)
		{
			timer method_timer;
			json_parser jp;

			json token;
			json object_definition;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("put_object", "start", start_time, __FILE__, __LINE__);

			object_definition = put_object_request[data_field];
			std::string user_name;

			if (not check_message(put_object_request, { auth_general }, user_name))
			{
				result = create_response(put_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			result =  check_object(user_name, object_definition);

			if (result[success_field])
			{
				json data = result[data_field];

				json item_array;
				if (data.array()) {
					item_array = jp.create_array();
					for (auto obj : data) {
						if (obj[success_field]) {
							json t = obj[data_field];
							item_array.push_back(t);
						}
					}
				}
				else 
				{
					item_array = jp.create_array();
					if (data[success_field]) {
						item_array.push_back(data[data_field]);
					}
				}

				json grouped_by_class_name = item_array.group([](json& _item) -> std::string {
					return _item[class_name_field];
					});

				auto classes_and_data = grouped_by_class_name.get_members();

				json child_objects = jp.create_array();

				for (auto class_pair : classes_and_data)
				{
					auto cd = load_class(class_pair.first);

					extract_child_objects(cd, child_objects, class_pair.second);

					// now that we have our class, we can go ahead and open the storage for it
					relative_ptr_type rpt = cd->table_location;
					json_table class_data(this, { object_id_field });
					class_data.open(rpt);

					class_data.put_array(class_pair.second);

					// update the indexes
					for (auto class_index : cd->indexes) {
						relative_ptr_type location = class_index.second->index_location;
						if (location > 0) {
							json indexed_objects = class_pair.second.map([&class_index](std::string _member, int _index, json& _item) -> json {
								return _item.extract(class_index.second->index_keys);
								});
							json_table index_to_update(this, class_index.second->index_keys);
							index_to_update.open(location);
							index_to_update.put_array(indexed_objects);
						}
					}
				}

				header.write(this);

				if (child_objects.size() == 0) {
					commit_check();
				}
				else 
				{
					put_object_request.put_member(data_field, child_objects);
					put_object(put_object_request);
				}

				result = create_response(put_object_request, true, "Object(s) created", data, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(put_object_request, false, result[message_field], result["Warnings"], method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("put_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		virtual json get_object(json get_object_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_object", "start", start_time, __FILE__, __LINE__);

			std::string user_name;
			json object_key = get_object_request.extract({ class_name_field, object_id_field });

			if (not check_message(get_object_request, { auth_general }, user_name))
			{
				result = create_response(get_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			std::string class_name = object_key[class_name_field];

			bool permission =  has_class_permission(user_name, class_name, "Get");
			if (not permission) {
				json result = create_response(get_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			bool include_children = (bool)get_object_request["include_children"];
			
			json obj =  select_single_object(object_key, include_children);
			if (obj.object()) 
			{
				if (include_children)
				{
					auto edit_class = load_class(class_name);
					std::string token = get_object_request[token_field];
					edit_class->run_queries(this, token, obj);
				}

				result = create_response(get_object_request, true, "Ok", obj, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(get_object_request, false, "Not found", object_key, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("get_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}


		virtual json delete_object(json delete_object_request)
		{
			timer method_timer;
			json response;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("delete_object", "start", start_time, __FILE__, __LINE__);

			std::string user_name;
			json object_key = delete_object_request.extract({ class_name_field, object_id_field });

			if (not check_message(delete_object_request, { auth_general }, user_name))
			{
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string class_name = object_key[class_name_field];

			bool permission = has_class_permission(user_name, class_name, "Delete");
			if (not permission) {
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto cd = load_class(class_name);

			if (cd->table_location) {
				json empty;
				relative_ptr_type rpt = cd->table_location;
				json_table class_data(this, { object_id_field });
				class_data.open(rpt);
				class_data.erase(object_key);
				response = create_response(delete_object_request, true, "Ok", object_key, method_timer.get_elapsed_seconds());
			}
			else 
			{
				response = create_response(delete_object_request, false, "Not found", object_key, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("delete_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			commit_check();

			return response;
		}

		virtual json copy_object(json copy_request)
		{
			timer method_timer;
			json_parser jp;

			json response;

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_function_start("copy_object", "start", start_time, __FILE__, __LINE__);

			std::string user_name;

			if (not check_message(copy_request, { auth_general }, user_name))
			{
				response = create_response(copy_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json source_spec = copy_request["from"];
			std::string source_class = source_spec[class_name_field];			
			std::string source_path = source_spec["path"];
			json source_key = source_spec.extract({ class_name_field, object_id_field });

			json dest_spec = copy_request["to"];
			std::string dest_class = dest_spec[class_name_field];
			std::string dest_path = dest_spec["path"];
			json dest_key = dest_spec.extract({ class_name_field, object_id_field });

			json transform_spec = copy_request["transform"];
			std::string transform_class = transform_spec[class_name_field];

			bool permission = has_class_permission(user_name, source_class, "Get");
			if (not permission) {
				response = create_response(copy_request, false, "Denied", source_spec, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			permission = has_class_permission(user_name, dest_class, "Put");
			if (not permission) {
				response = create_response(copy_request, false, "Denied", dest_spec, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			if (not transform_class.empty()) {
				permission = has_class_permission(user_name, transform_class, "Put");
				if (not permission) {
					response = create_response(copy_request, false, "Denied", transform_spec, method_timer.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return response;
				}
			}

			// load the complete object
			json object_source = select_object(source_key, true);

			json object_to_copy_result;
			json source_object;

			if (not object_source.empty())
			{
				// if the source object is not empty, then, walk the source path to extract the child object we want
				object_to_copy_result = object_source.query(source_path);
				source_object = object_to_copy_result["value"];

				// if that worked, then let's go for the copy
				if (source_object.object()) 
				{
					json new_object;
					// we transform the object into a new class while copying, if there is a 
					// transform specification

					if (not transform_class.empty()) {
						// so we create a new object of the class
						json transform_key = transform_spec.extract({ class_name_field });
						json cor = create_request(user_name, auth_general, transform_spec);
						json new_object_result = create_object(cor);

						// then we can copy the fields over
						if (new_object_result[success_field]) {
							new_object = new_object_result[data_field];
							auto new_object_fields = new_object.get_members();
							for (auto nof : new_object_fields) {
								if (source_object.has_member(nof.first)) {
									new_object.copy_member(nof.first, source_object);
								}
							}
						}
					}
					else 
					{
						new_object = source_object.clone();
						int64_t new_object_id = get_next_object_id();
						new_object.put_member_i64(object_id_field, new_object_id);
					}

					//
					json object_dest = select_single_object(dest_key, true);

					// now we are going to put this object in our destination
					json object_dest_result = object_dest.query(dest_path);

					if (object_dest_result.object()) {
						json update_obj;
						json target = object_dest_result["target"];
						std::string name = object_dest_result["name"];
						update_obj = object_dest_result["object"];
						if (target.array())
						{
							target.push_back(new_object);
						}
						else if (target.object())
						{
							target.put_member(name, new_object);
						}
						json por = create_request(user_name, auth_general, update_obj);
						response = put_object(por);
					}
					else {
						response = create_response(copy_request, false, "could not find dest object", object_dest, method_timer.get_elapsed_seconds());;
					}
				}
				else {
					response = create_response(copy_request, false, "could not find source object", object_source, method_timer.get_elapsed_seconds());;
				}
			}
			else {
				response = create_response(copy_request, false, "source object not specified", object_source, method_timer.get_elapsed_seconds());;;
			}

			system_monitoring_interface::global_mon->log_function_stop("copy_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		private:

		json create_system_request(json _data)
		{
			json_parser jp;

			json payload;

			payload = jp.create_object();

			json token = jp.create_object();
			token.put_member(user_name_field, default_user);
			token.put_member(authorization_field, auth_system);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			payload.put_member(token_field, base64_token_string);

			payload.put_member(data_field, _data);

			return payload;
		}


		json create_request(std::string _user_name, std::string _authorization, json _data)
		{
			json_parser jp;

			json payload;

			if (_data.object())
			{
				payload = jp.create_object();
			}

			json token = jp.create_object();
			token.put_member(user_name_field, _user_name);
			token.put_member(authorization_field, _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			payload.put_member(token_field, base64_token_string);
			payload.put_member(data_field, _data);

			return payload;
		}

	};

	bool test_database_engine(json& _proof, std::shared_ptr<application> _app)
	{
		bool success = true;
		std::shared_ptr<file> dtest = std::make_shared<file>();

		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("table proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_database.cdb");

		json_parser jp;
		json proof_assertion = jp.create_object();

		proof_assertion.put_member("test_name", "database");

		json dependencies = jp.parse_object(R"( 
{ 
	"create" : [  "table.get", "table.put", "table.create", "table.select" ],
	"login" : [  "table.get", "table.put",  "table.select" ],
    "class" : [ "table.get", "table.put", "table.create", "table.select" ],
	"class_index" : [ "table.get", "table.put", "table.create", "table.select" ],
	"create" : [ "table.get", "table.put", "table.select" ],
	"put" : [ "table.get", "table.put", "table.select" ],
	"put_index" : [ "table.get", "table.put", "table.select" ],
	"get" : [ "table.get" ],
	"get_index" : [ "table.get", "table.select" ],
	"delete" : [ "table.erase" ],
	"team_class" : [ "table.get", "table.put", "table.create", "table.select" ],
	"team_get" : [ "table.get", "table.select" ],
	"team_put" : [ "table.get", "table.put", "table.select" ],
	"team_delete" : [ "table.get", "table.put", "table.create", "table.select" ]
}
)");

		bool create_success;
		bool login_success;
		bool class_success;
		bool put_success;
		bool get_success;
		bool class_index_success;
		bool put_index_success;
		bool get_index_success;
		bool delete_success;
		bool team_class_success;
		bool team_get_success;
		bool team_put_success;
		bool team_delete_success;

		date_time start_schema = date_time::now();
		system_monitoring_interface::global_mon->log_job_start("test_database_engine", "start", start_schema, __FILE__, __LINE__);

		corona_database db(dtest);

		proof_assertion.put_member("dependencies", dependencies);
		json db_config = jp.create_object();
		json server_config = jp.create_object();
		server_config.put_member(sys_user_name_field, "todd");
		server_config.put_member(sys_user_password_field, "randomite");
		server_config.put_member(sys_user_email_field, "todd.bandrowsky@gmail.com");
		server_config.put_member(sys_default_team_field, "GuestTeam");
		db_config.put_member("Server", server_config);

		db.apply_config(db_config);

		relative_ptr_type database_location = db.create_database();

		login_success = true;

		json login_positive_request = R"(
{
	"user_name" : "todd",
	"password" : "randomite"
}
)"_jobject;

		json login_negative_request = R"(
{
	"user_name" : "todd",
	"password" : "reachio"
}
)"_jobject;

		json login_result;

		login_result = db.login_user(login_negative_request);

		if (login_result[success_field]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("able to login with bad account", __FILE__, __LINE__);
		}

		login_result = db.login_user(login_positive_request);

		if (not login_result[success_field]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("can't with good account", __FILE__, __LINE__);
		}


		system_monitoring_interface::global_mon->log_job_stop("test_database_engine", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		return success;
	}

	bool collection_tests()
	{

		try {

			application app;

			std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
			box->init(1 << 21);

			jschema schema;
			relative_ptr_type schema_id;

			schema = jschema::create_schema(box, 20, true, schema_id);

			relative_ptr_type quantity_field_id = null_row;
			relative_ptr_type last_name_field_id = null_row;
			relative_ptr_type first_name_field_id = null_row;
			relative_ptr_type birthday_field_id = null_row;
			relative_ptr_type count_field_id = null_row;
			relative_ptr_type title_field_id = null_row;
			relative_ptr_type institution_field_id = null_row;

			schema.bind_field("quantity", quantity_field_id);
			schema.bind_field("lastName", last_name_field_id);
			schema.bind_field("firstName", first_name_field_id);
			schema.bind_field("birthday", birthday_field_id);
			schema.bind_field("count", count_field_id);
			schema.bind_field("title", title_field_id);
			schema.bind_field("institutionName", institution_field_id);

			jcollection_ref ref;
			ref.data = std::make_shared<dynamic_box>(1 << 20);
			ref.max_objects = 50;
			ref.collection_size_bytes = 1 << 19;

			init_collection_id(ref.collection_id);

			jcollection people = schema.create_collection(&ref);

			put_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, count_field_id, quantity_field_id };
			relative_ptr_type person_class_id = schema.put_class(person);

			if (person_class_id == null_row)
			{
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			int birthdaystart = 1941;
			int countstart = 12;
			double quantitystart = 10.22;
			int increment = 5;

			relative_ptr_type people_object_id;

			auto sl = people.create_object(person_class_id, people_object_id);
			auto last_name = sl.get_string(0);
			auto first_name = sl.get_string(1);
			auto birthday = sl.get_time(2);
			auto count = sl.get_int64(3);
			auto qty = sl.get_double(4);
			last_name = "last 1";
			first_name = "first 1";
			birthday = birthdaystart + increment * 0;
			count = countstart + increment * 0;
			qty = quantitystart + increment * 0;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 2";
			first_name = "first 2";
			birthday = birthdaystart + increment * 1;
			count = countstart + increment * 1;
			qty = quantitystart + increment * 1;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 3";
			first_name = "first 3";
			birthday = birthdaystart + increment * 2;
			count = countstart + increment * 2;
			qty = quantitystart + increment * 2;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 4";
			first_name = "first 4";
			birthday = birthdaystart + increment * 3;
			count = countstart + increment * 3;
			qty = quantitystart + increment * 3;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
			first_name = "first 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
			birthday = birthdaystart + increment * 4;
			count = countstart + increment * 4;
			qty = quantitystart + increment * 4;

			int inc_count = 0;

			for (auto sl : people)
			{
				last_name = sl.item.get_string(0);
				if (not last_name.starts_with("last")) {
					std::cout << __LINE__ << ":last name failed" << std::endl;
					return false;
				}
				first_name = sl.item.get_string(1);
				if (not first_name.starts_with("first")) {
					std::cout << __LINE__ << ":first name failed" << std::endl;
					return false;
				}
				birthday = sl.item.get_time(2);
				count = sl.item.get_int64(3);
				qty = sl.item.get_double(4);

				if (birthday != birthdaystart + increment * inc_count) {
					std::cout << __LINE__ << ":birthday failed" << std::endl;
					return false;
				}

				if (count != countstart + increment * inc_count) {
					std::cout << __LINE__ << ":count failed" << std::endl;
					return false;
				}

				if (qty != quantitystart + increment * inc_count) {

					std::cout << __LINE__ << ":qty failed" << std::endl;
					return false;
				}

				inc_count++;
			}

			return true;
		}
		catch (std::exception exc)
		{
			system_monitoring_interface::global_mon->log_exception(exc);
			return false;
		}

	}

	bool array_tests()
	{

		try {
			std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
			box->init(1 << 21);
			application app;

			jschema schema;
			relative_ptr_type schema_id;

			schema = jschema::create_schema(box, 50, true, schema_id);

			put_class_request sprite_frame_request;

			sprite_frame_request.class_name = "spriteframe";
			sprite_frame_request.class_description = "sprite frame";
			sprite_frame_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h", "color" };
			relative_ptr_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

			if (sprite_frame_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			put_class_request sprite_class_request;
			sprite_class_request.class_name = "sprite";
			sprite_class_request.class_description = "sprite";
			sprite_class_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h" , member_field(sprite_frame_class_id) };
			relative_ptr_type sprite_class_id = schema.put_class(sprite_class_request);

			if (sprite_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			collection_id_type colid;

			init_collection_id(colid);

			relative_ptr_type classesb[2] = { sprite_class_id, null_row };

			jcollection_ref ref;
			ref.data = std::make_shared<dynamic_box>(1 << 20);
			ref.max_objects = 50;
			ref.collection_size_bytes = 1 << 19;

			init_collection_id(ref.collection_id);

			jcollection sprites = schema.create_collection(&ref);

			for (int i = 0; i < 10; i++) {

				relative_ptr_type new_sprite_id;
				auto slice = sprites.create_object(sprite_class_id, new_sprite_id);
				auto image_name = slice.get_string(0);


				auto bx = slice.get_int32("object_x");
				auto by = slice.get_int32("object_y");
				auto bw = slice.get_int32("object_w");
				auto bh = slice.get_int32("object_h");

				rectangle image_rect;

				image_name = std::format("{} #{}", "image", i);
				bx = 0;
				by = 0;
				bw = 1000;
				bh = 1000;

#if _DETAIL
				std::cout << "before:" << image_name << std::endl;
				std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

			}

			int scount = 0;

			for (auto slice : sprites)
			{
				auto image_name = slice.item.get_string(0);
				auto bx = slice.item.get_int32("object_x");
				auto by = slice.item.get_int32("object_y");
				auto bw = slice.item.get_int32("object_w");
				auto bh = slice.item.get_int32("object_h");

#if _DETAIL
				std::cout << image_name << std::endl;
				std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

				if (bw != 1000 or bh != 1000) {

					std::cout << __LINE__ << ":array failed" << std::endl;
					return false;
				}

			}

			return true;
		}
		catch (std::exception& exc)
		{
			system_monitoring_interface::global_mon->log_exception(exc);
			return false;
		}
	}
}

#endif
