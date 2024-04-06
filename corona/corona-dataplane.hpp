#ifndef CORONA_DATAPLANE_H
#define CORONA_DATAPLANE_H

namespace corona
{
	class data_api;
	class data_lake;
	class data_function;

	using json_method = std::function<call_status(json _params, data_lake *_api, data_function* _set)>;
	using gui_method = std::function<int(json _params, data_lake *_api, data_function* _set)>;

	struct dataplane_promise;

	class data_function
	{
	public:

		data_api*						api;
		std::string						name;
		json							data;
		json							parameters;
		time_t							last_refresh = 0;
		int								cache_seconds = 0;
		json_method						fetch;
		gui_method						share;
		call_status						status;
		bool							busy;
		std::vector<gui_method>			on_changed;

		data_function()
		{
			busy = false;
			json_parser jp;
			data = jp.create_object();
		}

		void run_share();
		int get(data_lake* _lake, json _params);
		void get(data_lake* _lake);

	};

	struct dataplane_task_ui
	{
	public:
		std::shared_ptr<data_function> fn;
		data_lake* lake;
		json params;

		void operator()()
		{
			fn->run_share();
		}
	};

	struct dataplane_task : public std::suspend_always
	{
	public:
		using promise_type = dataplane_promise;

		dataplane_task_ui dptui;

		void configure(data_lake* _lake, std::shared_ptr<data_function> _fn, json _params = {})
		{
			dptui.fn = _fn;
			dptui.lake = _lake;
			dptui.params = _params;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			threadomatic::run_complete(
				[this, handle]() {
					if (dptui.params.is_object())
					{
						dptui.fn->get(dptui.lake, dptui.params);
					}
					else
					{
						dptui.fn->get(dptui.lake);
					}
					handle();
				},
				dptui
			);
		}

		void await_resume()
		{
			std::cout << "dataplane_task await_resume:" << GetCurrentThreadId() << std::endl;
		}

		dataplane_task()
		{
			std::cout << this << ", dataplane_task ctor:" << GetCurrentThreadId() << std::endl;
		}

		~dataplane_task()
		{
			std::cout << this << ", dataplane_task destroyed:" << GetCurrentThreadId() << std::endl;
		}

	};

	struct dataplane_promise
	{
		dataplane_promise()
		{
			std::cout << "dataplane_task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
		}

		dataplane_task get_return_object()
		{
			std::cout << "dataplane_task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
			dataplane_task my_task;
			return my_task;
		};

		std::suspend_always initial_suspend() {
			std::cout << "dataplane_task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
			return {};
		}

		std::suspend_always final_suspend() noexcept {
			std::cout << "dataplane_task final_suspend:" << this << GetCurrentThreadId() << std::endl;
			return {};
		}

		void unhandled_exception() {
			std::cout << "dataplane_task exception:" << this << GetCurrentThreadId() << std::endl;
		}

	};


	class data_api
	{
		lockable api_lock;
		std::map<std::string, std::shared_ptr<data_function>> functions;

	public:

		data_lake* parent;

		std::string name;
		std::string description;
		std::string icon_path;

		friend class data_lake;

		void put_function(
			std::string _function_name,
			json_method _fetch,
			gui_method _gui,
			int _cache_seconds
		)
		{
			scope_lock locker(api_lock);

			std::shared_ptr<data_function> ds;

			if (functions.contains(_function_name))
			{
				ds = functions[_function_name];
			}
			else
			{
				ds = std::make_shared<data_function>();
				functions.insert_or_assign(_function_name, ds);
			}

			ds->api = this;
			ds->cache_seconds = _cache_seconds;
			ds->fetch = _fetch;
			ds->share = _gui;
			ds->name = _function_name;
		}

		void put_function(
			std::string _function_name,
			json _data
		)
		{
			scope_lock locker(api_lock);

			std::shared_ptr<data_function> ds;

			if (functions.contains(_function_name))
			{
				ds = functions[_function_name];
			}
			else
			{
				ds = std::make_shared<data_function>();
				functions.insert_or_assign(_function_name, ds);
			}

			ds->api = this;
			ds->cache_seconds = 3600;
			ds->data = _data;
			ds->name = _function_name;

			ds->fetch = [](json _params, data_lake* _lake, data_function* _set)->call_status
			{
				call_status dfs;

				return dfs;
			};

			ds->share = [](json _params, data_lake* _lake, data_function* _set)->int {
				return 0;
				};

		}

		call_status get_status(std::string _name)
		{
			auto spdf = get_function(_name);
			return spdf->status;
		}

