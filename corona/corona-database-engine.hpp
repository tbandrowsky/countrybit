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

		std::string send_grid_api_key;
		bool watch_polling;

		std::shared_ptr<file> database_file;

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

		lockable classes_rw_lock;
		lockable objects_rw_lock;
		lockable header_rw_lock;

	public:

		bool trace_check_class = false;

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
			
			{
				scope_lock lock_one(classes_rw_lock);
				scope_lock lock_two(objects_rw_lock);

				header.data.object_id = 1;
				header_location = header.append(database_file.get(), [this](int64_t _size)->int64_t {
					return database_file->add(_size);
					});

				header.data.object_id = 1;
				header.data.classes_location =  classes.create();
				header.data.class_objects_location =  class_objects.create();
				header.data.objects_location =  objects.create();
				header.data.objects_by_name_location =  objects_by_name.create();

				created_classes = jp.create_object();

				header.write(database_file.get(), nullptr, nullptr);

			}
		
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

			json test =  classes.get(R"({"ClassName":"SysObject"})");
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
	"ImplementMap" : [ "SchemaName", "SchemaVersion" ],
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

			test =  classes.get(R"({"ClassName":"SysSchemas"})");
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
	"ImplementMap" : [ "DatasetName", "DatasetVersion" ],
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

			test =  classes.get(R"({"ClassName":"SysSchemas"})");
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

			test =  classes.get(R"({"ClassName":"SysReference"})");
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
	"ImplementMap" : [ "Name" ],
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

			test =  classes.get(R"({"ClassName":"SysUser"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysUser after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysUser", true);

			response =  create_class(R"(
{	
	"BaseClassName" : "SysObject",
	"ClassName" : "SysLogin",
	"ClassDescription" : "A login of a user",
	"ImplementMap" : [ "Name" ],
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

			if (not response["Success"]) {
				system_monitoring_interface::global_mon->log_warning("create_class SysLogin put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes.get(R"({"ClassName":"SysLogin"})");
			if (test.empty() or test.is_member("ClassName", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class SysLogin after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("SysLogin", true);

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

			test =  classes.get(R"({"ClassName":"SysPermission"})");
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

			test =  classes.get(R"({"ClassName":"SysMember"})");
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

			test =  classes.get(R"({"ClassName":"SysGrant"})");
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

			test =  classes.get(R"({"ClassName":"SysClassGrant"})");
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

			test =  classes.get(R"({"ClassName":"SysObjectGrant"})");
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
	"ImplementMap" : [ "Name" ],
	"Fields" : {			
			"Name" : "string",
			"Description" : "string",
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

			test =  classes.get(R"({"ClassName":"SysTeam"})");
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
			new_user_data.put_member("Name", default_user);
			new_user_data.put_member("Email", default_email_address);

			new_user_request = create_system_request(new_user_data);
			json new_user_result =  create_user(new_user_request);
			json new_user = new_user_result["Data"];
			json user_return = create_response(new_user_request, true, "Ok", new_user, method_timer.get_elapsed_seconds());
			response = create_response(new_user_request, true, "Database Created", user_return, method_timer.get_elapsed_seconds());

			system_monitoring_interface::global_mon->log_job_stop("create_database", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

private:

		json create_class(std::string _text)
		{
			timer method_timer;
			json_parser jp;
			json j = jp.parse_object(_text);
			json response;
			json check_request = create_system_request(j);

			if (j.error()) {
				response = create_response(check_request, false, "parse error on class", j, method_timer.get_elapsed_seconds());
				return response;
			}

			json checked =  check_class(check_request);

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_class", "start", start_time, __FILE__, __LINE__);

			if (checked["Success"]) {
				scope_lock lock(classes_rw_lock);
				json adjusted_class = checked["Data"];
				relative_ptr_type ptr =  classes.put(adjusted_class);
				if (ptr != null_row) {
					json key = adjusted_class.extract({ "ClassName" });
					json temp =  classes.get(key); 
					if (temp.empty()) {
						response = create_response(check_request, false, "save check failed", adjusted_class, method_timer.get_elapsed_seconds());
						return response;
					}
					if (adjusted_class.has_member("Ancestors")) {
						auto ancestors = adjusted_class["Ancestors"];
						if (ancestors.object()) {
							auto ancestor_classes = ancestors.get_members_raw();
							for (auto acp : ancestor_classes) {
								std::string acn = acp.first;
								json class_key = jp.create_object();
								class_key.put_member("ClassName", acn);
								auto ancestor_class =  classes.get(class_key);
								json descendants;
								if (ancestor_class.has_member("Descendants")) {
									descendants = ancestor_class["Descendants"];
								}
								else 
								{
									descendants = jp.create_object();
								}
								descendants.put_member(acn, acn);
								ancestor_class.put_member("Descendants", descendants);
								 classes.put(ancestor_class);
							}
						}
					}
					response = create_response(check_request, true, "Ok", adjusted_class, method_timer.get_elapsed_seconds());
				}
				else {
					response = create_response(check_request, false, "didn't save", checked, method_timer.get_elapsed_seconds());
				}
			}
			else 
			{
				response = checked;
			}
			system_monitoring_interface::global_mon->log_function_stop("create_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return response;
		}

		bool compare_classes(json _classA, json _classB)
		{
			json_parser jp;
			json classA = jp.create_object();
			classA.copy_member("BaseClassName", _classA);
			classA.copy_member("Fields", _classA);
			classA.copy_member("ImplementMap", _classA);
			std::string sa = classA.to_json_typed();

			json classB = jp.create_object();
			classB.copy_member("BaseClassName", _classB);
			classB.copy_member("Fields", _classB);
			classB.copy_member("ImplementMap", _classB);
			std::string sb = classB.to_json_typed();

			return sa == sb;
		}

		json check_class(json check_class_request)
		{
			timer method_timer;
			json result;

			json class_definition = check_class_request["Data"];

			date_time start_time = date_time::now();
			if (trace_check_class) {
				bus->log_activity("check_class", start_time);
			}

			result = create_response(check_class_request, true, "Ok", class_definition, method_timer.get_elapsed_seconds());

			if (not class_definition.has_member("ClassName"))
			{
				result = create_response(check_class_request, false, "Class must have a name", class_definition, method_timer.get_elapsed_seconds());
			}

			if (trace_check_class) {
				bus->log_information("Supplied class definition");
				bus->log_json<json>(class_definition);
			}

			std::string class_name = class_definition["ClassName"];

			// here we are going to grab the ancestor chain for this class.

			if (class_definition.has_member("BaseClassName"))
			{

				std::string base_class_name = class_definition["BaseClassName"];

				json_parser jp;
				json base_class_key = jp.create_object();
				base_class_key.put_member("ClassName", base_class_name);
				base_class_key.set_compare_order({ "ClassName" });
				json base_class_def;

				{
					scope_lock lock_one(classes_rw_lock);

					base_class_def =  classes.get(base_class_key);
				}

				if (trace_check_class)
				{
					bus->log_information("Base class definition");
					bus->log_json<json>(base_class_def);
				}

				if (not base_class_def.object())
				{
					result = create_response(check_class_request, false, "Base class not found", class_definition, method_timer.get_elapsed_seconds());
				}

				json ancestors = base_class_def["Ancestors"];
				if (not ancestors.empty()) {
					ancestors.put_member(base_class_name, base_class_name);
					class_definition.put_member("Ancestors", ancestors);
				}
				else 
				{
					class_definition.put_member_object("Ancestors");
				}

				if (trace_check_class)
				{
					bus->log_information("Class ancestors");
					bus->log_json<json>(ancestors);
				}

				auto inh_fields = base_class_def["Fields"];
				if (inh_fields.object()) {
					auto inherited_fields = inh_fields.get_members();
					if (trace_check_class)
					{
						bus->log_information("Inherited fields");
					}

					if (not class_definition.has_member("Fields")) {
						class_definition.put_member_object("Fields");
					}
					json fieldso = class_definition["Fields"];
					for (auto field : inherited_fields) {
						fieldso.put_member(field.first, field.second);
						if (trace_check_class)
						{
							bus->log_information(field.first);
						}
					}

					if (trace_check_class) {
						bus->log_information("Apply inherited fields");
						bus->log_json<json>(class_definition);
					}
				}

				if (class_definition.has_member("ImplementMap")) {
					json class_fields = class_definition["Fields"];
					json unique_names = class_definition["ImplementMap"];
					if (unique_names.array()) {
						for (auto jfield_name : unique_names) {
							std::string field_name = (std::string)jfield_name;
							if (not class_fields.has_member(jfield_name)) {
								std::string msg = std::format("map key field {0} does not exist in class {1}", (std::string)jfield_name, (std::string)class_name);
								result = create_response(check_class_request, false, msg, class_definition, method_timer.get_elapsed_seconds());
								return result;
							}
						}
					}					
				}

			} 
			else 
			{
				class_definition.put_member_object("Ancestors");
			}

			if (not class_definition.has_member("ClassDescription"))
			{
				result = create_response(check_class_request, false, "Class must have a description", class_definition, method_timer.get_elapsed_seconds());
			}

			if (not class_definition.has_member("Fields") or !class_definition["Fields"].object())
			{
				result = create_response(check_class_request, false, "Class needs some fields", class_definition, method_timer.get_elapsed_seconds());
			}
			else
			{
				json fields_object = class_definition["Fields"];
				auto members = fields_object.get_members();
				for (auto& member : members)
				{
					json_parser jpx;
					json jp = member.second;
					if (jp.is_string())
					{
						std::string field_type = jp.get_string();
						if (not allowed_field_types.contains(field_type))
						{
							json err_field = jpx.create_object("Name", member.first);
							result = create_response(check_class_request, false, "Bad field type", err_field, method_timer.get_elapsed_seconds());
							return result;
						}
					}
					else if (jp.object())
					{
						std::string field_type = jp["FieldType"];
						if (not allowed_field_types.contains(field_type))
						{
							json err_field = jpx.create_object("Name", member.first);
							result = create_response(check_class_request, false, "Bad field type", err_field, method_timer.get_elapsed_seconds());
							return result;
						}
					}
					else
					{
						json err_field = jpx.create_object("Name", member.first);
						result = create_response(check_class_request, false, "Class field incorrect", err_field, method_timer.get_elapsed_seconds());
					}
				}
				result = create_response(check_class_request, true, "Ok", class_definition, method_timer.get_elapsed_seconds());
			}

			return result;
		}

		json check_object(json check_object_request)
		{
			timer method_timer;
			json result;
			json_parser jp;

			json object_load = check_object_request["Data"];
			bool strict_enabled = (bool)check_object_request["Strict"];

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

			std::map<std::string, json> classes_ahead;

			auto class_list = classes_group.get_members();
			json key_boy = jp.create_object();

			for (auto class_def : class_list) {

				key_boy.put_member("ClassName", class_def.first);
				json class_data = classes.get(key_boy);
				std::string class_name = key_boy["ClassName"];
				classes_ahead.insert_or_assign(class_name, class_data);
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
				}
				else
				{
					object_id =  get_next_object_id();

					object_definition.put_member_i64("ObjectId", object_id);
				}

				json warnings = jp.create_array();

				std::string class_name = object_definition["ClassName"];

				json class_data = classes_ahead[class_name];

				if (not class_data.empty())
				{
					result.put_member("ClassDefinition", class_data);
					// check the object against the class definition for correctness
					// first we see which fields are in the class not in the object
					json field_definition = class_data["Fields"];
					auto class_members = field_definition.get_members();
					for (auto kv : class_members) {
						json err_field = jp.create_object("Name", kv.first);
						if (object_definition.has_member(kv.first)) {
							std::string obj_type = object_definition[kv.first]->get_type_name();
							std::string member_type = kv.second;
							if (member_type != obj_type) {
								object_definition.change_member_type(kv.first, member_type);
							}
						}
						else
						{
							json warning = jp.create_object();
							warning.put_member("Error", "Required field missing");
							warning.put_member("FieldName", kv.first);
							warnings.push_back(warning);
						}
					}
					// then we see which fields are in the object that are not 
					// in the class definition.
					auto object_members = object_definition.get_members();
					for (auto om : object_members) {
						if (field_definition.has_member(om.first)) {
							;
						}
						else {
							json warning = jp.create_object();
							warning.put_member("Error", "Field not found in class definition");
							warning.put_member("FieldName", om.first);
							warnings.push_back(warning);
						}
					}
					result = jp.create_object();
					if (warnings.size() > 0) {
						std::string msg = std::format("Object '{0}' has problems", class_name);
						result.put_member("Message", msg);
						result.put_member("Success", 0);
						result.put_member("Warnings", warnings);
						result.put_member("Definition", class_data);
						result.put_member("Data", object_definition);
					}
					else {
						result.put_member("Message", "Ok");
						result.put_member("Success", 1);
						result.put_member("Definition", class_data);
						result.put_member("Data", object_definition);
					}

					if (class_data.has_member("ImplementMap"))
					{
						json objects_by_name_key = jp.create_object();
						json constraint_fields = class_data["ImplementMap"];
						std::vector<std::string> constraint_names;
						constraint_names.push_back("ClassName");
						for (auto constraint_field : constraint_fields) {
							constraint_names.push_back(constraint_field);
						}
						objects_by_name_key.set_compare_order(constraint_names);
						result.put_member("ImplementMapKey", objects_by_name_key);
					}
				}
				else
				{
					std::string msg = std::format("'{0}' is not valid class_name", class_name);
					result.put_member("Message", msg);
					result.put_member("Success", 0);
					result.put_member("Data", object_definition);
					result = class_data;
				}
				result_list.push_back(result);
			}
			header.write(database_file.get(), nullptr, nullptr);

			result = create_response(check_object_request, true, "Objects processed", result_list, method_timer.get_elapsed_seconds());
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
			token.put_member("Name", _user_name);
			token.put_member("Authorization", _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			std::string hash = crypter.hash(_data);
			token.put_member("DataHash", hash);
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
			payload.put_member_double("Seconds", _seconds);
			return payload;
		}

		bool check_token(json& token, std::vector<std::string> _authorizations)
		{
			if (not token.object())
			{
				return false;
			}

			if (not token.has_member("Signature"))
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

			if (not token.has_member("Authorization"))
			{
				return false;
			}

			std::string authorization = token["Authorization"];
			std::string user = token["Name"];

			if (authorization == "System" and user == "System")
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

		bool check_message(json& _message, std::vector<std::string> _authorizations)
		{
			json token = _message["Token"];

			bool result = check_token(token, _authorizations);

			return result;
		}

		json get_message_user(json _token)
		{
			if (_token.has_member("Token")) {
				_token = _token["Token"];
			}
			json token_name = _token.extract({ "Name" });
			return token_name;
		}

		json acquire_object(json _object_key)
		{
			json_parser jp;
			json obj;

			scope_lock lock_one(objects_rw_lock);

			json class_key = _object_key.extract({ "ClassName" } );
			json classdef;

			class_key.set_compare_order({ "ClassName" });
			classdef = classes.get(class_key);
			
			if (classdef.empty()) 
			{
				return obj;
			}

			if (classdef.has_member("ImplementMap"))
			{
				json objects_by_name_key = jp.create_object();
				json constraint_fields = classdef["ImplementMap"];
				std::vector<std::string> constraint_names;
				constraint_names.push_back("ClassName");
				for (auto constraint_field : constraint_fields) {
					constraint_names.push_back(constraint_field);
				}
				objects_by_name_key.set_compare_order(constraint_names);
				json name_id =  objects_by_name.get_first(objects_by_name_key, nullptr);
				if (name_id.object()) {
					json object_key = jp.create_object();
					object_key.copy_member("ObjectId", name_id);
					object_key.set_natural_order();
					obj =  objects.get(object_key);
				}
			}
			else if (_object_key.has_member("ObjectId"))
			{
				json object_key = jp.create_object();
				object_key.copy_member("ObjectId", _object_key);
				object_key.set_natural_order();
				obj =  objects.get(object_key);
			}

			return obj;
		}

		json get_linked_object(json _object_definition)
		{
			json_parser jp;
			json obj;

			if (_object_definition.is_member("ClassName", "SysReference"))
			{
				scope_lock lock_one(objects_rw_lock);

				json object_key = jp.create_object();
				db_object_id_type object_id = (db_object_id_type)_object_definition["LinkObjectId"];
				object_key.put_member("ObjectId", object_id);
				object_key.set_natural_order();
				obj =  objects.get(object_key);
			}
			else if (_object_definition.object())
			{
				obj = _object_definition;
			}

			return obj;
		}

		bool has_class_permission(
			json _token,
			std::string _class_name,
			std::string _permission)
		{

			bool granted = false;

			json_parser jp;
			json user;

			// check the token to make sure it is valid - this includes signature verification
			if (not check_token(_token, { auth_general })) {
				return false;
			}

			// extract the user key from the token and get the user object
			json user_key = get_message_user(_token);
			if ((std::string)user_key["Name"] == "System") {
				return true;
			}

			user_key.put_member("ClassName", "SysUser");

			user =  acquire_object(user_key);
			if (user.empty()) {
				return false;
			}

			// Now go through the teams the user is a member of and check the grants to see if we can access this
			json teams_list = user["Teams"];

			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team =  get_linked_object(item);

				if (team.is_member("ClassName", "Team")) {
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

		bool check_object_key_permission(
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
				return true;
			}

			user =  acquire_object(user_key);
			if (user.empty()) {
				return false;
			}

			json teams_list = user["Teams"];

			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team =  get_linked_object(item);

				if (team.is_member("ClassName", "SysTeam")) 
				{
					json team_grants = team["Grants"];

					for (int i = 0; i < team_grants.size(); i++)
					{
						json grant = team_grants.get_element(i);

						if (grant.is_member("ClassName", "SysObjectGrant"))
						{
							json filter = grant["ObjectFilter"];

							json obj =  acquire_object(object_key);

							bool has_matching_key = filter.compare(obj);

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

			if ((std::string)user_key["Name"] == "System") {
				return true;
			}

			// extract the user key from the token and get the user object
			user_key.set_compare_order({ "ClassName", "Name" });
			user =  acquire_object(user_key);
			if (user.empty()) {
				return false;
			}

			json teams_list = user["Teams"];

			for (int i = 0; i < teams_list.size(); i++)
			{
				json item = teams_list.get_element(i);

				json team =  get_linked_object(item);

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

			json class_obj =  classes.get(class_key);

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

			json class_obj =  classes.get(class_key);

			if (not class_obj.empty())
			{
				bool has_ancestor = class_obj["Ancestors"].has_member(_base_class);
				return has_ancestor;
			}

			return false;
		}


	public:

		std::string listen_point;
		std::string default_user;
		std::string default_password;
		std::string default_email_address;
		std::string default_guest_team;
		time_span token_life;
		comm_bus_interface* bus;

		// constructing and opening a database

		corona_database(comm_bus_interface* _bus, std::shared_ptr<file> _database_file) :
			bus(_bus),
			database_file(_database_file),
			classes(_database_file, { "ClassName" }),
			class_objects(_database_file, { "ClassName", "ObjectId" }),
			objects(_database_file, { "ObjectId" }),
			objects_by_name(_database_file, { "ClassName", "Name", "ObjectId" })
		{
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
				listen_point = server["ListenPoint"];
				default_user = server["DefaultUserName"];
				default_password = server["DefaultUserPassword"];
				default_email_address = server["DefaultUserEmailAddress"];
				default_guest_team = server["DefaultGuestTeam"];
			}
			system_monitoring_interface::global_mon->log_job_stop("apply_config", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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
			schema_key.put_member("ClassName", "SysDatasets");
			schema_key.set_compare_order({ "SchemaName", "SchemaVersion" });

			json schema_test =  acquire_object(schema_key);

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
								bus->log_error(class_result, __FILE__, __LINE__);
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
						json script_key = jp.create_object();
						script_key.copy_member("DatasetName", script_definition);
						script_key.copy_member("DatasetVersion", script_definition);
						script_key.put_member("ClassName", "SysDatasets");

						system_monitoring_interface::global_mon->log_job_section_start("DataSet", script_definition["DatasetName"], start_dataset, __FILE__, __LINE__);

						bool script_run = (bool)script_definition["RunOnChange"];
						json existing_script =  acquire_object(script_key);
						bool run_script = false;
						if (existing_script.empty() or script_run)
							run_script = true;

						script_definition.put_member("ClassName", "SysDatasets");
						json put_script_request = create_system_request(script_definition);
						// in corona, creating an object doesn't actually persist anything 
						// but a change in identifier.  It's a clean way of just getting the 
						// "new chumpy" item for ya.  
						json create_result =  create_object(put_script_request);
						if (create_result["Success"]) {
							json created_object = put_script_request["Data"];
							json save_result =  put_object(put_script_request);
							if (not save_result["Success"]) {
								system_monitoring_interface::global_mon->log_warning(save_result["Message"]);
								system_monitoring_interface::global_mon->log_json<json>(save_result);
							}
							else
								system_monitoring_interface::global_mon->log_information(save_result["Message"]);
						}

						json change_trigger = script_definition["RunOnChange"];
						{
							std::string class_to_watch = change_trigger["ClassToMonitor"];
							if (changed_classes.contains(class_to_watch)) {
								run_script = true;
								bool kill_and_fill = (bool)change_trigger["KillAndFill"];
								if (kill_and_fill) {
									json update_data = jp.create_object();
									update_data.put_member("ClassName", class_to_watch);
									json request = create_system_request(update_data);
									 update(request, R"({ "Active", false })"_jobject);
								}
							}
						}

						if (run_script and script_definition.has_member("Import"))
						{
							/*
							        "Type": "csv",
        "Delimiter": "|",
        "FileName": "itcont.csv",
        "TargetClass": "Individuals",
        "ColumnMap": {
*/
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
											 put_object(cor);
											system_monitoring_interface::global_mon->log_warning("FileName and Delimiter can't be blank.");
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
									else {
										object_definition.copy_member("Success", create_result);
										object_definition.copy_member("Message", create_result);
										system_monitoring_interface::global_mon->log_warning(create_result["Message"], __FILE__, __LINE__);
										system_monitoring_interface::global_mon->log_json<json>(create_result);
									}
								}
							}
							json put_script_request = create_system_request(script_definition);
							json save_script_result =  put_object(put_script_request);
							if (not save_script_result["Success"]) {
								system_monitoring_interface::global_mon->log_warning(save_script_result["Message"]);
								system_monitoring_interface::global_mon->log_json<json>(save_script_result);
							}
							else
								system_monitoring_interface::global_mon->log_information(save_script_result["Message"]);
						}

						system_monitoring_interface::global_mon->log_job_section_stop("DataSet", script_definition["DatasetName"], txs.get_elapsed_seconds(), __FILE__, __LINE__);


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

			relative_ptr_type header_location =  header.read(database_file.get(), _header_location);

			if (header_location != null_row) {
				relative_ptr_type result =  classes.open(header.data.classes_location);
				result =  class_objects.open(header.data.class_objects_location);
				result =  objects_by_name.open(header.data.objects_by_name_location);
				result =  objects.open(header.data.objects_location);
			}

			system_monitoring_interface::global_mon->log_job_stop("open_database", "Open database", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return header_location;
		}

		// this creates a user account, using an email and a phone number to send a confirmation code to.
		json create_user(json create_user_request)
		{
			timer method_timer;
			json_parser jp;
			json response;


			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_user", "start", start_time, __FILE__, __LINE__);

			json data = create_user_request["Data"];

			std::string user_name = data["Name"];
			std::string user_class = data["ClassName"];

			bool user_exists = true;
			int attempt_count = 0;

			do 
			{
				scope_lock lock_one(objects_rw_lock);

				json user_key = jp.create_object();
				user_key.put_member("ClassName", user_class);
				user_key.put_member("Name", user_name);
				user_key.set_compare_order({ "ClassName", "Name" });
				json existing_user_link =  objects_by_name.get(user_key);

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

			std::string hashed_pw = crypter.hash(data);

			json create_user_params = data;
			create_user_params.put_member("ClassName", user_class);
			create_user_params.put_member("Name", user_name);
			json create_object_request = create_request(create_user_request, create_user_params);
			json user_result =  put_object(create_object_request);
			if (user_result["Success"]) {
				json new_user_wrapper = user_result["Data"];
				if (new_user_wrapper.array() and new_user_wrapper.size()> 0) {
					json new_user = new_user_wrapper.get_element(0);

					json create_login_params = jp.create_object();
					create_login_params.put_member("ClassName", "SysLogin");
					create_login_params.put_member("Password", hashed_pw);
					create_login_params.put_member("Name", user_name);
					create_login_params.put_member("LoginState", "UserCreated");
					if (new_user.has_member("ObjectId")) {
						db_object_id_type objid = new_user["ObjectId"].get_int64();
						create_login_params.put_member("CurrentObjectId", objid);
					}
					json put_object_request = create_request(create_user_request, create_login_params);
					json put_response =  put_object(put_object_request);

					bool succeeded = (bool)put_response["Success"];
					if (succeeded)
					{
						data.put_member("Password", hashed_pw);
						response = create_response(user_name, auth_login_user, true, "User created", data, method_timer.get_elapsed_seconds());
					}
				} else 
					response = create_response(user_name, auth_login_user, false, "User created", data, method_timer.get_elapsed_seconds());
			}
			else
			{
				response = create_response(create_user_request, false, "User not created", data, method_timer.get_elapsed_seconds());
			}
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
			std::string user_name = data["Name"];
			std::string user_password = data["Password"];
			std::string hashed_user_password;

			json user_key = jp.create_object();
			user_key.put_member("Name", user_name);
			user_key.put_member("ClassName", "SysLogin");
			json gor = create_request(_login_request, user_key);		

			json user_login =  get_object(gor);

			/*   
			
			consider checking that when a user is a new user, you cannot log into it.

			create_login_params.put_member("LoginState", "UserCreated");  */

			if (user_login["Success"]) 
			{
				json ul = user_login["Data"];
				std::string lstest = ul["LoginState"];
				std::string existing_hashed_password = ul["Password"];
				if (hashed_user_password == existing_hashed_password)
				{
					ul.put_member("LoginState", "PasswordAccepted");
					json porq = create_request(_login_request, ul);
					 put_object(porq);

					data.copy_member("ObjectId", ul);

					response = create_response(user_name, auth_general, true, "Ok", data, method_timer.get_elapsed_seconds());
				}
				else 
				{
					response = create_response(_login_request, false, "Failed", jp.create_object(), method_timer.get_elapsed_seconds());
				}
			}
			else
			{
				response = create_response(_login_request, false, "Failed", jp.create_object(), method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("login_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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

			result_list =  classes.select([this, get_classes_request](int _index, json& _item) {
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
				result =  classes.get(key);
			}


			result = create_response(get_class_request, true, "Ok", result, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("get_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		json delete_objects(json delete_request)
		{

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("delete_objects", "start", start_time, __FILE__, __LINE__);

			timer method_timer;
			json_parser jp;
			json result;
			if (not check_message(delete_request, { auth_general }))
			{
				result = create_response(delete_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return result;
			}

			system_monitoring_interface::global_mon->log_function_stop("delete_objects", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			json delete_filter = delete_request["Data"];

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
			json class_definition = put_class_request["Data"];

			bool can_put_class =  has_class_permission(
				token,
				class_definition["ClassName"],
				"Put");

			if (not can_put_class) {
				result = create_response(put_class_request, false, "Denied", class_definition, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			scope_lock lock_one(classes_rw_lock);

			std::string class_name = class_definition["ClassName"];
			json class_key = jp.create_object();
			class_key.put_member("ClassName", class_name);
			class_key.set_compare_order({ "ClassName" });
			json class_exists = classes.get(class_key);
			
			bool changed_class = false;

			if (compare_classes(class_exists, class_definition)) {
				class_exists.put_member("RunScript", true);
				result = create_response(put_class_request, true, "Class has not changed", class_exists, method_timer.get_elapsed_seconds());
			}

			changed_class = true;

			json check_request = create_request(put_class_request, class_definition);
			json checked =  check_class(check_request);
			if (checked["Success"]) {
				scope_lock lock(classes_rw_lock);
				json adjusted_class = checked["Data"];
				relative_ptr_type ptr =  classes.put(adjusted_class);
				if (ptr != null_row) {
					json key = adjusted_class.extract({ "ClassName" });
					json temp =  classes.get(key);
					if (temp.empty()) {
						result = create_response(check_request, false, "save check failed", adjusted_class, method_timer.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("put_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

						return result;
					}
					if (adjusted_class.has_member("Ancestors")) {
						auto ancestors = adjusted_class["Ancestors"];
						if (ancestors.object()) {
							auto ancestor_classes = ancestors.get_members_raw();
							for (auto acp : ancestor_classes) {
								std::string acn = acp.first;
								json class_key = jp.create_object();
								class_key.put_member("ClassName", acn);
								auto ancestor_class =  classes.get(class_key);
								json descendants;
								if (ancestor_class.has_member("Descendants")) {
									descendants = ancestor_class["Descendants"];
								}
								else
								{
									descendants = jp.create_object();
								}
								descendants.put_member(acn, acn);
								ancestor_class.put_member("Descendants", descendants);
								classes.put(ancestor_class);
							}
						}
					}
					adjusted_class.put_member("ClassChanged", changed_class);
					result = create_response(check_request, true, "Ok", adjusted_class, method_timer.get_elapsed_seconds());
				}
				else {
					result = create_response(check_request, false, "didn't save", checked, method_timer.get_elapsed_seconds());
				}
			}
			else
			{
				result = checked;
			}
			system_monitoring_interface::global_mon->log_function_stop("put_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}

		json update(json query_class_request, json update_json)
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
			json filter = manip["Filter"];

			bool class_granted =  has_class_permission(token, base_class_name, "Get");
			if (not class_granted)
			{
				response = create_response(query_class_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json class_def;

			json class_key = query_class_request["Data"]
								.extract({ "ClassName" });
			class_def =  classes.get(class_key);
			std::string class_name = class_def["ClassName"];

			std::map<std::string, bool> class_names;

			class_names.insert_or_assign(class_name, true);

			json derived_classes = class_def["Descendants"];

			if (derived_classes.object()) {
				auto members = derived_classes.get_members();

				for (auto member : members)
				{
					class_names.insert_or_assign(member.first, true);
				}
			}

			json object_list = jp.create_object();

			for (auto class_pair : class_names)
			{
				json_parser jp;
				json search_key = jp.create_object("ClassName", class_pair.first);
				search_key.set_compare_order({ "ClassName" });
				json class_object_ids;

				class_object_ids =  class_objects.update(search_key, [](int _index, json& _item)->json {
					return _item;
					}, update_json);

				json get_object_id = jp.create_object("ObjectId", 0i64);
				get_object_id.put_member("Token", token);

				if (class_object_ids.array()) 
				{
					object_list = objects.get_list(class_object_ids);
				}
			}

			response = create_response(query_class_request, true, "Query completed", object_list, method_timer.get_elapsed_seconds());
			system_monitoring_interface::global_mon->log_function_stop("update", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}


		json query_class(json query_class_request)
		{
			json_parser jp;
			json jx; // not creating an object, leaving it empty.  should work with empty objects
			// or with an object that has no members.
			return update(query_class_request, jx);
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

			json class_key = jp.create_object();
			class_key.put_member("ClassName", class_name);
			class_key.set_natural_order();

			json class_data;

			{
				scope_lock lock_one(classes_rw_lock);
				class_data =  classes.get(class_key);
			}

			if (class_data.object()) {
				json field_definition = class_data["Fields"];
				auto members = field_definition.get_members();
				json new_object = jp.create_object();
				new_object.put_member("ClassName", class_name);

				for (auto& member : members)
				{
					if (member.first == "ClassName")
						continue;

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
					else if (jpx.object())
					{
						new_object.put_member_object(member.first);
					}
					else if (jpx.array())
					{
						new_object.put_member_array(member.first);
					}
				}
				response = create_response(create_object_request, true, "Object created", new_object, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else {
				std::string msg = std::format("create_object failed because the class '{0}' was never found.", class_name);
				system_monitoring_interface::global_mon->log_warning(msg);
				system_monitoring_interface::global_mon->log_json(class_key);
				response = create_response(create_object_request, false, "Couldn't find class", class_key, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
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
				else {
					item_array = jp.create_array();
					item_array.push_back(data);
				}

				json objects_by_name_array = jp.create_array();
				json objects_array = jp.create_array();
				json class_objects_array = jp.create_array();


				for (json obj : item_array) {

					json dobj = obj["Data"];
					if (dobj.has_member("ImplementMapKey"))
					{
						json keys = dobj["ImplementMapKey"];
						if (keys.array()) {
							json key_index = jp.create_object();
							std::vector<std::string> key_order;
							for (auto skey : keys) {
								std::string sskey = (std::string)skey;
								key_index.copy_member(skey, obj);
								key_order.push_back(skey);
							}
							key_index.set_compare_order(key_order);
							key_index.copy_member("ObjectId", obj);
							objects_by_name_array.push_back(key_index);
						}
					}

					obj.put_member("Active", true);
					objects_array.push_back(dobj);
					json cobj = dobj.extract({ "ClassName", "ObjectId" });
					cobj.put_member("Active", true);
					class_objects_array.push_back(cobj);
				}

				objects.put_array(objects_array);
				objects_by_name.put_array(objects_by_name_array);
				class_objects.put_array(class_objects_array);

				result = create_response(put_object_request, true, "Object(s) created", data, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(put_object_request, false, result["Message"], result["Warnings"], method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("put_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

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

		json pop_object(json pop_object_request)
		{
			timer method_timer;
			json response;
			json_parser jp;


			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("pop_object", "start", start_time, __FILE__, __LINE__);


			if (not check_message(pop_object_request, { auth_general }))
			{
				response = create_response(pop_object_request, false, "Denied", jp.create_object(), method_timer.get_elapsed_seconds());
				return response;
			}

			if (not check_object_key_permission(pop_object_request, "Delete")) {
				json result = create_response(pop_object_request, false, "Cannot delete object", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("pop_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json object_key = pop_object_request["Data"];

			scope_lock lock_one(objects_rw_lock);

			if (object_key.has_member("ClassName")) {
				json revised_key =  class_objects.get_first(object_key, nullptr);
				object_key.copy_member("ObjectId", revised_key);
			}

			json object_def =  objects.get(object_key);

			response = create_response(pop_object_request, false, "Failed", object_key, method_timer.get_elapsed_seconds());

			if (object_def.object()) {
				bool success =  class_objects.erase(object_def);
				if (success) {
					success =  objects.erase(object_key);
					if (success) {
						response = create_response(pop_object_request, true, "Ok", object_key, method_timer.get_elapsed_seconds());
					}
				}
			}
			else
			{
				response = create_response(pop_object_request, false, "Not found", object_key, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("pop_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
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

			if (not check_object_key_permission(delete_object_request, "Delete")) {
				json result = create_response(delete_object_request, false, "Cannot delete object", jp.create_object(), method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json object_key = delete_object_request["Data"];

			scope_lock lock_one(objects_rw_lock);

			json object_def =  objects.get(object_key);
			object_def.put_member("Active", false);

			response = create_response(delete_object_request, false, "Failed", object_key, method_timer.get_elapsed_seconds());

			if (object_def.object()) 
			{
				auto obj =  class_objects.get(object_key);
				json cobj = obj.extract({ "ClassName", "ObjectId" });
				cobj.put_member("Active", false);
				relative_ptr_type classput_result =  class_objects.put(cobj);
				relative_ptr_type objectput_result =  objects.put(object_def);
				if (classput_result != null_row and objectput_result != null_row) 
				{
					response = create_response(delete_object_request, true, "Ok", object_key, method_timer.get_elapsed_seconds());
				}
			}
			else 
			{
				response = create_response(delete_object_request, false, "Not found", object_key, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("delete_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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


		json get_banner()
		{
			json_parser jp;
			json body = jp.parse_object(R"(
{
	"Name":"Corona Server",
	"Version":"1.0"
}
)");
			return body;
		}

		http_handler_function corona_test = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->get_banner();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login_start = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto fn_response = this->login_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_classes_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto fn_response = this->get_classes(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->create_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->put_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->create_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->get_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_query = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->query_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->create_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->put_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_delete = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->delete_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_edit = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.is_member("ClassName", "SysParseError")) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = this->edit_object(parsed_request);
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

			if (not _root_path.ends_with('/')) {
				_root_path += "/";
			}

			std::string path = _root_path + "test/";
			_server.put_handler( HTTP_VERB::HttpVerbGET, path, corona_test);

			path = _root_path + "login/createuser/";
			_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_users_create);

			path = _root_path + "login/start/";
			_server.put_handler( HTTP_VERB::HttpVerbPOST, path, corona_login_start );

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


	bool test_database_engine(std::shared_ptr<application> _app)
	{
		bool success = true;
		std::shared_ptr<file> dtest = std::make_shared<file>();

		*dtest = _app->create_file(FOLDERID_Documents, "corona_json_database_test.ctb");

		date_time start_schema = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_job_start("test_database_engine", "start", start_schema, __FILE__, __LINE__);

		corona_database db(nullptr, dtest);

		json_file_watcher jf;
		json db_config, schema_config;

		jf.file_name = "db.json";
		jf.poll_contents(_app.get(), db_config);
		jf.file_name = "schema.json";
		jf.poll_contents(_app.get(), schema_config);

		db.apply_config(db_config);
		auto schema_task = db.apply_schema(schema_config);

		system_monitoring_interface::global_mon->log_information("create_database");

		relative_ptr_type database_location = db.create_database();

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
"Zip" : "42101"
}
)");

		json sys_request = db.create_system_request(user);

		json user_result =  db.create_user(sys_request);

		system_monitoring_interface::global_mon->log_job_stop("test_database_engine", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		return success;
	}

	bool run_server(std::shared_ptr<application> _app)
	{
		date_time start_schema = date_time::now();
		timer tx;
		try
		{
			system_monitoring_interface::global_mon->log_command_start("run_server", "start", start_schema, __FILE__, __LINE__);

			std::shared_ptr<file> dtest = std::make_shared<file>();
			*dtest = _app->create_file(FOLDERID_Documents, "corona_json_database_test.ctb");

			corona_database db(nullptr, dtest);

			corona_database *pdb = &db;

			json_file_watcher jf;
			json db_config, schema_config;

			jf.file_name = "db.json";
			jf.poll_contents(_app.get(), db_config);
			jf.file_name = "schema.json";
			jf.poll_contents(_app.get(), schema_config);

			db.apply_config(db_config);
			db.apply_schema(schema_config);
			
			relative_ptr_type database_location = db.create_database();

			http_server db_api_server;

			db.bind_web_server(db_api_server);
			db_api_server.start();

			while (true)
			{
				jf.file_name = "db.json";
				if (jf.poll_contents(_app.get(), db_config) != null_row) {
					db.apply_config(db_config);
				}

				jf.file_name = "schema.json";
				if (jf.poll_contents(_app.get(), schema_config) != null_row) {
					auto schema_task = db.apply_schema(schema_config);
				}

				 db_api_server.next_request();
			}

		}
		catch (std::exception exc)
		{
			system_monitoring_interface::global_mon->log_exception(exc);
		}

		system_monitoring_interface::global_mon->log_command_stop("run_server", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		return true;
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
