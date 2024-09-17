/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is the core database engine for the corona database server.  

Notes
The templated types such as file_transaction are all awaiters.
The threading behavior is designed to be, switch to thread.... 
allowing for massive scalability.
However, multiuser / multithread testing is necessary.

For Future Consideration
Load and multithread engine tests.
Deploy a database to azure.
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
***********************************************/


namespace corona
{

	class corona_db_header_struct
	{
	public:
		int64_t								object_id;
		relative_ptr_type					classes_location;
		relative_ptr_type					indexes_location;
		iarray<list_block_header, 62>		free_lists;

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
		std::string file_name;
		int			line_number;

		validation_error() = default;
		validation_error(const validation_error& _src) = default;
		validation_error(validation_error&& _src) = default;
		validation_error& operator = (const validation_error& _src) = default;
		validation_error& operator = (validation_error&& _src) = default;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("ClassName", class_name);
			_dest.put_member("FieldName", field_name);
			_dest.put_member("Message", message);
			_dest.put_member("FileName", file_name);
			_dest.put_member("LineNumber", line_number);
		}

		virtual void put_json(json& _src)
		{
			class_name = _src["ClassName"];
			field_name = _src["FieldName"];
			message = _src["Message"];
			file_name = _src["FileName"];
			line_number = _src["LineNumber"];
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			bool is_empty = _object_to_test.empty();
			if (required and is_empty) {
				validation_error ve;
				ve.class_name = _class_name;
				ve.field_name = _field_name;
				ve.file_name = __FILE__;
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (_object_to_test.array()) 
				{
					for (auto obj : _object_to_test) 
					{
						std::string object_class_name;
						if (obj.object()) {
							object_class_name = _object_to_test["ClassName"];
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
						if (allowed_classes.contains(object_class_name) or allowed_classes.contains("*"))
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
					ve.file_name = __FILE__;
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (_object_to_test.object())
				{
					std::string object_class_name;
					if (_object_to_test.object()) {
						object_class_name = _object_to_test["ClassName"];
					}
					if (allowed_classes.contains(object_class_name) or allowed_classes.contains("*"))
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
					ve.file_name = __FILE__;
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_validation_errors, _class_name, _field_name, _object_to_test)) {
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
					ve.file_name = __FILE__;
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_validation_errors, _class_name, _field_name, _object_to_test)) {
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
					ve.file_name = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format("Value '{0}' must be between {1} and {2}", chumpy, min_value, max_value);
					_validation_errors.push_back(ve);
					return false;
				};
			}
		}

	};

	template <typename scalar_type> class general_field_options : public field_options_base
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

		if (field_options_base::accepts(_validation_errors, _class_name, _field_name, _object_to_test)) {
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
				ve.file_name = __FILE__;
				ve.line_number = __LINE__;
				ve.message = std::format("Value '{0}' must be between {1} and {2}", chumpy, min_value, max_value);
				_validation_errors.push_back(ve);
				return false;
			};
		}

	};

	class choice_field_options : public field_options_base
	{
	public:
		std::string class_source;
		std::string id_field_name;
		std::string description_field_name;

		choice_field_options() = default;
		choice_field_options(const choice_field_options& _src) = default;
		choice_field_options(choice_field_options&& _src) = default;
		choice_field_options& operator = (const choice_field_options& _src) = default;
		choice_field_options& operator = (choice_field_options&& _src) = default;
		virtual ~choice_field_options() = default;


		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("class_source", class_source);
			_dest.put_member("id_field_name", id_field_name);
			_dest.put_member("description_field_name", description_field_name);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			class_source = _src["class_source"];
			id_field_name = _src["id_field_name"];
			description_field_name = _src["description_field_name"];
		}

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			bool is_empty = _object_to_test.empty();
			if (required and is_empty) {
				validation_error ve;
				ve.class_name = _class_name;
				ve.field_name = _field_name;
				ve.file_name = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "required field";
				_validation_errors.push_back(ve);
				return false;
			};
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

		virtual bool accepts(std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (options) {
				return options->accepts(_validation_errors, _class_name, _field_name, _object_to_test);
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
				options->get_json(joptions);
				_dest.put_member("options", joptions);
			}
		}

		virtual void put_json(json& _src)
		{
			field_type = _src["field_type"];
			field_name = _src["field_name"];

			json joptions = _src["options"];

			if (field_type == "object") 
			{
				options = std::make_shared<object_field_options>();
				options->put_json(joptions);
			}
			else if (field_type == "array")
			{
				options = std::make_shared<array_field_options>();
				options->put_json(joptions);
			}
			else if (field_type == "number" || field_type == "double")
			{
				options = std::make_shared<general_field_options<double>>();
				options->put_json(joptions);
			}
			else if (field_type == "int64")
			{
				options = std::make_shared<int64_field_options>();
				options->put_json(joptions);
			}
			else if (field_type == "string")
			{
				options = std::make_shared<string_field_options>();
				options->put_json(joptions);
			}
			else if (field_type == "bool")
			{
				options = std::make_shared<field_options_base>();
				options->put_json(joptions);
			}
			else if (field_type == "datetime")
			{
				options = std::make_shared<general_field_options<date_time>>();
				options->put_json(joptions);
			}
			else if (field_type == "function")
			{
				;
			}
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
			_dest.put_member("class_source", index_name);

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
				for (auto key : index_keys) {
					std::string key_name = key;
					index_keys.push_back(key);
				}
			}
			index_location = (int64_t)_src["index_location"];
		}
	};

	class class_definition 
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

		bool empty()
		{
			return class_name.empty();
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member("class_name", class_name);
			_dest.put_member("class_description", class_description);
			_dest.put_member("base_class_name", base_class_name);
			_dest.put_member_i64("table_location", table_location);

			if (fields.size() > 0) {
				json jfield_array = jp.create_array();
				for (auto field : fields) {
					json jfield_definition = jp.create_object();
					field.second->get_json(jfield_definition);
					jfield_array.push_back(jfield_definition);
				
				}
				_dest.put_member("fields", jfield_array);
			}

			if (indexes.size() > 0) {
				json jindex_array = jp.create_array();
				for (auto index : indexes) {
					json jindex_definition = jp.create_object();
					index.second->get_json(jindex_definition);
					jindex_array.push_back(jindex_definition);

				}
				_dest.put_member("indexes", jindex_array);
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
			class_name = _src["class_name"];
			class_description = _src["class_description"];
			base_class_name = _src["base_class_name"];
			table_location = (int64_t)_src["table_location"];

			json jfields, jindexes, jancestors, jdescendants;

			fields.clear();
			jfields = _src["fields"];
			if (jfields.array()) {
				for (auto jfield : jfields) {
					std::shared_ptr<field_definition> field = std::make_shared<field_definition>();
					field->put_json(jfield);
					fields.insert_or_assign(field->field_name, field);
				}
			}

			indexes.clear();
			jindexes = _src["indexes"];
			if (jindexes.array()) {
				for (auto jindex : jindexes) {
					std::shared_ptr<index_definition> index = std::make_shared<index_definition>();
					index->put_json(jindex);
					indexes.insert_or_assign(index->index_name, index);
				}
			}

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

		}

	};

	class corona_database : public file_block
	{
		corona_db_header header;

		json schema;


		std::map<class_method_key, json_function_function> functions;

		crypto crypter;

		std::string send_grid_api_key;
		bool watch_polling;

		std::map<std::string, bool> allowed_field_types = {
			{ "object", true },
			{ "array", true },
			{ "number", true },
			{ "int64", true },
			{ "string", true },
			{ "bool", true },
			{ "datetime", true },
			{ "function", true }
		};

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

		virtual relative_ptr_type allocate_space(int64_t _size) override
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
			commit();
	
			json response =  create_class(R"(
{	
	"ClassName" : "SysObject",
	"ClassDescription" : "Base of all objects",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"Created" : "datetime",
			"CreatedBy" : "string",
			"Updated" : "datetime",
			"UpdatedBy" : "string"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json test =  classes->get(R"({"ClassName":"SysObject"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysObject after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysObject", true);

			response =  create_class(R"(
{
	"ClassName" : "SysSchemas",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Database script changes",
	"Fields" : {			
			"SchemaName" : "string",
			"SchemaDescription" : "string",
			"SchemaVersion" : "string",
			"SchemaAuthors" : "array",
			"Classes" : "array",
			"Users" : "array",
			"Datasets" : "array"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysSchemas"})");
			if (test.empty() or test.is_member("ClassName", "SysParseErrors")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysSchemas after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			created_classes.put_member("SysSchemas", true);

			response =  create_class(R"(
{
	"ClassName" : "SysDatasets",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Database script changes",
	"Fields" : {			
			"DatasetName" : "string",
			"DatasetDescription" : "string",
			"DatasetVersion" : "string",
			"DatasetAuthors" : "array",
			"RunOnChange": "bool",
			"Objects" : "array",
			"Import" : "object"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysDatasets put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysSchemas"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysDatasets after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysSchemas", true);

			response =  create_class(R"(
{	
	"ClassName" : "SysReference",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"LinkObjectId" : "int64"
	}
}
)");

			created_classes.put_member("SysReference", true);

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysReference put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysReference"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysParseError after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			response =  create_class(R"(
{	
	"BaseClassName" : "SysObject",
	"ClassName" : "SysUser",
	"ClassDescription" : "A user",
	"Fields" : {			
			"ClassName" : "string",
			"FirstName" : "string",
			"LastName" : "string",
			"UserName" : "string",
			"Email" : "string",
			"Mobile" : "string",
			"Street" : "string",
			"City" : "string",
			"State" : "string",
			"Zip" : "string",
			"Teams" : {
				"FieldType" : "array"
			}
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysUser put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysUser"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysUser after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysUser", true);

			response =  create_class(R"(
{	
	"ClassName" : "SysPermission",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Permissions flags",
	"Fields" : {			
			"Get" : "bool",
			"Put" : "bool",
			"Delete" : "bool",
			"Replace" : "bool"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysPermission put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysPermission"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysPermission after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysPermission", true);


			response =  create_class(R"(
{	
	"ClassName" : "SysMember",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Team member",
	"Fields" : {
			"Permissions" : 
			{
				"FieldType" : "object"
			},
			"GrantUser" : 
			{
				"FieldType" : "object"
			}
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysMember put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysMember"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysMember after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysMember", true);

			response =  create_class(R"(
{	
	"ClassName" : "SysGrant",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"Permissions" : 
			{
					"FieldType" : "object"
			},
			"GrantClassName" : "string"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysGrant put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysGrant"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysGrant after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysGrant", true);

			response =  create_class(R"(
{	
	"ClassName" : "SysClassGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to a class"
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysClassGrant put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysClassGrant"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysGlassGrant after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysClassGrant", true);

			class_location =  create_class(R"(
{	
	"ClassName" : "SysObjectGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"ObjectFilter" : "object"
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysObjectGrant put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysObjectGrant"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysObjectGrant after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysObjectGrant", true);

			response =  create_class(R"(
{	
	"ClassName" : "SysTeam",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A team",
	"Fields" : {			
			"TeamName" : "string",
			"TeamDescription" : "string",
			"Members" : {
				"FieldType":"array"
			},
			"Grants" : {
				"FieldType":"array"
			}
	}
}
)");

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysTeam put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"ClassName":"SysTeam"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysTeam after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysTeam", true);

			json gc = jp.create_object();
			json gcr = create_system_request( gc );

			json classes_array_response =  get_classes(gcr);
			json classes_array = classes_array_response["Data"];
			json classes_grouped = classes_array.group([](json& _item) -> std::string {
				return (std::string)_item["ClassName"];
				});

			bool missing = classes_array.any([classes_grouped](json& _item) {
				return !classes_grouped.has_member(_item["ClassName"]);
				});

			if (missing) {
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				std::cout << __FILE__ << " " << __LINE__ << ":Class list returned from database missed some classes." << std::endl;
				return result;
			}

			json new_user_request;
			json new_user_data;

			new_user_data = jp.create_object();
			new_user_data.put_member("ClassName", "SysUser");
			new_user_data.put_member("UserName", default_user);
			new_user_data.put_member("Email", default_email_address);
			new_user_data.put_member("Password1", default_password);
			new_user_data.put_member("Password2", default_password);

			new_user_request = create_system_request(new_user_data);
			json new_user_result =  create_user(new_user_request);
			json new_user = new_user_result["Data"];
			json user_return = create_response(new_user_request, true, "Ok", new_user, method_timer.get_elapsed_seconds());
			response = create_response(new_user_request, true, "Database Created", user_return, method_timer.get_elapsed_seconds());

			commit();

			system_monitoring_interface::global_mon->log_job_stop("create_database", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

private:

		json create_class(std::string _text)
		{
			json_parser jp;

			json jclass_def = jp.parse_object(_text);

			json sys_request = create_system_request(jclass_def);

			json response = put_class(sys_request);

			return response;
		}

		json check_object(json check_object_request)
		{
			timer method_timer;
			json result;
			json_parser jp;
			date_time current_date = date_time::now();

			json object_load = check_object_request["Data"];
			bool strict_enabled = (bool)check_object_request["Strict"];
			std::string user = check_object_request.query("Token.Name")["value"];

			result = create_response(check_object_request, true, "Ok", object_load, method_timer.get_elapsed_seconds());

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
				return _item["ClassName"];
				});

			std::map<std::string, class_definition> classes_ahead;

			auto class_list = classes_group.get_members();

			for (auto class_pair : class_list) {

				auto class_def = load_class(class_pair.first);
				classes_ahead.insert_or_assign(class_pair.first, class_def);
			}

			for (auto object_definition : object_list)
			{
				if (not object_definition.object())
				{
					result = create_response(check_object_request, false, "This is not an object", object_definition, method_timer.get_elapsed_seconds());
					result_list.push_back(result);
				}

				if (not object_definition.has_member("ClassName"))
				{
					result = create_response(check_object_request, false, "Object must have class name", object_definition, method_timer.get_elapsed_seconds());
					result_list.push_back(result);
				}

				db_object_id_type object_id = -1;

				if (object_definition.has_member("ObjectId"))
				{
					object_id = object_definition["ObjectId"].get_int64s();
					object_definition.put_member("Modified", current_date);
					object_definition.put_member("ModifiedBy", user);
				}
				else
				{
					object_id =  get_next_object_id();
					object_definition.put_member_i64("ObjectId", object_id);
					object_definition.put_member("Created", current_date);
					object_definition.put_member("CreatedBy", user);
				}

				json warnings = jp.create_array();

				std::string class_name = object_definition["ClassName"];

				class_definition& class_data = classes_ahead[class_name];

				if (not class_data.empty())
				{

					std::vector<validation_error> validation_errors;

					// check the object against the class definition for correctness
					// first we see which fields are in the class not in the object

					for (auto kv : class_data.fields) {
						json err_field = jp.create_object("Name", kv.first);
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
						if (class_data.fields.contains(om.first)) {
							auto& fld = class_data.fields[om.first];
							fld->accepts(validation_errors, class_name, om.first, om.second);
						}
						else 
						{
							json warning = jp.create_object();
							validation_error ve;
							ve.class_name = class_name;
							ve.field_name = om.first;
							ve.file_name = __FILE__;
							ve.line_number = __LINE__;
							ve.message = "Field not found inn class definition";
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
						result.put_member("Message", msg);
						result.put_member("Success", 0);
						result.put_member("Errors", warnings);
						result.put_member("Data", object_definition);
					}
					else {
						result.put_member("Message", "Ok");
						result.put_member("Success", 1);
						result.put_member("Data", object_definition);
					}
				}
				else
				{
					std::string msg = std::format("'{0}' is not valid class_name", class_name);
					result.put_member("Message", msg);
					result.put_member("Success", 0);
					result.put_member("Data", object_definition);
				}
				result_list.push_back(result);
			}

			result = create_response(check_object_request, true, "Objects processed", object_list, method_timer.get_elapsed_seconds());
			result.put_member("Notes", result_list);
			return result;
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
			token.put_member("UserName", _user_name);
			token.put_member("Authorization", _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			std::string hash = crypter.hash(_data);
			token.put_member("DataHash", hash);
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			std::string token_string = token.to_json();
			std::string base64_token_string = base64_encode(token_string );

			payload.put_member("Token", base64_token_string);
			payload.put_member("Success", _success);
			payload.put_member("Message", _message);
			payload.put_member("Data", _data);
			payload.put_member("Seconds", _seconds);

			return payload;
		}

		json create_response(json _request, bool _success, std::string _message, json _data, double _seconds)
		{
			json_parser jp;
			json payload = jp.create_object();
			json token = jp.create_object();
			json src_token = _request["Token"];
			
			token.copy_member("UserName", src_token);
			token.copy_member("Authorization", src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			std::string token_string = token.to_json();
			std::string base64_token_string = base64_encode(token_string);

			payload.put_member("Token", base64_token_string);
			payload.put_member("Success", _success);
			payload.put_member("Message", _message);
			payload.put_member("Data", _data);
			payload.put_member_double("Seconds", _seconds);
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

			if (not token.has_member("Signature"))
			{
				return empty;
			}

			std::string signature = token["Signature"];
			token.erase_member("Signature");

			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());

			token.put_member("Signature", signature);

			date_time current = date_time::utc_now();
			date_time expiration = (date_time)token["TokenExpires"];

			if (current > expiration)
			{
				return empty;
			}

			if (cipher_text != signature)
			{
				return empty;
			}

			if (not token.has_member("Authorization"))
			{
				return empty;
			}

			std::string authorization = token["Authorization"];
			std::string user = token["UserName"];

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

		bool check_message(json& _message, std::vector<std::string> _authorizations)
		{
			std::string token = _message["Token"];

			json result = check_token(token, _authorizations);
			bool is_ok = not result.empty();

			return is_ok;
		}

		json get_message_user(json _token)
		{
			if (_token.has_member("Token")) {
				_token = _token["Token"];
			}
			json token_name = _token.extract({ "UserName" });
			return token_name;
		}

		json acquire_object(json _object_key)
		{
			json_parser jp;
			json obj;


			std::string class_name = _object_key["ClassName"];
			class_definition classd = load_class(class_name);

			if (not classd.empty()) 
			{
				json_table class_data(this, { "ObjectId" });
				class_data.open(classd.table_location);
				obj = class_data.get(_object_key);
				return obj;
			}

			return obj;
		}

		json select_object(json _key)
		{
			json_parser jp;
			json obj;

			_key.set_natural_order();

			std::string class_name = _key["ClassName"];

			class_definition classd = load_class(class_name);

			if (not classd.empty())
			{
				relative_ptr_type rpt = classd.table_location;
				json_table class_data(this, { "ObjectId" });
				class_data.open(rpt);

				// Now, if there is an index set specified, let's go see if we can find one and use it 
				// rather than scanning the table

				if (classd.indexes.size() > 0)
				{
					std::shared_ptr<index_definition> matched_index;
					int max_matched_key_count = 0;

					// go through each index

					for (auto index_pair : classd.indexes)
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

						obj = jp.create_array();
						json object_key = jp.create_object();
						object_key.put_member("ClassName", class_name);

						obj = index_table.select(_key, [&object_key, &class_data](int _idx, json& _item) -> json {
							object_key.copy_member("ObjectId", _item);
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
			}

			return obj;
		}

		json get_user(std::string _user_name)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member("ClassName", "SysUser");
			obj.put_member("UserName", _user_name);
			return select_object(obj);
		}

		json get_team(std::string _team_name)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member("ClassName", "SysTeam");
			obj.put_member("TeamName", _team_name);
			return select_object(obj);
		}

		json get_schema(std::string schema_name, std::string schema_version)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member("ClassName", "SysSchemas");
			obj.put_member("SchemaName", schema_name);
			obj.put_member("SchemaVersion", schema_version);
			return select_object(obj);
		}

		json get_dataset(std::string dataset_name, std::string dataset_version)
		{
			json_parser jp;
			json obj = jp.create_object();
			obj.put_member("ClassName", "SysDatasets");
			obj.put_member("DatasetName", dataset_name);
			obj.put_member("DatasetVersion", dataset_version);
			return select_object(obj);
		}

		bool has_class_permission(
			std::string _token,
			std::string _class_name,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			json token = check_token(_token, { auth_general });
			// check the token to make sure it is valid - this includes signature verification
			if (token.empty()) {
				return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_message_user(token);
			std::string user_name = user_key["UserName"];

			if (user_name == default_user) 
			{
				return true;
			}

			user =  get_user(user_name);
			if (user.empty()) {
				return false;
			}

			// Now go through the teams the user is a member of and check the grants to see if we can access this
			json teams_list = user["Teams"];

			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team =  get_team(item);

				if (team.is_member("ClassName", "SysTeam")) {
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysClassGrant"))
						{
							bool has_permissions = (bool)grant["Permissions"][_permission];
							if (has_permissions) {
								granted = has_permissions;
								return granted;
							}
						}
					}
				}
			}
			return granted;
		}

		bool check_object_permission(
			json _request,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			json object = _request["Data"];

			// check the token to make sure it is valid - this includes signature verification
			json user_key = get_message_user(_request);
			user_key.put_member("ClassName", "SysUser");

			if ((std::string)user_key["UserName"] == default_user) {
				return true;
			}

			// extract the user key from the token and get the user object
			user =  get_user(user_key["UserName"]);
			if (user.empty()) {
				return false;
			}

			json teams_list = user["Teams"];

			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team =  get_team(item);

				if (team.is_member("ClassName", "SysTeam"))
				{
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysObjectGrant"))
						{
							json filter = grant["ObjectFilter"];

							bool has_matching_key = filter.compare(object) == 0;

							bool has_permissions = (bool)grant["Permissions"][_permission];

							granted = has_permissions or has_matching_key;

							if (granted) {
								return granted;
							}
						}
					}
				}
			}
			return granted;
		}

		db_object_id_type get_next_object_id()
		{
			InterlockedIncrement64(&header.data.object_id);
			return header.data.object_id;
		}

		bool is_ancestor(json _token, std::string _base_class, std::string _class_to_check)
		{
			json_parser jp;

			json class_key = jp.create_object("ClassName", _base_class);
			class_key.set_natural_order();

			json class_obj =  classes->get(class_key);

			if (not class_obj.empty())
			{
				bool has_ancestor = class_obj["Ancestors"].has_member(_class_to_check);
				return has_ancestor;
			}

			return false;
		}

		bool is_descendant(json _token, std::string _base_class, std::string _class_to_check)
		{
			json_parser jp;

			json class_key = jp.create_object("ClassName", _class_to_check);
			class_key.set_natural_order();


			json class_obj =  classes->get(class_key);

			if (not class_obj.empty())
			{
				bool has_ancestor = class_obj["Ancestors"].has_member(_base_class);
				return has_ancestor;
			}

			return false;
		}


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
			std::vector<std::string> class_key_fields({ "ClassName" });
			std::vector<std::string> indexes_key_fields({ "ClassName" });
			classes = std::make_shared<json_table>(this, class_key_fields);
			indexes = std::make_shared<json_table>(this, indexes_key_fields);
			token_life = time_span(1, time_models::hours);	
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
				default_user = server["DefaultUserName"];
				default_password = server["DefaultUserPassword"];
				default_email_address = server["DefaultUserEmailAddress"];
				default_guest_team = server["DefaultGuestTeam"];
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_config", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		}

		class_definition load_class(std::string _class_name)
		{
			json_parser jp;
			json key = jp.create_object();
			key.put_member("ClassName", _class_name);
			json class_def = classes->get(key);
			class_definition cd;
			cd.put_json(class_def);
			return cd;
		}

		json save_class(class_definition& _class_to_save)
		{
			json_parser jp;
			json class_def = jp.create_object();
			_class_to_save.get_json(class_def);
			classes->put(class_def);
			return class_def;
		}

		json apply_schema(json _schema)
		{
			date_time start_schema = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("apply_schema", "Applying schema file", start_schema, __FILE__, __LINE__);

			bool new_database = true;

			std::map<std::string, bool> changed_classes;

			if (not _schema.has_member("SchemaName"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema name");
			}

			if (not _schema.has_member("SchemaVersion"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema version");
			}

			if (not _schema.has_member("SchemaAuthors"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema author");
			}

			json_parser jp;

			json schema_key = jp.create_object();
			schema_key.copy_member("SchemaName", _schema);
			schema_key.copy_member("SchemaVersion", _schema);
			schema_key.put_member("ClassName", "SysSchemas");
			schema_key.set_compare_order({ "SchemaName", "SchemaVersion" });

			json schema_test =  select_object(schema_key);

			if (schema_test.object()) 
			{
				new_database = false;
			}

			if (_schema.has_member("Classes"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Classes", start_section, __FILE__, __LINE__);

				json class_array = _schema["Classes"];
				if (class_array.array())
				{
					for (int i = 0; i < class_array.size(); i++)
					{
						date_time start_class = date_time::now();
						timer txc;

						json class_definition = class_array.get_element(i);
						system_monitoring_interface::global_mon->log_function_start("put class", class_definition["ClassName"], start_class, __FILE__, __LINE__);

						try {

							json put_class_request = create_system_request(class_definition);
							json class_result =  put_class(put_class_request);

							if (class_result.error()) 
							{
								system_monitoring_interface::global_mon->log_warning(class_result, __FILE__, __LINE__);
							}
							else 
							{
								json class_data = class_result["Data"];
								bool changed_class = (bool)class_data["ClassChanged"];
								if (changed_class) {
									std::string class_name = class_data["ClassName"];
									changed_classes[class_name] = true;
								}
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::global_mon->log_exception(exc);
						}
						system_monitoring_interface::global_mon->log_function_stop("put class", class_definition["ClassName"], txc.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Classes", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else
			{
				system_monitoring_interface::global_mon->log_warning("Classes not found in schema");
			}

			if (_schema.has_member("Users"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Users", start_section, __FILE__, __LINE__);
				json user_array = _schema["Users"];
				if (user_array.array())
				{
					for (int i = 0; i < user_array.size(); i++)
					{
						date_time start_user = date_time::now();
						timer txu;

						json user_definition = user_array.get_element(i);
						system_monitoring_interface::global_mon->log_function_start("put user", user_definition["UserName"], start_user, __FILE__, __LINE__);
						json put_user_request = create_system_request(user_definition);
						create_user(put_user_request);
					    system_monitoring_interface::global_mon->log_function_stop("put class", user_definition["UserName"], txu.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Users", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
	
			if (_schema.has_member("Datasets"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Datasets", start_section, __FILE__, __LINE__);
				json user_array = _schema["Users"];

				json script_array = _schema["Datasets"];
				if (script_array.array())
				{
					for (int i = 0; i < script_array.size(); i++)
					{
						date_time start_dataset = date_time::now();
						timer txs;

						json script_definition = script_array.get_element(i);

						script_definition.put_member("ClassName", "SysDatasets");
						std::string dataset_name = script_definition["DatasetName"];
						std::string dataset_version = script_definition["DatasetVersion"];

						system_monitoring_interface::global_mon->log_job_section_start("DataSet", dataset_name + " Start", start_dataset, __FILE__, __LINE__);

						bool script_run = (bool)script_definition["RunOnChange"];
						json existing_scripts = get_dataset(dataset_name, dataset_version);
						json existing_script = existing_scripts.get_first_element();
						bool run_script = false;
						if (existing_script.empty() or script_run)
							run_script = true;

						if (existing_script.empty())
						{
							// in corona, creating an object doesn't actually persist anything 
							// but a change in identifier.  It's a clean way of just getting the 
							// "new chumpy" item for ya.  Or you can just shove it in there.
							json put_script_request = create_system_request(script_definition);
							json created_object = put_script_request["Data"];
							json save_result = put_object(put_script_request);
							if (not save_result["Success"]) {
								system_monitoring_interface::global_mon->log_warning(save_result["Message"]);
								system_monitoring_interface::global_mon->log_json<json>(save_result);
								existing_script = save_result["Data"];
							}
							else
								system_monitoring_interface::global_mon->log_information(save_result["Message"]);
						}

						if (run_script and script_definition.has_member("Import"))
						{
							json import_spec = script_definition["Import"];
							std::vector<std::string> missing;

							if (not import_spec.has_members(missing, { "TargetClass", "Type" })) {
								system_monitoring_interface::global_mon->log_warning("Import missing:");
								std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
									system_monitoring_interface::global_mon->log_warning(s);
									});
								system_monitoring_interface::global_mon->log_information("the source json is:");
								system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
								continue;
							}

							std::string target_class = import_spec["TargetClass"];
							std::string import_type = import_spec["Type"];

							if (import_type == "csv") {

								if (not import_spec.has_members(missing, { "FileName", "Delimiter" })) {
									system_monitoring_interface::global_mon->log_warning("Import CSV missing:");
									std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
										system_monitoring_interface::global_mon->log_warning(s);
										});
									system_monitoring_interface::global_mon->log_information("the source json is:");
									system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
									continue;
								}

								std::string file_name = import_spec["FileName"];
								std::string delimiter = import_spec["Delimiter"];
								if (file_name.empty() or delimiter.empty()) {
									system_monitoring_interface::global_mon->log_warning("FileName and Delimiter can't be blank.");
								}

								json column_map = import_spec["ColumnMap"];

								FILE* fp = nullptr;
								int error_code = fopen_s(&fp, file_name.c_str(), "rS");

								if (fp) {
									// Buffer to store each line of the file.
									char line[8182];
									json datomatic = jp.create_array();

									// create template object
									json codata = jp.create_object();
									codata.put_member("ClassName", target_class);
									json cor = create_system_request(codata);
									json new_object_response =  create_object(cor);

									if (new_object_response["Success"]) {
										json new_object_template = new_object_response["Data"];

										// Read each line from the file and store it in the 'line' buffer.
										int64_t total_row_count = 0;
										while (fgets(line, sizeof(line), fp)) {
											// Print each line to the standard output.
											json new_object = new_object_template.clone();
											new_object.erase_member("ObjectId");
											jp.parse_delimited_string(new_object, column_map, line, delimiter[0]);
											datomatic.push_back(new_object);
											if (datomatic.size() > 1000) {
												timer tx;
												json cor = create_system_request(datomatic);
												json put_result =  put_object(cor);
												if (put_result["Success"]) {
													double e = tx.get_elapsed_seconds();
													total_row_count += datomatic.size();
													std::string msg = std::format("import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
													system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
													datomatic = jp.create_array();
												}
												else {
													std::string msg = std::format("Error saving object {0}", (std::string)put_result["Message"]);
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
											if (put_result["Success"]) {
												double e = tx.get_elapsed_seconds();
												total_row_count += datomatic.size();
												std::string msg = std::format("import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
												system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
												datomatic = jp.create_array();
											}
											else {
												std::string msg = std::format("Error saving object {0}", (std::string)put_result["Message"]);
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
									std::string msg = std::format("could not open file {0}:{1}", file_name, error_buffer);
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

						if (run_script and script_definition.has_member("Objects")) {
							json object_list = script_definition["Objects"];
							if (object_list.array()) {
								for (int j = 0; j < object_list.size(); j++) {
									json object_definition = object_list.get_element(i);
									json put_object_request = create_system_request(object_definition);
									// in corona, creating an object doesn't actually persist anything 
									// but a change in identifier.  It's a clean way of just getting the 
									// "new chumpy" item for ya.  
									json create_result =  create_object(put_object_request);
									if (create_result["Success"]) {
										json created_object = put_object_request["Data"];
										json save_result =  put_object(put_object_request);
										if (not save_result["Success"]) {
											system_monitoring_interface::global_mon->log_warning(save_result["Message"]);
											system_monitoring_interface::global_mon->log_json<json>(save_result);
										}
										else
											system_monitoring_interface::global_mon->log_information(save_result["Message"]);
										object_definition.copy_member("Success", create_result);
										object_definition.copy_member("Message", create_result);
									}
									else 
									{
										object_definition.copy_member("Success", create_result);
										object_definition.copy_member("Message", create_result);
										system_monitoring_interface::global_mon->log_warning(create_result["Message"], __FILE__, __LINE__);
										system_monitoring_interface::global_mon->log_json<json>(create_result);
									}
								}
							}

							date_time completed_date = date_time::now();
							script_definition.put_member("Completed", completed_date);
							json put_script_request = create_system_request(script_definition);
							json save_script_result =  put_object(put_script_request);
							if (not save_script_result["Success"]) {
								system_monitoring_interface::global_mon->log_warning(save_script_result["Message"]);
								system_monitoring_interface::global_mon->log_json<json>(save_script_result);
							}
							else
								system_monitoring_interface::global_mon->log_information(save_script_result["Message"]);
						}

						system_monitoring_interface::global_mon->log_job_section_stop("DataSet", dataset_name + " Finished", txs.get_elapsed_seconds(), __FILE__, __LINE__);
					}
					system_monitoring_interface::global_mon->log_job_section_stop("DataSets", "", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}

			_schema.put_member("ClassName", "SysSchemas");

			json put_schema_request = create_system_request(_schema);
			// in corona, creating an object doesn't actually persist anything 
			// but a change in identifier.  It's a clean way of just getting the 
			// "new chumpy" item for ya.  
			json create_schema_result =  create_object(put_schema_request);
			if (create_schema_result["Success"]) {
				json created_object = put_schema_request["Data"];
				json save_schema_result =  put_object(put_schema_request);
				if (not save_schema_result["Success"]) {
					system_monitoring_interface::global_mon->log_warning(save_schema_result["Message"]);
					system_monitoring_interface::global_mon->log_json<json>(save_schema_result);
				}
				else
					system_monitoring_interface::global_mon->log_information(save_schema_result["Message"]);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_warning(create_schema_result["Message"], __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(create_schema_result);
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_schema", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			json temp = R"({ "Success" : true, "Message" : "Everything Ok, suitation normal."})"_jobject;

			return temp;
		}

		relative_ptr_type open_database(relative_ptr_type _header_location)
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

		json create_user(json create_user_request)
		{
			timer method_timer;
			json_parser jp;
			json response;


			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_user", "start", start_time, __FILE__, __LINE__);

			json data = create_user_request["Data"];

			std::string user_name = data["UserName"];
			std::string user_password1 = data["Password1"];
			std::string user_password2 = data["Password2"];
			std::string user_class = "SysUser";

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
			create_user_params.put_member("ClassName", user_class);
			create_user_params.put_member("UserName", user_name);
			create_user_params.put_member("Password", hashed_pw);
			create_user_params.copy_member("FirstName", data);
			create_user_params.copy_member("LastName", data);
			create_user_params.copy_member("Email", data);
			create_user_params.copy_member("Mobile", data);
			create_user_params.copy_member("Street", data);
			create_user_params.copy_member("City", data);
			create_user_params.copy_member("State", data);
			create_user_params.copy_member("Zip", data);

			json create_object_request = create_request(create_user_request, create_user_params);
			json user_result =  put_object(create_object_request);
			if (user_result["Success"]) {
				json new_user_wrapper = user_result["Data"];
				response = create_response(user_name, auth_general, true, "User created", data, method_timer.get_elapsed_seconds());
			}
			else
			{
				response = create_response(create_user_request, false, "User not created", data, method_timer.get_elapsed_seconds());
			}

			commit();

			system_monitoring_interface::global_mon->log_function_stop("create_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return response;
		}

		// this starts a login attempt
		json login_user(json _login_request)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("login_user", "start", start_time, __FILE__, __LINE__);

			json data = _login_request;
			std::string user_name = data["UserName"];
			std::string user_password = data["Password"];
			std::string hashed_user_password;

			std::string hashed_pw = crypter.hash(user_password);

			json users = get_user(user_name);

			json user = users.get_first_element();
			std::string pw = user["Password"];

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

			commit();

			return response;
		}

		std::string get_random_code()
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

		json edit_object(json _edit_object_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("edit_object", "start", start_time, __FILE__, __LINE__);

			json token = _edit_object_request["Token"];
			json object_key = _edit_object_request["ObjectId"];

			if (not check_message(_edit_object_request, { auth_general }))
			{
				result = create_response(_edit_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return result;
			}

			json object_options = jp.create_object();
			object_options.copy_member("ObjectId", object_key);
			object_options.put_member_object("Fields");
			json object_fields = object_options["Fields"];
			object_options.put_member_object("Edit");
			json edit_options_root = object_options["Edit"];

			json get_response =  get_object(_edit_object_request);
			if (get_response["Success"]) {
				json obj = get_response["Data"];
				object_options.put_member("Data", obj);
				json class_key = obj.extract({ "ClassName" });
				class_key.put_member("Token", token);
				json class_response =  get_class(class_key);
				if (class_response["Success"]) {
					json class_definition = class_response["Data"];
					object_options.put_member_object("ClassDefinition", class_definition);
					auto fields = class_definition["Fields"].get_members();
					for (auto field : fields) {
						json edit_options = edit_options_root.put_member_array(field.first);
						object_fields.put_member(field.first, field.second);
					}
				}
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return create_response(_edit_object_request, true, "Ok", object_options, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return create_response(_edit_object_request, false, "Sadly, this object eludes you.", object_options, method_timer.get_elapsed_seconds());
		}


		json get_classes(json get_classes_request)
		{
			timer method_timer;
			json_parser jp;

			json result;
			json result_list;
			
			scope_lock lock_one(classes_rw_lock);

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_classes", "start", start_time, __FILE__, __LINE__);


			if (not check_message(get_classes_request, { auth_general }))
			{
				result = create_response(get_classes_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return result;
			}

			result_list =  classes->select([this, get_classes_request](int _index, json& _item) {
				json_parser jp;
				json token = get_classes_request["Token"];
				bool has_permission = has_class_permission(token, _item["ClassName"], "Get");

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

		json get_class(json get_class_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_class", "start", start_time, __FILE__, __LINE__);


			std::vector<std::string> missing_elements;
			if (not get_class_request.has_members(missing_elements, { "Token", "Data" })) {
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


			if (not check_message(get_class_request, { auth_general }))
			{
				result = create_response(get_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json token = get_class_request["Token"];
			std::string class_name = get_class_request["ClassName"];

			bool can_get_class =  has_class_permission(
				token,
				class_name,
				"Get");

			json key = jp.create_object("ClassName", class_name);
			key.set_natural_order();

			{
				scope_lock lock_one(classes_rw_lock);
				result =  classes->get(key);
			}


			result = create_response(get_class_request, true, "Ok", result, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("get_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		json delete_objects(json delete_request)
		{

			date_time start_time = date_time::now();
			timer tx;

			json delete_filter = delete_request["Data"];

			system_monitoring_interface::global_mon->log_function_start("delete_objects", "start", start_time, __FILE__, __LINE__);

			timer method_timer;
			json_parser jp;
			json result;
			if (not check_message(delete_request, { auth_general }))
			{
				result = create_response(delete_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return result;
			}

			commit();

			system_monitoring_interface::global_mon->log_function_stop("delete_objects", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		}

		json put_class(json put_class_request)
		{
			timer method_timer;
			json result;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("put_class", "start", start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;
			if (not put_class_request.has_members(missing_elements, { "Token", "Data" })) {
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

			if (not check_message(put_class_request, { auth_general }))
			{
				result = create_response(put_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			json token = put_class_request["Token"];
			json jclass_definition = put_class_request["Data"];
			std::string class_name = jclass_definition["ClassName"];

			bool can_put_class =  has_class_permission(
				token,
				class_name,
				"Put");

			if (not can_put_class) {
				result = create_response(put_class_request, false, "Denied", jclass_definition, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			class_definition existing_class = load_class(class_name);
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

			std::string class_name = class_def.class_name;

			// here we are going to grab the ancestor chain for this class.

			if (not class_def.base_class_name.empty())
			{
				std::string base_class_name = class_def.base_class_name;

				class_definition base_class = load_class(base_class_name);

				if (base_class.empty())
				{
					result = create_response(put_class_request, false, "Base class not found", jclass_definition, method_timer.get_elapsed_seconds());
				}

				class_def.ancestors = base_class.ancestors;
				class_def.ancestors.insert_or_assign(base_class_name, true);

				base_class.descendants.insert_or_assign(class_name, true);
				class_def.descendants = base_class.descendants;	

				for (auto temp_field : base_class.fields)
				{
					class_def.fields.insert_or_assign(temp_field.first, temp_field.second);
				}

				for (auto descendant : class_def.descendants)
				{
					auto desc_class = load_class(descendant.first);
					desc_class.ancestors.insert_or_assign(class_name, true);
					save_class(desc_class);
				}
			}

			class_def.table_location = existing_class.table_location;

			json_table class_data(this, { "ObjectId" });
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
							if (_key == "ObjectId") {
								return true;
							}
							return false;
						});
				if (first == new_index.second->index_keys.end()) {
					new_index.second->index_keys.push_back("ObjectId");
				}
			}

			// loop through existing indexes,
			// dropping old ones that don't match
			for (auto old_index : existing_class.indexes)
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

			// reindex tables list

			std::vector<std::shared_ptr<index_definition>> indexes_to_create;

			// and once again through the indexes

			for (auto& new_index : class_def.indexes)
			{
				// don't trust the inbounnd table locations
				new_index.second->index_location = 0;

				auto existing = existing_class.indexes.find(new_index.first);

				if (existing != existing_class.indexes.end()) 
				{
					new_index.second->index_location = existing->second->index_location;
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
				});
			}

			json saved_class = save_class(class_def);	
			result = create_response(put_class_request, true, "Ok", saved_class, method_timer.get_elapsed_seconds());

			commit();
			system_monitoring_interface::global_mon->log_function_stop("put_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		json query_class_base(json query_class_request, json update_json)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("update", "start", start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;
			if (not query_class_request.has_members(missing_elements, { "Token", "Data" })) {
				std::string error_message;
				error_message = "Missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				system_monitoring_interface::global_mon->log_warning(error_message, __FILE__, __LINE__);
				response = create_response(query_class_request, false, error_message, jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			missing_elements.clear();
			json manip = query_class_request["Data"];
			if (not manip.has_members(missing_elements, { "Filter", "ClassName" })) {
				std::string error_message;
				error_message = "Missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				system_monitoring_interface::global_mon->log_warning(error_message, __FILE__, __LINE__);
				response = create_response(query_class_request, false, error_message, jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			if (not check_message(query_class_request, { auth_general }))
			{
				response = create_response(query_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json token = query_class_request["Token"];
			json base_class_name = manip["ClassName"];
			if (base_class_name.empty()) {
				response = create_response(query_class_request, false, "Classname not specified", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			bool class_granted =  has_class_permission(token, base_class_name, "Get");
			if (not class_granted)
			{
				response = create_response(query_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			class_definition class_def = load_class(base_class_name);

			json object_list = jp.create_array();

			json filter = manip["Filter"];

			for (auto class_pair : class_def.descendants)
			{
				json class_key;
				class_key = filter.clone();
				class_key.put_member("ClassName", class_pair.first);
				json objects = select_object(class_key);
				if (objects.array()) {
					for (auto obj : objects) {
						object_list.push_back(obj);
					}
				}
			}

			response = create_response(query_class_request, true, "Query completed", object_list, method_timer.get_elapsed_seconds());
			commit();

			system_monitoring_interface::global_mon->log_function_stop("update", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}


		json query_class(json query_class_request)
		{
			json_parser jp;
			json jx; // not creating an object, leaving it empty.  should work with empty objects
			// or with an object that has no members.
			return query_class_base(query_class_request, jx);
		}

		json create_object(json create_object_request)
		{
			timer method_timer;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_object", "start", start_time, __FILE__, __LINE__);


			json token = create_object_request["Token"];
			json data = create_object_request["Data"];
			std::string class_name = data["ClassName"];
			json response;

			if (not check_message(create_object_request, { auth_general }))
			{
				response = create_response(create_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			bool permission =  has_class_permission(token, class_name, "Get");
			if (not permission) {
				json result = create_response(create_object_request, false, "Cannot get class", data, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			auto class_def = load_class(class_name);

			if (not class_def.empty()) {

				json new_object = jp.create_object();
				new_object.put_member("ClassName", class_name);

				for (auto& member : class_def.fields)
				{
					if (member.first == "ClassName")
						continue;

					auto &field = member.second;
					std::string &field_type = field->field_type;

					if (field_type == "object") 
					{
						new_object.put_member_object(member.first);
					}
					else if (field_type == "array") 
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
			commit();

			return response;
			
		}

		json put_object(json put_object_request)
		{
			timer method_timer;
			json_parser jp;

			json token;
			json object_definition;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("put_object", "start", start_time, __FILE__, __LINE__);

			object_definition = put_object_request["Data"];

			if (not check_message(put_object_request, { auth_general }))
			{
				result = create_response(put_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			bool permission =  check_object_permission(put_object_request, "Put");
			if (not permission) {
				json result = create_response(put_object_request, false, "Cannot create object", put_object_request["Data"], method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			result =  check_object(put_object_request);

			if (result["Success"])
			{
				json data = result["Data"];

				json item_array;
				if (data.array()) {
					item_array = data;
				}
				else 
				{
					item_array = jp.create_array();
					item_array.push_back(data);
				}

				json grouped_by_class_name = item_array.group([](json& _item) -> std::string {
					return _item["ClassName"];
					});

				auto classes_and_data = grouped_by_class_name.get_members();

				for (auto class_pair : classes_and_data)
				{
					class_definition cd = load_class(class_pair.second);

					// now that we have our class, we can go ahead and open the storage for it
					relative_ptr_type rpt = cd.table_location;
					json_table class_data(this, { "ObjectId" });
					class_data.open(rpt);
					class_data.put_array(class_pair.second);

					// update the indexes
					for (auto class_index : cd.indexes) {
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
				commit();

				result = create_response(put_object_request, true, "Object(s) created", data, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(put_object_request, false, result["Message"], result["Warnings"], method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("put_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			commit();

			return result;
		}

		json get_object(
			json get_object_request
		)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_object", "start", start_time, __FILE__, __LINE__);

			if (not check_message(get_object_request, { auth_general }))
			{
				result = create_response(get_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			bool permission =  check_object_permission(get_object_request, "Get");
			if (not permission) {
				json result = create_response(get_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}
			
			json payload = get_object_request["Data"];
			json obj =  acquire_object(payload);

			result = create_response(get_object_request, true, "Ok", obj, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("get_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}


		json delete_object(json delete_object_request)
		{
			timer method_timer;
			json response;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("delete_object", "start", start_time, __FILE__, __LINE__);


			if (not check_message(delete_object_request, { auth_general }))
			{
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			if (not check_object_permission(delete_object_request, "Delete")) {
				json result = create_response(delete_object_request, false, "Cannot delete object", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json object_key = delete_object_request["Data"];

			response = create_response(delete_object_request, false, "Failed", object_key, method_timer.get_elapsed_seconds());

			json class_key = jp.create_object();
			json class_name = object_key["ClassName"];
			class_key.put_member("ClassName", class_name);
			json class_def = classes->get("class_key");

			if (class_def.has_member("Table")) {
				json empty;
				relative_ptr_type rpt = class_def["Table"];
				json_table class_data(this, { "ObjectId" });
				class_data.open(rpt);
				class_data.erase(object_key);
				response = create_response(delete_object_request, true, "Ok", object_key, method_timer.get_elapsed_seconds());
			}
			else 
			{
				response = create_response(delete_object_request, false, "Not found", object_key, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("delete_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			commit();

			return response;
		}

		json copy_object(json copy_request)
		{
			timer method_timer;
			json_parser jp;

			json response;

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_function_start("copy_object", "start", start_time, __FILE__, __LINE__);

			if (not check_message(copy_request, { auth_general }))
			{
				response = create_response(copy_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json source_key = copy_request["SourceKey"];

			json object_copy =  acquire_object(source_key);

			json check_request = create_request(copy_request, object_copy);

			bool permission =  check_object_permission(copy_request, "Get");
			if (not permission) {
				json result = create_response(copy_request, false, "Denied", source_key, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json new_object = object_copy.clone();
			db_object_id_type new_object_id =  get_next_object_id();
			new_object.put_member_i64("ObjectId", new_object_id);

			json por = create_request(copy_request, new_object);
			json result =  put_object(por);

			if (result["Success"]) {
				response = create_response(copy_request, true, "Ok", result["Data"], method_timer.get_elapsed_seconds());
			}
			else
			{
				response = result;
			}
			system_monitoring_interface::global_mon->log_function_stop("copy_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		json create_system_request(json _data)
		{
			json_parser jp;

			json payload = jp.create_object();
			json token = jp.create_object();
			token.put_member("UserName", default_user);
			token.put_member("Authorization", auth_system);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			payload.put_member("Token", token);
			payload.put_member("Success", true);
			payload.put_member("Message", "Ok");
			payload.put_member("Data", _data);
			payload.put_member("Seconds", 0);
			return payload;
		}

		json create_request(json _request, json _data)
		{
			json_parser jp;

			json payload = jp.create_object();
			json src_token = _request["Token"];
			json token = jp.create_object();

			token.copy_member("UserName", src_token);
			token.copy_member("Authorization", src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string hash = crypter.hash(_data);
			token.put_member("DataHash", hash);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			payload.put_member("Token", token);
			payload.put_member("Success", true);
			payload.put_member("Message", "Ok");
			payload.put_member("Data", _data);
			payload.put_member("Signature", cipher_text);
			return payload;
		}


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
		timer tx;
		system_monitoring_interface::global_mon->log_job_start("test_database_engine", "start", start_schema, __FILE__, __LINE__);

		corona_database db(dtest);
		json_parser jp;

		proof_assertion.put_member("dependencies", dependencies);
		json db_config = jp.create_object();
		json server_config = jp.create_object();
		server_config.put_member("DefaultUserName", "todd");
		server_config.put_member("DefaultUserPassword", "randomite");
		server_config.put_member("DefaultUserEmailAddress", "todd.bandrowsky@gmail.com");
		server_config.put_member("DefaultGuestTeam", "GuestTeam");
		db_config.put_member("Server", server_config);

		db.apply_config(db_config);

		relative_ptr_type database_location = db.create_database();

		login_success = true;

		json login_positive_request = R"(
{
	"UserName" : "todd",
	"Password" : "randomite"
}
)"_jobject;

		json login_negative_request = R"(
{
	"UserName" : "todd",
	"Password" : "reachio"
}
)"_jobject;

		json login_result;

		login_result = db.login_user(login_negative_request);

		if (login_result["Success"]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("able to login with bad account", __FILE__, __LINE__);
		}

		login_result = db.login_user(login_positive_request);

		if (not login_result["Success"]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("can't with good account", __FILE__, __LINE__);
		}

		json create_class_request = R"(
{
	"ClassName" : "Document",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Document Base Class",
	"Fields" : {			
		"DocumentName": "string",
		"DocumentDescription": "string",
		"DocumentUrl": "string"
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Encounter",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "ContactBase",
	"Fields" : {			
		"EncounterName": "string",
		"EncounterDescription": "string",
		"EncounterComments": "string",
		"EncounterDate": "datetime",
		"EncounterDocuments" : {
				"FieldType" : "array",
				"ElementClassNames" : [ "Document" ]
		}
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Contact",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A Person and how to connect with them, along with documentation for them",
	"Fields" : {			
		"ContactName": "string",
		"ContactDescription": "string",
		"ContactStreet1": "string",
		"ContactStreet2": "string",
		"ContactCity": "string",
		"ContactState": "string",
		"ContactZip": "string",
		"ClientPhone": "string",
		"Documents": "array",
		"Encounters": "array",
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Employee",
	"BaseClassName" : "Contact",
	"ClassDescription" : "A Person that works for you",
	"Fields" : {			
		"EmployeeTitle": "string",
		"EmployeeJobDescription": "string",
		"Documents": {
				"FieldType" : "array",
				"ElementClassNames" : [ "Document" ]
		},
		"DirectReports": {
				"FieldType" : "array",
				"ElementClassNames" : [ "Employee" ]
		}
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Property",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Base of all property",
	"Fields" : {			
		"PropertyCode",
		"PropertyName",
		"PropertyDescription",
		"PropertyValue"
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Furniture",
	"BaseClassName" : "Property",
	"ClassDescription" : "Furniture",
	"Fields" : {			
		"YearMade",
		"Manufacturer"
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Vehicle",
	"BaseClassName" : "Property",
	"ClassDescription" : "Cars and Trucks",
	"Fields" : {			
		"VehicleMake",
		"VehicleModel",
		"VehicleYear",
		"VehicleMiles"
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Electronics",
	"BaseClassName" : "Property",
	"ClassDescription" : "Computers, Game Consoles, Radios",
	"Fields" : {			
		"ElectronicBrand",
		"ElectronicYear"
	}
}
)"_jobject;


		create_class_request = R"(
{
	"ClassName" : "Building",
	"BaseClassName" : "Property",
	"ClassDescription" : "Base of all property",
	"Fields" : {			
		"ConstructionCode": "string",
		"OccupancyCode": "string",
		"Street1": "string",
		"Street2": "string",
		"City": "string",
		"State": "string",
		"Zip": "string",
		"Geocodes" : "string",
		"Contents" : {
			"FieldType"

		}
	}
}
)"_jobject;

		// now, with my login, it's the default, or "sa" login
		// so, let's see if we can create a class

		create_class_request = R"(
{
	"ClassName" : "Coverage",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Base of all coverages",
	"Fields" : {			
		"CoverageCode",
		"DamageName",
		"DamageDescription"
	}
}
)"_jobject;

		// now, with my login, it's the default, or "sa" login
// so, let's see if we can create a class

		create_class_request = R"(
{
	"ClassName" : "CoveredRisk",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Quotes",
	"Fields" : {			
		"Coverages" : {

		},
		"Properties" : {

		},
		"AttachmentPoint" : {

		},
		"Limit" : {

		},
		"Deductible" : {

		}
	}
}
)"_jobject;

		// now, with my login, it's the default, or "sa" login
// so, let's see if we can create a class

		create_class_request = R"(
{
	"ClassName" : "Quote",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Quotes",
	"Fields" : {			
		"Coverages" : {

		}
	}
}
)"_jobject;

		create_class_request = R"(
{
	"ClassName" : "Client",
	"BaseClassName" : "Contact",
	"ClassDescription" : "ContactBase",
	"Fields" : {
		"People" : {
			"FieldType":"array",
			"ElementClassNames" : ["Contact"],
		},
		"PolicyQuotes" : {
			"FieldType" : "array",
			"ElementClassNames" : ["PolicyQuote"],
		}
	}
}
)"_jobject;

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