		std::shared_ptr<data_function> get_function(std::string _name)
		{
			scope_lock locker(api_lock);

			std::shared_ptr<data_function> sp;
			if (functions.contains(_name)) {
				sp = functions[_name];
			}
			return sp;
		}

		void call_function(data_lake* _lake, std::string _name)
		{
			std::shared_ptr<data_function> dsp = get_function(_name);

			if (dsp) {
				dsp->get(_lake);
			}
		}

		void call_function(data_lake* _lake, std::string _name, json _parameters)
		{
			std::shared_ptr<data_function> dsp = get_function(_name);

			if (dsp) {
				dsp->get(_lake, _parameters);
			}
		}

		json get_result(std::string _name)
		{
			json set;

			std::shared_ptr<data_function> dsp = get_function(_name);

			if (dsp) {
				set = dsp->data;
			}
			return set;
		}

	};

	class data_lake;
	class data_function;

	class data_lake
	{
		lockable lake_lock, log_lock;

		std::map<std::string, std::shared_ptr<data_api>> apis;
		std::vector<gui_method> logevent_bindings;
		json ds_log;

	public:


		data_lake()
		{
			log_clear();
		}

		void logged(json _params, data_lake* _api, data_function* _set)
		{
			for (auto ol : logevent_bindings) {
				ol(_params, _api, _set);
			}
		}

		void on_logged(gui_method _method)
		{
			logevent_bindings.push_back(_method);
		}

		void on_changed(std::string _source, std::string _function, gui_method _method)
		{
			if (_function != "*") {
				auto fnx = get_function(_source, _function);
				fnx->on_changed.push_back(_method);
			}
			else 
			{
				auto apix = get_api(_source);
				for (auto fnx : apix->functions) {
					fnx.second->on_changed.push_back(_method);
				}
			}
		}

		std::shared_ptr<data_api> get_api(std::string _source)
		{
			scope_lock locker(lake_lock);
			std::shared_ptr<data_api> source;

			if (apis.contains(_source))
			{
				source = apis[_source];
			}
			else {
				std::string message = "API " + _source + " not found";
				throw std::invalid_argument(message);
			}
			return source;
		}

		std::shared_ptr<data_function> get_function(std::string _source, std::string _name)
		{
			std::shared_ptr<data_function> fn;

			auto ds = get_api(_source);

			fn = ds->get_function(_name);
			if (!fn) {
				std::string message = "Function " + _name + " not found";
				throw std::invalid_argument(message);
			}

			return fn;
		}

		dataplane_task invoke_function(std::string _source, std::string _name)
		{
			dataplane_task ts;

			std::shared_ptr<data_function> dsp = get_function(_source, _name);

			ts.configure(this, dsp);

			return ts;
		}

		dataplane_task invoke_function(std::string _source, std::string _name, json _parameters)
		{
			dataplane_task ts;

			std::shared_ptr<data_function> dsp = get_function(_source, _name);

			ts.configure(this, dsp, _parameters);

			return ts;
		}

		task call_function(std::string _source, std::string _name, json _parameters)
		{
			try 
			{
				std::cout << "call function -before- " << _name << " on thread " << GetCurrentThreadId() << std::endl;
				co_await invoke_function(_source, _name, _parameters);
				std::cout << "call function -after- " << _name << " on thread " << GetCurrentThreadId() << std::endl;
			}
			catch (std::exception exc)
			{
				std::cout << "exception:" << exc.what() << std::endl;
			}
		}

		task call_function(std::string _source, std::string _name)
		{
			try
			{
				std::cout << "call function -before- " << _name << " on thread " << GetCurrentThreadId() << std::endl;
				co_await invoke_function(_source, _name);
				std::cout << "call function -after- " << _name << " on thread " << GetCurrentThreadId() << std::endl;
			}
			catch (std::exception exc)
			{				
				std::cout << "exception:" << exc.what() << std::endl;
			}
		}

		bool is_busy(std::string _source, std::string _name)
		{
			bool busy = false;
			std::shared_ptr<data_function> dsp = get_function(_source, _name);

			if (dsp) {
				busy = dsp->busy;
			}
			return busy;
		}

		call_status get_status(std::string _source, std::string _name)
		{
			call_status status;

			std::shared_ptr<data_function> dsp = get_function(_source, _name);

			if (dsp) {
				status = dsp->status;
			}

			return status;
		}

		json get_result(std::string _source, std::string _name)
		{
			json set;

			std::shared_ptr<data_function> dsp = get_function(_source, _name);

			if (dsp) {
				set = dsp->data;
			}
			return set;
		}

