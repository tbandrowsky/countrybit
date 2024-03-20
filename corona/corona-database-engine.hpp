#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

#include "corona-json-table.hpp"

/*********************************************** 

Journal

3/14/2024 - Objectives
Create and list teams and users, 
Team permission checks for all objects,
Create 

3/15/2024 - log
Create and edit 

***********************************************/


namespace corona
{

	class corona_db_header_struct
	{
	public:
		int64_t			  object_id;
		relative_ptr_type classes_location;
		relative_ptr_type class_objects_location;
		relative_ptr_type objects_location;
		relative_ptr_type objects_by_name_location;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
			class_objects_location = -1;
			objects_location = -1;
			objects_by_name_location = -1;
		}
	};

	using corona_db_header = poco_node<corona_db_header_struct>;
	using class_method_key = std::tuple<std::string, std::string>;

	class corona_database
	{
		corona_db_header header;

		json_table classes;
		json_table class_objects;
		json_table objects;
		json_table objects_by_name;

		json schema;

		std::map<class_method_key, json_function_function> functions;

		file* database_file;

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

		database_transaction<json> create_class(json& _token, std::string _text)
		{
			json_parser jp;
			json j = jp.parse_object(_text);
			json checked = check_class(j);
			if (checked["Success"]) {
				classes.put(j);
			}
			co_return j;
		}

		json create_response(bool _success,
			std::string _message,
			json _payload,
			double _seconds
		)
		{
			json_parser jp;
			json j = jp.create_object();
			j.put_member("Success", _success);
			j.put_member("Message", _message);
			j.put_member("Data", _payload);
			j.put_member("Seconds", _seconds);
			return j;
		}

		json create_response(bool _success, std::string _message, std::string _object_name, double _seconds = 0.0)
		{
			json_parser jp;
			json j = jp.create_object();
			j.put_member("Success", _success);
			j.put_member("Message", _message);
			j.put_member("Seconds", _seconds);
			j.put_member("Name", _object_name);
			return j;
		}

		json create_response(bool _success, std::string _message, double _seconds = 0.0)
		{
			json_parser jp;
			json j = jp.create_object();
			j.put_member("Success", _success);
			j.put_member("Message", _message);
			j.put_member("Seconds", _seconds);
			return j;
		}

		database_transaction<json> check_class(json _class_definition)
		{
			json result;

			result = create_response(true, "Ok");

			if (!_class_definition.has_member("ClassName"))
			{
				result = create_response(false, "Class must have a name", _class_definition, 0);
			}

			std::string class_name = _class_definition["ClassName"];

			// here we are going to grab the ancestor chain for this class.

			if (_class_definition.has_member("BaseClassName"))
			{

				std::string base_class_name = _class_definition["BaseClassName"];

				json_parser jp;
				json class_key = jp.create_object("ClassName", base_class_name);

				json base_class_def = co_await classes.get(class_key);

				if (!base_class_def.is_object())
				{
					result = create_response(false, "Base class not found", _class_definition, 0);
				}

				json ancestors = base_class_def["Ancestors"];
				if (!ancestors.is_empty()) {
					ancestors.put_member(base_class_name, base_class_name);
					_class_definition.put_member("Ancestors", ancestors);
				}

				auto fields = base_class_def["Fields"].get_members();
				auto class_fields = _class_definition["Fields"];
				for (auto field : fields) {
					class_fields.put_member(field.first, field.second);
				}
			}
			else 
			{
				_class_definition.put_member_blob("Ancestors");
			}

			if (!_class_definition.has_member("ClassDescription"))
			{
				result = create_response(false, "Class must have a class description", _class_definition, 0);
			}

			if (!_class_definition.has_member("Fields") || !_class_definition["Fields"].is_object())
			{
				result = create_response(false, "Missing Fields", _class_definition, 0);
			}
			else
			{
				json fields_object = _class_definition["Fields"];
				auto members = fields_object.get_members();
				for (auto& member : members)
				{
					member.first;
					json jp = member.second;
					if (jp.is_string())
					{
						std::string field_type = jp.get_string();
						if (!allowed_field_types.contains(field_type))
						{
							result = create_response(false, "Bad field", fields_object, 0);
							co_return result;
						}
					}
					else if (jp.is_object())
					{
						std::string field_type = jp["FieldType"];
						if (!allowed_field_types.contains(field_type))
						{
							result = create_response(false, "Bad field", fields_object, 0);
							co_return result;
						}
						if (field_type == "array" || field_type == "object") {
							std::string field_classes = jp["AllowedClasses"];
						}
					}
					else
					{
						result = create_response(false, "Class field incorrect", member.second, 0);
					}
				}
			}

			co_return result;
		}

		database_method_transaction<json> check_object(json _token, json _object_definition)
		{
			json result;
			json_parser jp;

			result = create_response(true, "Ok");

			if (!_object_definition.is_object())
			{
				result = create_response(false, "This is not an object", _object_definition, 0);
				co_return result;
			}

			if (!_object_definition.has_member("ClassName"))
			{
				result = create_response(false, "Object must have a class name", _object_definition, 0);
				co_return result;
			}

			json key_boy = _object_definition.extract({ "ClassName" });
			json class_data = co_await classes.get(key_boy);

			if (!class_data.is_empty())
			{
				result.put_member("ClassDefinition", class_data);
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
				for (auto kv : members) {
					if (!_object_definition.has_member(kv.first)) {
						result = create_response(false, "Object missing field", kv.first, 0);
						co_return result;
					}
					auto obj_type = _object_definition[kv.first]->get_type_name();
					auto member_type = kv.second->get_type_prefix();
					if (member_type != obj_type) {
						result = create_response(false, "Object field is incorrect type", kv.first, 0);
						co_return result;
					}
				}
			}
			else
			{
				result = class_data;
			}

			int64_t object_id = _object_definition["ObjectId"];

			if (object_id > 0)
			{
				json_parser jpy;
				json key_object = jpy.create_object();

				key_object.put_member_i64("ObjectId", object_id);
				key_object.set_natural_order();

				json obj = objects.get(key_object);

				result.put_member("PreviousVersion", obj);
			}

			co_return result;
		}

		lockable header_lock;

		buffer get_crypto_buffer(BCRYPT_HANDLE _item, const wchar_t* _property_name)
		{
			DWORD bytes_length = {};
			DWORD result_bytes = {};
			NTSTATUS status;
			buffer new_buffer;

			status = ::BCryptGetProperty(_item, _property_name, (PUCHAR)&bytes_length, sizeof(bytes_length), &result_bytes, 0);
			if (!status) {
				new_buffer = buffer(bytes_length);
			}
			else
			{
				throw std::logic_error("Cannot get crypto property");
			}
			return new_buffer;
		}


		std::string hash(json& _src)
		{
			std::string signature;

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_HASH_HANDLE hash_handle = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

			if (algorithm)
			{
				status = BCryptCreateHash(algorithm, &hash_handle, nullptr, 0, nullptr, 0, 0);

				if (hash_handle)
				{
					std::string hash_data = _src.to_json_typed();
					status = BCryptHashData(hash_handle, (PUCHAR)hash_data.c_str(), hash_data.size(), 0);
					if (!status)
					{
						DWORD hash_length = {};
						DWORD count_bytes = {};
						status = ::BCryptGetProperty(hash_handle, BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(hash_length), &count_bytes, 0);

						if (hash_length)
						{
							buffer buff(hash_length);
							::BCryptFinishHash(hash_handle, (PUCHAR)buff.get_ptr(), hash_length, 0);

							signature = buff.to_hex();
						}
					}
				}
			}

			if (hash_handle)
			{
				BCryptDestroyHash(hash_handle);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return signature;
		}

		std::string encrypt(json& _src, std::string _pass_phrase, std::string _iv)
		{
			std::string cipher_text;
			std::string plain_text;

			plain_text = _src.to_json_typed();

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_KEY_HANDLE key = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_AES_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

			if (algorithm) 
			{
				buffer key_buffer = get_crypto_buffer(algorithm, BCRYPT_OBJECT_LENGTH);
				buffer iv_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);

				if (status = BCryptSetProperty(
					algorithm,
					BCRYPT_CHAINING_MODE,
					(PBYTE)BCRYPT_CHAIN_MODE_CBC,
					sizeof(BCRYPT_CHAIN_MODE_CBC),
					0))
				{
					goto cleanup;
				}

				// Generate the key from supplied input key bytes.
				if (status = BCryptGenerateSymmetricKey(
					algorithm,
					&key,
					key_buffer.get_uptr(),
					key_buffer.get_size(),
					(PBYTE)_pass_phrase.c_str(),
					_pass_phrase.size(),
					0))
				{
					goto cleanup;
				}

				if (key) 
				{
					DWORD cipher_text_size = {};

					if (status = BCryptEncrypt(
						key,
						(PUCHAR)plain_text.c_str(),
						plain_text.size(),
						NULL,
						iv_buffer.get_uptr(),
						iv_buffer.get_size(),
						NULL,
						0,
						&cipher_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					buffer cipher_text_buffer(cipher_text_size);

					if (status = BCryptEncrypt(
						key,
						(PUCHAR)plain_text.c_str(),
						plain_text.size(),
						NULL,
						iv_buffer.get_uptr(),
						iv_buffer.get_size(),
						cipher_text_buffer.get_uptr(),
						cipher_text_buffer.get_size(),
						&cipher_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					cipher_text = cipher_text_buffer.to_hex();
				}
			}


		cleanup:

			if (key)
			{
				BCryptDestroyKey(key);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}			

			return cipher_text;
		}

		std::string decrypt(std::string cipher_text, std::string _pass_phrase, std::string _iv)
		{
			std::string plain_text;

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_KEY_HANDLE key = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_AES_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

			if (algorithm)
				{
				buffer key_buffer = get_crypto_buffer(algorithm, BCRYPT_OBJECT_LENGTH);
				buffer iv_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);

				iv_buffer.set_buffer(_iv);

				if (status = BCryptSetProperty(
					algorithm,
					BCRYPT_CHAINING_MODE,
					(PBYTE)BCRYPT_CHAIN_MODE_CBC,
					sizeof(BCRYPT_CHAIN_MODE_CBC),
					0))
				{
					goto cleanup;
				}

				// Generate the key from supplied input key bytes.
				if (status = BCryptGenerateSymmetricKey(
					algorithm,
					&key,
					key_buffer.get_uptr(),
					key_buffer.get_size(),
					(PBYTE)_pass_phrase.c_str(),
					_pass_phrase.size(),
					0))
				{
					goto cleanup;
				}

				if (key)
				{
					DWORD plain_text_size = {};

					if (status = BCryptDecrypt(
						key,
						(PUCHAR)cipher_text.c_str(),
						cipher_text.size(),
						NULL,
						iv_buffer.get_uptr(),
						iv_buffer.get_size(),
						NULL,
						0,
						&plain_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					buffer plain_text_buffer(plain_text_size);

					if (status = BCryptDecrypt(
						key,
						(PUCHAR)cipher_text.c_str(),
						cipher_text.size(),
						NULL,
						iv_buffer.get_uptr(),
						iv_buffer.get_size(),
						plain_text_buffer.get_uptr(),
						plain_text_buffer.get_size(),
						&plain_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					plain_text = plain_text_buffer.get_ptr();
				}
			}

		cleanup:

			if (key)
			{
				BCryptDestroyKey(key);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return plain_text;
		}

		bool check(json& _src, std::string _hash)
		{
			std::string hash_check = hash(_src);
			return hash_check == _hash;
		}

		std::string get_pass_phrase()
		{
			return "This is a test pass phrase";
		}

		std::string get_iv()
		{
			return "This is a test iv";
		}

		json create_token(std::string _user_name, std::string _role, json _letter)
		{
			json_parser jp;

			json payload = jp.create_object();
			payload.put_member("Name", _user_name);
			payload.put_member("Role", _role);
			date_time expiration = date_time::utc_now() + this->token_life;
			payload.put_member("TokenExpires", expiration);
			payload.put_member("Data", _letter);
			std::string cipher_text = encrypt(payload, get_pass_phrase(), get_iv());

			json envelope = jp.create_object();
			envelope.put_member("Contents", payload);
			envelope.put_member("Signature", cipher_text);

			return envelope;
		}

		bool check_token(json& _token, std::string _expected_role)
		{
			if (!_token.is_object())
			{
				return false;
			}

			if (!_token.has_member("Contents"))
			{
				return false;
			}

			json payload = _token["Contents"];

			std::string cipher_text = encrypt(payload, get_pass_phrase(), get_iv());
			std::string signature = _token["Signature"];

			date_time current = date_time::utc_now();
			date_time expiration = (date_time)payload["TokenExpires"];

			if (current > expiration)
			{
				return false;
			}

			if (cipher_text != signature)
			{
				return false;
			}

			if (!payload.has_member("Role"))
			{
				return false;
			}

			std::string role = payload["Role"];
			std::string user = payload["Name"];

			if (_expected_role != role || 
				(role == "system" &&
				user == "system"))
			{
				return false;
			}

			return true;
		}

		json get_token_user(json _token)
		{
			json token_name = _token["Contents"].extract({ "Name" });
			return token_name;
		}

		table_transaction<json> acquire_object(json _object_key)
		{
			json_parser jp;
			json obj;

			if (_object_key.has_member("Name"))
			{
				json objects_by_name_key = jp.create_object();
				objects_by_name_key.copy_member("ClassName", _object_key);
				objects_by_name_key.put_member("Name", _object_key["Name"]);
				objects_by_name_key.set_compare_order({ "ClassName", "Name" });
				json name_id = co_await objects_by_name.get_first(objects_by_name_key);
				if (name_id.is_object()) {
					json object_key = jp.create_object();
					object_key.copy_member("ObjectId", name_id);
					object_key.set_natural_order();
					obj = co_await objects.get(object_key);
				}
			}
			else if (_object_key.has_member("ObjectId"))
			{
				json object_key = jp.create_object();
				object_key.copy_member("ObjectId", _object_key);
				object_key.set_natural_order();
				obj = co_await objects.get(object_key);
			}

			co_return obj;
		}

		table_transaction<json> get_linked_object(json _object_definition)
		{
			json_parser jp;
			json obj;

			if (_object_definition.is_member("ClassName", "SysReference"))
			{
				json object_key = jp.create_object();
				db_object_id_type object_id = (db_object_id_type)_object_definition["SysObjectId"];
				object_key.put_member("ObjectId", object_id);
				object_key.set_natural_order();
				obj = co_await objects.get(object_key);
			}
			else if (_object_definition.is_object())
			{
				obj = _object_definition;
			}

			co_return obj;
		}

		database_method_transaction<bool> has_class_permission(
			json _token,
			std::string _class_name,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			// check the token to make sure it is valid - this includes signature verification
			if (!check_token(_token, "user")) {
				co_return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_token_user(_token);
			user_key.put_member("ClassName", "SysUser");

			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			// Now go through the teams the user is a member of and check the grants to see if we can access this
			json teams_list = user["Teams"];

			bool granted = false;
			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team = co_await get_linked_object(item);

				if (team.is_member("ClassName", "Team")) {
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysClassGrant"))
						{
							bool has_permissions = grant["Permissions"][_permission];
							granted = has_permissions;
						}
					}
				}
			}
			co_return granted;
		}

		database_method_transaction<bool> has_object_permission(
			json _token,
			json _object_key,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			// check the token to make sure it is valid - this includes signature verification
			if (!check_token(_token, "user")) {
				co_return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_token_user(_token);
			user_key.put_member("ClassName", "SysUser");

			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			json teams_list = user["Teams"];

			bool granted = false;
			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team = co_await get_linked_object(item);

				if (team.is_member("ClassName", "SysTeam")) 
				{
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysObjectGrant"))
						{
							json filter = grant["ObjectFilter"];

							json obj = co_await objects.get(_object_key);

							bool has_matching_key = filter.compare(obj);

							bool has_permissions = grant["Permissions"][_permission];

							granted = has_permissions || has_matching_key;

							if (granted) {
								co_return granted;
							}
						}
					}
				}
			}
			co_return granted;
		}

		database_method_transaction<bool> has_object_permission(
			json _token,
			json _object,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			// check the token to make sure it is valid - this includes signature verification
			if (!check_token(_token, "user")) {
				co_return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_token_user(_token);
			user_key.put_member("ClassName", "SysUser");

			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			json teams_list = user["Teams"];

			bool granted = false;
			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team = co_await get_linked_object(item);

				if (team.is_member("ClassName", "SysTeam"))
				{
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysObjectGrant"))
						{
							json filter = grant["ObjectFilter"];

							bool has_matching_key = filter.compare(_object);

							bool has_permissions = grant["Permissions"][_permission];

							granted = has_permissions || has_matching_key;

							if (granted) {
								co_return granted;
							}
						}
					}
				}
			}
			co_return granted;
		}

		json create_login_token(std::string _user_name)
		{
			json_parser jp;
			json empty;

			return create_token(_user_name, "login", empty);
		}

		json create_user_token(std::string _user_name)
		{
			json_parser jp;
			json empty;

			return create_token(_user_name, "user", empty);
		}

		json create_system_token()
		{
			json_parser jp;
			json empty;

			return create_token("system", "system", empty);
		}

		table_transaction<db_object_id_type> get_next_object_id()
		{
			scope_lock hlock(header_lock);
			header.data.object_id++;
			co_await header.write(database_file);
			co_return header.data.object_id;
		}

	public:

		time_span token_life;

		// constructing and opening a database

		corona_database(file* _file)
			: database_file(_file),
			classes(_file, { "ClassName" }),
			class_objects(_file, { "ClassName", "ObjectId" }),
			objects(_file, { "ObjectId" }),
			objects_by_name(_file, { "ClassName", "Name", "ObjectId" })
		{
			token_life = time_span(1, time_models::hours);
		}

		database_transaction<relative_ptr_type> open_database(relative_ptr_type _header_location)
		{
			relative_ptr_type header_location = co_await header.read(database_file, _header_location);
			co_return header_location;
		}

		user_transaction<relative_ptr_type> create_database()
		{
			header.object_id = 1;
			relative_ptr_type header_location = co_await header.append(database_file);

			header.data.object_id = 1;
			header.data.classes_location = co_await classes.create();
			header.data.class_objects_location = co_await class_objects.create();
			header.data.objects_location = co_await objects.create();
			header.data.objects_by_name_location = co_await objects_by_name.create();

			json_parser jp;

			co_await header.write(database_file);

			co_await classes.put(R"(
{	
	"ClassName" : "SysObject",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"Active" : "bool"
	},
}
)");

			json test = classes.get(R"({"ClassName":"SysObject"})");
			if (test.is_empty()) {
				co_return null_row;
			}

			co_await classes.put(R"(
{	
	"ClassName" : "SysReference",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"DeepCopy" : "bool",
			"DeepGet" : "bool",
			"LinkObjectId" : "int64"
	},
}
)");

			test = classes.get(R"({"ClassName":"SysReference"})");
			if (test.is_empty()) {
				co_return null_row;
			}

			co_await classes.put(R"(
{	
	"ClassName" : "SysUser",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"FirstName" : "string",
			"LastName" : "string",
			"Name" : "string",
			"SSN" : "string",
			"Email" : "string",
			"Mobile" : "string",
			"Street" : "string",
			"City" : "string",
			"State" : "string",
			"Zip" : "string",
			"Teams" : {
				"FieldType" : "array",
				"AllowedClasses" : "SysTeam",
			},
			"CurrentObjectId" : "int64",
			"CurrentForm" : {
				"FieldType" : "object",
				"AllowedClasses" : "SysForm"
			},
			"CurrentToken" : "object"
	},
}
)");


			test = classes.get(R"({"ClassName":"SysUser"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_await classes.put(R"(
{	
	"ClassName" : "SysPermission",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Permissions flags",
	"Fields" : {			
			"Get" : "bool",
			"Put" : "bool",
			"Delete" : "bool",
			"Replace" : "bool"
	},
}
)");

			test = classes.get(R"({"ClassName":"SysPermission"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_await classes.put(R"(
{	
	"ClassName" : "SysMember",
	"BaseClassName" : "SysObect",
	"ClassDescription" : "Team member",
	"Fields" : {
			"Permissions" : 
			{
				"FieldType" : "object",					
				"AllowedClasses" : "SysPermission"
			}
			"GrantUser" : 
			{
				"FieldType" : "object",
				"AllowedClasses" : "SysUser"
			}
	}
}
)");

			test = classes.get(R"({"ClassName":"SysMember"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_await classes.put(R"(
{	
	"ClassName" : "SysGrant",
	"BaseClassName" : "SysObect",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"Permissions" : 
			{
					"FieldType" : "object",
					"AllowedClasses" : "SysPermission"
			},
			"GrantClassName" : "string"
	},
}
)");

			test = classes.get(R"({"ClassName":"SysGrant"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_await classes.put(R"(
{	
	"ClassName" : "SysClassGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to a class",
	"Fields" : {
			
	},
}
)");

			test = classes.get(R"({"ClassName":"SysClassGrant"})");
			if (test.is_empty()) {
				co_return null_row;
			}

			co_await classes.put(R"(
{	
	"ClassName" : "SysObjectGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"ObjectFilter" : "object"
	},
}
)");

			test = classes.get(R"({"ClassName":"SysObjectGrant"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_await classes.put(R"(
{	
	"ClassName" : "SysTeam",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A team",
	"NameIndex" : true,
	"Fields" : {			
			"Name" : "string",
			"Description" : "string",
			"Members" : {
				"FieldType":"array",
				"AllowedClasses": "SysMember"
			}
			"Grants" : {
				"FieldType":"array",
				"AllowedClasses": "SysGrant"
			}
	},
}
)");

			test = classes.get(R"({"ClassName":"SysObjectGrant"})");
			if (test.is_empty()) {
				co_return null_row;
			}


			co_return header_location;
		}

		database_transaction<json> edit_object(json _token, json _object_key)
		{
			json_parser jp;
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get edit object");
				co_return result;
			}

			json object_options = jp.create_object();

			object_options.copy_member("ObjectId", _object_key);
			object_options.put_member_object("Fields");
			json object_fields = object_options["Fields"];
			object_options.put_member_array("Edit");
			json edit_options = object_options["Edit"];

			json object_response = co_await get_object(_token, _object_key);
			if (object_response["Success"]) {
				json obj = object_response["Data"];
				object_options.put_member("Data", obj);
				json class_key = obj.extract({ "ClassName" });
				json class_response = co_await get_class(_token, class_key);
				if (class_response["Success"]) {
					json class_definition = class_response["Data"];
					object_options.put_member_object("ClassDefinition", class_definition);
					auto fields = class_definition["Fields"].get_members();
					for (auto field : fields) {
						if (field.second.is_object()) {
							std::string field_type = field.second["FieldType"];
							if (field_type == "array" || field_type == "object")
							{
								if (field.second.has_member("Goal")) 
								{
									json field_options = co_await get_goal_field_options(class_definition, obj, field.first);
									if (field_options.is_array()) {
										for (int i = 0; i < field_options.size(); i++) {
											edit_options.append_element(field_options.get_element(i));
										}
									}
								}
								else if (field.second.has_member("AllowedClasses")) 
								{
									json allowed = field.second["AllowedClasses"];
									if (allowed.is_string()) {
										json build_option = jp.create_object();
										build_option.put_member("ClassName", "SysCreateObject");
										build_option.put_member_i64("TargetObjectId", (int64_t)obj["ObjectId"]);
										build_option.put_member("TargetMemberName", field.first);
										build_option.put_member("TargetClassName", allowed);
										build_option.put_member("TargetClassDescription", allowed);
										edit_options.append_element(build_option);

										json build_option = jp.create_object();
										build_option.put_member("ClassName", "SysCreateSubClass");
										build_option.put_member("TargetBaseClassName", allowed);
										build_option.put_member("TargetClassDescription", allowed);
										edit_options.append_element(build_option);
									}
								}
							}
							else if (field.second.has_member("Choices"))
							{
								json choices = field.second["Choices"];
								if (choices.is_object()) 
								{
									std::string choice_class_name = choices["ChoiceClassName"];
									std::string choice_name_field = choices["NameField"];
									std::string choice_value_field = choices["ValueField"];
									json choice_filter = choices["ChoiceFilter"];

									json class_key = jp.create_object();
									class_key.put_member("ClassName", choice_class_name);
									json choice_list = co_await get_objects_by_class(_token, class_key, choice_filter);
									choices.put_member("Items", choice_list);
									object_fields.put_member(field.first, choices);
								}
							}
							else {
								object_fields.put_member(field.first, field.second);
							}
						}
						else {
							object_fields.put_member(field.first, field.second);
						}
					}
				}
				co_return create_response(true, "Ok", object_options, 0.0);
			}
			co_return create_response(false, "Sadly, this object eludes you.", object_options, 0.0);
		}

		database_method_transaction<json> get_goal_field_options(json class_def, json parent, std::string field_name)
		{
			json_parser jp;
			json build_options;
			build_options = jp.create_array();

			std::string step_field_name = field_name;
			json step_field_definition = class_def["Fields"][step_field_name];

			if (!step_field_definition.is_object())
			{
				json response = create_response(true, "Ok", build_options, 0.0);
				co_return response;
			}

			if (!step_field_definition.has_member("Goal"))
			{
				json response = create_response(true, "Ok", build_options, 0.0);
				co_return response;
			}

			json required_objects = step_field_definition["Goal"]["RequiredObjects"];
			std::string class_of_object_to_create = step_field_definition["Goal"]["CreateObjectClass"];
			std::string member_destination_of_object = step_field_definition["Goal"]["CreateObjectMember"];

			json actual_objects = parent[step_field_name];
			json token = create_system_token();

			if (required_objects.is_array()) {
				std::string object_to_add;

				json actual_object_classes = jp.create_object();

				for (int i = 0; i < actual_objects.size(); i++)
				{
					json object_data;
					json item = actual_objects.get_element(i);

					if (item.is_member("ClassName", "SysReference"))
					{
						object_data = item["Data"];
						int64_t object_id = item["LinkObjectId"];

						if (object_data.is_empty())
						{
							json_parser jp;
							json key = jp.create_object("ObjectId", object_id);
							object_data = co_await objects.get(key);
						}

						json build_option = jp.create_object();
						build_option.put_member("ClassName", "SysEditObject");
						build_option.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
						build_option.put_member("TargetMemberName", step_field_name);
						build_option.put_member_i64("SelectObjectId", object_id);
						build_options.append_element(build_option);
					}
					else
					{
						object_data = item;

						if (item.is_object()) 
						{
							json build_option = jp.create_object();
							build_option.put_member("ClassName", "SysEditObject");
							build_option.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
							build_option.put_member("TargetMemberName", step_field_name);
							build_option.put_member_i64("SelectObjectId", item["ObjectId"]);
							build_options.append_element(build_option);
						}
					}

					if (object_data.is_object()) {
						std::string object_class_name = object_data["ClassName"];
						actual_object_classes.put_member(object_class_name, object_class_name);
					}
				}

				json create_options = jp.create_object();
				bool satisfied = true;

				for (int i = 0; i < required_objects.size(); i++)
				{
					std::string required_class_name = required_objects.get_element(i);
					json required_class_key = jp.create_object("ClassName", required_class_name);
					json required_class = co_await classes.get(required_class_key);
					json required_class_description = required_class["ClassDescription"];
					json descendants = required_class["Descendants"];
					if (!descendants.any([this, actual_object_classes](json& _item) -> bool {
						std::string class_name = _item;
						return actual_object_classes.has_member(class_name);
						})) {
						json build_option = jp.create_object();
						build_option.put_member("ClassName", "SysCreateObject");
						build_option.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
						build_option.put_member("TargetMemberName", step_field_name );
						build_option.put_member("TargetClassName", required_class_name);
						build_option.put_member("TargetClassDescription", required_class["ClassDescription"]);
						build_options.append_element(build_option);

						json build_option = jp.create_object();
						build_option.put_member("ClassName", "SysCreateSubClass");
						build_option.put_member("TargetBaseClassName", required_class_name);
						build_option.put_member("TargetClassDescription", required_class_description);
						build_options.append_element(build_option);
						satisfied = false;
					}
				}

				// now, if we are satisfied, then we can create a new option
				if (satisfied) {
					json build_option = jp.create_object();
					build_option.put_member("ClassName", "SysCreateObject");
					build_option.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
					build_option.put_member("TargetMemberName", member_destination_of_object);
					build_option.put_member("TargetClassName", class_of_object_to_create);
					build_options.append_element(build_option);
					satisfied = false;
				}
			}

			json response = create_response(true, "ok", build_options, 0);
			co_return response;
		};

		database_method_transaction<json> get_classes(json _token)
		{
			json_parser jp;

			json result;
			json result_list;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Denied");
				co_return result;
			}

			auto result_task = classes.select_array([this, _token](int _index, json& _item) {
				json_parser jp;
				auto permission_task = has_class_permission(_token, _item["ClassName"], "Get");
				bool has_permission = permission_task.wait();

				if (has_permission) 
				{
					return _item;
				}
				else 
				{
					json empty = jp.create_object();
					return empty;
				}
				});

			result_list = result_task.wait();
			result = create_response(true, "Ok", result_list, 0);

			co_return result;
		}

		database_composed_transaction<json> get_class(json _token, std::string _class_name)
		{
			json_parser jp;
			json result;

			bool can_get_class = co_await has_class_permission(
				_token,
				_class_name,
				"Get");

			json key = jp.create_object("ClassName", _class_name);
			key.set_natural_order();

			json result = co_await classes.get(key);

			result = create_response(true, "Ok", result, 0);
			co_return result;
		}

		database_composed_transaction<json> get_class(json _token, json& _object)
		{
			json class_def;
			json key = _object.extract({ "ClassName" });
			json result;

			bool can_get_class = co_await has_class_permission(
				_token,
				key["ClassName"],
				"Get");

			if (!can_get_class) {
				result = create_response(false, "Denied", _object, 0.0);
				co_return result;
			}

			class_def = co_await classes.get(key);

			if (class_def.is_empty())
			{
				result = create_response(false, "Undefined", key, 0);
			}
			else 
			{
				result = create_response(true, "Ok", class_def, 0);
			}
			co_return result;
		}

		database_composed_transaction<json> put_class(json _token, json _class_definition)
		{
			json result;

			bool can_put_class = co_await has_class_permission(
				_token,
				_class_definition["ClassName"] ,
				"Put");

			if (!can_put_class) {
				result = create_response(false, "Denied", _class_definition, 0.0);
				co_return result;
			}

			result = check_class(_class_definition);
			json_parser jp;

			if (result["Success"]) {
				co_await classes.put(_class_definition);
				result = create_response(true, "Ok");

				auto ancestors = _class_definition["Ancestors"];
				if (ancestors.is_object()) {
					auto ancestor_classes = ancestors.get_members_raw();
					for (auto acp : ancestor_classes) {
						std::string acn = acp.first;
						json class_key = jp.create_object();
						class_key.put_member("ClassName", acn);
						auto ancestor_class = co_await classes.get(class_key);
						if (!ancestor_class.has_member("Descendants")) {
							ancestor_class.put_member_object("Descendants");
						}
						ancestor_class["Descendants"].put_member(acn, acn);
						co_await classes.put(ancestor_class);
					}
				}
			}
			co_return result;
		}

		database_method_transaction<bool> is_ancestor(json _token, std::string _base_class, std::string _class_to_check)
		{
			json_parser jp;

			json class_key = jp.create_object("ClassName", _base_class);
			class_key.set_natural_order();

			json class_obj = co_await classes.get(class_key);

			if (!class_obj.is_empty())
			{
				bool has_ancestor = class_obj["Ancestors"].has_member(_class_to_check);
				co_return has_ancestor;
			}

			co_return false;
		}

		database_method_transaction<bool> is_descendant(json _token, std::string _base_class, std::string _class_to_check )
		{
			json_parser jp;

			json class_key = jp.create_object("ClassName", _class_to_check);
			class_key.set_natural_order();

			json class_obj = co_await classes.get(class_key);
			
			if (!class_obj.is_empty()) 
			{
				bool has_ancestor = class_obj["Ancestors"].has_member(_base_class);
				co_return has_ancestor;
			}

			co_return false;
		}

		database_method_transaction<json> get_objects_by_class(json _token, json _class_key, json _filter)
		{
			json_parser jp;
			json response;

			std::string base_class_name = _class_key["ClassName"];

			if (!has_class_permission(_token, base_class_name, "Get"))
			{
				response = create_response(false, "Denied", 0.0);
				co_return response;
			}

			json objects = jp.create_array();

			json class_def = co_await classes.get(_class_key);
			json derived_classes = class_def["Descendants"];
			auto members = derived_classes.get_members();

			for (auto member : members) 
			{
				json_parser jp;
				json search_key = jp.create_object("ClassName", member.first);
				auto class_object_ids = co_await class_objects.select_array(search_key, [](int _index, json& _item)->json {
					return _item;
					});
				json get_object_id = jp.create_object("ObjectId", 0i64);

				for (db_object_id_type i = 0; i < class_object_ids.size(); i++)
				{
					db_object_id_type ri = class_object_ids.get_element(i)["ObjectId"];
					get_object_id.put_member("ObjectId", ri);
					get_object_id.set_natural_order();

					if (has_object_permission(_token, get_object_id, "Get"))
					{
						json objx = co_await get_object(_token, get_object_id);
						if (objx["Success"])
						{
							objects.append_element(objx["Data"]);
						}
					}
				}
			}

			response = create_response(true, "Ok", objects, 0.0);
			co_return response;
		}

		database_method_transaction<json> create_object(json _token, std::string _class_name)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot create object");
				co_return result;
			}

			if (!has_class_permission(_token, _class_name, "Get")) {
				json result = create_response(false, "Cannot get object");
				co_return result;
			}

			json class_key = jp.create_object("ClassName", _class_name);
			class_key.set_natural_order();

			json class_data = co_await classes.get(class_key);

			json response;

			if (class_data.is_object()) {
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
				json new_object = jp.create_object();
				new_object.put_member("ClassName", _class_name);

				for (auto& member : members)
				{
					json jpx = member.second;
					if (jpx.is_string())
					{
						std::string field_type = jpx.get_string();

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
							auto key = std::make_tuple(_class_name, member.first);
							if (functions.contains(key)) {
								new_object.put_member_function(member.first, functions[key]);
							}
							else 
							{
								std::string err_message = std::format("function {0} {1} not defined", _class_name, member.first);
								new_object.put_member(member.first, err_message);
							}
						}
					}
					else if (jpx.is_object())
					{
						new_object.put_member_object(member.first);
					}
					else if (jpx.is_array())
					{
						new_object.put_member_array(member.first);
					}
				}
				response = create_response(true, "Created", new_object, 0);
			}
			co_return response;
			
		}

		database_method_transaction<json> put_object(json _token, json _object_definition)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot create object");
				co_return result;
			}

			if (!has_object_permission(_token, _object_definition, "Put")) {
				json result = create_response(false, "Cannot put object");
				co_return result;
			}

			json result = co_await check_object(_token, _object_definition);

			if (result["Success"])
			{
				json obj = result["Data"];

				db_object_id_type object_id = -1;

				if (obj.has_member("ObjectId")) 
				{
					object_id = obj["ObjectId"];
				}
				else 
				{
					object_id = co_await get_next_object_id();
				}

				auto child_members = obj.get_members();

				for (auto child_member : child_members)
				{
					auto cm = child_member.second;
					if (cm.is_array())
					{
						for (int64_t index = 0; index < cm.size(); index++)
						{
							json em = cm.get_element(index);
							if (em.is_object() &&
								em.is_member("ClassName", "SysReference") &&
								em.is_member("DeepGet", true)
								)
							{
								em.erase_member("Data");
							}
						}
					}
					else if (
						cm.is_member("ClassName", "SysReference") &&
						cm.is_member("DeepCopy", true)
						)
					{
						cm.erase_member("Data");
					}
				}

				json previous_version = result["PreviousVersion"];
				std::string previous_name = previous_version["Name"];
				std::string new_name = _object_definition["Name"];

				if (previous_name != new_name)
				{
					json key_index = jp.create_object();
					key_index.put_member("ClassName", previous_version["ClassName"]);
					key_index.put_member("Name", previous_name);
					key_index.put_member("ObjectId", object_id);
					key_index.set_compare_order({ "Name", "ObjectId" });
					objects_by_name.erase(key_index);

					key_index.put_member("ClassName", _object_definition["ClassName"]);
					key_index.put_member("Name", new_name);
					objects_by_name.put(key_index);
				}

				relative_ptr_type put_result = co_await objects.put( _object_definition);

				json cobj = _object_definition.extract({ "ClassName", "ObjectId" });
				relative_ptr_type classput_result = co_await class_objects.put(cobj);

				result = create_response(true, "Ok");
			}
			else 
			{
				result = create_response(false, "Invalid object", _object_definition, 0);
			}

			co_return result;
		}

		database_method_transaction<json> get_object(json _token, std::string _class_name, std::string _object_name)
		{
			json_parser jp;
			json result;

			json obj_key = jp.create_object("Name", _object_name);
			json obj = co_await acquire_object(obj_key);

			if (!has_object_permission(_token, obj, "Get"))
			{
				result = create_response(false, "Denied", 0.0);
				co_return result;
			}

			result = create_response(true, "Ok", obj, 0.0);

			co_return result;
		}

		database_method_transaction<json> get_object(json _token, json _object_key)
		{
			json_parser jp;
			json result;

			json obj = co_await acquire_object(_object_key);

			if (!has_object_permission(_token, obj, "Get"))
			{
				result = create_response(false, "Denied", 0.0);
				co_return result;
			}

			result = create_response(true, "Ok", obj, 0.0);

			co_return result;
		}

		database_transaction<json> copy_object(json _token, json _object_key_src,
			std::function<bool(json _object_changes)> _fn)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Denied");
				co_return result;
			}

			json object_copy = co_await acquire_object(_object_key_src);

			if (!has_object_permission(_token, object_copy, "Get")) {
				json result = create_response(false, "Denied");
				co_return result;
			}

			if (!has_object_permission(_token, object_copy, "Put")) {
				json result = create_response(false, "Denied");
				co_return result;
			}

			json response;

			if (_fn(object_copy)) 
			{
				json new_object = object_copy.clone();
				db_object_id_type new_object_id = co_await get_next_object_id();
				new_object.put_member("ObjectId", new_object_id);

				auto child_members = new_object.get_members();

				for (auto child_member : child_members)
				{
					auto cm = child_member.second;
					if (cm.is_array()) 
					{
						for (int64_t index = 0; index < cm.size(); index++)
						{
							json em = cm.get_element(index);
							if (em.is_object() &&
								em.is_member("ClassName", "SysReference") &&
								em.is_member("DeepCopy", true)
								)
							{
								db_object_id_type old_id = cm["LinkObjectId"];
								json object_key = jp.create_object("ObjectId", old_id);
								json new_object = co_await copy_object(_token, object_key, [](json _changes) -> bool
									{
										return true;
									});
								em.put_member_i64("LinkObjectId", new_object["ObjectId"]);
							}
						}
					}
					else if (
						cm.is_member("ClassName", "SysReference") &&
						cm.is_member("DeepCopy", true)
						)
					{
						db_object_id_type old_id = cm["LinkObjectId"];
						json object_key = jp.create_object("ObjectId", old_id);
						json new_object = co_await copy_object(_token, object_key, [](json _changes) -> bool
							{
								return true;
							});
						cm.put_member_i64("LinkObjectId", new_object["ObjectId"]);
					}
				}

				json result = co_await put_object(_token, new_object);

				if (result["Success"]) {
					response = create_response(true, "Ok", result["Data"], 0);
				}
				else 
				{
					response = result;
				}
			}
			else 
			{
				response = create_response(true, "Ok, not copied");
			}

			co_return response;
		}

		database_transaction<json> delete_object(json _token, json _object_key)
		{
			json ret;
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot delete object");
				co_return result;
			}

			if (!has_object_permission(_token, _object_key, "Delete")) {
				json result = create_response(false, "Cannot delete object");
				co_return result;
			}

			json object_def = co_await objects.get(_object_key);

			ret = create_response(false, "Failed", 0.0);

			if (object_def.is_object()) {
				bool success = co_await class_objects.erase(object_def);
				if (success) {
					success = co_await objects.erase(_object_key);
					if (success) {
						ret = create_response(success, "Ok", 0.0);
					}
				}

				auto child_members = object_def.get_members();

				for (auto child_member : child_members)
				{
					auto cm = child_member.second;
					if (cm.is_array())
					{
						for (int64_t index = 0; index < cm.size(); index++)
						{
							json em = cm.get_element(index);
							if (em.is_object() &&
								em.is_member("ClassName", "SysReference") &&
								em.is_member("DeepCopy", true)
								)
							{
								db_object_id_type old_id = cm["LinkObjectId"];
								json object_key = jp.create_object("ObjectId", old_id);
								json new_object = co_await delete_object(_token, object_key);
							}
						}
					}
					else if (
						cm.is_member("ClassName", "SysReference") &&
						cm.is_member("DeepCopy", true)
						)
					{
						db_object_id_type old_id = cm["LinkObjectId"];
						json object_key = jp.create_object("ObjectId", old_id);
						json new_object = co_await delete_object(_token, object_key);
					}
				}

			}
			else 
			{
				ret = create_response(false, "Not found", _object_key, 0.0);
			}
			co_return ret;
		}

	};

	user_transaction<bool> test_database_engine(corona::application& _app)
	{
		file dtest = _app.create_file(FOLDERID_Documents, "corona_json_database_test.ctb");

		std::cout << "test_database_engine, thread:" << ::GetCurrentThreadId() << std::endl;

		corona_database db(&dtest);

		std::cout << "test_database_engine, create_database, thread:" << ::GetCurrentThreadId() << std::endl;

		auto create_database_task = db.create_database();
		create_database_task.wait();
	}
}

#endif

