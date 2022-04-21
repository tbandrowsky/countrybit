#pragma once

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>

#include "jobject.h"
#include "application.h"
#include "messages.h"

namespace countrybit
{
	namespace database
	{

		using collection_table_type = table<jcollection_ref>;
		using collections_by_name_type = sorted_index<object_name, row_id_type, 1>;
		using collections_by_id_type = sorted_index<collection_id_type, row_id_type, 1>;

		struct jdatabase_control_map
		{
		public:
			char		file_kind[32];
			row_id_type schema_location;
			row_id_type collections_location;
			row_id_type collections_by_name_location;
			row_id_type collections_by_id_location;
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
			int num_collections;
			int num_classes;
			int num_models;

			object_path database_filename;
			object_path database_folder;
		};

		class db_response : public system::base_result
		{
		public:
			dynamic_box response_data;
			system::os_result os_code;
		};

		class create_collection_request 
		{
		public:
			object_name model_name;
			object_name collection_name;
			int max_actors;
			int max_objects;
		};

		class get_collection_request
		{
		public:
			object_name collection_name;
			collection_id_type collection_id;
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

		class model_response : public db_response
		{
		public:
			jmodel info;
		};

		class actor_response : public db_response
		{
		public:
			jactor info;
		};

		class command_response : public db_response
		{
		public:
			actor_command_response info;
		};

		class jdatabase
		{

			dynamic_box						database_box;

			jschema							schema;
			collection_table_type			collections;
			collections_by_name_type		collections_by_name;
			collections_by_id_type			collections_by_id;

			system::application				*application;
			jdatabase_control_map			*map;

			template<typename request_type> field_response field_invoke(std::function<row_id_type(request_type& _request)> fn, request_type& _request)
			{
				field_response response;

				try 
				{
					row_id_type fid = fn(_request);
					if (fid != null_row) 
					{
						response.info = schema.get_field(fid);
					}
					else 
					{
						response.message = "Could not manage field [" + _request.name + "]";
					}
				}
				catch (std::logic_error& le)
				{
					response.message = "Could not manage field [" + _request.name + "]: " + le.what();
				}
				catch (std::invalid_argument& ie)
				{
					response.message = "Could not manage field [" + _request.name + "]: " + ie.what();
				}
				catch (std::exception& exc)
				{
					response.message = "Could not manage field [" + _request.name + "]: " + exc.what();
				}

				return response;
			}

			template<typename request_type> class_response class_invoke(std::function<row_id_type(request_type& _request)> fn, request_type& _request)
			{
				class_response response;

				try
				{
					row_id_type cid = fn(_request);
					if (cid != null_row)
					{
						response.info = schema.get_class(cid);
					}
					else
					{
						response.message = "Could not manage class [" + _request.name + "]";
					}
				}
				catch (std::logic_error& le)
				{
					response.message = "Could not manage class [" + _request.name + "]: " + le.what();
				}
				catch (std::invalid_argument& ie)
				{
					response.message = "Could not manage class [" + _request.name + "]: " + ie.what();
				}
				catch (std::exception& exc)
				{
					response.message = "Could not manage class [" + _request.name + "]: " + exc.what();
				}

				return response;
			}

			template<typename request_type> model_response model_invoke(std::function<row_id_type(request_type& _request)> fn, request_type& _request)
			{
				model_response response;

				try
				{
					row_id_type cid = fn(_request);
					if (cid != null_row)
					{
						response.info = schema.get_model(cid);
					}
					else
					{
						response.message = "Could not manage model [" + _request.name + "]";
					}
				}
				catch (std::logic_error& le)
				{
					response.message = "Could not manage model [" + _request.name + "]: " + le.what();
				}
				catch (std::invalid_argument& ie)
				{
					response.message = "Could not manage model [" + _request.name + "]: " + ie.what();
				}
				catch (std::exception& exc)
				{
					response.message = "Could not manage model [" + _request.name + "]: " + exc.what();
				}

				return response;
			}

		public:

			jdatabase(system::application* _application);
			~jdatabase();

			system::task<db_response> open(open_db_request _open);
			system::task<db_response> create(create_db_request _create);

			field_response put_string_field(put_string_field_request request);
			field_response put_time_field(put_time_field_request request);
			field_response put_integer_field(put_integer_field_request request);
			field_response put_double_field(put_double_field_request request);
			field_response put_query_field(put_named_query_field_request request);
			field_response put_sql_remote_field(put_named_sql_remote_field_request request);
			field_response put_http_remote_field(put_named_http_remote_field_request request);
			field_response put_file_remote_field(put_named_file_remote_field_request request);
			field_response put_point_field(put_point_field_request request);
			field_response put_rectangle_field(put_rectangle_field_request request);
			field_response put_image_field(put_image_field_request request);
			field_response put_wave_field(put_wave_field_request request);
			field_response put_midi_field(put_midi_field_request request);
			field_response put_color_field(put_color_field_request request);
			field_response get_field(object_name name);

			class_response put_class(put_class_request request);		
			class_response get_class(object_name name);

			model_response put_model(jmodel request);
			model_response get_model(object_name name);

			collection_response create_collection(create_collection_request _create_collection);
			collection_response get_collection(get_collection_request _get_collection);

			actor_response put_actor(jactor _actor);
			actor_response get_actor(jactor _actor_id);

			command_response get_command_result(row_id_type _actor);
			command_response select_object(const actor_select_object& _select);
			command_response create_object(actor_create_object& _create);
			command_response update_object(actor_update_object& _update);
			
		};
	}
};