		void put_api(
			std::string _name,
			std::string _description,
			std::string _icon_path
		)
		{
			scope_lock locker(lake_lock);

			std::shared_ptr<data_api> source;

			if (apis.contains(_name)) 
			{
				source = apis[_name];
			}
			else 
			{
				source = std::make_shared<data_api>();
			}

			source->parent = this;
			source->name = _name;
			source->description = _description;
			source->icon_path = _icon_path;
			apis.insert_or_assign(_name, source);
		}

		void put_function(
			std::string _source_name,
			std::string _set_name,
			json_method _fetch,
			gui_method _gui,
			int _cache_seconds
			)
		{
			scope_lock locker(lake_lock);


			std::shared_ptr<data_api> dapi;

			if (apis.contains(_source_name))
			{
				dapi = get_api(_source_name);
				dapi->put_function(_set_name, _fetch, _gui, _cache_seconds);
			}
		}

		void put_function(
			std::string _source_name,
			std::string _set_name,
			json _data
		)
		{
			scope_lock locker(lake_lock);

			std::shared_ptr<data_api> dapi;

			if (apis.contains(_source_name))
			{
				dapi = get_api(_source_name);
				dapi->put_function(_set_name, _data);
			}
		}

		void log_clear()
		{
			json_parser jp;
			ds_log = jp.create_array();
		}

		void log_information(std::string _source, std::string _function, std::string _status, std::string _message)
		{
			scope_lock lock_me(log_lock);
			json_parser jp;

			auto ds_object = jp.create_object();
			ds_object.put_member("source", _source);
			ds_object.put_member("function", _function);
			ds_object.put_member("status", _status);
			ds_object.put_member("message", _message);
			ds_log.put_element_object(-1, ds_object);
			logged(ds_object, this, nullptr);
		}

		void log_status(std::string _source, std::string _function, call_status _status)
		{
			scope_lock lock_me(log_lock);
			json_parser jp;

			auto ds_object = jp.create_object();
			ds_object.put_member("source", _source);
			ds_object.put_member("function", _function);
			ds_object.put_member("status", "error");
			ds_object.put_member("message", _status.message);
			ds_object.put_member("method", _status.request.http_method);
			ds_object.put_member("host", _status.request.host);
			ds_object.put_member("path", _status.request.path);
			ds_object.put_member("headers", _status.request.headers);
			if (_status.request.body.get_ptr()) {
				std::string body = _status.request.body.get_ptr();
				ds_object.put_member("request_body", body);
			}
			if (_status.response.response_body.get_ptr()) {
				std::string body = _status.response.response_body.get_ptr();
				ds_object.put_member("response_body", body);
			}
			ds_log.put_element_object(-1, ds_object);
			logged(ds_object, this, nullptr);
		}

		json& get_log()
		{
			return ds_log;
		}

	};

	int data_function::get(data_lake* _lake, json _params)
	{
		json_parser jp;
		auto jobj = jp.create_object();

		try
		{
			parameters = _params;

			time_t current_time, time_after;
			time(&current_time);
			time_t seconds = current_time - last_refresh;

			if (seconds >= cache_seconds) {

				last_refresh = current_time;
				busy = true;
				status = fetch(_params, _lake, this);
				time(&time_after);
				time_t exec_seconds = time_after - current_time;
				status.call_time = exec_seconds;
				_lake->log_status(api->name, name, status);
				for (auto cn : on_changed) {
					cn(parameters, _lake, this);
				}

				busy = false;
			}

			data.put_member_object("status", jobj);
		}
		catch (std::logic_error exc)
		{
			_lake->log_information(api->name, name, "Error", exc.what());
			return 0;
		}
		catch (std::exception exc)
		{
			_lake->log_information(api->name, name, "Error", exc.what());
			return 0;
		}
		return 1;
	}

	void data_function::get(data_lake* _lake)
	{
		json_parser jp;
		auto jobj = jp.create_object();

		try
		{
			time_t current_time, time_after;
			time(&current_time);
			time_t seconds = current_time - last_refresh;

			busy = true;
			if (seconds >= cache_seconds) {
				last_refresh = current_time;
				status = fetch(data, _lake, this);
				time(&time_after);
				time_t exec_seconds = time_after - current_time;
				status.call_time = exec_seconds;
				_lake->log_status(api->name, name, status);
				for (auto cn : on_changed) {
					cn(parameters, _lake, this);
				}
			}
			busy = false;
		}
		catch (std::logic_error exc)
		{
			_lake->log_information(api->name, name, "Error", exc.what());
		}
		catch (std::exception exc)
		{
			_lake->log_information(api->name, name, "Error", exc.what());
		}
	}

	void data_function::run_share()
	{
		if (share) {
			share(data, api->parent, this);
		}
	}
}

#endif
