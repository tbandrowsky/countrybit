#ifndef CORONA_JDATABASE_SERVER_H
#define CORONA_JDATABASE_SERVER_H

#include "corona-constants.hpp"
#include "corona-store_box.hpp"
#include "corona-string_box.hpp"
#include "corona-collection_id_box.hpp"
#include "corona-table.hpp"
#include "corona-sorted_index.hpp"
#include "corona-jobject.hpp"
#include "corona-jfield.hpp"
#include "corona-jvalue.hpp"

#include <string>
#include <iostream>
#include <compare>
#include <coroutine>
#include <filesystem>

namespace corona {

	using collection_table_type = table<jcollection_ref>;
	using collections_by_name_type = sorted_index<object_name, collection_id_type, 1>;
	using collections_by_id_type = sorted_index<collection_id_type, jcollection_ref, 1>;

	struct jdatabase_control_map
	{
	public:
		char		file_kind[32];
		relative_ptr_type schema_location;
		relative_ptr_type collections_location;
		relative_ptr_type collections_by_name_location;
		relative_ptr_type collections_by_id_location;
		object_path database_folder;
		object_path filename;
	};

	class open_db_request
	{
	public:
		object_path filename;
	};

	class create_db_request
	{
	public:
		int num_classes;
		int num_fields;
		int total_class_fields;

		object_path database_filename;
		object_path database_folder;
	};

	class db_response : public base_result
	{
	public:
		dynamic_box response_data;
		os_result os_code;
	};

	std::ostream& operator <<(std::ostream& output, db_response& src);

	class create_collection_request
	{
	public:
		object_name model_name;
		object_name collection_name;
		int max_actors;
		int max_objects;
	};

	class collection_response : public db_response
	{
	public:
		jcollection collection;
	};

	class field_response : public db_response
	{
	public:
		jfield info;
	};

	class class_response : public db_response
	{
	public:
		jclass info;
	};

	class query_response : public db_response
	{
	public:
	};

	class jdatabase
	{
		std::shared_ptr<dynamic_box>	database_box;
		jschema							schema;
		collections_by_id_type			collections_by_id;
		collections_by_name_type		collections_by_name;

		application* current_application;
		jdatabase_control_map* map;

		template <typename request_type, typename response_type, typename object_type> response_type schema_invoke(
			const char* _name,
			request_type& _request,
			std::function<relative_ptr_type(request_type& _request)> process_fn,
			std::function<object_type(relative_ptr_type)> get_fn
		)
		{
			response_type response;
			response.success = false;

			try
			{
				relative_ptr_type fid = process_fn(_request);
				if (fid != null_row)
				{
					response.info = get_fn(fid);
					response.success = true;
					database_box->commit();
				}
				else
				{
					response.message = std::format("Could not manage [{}]", _name);
				}
			}
			catch (std::logic_error& le)
			{
				response.message = std::format("Could not manage [{}] :{}", _name, le.what());
			}
			catch (std::exception& exc)
			{
				response.message = std::format("Could not manage [{}] :{}", _name, exc.what());
			}

			return response;
		}

		template <typename response_type, typename object_type> response_type schema_put_named(
			object_type& _request,
			std::function<void(object_type& _request)> process_fn,
			std::function<object_type(object_name& _name)> get_fn
		)
		{
			response_type response;
			response.success = false;

			try
			{
				process_fn(_request);
				response.info = get_fn(_request.name);
				database_box->commit();
			}
			catch (std::logic_error& le)
			{
				response.message = std::format("Could not manage [{}] :{}", _request.name.c_str(), le.what());
			}
			catch (std::exception& exc)
			{
				response.message = std::format("Could not manage [{}] :{}", _request.name.c_str(), exc.what());
			}

			return response;
		}

		template <typename response_type, typename object_type> response_type schema_get_by_name(
			object_name& _name,
			std::function<object_type(object_name& _name)> get_fn
		)
		{
			response_type response;
			response.success = false;

			try
			{
				response.info = get_fn(_name);
				database_box->commit();
			}
			catch (std::logic_error& le)
			{
				response.message = std::format("Could not manage [{}] :{}", _name.c_str(), le.what());
			}
			catch (std::exception& exc)
			{
				response.message = std::format("Could not manage [{}] :{}", _name.c_str(), exc.what());
			}

			return response;
		}

		template<typename request_type> field_response field_invoke(const char* _name, std::function<relative_ptr_type(request_type& _request)> fn, request_type& _request)
		{
			return schema_invoke<request_type, field_response, jfield>(_name, _request, fn, [this](relative_ptr_type id) { return schema.get_field(id); });
		}

