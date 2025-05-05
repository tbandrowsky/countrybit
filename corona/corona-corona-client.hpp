#pragma once

#ifndef CORONA_CORONA_CLIENT_HPP
#define CORONA_CORONA_CLIENT_HPP

namespace corona
{

	class client_class 
	{

	protected:
		int64_t		class_id;
		std::string class_name;
		std::string class_description;
		std::string base_class_name;
		std::vector<std::string> table_fields;
		std::map<std::string, std::shared_ptr<field_interface>> fields;
		std::map<std::string, std::shared_ptr<index_interface>> indexes;
		std::map<std::string, bool> ancestors;
		std::map<std::string, bool> descendants;
		std::shared_ptr<sql_integration> sql;

		void copy_from(const class_interface* _src)
		{
			class_id = _src->get_class_id();
			class_name = _src->get_class_name();
			class_description = _src->get_class_description();
			base_class_name = _src->get_base_class_name();
			table_fields = _src->get_table_fields();
			auto new_fields = _src->get_fields();
			for (auto fld : new_fields) {
				fields.insert_or_assign(fld->get_field_name(), fld);
			}
			auto new_indexes = _src->get_indexes();
			for (auto idx : new_indexes) {
				indexes.insert_or_assign(idx->get_index_name(), idx);
			}
			ancestors = _src->get_ancestors();
			descendants = _src->get_descendants();

		}


	public:

		client_class()
		{
			class_id = null_row;
		}

		client_class(const class_interface* _src)
		{
			copy_from(_src);
		}
		client_class(const class_implementation& _src)
		{
			copy_from(&_src);
		};
		client_class(class_implementation&& _src)
		{
			copy_from(&_src);
		}
		client_class& operator = (const class_implementation& _src)
		{
			copy_from(&_src);
			return *this;
		}
		client_class& operator = (class_implementation&& _src)
		{
			copy_from(&_src);
			return *this;
		}

		virtual int64_t	get_class_id() 
		{
			return class_id;
		}

		virtual std::string get_class_name() const 
		{
			return class_name;
		}

		client_class& set_class_name(const std::string& _class_name)
		{
			class_name = _class_name;
			return *this;
		}

		virtual std::string get_class_description() const
		{
			return class_description;
		}

		client_class& set_class_description(const std::string& _class_description)
		{
			class_description = class_description;
			return *this;
		}

		virtual std::string get_base_class_name() const
		{
			return base_class_name;
		}

		client_class& set_base_class_name(const std::string& _base_class_name)
		{
			base_class_name = _base_class_name;
			return *this;
		}

		virtual std::vector<std::string> get_table_fields() const
		{
			return table_fields;
		}

		virtual std::map<std::string, bool>  const& get_descendants() const
		{
			return descendants;
		}

		virtual std::map<std::string, bool>  const& get_ancestors() const
		{
			return ancestors;
		}

		bool empty()
		{
			return class_name.empty();
		}

