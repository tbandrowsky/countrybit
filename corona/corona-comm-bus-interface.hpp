#pragma once

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
interface, in the sense of, junk declared as interfaces to solve foward
declaration problems.  idea was originally to derive both the app and service
buses from the same thing.

Notes

For Future Consideration
*/

#ifndef CORONA_COMM_BUS_INTERFACE_H
#define CORONA_COMM_BUS_INTERFACE_H

namespace corona
{

	class corona_bus_command;
	class control_base;

	class event_waiter
	{
	public:
		HANDLE hevent;

		event_waiter() : hevent(INVALID_HANDLE_VALUE)
		{
			hevent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		event_waiter(const event_waiter& _waiter)
		{
			::DuplicateHandle(GetCurrentProcess(),
				_waiter.hevent,
				GetCurrentProcess(),
				&hevent,
				0,
				FALSE,
				DUPLICATE_SAME_ACCESS);
		}

		event_waiter operator = (const event_waiter& _src)
		{
			::DuplicateHandle(GetCurrentProcess(),
				_src.hevent,
				GetCurrentProcess(),
				&hevent,
				0,
				FALSE,
				DUPLICATE_SAME_ACCESS);
			return *this;
		}

		event_waiter(event_waiter&& _src)
		{
			hevent = _src.hevent;
			_src.hevent = INVALID_HANDLE_VALUE;
		}

		event_waiter& operator = (event_waiter&& _src)
		{
			hevent = _src.hevent;
			_src.hevent = INVALID_HANDLE_VALUE;
			return *this;
		}

		virtual ~event_waiter()
		{
			if (hevent != INVALID_HANDLE_VALUE)
				::CloseHandle(hevent);
		}

		void wait()
		{
			::WaitForSingleObject(hevent, INFINITE);
		}
	};

	class topic_event_waiter : public event_waiter
	{
	public:
		std::string topic;

		topic_event_waiter()
		{

		}
	};


	class windows_event_waiter : public event_waiter
	{
	public:
		UINT msg;
		UINT control_id;

		windows_event_waiter() : msg(0), control_id(0)
		{

		}
	};

	template <typename transaction_result>
	class comm_bus_transaction
	{
	public:

		struct promise_type
		{
			transaction_result m_value;

			promise_type()
			{
				m_value = {};
				debug_functions&& std::cout << "commbus_transaction::promise:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			comm_bus_transaction  get_return_object() {
				debug_functions&& std::cout << "commbus_transaction::get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				comm_bus_transaction  fbr(promise_coro);
				return fbr;
			}

			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }

