/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
Thread pool implementation.  You make job objects and push them on there.
And away you go.

Notes

For Future Consideration
*/


#ifndef CORONA_QUEUE_H
#define CORONA_QUEUE_H

namespace corona {

	class application;

	class job_queue;

	ULONG_PTR completion_key_compute = 1;
	ULONG_PTR completion_key_file = 2;
	ULONG_PTR completion_key_http = 3;

	class job_notify {

		enum notifies {
			postmessage,
			setevent,
			none
		} notification;

		MSG msg;
		HANDLE signal;

		void notify();

	public:

		bool repost, shouldDelete;

		job_notify();
		job_notify(job_notify&& _src);
		void setSignal(HANDLE signal);
		void setPostMsg(HWND _hwnd, UINT _message, WPARAM _wParam, LPARAM _lParam);
		~job_notify();
		void operator = (const job_notify& _src);
		friend class job_queue;
	};

	class job
	{
	public:
		OVERLAPPED overlapped;
		int job_id;

		job();
		virtual ~job();
		virtual bool queued(job_queue* _callingQueue) = 0;
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	using runnable = std::function<void()>;
	using runnable_http_request = std::function<call_status()>;
	using runnable_http_response = std::function<void(call_status)>;

	class io_job : public job
	{
	public:
		virtual LPOVERLAPPED get_job_key() = 0;
		virtual HANDLE get_file_handle() = 0;
	};

	class general_job : public job
	{
	public:
		HANDLE notification_handle;
		runnable function_to_run;
		double execution_time_seconds;
		timer queue_time;
		general_job();
		general_job(runnable _runnable, HANDLE _notification_handle = nullptr);
		virtual bool queued(job_queue* _callingQueue) override {
			return true;
		}
		virtual ~general_job();
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	class system_job : public job
	{
	public:
		HANDLE notification_handle;
		std::string system_command;

		system_job();
		system_job(std::string _system_command, HANDLE _notification_handle = nullptr);

		virtual ~system_job();
		virtual bool queued(job_queue* _callingQueue) override {
			return true;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	class finish_job : public job {

	public:

		HANDLE handle;

		finish_job();
		virtual ~finish_job();

		virtual bool queued(job_queue* _callingQueue) override {
			return true;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	const int maxWorkerThreads = 63;

	class job_file_request
	{
    public:
		thread_safe_map<LPOVERLAPPED, io_job*> jobs_by_ovp;
	};

	class job_queue : public lockable
	{
	protected:
		HANDLE ioCompPort;
		lockable queueLock;

		std::vector<std::thread> threads;

		bool shutDownOrdered;

		HANDLE empty_queue_event;

		DWORD thread_id;
		std::atomic<int> job_id = { 0 };

		thread_safe_map<int64_t, job*> compute_jobs;
		thread_safe_map<HANDLE, std::shared_ptr<job_file_request>> io_jobs;

        void add_io_job(io_job* _jobMessage);
		io_job *find_io_job(DWORD completionKey, LPOVERLAPPED overlapped);
		bool remove_io_job(io_job* _jobMessage);

	public:

		inline HANDLE getPort() { return ioCompPort; }
		inline DWORD getThreadCount() { return threads.size(); }
		inline bool wasShutDownOrdered() { return shutDownOrdered; }

		job_queue();
		virtual ~job_queue();

		void start(int _numThreads);

		void post_ui_message(UINT msg, WPARAM wparam, LPARAM lparam);
		HANDLE listen_file(HANDLE _otherQueue);
		bool listen_job(io_job *_jobMessage);
		void submit_job(job* _jobMessage);
		void submit_job(runnable _function, HANDLE handle);
		void shutDown();
		void kill();
		bool run_next_job();
		static unsigned int jobQueueThread(job_queue* jobQueue);
		void waitForThreadFinished();

		void waitForEmptyQueue();

		int numberOfProcessors();

	};

	std::unique_ptr<job_queue> global_job_queue;

	void job_notify::notify()
	{
		switch (notification) {
		case postmessage:
			PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			break;
		case setevent:
			SetEvent(signal);
			break;
		}
	}

	job_notify::job_notify()
	{
		repost = false;
		notification = none;
		shouldDelete = false;
		msg = {};
	}

	job_notify::job_notify(job_notify&& _src)
	{
		repost = _src.repost;
		msg.hwnd = _src.msg.hwnd;
		msg.message = _src.msg.message;
		msg.wParam = _src.msg.wParam;
		msg.lParam = _src.msg.lParam;
		notification = _src.notification;
		shouldDelete = _src.shouldDelete;
		signal = _src.signal;
	}

	void job_notify::setSignal(HANDLE _signal)
	{
		if (_signal and _signal != INVALID_HANDLE_VALUE) {
			notification = setevent;
			signal = _signal;
		}
		else 
		{
			system_monitoring_interface::active_mon->log_warning("job_notify: setSignal called with invalid handle.", __FILE__, __LINE__);
		}
	}

	void job_notify::setPostMsg(HWND _hwnd, UINT _message, WPARAM _wParam, LPARAM _lParam)
	{
		notification = postmessage;
		msg.hwnd = _hwnd;
		msg.message = _message;
		msg.wParam = _wParam;
		msg.lParam = _lParam;
	}

	job_notify::~job_notify()
	{
		;
	}

	void job_notify::operator = (const job_notify& _src)
	{
		repost = _src.repost;
		msg.hwnd = _src.msg.hwnd;
		msg.message = _src.msg.message;
		msg.wParam = _src.msg.wParam;
		msg.lParam = _src.msg.lParam;
		notification = _src.notification;
		shouldDelete = _src.shouldDelete;
		signal = _src.signal;
	}

	// ----------------------------------------------------------------------------


	job::job()
	{
        job_id = 0;
		overlapped = {};
	}

	job::~job()
	{
		;
	}

	job_notify job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;

		jobNotify.shouldDelete = false;

		return jobNotify;
	}

	

	// -------------------------------------------------------------------------------

	general_job::general_job()
	{
		notification_handle = nullptr;
		execution_time_seconds = 0;
	}

	general_job::general_job(runnable _runnable, HANDLE _notification_handle)
	{
		notification_handle = _notification_handle;
		function_to_run = _runnable;
		execution_time_seconds = 0;
	}

	general_job::~general_job()
	{
		;
	}

	job_notify general_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;
		timer tx;

		if (notification_handle) {
			jobNotify.setSignal(notification_handle);
		}

		if (function_to_run)
		{
			function_to_run();
		}

		jobNotify.shouldDelete = true;

		return jobNotify;
	}

	// -------------------------------------------------------------------------------

	system_job::system_job()
	{
		notification_handle = nullptr;
	}

	system_job::system_job(std::string _system_command, HANDLE _notification_handle)
	{
        system_command = _system_command;
		notification_handle = _notification_handle;
	}

	system_job::~system_job()
	{
		;
	}

	job_notify system_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;

		if (notification_handle) {
			jobNotify.setSignal(notification_handle);
		}

		try 
		{
			if (not system_command.empty()) {
				std::system(system_command.c_str());
            }
		}
		catch (std::exception & exc) 
		{
			jobNotify.repost = false;
			jobNotify.shouldDelete = true;
			system_monitoring_interface::active_mon->log_exception(exc, __FILE__, __LINE__);
			return jobNotify;
        }

		jobNotify.shouldDelete = true;

		return jobNotify;
	}

	// -------------------------------------------------------------------------------

	finish_job::finish_job() : job()
	{
		handle = CreateEventW(NULL, false, false, NULL);
	}

	finish_job::~finish_job()
	{
		CloseHandle(handle);
	}

	job_notify finish_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;

		jobNotify.setSignal(handle);

		return jobNotify;
	}


