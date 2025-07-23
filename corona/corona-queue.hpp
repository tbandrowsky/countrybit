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

	class job_notify {

		enum notifies {
			postmessage,
			setevent,
			none
		} notification;

		MSG msg;

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

	class job;

	class job_container
	{
	public:
		OVERLAPPED ovp;
		job* jobdata;
		HANDLE parent;

		job_container() :jobdata(nullptr), parent(nullptr)
		{
			ovp = {};
		}

		~job_container()
		{
		}
	};

	class job
	{
	public:
		job_container container;
		job();
		virtual ~job();
		virtual bool queued(job_queue* _callingQueue) { return true; }
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	using runnable = std::function<void()>;
	using runnable_http_request = std::function<call_status()>;
	using runnable_http_response = std::function<void(call_status)>;

	class general_job : public job
	{
	public:
		HANDLE notification_handle;
		runnable function_to_run;
		general_job();
		general_job(runnable _runnable, HANDLE _notification_handle = nullptr);
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
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	class finish_job : public job {

	public:

		HANDLE handle;

		finish_job();
		virtual ~finish_job();
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	const int maxWorkerThreads = 63;

	class job_queue : public lockable
	{

		HANDLE ioCompPort;

		std::vector<std::thread> threads;
		int numWorkerThreads;

		bool shutDownOrdered;

		std::atomic_int num_outstanding_jobs;
		HANDLE empty_queue_event;

		DWORD thread_id;

	public:

		inline HANDLE getPort() { return ioCompPort; }
		inline DWORD getThreadCount() { return numWorkerThreads; }
		inline bool wasShutDownOrdered() { return shutDownOrdered; }

		job_queue();

		virtual ~job_queue();

		void start(int _numThreads);
		void listen(HANDLE _otherQueue);

		void post_ui_message(UINT msg, WPARAM wparam, LPARAM lparam);
		bool listen_job(job *_jobMessage);
		void add_job(job* _jobMessage);
		void add_job(runnable _function, HANDLE handle);
		void shutDown();
		void kill();
		void run_job(job* _jobMessage);
		unsigned int run_next_job();
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
			// ::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			break;
		case setevent:
			SetEvent((HANDLE)msg.lParam);
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
	}

	void job_notify::setSignal(HANDLE signal)
	{
		if (signal and signal != INVALID_HANDLE_VALUE) {
			notification = setevent;
			msg.lParam = (LPARAM)(signal);
		}
		else 
		{
			system_monitoring_interface::global_mon->log_warning("job_notify: setSignal called with invalid handle.", __FILE__, __LINE__);
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
	}

	// ----------------------------------------------------------------------------


	job::job()
	{
		container.jobdata = this;
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
	}

	general_job::general_job(runnable _runnable, HANDLE _notification_handle)
	{
		notification_handle = _notification_handle;
		function_to_run = _runnable;
	}

	general_job::~general_job()
	{
		;
	}

	job_notify general_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;

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
			system_monitoring_interface::global_mon->log_exception(exc, __FILE__, __LINE__);
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
		ioCompPort = nullptr;
		num_outstanding_jobs = 0;
		empty_queue_event = CreateEventW(nullptr, false, true, nullptr);
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
		threadCount = 2;

		if (_numThreads > maxWorkerThreads or _numThreads == 0) _numThreads = threadCount;

		if (not ioCompPort) {

			numWorkerThreads = _numThreads;

			ioCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _numThreads);

			if (ioCompPort) {
				for (i = 0; i < numWorkerThreads; i++) {
					threads.push_back(std::thread(jobQueueThread, this));
				}
			}
		}
	}

	void job_queue::listen(HANDLE _otherQueue)
	{
		if (CreateIoCompletionPort(_otherQueue, ioCompPort, (ULONG_PTR)_otherQueue, 0) == NULL) {
			throw std::invalid_argument("job_queue:cannot listen.");
		}
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

	void job_queue::add_job(job* _jobMessage)
	{
		LONG result;
		if (not _jobMessage)
			return;
		if (_jobMessage->queued(this)) {
			++num_outstanding_jobs;
			ResetEvent(empty_queue_event);
			result = PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(&_jobMessage->container));
		}
	}

	void job_queue::add_job(runnable _function, HANDLE handle)
	{
		LONG result;
		general_job* _job_message = new general_job(_function, handle);
		if (_job_message->queued(this)) {
			++num_outstanding_jobs;
			ResetEvent(empty_queue_event);
			result = PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(&_job_message->container));
		}
	}

	bool job_queue::listen_job(job* _jobMessage)
	{
		if (not _jobMessage)
			return false;
		return _jobMessage->queued(this);
	}

	void job_queue::post_ui_message(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto success = ::PostThreadMessage(thread_id, msg, wparam, lparam);		
	}

	unsigned int job_queue::jobQueueThread(job_queue* jobQueue)
	{

		LPOVERLAPPED lpov = {};
		job* waiting_job;
		job_container* container;

		BOOL success;
		DWORD bytesTransferred;
		ULONG_PTR compKey;

		job_notify jobNotify;

#ifdef COM_INITIALIZATION
		//	::CoInitializeEx( NULL, COINIT_MULTITHREADED );
		CoInitialize(NULL);
#endif

		while (!jobQueue->wasShutDownOrdered()) {
			success = ::GetQueuedCompletionStatus(jobQueue->ioCompPort, &bytesTransferred, &compKey, &lpov, 1000);
			if (success and lpov) {
				container = (job_container*)lpov;
				if (container) {
					waiting_job = container->jobdata;

					if (waiting_job) {
						try {
							jobNotify = waiting_job->execute(jobQueue, bytesTransferred, success);
							jobNotify.notify();
							if (jobNotify.repost and (!jobQueue->wasShutDownOrdered())) {
								jobQueue->add_job(waiting_job);
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::global_mon->log_warning(exc.what(), __FILE__, __LINE__);
						}
						if (waiting_job->container.parent) {
							SetEvent(waiting_job->container.parent);
                        }

						jobQueue->num_outstanding_jobs--;
					}

					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}

				int jcresult = jobQueue->num_outstanding_jobs;

				if (jcresult == 0) {
					::SetEvent(jobQueue->empty_queue_event);
				}
			}
		}

#ifdef COM_INITIALIZATION
		::CoUninitialize();
#endif

		return 0;
	};

	unsigned int job_queue::run_next_job()
	{

		LPOVERLAPPED lpov = {};
		job* waiting_job;
		job_container* container;

		BOOL success;
		DWORD bytesTransferred;
		ULONG_PTR compKey;

		job_notify jobNotify;

		if (!wasShutDownOrdered()) {
			success = ::GetQueuedCompletionStatus(ioCompPort, &bytesTransferred, &compKey, &lpov, 100);
			if (success and lpov) {
				container = (job_container*)lpov;
				if (container) {
					waiting_job = container->jobdata;

					if (waiting_job) {
						try {
							jobNotify = waiting_job->execute(this, bytesTransferred, success);
							jobNotify.notify();
							if (jobNotify.repost and (!wasShutDownOrdered())) {
								add_job(waiting_job);
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::global_mon->log_warning(exc.what(), __FILE__, __LINE__);
						}
						if (waiting_job->container.parent) {
							::SetEvent(waiting_job->container.parent);
						}
					}

					num_outstanding_jobs--;

					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}

				LONG numJobs = num_outstanding_jobs;

				if (not numJobs) {
					::SetEvent(empty_queue_event);
				}
			}
		}

#ifdef COM_INITIALIZATION
		::CoUninitialize();
#endif

		return num_outstanding_jobs;
	};

	void job_queue::waitForThreadFinished()
	{
		finish_job fj;
		add_job(&fj);
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

	void job_queue::run_job(job* _jobMessage)
	{		
        HANDLE wait_handle = CreateEventA(NULL, FALSE, FALSE, NULL);
		if (_jobMessage && _jobMessage->queued(this)) {
			_jobMessage->container.parent = wait_handle;
			try {
				ResetEvent(empty_queue_event);
				PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(&_jobMessage->container));
				bool complete = false;
				while (not complete) {
					DWORD result = WaitForSingleObject(wait_handle, 100);
					if (result == 0) {
						complete = true;
					}
					else {
						run_next_job();
					}
				}
			}
			catch (std::exception& exc) 
			{
				system_monitoring_interface::global_mon->log_exception(exc, __FILE__, __LINE__);
            }			
			CloseHandle(wait_handle);
		}
    }
	
	void test_locks(std::shared_ptr<test_set> _tests)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("lock proof", "start", st, __FILE__, __LINE__);

		int test_seconds = 2;
		int test_milliseconds = test_seconds * 1000;

		json_parser jp;

		object_locker locker;

		std::vector<cob_key>  lock_keys = { { object_lock_types::lock_table, 0, 0 },
											{ object_lock_types::lock_object, 0, 42 } };

		HANDLE wait_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

		{
			scope_multilock lock = locker.lock(lock_keys);
			system_monitoring_interface::global_mon->log_information("1st lock on thread", __FILE__, __LINE__);

			scope_multilock lock2 = locker.lock(lock_keys);
			system_monitoring_interface::global_mon->log_information("2nd lock on same thread", __FILE__, __LINE__);

			_tests->test({ "same thread", true, __FILE__, __LINE__ });

			global_job_queue->add_job([&locker, &_tests, test_seconds]() -> void
				{
					timer tx2;
					system_monitoring_interface::global_mon->log_information("waiting for lock", __FILE__, __LINE__);
					scope_multilock locko = locker.lock({ object_lock_types::lock_table, 0, 0 });
					if (tx2.get_elapsed_seconds() < test_seconds) {
						system_monitoring_interface::global_mon->log_warning("thread skipped lock", __FILE__, __LINE__);
						_tests->test({ "thread wait", false, __FILE__, __LINE__ });
					}
					else {
						_tests->test({"thread wait", true, __FILE__, __LINE__ });
					}
					system_monitoring_interface::global_mon->log_information("lock released", __FILE__, __LINE__);
				}, wait_handle);

			system_monitoring_interface::global_mon->log_information("waiting for job to get lock", __FILE__, __LINE__);
			::Sleep(test_milliseconds);
		}

		system_monitoring_interface::global_mon->log_information("should have released", __FILE__, __LINE__);
		WaitForSingleObject(wait_handle, INFINITE);
		if (tx.get_elapsed_seconds() < test_seconds) {
			_tests->test({"wait suffice", false, __FILE__, __LINE__});
		}
		else {
			_tests->test({"wait suffice", true, __FILE__, __LINE__ });
		}

		system_monitoring_interface::global_mon->log_function_stop("lock proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	void test_rw_locks(std::shared_ptr<test_set> _tests)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("rw lock proof", "start", st, __FILE__, __LINE__);

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

		wait_handle.resize(max_job_count);
		for (int i = 0; i < max_job_count; i++)
		{
			wait_handle[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
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
				global_job_queue->add_job([&tst_timer, &lock_test, &_tests, test_seconds, &fail_count, i, x]() -> void
					{
						read_locked_sp test_read(lock_test);
						double elapsed = tst_timer.get_elapsed_seconds_total();
						std::string test_name = std::format("read thread {0}.{1} {2}", x, i, elapsed);
                        system_monitoring_interface::global_mon->log_information(test_name, __FILE__, __LINE__);	
						if (elapsed < test_seconds) {
							InterlockedIncrement(&fail_count);
							_tests->test({ test_name, false, __FILE__, __LINE__ });
						}
						else {
							_tests->test({ test_name, true, __FILE__, __LINE__ });
						}
					}, wait_handle[i]);
			}
			system_monitoring_interface::global_mon->log_information(std::format("testing write blocks read {0}, {1} fails", x, fail_count), __FILE__, __LINE__);
			::Sleep(test_milliseconds);
			write_lock = nullptr;
			WaitForMultipleObjects(max_job_count, wait_handle.data(), TRUE, INFINITE);
			write_lock = std::make_shared<write_locked_sp<lockable_item>>(lock_test);
			bool result = fail_count == 0;
			_tests->test({ "wait fail count", result, __FILE__, __LINE__ });
		}

		// testing that readers block writers
		write_lock = nullptr;
		std::shared_ptr<read_locked_sp<lockable_item>> read_lock = std::make_shared<read_locked_sp<lockable_item>>(lock_test);
		int start_count = read_lock->get()->get_count();

		system_monitoring_interface::global_mon->log_information("thread with multiple writers away, blocked by reader", __FILE__, __LINE__);
		fail_count = 0;
		LONG active_count = max_job_count;
		timer test_timer2;

		for (int i = 0; i < max_job_count; i++)
		{
			global_job_queue->add_job([&test_timer2, &lock_test, &_tests, test_seconds, i, &active_count]() -> void
				{
					write_locked_sp test_write(lock_test);

					int y0 = lock_test->get_count();
					lock_test->add_count();
					int y1 = lock_test->get_count();

					std::string test_name = std::format("write thread {0}", i);
					bool result = y1 - y0 == 1;
					_tests->test({ test_name, result, __FILE__, __LINE__ });
					InterlockedDecrement(&active_count);

					result = (test_timer2.get_elapsed_seconds_total() >= test_seconds);
					test_name = std::format("write thread time {0}", i);
					_tests->test({ test_name, result, __FILE__, __LINE__ });

				}, wait_handle[i]);
		}

		::Sleep(test_milliseconds);
		read_lock = nullptr;
		WaitForMultipleObjects(max_job_count, wait_handle.data(), TRUE, INFINITE);

		for (int i = 0; i < max_job_count; i++)
		{
			CloseHandle(wait_handle[i]);
		}

		system_monitoring_interface::global_mon->log_information(std::format("{0} writers released and complete {1} active count", max_job_count, active_count), __FILE__, __LINE__);

		bool result = active_count == 0;
		_tests->test({ "active_count", result, __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("rw lock proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif
