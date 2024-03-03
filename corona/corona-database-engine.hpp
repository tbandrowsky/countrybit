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
		relative_ptr_type frames_location;
		relative_ptr_type objects_location;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
			class_objects_location = -1;
			frames_location = -1;
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
		json_table frames;
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

			if (_class_definition.has_member("BaseClass"))
			{
				std::string bc = _class_definition["BaseClass"];

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

		database_transaction<json> check_object(json _object_definition)
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

			json class_definition = co_await get_class(_object_definition);

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

	public:

		corona_database(file *_file) 
			: database_file(_file),
			classes(_file),
			class_objects(_file),
			frames(_file),
			objects(_file)
		{
			classes.get_key = [](json& _item) {
				return _item.extract({ "ClassName" } );
				};
			class_objects.get_key = [](json& _item) {
				return _item.extract({ "ClassName", "ObjectId" });
				};
			frames.get_key = [](json& _item) {
				return _item.extract({ "FrameName" });
				};
			objects.get_key = [](json& _item) {
				return _item.extract({ "ObjectId" });
				};
		}

		table_transaction<db_object_id_type> get_next_object_id()
		{
			scope_lock hlock(header_lock);
			header.data.object_id++;
			co_await header.write(database_file);
			co_return header.data.object_id;
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
			header.data.frames_location = co_await frames.create();
			header.data.objects_location = co_await objects.create();
			header.data.class_objects_location = co_await class_objects.create();

			json_parser jp;

			std::string object_base_class_string = R"(
{
	"ClassName" : "SysObjectBase",
	"ClassDescription" : "Base Class of All Objects",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"Activity" : "array"
	}
}
)";
			json object_base_class = jp.parse_object(object_base_class_string);
			co_await put_class(object_base_class);

			std::string user_base_class_string = R"(
{
	"ClassName" : "SysUserBase",
	"ClassDescription" : "Base Class of All Users",
	"Fields" : {			
			"ObjectId" : "int64",
			"ClassName" : "string",
			"Activity" : "array"
	}
}
)";
			json user_base_class = jp.parse_object(user_base_class_string);
			co_await put_class(user_base_class);

			std::string system_activity_class_string = R"(
{
	"ClassName" : "SysActivityBase",
	"ClassDescription" : "Records a review, approval or modification",
	"Fields" : {
			"ActivityId" : "int64",
			"UserName" : "string",
			"DateTime" : "object",
			"ActivityType" : "string"
	}
}
)";

			json activity_base_class = jp.parse_object(system_activity_class_string);
			co_await put_class(activity_base_class);

			std::string object_reference_class_string = R"(
{
	"ClassName" : "SysObjectReference",
	"ClassDescription" : "A link to an object",
	"Fields" : {
			"LinkObjectId" : "int64",
			"DeepCopy" : "bool",
			"DeepGet" : "bool"
	}
}
)";

			json object_reference_class = jp.parse_object(object_reference_class_string);
			co_await put_class(object_reference_class);


			co_await header.write(database_file);
			co_return header_location;
		}

		database_transaction<json> get_class(json& _object)
		{
			json class_def;
			json key = _object.extract({ "ClassName" });
			json result;

			class_def = co_await get_class(key);

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

		database_transaction<json> put_class(json _class_definition)
		{
			json result = check_class(_class_definition);

			if (result["Success"]) {
				co_await classes.put(_class_definition);
				result = create_response(true, "Ok");
			}
			co_return result;
		}

		database_transaction<json> get_derived_classes(json _class_definition)
		{
			json_parser jp;
			json derived_classes = jp.create_object();

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

		database_transaction<json> get_ancestor_classes(json _class_definition)
		{
			json_parser jp;
			json ancestor_classes = jp.create_object();

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

		database_transaction<bool> is_descendant_of(std::string _base_class, std::string _class_to_check )
		{
			json_parser jp;
			json class_check = jp.create_object("ClassName", _class_to_check);
			auto ancestors_task = get_ancestor_classes(class_check);
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

		database_transaction<json> get_objects_by_class(json _object_definition)
		{
			json_parser jp;

			json response;
			json objects = jp.create_array();

			json class_def = co_await get_class(_object_definition);
			json derived_classes = co_await get_derived_classes(class_def);
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
					json obj = co_await get_object(get_object_id);
					objects.append_element(obj);
				}
			}

			response = create_response(true, "Ok", objects, 0.0);
			co_return response;
		}

		database_transaction<json> create_object(std::string _class_name)
		{
			json_parser jp;
			json new_object = jp.create_object();
			json class_key = jp.create_object( "ClassName", _class_name );
			json classdef_response = co_await get_class(class_key);
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

		database_transaction<json> put_object(json _object_definition)
		{
			json_parser jp;

			json result = co_await check_object(_object_definition);

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

		database_transaction<json> get_object(json _object_key)
		{
			json_parser jp;

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
							json child_obj = co_await get_object(object_key);
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
					json child_obj = co_await get_object(object_key);
					if (child_obj["Success"]) {
						json data = child_obj["Data"];
						cm.put_member("Data", data);
					}
				}
			}

			co_return response;
		}

		database_transaction<json> copy_object(json _object_key, 
			std::function<bool(json _object_changes)> _fn)
		{
			json_parser jp;
			json object_copy = co_await get_object( _object_key );

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
								json new_object = co_await copy_object(object_key, [](json _changes) -> bool
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
						json new_object = co_await copy_object(object_key, [](json _changes) -> bool
							{
								return true;
							});
						cm.put_member_i64("LinkObjectId", new_object["ObjectId"]);
					}
				}

				json result = co_await put_object(new_object);

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

		database_transaction<json> delete_object(json _object_key)
		{
			json ret;
			json_parser jp;

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
								json new_object = co_await delete_object(object_key);
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
						json new_object = co_await delete_object(object_key);
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
}

#endif