	job_queue::job_queue()
	{
		ioCompPort = NULL;
		empty_queue_event = CreateEventW(nullptr, false, false, nullptr);
		thread_id = ::GetCurrentThreadId();
	}

	job_queue::~job_queue()
	{
		shutDown();
		::CloseHandle(empty_queue_event);
	}

	void job_queue::start(int _numThreads)
	{
		int i;

		shutDownOrdered = false;

		int threadCount = job_queue::numberOfProcessors();

		if (_numThreads > maxWorkerThreads or _numThreads == 0) _numThreads = threadCount;

		if (ioCompPort==NULL) {

			ioCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, _numThreads);

			if (ioCompPort) {
				for (i = 0; i < _numThreads; i++) {
					threads.push_back(std::thread(jobQueueThread, this));
				}
			}
		}
	}

	HANDLE job_queue::listen_file(HANDLE _otherQueue)
	{
		HANDLE hport;

		hport = CreateIoCompletionPort(_otherQueue, ioCompPort, (ULONG_PTR)_otherQueue, 0);

        if (hport == NULL) {
            os_result osr;
            system_monitoring_interface::active_mon->log_warning(std::format("CreateIoCompletionPort failed with error #{0}", osr.message), __FILE__, __LINE__);
            return NULL;
        }

        std::shared_ptr<job_file_request> jfr = std::make_shared<job_file_request>();
		if (not io_jobs.try_get(_otherQueue, jfr)) {
            io_jobs.insert(_otherQueue, jfr);
		}

		return hport;
	}