		virtual void init_validation()
		{
			for (auto& fld : fields) {
				fld.second->init_validation();
			}
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member(class_name_field, class_name);
			_dest.put_member("class_description", class_description);
			_dest.put_member("base_class_name", base_class_name);
			
			if (table_fields.size() > 0) {
				json jtable_fields = jp.create_array();
				for (auto tf : table_fields) {
					jtable_fields.push_back(tf);
				}
				_dest.put_member("table_fields", jtable_fields);
			}

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

			if (sql) {
				json jsql = jp.create_object();
				sql->get_json(jsql);
				_dest.put_member("sql", jsql);
			}
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src)
		{

			json jfields, jindexes, jancestors, jdescendants, jtable_fields;

			class_name = _src[class_name_field];
			class_description = _src["class_description"];
			base_class_name = _src["base_class_name"];

			jtable_fields = _src["table_fields"];

			table_fields.clear();
			if (jtable_fields.array()) {
				for (auto tf : jtable_fields) {
					table_fields.push_back((std::string)tf);
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
			else if (not jancestors.empty()) {
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "ancestors must be an array of strings";
				_errors.push_back(ve);
			}

			descendants.clear();
			jdescendants = _src["descendants"];
			if (jdescendants.array())
			{
				for (auto jdescendant : jdescendants)
				{
					std::string descendant = jdescendant;
					descendants.insert_or_assign(descendant, true);
				}
			}
			else if (not jdescendants.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "descendants must be an array of strings";
				_errors.push_back(ve);
			}

			fields.clear();
			jfields = _src["fields"];

			if (jfields.object()) {
				auto jfield_members = jfields.get_members();
				for (auto jfield : jfield_members) {
					std::shared_ptr<field_implementation> field = std::make_shared<field_implementation>();
					field->set_field_type(field_types::ft_none);
					if (jfield.second.object())
					{
						field->put_json(_errors, jfield.second);
					}
					else if (jfield.second.is_string())
					{
						auto fi = allowed_field_types.find(jfield.second);
						if (fi != std::end(allowed_field_types)) {
							field->set_field_type(fi->second);
						}
					}
					else
					{
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = field->get_field_name();
						ve.message = "Is not a valid field specification. Can either be a string or an object.";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}

					if (field->get_field_name().empty())
					{
						field->set_field_name(jfield.first);
					}

					if (field->get_field_type() != field_types::ft_none)
					{
						fields.insert_or_assign(field->get_field_name(), field);
					}
					else {
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = field->get_field_name();
						ve.message = "Invalid field type";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}
				}
			}
			else if (not jfields.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "fields must be an object.  each field is a member, with a field as the '\"field_name\" : \"field_type\"' or '\"field_name\" : { options }'";
				_errors.push_back(ve);
			}

			indexes.clear();
			jindexes = _src["indexes"];
			if (jindexes.object()) {
				auto jindex_members = jindexes.get_members();
				for (auto jindex : jindex_members) {
					std::shared_ptr<index_implementation> index = std::make_shared<index_implementation>();
					index->put_json(_errors, jindex.second);
					index->set_index_name(jindex.first);

					if (index->get_index_name().empty())
					{
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = index->get_index_key_string();
						ve.message = "Missing index name.";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}

					indexes.insert_or_assign(jindex.first, index);
				}
			}
			else if (not jindexes.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "indexes must be an object.";
				_errors.push_back(ve);
			}

			json jsql = _src["sql"];
			if (jsql.object()) {
				sql = std::make_shared<sql_integration>();
				sql->put_json(_errors, jsql);
				for (auto& mp : sql->mappings) {
					auto fi = fields.find(mp.corona_field_name);
					if (fi != fields.end()) {
						mp.field_type = fi->second->get_field_type();
						if (mp.string_size <= 0) {
							mp.string_size = 100;
						}
					}
				}
				std::vector keys = sql->primary_key;
				keys.insert(keys.end(), object_id_field);
				std::string backing_index_name = sql->sql_table_name + "_idx";
				std::shared_ptr<index_implementation> idx = std::make_shared<index_implementation>(backing_index_name, keys, nullptr);
				indexes.insert_or_assign(backing_index_name, idx);
			}

		}


		virtual std::shared_ptr<xtable> find_index(corona_database_interface* _db, json& _object)  const
		{
			std::shared_ptr<xtable> index_table;
			std::shared_ptr<index_interface> matched_index;
			int max_matched_key_count = 0;

			for (auto idx : indexes)
			{
				auto& keys = idx.second->get_index_keys();

				int matched_key_count = 0;

				for (auto ikey : keys)
				{
					if (not _object.has_member(ikey))
					{
						break;
					}
					else
					{
						matched_key_count++;
					}
				}

				if (matched_key_count > max_matched_key_count)
				{
					matched_index = idx.second;
					max_matched_key_count = matched_key_count;
				}
			}

			if (matched_index) {
				index_table = matched_index->get_xtable(_db);
			}

			return index_table;
		}


		virtual void put_field(std::shared_ptr<field_interface>& _new_field) 
		{
			fields.insert_or_assign(_new_field->get_field_name(), _new_field);
		}

		virtual std::shared_ptr<field_interface>		get_field(const std::string& _name)  const 
		{
			auto found = fields.find(_name);
			if (found != std::end(fields)) {
				return found->second;
			}
			return nullptr;
		}

		virtual std::vector<std::shared_ptr<field_interface>> get_fields()  const 
		{
			std::vector<std::shared_ptr<field_interface>> fields_list;
			for (auto fld : fields) {
				fields_list.push_back(fld.second);
			}
			return fields_list;
		}

		virtual std::shared_ptr<index_interface> get_index(const std::string& _name)  const 
		{
			auto found = indexes.find(_name);
			if (found != std::end(indexes)) {
				return found->second;
			}
			return nullptr;
		}

		virtual std::vector<std::shared_ptr<index_interface>> get_indexes()  const 
		{
			std::vector<std::shared_ptr<index_interface>> indexes_list;
			for (auto fld : indexes) {
				indexes_list.push_back(fld.second);
			}
			return indexes_list;
		}

	};

	class client_object_id {
	public:
		std::string class_name;
		int64_t		object_id;
	};

	class client_object {
	public:
		json data;

		client_object_id get_id()
		{
			client_object_id coi;
			coi.object_id = (int64_t)data[object_id_field];
			coi.class_name = data[class_name_field];
			return coi;
		}
	};

	class corona_class_response : public corona_client_response
	{
	public:

		std::vector<std::shared_ptr<client_class>> classes;
		std::vector<validation_error> errors;

		corona_class_response& operator = (http_params& _params)
		{
			classes.clear();
			errors.clear();

			corona_client_response::operator=(_params);

			if (data.array())
			{
				for (auto cls : data)
				{
					std::shared_ptr<client_class> new_class = std::make_shared<client_class>();
					new_class->put_json(errors, cls);
				}
			}

			return *this;
		}
	};

	class corona_client : public corona_client_interface
	{
	public:
		std::string host;
		int port;
		std::string base_path;
		std::string authorization_header;

		corona_client()
		{
			;
		}

		virtual ~corona_client()
		{
			;
		}

		virtual corona_client_response register_user(std::string user_name, std::string email, std::string password1, std::string password2) override
		{
			corona_client_response result;

			json_parser jp;
			http_client cc;

			authorization_header = "";

			std::string header = "Content-Type: application/json\r\n";

			json payload = jp.create_object();
			json data = jp.create_object();
			data.put_member("user_name", user_name);
			data.put_member("password1", password1);
			data.put_member("password2", password2);
			data.put_member("email", email);
			payload.put_member("data", data);

			std::string path = base_path + "/corona/login/createuser";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response confirm_user(std::string user_name, std::string confirmation_code) override
		{
			json_parser jp;

			corona_client_response result;

			http_client cc;

			authorization_header = "";

			std::string header = "Content-Type: application/json\r\n";

			json payload = jp.create_object();
			json data = jp.create_object();
			data.put_member("user_name", user_name);
			data.put_member("validation_code", confirmation_code);

			std::string path = base_path + "corona/login/confirmuser";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;

		}

		virtual corona_client_response send_user(std::string user_name) override
		{
			corona_client_response result;

			json_parser jp;
			http_client cc;

			authorization_header = "";

			std::string header = "Content-Type: application/json\r\n";

			json payload = jp.create_object();
			json data = jp.create_object();
			data.put_member("user_name", user_name);
			payload.put_member("data", data);

			std::string path = base_path + "/corona/login/senduser";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;

		}

		std::string user_name;
		std::string user_password;
		std::string token;

		virtual corona_client_response login(std::string _user_name, std::string _password) override
		{
			corona_client_response result;

			user_name = _user_name;
			user_password = _password;

			result = login();

			return result;
		}

		virtual corona_client_response login() override
		{
			corona_client_response result;

			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();
			data.put_member("user_name", user_name);
			data.put_member("password", user_password);
			payload.put_member("data", data);

			std::string path = base_path + "/corona/login/loginuser";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			if (result.success) 
			{
				authorization_header = "Authorization: Bearer " + result.data[token_field];
			}
			else 
			{
				authorization_header = "";
			}

			return result;
		}

		virtual corona_client_response set_password(std::string user_name, std::string validation_code, std::string password1, std::string password2) override
		{
			corona_client_response result;

			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();
			data.put_member("user_name", user_name);
			data.put_member("validation_code", validation_code);
			data.put_member("password1", password1);
			data.put_member("password2", password2);
			payload.put_member("data", data);

			std::string path = base_path + "/corona/login/passworduser";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response get_classes() override
		{
			corona_class_response result;

			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			std::string path = base_path + "/corona/classes/get";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response get_class(std::string class_name) override
		{
			corona_class_response result;

			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			payload.put_member("class_name", class_name);

			std::string path = base_path + "/corona/classes/get/details/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response put_class(json _class_definition) override
		{
			corona_class_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			payload.put_member("data", _class_definition);

			std::string path = base_path + "/corona/classes/put/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response put_class(std::shared_ptr<client_class>& _client)
		{
			corona_class_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();
			_client->get_json(data);
			payload.put_member("data", data);

			std::string path = base_path + "/corona/classes/put/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response create_object(std::string _class_name) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();

			data.put_member(class_name_field, _class_name);

			payload.put_member("data", data);

			std::string path = base_path + "/corona/objects/create/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}


		virtual corona_client_response edit_object(std::string _class_name, int64_t _object_id, bool _object_id_field) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();

			data.put_member(class_name_field, _class_name);
			data.put_member(object_id_field, _object_id);
			data.put_member("include_children", true);

			payload.put_member("data", data);

			std::string path = base_path + "/corona/objects/get/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response get_object(std::string _class_name, int64_t _object_id, bool _include_children) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();

			data.put_member(class_name_field, _class_name);
			data.put_member(object_id_field, _object_id);
			data.put_member("include_children", _include_children);

			payload.put_member("data", data);

			std::string path = base_path + "/corona/objects/get/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response put_object(json _object) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			payload.put_member("data", _object);

			std::string path = base_path + "/corona/objects/put/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response run_object(json _object) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			payload.put_member("data", _object);

			std::string path = base_path + "/corona/objects/run/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response delete_object(std::string _class_name, int64_t _object_id) override
		{
			corona_client_response result;
			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			json payload = jp.create_object();
			json data = jp.create_object();

			data.put_member(class_name_field, _class_name);
			data.put_member(object_id_field, _object_id);

			payload.put_member("data", data);

			std::string path = base_path + "/corona/objects/delete/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), payload, header.c_str());

			result = params;

			return result;
		}

		virtual corona_client_response query_objects(json _query) override
		{
			corona_client_response result;

			json_parser jp;
			http_client cc;

			std::string header = "Content-Type: application/json\r\n" + authorization_header;

			std::string path = base_path + "/corona/objects/query/";

			http_params params = cc.post(host.c_str(), port, path.c_str(), _query, header.c_str());

			result = params;

			return result;
		}

	};

}

#endif
