/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This implements some of the things of things that can be tossed
onto a threadpool, including awaitable functions,
quick and easy fire and forget with threadsafe ui updates.

Notes

For Future Consideration
*/



#ifndef CORONA_FUNCTION_H
#define CORONA_FUNCTION_H


const int debug_functions = 0;


namespace corona
{

	const int WM_CORONA_JOB_COMPLETE = WM_APP + 1;
	const int WM_CORONA_TASK_COMPLETE = WM_APP + 2;
	const int WM_CORONA_HTTP_JOB_COMPLETE = WM_APP + 3;
	const int WM_CORONA_HTTP_TASK_COMPLETE = WM_APP + 4;
	const int WM_CORONA_RESET = WM_APP + 5;

	template <typename parameter_type> class task_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		std::shared_ptr<parameter_type> params;
		std::function<void (std::shared_ptr<parameter_type>) > on_run;

		task_job() : job()
		{
		}

		task_job(std::coroutine_handle<> _handle, 
			HANDLE _event, 
			std::shared_ptr<parameter_type> _params) : handle(_handle), event(_event), params(_params)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			debug_functions&& std::cout << "job start:" << GetCurrentThreadId() << std::endl;

			try 
			{
				if (on_run) {
					on_run(params);
				}

				if (handle) {
					handle.resume();
					handle.destroy();
				}
			}
			catch (...)
			{
				;
			}

			jn.setSignal(event);
			debug_functions && std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class ui_task_result
	{
	public:
		json parameters;
		runnable on_gui;

		ui_task_result(json _param, runnable _on_gui) : parameters(_param), on_gui(_on_gui)
		{
			;
		}
	};

	class http_task_result
	{
	public:
		call_status status;
		runnable_http_response on_gui;

		http_task_result(call_status _param, runnable_http_response _on_gui) : status(_param), on_gui(_on_gui)
		{
			;
		}
	};

	using runnable_json = std::function<void(json)>;

	class ui_task_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		json params;
		runnable on_gui;
		runnable_json on_run;

		ui_task_job() : job()
		{
		}

		ui_task_job(std::coroutine_handle<> _handle,
			HANDLE _event,
			json _params,
			runnable _on_gui,
			runnable_json _on_run) :
			handle(_handle), 
			event(_event), 
			params(_params), 
			on_gui(_on_gui),
			on_run(_on_run)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			debug_functions&& std::cout << "ui task start:" << GetCurrentThreadId() << std::endl;

			json blank_params;

			runnable cylon = []() -> void
			{
				//on_gui();
			};

			auto result = new ui_task_result(
				blank_params,
				cylon
			);

			try
			{
				if (on_run)
				{
					on_run(params);
				}
				if (handle) {
					handle.resume();
					handle.destroy();
				}
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, TRUE, (LPARAM)result);
			}
			catch (...)
			{
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
			}
			
			jn.setSignal(event);
			debug_functions&& std::cout << "ui task end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class general_ui_job : public job
	{
	public:
		runnable on_gui;
		runnable on_run;

		general_ui_job() : job()
		{
		}

		general_ui_job(
			runnable _on_run,
			runnable _on_gui) :
			on_run(_on_run),
			on_gui(_on_gui)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			json empty_default;

//			std::cout << "gen ui task start:" << GetCurrentThreadId() << std::endl;

			auto result = new ui_task_result(empty_default, on_gui);

			try
			{
				if (on_run)
				{
					on_run();
				}
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, TRUE, (LPARAM)result);
			}
			catch (...)
			{
				_callingQueue->post_ui_message(WM_CORONA_TASK_COMPLETE, FALSE, (LPARAM)result);
			}

