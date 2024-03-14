#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

#include "corona-json-table.hpp"

/*********************************************** 

Journal

3/14/2024 - Objectives
Create and list teams and users, 
Team permission checks for all objects,
Create 

***********************************************/


namespace corona
{

	class corona_db_header_struct
	{
	public:
		int64_t			  object_id;
		relative_ptr_type classes_location;
		relative_ptr_type class_objects_location;
		relative_ptr_type teams_location;
		relative_ptr_type objects_location;
		relative_ptr_type objects_by_name_location;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
			class_objects_location = -1;
			teams_location = -1;
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

			if (_class_definition.has_member("BaseClassName"))
			{
				std::string bc = _class_definition["BaseClassName"];

				json_parser jp;
				json class_key = jp.create_object("ClassName", bc);

				bool has_class = co_await classes.contains(class_key);

				if (!has_class)
				{
					result = create_response(false, "Base class not found", _class_definition, 0);
				}
			}

			if (!_class_definition.has_member("ClassName"))
			{
				result = create_response(false, "Class must have a name", _class_definition, 0);
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

		json create_token(std::string _user_name, std::string _role, json _letter)
		{
			json_parser jp;

			json payload = jp.create_object();
			payload.put_member("UserName", _user_name);
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
			std::string role;

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

			role = payload["Role"];

			if (_expected_role != role)
			{
				return false;
			}

			return true;
		}

		database_composed_transaction<bool> has_object_permission(
			json _token,
			json _user_key, 
			json _match_key,
			std::string _permission)
		{

			bool granted = false;

			json user;
			json sys_token = create_system_token();

			// a user key could either by name or by object, so we get our object

			if (_user_key.has_member("Name")) 
			{
				user = co_await get_object(sys_token, "SysUser", _user_key["Name"]);
			}
			else if (_user_key.has_member("ObjectId"))
			{
				user = co_await get_object(sys_token, _user_key);
			}

			json teams_list = user["Teams"];

			bool granted = false;
			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				if (item.is_object() &&
					item.is_member("ClassName", "SysObjectReference") &&
					item.is_member("DeepGet", true)
					)
				{
					json team = item["Data"];

					if (team.is_object()) {
						json team_grants = team["Grants"];

						for (int i = 0; i < team_grants.size(); i++)
						{
							json grant = team_grants.get_element(i);

							if (grant.is_member("ClassName", "SysObjectGrant"))
							{
								json filter = grant["ObjectFilter"];

								bool has_matching_key = co_await objects.any(filter, [_match_key](int _index, json& _item) -> bool {
									return true;
									});

								bool has_permissions = grant["Permissions"][_permission];

								granted = has_permissions || has_matching_key;
							}
						}
					}

				}
			}
			co_return granted;
		}

		database_composed_transaction<bool> has_class_permission(
			json _token,
			json _user_key,
			std::string _class_name,
			std::string _permission)
		{

			bool granted = false;

			json user;
			json sys_token = create_system_token();

			// a user key could either by name or by object, so we get our object

			if (_user_key.has_member("Name"))
			{
				user = co_await get_object(sys_token, "SysUser", _user_key["Name"]);
			}
			else if (_user_key.has_member("ObjectId"))
			{
				user = co_await get_object(sys_token, _user_key);
			}

			json teams_list = user["Teams"];

			bool granted = false;
			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				if (item.is_object() &&
					item.is_member("ClassName", "SysObjectReference") &&
					item.is_member("DeepGet", true)
					)
				{
					json team = item["Data"];

					if (team.is_object()) {
						json team_grants = team["Grants"];

						for (int i = 0; i < team_grants.size(); i++)
						{
							json grant = team_grants.get_element(i);

							if (grant.is_member("ClassName", "SysClassGrant"))
							{
								bool has_permissions = grant["Permissions"][_permission];
								if (has_permissions) {
									granted = has_permissions;
								}
							}
						}
					}

				}
			}
			co_return granted;
		}


