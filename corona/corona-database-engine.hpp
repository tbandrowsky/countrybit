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

	class json_file_watcher 
	{
		std::string last_contents;
	public:
		std::string file_name;
		json		contents;

		json_file_watcher() 
		{
			;
		}

		file_transaction<relative_ptr_type> poll(application* _app)
		{
			json_parser jp;
			try {
				file f = _app->open_file(file_name, file_open_types::open_existing);
				if (f.success()) {
					auto fsize = f.size();
					buffer b(fsize + 1);
					auto result = co_await f.read(0, b.get_ptr(), fsize);
					if (result.success) {
						crypto crypter;
						if (b.is_safe_string()) {
							std::string s_contents = b.get_ptr();
							if (s_contents != last_contents) {
								json temp_contents = jp.parse_object(s_contents);
								if (!jp.parse_errors.size()) {
									last_contents = contents;
									contents = temp_contents;
									co_return true;
								}
							}
						}
					}
				}
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
			}
			co_return false;
		}
	};

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

		crypto crypter;

		json_file_watcher config_file;
		std::string send_grid_api_key;

		json_file_watcher schema_file;
		bool watch_polling;

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

		const std::string auth_login_user = "auth_login_user";
		const std::string auth_create_user = "auth_create_user";
		const std::string auth_send_login_confirmation_code = "auth_send_login_confirmation_code";
		const std::string auth_send_password_reset_code = "auth_send_password_reset_code";
		const std::string auth_receive_login_confirmation_code = "auth_receive_login_confirmation_code";
		const std::string auth_receive_reset_password_code = "auth_receive_reset_password_code";
		const std::string auth_general = "auth-general";
		
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

	public:

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
			json created_classes = jp.create_object();
			relative_ptr_type class_location;

			co_await header.write(database_file);
		
			json response = co_await create_class(R"(
{	
	"ClassName" : "SysObject",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string"
	}
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			json test = co_await classes.get(R"({"ClassName":"SysObject"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysObject", true);

			response = co_await create_class(R"(
{	
	"ClassName" : "SysReference",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "A reference to another object",
	"Fields" : {			
			"DeepCopy" : "bool",
			"DeepGet" : "bool",
			"LinkObjectId" : "int64"
	}
}
)");

			created_classes.put_member("SysReference", true);

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysReference"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			response = co_await create_class(R"(
{	
	"BaseClassName" : "SysObject",
	"ClassName" : "SysUser",
	"ClassDescription" : "A user",
	"NameIndex" : true,
	"Fields" : {			
			"ClassName" : "string",
			"FirstName" : "string",
			"LastName" : "string",
			"Name" : "string",
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
	}
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysUser"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysUser", true);


			response = co_await create_class(R"(
{	
	"BaseClassName" : "SysObject",
	"ClassName" : "SysLogin",
	"ClassDescription" : "A login of a user",
	"NameIndex" : true,
	"Fields" : {			
			"Name" : "string",
			"Password" : "string",
			"ConfirmCode" : "string",
			"LoginState" : "string",
			"CurrentObjectId" : "int64",
			"Status" : "string",
			"ConfirmationCode" : "string",
			"Password" : "string"
	}
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysLogin"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysLogin", true);





			response = co_await create_class(R"(
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

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysPermission"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysPermission", true);


			response = co_await create_class(R"(
{	
	"ClassName" : "SysMember",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Team member",
	"Fields" : {
			"Permissions" : 
			{
				"FieldType" : "object",					
				"AllowedClasses" : "SysPermission"
			},
			"GrantUser" : 
			{
				"FieldType" : "object",
				"AllowedClasses" : "SysUser"
			}
	}
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysMember"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysMember", true);

			response = co_await create_class(R"(
{	
	"ClassName" : "SysGrant",
	"BaseClassName" : "SysObject",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"Permissions" : 
			{
					"FieldType" : "object",
					"AllowedClasses" : "SysPermission"
			},
			"GrantClassName" : "string"
	}
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysGrant"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysGrant", true);

			response = co_await create_class(R"(
{	
	"ClassName" : "SysClassGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to a class"
}
)");

			if (!response["Success"]) {
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				std::cout << response.to_json() << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysClassGrant"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysClassGrant", true);

			class_location = co_await create_class(R"(
{	
	"ClassName" : "SysObjectGrant",
	"BaseClassName" : "SysGrant",
	"ClassDescription" : "Grant to an object",
	"Fields" : {
			"ObjectFilter" : "object"
	}
}
)");

			if (!response["Success"]) {
				std::cout << response.to_json() << std::endl;
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysObjectGrant"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysObjectGrant", true);

			response = co_await create_class(R"(
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
			},
			"Grants" : {
				"FieldType":"array",
				"AllowedClasses": "SysGrant"
			}
	}
}
)");

			if (!response["Success"]) {
				std::cout << response.to_json() << std::endl;
				std::cout << __FILE__ << " " << __LINE__ << ":classes.put failed." << std::endl;
				co_return null_row;
			}

			test = co_await classes.get(R"({"ClassName":"SysTeam"})");
			if (test.is_empty() || test.is_member("ClassName", "SysParseError")) {
				std::cout << __FILE__ << " " << __LINE__ << ":Could not find class after creation." << std::endl;
				co_return null_row;
			}

			created_classes.put_member("SysTeam", true);

			json gc = jp.create_object();
			json gcr = create_system_request( gc );

			json classes_array_response = co_await get_classes(gcr);
			json classes_array = classes_array_response["Data"];
			json classes_grouped = classes_array.group([](json& _item) -> std::string {
				return (std::string)_item["ClassName"];
				});

			bool missing = classes_array.any([classes_grouped](json& _item) {
				return !classes_grouped.has_member(_item["ClassName"]);
				});

			if (missing) {
				std::cout << __FILE__ << " " << __LINE__ << ":Class list returned from database missed some classes." << std::endl;
				co_return null_row;
			}
			else {
				std::cout << "database creation complete with :" << classes_grouped.to_json() << std::endl;
			}

			co_return header_location;
		}