	void job_queue::shutDown()
	{
		int i;

		// keep sending shutdown messages until all of the queues terminate!

		shutDownOrdered = true;

		for (i = 0; i < threads.size(); i++) {
			if (threads[i].joinable())
				threads[i].join();
		}

		::CloseHandle(ioCompPort);

		ioCompPort = NULL;
	}

	void job_queue::kill()
	{
		shutDown();
	}

	void job_queue::submit_job(job* _jobMessage)
	{
		LONG result;
		if (not _jobMessage)
			return;

		if (_jobMessage->queued(this)) {
			job_id++;
			_jobMessage->job_id = job_id;
			compute_jobs.insert(job_id, _jobMessage);
			result = PostQueuedCompletionStatus(ioCompPort, job_id, completion_key_compute, nullptr);
		}
	}

	void job_queue::submit_job(runnable _function, HANDLE handle)
	{
		LONG result;
		general_job* _job_message = new general_job(_function, handle);
		if (_job_message->queued(this)) {
			job_id++;
			_job_message->job_id = job_id;
			compute_jobs.insert(job_id, _job_message);
			result = PostQueuedCompletionStatus(ioCompPort, job_id, completion_key_compute, nullptr);
		}
	}

	bool job_queue::listen_job(io_job* _jobMessage)
	{
		if (not _jobMessage)
			return false;
		
        add_io_job(_jobMessage);
		return _jobMessage->queued(this);
	}

	void job_queue::post_ui_message(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto success = ::PostThreadMessage(thread_id, msg, wparam, lparam);		
	}

	unsigned int job_queue::jobQueueThread(job_queue* jobQueue)
	{

		
#ifdef COM_INITIALIZATION
		//	::CoInitializeEx( NULL, COINIT_MULTITHREADED );
		CoInitialize(NULL);
#endif

		while (jobQueue->run_next_job());

#ifdef COM_INITIALIZATION
		::CoUninitialize();
#endif

		return 0;
	};