		bool can_get_class(json& _token, json& _class_definition)
		{
			bool granted = false;
			json user_key = _class_definition.extract({ "ClassName" });

			granted = has_class_permission(_token, user_key, "Get");

			return granted;
		}

		bool can_put_class(json& _token, json& _class_definition)
		{
			bool granted = false;
			json user_key = _class_definition.extract({ "ClassName" });

			granted = has_permission(_token, &classes, user_key, "Put");

			return granted;
		}

		bool can_delete_class(json& _token, json& _class_definition)
		{
			bool granted = false;
			json user_key = _class_definition.extract({ "ClassName" });

			granted = has_permission(_token, &classes, user_key, "Delete");

			return granted;
		}

		bool can_subclass_class(json& _token, json& _class_definition)
		{
			bool granted = false;
			json user_key = _class_definition.extract({ "ClassName" });

			granted = has_permission(_token, &classes, user_key, "Subclass");

			return granted;
		}


		bool can_get_object(json& _token, json& _object_definition)
		{
			bool granted = false;
			json user_key = _object_definition.extract({ "ObjectId" });

			granted = has_permission(_token, &objects, user_key, "Get");

			return granted;
		}

		bool can_put_object(json& _token, json& _object_definition)
		{
			bool granted = false;
			json user_key = _object_definition.extract({ "ObjectId" });

			granted = has_permission(_token, &objects, user_key, "Put");

			return granted;
		}

		bool can_delete_object(json& _token, json& _object_definition)
		{
			bool granted = false;
			json user_key = _object_definition.extract({ "ObjectId" });

			granted = has_permission(_token, &objects, user_key, "Delete");

			return granted;
		}

		std::string get_pass_phrase()
		{
			return "This is a test pass phrase";
		}

		std::string get_iv()
		{
			return "This is a test iv";
		}

		json create_login_attempt_token(std::string _user_name)
		{
			json_parser jp;

			json payload = jp.create_object();
			payload.put_member("UserName", _user_name);
			payload.put_member("Authorization", "login");
			payload.put_member("TokenCreated", date_time::utc_now());
			std::string cipher_text = encrypt(payload, get_pass_phrase(), get_iv());

			json envelope = jp.create_object();
			envelope.put_member("Data", payload);
			envelope.put_member("Signature", cipher_text);
		}

		json create_user_token(std::string _user_name)
		{
			json_parser jp;

			json payload = jp.create_object();
			payload.put_member("UserName", _user_name);
			payload.put_member("Authorization", "user");
			payload.put_member("TokenCreated", date_time::utc_now());
			std::string cipher_text = encrypt(payload, get_pass_phrase(), get_iv());

			json envelope = jp.create_object();
			envelope.put_member("Data", payload);
			envelope.put_member("Signature", cipher_text);
		}