private:

		database_transaction<json> create_class(std::string _text)
		{
			json_parser jp;
			json j = jp.parse_object(_text);
			json check_request = create_system_request(j);
			json checked = co_await check_class(check_request);
			json response;

			if (checked["Success"]) {
				json adjusted_class = checked["Data"];
				relative_ptr_type ptr = co_await classes.put(adjusted_class);
				if (ptr != null_row) {
					json key = adjusted_class.extract({ "ClassName" });
					json temp = co_await classes.get(key); 
					if (temp.is_empty()) {
						response = create_response(check_request, false, "save check failed", adjusted_class, 0.0);
						co_return response;
					}
					if (adjusted_class.has_member("Ancestors")) {
						auto ancestors = adjusted_class["Ancestors"];
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
					response = create_response(check_request, true, "Ok", adjusted_class, 0.0);
				}
				else {
					response = create_response(check_request, false, "didn't save", checked, 0.0);
				}
			}
			else 
			{
				response = checked;
			}
			co_return response;
		}

		database_transaction<json> check_class(json check_class_request)
		{
			json result;

			json class_definition = check_class_request["Data"];

			result = create_response(check_class_request, true, "Ok", class_definition, 0.0);

			if (!class_definition.has_member("ClassName"))
			{
				result = create_response(check_class_request, false, "Class must have a name", class_definition, 0.0);
			}

			std::string class_name = class_definition["ClassName"];

			// here we are going to grab the ancestor chain for this class.

			if (class_definition.has_member("BaseClassName"))
			{

				std::string base_class_name = class_definition["BaseClassName"];

				json_parser jp;
				json class_key = jp.create_object("ClassName", base_class_name);

				json base_class_def = co_await classes.get(class_key);

				if (!base_class_def.is_object())
				{
					result = create_response(check_class_request, false, "Base class not found", class_definition, 0.0);
				}

				json ancestors = base_class_def["Ancestors"];
				if (!ancestors.is_empty()) {
					ancestors.put_member(base_class_name, base_class_name);
					class_definition.put_member("Ancestors", ancestors);
				}
				else {
					class_definition.put_member_object("Ancestors");
				}

				auto fields = base_class_def["Fields"].get_members();
				if (!class_definition.has_member("Fields")) {
					class_definition.put_member_object("Fields");
				}
				auto class_fields = class_definition["Fields"];
				for (auto field : fields) {
					class_fields.put_member(field.first, field.second);
				}
			}
			else 
			{
				class_definition.put_member_object("Ancestors");
			}

			if (!class_definition.has_member("ClassDescription"))
			{
				result = create_response(check_class_request, false, "Class must have a description", class_definition, 0.0);
			}

			if (!class_definition.has_member("Fields") || !class_definition["Fields"].is_object())
			{
				result = create_response(check_class_request, false, "Class needs some fields", class_definition, 0.0);
			}
			else
			{
				json fields_object = class_definition["Fields"];
				auto members = fields_object.get_members();
				for (auto& member : members)
				{
					json_parser jpx;
					json err_field = jpx.create_object("Name", member.first);
					json jp = member.second;
					if (jp.is_string())
					{
						std::string field_type = jp.get_string();
						if (!allowed_field_types.contains(field_type))
						{
							result = create_response(check_class_request, false, "Bad field", err_field, 0.0);
							co_return result;
						}
					}
					else if (jp.is_object())
					{
						std::string field_type = jp["FieldType"];
						if (!allowed_field_types.contains(field_type))
						{
							result = create_response(check_class_request, false, "Bad field", err_field, 0.0);
							co_return result;
						}
						if (field_type == "array" || field_type == "object") {
							std::string field_classes = jp["AllowedClasses"];
						}
					}
					else
					{
						result = create_response(check_class_request, false, "Class field incorrect", err_field, 0.0);
					}
				}
				result = create_response(check_class_request, true, "Ok", class_definition, 0.0);
			}

			co_return result;
		}

		database_method_transaction<json> check_object(json check_object_request)
		{
			json result;
			json_parser jp;

			json object_definition = check_object_request["Data"];
			bool strict_enabled = check_object_request["Strict"];

			result = create_response(check_object_request, true, "Ok", object_definition, 0.0);

			if (!object_definition.is_object())
			{
				result = create_response(check_object_request, false, "This is not an object", object_definition, 0);
				co_return result;
			}

			if (!object_definition.has_member("ClassName"))
			{
				result = create_response(check_object_request, false, "Object must have class name", object_definition, 0);
				co_return result;
			}

			db_object_id_type object_id = -1;

			if (object_definition.has_member("ObjectId"))
			{
				object_id = object_definition["ObjectId"].get_int64s();
			}
			else
			{
				object_id = co_await get_next_object_id();
			}

			object_definition.put_member_i64("ObjectId", object_id);

			json key_boy = object_definition.extract({ "ClassName" });
			json class_data = co_await classes.get(key_boy);

			if (!class_data.is_empty())
			{
				result.put_member("ClassDefinition", class_data);
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
				for (auto kv : members) {
					json err_field = jp.create_object("Name", kv.first);
					if (object_definition.has_member(kv.first)) {
						std::string obj_type = object_definition[kv.first]->get_type_name();
						std::string member_type = kv.second;
						if (member_type != obj_type) {
							object_definition.change_member_type(kv.first, member_type);
						}
					}
				}
			}
			else
			{
				result = class_data;
			}

			co_return result;
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
			token.put_member("Name", _user_name);
			token.put_member("Authorization", _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			payload.put_member("Token", token);
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
			
			token.copy_member("Name", src_token);
			token.copy_member("Authorization", src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			payload.put_member("Token", token);
			payload.put_member("Success", _success);
			payload.put_member("Message", _message);
			payload.put_member("Data", _data);
			payload.put_member("Seconds", _seconds);
			return payload;
		}


		bool check_message(json& _message, std::vector<std::string> _authorizations)
		{
			json token = _message["Token"];

			if (!token.is_object())
			{
				return false;
			}

			if (!token.has_member("Signature"))
			{
				return false;
			}

			std::string signature = token["Signature"];
			token.erase_member("Signature");

			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());

			token.put_member("Signature", signature);

			date_time current = date_time::utc_now();
			date_time expiration = (date_time)token["TokenExpires"];

			if (current > expiration)
			{
				return false;
			}

			if (cipher_text != signature)
			{
				return false;
			}

			if (!token.has_member("Authorization"))
			{
				return false;
			}

			std::string authorization = token["Authorization"];
			std::string user = token["Name"];

			if (authorization == "System" && user == "System")
			{
				return true;
			}

			for (auto _authorization : _authorizations)
			{
				if (_authorization == authorization)
					return true;
			}

			return false;
		}

		json get_message_user(json _token)
		{
			if (_token.has_member("Token")) {
				_token = _token["Token"];
			}
			json token_name = _token.extract({ "Name" });
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
				objects_by_name_key.copy_member("Name", _object_key);
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
			if (!check_message(_token, { auth_general })) {
				co_return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_message_user(_token);
			if ((std::string)user_key["Name"] == "System") {
				co_return true;
			}

			user_key.put_member("ClassName", "SysUser");

			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			// Now go through the teams the user is a member of and check the grants to see if we can access this
			json teams_list = user["Teams"];

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
							if (has_permissions) {
								granted = has_permissions;
								co_return granted;
							}
						}
					}
				}
			}
			co_return granted;
		}

		database_method_transaction<bool> check_object_key_permission(
			json _message,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			json object_key = _message["Data"];

			// extract the user key from the token and get the user object
			json user_key = get_message_user(_message);
			user_key.put_member("ClassName", "SysUser");

			if ((std::string)user_key["Name"] == "System") {
				co_return true;
			}

			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			json teams_list = user["Teams"];

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

							json obj = co_await acquire_object(object_key);

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

		database_method_transaction<bool> check_object_permission(
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

			if ((std::string)user_key["Name"] == "System") {
				co_return true;
			}

			// extract the user key from the token and get the user object
			user_key.set_compare_order({ "ClassName", "Name" });
			user = co_await acquire_object(user_key);
			if (user.is_empty()) {
				co_return false;
			}

			json teams_list = user["Teams"];

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

							bool has_matching_key = filter.compare(object);

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

		table_transaction<db_object_id_type> get_next_object_id()
		{
			scope_lock hlock(header_lock);
			header.data.object_id++;
			co_await header.write(database_file);
			co_return header.data.object_id;
		}

		database_method_transaction<json> get_goal_field_options(json request, json class_def, json parent, std::string field_name)
		{
			json_parser jp;
			json build_options;
			build_options = jp.create_array();

			std::string step_field_name = field_name;
			json step_field_definition = class_def["Fields"][step_field_name];

			if (!step_field_definition.is_object())
			{
				json response = create_response(request, true, "Ok", build_options, 0.0);
				co_return response;
			}

			if (!step_field_definition.has_member("Goal"))
			{
				json response = create_response(request, true, "Ok", build_options, 0.0);
				co_return response;
			}

			json required_objects = step_field_definition["Goal"]["RequiredObjects"];
			std::string class_of_object_to_create = step_field_definition["Goal"]["CreateObjectClass"];
			std::string member_destination_of_object = step_field_definition["Goal"]["CreateObjectMember"];

			json actual_objects = parent[step_field_name];

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
						build_option.put_member("Function", "objects/create/");
						json build_message = jp.create_object();
						build_message.put_member("ClassName", required_class_name);
						build_message.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
						build_message.put_member("TargetMemberName", step_field_name);
						build_options.append_element(build_option);

						build_option = jp.create_object();
						build_message = jp.create_object();
						build_option.put_member("Function", "classes/put/");
						build_option.put_member("BaseClassName", required_class_name);
						build_option.put_member("ClassDescription", required_class_description);
						build_options.append_element(build_option);
						satisfied = false;
					}
				}

				// now, if we are satisfied, then we can create a new option
				if (satisfied) {
					json build_option = jp.create_object();
					build_option.put_member("Function", "objects/create/");
					json build_message = jp.create_object();
					build_message.put_member_i64("TargetObjectId", (int64_t)parent["ObjectId"]);
					build_message.put_member("TargetMemberName", member_destination_of_object);
					build_message.put_member("ClassName", class_of_object_to_create);
					build_options.append_element(build_option);
					satisfied = false;
				}
			}

			json response = create_response(request, true, "ok", build_options, 0);
			co_return response;
		};

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

		database_method_transaction<bool> is_descendant(json _token, std::string _base_class, std::string _class_to_check)
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

		void apply_config(json _config)
		{
			if (_config.has_member("SendGrid"))
			{
				json send_grid = _config["SendGrid"];
				send_grid_api_key = send_grid["ApiKey"];
				json server = _config["Server"];
				listen_point = server["ListenPoint"];
				default_user = server["DefaultUserName"];
				default_password = server["DefaultUserPassword"];
				default_email_address = server["DefaultUserEmailAddress"];
				default_guest_team = server["DefaultGuestTeam"];
			}
			else
			{
				std::cout << "Send grid api key not found in configuration" << std::endl;
			}
		}

		database_transaction<json> apply_schema(json _schema)
		{
			if (_schema.has_member("Classes"))
			{
				json class_array = _schema["Classes"];
				if (class_array.is_array())
				{
					for (int i = 0; i < class_array.size(); i++)
					{
						json class_definition = class_array.get_element(i);
						json put_class_request = create_system_request(class_definition);
						co_await create_class(put_class_request);
					}
				}
				else if (class_array.is_object())
				{
					json class_definition = class_array;
					json put_class_request = create_system_request(class_definition);
					co_await create_class(put_class_request);
				}
			}
			else
			{
				std::cout << "Classes not found in schema." << std::endl;
			}

			if (_schema.has_member("Users"))
			{
				json user_array = _schema["Users"];
				if (user_array.is_array())
				{
					for (int i = 0; i < user_array.size(); i++)
					{
						json user_definition = user_array.get_element(i);
						json put_user_request = create_system_request(user_definition);
						co_await create_user(put_user_request);
					}
				}
				else if (user_array.is_object())
				{
					json user_definition = user_array;
					json put_user_request = create_system_request(user_definition);
					co_await create_user(put_user_request);
				}
			}
			else
			{
				std::cout << "Send grid api key not found in configuration" << std::endl;
			}

			if (_schema.has_member("Objects"))
			{
				json object_array = _schema["Objects"];
				if (object_array.is_array())
				{
					for (int i = 0; i < object_array.size(); i++)
					{
						json object_definition = object_array.get_element(i);
						json put_object_request = create_system_request(object_definition);
						co_await create_user(put_object_request);
					}
				}
				else if (object_array.is_object())
				{
					json object_definition = object_array;
					json put_object_request = create_system_request(object_definition);
					co_await put_class(put_object_request);
				}
			}
			else
			{
				std::cout << "Send grid api key not found in configuration" << std::endl;
			}

		}

	public:

		std::string listen_point;
		std::string default_user;
		std::string default_password;
		std::string default_email_address;
		std::string default_guest_team;
		time_span token_life;

		// constructing and opening a database

		corona_database(application& _app, file* _file)
			: database_file(_file),
			classes(_file, { "ClassName" }),
			class_objects(_file, { "ClassName", "ObjectId" }),
			objects(_file, { "ObjectId" }),
			objects_by_name(_file, { "ClassName", "Name", "ObjectId" })
		{
			token_life = time_span(1, time_models::hours);	

			config_file.file_name = "config.json";
			schema_file.file_name = "schema.json";

			watch_polling = true;
		}

		void read_config(application* _app)
		{
			try {
				file_transaction<relative_ptr_type> config_tran = config_file.poll(_app);
				if (config_tran.wait() != null_row)
				{
					apply_config(config_file.contents);
				}
			}
			catch (std::exception exc)
			{
				std::cout << "File change exception:" << exc.what() << std::endl;
			}
		}

		void read_schema(application* _app)
		{
			try {
				file_transaction<relative_ptr_type> schema_tran = schema_file.poll(_app);
				if (schema_tran.wait() != null_row)
				{
					auto schema_task = apply_schema(schema_file.contents);
					schema_task.wait();
				}
			}
			catch (std::exception exc)
			{
				std::cout << "File change exception:" << exc.what() << std::endl;
			}
		}

		void watch_config_schema(application* _app)
		{
			if (watch_polling) 
			{
				threadomatic::run([this, _app]() -> void
					{
						try {
							file_transaction<relative_ptr_type> config_tran = config_file.poll(_app);
							if (config_tran.wait())
							{
								apply_config(config_file.contents);
							}
							file_transaction<relative_ptr_type> schema_tran = schema_file.poll(_app);
							if (schema_tran.wait())
							{
								auto schema_task = apply_schema(schema_file.contents);
								schema_task.wait();
							}
							::Sleep(1000);
							watch_config_schema(_app);
						}
						catch (std::exception exc)
						{
							std::cout << "File change exception:" << exc.what() << std::endl;
						}
					});
			}
		}

		database_transaction<relative_ptr_type> open_database(relative_ptr_type _header_location)
		{
			relative_ptr_type header_location = co_await header.read(database_file, _header_location);
			co_return header_location;
		}

		// this creates a user account, using an email and a phone number to send a confirmation code to.
		database_transaction<json> create_user(json create_user_request)
		{
			json_parser jp;
			json response;

			json data = create_user_request["Data"];

			std::string user_name = data["Name"];
			std::string user_class = data["ClassName"];
			bool user_exists = true;
			int attempt_count = 0;

			do 
			{
				json user_key = jp.create_object();
				user_key.put_member("ClassName", user_class);
				user_key.put_member("Name", user_name);
				user_key.set_compare_order({ "ClassName", "Name" });
				json existing_user_link = co_await objects_by_name.get(user_key);

				if (existing_user_link.is_object()) 
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

			std::string password1 = data["Password1"];
			std::string password2 = data["Password2"];

			if (password1 == password2 || password1.size()==0)
			{
				std::string hashed_pw = crypter.hash(password1);

				json create_user_params = data;
				create_user_params.put_member("ClassName", user_class);
				create_user_params.put_member("Name", user_name);
				json create_object_request = create_request(create_user_request, create_user_params);
				json user_result = co_await put_object(create_object_request);
				if (user_result["Success"]) {
					json new_user = user_result["Data"];

					json create_login_params = jp.create_object();
					create_login_params.put_member("ClassName", "SysLogin");
					create_login_params.put_member("Password", hashed_pw);
					create_login_params.put_member("Name", user_name);
					create_login_params.put_member("LoginState", "UserCreated");
					db_object_id_type objid = new_user["ObjectId"].get_int64();
					create_login_params.put_member("CurrentObjectId", objid);
					json put_object_request = create_request(create_user_request, create_login_params);
					json put_response = co_await put_object(put_object_request);

					if (put_response["Succeeded"])
					{
						response = create_response(user_name, auth_login_user, true, "User created", data, 0.0);
					}
				}
				else
				{
					response = create_response(create_user_request, false, "User not created", data, 0.0);
				}
			}
			else 
			{
				response = create_response(create_user_request, false, "Passwords don't match", data, 0.0);
			}

			co_return response;
		}

		// this starts a login attempt
		database_transaction<json> login_user(json _login_request)
		{
			json_parser jp;
			json response;

			json data = _login_request;
			std::string user_name = data["Name"];
			std::string user_password = data["Password"];
			std::string hashed_user_password;

			json user_key = jp.create_object();
			user_key.put_member("Name", user_name);
			user_key.put_member("ClassName", "SysLogin");
			json gor = create_request(_login_request, user_key);		

			json user_login = co_await get_object(gor);

			/*   
			
			consider checking that when a user is a new user, you cannot log into it.

			create_login_params.put_member("LoginState", "UserCreated");  */

			if (user_login["Success"]) 
			{
				json ul = user_login["Data"];
				std::string lstest = ul["LoginState"];
				if (lstest == "UserCreated") {
					response = create_response(_login_request, false, "Failed", jp.create_object(), 0.0);
					co_return response;
				}
				hashed_user_password = crypter.hash(user_password);
				std::string existing_hashed_password = ul["Password"];
				if (hashed_user_password == existing_hashed_password)
				{
					ul.put_member("LoginState", "PasswordAccepted");
					json porq = create_request(_login_request, ul);
					co_await put_object(porq);

					response = create_response(user_name, auth_send_login_confirmation_code, true, "Ok", data, 0.0);
				}
				else 
				{
					response = create_response(_login_request, false, "Failed", jp.create_object(), 0.0);
				}
			}
			else
			{
				response = create_response(_login_request, false, "Failed", jp.create_object(), 0.0);
			}

			co_return response;
		}

		std::string get_random_code()
		{
			std::string s_confirmation_code = "";
			int confirmation_code_digits = 6;
			char confirmation_code[32] = {};

			int rc = 0;
			int lc = 0;
			int i = 0;
			while (i < confirmation_code_digits && i < sizeof(confirmation_code))
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

		// this creates a confirmation code and sends it to the user
		database_transaction<json> send_login_confirmation_code(json _send_request)
		{
			json_parser jp;

			json response;

			if (!check_message(_send_request, { auth_send_login_confirmation_code }))
			{
				response = create_response(_send_request, false, "Cannot login", jp.create_object(), 0.0);
				co_return response;
			}

			json send_data = _send_request["Data"];
			std::string user_name = send_data["Name"];

			json obj_spec = jp.create_object();
			obj_spec.put_member("ClassName", "SysLogin");
			obj_spec.put_member("Name", user_name);
			json gor = create_request(_send_request, obj_spec);

			json user_login = co_await get_object(obj_spec);

			if (user_login["Success"])
			{
				json ul = user_login["Data"];

				std::string existing_state = ul["LoginState"];

				if (existing_state == "PasswordAccepted") {

					std::string confirmation_code = get_random_code();
					ul.put_member("ConfirmationCode", confirmation_code);
					ul.put_member("LoginState", "ConfirmationCodeSent");
					json por = create_request(_send_request, ul);
					co_await put_object(por);

					std::string code_email = "Your Countrybit confirmation code is " + confirmation_code;
					sendgrid_client sgc;
					sgc.api_key = send_grid_api_key;

					json gur = create_request(_send_request, ul);
					obj_spec.put_member("ClassName", "SysUser");
					json user = co_await get_object(gur);
					if (user["Success"])
					{
						sgc.send_email(user, "Countrybit Confirmation Code", code_email, "text/plain");
						response = create_response(user_name, auth_receive_login_confirmation_code, true, "Ok", send_data, 0.0);
					}
					else {
						response = create_response(_send_request, false, "Could not read user", send_data, 0.0);
					}
				}
				else {
					response = create_response(_send_request, false, "You forgot to do something", send_data, 0.0);
				}
			}
			else 
			{
				response = create_response(_send_request, false, "Could not send code", send_data, 0.0);
			}
			co_return response;
		}

		// this creates a user account, using an email and a phone number to send a confirmation code to.
		database_transaction<json> receive_login_confirmation_code(json _receive_request)
		{
			json_parser jp;

			json response;
			json recv_data = _receive_request["Data"];
			std::string user_name = recv_data["Name"];
			std::string confirmation_code = recv_data["ConfirmationCode"];

			if (!check_message(_receive_request, { auth_receive_login_confirmation_code }))
			{
				response = create_response(_receive_request, false, "Cannot login", jp.create_object(), 0.0);
				co_return response;
			}

			json gor = jp.create_object();
			gor.put_member("Name", user_name);
			gor.put_member("ClassName", "SysLogin");
			json gord = create_request(_receive_request, gor);
			json user_login = co_await get_object(gord);

			if (user_login["Success"])
			{
				json ul = user_login["Data"];
				std::string status = ul["LoginState"];
				std::string existing_code = ul["ConfirmationCode"];
				if (status == "ConfirmationCodeSent" && existing_code == confirmation_code) {
					ul.put_member("LoginState", "LoggedIn");
					json por = create_request(_receive_request, ul);
					co_await put_object(por);
					response = create_response(user_name, auth_general, true, "Ok", jp.create_object(), 0.0);
				}
				else 
				{
					response = create_response(_receive_request, false, "Yup, you forgot to do something.", jp.create_object(), 0.0);
				}
			}
			else 
			{
				response = create_response(_receive_request, false, "Could not get user", jp.create_object(), 0.0);
			}

			co_return response;
		}

		// this looks at an existing account and forces it to send a password email to itself.
		// this may only happen periodically.
		database_transaction<json> send_password_reset_code(json _send_request)
		{

			json response;

			json_parser jp;
			json data = _send_request["Data"];
			std::string user_name = data["Name"];
			std::string confirmation_code = data["ConfirmationCode"];

			if (!check_message(_send_request, { auth_receive_login_confirmation_code }))
			{
				response = create_response(_send_request, false, "Cannot login", jp.create_object(), 0.0);
				co_return response;
			}

			json gor = jp.create_object();
			gor.put_member("Name", user_name);
			gor.put_member("ClassName", "SysLogin");
			json gord = create_request(_send_request, gor);
			json user_login = co_await get_object(gord);

			if (user_login["Success"])
			{
				json ul = user_login["Data"];
				std::string status = ul["LoginState"];
				std::string existing_code = ul["ConfirmationCode"];
				if (status == "ConfirmationCodeSent" && existing_code == confirmation_code) {
					ul.put_member("LoginState", "ConfirmationCodeAccepted");

					json obj_spec = jp.create_object();
					obj_spec.put_member("ClassName", "SysLogin");
					obj_spec.put_member("Name", user_name);
					json gor = create_request(_send_request, obj_spec);
					json user_login = co_await get_object(obj_spec);

					json por = create_request(_send_request, ul);
					co_await put_object(por);
					sendgrid_client sgc;
					sgc.api_key = send_grid_api_key;

					obj_spec.put_member("ClassName", "SysUser");
					json gur = create_request(_send_request, obj_spec);
					json user = co_await get_object(gur);
					if (user["Success"])
					{
						std::string code_email = "Your Countrybit password reset code is " + confirmation_code;

						sgc.send_email(user, "Countrybit Password Reset Code", code_email, "text/plain");
						response = create_response(user_name, auth_receive_reset_password_code, true, "Ok", jp.create_object(), 0.0);
					}
					else 
					{
						response = create_response(_send_request, false, "Could not read user", jp.create_object(), 0.0);
					}
					response = create_response(_send_request, true, "Ok", jp.create_object(), 0.0);
				}
			}
			co_return response;
		}


		database_transaction<json> receive_reset_password_code(json _receive_request)
		{
			json_parser jp;
			json data = _receive_request["Data"];
			std::string user_name = data["Name"];
			std::string confirmation_code = data["ConfirmationCode"];

			json response = create_response(_receive_request, false, "Denied", data, 0.0);

			if (!check_message(_receive_request, { auth_receive_login_confirmation_code }))
			{
				response = create_response(_receive_request, false, "Cannot login", data, 0.0);
				co_return response;
			}

			json gor = jp.create_object();
			gor.put_member("ClassName", "SysLogin");
			gor.put_member("Name", user_name);
			json gorx = create_request(_receive_request, gor);
			json user_login = co_await get_object(gorx);

			if (user_login["Success"])
			{
				json ul = user_login["Data"];
				std::string status = ul["LoginState"];
				std::string existing_code = ul["ConfirmationCode"];
				if (status == "ConfirmationCodeSent" && existing_code == confirmation_code) {
					ul.put_member("LoginState", "ConfirmationCodeAccepted");
					std::string password1 = data["Password1"];
					std::string password2 = data["Password2"];
					if (password1 == password2) {
						std::string hashed_pw = crypter.hash(password1);
						ul.put_member("Password", hashed_pw);
						json por = create_request(_receive_request, ul);
						co_await put_object(por);
						response = create_response(user_name, auth_general, true, "Ok", jp.create_object(), 0.0);
					}
					else {
						response = create_response(_receive_request, false, "Passwords don't match", data, 0.0);
					}
				}
			}
			co_return response;
		}


		// this creates a user account, using an email and a phone number to send a confirmation code to.


		database_transaction<json> edit_object(json _edit_object_request)
		{
			json_parser jp;
			json result;

			json token = _edit_object_request["Token"];
			json object_key = _edit_object_request["ObjectId"];

			if (!check_message(_edit_object_request, { auth_general }))
			{
				result = create_response(_edit_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			json object_options = jp.create_object();
			object_options.copy_member("ObjectId", object_key);
			object_options.put_member_object("Fields");
			json object_fields = object_options["Fields"];
			object_options.put_member_object("Edit");
			json edit_options_root = object_options["Edit"];

			json get_response = co_await get_object(_edit_object_request);
			if (get_response["Success"]) {
				json obj = get_response["Data"];
				object_options.put_member("Data", obj);
				json class_key = obj.extract({ "ClassName" });
				class_key.put_member("Token", token);
				json class_response = co_await get_class(class_key);
				if (class_response["Success"]) {
					json class_definition = class_response["Data"];
					object_options.put_member_object("ClassDefinition", class_definition);
					auto fields = class_definition["Fields"].get_members();
					for (auto field : fields) {

						json edit_options = edit_options_root.put_member_array(field.first);

						if (field.second.is_object()) {
							std::string field_type = field.second["FieldType"];
							if (field_type == "array" || field_type == "object")
							{
								if (field.second.has_member("Goal")) 
								{
									json field_options = co_await get_goal_field_options(_edit_object_request, class_definition, obj, field.first);
									if (field_options["Success"]) {
										json field_options_array = field_options["Data"];
										for (int i = 0; i < field_options_array.size(); i++) {
											edit_options.append_element(field_options_array.get_element(i));
										}
									}
								}
								else if (field.second.has_member("AllowedClasses")) 
								{
									json allowed = field.second["AllowedClasses"];
									if (allowed.is_array()) {
										for (int i = 0; i < allowed.size(); i++) {
											json allowed_class = allowed.get_element(i);
											if (allowed_class.is_string()) {
												json build_option = jp.create_object();
												json build_message = jp.create_object();
												build_message.put_member_i64("TargetObjectId", (int64_t)obj["ObjectId"]);
												build_message.put_member("TargetMemberName", field.first);
												build_message.put_member("ClassName", allowed_class);
												build_message.put_member("ClassDescription", allowed_class);
												build_option.put_member("Data", build_message);
												build_option.put_member("Function", "/objects/create/");
												edit_options.append_element(build_option);

												build_option = jp.create_object();
												json build_message = jp.create_object();
												build_message.put_member("BaseClassName", allowed_class);
												build_option.put_member("Data", build_message);
												build_option.put_member("Function", "/classes/create/");
												edit_options.append_element(build_option);
											}
										}
									}
									else if (allowed.is_string()) {
										json build_option = jp.create_object();
										json build_message = jp.create_object();
										build_message.put_member_i64("TargetObjectId", (int64_t)obj["ObjectId"]);
										build_message.put_member("TargetMemberName", field.first);
										build_message.put_member("ClassName", allowed);
										build_message.put_member("ClassDescription", allowed);
										build_option.put_member("Function", "/objects/create/");
										build_option.put_member("Data", build_message);
										edit_options.append_element(build_option);

										build_option = jp.create_object();
										json build_message = jp.create_object();
										build_message.put_member("BaseClassName", allowed);
										build_option.put_member("Data", build_message);
										build_option.put_member("Function", "/classes/create/");
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
									class_key.put_member("Token", token);
									class_key.put_member("Filter", token);
									json choice_list = co_await query_class(class_key);
									if (choice_list["Success"]) 
									{
										json data = choice_list["Data"];
										choices.put_member("Items", data );
									}
									else 
									{
										choices.put_member_array("Items");
									}
									object_fields.put_member(field.first, field.second);
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
				co_return create_response(_edit_object_request, true, "Ok", object_options, 0.0);
			}
			co_return create_response(_edit_object_request, false, "Sadly, this object eludes you.", object_options, 0.0);
		}


		database_method_transaction<json> get_classes(json get_classes_request)
		{
			json_parser jp;

			json result;
			json result_list;
			
			if (!check_message(get_classes_request, { auth_general }))
			{
				result = create_response(get_classes_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			result_list = co_await classes.select_array([this, get_classes_request](int _index, json& _item) {
				json_parser jp;
				auto permission_task = has_class_permission(get_classes_request, _item["ClassName"], "Get");
				bool has_permission = permission_task.wait();

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

			std::cout << "result_list::" << result_list.to_json() << std::endl;

			result = create_response(get_classes_request, true, "Ok", result_list, 0);

			co_return result;
		}

		database_composed_transaction<json> get_class(json get_class_request)
		{
			json_parser jp;
			json result;

			if (!check_message(get_class_request, { auth_general }))
			{
				result = create_response(get_class_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			json token = get_class_request["Token"];
			std::string class_name = get_class_request["ClassName"];

			bool can_get_class = co_await has_class_permission(
				token,
				class_name,
				"Get");

			json key = jp.create_object("ClassName", class_name);
			key.set_natural_order();

			result = co_await classes.get(key);

			result = create_response(get_class_request, true, "Ok", result, 0);
			co_return result;
		}

		database_composed_transaction<json> create_class(json put_class_request)
		{
			json result;
			json_parser jp;

			if (!check_message(put_class_request, { auth_general }))
			{
				result = create_response(put_class_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			json token = put_class_request["Token"];
			json class_definition = put_class_request["Data"];

			bool can_put_class = co_await has_class_permission(
				token,
				class_definition["ClassName"],
				"Put");

			if (!can_put_class) {
				result = create_response(put_class_request, false, "Denied", class_definition, 0.0);
				co_return result;
			}

			std::string class_name = class_definition["ClassName"];
			json class_key = jp.create_object();
			class_key.put_member("ClassName", class_name);
			class_key.set_compare_order({ "ClassName" });
			json class_exists = classes.get(class_key);
			if (class_exists.is_object()) {
				result = create_response(put_class_request, false, "Class already exists", class_definition, 0.0);
				co_return result;
			}

			json checked = co_await check_class(class_definition);
			if (checked["Success"]) {
				json adjusted_class = checked["Data"];
				relative_ptr_type ptr = co_await classes.put(adjusted_class);
				if (ptr != null_row) {
					auto ancestors = adjusted_class["Ancestors"];
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
					result = create_response(put_class_request, true, "Ok", adjusted_class, 0.0);
				}
				else {
					result = create_response(put_class_request, false, "class not written", adjusted_class, 0.0);
				}
			}
			else
			{
				result = create_response(put_class_request, false, "class failed checks", checked, 0.0);
			}
			co_return result;
		}

		database_composed_transaction<json> put_class(json put_class_request)
		{
			json result;
			json_parser jp;

			if (!check_message(put_class_request, { auth_general }))
			{
				result = create_response(put_class_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			json token = put_class_request["Token"];
			json class_definition = put_class_request["Data"];

			bool can_put_class = co_await has_class_permission(
				token,
				class_definition["ClassName"] ,
				"Put");

			if (!can_put_class) {
				result = create_response(put_class_request, false, "Denied", class_definition, 0.0);
				co_return result;
			}

			json checked = co_await check_class(class_definition);
			if (checked["Success"]) {
				json adjusted_class = checked["Data"];
				relative_ptr_type ptr = co_await classes.put(adjusted_class);
				if (ptr != null_row) {
					auto ancestors = adjusted_class["Ancestors"];
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
					result = create_response(put_class_request, true, "Ok", adjusted_class, 0.0);
				}
				else {
					result = create_response(put_class_request, false, "class not written", adjusted_class, 0.0);
				}
			}
			else 
			{
				result = create_response(put_class_request, false, "class failed checks", checked, 0.0);
			}
			co_return result;
		}

		database_method_transaction<json> query_class(json query_class_request)
		{
			json_parser jp;
			json response;

			if (!check_message(query_class_request, { auth_general }))
			{
				response = create_response(query_class_request, false, "Denied", jp.create_object(), 0.0);
				co_return response;
			}

			json token = query_class_request["Token"];
			json base_class_name = query_class_request["ClassName"];
			json filter = query_class_request["Filter"];


			if (!has_class_permission(token, base_class_name, "Get"))
			{
				response = create_response(query_class_request, false, "Denied", jp.create_object(), 0.0);
				co_return response;
			}

			json objects = jp.create_array();

			json class_def = co_await classes.get(query_class_request);
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
				get_object_id.put_member("Token", token);

				for (db_object_id_type i = 0; i < class_object_ids.size(); i++)
				{
					db_object_id_type ri = class_object_ids.get_element(i)["ObjectId"];
					get_object_id.put_member_i64("ObjectId", ri);
					get_object_id.set_natural_order();
					json check_request = create_request(query_class_request, get_object_id);
					if (check_object_key_permission(check_request, "Get"))
					{
						json objx = co_await get_object(get_object_id);
						if (objx["Success"])
						{
							objects.append_element(objx["Data"]);
						}
					}
				}
			}

			response = create_response(query_class_request, true, "Query completed", objects, 0.0);
			co_return response;
		}

		database_method_transaction<json> create_object(json create_object_request)
		{
			json_parser jp;

			json token = create_object_request["Token"];
			json data = create_object_request["Data"];
			std::string class_name = data["ClassName"];
			json response;

			if (!check_message(create_object_request, { auth_general }))
			{
				response = create_response(create_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return response;
			}

			if (!has_class_permission(token, class_name, "Get")) {
				json result = create_response(create_object_request, false, "Cannot get class", data, 0.0);
				co_return result;
			}

			json class_key = jp.create_object("ClassName", class_name);
			class_key.set_natural_order();

			json class_data = co_await classes.get(class_key);

			if (class_data.is_object()) {
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
				json new_object = jp.create_object();
				new_object.put_member("ClassName", class_name);

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
					else if (jpx.is_object())
					{
						new_object.put_member_object(member.first);
					}
					else if (jpx.is_array())
					{
						new_object.put_member_array(member.first);
					}
				}
				response = create_response(create_object_request, true, "Object created", new_object, 0.0);

			}
			co_return response;
			
		}

		database_method_transaction<json> put_object(json put_object_request)
		{
			json_parser jp;

			json token;
			json object_definition;
			json result;

			object_definition = put_object_request["Data"];

			if (!check_message(put_object_request, { auth_general }))
			{
				result = create_response(put_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			bool permission = co_await check_object_permission(put_object_request, "Put");
			if (!permission) {
				json result = create_response(put_object_request, false, "Cannot create object", put_object_request["Data"], 0.0);
				co_return result;
			}

			result = co_await check_object(put_object_request);

			if (result["Success"])
			{
				json obj = result["Data"];

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

				json key_index = jp.create_object();
				key_index.copy_member("ClassName", obj);
				key_index.copy_member("Name", obj);
				key_index.copy_member("ObjectId", obj);
				co_await objects_by_name.put(key_index);

				relative_ptr_type put_result = co_await objects.put( obj );

				json cobj = object_definition.extract({ "ClassName", "ObjectId" });
				relative_ptr_type classput_result = co_await class_objects.put(cobj);

				result = create_response(put_object_request, true, "Object created", object_definition, 0.0);
			}
			else 
			{
				result = create_response(put_object_request, false, "Invalid object", put_object_request["Data"], 0.0);
			}

			co_return result;
		}

		database_method_transaction<json> get_object(
			json get_object_request
		)
		{
			json_parser jp;
			json result;

			if (!check_message(get_object_request, { auth_general }))
			{
				result = create_response(get_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			bool permission = co_await check_object_permission(get_object_request, "Get");
			if (!permission) {
				json result = create_response(get_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return result;
			}

			json token = get_object_request["Token"];
			json obj = co_await acquire_object(get_object_request);

			auto child_members = obj.get_members();

			for (auto child_member : child_members)
			{
				auto cm = child_member.second;
				if (cm.is_array())
				{
					json linked_object_key = jp.create_object();
					linked_object_key.put_member("Token", token);
					for (int64_t index = 0; index < cm.size(); index++)
					{
						json em = cm.get_element(index);
						if (em.is_object() &&
							em.is_member("ClassName", "SysReference") &&
							em.is_member("DeepGet", true)
							)
						{
							int64_t linked_object_id = em["LinkedObjectId"];
							linked_object_key.put_member_i64("ObjectId", linked_object_id);
							linked_object_key.set_natural_order();
							json lo_req = create_request(get_object_request, linked_object_key);
							database_method_transaction<json> get_object_task = get_object(lo_req);
							json get_response = get_object_task.wait();
							if (get_response["Success"])
							{
								json data = get_response["Data"];
								em.put_member("Data", data);
							}
						}
					}
				}
				else if (
					cm.is_member("ClassName", "SysReference") &&
					cm.is_member("DeepGet", true)
					)
				{
					int64_t linked_object_id = cm["LinkedObjectId"];
					json linked_object_key = jp.create_object();
					linked_object_key.put_member("Token", token);
					linked_object_key.put_member_i64("ObjectId", linked_object_id);
					linked_object_key.set_natural_order();
					json lo_req = create_request(get_object_request, linked_object_key);
					database_method_transaction<json> get_object_task = get_object(lo_req);
					json get_response = get_object_task.wait();
					if (get_response["Success"])
					{
						json data = get_response["Data"];
						cm.put_member("Data", data);
					}
				}
			}

			result = create_response(get_object_request, true, "Ok", obj, 0.0);

			co_return result;
		}


		database_transaction<json> delete_object(json delete_object_request)
		{
			json response;
			json_parser jp;

			if (!check_message(delete_object_request, { auth_general }))
			{
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), 0.0);
				co_return response;
			}

			if (!check_object_key_permission(delete_object_request, "Delete")) {
				json result = create_response(delete_object_request, false, "Cannot delete object", jp.create_object(), 0.0);
				co_return result;
			}

			json object_key = delete_object_request["Data"];

			json object_def = co_await objects.get(object_key);

			response = create_response(delete_object_request, false, "Failed", object_key, 0.0);

			if (object_def.is_object()) {
				bool success = co_await class_objects.erase(object_def);
				if (success) {
					success = co_await objects.erase(object_key);
					if (success) {
						response = create_response(delete_object_request, true, "Ok", object_key, 0.0);
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
								json dor = create_request(delete_object_request, object_key);
								json new_object = co_await delete_object(dor);
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
						json dor = create_request(delete_object_request, object_key);
						json new_object = co_await delete_object(dor);
					}
				}

			}
			else 
			{
				response = create_response(delete_object_request, false, "Not found", object_key, 0.0);
			}
			co_return response;
		}

		database_transaction<json> copy_object(json copy_request)
		{
			json_parser jp;

			json response;

			if (!check_message(copy_request, { auth_general }))
			{
				response = create_response(copy_request, false, "Denied", jp.create_object(), 0.0);
				co_return response;
			}

			json source_key = copy_request["SourceKey"];

			json object_copy = co_await acquire_object(source_key);

			json check_request = create_request(copy_request, object_copy);

			bool permission = co_await check_object_permission(copy_request, "Get");
			if (!permission) {
				json result = create_response(copy_request, false, "Denied", source_key, 0.0);
				co_return result;
			}

			json new_object = object_copy.clone();
			db_object_id_type new_object_id = co_await get_next_object_id();
			new_object.put_member_i64("ObjectId", new_object_id);

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
							json cy_request = create_request(copy_request, object_key);
							json new_object = co_await copy_object(cy_request);
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
					json cy_request = create_request(copy_request, object_key);
					json new_object = co_await copy_object(cy_request);
					cm.put_member_i64("LinkObjectId", new_object["ObjectId"]);
				}
			}

			json por = create_request(copy_request, new_object);
			json result = co_await put_object(por);

			if (result["Success"]) {
				response = create_response(copy_request, true, "Ok", result["Data"], 0);
			}
			else
			{
				response = result;
			}

			co_return response;
		}

		json create_system_request(json _data)
		{
			json_parser jp;

			json payload = jp.create_object();
			json token = jp.create_object();
			token.put_member("Name", "System");
			token.put_member("Authorization", "System");
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

			token.copy_member("Name", src_token);
			token.copy_member("Authorization", src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member("TokenExpires", expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member("Signature", cipher_text);

			payload.put_member("Token", token);
			payload.put_member("Success", true);
			payload.put_member("Message", "Ok");
			payload.put_member("Data", _data);
			payload.put_member("Signature", cipher_text);
			return payload;
		}


		database_transaction<json> get_banner()
		{
			json_parser jp;
			json body = jp.parse_object(R"(
{
	"Name":"Corona Server",
	"Version":"1.0"
}
)");
			co_return body;
		}

		http_handler_function corona_test = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto btask = this->get_banner();
			json fn_response = btask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login_start = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto lutask = this->login_user(parsed_request);
			json fn_response = lutask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login_confirmation = [this](http_action_request _request)-> void{
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto lotask = this->send_login_confirmation_code(parsed_request);
			json fn_response = lotask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login_confirm = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto lotask = this->login_user(parsed_request);
			json fn_response = lotask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login_password_reset = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto sptask = this->send_password_reset_code(parsed_request);
			json fn_response = sptask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto gctask = this->get_classes(parsed_request);
			json fn_response = gctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->create_class(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->put_class(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->create_user(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->get_object(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_query = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->query_class(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->create_object(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->put_object(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_delete = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->delete_object(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_edit = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto pctask = this->edit_object(parsed_request);
			json fn_response = pctask.wait();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		json parse_request(http_request _request)
		{
			json_parser jph;
			json request;

			if (_request.body.get_size() > 1)
			{
				json request = jph.parse_object(_request.body.get_ptr());
			}
			else
			{
				request = jph.create_object();
			}

			return request;
		}

		http_response check_parse_error(json _request)
		{
			http_response response;

			response.content_type = "application/json";
			response.http_status_code = 200;

			if (_request.is_member("ClassName", "SysParseError")) {
				response.http_status_code = 504;
				response.response_body = (buffer)_request;
				response.content_length = response.response_body.get_size();
				return response;
			}

			return response;
		}

		http_response create_response(int _http_status_code, json _source)
		{
			http_response response;

			response.http_status_code = _http_status_code;
			response.content_type = "application/json";
			response.response_body = (buffer)_source;
			response.content_length = response.response_body.get_size();
			response.server = "Corona 1.0";
			response.system_result = os_result(0);
			return response;
		}

		void bind_web_server(http_server& _server)
		{
			std::string _root_path = listen_point;

			if (!_root_path.ends_with('/')) {
				_root_path += "/";
			}

			std::string path = _root_path + "test/";
			_server.put_handler( HTTP_VERB::HttpVerbGET, path, corona_test);

			path = _root_path + "login/createuser/";
			_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_users_create);

			path = _root_path + "login/start/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_login_start );

			path = _root_path + "login/sendcode/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_login_confirmation);

			path = _root_path + "login/confirmcode/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_login_confirm);

			path = _root_path + "login/passwordset/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_login_password_reset);

			path = _root_path + "classes/get/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_classes_get);

			path = _root_path + "classes/create/";
			_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_classes_create);

			path = _root_path + "classes/put/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_classes_put);

			path = _root_path + "objects/get/";
			_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_get);

			path = _root_path + "objects/query/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_objects_query);

			path = _root_path + "objects/create/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_objects_create);

			path = _root_path + "objects/put/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_objects_put);

			path = _root_path + "objects/delete/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_objects_delete );

			path = _root_path + "objects/edit/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_objects_edit);

		}

	};


	user_transaction<bool> test_database_engine(corona::application& _app)
	{
		bool success = true;

		file dtest = _app.create_file(FOLDERID_Documents, "corona_json_database_test.ctb");

		std::cout << "test_database_engine, thread:" << ::GetCurrentThreadId() << std::endl;

		corona_database db( _app, & dtest);

		std::cout << "test_database_engine, create_database, thread:" << ::GetCurrentThreadId() << std::endl;

		auto create_database_task = db.create_database();
		relative_ptr_type database_location = create_database_task.wait();

		json_parser jp;
		json user = jp.parse_object(R"(
{
"ClassName" : "SysUser",
"Name" : "testuser",
"FirstName" : "Jake",
"LastName" : "Rogers",
"Email" : "todd.bandrowsky@gmail.com",
"Mobile" : "443 877 8606",
"Street" : "120 West McLellan Rd",
"City" : "Bowling Green",
"State" : "KY",
"Zip" : "42101",
"Password1" : "Mypassword",
"Password2" : "Mypassword"
}
)");

		json sys_request = db.create_system_request(user);

		json user_result = co_await db.create_user(sys_request);

		co_return success;
	}

	user_transaction<bool> run_server(corona::application& _app)
	{
		try 
		{
			file dtest = _app.create_file(FOLDERID_Documents, "corona_json_database_test2.ctb");

			std::cout << "test_database_engine, thread:" << ::GetCurrentThreadId() << std::endl;

			corona_database db(_app, &dtest), * pdb = &db;

			db.read_config(&_app);

			std::cout << "test_database_engine, create_database, thread:" << ::GetCurrentThreadId() << std::endl;
			
			auto create_database_task = db.create_database();
			relative_ptr_type database_location = create_database_task.wait();

			std::cout << "test_database_engine, create_database, thread:" << ::GetCurrentThreadId() << std::endl;

			http_server db_api_server;

			db.bind_web_server(db_api_server);
			db_api_server.start();

			while (true)
			{
				co_await db_api_server.next_request();
			}

		}
		catch (std::exception exc)
		{
			std::cout << "Exception:" << exc.what() << std::endl;
		}

		co_return true;
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
				if (!last_name.starts_with("last")) {
					std::cout << __LINE__ << ":last name failed" << std::endl;
					return false;
				}
				first_name = sl.item.get_string(1);
				if (!first_name.starts_with("first")) {
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
			std::cout << exc.what() << std::endl;
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

				if (bw != 1000 || bh != 1000) {

					std::cout << __LINE__ << ":array failed" << std::endl;
					return false;
				}

			}

			return true;
		}
		catch (std::exception& exc)
		{
			std::cout << exc.what() << std::endl;
			return false;
		}
	}

}

#endif
