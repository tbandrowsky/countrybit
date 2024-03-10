#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

#include "corona-json-table.hpp"

namespace corona
{

	class corona_db_header_struct
	{
	public:
		int64_t			  object_id;
		relative_ptr_type classes_location;
		relative_ptr_type class_objects_location;
		relative_ptr_type forms_location;
		relative_ptr_type users_location;
		relative_ptr_type teams_location;
		relative_ptr_type objects_location;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
			class_objects_location = -1;
			forms_location = -1;
			users_location = -1;
			teams_location = -1;
			objects_location = -1;
		}
	};

	using corona_db_header = poco_node<corona_db_header_struct>;
	using class_method_key = std::tuple<std::string, std::string>;

	class corona_database
	{
		corona_db_header header;

		json_table classes;
		json_table class_objects;
		json_table forms;
		json_table users;
		json_table teams;
		json_table objects;

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
					else if (jp.is_object() || jp.is_array())
					{
						;
					}
					else
					{
						result = create_response(false, "Class field incorrect", member.second, 0);
					}
				}
			}

			co_return result;
		}

		database_transaction<json> check_object(json _token, json _object_definition)
		{
			json result;

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

			std::string class_name = _object_definition["ClassName"];

			json class_definition = co_await get_class(_token, _object_definition);

			if (class_definition["Success"])
			{
				json class_data = class_definition["Data"];
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
				result = class_definition;
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

		corona_database(file* _file)
			: database_file(_file),
			classes(_file, { "ClassName" }),
			class_objects(_file, { "ClassName", "ObjectId" }),
			forms(_file, { "FormName" }),
			objects(_file, { "ObjectId" }),
			users(_file, { "UserName" }),
			teams(_file, { "TeamName" })
		{

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
			header.data.forms_location = co_await forms.create();
			header.data.users_location = co_await users.create();
			header.data.teams_location = co_await teams.create();
			header.data.class_objects_location = co_await class_objects.create();
			header.data.objects_location = co_await objects.create();

			json_parser jp;

			co_await header.write(database_file);

			std::string object_base_class_string = R"(
{
	"ClassName" : "SysObjectBase",
	"ClassDescription" : "Base Class of All Objects",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"History" : "array"
	},
}
)";

			classes.put(object_base_class_string);

			std::string object_reference_class_string = R"(
{	
	"BaseClassName" : "SysObjectBase",
	"ClassName" : "SysObjectReference",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"DeepCopy" : "bool",
			"DeepGet" : "bool"
	},
}
)";

			classes.put(object_reference_class_string);

			co_return header_location;
		}

		database_transaction<json> create_user_password(json _token, std::string _user_name, std::string _new_password)
		{
			json result = create_response(false, "Could not log in");

			if (!check_token(_token, "login")) {
				co_return result;
			}

			json user_result = co_await get_user(_token, _user_name, { "UserName"} );
			if (user_result["Success"]) {
				if (user_result.has_member("Data")) {
					json_parser jp;
					int rand_code = rand() % 500000 + 100000;
					json user_data = user_result["Data"];

					auto jencrypt = jp.create_object("Password", _new_password);
					std::string attempt_encrypted = hash(jencrypt);
					user_data.put_member("PasswordEncrypted", attempt_encrypted);

					result = create_response(true, "Password created");
					co_await users.put(user_data);
				}
				else
				{
					result = create_response(false, "User had no data");
				}
			}
			else
			{
				result = create_response(false, "User not found");
			}
			co_return result;
		}

		database_transaction<json> login_user_password(json _token, std::string _user_name, std::string _password)
		{
			json result = create_response(false, "Could not log in" );

			if (!check_token(_token, "login")) {
				co_return result;
			}

			json user_result = co_await get_user(_token, _user_name, { "UserName", "LoginAttempts", "PasswordEncrypted" });

			if (user_result["Success"]) {
				if (user_result.has_member("Data")) {
					json_parser jp;

					auto user_data = user_result["Data"];

					if (!user_data.has_member("LoginAttempts"))
					{
						user_data.put_member("LoginAttempts", 0);
					}

					int attempt_count = user_data["LoginAttempts"];

					auto jencrypt = jp.create_object("Password", _password);
					std::string attempt_encrypted = hash(jencrypt);
					std::string pw_existing = user_data["PasswordEncrypted"];

					attempt_count++;					
					user_data.put_member("LoginAttempts", attempt_count);

					if (pw_existing == attempt_encrypted) {
						user_data.put_member("LoginPasswordSuccess", true);
						result = create_response(true, "Ok", user_data, 0);
					}
					else if (attempt_count > 3)
					{
						user_data.put_member("LoginPasswordSuccess", false);
						result = create_response(false, "Too many login attempts", user_data, 0);
					}
					else
					{
						user_data.put_member("LoginPasswordSuccess", false);
						result = create_response(false, "Nope, no login", user_data, 0);
					}

					co_await users.put(user_data);
				}
			}
			co_return result;
		}

		database_transaction<json> create_user_code(json _token, std::string _user_name)
		{
			json result = create_response(false, "Could not log in");

			if (!check_token(_token, "login")) {
				co_return result;
			}

			json user_result = co_await get_user(_token, _user_name, {"UserName"});
			if (user_result["Success"]) {
				if (user_result.has_member("Data")) {
					json_parser jp;
					json user_data = user_result["Data"];

					int rand_code = rand() % 500000 + 100000;
					std::string code = std::format("{0}", rand_code);

					user_data.put_member("LoginChallengeCode", code);
					result = create_response(true, "Challenge created");
					co_await users.put(user_data);
				}
				else
				{
					result = create_response(false, "User had no data");
				}
			}
			else 
			{
				result = create_response(false, "User not found");
			}
			co_return result;
		}

		database_transaction<json> login_user_code(json _token, std::string _user_name, std::string _user_code)
		{
			json result = create_response(false, "Could not log in");

			if (!check_token(_token, "login")) {
				co_return result;
			}

			json user_result = co_await get_user(_token, _user_name, { "UserName", "LoginAttempts", "LoginChallengeCode" });
			if (user_result["Success"]) {
				if (user_result.has_member("Data")) {

					json_parser jp;

					auto user_data = user_result["Data"];

					if (!user_data.has_member("LoginAttempts"))
					{
						user_data.put_member("LoginAttempts", 0);
					}

					int attempt_count = user_data["LoginAttempts"];

					attempt_count++;
					user_data.put_member("LoginAttempts", attempt_count);

					std::string existing_code = user_data["LoginChallengeCode"];

					if (existing_code == _user_code) {
						user_data.put_member("LoginChallengeCodeSuccess", true);
						user_data.erase_member("LoginChallengeCode");
						result = create_response(true, "Ok", user_data, 0);
					}
					else if (attempt_count > 3)
					{
						user_data.put_member("LoginChallengeCodeSuccess", false);
						result = create_response(false, "Too many login attempts", user_data, 0);
					}
					else 
					{
						user_data.put_member("LoginChallengeCodeSuccess", false);
						result = create_response(false, "Incorrect code", user_data, 0);
					}
					
					co_await users.put(user_data);
				}
			}
			co_return result;
		}

		bool form_name_available(json _token, std::string _name)
		{
			json_parser jp;
			json key = jp.create_object("FormName", _name);
			auto contains_task = forms.contains(key);
			bool answer = contains_task.wait();
			return answer;
		}

		bool user_name_available(json _token, std::string _name)
		{
			json_parser jp;
			json key = jp.create_object("UserName", _name);
			auto contains_task = users.contains(key);
			bool answer = contains_task.wait();
			return answer;
		}

		bool team_name_available(json _token, std::string _name)
		{
			json_parser jp;
			json key = jp.create_object("TeamName", _name);
			auto contains_task = teams.contains(key);
			bool answer = contains_task.wait();
			return answer;
		}

		bool class_name_available(json _token, std::string _name)
		{
			json_parser jp;
			json key = jp.create_object("TeamName", _name);
			auto contains_task = teams.contains(key);
			bool answer = contains_task.wait();
			return answer;
		}

		database_transaction<json> put_user(json _token, json _user_def)
		{
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot put user");
				co_return result;
			}

			result = create_response(true, "Ok");

			if (!_user_def.is_object())
			{
				result = create_response(false, "This is not an object", _user_def, 0);
				co_return result;
			}

			std::vector<std::string> required_fields = { "UserName"  };

			for (auto rf : required_fields) 
			{
				if (!_user_def.has_member(rf)) {
					result = create_response(false, "This is not a user", _user_def, 0);
					co_return result;
				}
			}

			co_await users.put(_user_def);

			result = create_response(true, "user updated", _user_def, 0);
			co_return result;
		}

		database_transaction<json> get_user(json _token, std::string _user_name, std::initializer_list<std::string> _include_fields)
		{
			json_parser jp;
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get user");
				co_return result;
			}

			result = create_response(true, "Ok");

			json key = jp.create_object("UserName", _user_name);
			key.set_natural_order();

			json result = co_await users.get(key, _include_fields);	

			result = create_response(true, "got user", result, 0);
			co_return result;
		}

		database_transaction<json> put_team(json _token, json _team_def)
		{
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot put team");
				co_return result;
			}

			result = create_response(true, "Ok");

			if (!_team_def.is_object())
			{
				result = create_response(false, "This is not an object", _team_def, 0);
				co_return result;
			}

			std::vector<std::string> required_fields = { "TeamName", "TeamDescription", "TeamMembers", "Grants" };

			for (auto rf : required_fields)
			{
				if (!_team_def.has_member(rf)) {
					result = create_response(false, "This is not a team", _team_def, 0);
					co_return result;
				}
			}

			json tm = _team_def["TeamMembers"];
			bool invalid_team = tm.any([this, _token](json& _item)->bool {
				bool invalid_user_name = user_name_available(_token, _item["UserName"]);
				});

			if (!invalid_team)
			{
				result = create_response(false, "Team missing members", _team_def, 0);
				co_return result;
			}

			json gm = _team_def["Grants"];

			tm.any([this, _token](json& _item)->bool {
				if (_item.is_object()) 
				{
					std::string grant_class_name = _item["ClassName"];
					return class_name_available(_token, _item["ClassName"]);
				}
				else if (_item.is_string()) 
				{
					return class_name_available(_token, _item);
				}
				});

			co_await teams.put(_team_def);

			result = create_response(true, "user updated", _team_def, 0);
			co_return result;

		}

		database_transaction<json> get_team(json _token, std::string _team_name)
		{
			json_parser jp;
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get team");
				co_return result;
			}

			result = create_response(true, "Ok");

			json key = jp.create_object("TeamName", _team_name);
			key.set_natural_order();

			json result = co_await teams.get(key);

			result = create_response(true, "got team", result, 0);
			co_return result;
		}

		database_transaction<json> get_class(json _token, std::string _class_name)
		{
			json_parser jp;
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot get class");
				co_return result;
			}

			result = create_response(true, "Ok");

			json key = jp.create_object("ClassName", _class_name);
			key.set_natural_order();

			json result = co_await classes.get(key);

			result = create_response(true, "got class", result, 0);
			co_return result;
		}

		database_transaction<json> get_class(json _token, json& _object)
		{
			json class_def;
			json key = _object.extract({ "ClassName" });
			json result;

			if (!check_token(_token, "user")) {
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

		database_transaction<json> put_class(json _token, json _class_definition)
		{
			json result;

			if (!check_token(_token, "user")) {
				result = create_response(false, "Cannot put class");
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

		database_transaction<json> get_derived_classes(json _token, json _class_definition)
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

		database_transaction<json> get_ancestor_classes(json _token, json _class_definition)
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

		database_transaction<bool> is_descendant_of(json _token, std::string _base_class, std::string _class_to_check )
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

		database_transaction<json> get_objects_by_class(json _token, json _object_definition)
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
					json obj = co_await get_object(_token, get_object_id);
					objects.append_element(obj);
				}
			}

			response = create_response(true, "Ok", objects, 0.0);
			co_return response;
		}

		database_transaction<json> create_object(json _token, std::string _class_name)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot create object");
				co_return result;
			}

			json new_object = jp.create_object();
			json class_key = jp.create_object( "ClassName", _class_name );
			json classdef_response = co_await get_class(_token, class_key);
			json response;

			if (classdef_response["Success"]) {
				json class_data = classdef_response["Data"];
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
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
							new_object.put_member(member.first, 0);
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

		database_transaction<json> put_object(json _token, json _object_definition)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
				json result = create_response(false, "Cannot create object");
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
								em.is_member("ClassName", "SysObjectReference") &&
								em.is_member("DeepGet", true)
								)
							{
								em.erase_member("Data");
							}
						}
					}
					else if (
						cm.is_member("ClassName", "SysObjectReference") &&
						cm.is_member("DeepCopy", true)
						)
					{
						cm.erase_member("Data");
					}
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

		database_transaction<json> get_object(json _token, json _object_key)
		{
			json_parser jp;

			if (!check_token(_token, "user")) {
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
					cm.is_member("DeepCopy", true)
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

			json object_copy = co_await get_object(_token, _object_key );

			if (!object_copy["Success"])
				co_return object_copy;

			json response;

			if (_fn(object_copy)) 
			{
				json new_object = object_copy["Data"];
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
				json result = create_response(false, "Cannot copy object");
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
								em.is_member("ClassName", "SysObjectReference") &&
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

