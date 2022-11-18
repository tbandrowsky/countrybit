#pragma once

namespace corona
{
	namespace database
	{

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

		class network_status_response : public db_response
		{
		public:

		};

		class collection_status_response : public db_response
		{
		public:

		};

		class schema_response : public db_response
		{
		public:

		};

		class log_response : public db_response
		{
		public:

		};


		class jdatabase
		{
			persistent_box					database_box;
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
						database_box.commit();
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
					database_box.commit();
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
					database_box.commit();
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
					database_box.commit();
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

			task<db_response> open(open_db_request _open);
			task<db_response> create(create_db_request _create);

			network_status_response get_network_status();
			collection_status_response get_collection_status();
			schema_response get_schema();
			log_response get_log();

			field_response put_string_field(put_string_field_request request);
			field_response put_time_field(put_time_field_request request);
			field_response put_integer_field(put_integer_field_request request);
			field_response put_double_field(put_double_field_request request);
			field_response put_point_field(put_point_field_request request);
			field_response put_rectangle_field(put_rectangle_field_request request);
			field_response put_image_field(put_image_field_request request);
			field_response put_wave_field(put_wave_field_request request);
			field_response put_midi_field(put_midi_field_request request);
			field_response put_color_field(put_color_field_request request);
			field_response get_field(object_name name);

			class_response put_class(put_class_request request);
			class_response get_class(object_name name);

			collection_response create_collection(create_collection_request _create_collection);
			collection_response get_collection(object_name _name);
			collection_response get_collection(collection_id_type _id);

		};
	}
};