		template<typename request_type> class_response class_invoke(const char* _name, std::function<relative_ptr_type(request_type& _request)> fn, request_type& _request)
		{
			return schema_invoke<request_type, class_response, jclass>(_name, _request, fn, [this](relative_ptr_type id) { return schema.get_class(id); });
		}

		template <typename request_type, typename response_type, typename object_type> response_type collection_invoke(
			const char* _name,
			request_type& _request,
			std::function<relative_ptr_type(jcollection& _collection, request_type& _request)> process_fn,
			std::function<object_type(jcollection& _collection, relative_ptr_type)> get_fn
		)
		{
			response_type response;
			response.success = false;

			try
			{
				auto collection_response = get_collection(_request.collection_id);
				if (!collection_response.success)
				{
					response.message = "Invalid collection";
					return response;
				}

				jcollection collection = collection_response.collection;

				relative_ptr_type fid = process_fn(collection, _request);
				if (fid != null_row)
				{
					response.info = get_fn(collection, fid);
					response.success = true;
				}
				else
				{
					response.message = std::format("Could not manage [{}]", _name);
				}
				database_box->commit();
			}
			catch (std::logic_error& le)
			{
				response.message = std::format("Could not manage [{}] :{}", _name, le.what());
			}
			catch (std::exception& exc)
			{
				response.message = std::format("Could not manage [{}] :{}", _name, exc.what());
			}

			return response;
		}

	public:

		jdatabase(application* _application);
		~jdatabase();

		db_response open(open_db_request _open);
		db_response create(create_db_request _create);

		field_response put_string_field(put_string_field_request request);
		field_response put_time_field(put_time_field_request request);
		field_response put_integer_field(put_integer_field_request request);
		field_response put_double_field(put_double_field_request request);
		field_response put_image_field(put_image_field_request request);
		field_response put_wave_field(put_wave_field_request request);
		field_response put_midi_field(put_midi_field_request request);
		field_response get_field(object_name name);

		class_response put_class(put_class_request request);
		class_response get_class(object_name name);

		collection_response create_collection(create_collection_request _create_collection);
		collection_response get_collection(object_name _name);
		collection_response get_collection(collection_id_type _id);

		query_response query(const std::string& _query_expression);

	};

	jdatabase::jdatabase(application* _application) : current_application(_application)
	{
		database_box = std::make_shared<dynamic_box>();
	}

	jdatabase::~jdatabase()
	{
		;
	}

	std::ostream& operator <<(std::ostream& output, db_response& src)
	{
		output << src.os_code << " " << src.message;
		return output;
	}

	db_response jdatabase::open(open_db_request _open)
	{
		int success = 1;
		map = database_box->get_object<jdatabase_control_map>(0);
		collections_by_id = collections_by_id_type::get_sorted_index(database_box, map->collections_by_id_location);
		db_response jfr;
		jfr.success = true;
		return jfr;
	}

	db_response jdatabase::create(create_db_request _create)
	{
		os_result last_err;

		int success = 1;

		int64_t schema_size_bytes = jschema::get_box_size(_create.num_classes, _create.num_fields, _create.total_class_fields, true);
		int64_t collection_by_name_bytes = collections_by_name_type::get_box_size();
		int64_t collection_by_id_bytes = collections_by_id_type::get_box_size();

		map = database_box->allocate<jdatabase_control_map>(1);

		std::filesystem::path dbpath = _create.database_filename.c_str();
		std::filesystem::path dbfolder = dbpath.parent_path();

		map->filename = dbpath;
		map->database_folder = dbfolder;

		schema = jschema::create_schema(database_box, _create.num_classes, true, map->schema_location);
		collections_by_id = collections_by_id_type::create_sorted_index(database_box, map->collections_by_id_location);
		collections_by_name = collections_by_name_type::create_sorted_index(database_box, map->collections_by_name_location);
		database_box->commit();

		db_response jfr;
		jfr.success = true;
		return jfr;
	}

	collection_response jdatabase::create_collection(create_collection_request _create_collection)
	{
		collection_response response;

		try {

			if (_create_collection.collection_name.has_any("./\\:"))
			{
				response.success = false;
				response.message = "Invalid collection name.  Collection names may not contain path characters.";
				return response;
			}

			if (collections_by_name.contains(_create_collection.collection_name))
			{
				response.success = false;
				response.message = "Collection exists";
				return response;
			}

			jcollection_ref new_collection;
			new_collection.collection_name = _create_collection.collection_name;
			new_collection.model_name = _create_collection.model_name;
			new_collection.max_objects = _create_collection.max_objects;
			new_collection.data = nullptr;

			if (!init_collection_id(new_collection.collection_id))
			{
				response.success = false;
				response.message = "Could not create collection id.";
				return response;
			}

			std::filesystem::path collection_path = map->database_folder.c_str();
			collection_path /= new_collection.collection_name.c_str();
			collection_path /= ".corc";
			new_collection.collection_file_name = collection_path;

			collections_by_id.insert_or_assign(new_collection.collection_id, new_collection);
			collections_by_name.insert_or_assign(new_collection.collection_name, new_collection.collection_id);

			database_box->commit();

			response = get_collection(new_collection.collection_id);
		}
		catch (std::exception exc)
		{
			response.success = false;
			response.message = exc.what();
		}

		return response;
	}