	bool job_queue::run_next_job()
	{

		LPOVERLAPPED lpov = {};
		job* waiting_job = nullptr;

		BOOL success = false;
		DWORD bytesTransferred = 0;
		ULONG_PTR compKey = 0;

		job_notify jobNotify;

		if (!wasShutDownOrdered()) {
			success = ::GetQueuedCompletionStatus(ioCompPort, &bytesTransferred, &compKey, &lpov, 10000);
			if (success) {

				io_job* completed_io = find_io_job(compKey, lpov);
				if (completed_io) {
					try {
						// if waiting_job is whacked, that means the pointer for the job was actually deleted.
						// this is the case where, Windows has completed the IO operation and we are handling the results
						jobNotify = completed_io->execute(this, bytesTransferred, success);
						jobNotify.notify();
					}
					catch (std::exception exc)
					{
						system_monitoring_interface::active_mon->log_warning(exc.what(), __FILE__, __LINE__);
					}
					if (remove_io_job(completed_io)) {
						::SetEvent(empty_queue_event);
					}
					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}
				else if (compKey == completion_key_compute)
				{
					int job_id = (int)bytesTransferred;
					if (compute_jobs.try_get(job_id, waiting_job)) {
						try {
							// if waiting_job is whacked, that means the pointer for the job was actually deleted.
							jobNotify = waiting_job->execute(this, bytesTransferred, success);
							jobNotify.notify();
							if (jobNotify.repost and (!wasShutDownOrdered())) {
								submit_job(waiting_job);
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::active_mon->log_warning(exc.what(), __FILE__, __LINE__);
						}
						if (compute_jobs.erase(job_id)) {
							::SetEvent(empty_queue_event);
						}
						if (jobNotify.shouldDelete) {
							delete waiting_job;
						}
					}
				}
				else {
					system_monitoring_interface::active_mon->log_warning("Unknown completion key for job, so, let's try again.", __FILE__, __LINE__);
                    PostQueuedCompletionStatus(ioCompPort, bytesTransferred, compKey, lpov);
				}
			}
			success = true;
		}
		return success;
	};

	void job_queue::waitForThreadFinished()
	{
		finish_job fj;
		submit_job(&fj);
		WaitForSingleObject(fj.handle, INFINITE);
	}

	void job_queue::waitForEmptyQueue()
	{
		WaitForSingleObject(empty_queue_event, INFINITE);
	}

	int job_queue::numberOfProcessors()
	{
		PROCESSOR_NUMBER pn;

		GetCurrentProcessorNumberEx(&pn);
		int threadCount = GetMaximumProcessorCount(pn.Group);

		return threadCount;
	}

//	thread_safe_map<int64_t, job*> compute_jobs;
	//thread_safe_map<HANDLE, std::shared_ptr<job_file_request>> io_jobs;

	// There's actually potential races in each of these but I am thinking I might get away with it in this case.
	// This might be an appalling belief to have...

	std::atomic<int> pending_io_jobs;

	void job_queue::add_io_job(io_job* _jobMessage)
	{
		std::shared_ptr<job_file_request> file_jobs;

		auto file_handle = _jobMessage->get_file_handle();
		auto job_key = _jobMessage->get_job_key();
		scope_lock lockme(queueLock);

		if (io_jobs.try_get(file_handle, file_jobs)) {
			file_jobs->jobs_by_ovp.insert(job_key, _jobMessage);
		}
		else 
		{
            file_jobs = std::make_shared<job_file_request>();
			io_jobs.insert(file_handle, file_jobs);
			file_jobs->jobs_by_ovp.insert(job_key, _jobMessage);
		}
		pending_io_jobs++;
	}

	io_job* job_queue::find_io_job(DWORD completionKey, LPOVERLAPPED overlapped)
	{
		scope_lock lockme(queueLock);

		io_job* my_job = nullptr;
		HANDLE hfile = (HANDLE)completionKey;
		std::shared_ptr<job_file_request> file_jobs;
		if (io_jobs.try_get(hfile, file_jobs)) {
			file_jobs->jobs_by_ovp.try_get(overlapped, my_job);
		}
		return my_job;
	}

	bool job_queue::remove_io_job(io_job* _jobMessage)
	{
		scope_lock lockme(queueLock);

		bool removed = false;
		std::shared_ptr<job_file_request> file_jobs;

		auto file_handle = _jobMessage->get_file_handle();
		auto job_key = _jobMessage->get_job_key();

		if (io_jobs.try_get(file_handle, file_jobs)) {
			if (file_jobs->jobs_by_ovp.erase(job_key)) {
				io_jobs.erase(file_handle);
            }
		}

		pending_io_jobs--;
        if (pending_io_jobs.load() == 0) {
            ::SetEvent(empty_queue_event);
			removed = true;
        }
		return removed;
	}

	void test_locks(std::shared_ptr<test_set> _tests)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::active_mon->log_function_start("lock proof", "start", st, __FILE__, __LINE__);

		int test_seconds = 2;
		int test_milliseconds = test_seconds * 1000;

		json_parser jp;

		object_locker locker;

		std::vector<cob_key>  lock_keys = { { object_lock_types::lock_table, 0, 0 },
											{ object_lock_types::lock_object, 0, 42 } };

		HANDLE wait_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

		{
			scope_multilock lock = locker.lock(lock_keys);
			system_monitoring_interface::active_mon->log_information("1st lock on thread", __FILE__, __LINE__);

			scope_multilock lock2 = locker.lock(lock_keys);
			system_monitoring_interface::active_mon->log_information("2nd lock on same thread", __FILE__, __LINE__);
			timer job_timer;

			_tests->test({ "same thread", true, __FILE__, __LINE__ });

			global_job_queue->submit_job([&locker, &job_timer, &_tests, test_seconds]() -> void
				{
					timer exec_time;
					system_monitoring_interface::active_mon->log_information("job start", __FILE__, __LINE__);
					scope_multilock locko = locker.lock({ object_lock_types::lock_table, 0, 0 });
                    double elapsed = job_timer.get_elapsed_seconds();	
					if (elapsed < test_seconds) {
						system_monitoring_interface::active_mon->log_warning("thread skipped lock", __FILE__, __LINE__);
						_tests->test({ "thread wait", false, __FILE__, __LINE__ });
					}
					else {
						_tests->test({"thread wait", true, __FILE__, __LINE__ });
					}
					system_monitoring_interface::active_mon->log_information("job complete", __FILE__, __LINE__);
				}, wait_handle);

			system_monitoring_interface::active_mon->log_information("waiting for job to get lock", __FILE__, __LINE__);
			::Sleep(test_milliseconds);
		}

		system_monitoring_interface::active_mon->log_information("Waiting for job to complete", __FILE__, __LINE__);
		WaitForSingleObject(wait_handle, INFINITE);
		system_monitoring_interface::active_mon->log_information("Job complete", __FILE__, __LINE__);
		if (tx.get_elapsed_seconds() < test_seconds) {
			_tests->test({"wait suffice", false, __FILE__, __LINE__});
		}
		else {
			_tests->test({"wait suffice", true, __FILE__, __LINE__ });
		}

		system_monitoring_interface::active_mon->log_function_stop("lock proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_rw_locks(std::shared_ptr<test_set> _tests)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::active_mon->log_function_start("rw lock proof", "start", st, __FILE__, __LINE__);

		double test_seconds = .5;
		int test_milliseconds = test_seconds * 1000;
		int max_job_count = global_job_queue->getThreadCount() * 4;

		json_parser jp;

		class lockable_item : public shared_lockable
		{
			LONG count;
		public:
			lockable_item()
			{
				count = 0;
			}
			void add_count()
			{
				::InterlockedIncrement(&count);
			}
			int get_count()
			{
				return count;
			}
		};


		// testing that writers block reads 

		int thread_count = 0;
		std::vector<HANDLE> wait_handle;

		for (int i = 0; i < max_job_count; i++)
		{
            HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
			wait_handle.push_back(handle);
		}

		// testing that writers block readers
        // raise a write lock, then spawn 10 read threads
        // the threads should all wait and block until the write lock is released

		long fail_count = 0;

		std::shared_ptr<lockable_item> lock_test = std::make_shared<lockable_item>();
		std::shared_ptr<write_locked_sp<lockable_item>> write_lock = std::make_shared<write_locked_sp<lockable_item>>(lock_test);

		for (int x = 0; x < 5; x++)
		{

			timer ty;
			fail_count = 0;
			timer tst_timer;

			for (int i = 0; i < max_job_count; i++)
			{
				global_job_queue->submit_job([&tst_timer, &lock_test, &_tests, test_seconds, &fail_count, i, x]() -> void
					{
						read_locked_sp test_read(lock_test);
						double elapsed = tst_timer.get_elapsed_seconds_total();
						std::string test_name = std::format("read thread {0}.{1} {2}", x, i, elapsed);
                        system_monitoring_interface::active_mon->log_information(test_name, __FILE__, __LINE__);	
						if (elapsed < test_seconds) {
							InterlockedIncrement(&fail_count);
							_tests->test({ test_name, false, __FILE__, __LINE__ });
						}
						else {
							_tests->test({ test_name, true, __FILE__, __LINE__ });
						}
					}, wait_handle[i]);
			}
			system_monitoring_interface::active_mon->log_information(std::format("testing write blocks read {0}, {1} fails", x, fail_count), __FILE__, __LINE__);
			::Sleep(test_milliseconds);
			write_lock = nullptr;
			for (auto wh : wait_handle) {
				WaitForSingleObject(wh, INFINITE);			
			}
			write_lock = std::make_shared<write_locked_sp<lockable_item>>(lock_test);
			bool result = fail_count == 0;
			_tests->test({ "wait fail count", result, __FILE__, __LINE__ });
		}

		// testing that readers block writers
		write_lock = nullptr;
		std::shared_ptr<read_locked_sp<lockable_item>> read_lock = std::make_shared<read_locked_sp<lockable_item>>(lock_test);
		int start_count = read_lock->get()->get_count();

		system_monitoring_interface::active_mon->log_information("thread with multiple writers away, blocked by reader", __FILE__, __LINE__);
		fail_count = 0;
		LONG active_count = max_job_count;
		timer test_timer2;

		for (int i = 0; i < max_job_count; i++)
		{
			global_job_queue->submit_job([&test_timer2, &lock_test, &_tests, test_seconds, i, &active_count]() -> void
				{
					write_locked_sp test_write(lock_test);

					int y0 = lock_test->get_count();
					lock_test->add_count();
					int y1 = lock_test->get_count();
					int dy = y1 - y0;
					bool result = dy == 1;

					std::string test_name = std::format("write thread {0}, count:{1}", i, dy);
					_tests->test({ test_name, result, __FILE__, __LINE__ });
					InterlockedDecrement(&active_count);

					double dt = test_timer2.get_elapsed_seconds_total() - test_seconds;
					result = dt > 0.0;
					test_name = std::format("write thread {0}, time: {1}, ", i, dt);
					_tests->test({ test_name, result, __FILE__, __LINE__ });

				}, wait_handle[i]);
		}

		::Sleep(test_milliseconds);
		read_lock = nullptr;
		for (auto wh : wait_handle) {
			WaitForSingleObject(wh, INFINITE);
			CloseHandle(wh);
		}

		system_monitoring_interface::active_mon->log_information(std::format("{0} writers released and complete {1} active count", max_job_count, active_count), __FILE__, __LINE__);

		bool result = active_count == 0;
		_tests->test({ "active_count", result, __FILE__, __LINE__ });

		system_monitoring_interface::active_mon->log_function_stop("rw lock proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}



}

#endif