//			std::cout << "gen ui task end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	class general_http_ui_job : public job
	{
	public:
		runnable_http_response on_gui;
		runnable_http_request on_run;

		general_http_ui_job() : job()
		{
		}

		general_http_ui_job(
			runnable_http_request _on_run,
			runnable_http_response _on_gui) :
			on_run(_on_run),
			on_gui(_on_gui)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;

			json empty_default;

			//			std::cout << "gen ui task start:" << GetCurrentThreadId() << std::endl;

			try
			{
				if (on_run)
				{
					call_status status = on_run();
					auto result = new http_task_result(status, on_gui);
					_callingQueue->post_ui_message(WM_CORONA_HTTP_TASK_COMPLETE, TRUE, (LPARAM)result);
				}
				else {
					call_status status = {};
					auto result_empty = new http_task_result(status, on_gui);
					_callingQueue->post_ui_message(WM_CORONA_HTTP_TASK_COMPLETE, TRUE, (LPARAM)result_empty);

				}
			}
			catch (...)
			{
				call_status status = {};
				auto result_empty = new http_task_result(status, on_gui);
				_callingQueue->post_ui_message(WM_CORONA_HTTP_TASK_COMPLETE, FALSE, (LPARAM)result_empty);
			}

			//			std::cout << "gen ui task end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = true;

			return jn;
		}
	};

	template <typename Param> concept task_io_params = requires(Param param) {
		param.bytes_transferred = 0;
		param.success = false;
	};

	template <typename Param>
	requires(task_io_params<Param>)
	class async_io_job : public job
	{
	public:
		std::coroutine_handle<> handle;
		HANDLE					event;
		Param* params;

		async_io_job() : job()
		{
			params = {};
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			params->bytes_transferred = _bytesTransferred;
			params->success = _success;
			jn.setSignal(event);
			debug_functions&& std::cout << "job start:" << GetCurrentThreadId() << std::endl;
			if (handle) {
				handle.resume();
				handle.destroy();
			}
			debug_functions&& std::cout << "job end:" << GetCurrentThreadId() << std::endl;
			jn.shouldDelete = false;
			return jn;
		}
	};

	template <typename IOParams> struct async_io_task : public std::suspend_always
	{
	public:
		job_queue* queue;
		IOParams	params;
		std::function<bool(HANDLE hevent, IOParams* _src, std::coroutine_handle<> _handle)> runner;

		void configure(job_queue* _queue, const IOParams& _params, std::function<bool(HANDLE hevent, IOParams* _src, std::coroutine_handle<> _handle)> _runner)
		{
			debug_functions&& std::cout << "configure:" << this << GetCurrentThreadId() << std::endl;
			queue = _queue;
			params = _params;
			runner = _runner;
		}

		struct promise_type
		{
			IOParams m_value;

			promise_type()
			{
				debug_functions&& std::cout << "async_io_task promise_type:" << this << " " << GetCurrentThreadId() << std::endl;
			}

			async_io_task<IOParams> get_return_object()
			{
				debug_functions&& std::cout << "task get_return_object:" << this << " " << GetCurrentThreadId() << std::endl;
				async_io_task<IOParams> my_task;
				my_task.coroutine = std::coroutine_handle<promise_type>::from_promise(*this);
				return my_task;
			};

			void return_value(IOParams value) {
				debug_functions&& std::cout << "async_io_task return_value:" << value << " " << this << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_always initial_suspend() {
				debug_functions&& std::cout << "async_io_task initial_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				debug_functions&& std::cout << "async_io_task final_suspend:" << this << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {
				debug_functions&& std::cout << "async_io_task unhandled exception:" << this << GetCurrentThreadId() << std::endl;
			}
		};

		std::coroutine_handle<promise_type> coroutine;

		void await_suspend(std::coroutine_handle<> handle)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			debug_functions&& std::cout << this << ", async_io_task await_suspend:" << GetCurrentThreadId() << std::endl;
			if (runner) {
				if (runner(hevent, &params, handle)) {
					debug_functions&& std::cout << this << ", async_io_task await_suspend away:" << GetCurrentThreadId() << std::endl;
					::WaitForSingleObject(hevent, INFINITE);
					debug_functions&& std::cout << "async_io_task await_suspend finished:" << GetCurrentThreadId() << std::endl;
				}
				else
				{
					debug_functions&& std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
				}
			}
			::CloseHandle(hevent);
		}

		IOParams await_resume()
		{
			IOParams t = {};
			debug_functions&& std::cout << "async_io_task await_resume:" << GetCurrentThreadId() << std::endl;
			if (coroutine) {
				t = coroutine.promise().m_value;
			}
			return t;
		}

		operator IOParams() {
			debug_functions&& std::cout << this << ", async_io_task cast to T:" << GetCurrentThreadId() << std::endl;
			return coroutine.promise().m_value;
		}

		async_io_task(std::coroutine_handle<> _coroutine) : coroutine(_coroutine)
		{
			debug_functions&& std::cout << typeid(*this).name() << " ctor:" << GetCurrentThreadId() << std::endl;
		}

		async_io_task()
		{
			debug_functions&& std::cout << typeid(*this).name() << " ctor" << GetCurrentThreadId() << std::endl;
		}

	};


	template <typename T> class sync 
	{
	public:

		T value;

		struct promise_type
		{
			T m_value;
			std::coroutine_handle<promise_type> coro;

			promise_type()
			{
				m_value = {};
				debug_functions&& std::cout << typeid(*this).name() << " promise_type(" << m_value << ")" << GetCurrentThreadId() << std::endl;
			}

			sync<T> get_return_object()
			{
				debug_functions&& std::cout << typeid(*this).name() << " get_return_object(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				std::coroutine_handle<promise_type> promise_coro = std::coroutine_handle<promise_type>::from_promise(*this);
				sync<T> sn(promise_coro);
				return sn;
			}

			void return_value(T value) {
				debug_functions&& std::cout << typeid(*this).name() << " return_value(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				m_value = value;
			}

			std::suspend_never initial_suspend() {
				debug_functions&& std::cout << typeid(*this).name() << " initial_suspend(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				return {};
			}

			std::suspend_always final_suspend() noexcept {
				debug_functions&& std::cout << typeid(*this).name() << " final_suspend(" << m_value << ")" << GetCurrentThreadId() << std::endl;
				return {};
			}

			void unhandled_exception() {}
		};

		std::coroutine_handle<promise_type> coro;

		sync()
		{
			value = {};
			coro = {};
		}

		sync(std::coroutine_handle<promise_type> _coro)
		{
			value = {};
			coro = _coro;
		}

		sync(const T& _value)
		{
			value = _value;
			coro = {};
			debug_functions&& std::cout << typeid(*this).name() << " ctor (" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		sync(const sync & _src)
		{
			value = _src.value;
			coro = {};
			debug_functions&& std::cout << typeid(*this).name() << " ctor (" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		sync operator =(const sync& _src)
		{
			value = _src.value;
			coro = {};
			debug_functions&& std::cout << typeid(*this).name() << " op= (" << value << ")" << GetCurrentThreadId() << std::endl;
			return *this;
		}

		bool await_ready()
		{
			debug_functions&& std::cout << typeid(*this).name() << " await_ready(" << value << ")" <<  GetCurrentThreadId() << std::endl;
			return false;
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			coro = handle;
			debug_functions&& std::cout << typeid(*this).name() << " await_suspend(" << value << ")" << GetCurrentThreadId() << std::endl;
		}

		T await_resume()
		{
			debug_functions&& std::cout << typeid(*this).name() << " await_resume.(" << value << ")" <<  GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return value;
		}

		operator T() {
			debug_functions&& std::cout << typeid(*this).name() << " cast (" << value << ")" <<  GetCurrentThreadId() << std::endl;
			return value;
		}

		T wait()
		{
			debug_functions&& std::cout << typeid(*this).name() << " await_resume.(" << value << ")" << GetCurrentThreadId() << std::endl;
			if (coro) {
				coro.resume();
			}
			return value;
		}

	};

	struct task
	{
		struct promise_type
		{
			task get_return_object() { return {}; }
			std::suspend_never initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			void return_void() {}
			void unhandled_exception() {}
		};
	};

	class threadomatic 
	{
	public:

		static void run(runnable _runnable)
		{
			general_job* gj = new general_job(_runnable);
			global_job_queue->add_job(gj);
		}

		static void run_complete(runnable _runnable, runnable _ui_complete)
		{
			general_ui_job* guj = new general_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
		}

		static void run_http(runnable_http_request _runnable, runnable_http_response _ui_complete)
		{
			general_http_ui_job* guj = new general_http_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
		}

		template <typename dest, typename item> static void run_each(dest *_targets, std::vector<item>& _items, std::function<void(dest *_target, item& _src)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);
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


		template <typename item> static void run_each(std::vector<item> &_items, std::function<void(item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);
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

		template <typename item> static void run_each(std::vector<item>& _items, int _width, int _height, std::function<void(int _x, int _y, item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);
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

		template <typename IOParams> static void run_io(job_queue* queue, IOParams params, std::function<bool(HANDLE hevent, IOParams* _src)> runner)
		{
			HANDLE hevent = ::CreateEvent(NULL, false, false, NULL);
			debug_functions&& std::cout << "run_io_task start:" << GetCurrentThreadId() << std::endl;
			if (runner) {
				if (runner(hevent, &params)) {
					debug_functions&& std::cout << "run_io await_suspend away : " << GetCurrentThreadId() << std::endl;
					::WaitForSingleObject(hevent, INFINITE);
					debug_functions&& std::cout << "run_io await_suspend finished:" << GetCurrentThreadId() << std::endl;
				}
				else
				{
					debug_functions&& std::cout << "error skipped finished:" << GetCurrentThreadId() << std::endl;
				}
			}
			::CloseHandle(hevent);
		}
	};
}

#endif
