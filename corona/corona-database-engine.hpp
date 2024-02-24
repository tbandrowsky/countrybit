#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

#include "corona-json-table.hpp"

namespace corona
{

	class corona_db_header_struct
	{
	public:
		int64_t object_id;
		relative_ptr_type directory_location;
		relative_ptr_type classes_location;
		relative_ptr_type frames_location;
		relative_ptr_type classes_index_location;
		relative_ptr_type fields_index_location;

		corona_db_header_struct() 
		{
			object_id = -1;
			directory_location = -1;
			classes_location = -1;
			frames_location = -1;
			classes_index_location = -1;
			fields_index_location = -1;
		}
	};

	using corona_db_header = poco_node<corona_db_header_struct>;

	class corona_database
	{
		corona_db_header header;

		json_table classes;
		json_table frames;
		json_node classes_index;

		file* database_file;

		std::map<std::string, bool> allowed_field_types = {
			{ "object", true },
			{ "array", true },
			{ "number", true },
			{ "string", true },
			{ "int64", true },
			{ "datetime", true },
			{ "objectref", true }
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

		json check_class(json _class_definition)
		{
			json result;

			result = create_response(true, "Ok");

			if (_class_definition.has_member("BaseClass"))
			{
				std::string bc = _class_definition["BaseClass"];

				if (!classes_index.data.has_member(bc)) 
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
							return result;
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

			return result;
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

			json class_definition = co_await get_class(class_name);

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
			frames(_file)
		{
			;
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

		database_transaction<relative_ptr_type> create_database()
		{
			header.object_id = 1;
			relative_ptr_type header_location = co_await header.append(database_file);

			header.data.object_id = 1;
			header.data.classes_location = co_await classes.create();
			header.data.frames_location = co_await frames.create();
			header.data.classes_index_location = co_await classes_index.append(database_file);

			co_await header.write(database_file);
			co_return header_location;
		}

		database_transaction<json> get_class(std::string _name)
		{
			json result;

			if (classes_index.data.has_member(_name)) {
				db_object_id_type class_id = classes_index.data[_name].get_int64();
				result = co_await get_class(class_id);
			}
			else 
			{
				result = create_response(false, "Undefined class", _name);
			}

			co_return result;
		}

		database_transaction<json> get_class(db_object_id_type _class_id)
		{
			json return_obj;

			bool has_class = co_await classes.contains(_class_id);

			if (!has_class) 
			{
				json_node class_node = co_await classes.get(_class_id);
				return_obj = create_response(true, "Ok", class_node.data, 0);
			}
			else 
			{
				return_obj = create_response(false, "Class not found" );
			}
			co_return return_obj;
		}

		database_transaction<json> put_class(json _class_definition)
		{
			json result = check_class(_class_definition);

			if (result["Success"]) {
				db_object_id_type object_id;
				if (_class_definition.has_member("ObjectId")) {
					object_id = _class_definition["ObjectId"];
				}
				else 
				{
					object_id = co_await get_next_object_id();
					_class_definition.put_member_i64("ObjectId", object_id);
				}
				std::string className = _class_definition["ClassName"];
				classes_index.data.put_member_i64(className, object_id);
				co_await classes_index.write(database_file);
				result = create_response(true, "Ok");
			}
			co_return result;
		}

		database_transaction<json> create_object(std::string _class_name)
		{
			json_parser jp;
			json new_object = jp.create_object();
			json classdef_response = co_await get_class(_class_name);
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
						else if (field_type == "objectref")
						{
							new_object.put_member_object(member.first);
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

					response = create_response(true, "Created", new_object, 0);
				}
			}
			co_return response;
			
		}

		database_transaction<json> put_object(json _object_definition)
		{
			json result = co_await check_object(_object_definition);

			if (result["Success"])
			{
				json obj = result["Data"];
				json class_def = result["ClassDefinition"];

				relative_ptr_type table_location = -1;
				table_location = class_def["TableLocation"];
				db_object_id_type object_id = -1;

				if (obj.has_member("ObjectId")) 
				{
					object_id = obj["ObjectId"];
				}
				else 
				{
					object_id = co_await get_next_object_id();
				}

				json_table jt(database_file);
				jt.open(table_location);
				co_await jt.put(object_id, _object_definition);
			}

			co_return result;
		}

		database_transaction<json> get_object(json _object_search)
		{

			json response;

			std::string			class_name;
			db_object_id_type	object_id;

			if (!_object_search.is_object() || !_object_search.has_member("ClassName") || !_object_search.has_member("ObjectId"))
			{
				response = create_response(false, "Invalid get_object, must have ClassName and ObjectId as members", _object_search, 0 );
				co_return response;
			}

			class_name = _object_search["ClassName"];
			object_id = _object_search["ObjectId"];

			json class_def = co_await get_class(class_name);

			if (class_def["Success"])
			{
				json obj = class_def["Data"];
				json class_def = class_def["ClassDefinition"];

				relative_ptr_type table_location = -1;
				table_location = class_def["TableLocation"];

				json_table jt(database_file);
				jt.open(table_location);
				co_await jt.get(object_id, _object_definition);
			}

			co_return _object_definition;
		}

		database_transaction<json> copy_object(db_object_id_type _object_id, json _object_changes)
		{
			json object_copy = co_await get_object( _object_id );

			co_await object_copy.put_member("ObjectId", object_id);

			co_return object_copy;
		}

		database_transaction<json> delete_object(json _class_definition)
		{
			;
		}

	};
}

#endif