			void return_value(transaction_result value) {
				debug_functions&& std::cout << "commbus_transaction::promise return_value:" << " " << " " << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "commbus_transaction::promise unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coro;

		// object manip

		comm_bus_transaction(std::coroutine_handle<promise_type> _promise_coro)
		{
			coro = _promise_coro;
			debug_functions&& std::cout << "commbus_transaction: coro ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		comm_bus_transaction()
		{
			debug_functions&& std::cout << "commbus_transaction: empty ctor:" << ::GetCurrentThreadId() << std::endl;
		}

		// awaiter

		bool await_ready()
		{
			debug_functions&& std::cout << "commbus_transaction::await_ready:" << this << " " << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}

			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			debug_functions&& std::cout << "commbus_transaction::await_suspend:" << this << " " << GetCurrentThreadId() << std::endl;
			handle();
		}

		transaction_result await_resume()
		{
			debug_functions&& std::cout << "commbus_transaction::await_resume:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result;

			try
			{
				if (coro) {
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}

			return result;
		}

		transaction_result wait()
		{
			debug_functions&& std::cout << "commbus_transaction::wait:" << this << " " << GetCurrentThreadId() << std::endl;
			transaction_result result = {};

			try
			{
				if (coro) {
					coro.resume();
					result = coro.promise().m_value;
				}
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_exception(exc);
			}


			debug_functions&& std::cout << "commbus_transaction: complete" << " " << ::GetCurrentThreadId() << std::endl;

			return result;
		}

		operator transaction_result()
		{
			transaction_result result = coro.promise().m_value;
			return result;
		}

	};

	enum class corona_instance
	{
		remote = 0,
		local = 1
	};

	class comm_bus_app_interface : public system_monitoring_interface
	{

		std::multimap<UINT, windows_event_waiter> windows_waiters;
		std::multimap<std::string, topic_event_waiter> topic_waiters;

	public:

		static comm_bus_app_interface* global_bus;

		comm_bus_app_interface()
		{
			if (global_bus == nullptr) {
				global_bus = this;
			}
		}

		virtual ~comm_bus_app_interface()
		{
			if (global_bus == this) {
				global_bus = nullptr;
			}
		}

		std::string default_page;

		virtual void run_app_ui(HINSTANCE hInstance, LPSTR command_line, bool fullScreen) = 0;

		virtual corona_client_response remote_register_user(std::string _user_name, std::string _email, std::string _password1, std::string _password2) = 0;
		virtual corona_client_response remote_confirm_user(std::string _user_name, std::string _confirmation_code) = 0;
		virtual corona_client_response remote_send_user(std::string _user_name) = 0;
		virtual corona_client_response remote_login(std::string _user_name, std::string _password) = 0;
		virtual corona_client_response remote_login() = 0;
		virtual corona_client_response remote_set_password(std::string user_name, std::string validation_code, std::string password1, std::string password2) = 0;
		virtual corona_client_response remote_get_classes() = 0;
		virtual corona_client_response remote_get_class(std::string class_name) = 0;
		virtual corona_client_response remote_put_class(json _class_data) = 0;

		virtual corona_client_response create_object(corona_instance _instance, std::string class_name) = 0;
		virtual corona_client_response run_object(corona_instance _instance, json object_information) = 0;
		virtual corona_client_response edit_object(corona_instance _instance, json object_information) = 0;
		virtual corona_client_response put_object(corona_instance _instance, json object_information) = 0;
		virtual corona_client_response get_object(corona_instance _instance, json object_information) = 0;
		virtual corona_client_response delete_object(corona_instance _instance, json object_information) = 0;
		virtual corona_client_response query_objects(corona_instance _instance, json object_information) = 0;
		virtual void error(json _error) = 0;

		virtual void when(UINT topic, std::function<void()> _runnable) = 0;
		virtual void when(std::string _topic, std::function<void()> _runnable) = 0;

		virtual void select_page(std::string _page, std::string _target_name, json _obj) = 0;
		virtual void select_page(std::string _page, std::string _target_name, std::string _target_page_name, std::string _form_to_load, json _obj) = 0;

		void check_windows_queue(MSG* _msg)
		{
			auto waiters = windows_waiters.find(_msg->message);
			while (waiters != std::end(windows_waiters))
			{
				auto temp_waiter = waiters;
				auto& waiter = *waiters;
				SetEvent(waiter.second.hevent);
				waiters++;
			}
		}

		void check_topic(std::string _topic)
		{
			auto waiters = topic_waiters.find(_topic);
			while (waiters != std::end(topic_waiters))
			{
				auto& waiter = *waiters;
				SetEvent(waiter.second.hevent);
				waiters++;
			}
			topic_waiters.erase(_topic);
		}

		virtual void log_error(corona_client_response& ccr, const char* _file = nullptr, int _line = 0)
		{
			log_warning(ccr.message, _file, _line);
		}

		virtual void log_error(json j, const char* _file = nullptr, int _line = 0)
		{
			std::string msg = j["message"];
			if (msg.empty())
				msg = "Error";
			log_warning(msg, _file, _line);
			log_json<json>(j, 4);
		}

		void run(runnable _runnable)
		{
			general_job* gj = new general_job(_runnable);
			global_job_queue->add_job(gj);
		}

		void run_ui(runnable _ui_complete)
		{
			runnable temp;
			general_ui_job* guj = new general_ui_job(temp, _ui_complete);
			global_job_queue->add_job(guj);
		}

		void run_complete(runnable _runnable, runnable _ui_complete)
		{
			general_ui_job* guj = new general_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
		}

		void run_http(runnable_http_request _runnable, runnable_http_response _ui_complete)
		{
			general_http_ui_job* guj = new general_http_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
		}

		template <typename dest, typename item> void run_each(dest* _targets, std::vector<item>& _items, std::function<void(dest* _target, item& _src)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				general_job* gj = new general_job([_targets, idx, end, _on_each, &_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						item& itm = _items[x];
						dest* d = &_targets[x];
						_on_each(d, itm);
					}
					}, handle);
				global_job_queue->add_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}

		template <typename item> void run_each(std::vector<item>& _items, std::function<void(item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				std::vector<item>* src_items = &_items;
				general_job* gj = new general_job([idx, end, _on_each, src_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						item& itm = (*src_items)[x];
						_on_each(itm);
					}
					}, handle);
				global_job_queue->add_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}

		template <typename item> void run_each(std::vector<item>& _items, int _width, int _height, std::function<void(int _x, int _y, item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				std::vector<item>* src_items = &_items;
				general_job* gj = new general_job([idx, _width, _height, end, _on_each, src_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						int px = x % _width;
						int py = x / _width;
						item& itm = (*src_items)[x];
						_on_each(px, py, itm);
					}
					}, handle);
				global_job_queue->add_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}

		virtual control_base* find_control(int _id) = 0;
		virtual control_base* find_control(std::string _name) = 0;
		virtual json get_form_data(std::string _form_name) = 0;
		virtual void poll(bool _select_default_page) = 0;
		virtual void run_command(std::shared_ptr<corona_bus_command> _command) = 0;
	};

	comm_bus_app_interface* comm_bus_app_interface::global_bus = nullptr;

	class corona_bus_command : public json_serializable
	{
	public:
		corona_bus_command()
		{
			;
		}

		virtual json execute(json context, comm_bus_app_interface* _bus) = 0;

		virtual void get_json(json& _dest)
		{
		}

		virtual void put_json(json& _src)
		{
		}
	};
}

#endif
