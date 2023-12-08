#ifndef CORONA_DATAPLANE_H
#define CORONA_DATAPLANE_H

#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-json.hpp"
#include "corona-httpclient.hpp"
#include "corona-presentation.hpp"

namespace corona
{
	class data_source
	{
	public:
		std::string name;
		std::string description;
		std::string icon_path;
	};

	class data_plane;
	class data_set;

	using json_method = std::function<int(json _params, data_set *_set)>;
	using gui_method = std::function<int(json _params, data_set* _set)>;

	class data_set
	{
	public:
		std::string						name;
		json							data;
		json							parameters;
		time_t							last_refresh = 0;
		int								cache_seconds = 0;
		json_method						fetch;
		gui_method						share;
		std::shared_ptr<data_source>	source;

		data_set()
		{
			json_parser jp;
			data = jp.create_object();
		}

		void run_share()
		{
			share(data, this);
		}

		void queue_share()
		{
			threadomatic::run_complete([]() {}, [this]() { run_share(); });
		}

		auto get_error()
		{
			struct result { std::string message;  bool error; };
			std::string err_string;
			bool err = false;
			if (data.has_member("status")) {
				err_string = data["status"]["message"];
				err = data["status"]["success"];
			}
			return result{ err_string, err };
		}

		int get(json _params)
		{
			json_parser jp;
			auto jobj = jp.create_object();

			try
			{
				parameters = _params;

				time_t current_time;
				time(&current_time);
				time_t seconds = current_time - last_refresh;

				if (seconds >= cache_seconds) {

					last_refresh = current_time;
					fetch(_params, this);
				}

				jobj.put_member("success", true);
				jobj.put_member("message", "Ok.");
				data.put_member_object("status", jobj);

				queue_share();
			}
			catch (std::logic_error exc)
			{
				jobj.put_member("success", false);
				jobj.put_member("message", exc.what() ? exc.what() : "logic error.");
				data.put_member_object("status", jobj);
				queue_share();
			}
			catch (std::exception exc)
			{
				jobj.put_member("success", false);
				jobj.put_member("message", exc.what() ? exc.what() : "general exception.");
				data.put_member_object("status", jobj);
				queue_share();
				return 0;
			}
			return 1;
		}

		void get()
		{
			json_parser jp;
			auto jobj = jp.create_object();

			try
			{
				time_t current_time;
				time(&current_time);
				time_t seconds = current_time - last_refresh;

				if (seconds >= cache_seconds) {
					last_refresh = current_time;
					fetch(data, this);
				}
				jobj.put_member("success", true);
				data.put_member_object("status", jobj);
				queue_share();
			}
			catch (std::logic_error exc)
			{
				jobj.put_member("success", false);
				jobj.put_member("message", exc.what() ? exc.what() : "logic error.");
				data.put_member_object("status", jobj);
				queue_share();
			}
			catch (std::exception exc)
			{
				jobj.put_member("success", false);
				jobj.put_member("message", exc.what());
				data.put_member_object("status", jobj);
				queue_share();
			}
		}

	};

	class data_plane
	{

		lockable set_lock;
		lockable control_lock;

		std::map<std::string, std::shared_ptr<data_set>> data_sets;
		std::map<std::string, std::shared_ptr<data_source>> data_sources;
		std::map<std::string, int> control_ids;
		std::map<std::string, control_json_mapper> class_control_map;

	public:

		int get_control_id(std::string _name, std::function<int()> _id)
		{
			int temp = 0;
			scope_lock lockit(set_lock);

			if (!control_ids.contains(_name)) {
				control_ids.insert_or_assign(_name, _id());
			}
			return control_ids[_name];
		}

		std::shared_ptr<data_set> get_data_set(std::string _name)
		{
			scope_lock locker(set_lock);

			std::shared_ptr<data_set> sp;
			if (data_sets.contains(_name)) {
				sp = data_sets[_name];
			}
			return sp;
		}

		json get(std::string _name)
		{
			json set;

			std::shared_ptr<data_set> dsp = get_data_set(_name);

			if (dsp) {
				dsp->get();
				set = dsp->data;
			}
			return set;
		}

		control_json_mapper get_class_control_factory(std::string class_name)
		{
			scope_lock locker(control_lock);

			control_json_mapper cjm;

			if (class_control_map.contains(class_name)) {
				cjm = class_control_map[class_name];
			}

			return cjm;
		}

		void put_class_control_factory(std::string class_name, control_json_mapper mapper)
		{
			scope_lock locker(control_lock);

			class_control_map.insert_or_assign(class_name, mapper);
		}

		void put_data_source(
			std::string _name,
			std::string _description,
			std::string _icon_path
		)
		{
			std::shared_ptr<data_source> source;

			if (data_sources.contains(_name)) {
				source = data_sources[_name];
			}
			else 
			{
				source = std::make_shared<data_source>();
			}

			source->name = _name;
			source->description = _description;
			source->icon_path = _icon_path;
			data_sources.insert_or_assign(_name, source);
		}

		void put_data_set(
			std::string _source_name,
			std::string _set_name,
			json_method _fetch,
			gui_method _gui,
			int _cache_seconds
			)
		{
			scope_lock locker(set_lock);

			if (!data_sources.contains(_source_name)) {
				throw std::invalid_argument("Invalid source name for dataset");
			}

			auto ds = std::make_shared<data_set>();
			ds->source = data_sources[_source_name];
			ds->cache_seconds = _cache_seconds;
			ds->fetch = _fetch;
			ds->share = _gui;
			ds->name = _set_name;
			data_sets.insert_or_assign(_set_name, ds);
		}

		void put_data_set(
			std::string _source_name,
			std::string _set_name,
			json _data
		)
		{
			scope_lock locker(set_lock);

			if (!data_sources.contains(_source_name)) {
				throw std::invalid_argument("Invalid source name for dataset");
			}

			auto ds = std::make_shared<data_set>();
			ds->source = data_sources[_source_name];
			ds->cache_seconds = 3600;
			ds->data = _data;

			ds->fetch = [](json _params, data_set* _set)->int {
				return 0;
				};

			ds->share = [](json _params, data_set* _set)->int {
				return 0;
			};

			ds->name = _set_name;
			data_sets.insert_or_assign(_set_name, ds);
		}

	};
}

#endif
