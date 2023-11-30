#ifndef CORONA_DATAPLANE_H
#define CORONA_DATAPLANE_H

#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-json.hpp"
#include "corona-httpclient.hpp"

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

	using json_method = std::function<sync<int>(json _params, data_set *_set)>;
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

		ui_task get(json _params)
		{
			ui_task uit;
			parameters = _params;

			uit.configure(
				_params,
				[this](json _data) {
					time_t current_time;
					time(&current_time);
					time_t seconds = current_time - last_refresh;

					if (seconds >= cache_seconds) {
						last_refresh = current_time;
						fetch(_data, this);
					}
				},
				[this](json _data) {
					share(_data, this);
				}
			);

			return uit;
		}

		ui_task get()
		{
			ui_task uit;

			uit.configure(
				parameters,
				[this](json _data) {
					time_t current_time;
					time(&current_time);
					time_t seconds = current_time - last_refresh;

					if (seconds >= cache_seconds) {
						last_refresh = current_time;
						fetch(_data, this);
					}
				},
				[this](json _data) {
					share(_data, this);
				}
			);

			return uit;
		}

	};

	class data_plane
	{
	public:
		std::map<std::string, data_set> data_sets;
		std::map<std::string, std::shared_ptr<data_source>> data_sources;
		std::map<std::string, int> control_ids;
		std::map<std::string, control_json_mapper> class_control_map;

		int get_control_id(std::string _name, std::function<int()> _id)
		{
			if (!control_ids.contains(_name)) {
				control_ids.insert_or_assign(_name, _id());
			}
			return control_ids[_name];
		}

		sync<json> get(std::string _name)
		{
			json set;
			if (data_sets.contains(_name)) {
				auto& ds = data_sets[_name];
				co_await ds.get();
				set = ds.data;
			}
			co_return set;
		}

		control_json_mapper get_class_control_factory(std::string class_name)
		{
			control_json_mapper cjm;

			if (class_control_map.contains(class_name)) {
				cjm = class_control_map[class_name];
			}
			return cjm;
		}

		void put_class_control_factory(std::string class_name, control_json_mapper mapper)
		{
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
			if (!data_sources.contains(_source_name)) {
				throw std::invalid_argument("Invalid source name for dataset");
			}
			data_set ds;
			ds.source = data_sources[_source_name];
			ds.cache_seconds = _cache_seconds;
			ds.fetch = _fetch;
			ds.share = _gui;
			ds.name = _set_name;
			data_sets.insert_or_assign(_set_name, ds);
		}

		void put_data_set(
			std::string _source_name,
			std::string _set_name,
			json _data
		)
		{

			if (!data_sources.contains(_source_name)) {
				throw std::invalid_argument("Invalid source name for dataset");
			}

			data_set ds;
			ds.source = data_sources[_source_name];
			ds.cache_seconds = 3600;
			ds.data = _data;

			using json_method = std::function<sync<int>(json _params, data_set* _set)>;
			using gui_method = std::function<int(json _params, data_set* _set)>;

			ds.fetch = [](json _params, data_set* _set)->sync<int> {				
				co_return 0;
				};

			ds.share = [](json _params, data_set* _set)->int {
				return 0;
			};

			ds.name = _set_name;
			data_sets.insert_or_assign(_set_name, ds);
		}

		sync<int> query(json sets)
		{
			int count = 0;
			for (int i = 0; i < sets.size(); i++)
			{
				auto set = sets[i];
				std::string setName = set["set"];
				json params = set["param"];
				if (data_sets.contains(setName)) {
					auto& ds = data_sets[setName];
					json result = co_await ds.get(params);
					set.put_member("result", result );
					count++;
				}
			}
			co_return count;
		}
	};
}

#endif
