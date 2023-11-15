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

	using json_method = std::function<task<json>(json _params)>;

	class data_set
	{
	public:
		std::string						name;
		json							data;
		time_t							last_refresh = 0;
		int								cache_seconds = 0;
		json_method						update;
		std::shared_ptr<data_source>	source;

		task<json> get(json _params)
		{
			json result;

			time_t current_time;
			time(&current_time);
			time_t seconds = current_time - last_refresh;

			if (seconds >= cache_seconds) {
				last_refresh = current_time;
				data = update(_params);
			}

			result = data;
			co_return result;
		}
	};

	class data_plane
	{
	public:
		std::map<std::string, data_set> data_sets;

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