	collection_response jdatabase::get_collection(object_name _name)
	{
		collection_response response;
		try {

			response.success = false;
			if (!collections_by_name.contains(_name)) {
				response.message = "[" + _name + "] not found";
				return response;
			}
			auto collection_id = collections_by_name[_name].second;
			response = get_collection(collection_id);
		}
		catch (std::exception exc)
		{
			response.success = false;
			response.message = exc.what();
		}
		return response;
	}

	collection_response jdatabase::get_collection(collection_id_type _id)
	{
		collection_response response;
		if (!collections_by_id.contains(_id)) {
			response.message = "collection not found";
			return response;
		}
		auto ref = collections_by_id[_id];
		if (ref.second.data.get() == nullptr) {
			ref.second.data = std::make_shared<dynamic_box>(1<<20);
		}
		response.collection = jcollection(&schema, &ref.second);
		response.success = true;
		return response;
	}

	field_response jdatabase::put_string_field(put_string_field_request request)
	{
		return field_invoke<put_string_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_string_field(r); }, request);
	}

	field_response jdatabase::put_time_field(put_time_field_request request)
	{
		return field_invoke<put_time_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_time_field(r); }, request);
	}

	field_response jdatabase::put_integer_field(put_integer_field_request request)
	{
		return field_invoke<put_integer_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_integer_field(r); }, request);
	}

	field_response jdatabase::put_double_field(put_double_field_request request)
	{
		return field_invoke<put_double_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_double_field(r); }, request);
	}

	field_response jdatabase::put_image_field(put_image_field_request request)
	{
		return field_invoke<put_image_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_image_field(r); }, request);
	}

	field_response jdatabase::put_midi_field(put_midi_field_request request)
	{
		return field_invoke<put_midi_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_midi_field(r); }, request);
	}

	field_response jdatabase::put_wave_field(put_wave_field_request request)
	{
		return field_invoke<put_wave_field_request>(request.name.name.c_str(), [this](auto& r) { return schema.put_wave_field(r); }, request);
	}

	field_response jdatabase::get_field(object_name name)
	{
		return field_invoke<object_name>(name.c_str(), [this](auto& r) { return schema.find_field(r); }, name);
	}

	class_response jdatabase::put_class(put_class_request request)
	{
		return class_invoke<put_class_request>(request.class_name.c_str(), [this](auto& r) { return schema.put_class(r); }, request);
	}

	class_response jdatabase::get_class(object_name name)
	{
		return class_invoke<object_name>(name.c_str(), [this](auto& r) { return schema.find_class(r); }, name);
	}

	class query_element
	{
	public:
		std::weak_ptr<query_element> parent;
		std::string body;
		std::vector<std::shared_ptr<query_element>> children;
	};

	class query_statement : public query_element {		};
	class class_expression : public query_element {		};
	class class_where_expression : public query_element {		};
	class field_expression : public query_element {		};
	class constant_expression : public query_element {		};

	query_response jdatabase::query(const std::string& _query_expression)
	{
		query_response response;
		return response;
	}

	/*
	std::stack<std::shared_ptr<query_element>> items;

	std::shared_ptr<query_statement> pi = std::make_unique<query_statement>();

	items.push(pi);

	for (auto c : _query_expression)
	{
		auto current_item = items.top();
		if (std::isalpha(c) || std::isalnum(c))
		{
			if (current_item->is_class_process_item() ||
				current_item->is_field_process_item() ||
				current_item->is_string_process_item()) {
				current_item->add(c);
			}
			else if (current_item->is_query_process_item())
			{

			}
			else if (current_item->is_where_process_item())
			{

			}
		}
		else if (c == '[')
		{
			if (current_item->is_string_process_item())
			{
				current_item->add(c);
			}
			else if (current_item->is_class_process_item())
			{
				where_process_item wpi = std::make_unique<where_process_item>();
				items.push(wpi);
			}
			else
			{
				current_item->add(c);
			}
		}
		else if (c == ']')
		{

		}
		else if (c == '(')
		{

		}
		else if (c == ')')
		{

		}
		else if (c == '"')
		{

		}
		else if (c == '{')
		{

		}
		else if (c == '}')
		{

		}
		else if (c == '@')
		{

		}
		else if (std::isspace(c))
		{

		}
		*/

}

#endif