		json create_system_token()
		{
			json_parser jp;

			json payload = jp.create_object();
			payload.put_member("UserName", "system");
			payload.put_member("Authorization", "system");
			payload.put_member("TokenCreated", date_time::utc_now());
			std::string cipher_text = encrypt(payload, get_pass_phrase(), get_iv());

			json envelope = jp.create_object();
			envelope.put_member("Data", payload);
			envelope.put_member("Signature", cipher_text);
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

			co_await classes.put(R"(
{	
	"ClassName" : "SysStep",
	"ClassDescription" : "A step in a process",
	"NameIndex" : true,
	"Fields" : {			
			"Completed" : "bool",
			"CompleteDate" : "datetime"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysLoginStep",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysCodeSend",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"GeneratedCode" : "string",
			"EmailDeliveryId" : "string"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysCodeChallenge",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"ChallengeCode" : "string"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysPasswordSet",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"Text" : "string",
			"Completed" : "bool",
			"CompleteDate" : "datetime",
			"PasswordEncrypted1" : "string",
			"PasswordEncrypted2" : "string"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysPasswordChallenge",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"Text" : "string",
			"Completed" : "bool",
			"CompleteDate" : "datetime",
			"ChallengePassword" : "string"
	},
}
)");

			 co_await classes.put(R"(
{	
	"ClassName" : "SysTokenGenerated",
	"BaseClassName" : "SysLoginStep",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"Text" : "string",
			"Token" : "object"
	},
}
)");

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
				"AllowedClasses" : "SysTeam"
			}
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysPermission",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Permissions flags",
	"NameIndex" : true,
	"Fields" : {			
			"Get" : "bool",
			"Put" : "bool",
			"Delete" : "bool",
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysPermissionClass",
	"BaseClassName" : "SysPermission",
	"ClassDescription" : "Permissions flags",
	"NameIndex" : true,
	"Fields" : {			
			"Replace" : "bool"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysPermissionObject",
	"BaseClassName" : "SysPermission",
	"ClassDescription" : "Permissions flags",
	"NameIndex" : true,
	"Fields" : {			
			"Replace" : "bool"
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysMember",
	"BaseClassName" : "SysObect",
	"ClassDescription" : "Team member",
	"NameIndex" : true,
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

			co_await classes.put(R"(
{	
	"ClassName" : "SysClassGrant",
	"BaseClassName" : "SysObect",
	"ClassDescription" : "Grant to a class",
	"Fields" : {
			
	},
}
)");

			co_await classes.put(R"(
{	
	"ClassName" : "SysObjectGrant",
	"BaseClassName" : "SysObect",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"ObjectFilter" : "object"
	},
}
)");

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

			co_await classes.put(R"(
{
	"ClassName" : "SysControl",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Control Base",
	"Fields" : {			
			"X" : "string",
			"Y" : "string",
			"Height" : "string",
			"Width" : "string",
			"History" : "array",
			"Font" : "string",
			"Background" : "string",
			"Foreground" : "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysColumnCell",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "A cell in a column",
	"Fields" : {			
			"ColumnWidth" : "string",
			"Contents" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysControl"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysRowCell",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "A cell in a row",
	"Fields" : {			
			"CellWidth" : "string",
			"Contents" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysControl"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysAbsoluteCell",
	"BaseClassName" : "SysControlBase",
	"ClassDescription" : "A cell in an absolute",
	"Fields" : {			
			"Contents" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysControl"
			}
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysColumn",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "A column",
	"Fields" : {			
			"Columns" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysColumnCell"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysRow",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "A row on a form",
	"Fields" : {			
			"Rows" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysRowCell"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysAbsolute",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Control",
	"Fields" : {			
			"Items" : {
					"FieldType": "array",
					"AllowedClasses" :  "SysAbsoluteCell"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysTitle",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Title",
	"Fields" : {			
		"Text": "string"
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysSubTitle",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "SubTitle",
	"Fields" : {			
		"Text": "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysSection",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Section",
	"Fields" : {			
		"Text": "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysSubSection",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "SysSubSection",
	"Fields" : {			
		"Text": "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysParagraph",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Paragraph",
	"Fields" : {			
		"Text": "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysImage",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Image",
	"Fields" : {			
		"ImagePath": "string",
		"ImageName": "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysPoint",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Image",
	"Fields" : {			
		"X": "number",
		"Y": "number"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysDrawPoint",
	"BaseClassName" : "SysPoint",
	"ClassDescription" : "Draw to Point",
	"Fields" : {			
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysMovePoint",
	"BaseClassName" : "SysPoint",
	"ClassDescription" : "Move to Point",
	"Fields" : {			
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysSignature",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Image",
	"Fields" : {			
			"Shape" : {  
					"FieldType": "array", 	
					"AllowedClasses" :  "SysPoint",
					"UseReferences" : "False"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysSearchList",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "SearchList",
	"Fields" : {		
			"ClassToSearch" : "string",
			"ClassForOptions" : "string"
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysInputBase",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Input",
	"Fields" : {			
			"Columns" : {
					"FieldName": "string",
					"TooltipText" : "string",
					"Required": "bool"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysText",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Text Input",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "0",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysNumber",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Text Input",
	"Fields" : {			
			"Columns" : {
					"MinValue": "number",
					"MaxValue": "number",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysCurrency",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Text Input",
	"Fields" : {			
			"Columns" : {
					"MinValue": "number",
					"MaxValue": "number",
			}
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysInt64",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Number Input",
	"Fields" : {			
			"Columns" : {
					"MinValue": "number",
					"MaxValue": "number",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysString",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "String Input",
	"Fields" : {			
			"Columns" : {
					"MinLength": "number",
					"MaxLength": "number",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysName",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Text Input",
	"Fields" : {			
			"Columns" : {
					"MinLength": "number",
					"MaxLength": "number",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysDescription",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Text Input",
	"Fields" : {			
			"Columns" : {
					"MinLength": "number",
					"MaxLength": "number",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysSsn",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "SSN",
	"Fields" : {			
			"Columns" : {
					"MinLength": "number",
					"MaxLength": "10",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysEin",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "EIN",
	"Fields" : {			
			"Columns" : {
					"MinLength": "number",
					"MaxLength": "10",
			}
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysRealId",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Real Id",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "20",
			}
	},
}
)");


			co_await classes.put(R"(
{
	"ClassName" : "SysStreet",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Street",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "150",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysCity",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "City",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "150",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysState",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "State",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "150",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysZip",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Zip",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "10",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysEmail",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Email",
	"Fields" : {			
			"Columns" : {
					"MinLength": "0",
					"MaxLength": "100",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysCommandButton",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Button",
	"Fields" : {			
			"Columns" : {
					"ButtonText": "string",
					"ButtonIcon": "string",
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysLinkButton",
	"BaseClassName" : "SysControl",
	"ClassDescription" : "Button",
	"Fields" : {			
			"Columns" : {
					"ButtonText": "string",
					"ButtonIcon": "string"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysCheckbox",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Checkbox",
	"Fields" : {			
			"Columns" : {
					"ButtonText": "string",
					"ButtonIcon": "string"
			}
	},
}
)");

			co_await classes.put(R"(
{
	"ClassName" : "SysDropdownList",
	"BaseClassName" : "SysInput",
	"ClassDescription" : "Checkbox",
	"Fields" : {			
			"Columns" : {
					"ClassName": "string",
					"MemberValue": "string",
					"MemberDisplay": "string"
			}
	},
}
)");

			co_await classes.put(R"(
{	
	"BaseClassName" : "SysControl",
	"ClassName" : "SysForm",
	"ClassDescription" : "A form",
	"NameIndex" : true,
	"Fields" : {			
			"Name" : "string",
			"Controls" : {
				"FieldType" : "array",
				"FieldControlClass" : "SysControl",
				"FieldObjectClass" : "SysControl"
			}
	},
}
)");


			co_return header_location;
		}

		database_method_transaction<json> get_path(json _token, json _path)
		{
			json empty;

			co_return put_path(_token, _path, empty);
		}

		database_method_transaction<json> put_path(json _token, json _path, json _update)
		{
			json_parser jp;
			json response;

			if (_path.is_array()) 
			{
				json current_obj, *pcurrent_obj = &current_obj;
				json current_node, *pcurrent_node = &current_node;
				std::string current_class;
				std::string *pcurrent_class = &current_class;
				db_object_id_type object_id, *pobject_id = &object_id;
				object_id = -1;

				_path.for_each_element([this, pcurrent_obj, pcurrent_node, pcurrent_class, pobject_id](json & _path_element) -> void {
					std::string path_class_name = _path_element["ClassName"];
					if (path_class_name == "PathId")
					{
						*pobject_id = _path_element["PathObjectId"];
						json_parser jpx;
						json filter = jpx.create_object("ObjectId", *pobject_id);
						filter.set_natural_order();
						auto obj_task = objects.get(filter);
						*pcurrent_obj = obj_task.wait();
						*pcurrent_node = *pcurrent_obj;
					}
					else if (path_class_name == "PathName")
					{
						json class_name = _path_element["PathObjectClass"];
						std::string name = _path_element["PathObjectName"];
						json_parser jpx;
						json filter = jpx.create_object();
						filter.put_member("ClassName", class_name);
						filter.put_member("Name", name);
						filter.set_natural_order();
						auto obj_task = objects.get(filter);
						*pcurrent_obj = obj_task.wait();
						*pcurrent_node = *pcurrent_obj;
					}
					else if (path_class_name == "PathMember")
					{
						json path_member_name = _path_element["PathMember"];
						if (pcurrent_node->is_object()) {
							*pcurrent_node = pcurrent_node->get_member(path_member_name);
						}
					}
					else if (path_class_name == "PathElement")
					{
						json path_element_index = _path_element["PathElement"];				

						int64_t element_index;

						if (path_element_index.is_int64() || path_element_index.is_double()) {
							element_index = path_element_index.get_int64s();
						}
						else if ((std::string)path_element_index == "first")
						{
							element_index = 0;
						}
					
						if (pcurrent_node->is_object()) 
						{
							*pcurrent_node = pcurrent_node->get_element(element_index);
						}
					}
				});

				if (current_obj.is_object()) 
				{
					if (!_update.is_empty()) {
						current_node.assign_update(_update);
						co_await objects.put(current_obj);
					}
					response = create_response(true, "Saved", current_node, 0);
				}
				else if (!current_node.is_empty())
				{
					response = create_response(true, "Ok", current_node, 0);
				}
				else
				{
					response = create_response(false, "Not found", current_node, 0);
				}
			}

			co_return response;
		}

		database_method_transaction<json> get_classes(json _token)
		{
			json_parser jp;

			json result;
			json result_list;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			auto result_task = classes.select_array([](int _index, json& _item) {
				return _item;
				});

			result_list = result_task.wait();
			result = create_response(true, "Ok", result_list, 0);

			co_return result;
		}

		database_method_transaction<json> get_class(json _token, std::string _class_name)
		{
			json_parser jp;
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			json key = jp.create_object("ClassName", _class_name);
			key.set_natural_order();

			if (!can_get_class(_token, key)) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			json result = co_await classes.get(key);

			result = create_response(true, "got class", result, 0);
			co_return result;
		}

		database_method_transaction<json> get_class(json _token, json& _object)
		{
			json class_def;
			json key = _object.extract({ "ClassName" });
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			if (!can_get_class(_token, key)) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			class_def = co_await get_class(_token, key);

			if (class_def.is_empty())
			{
				result = create_response(false, "Undefined class", key, 0);
			}
			else 
			{
				result = create_response(true, "Ok", class_def, 0);
			}

			co_return result;
		}

		database_method_transaction<json> put_class(json _token, json _class_definition)
		{
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot put class");
				co_return result;
			}

			if (!can_put_class(_token, _class_definition)) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			result = check_class(_class_definition);
			json_parser jp;

			if (result["Success"]) {
				if (_class_definition.has_member("BaseClass")) {
					json class_key = jp.create_object();
					class_key.put_member("ClassName", (std::string)_class_definition["BaseClass"]);
					auto existing_class_task = get_class(_token, class_key);
					json existing_class = existing_class_task.wait();
					if (existing_class["Success"]) {
						auto existing_definition = existing_class["Data"];
						auto fields = existing_definition["Fields"].get_members();
						auto class_fields = _class_definition["Fields"];
						for (auto field : fields) {
							class_fields.put_member(field.first, field.second);
						}
					}
				}
				co_await classes.put(_class_definition);
				result = create_response(true, "Ok");
			}
			co_return result;
		}

		database_method_transaction<json> get_derived_classes(json _token, json _class_definition)
		{
			json_parser jp;
			json derived_classes = jp.create_object();

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot get derived classes");
				co_return result;
			}

			json class_ancestry_key = _class_definition.extract({ "BaseClassName" });

			auto so_task = classes.select_object(
				derived_classes,
				class_ancestry_key,
				[](int _index, json& _item) ->json {
					auto ret = _item.extract({ "ClassName" });
					return ret;
				},
				[](int _index, json& _item) ->json {
					std::string class_to_search = _item["ClassName"];
					json_parser jpx;
					json new_key = jpx.create_object("BaseClassName", class_to_search);
					return new_key;
				},
				{ "ClassName" }
			);

			so_task.wait();

			json result = create_response(true, "Ok", derived_classes, 0.0);

			co_return result;
		}

		database_method_transaction<json> get_ancestor_classes(json _token, json _class_definition)
		{
			json_parser jp;
			json ancestor_classes = jp.create_object();

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot get ancestor classes");
				co_return result;
			}

			json class_ancestry_key = _class_definition.extract({ "ClassName" });

			auto so_task = classes.select_object(
				ancestor_classes,
				class_ancestry_key,
				[](int _index, json& _item) ->json {
					auto ret = _item.extract({ "ClassName" });
					return ret;
				},
				[](int _index, json& _item) ->json {
					std::string class_to_search = _item["BaseClassName"];
					json_parser jpx;
					json new_key = jpx.create_object("ClassName", class_to_search);
					return new_key;
				},
				{ "ClassName" }
			);

			so_task.wait();

			json result = create_response(true, "Ok", ancestor_classes, 0.0);

			co_return result;
		}

		database_method_transaction<bool> is_descendant_of(json _token, std::string _base_class, std::string _class_to_check )
		{
			json_parser jp;

			json class_check = jp.create_object("ClassName", _class_to_check);
			auto ancestors_task = get_ancestor_classes(_token, class_check);
			json ancestors = ancestors_task.wait();
			
			if (ancestors["Success"]) 
			{
				json class_defs = ancestors["Data"];
				if (class_defs.has_member(_base_class)) {
					co_return true;
				}
			}

			co_return false;
		}

		database_method_transaction<json> get_objects_by_class(json _token, json _object_definition, json _filter)
		{
			json_parser jp;

			json response;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot get ancestor classes");
				co_return result;
			}

			json objects = jp.create_array();

			json class_def = co_await get_class(_token, _object_definition);
			json derived_classes = co_await get_derived_classes(_token, class_def);
			auto members = derived_classes.get_members();

			for (auto member : members) 
			{
				json_parser jp;
				json search_key = jp.create_object("ClassName", member.first);
				auto class_object_ids_task = class_objects.select_array(search_key, [](int _index, json& _item)->json {
					return _item;
					});
				json class_object_ids = class_object_ids_task.wait();
				json get_object_id = jp.create_object("ObjectId", 0i64);

				for (db_object_id_type i = 0; i < class_object_ids.size(); i++)
				{
					db_object_id_type ri = class_object_ids.get_element(i)["ObjectId"];
					get_object_id.put_member("ObjectId", ri);
					get_object_id.set_natural_order();

					if (can_get_object(_token, get_object_id))
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

			json class_key = jp.create_object("ClassName", _class_name);
			class_key.set_natural_order();

			if (!can_put_object(_token, class_key)) {
				json result = create_response(false, "Cannot get object");
				co_return result;
			}

			json classdef_response = co_await get_class(_token, class_key);
			json response;

			if (classdef_response["Success"]) {
				json class_data = classdef_response["Data"];
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

			if (!can_put_object(_token, _object_definition)) {
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

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot get object");
				co_return result;
			}

			result = create_response(false, "Not found", 0.0);

			json key_index = jp.create_object();
			key_index.put_member("ClassName", _class_name);
			key_index.put_member("Name", _object_name);
			key_index.set_compare_order({ "ClassName", "Name" });
			json index_entry = objects_by_name.get_first(key_index);

			if (!index_entry.is_empty()) 
			{
				json key_obj= jp.create_object();
				key_obj.put_member("ClassName", _class_name);
				key_obj.set_compare_order({ "ObjectId" });
				db_object_id_type obj_id = index_entry["ObjectId"];
				json obj_data = co_await get_object(_token, key_obj);
				result = create_response(true, "Ok", obj_data, 0.0);
			}

			co_return result;
		}

		database_method_transaction<json> get_object(json _token, json _object_key)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot get object");
				co_return result;
			}

			if (!can_get_object(_token, _object_key)) {
				json result = create_response(false, "Cannot get object");
				co_return result;
			}

			json response;
			json my_object = co_await objects.get(_object_key);

			auto child_members = my_object["Data"].get_members();

			for (auto child_member : child_members)
			{
				auto cm = child_member.second;
				if (cm.is_array())
				{
					for (int64_t index = 0; index < cm.size(); index++)
					{
						json em = cm.get_element(index);
						if (em.is_object() &&
							em.is_member("ClassName", "SysObjectReference") &&
							em.is_member("DeepGet", true)
							)
						{
							db_object_id_type old_id = cm["LinkObjectId"];
							json object_key = jp.create_object("ObjectId", old_id);
							json child_obj = co_await get_object(_token, object_key);
							if (child_obj["Success"]) {
								json data = child_obj["Data"];
								em.put_member("Data", data);
							}
						}
					}
				}
				else if (
					cm.is_member("ClassName", "SysObjectReference") &&
					cm.is_member("DeepGet", true)
					)
				{
					db_object_id_type old_id = cm["LinkObjectId"];
					json object_key = jp.create_object("ObjectId", old_id);
					json child_obj = co_await get_object(_token, object_key);
					if (child_obj["Success"]) {
						json data = child_obj["Data"];
						cm.put_member("Data", data);
					}
				}
			}

			co_return response;
		}

		database_transaction<json> copy_object(json _token, json _object_key,
			std::function<bool(json _object_changes)> _fn)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot copy object");
				co_return result;
			}

			if (!can_get_object(_token, _object_key)) {
				json result = create_response(false, "Cannot copy object");
				co_return result;
			}

			json object_copy = co_await get_object(_token, _object_key );

			if (!object_copy["Success"])
				co_return object_copy;

			json response;

			if (_fn(object_copy)) 
			{
				json new_object = object_copy["Data"];
				db_object_id_type new_object_id = co_await get_next_object_id();
				new_object.put_member("ObjectId", new_object_id);

				if (!can_put_object(_token, new_object)) {
					json result = create_response(false, "Cannot copy object");
					co_return result;
				}

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
								em.is_member("ClassName", "SysObjectReference") &&
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
						cm.is_member("ClassName", "SysObjectReference") &&
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

			if (!can_delete_object(_token, _object_key)) {
				json result = create_response(false, "Cannot delete object");
				co_return result;
			}

			json object_def = co_await objects.get(_object_key);

			ret = create_response(false, "Still there", 0.0);

			if (object_def["Success"]) {
				bool success = co_await class_objects.erase(object_def["Data"]);
				if (success) {
					success = co_await objects.erase(_object_key);
					if (success) {
						ret = create_response(success, "Ok", 0.0);
					}
				}

				auto child_members = object_def["Data"].get_members();

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
						cm.is_member("ClassName", "SysObjectReference") &&
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

		// create the database

		std::cout << "test_database_engine, create_database, thread:" << ::GetCurrentThreadId() << std::endl;

		auto create_database_task = db.create_database();
		create_database_task.wait();
	}
}

#endif

