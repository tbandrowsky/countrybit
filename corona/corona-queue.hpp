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

		job_container() :jobdata(nullptr)
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

	class finish_job : public job {

	public:

		HANDLE handle;

		finish_job();
		virtual ~finish_job();
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	class io_job : public job
	{
	public:
		std::coroutine_handle<> handle;

		io_job(std::coroutine_handle<> _handle) : handle(_handle)
		{
			;
		}

		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify jn;
			jn.shouldDelete = false;
			return jn;
		}
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
		notification = setevent;
		msg.lParam = (LPARAM)(signal);
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
						jobNotify = waiting_job->execute(jobQueue, bytesTransferred, success);
						jobNotify.notify();
						if (jobNotify.repost and (!jobQueue->wasShutDownOrdered())) {
							jobQueue->add_job(waiting_job);
						}
					}

					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}

				LONG numJobs = --jobQueue->num_outstanding_jobs;

				if (not numJobs) {
					::SetEvent(jobQueue->empty_queue_event);
				}
			}
		}

#ifdef COM_INITIALIZATION
		::CoUninitialize();
#endif

		return 0;
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
		SYSTEM_INFO si;
		::GetSystemInfo(&si);
		int threadCount = si.dwNumberOfProcessors;
		return threadCount;
	}

	void test_locks(json& _proof)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("lock proof", "start", st, __FILE__, __LINE__);

		int test_seconds = 5;
		int test_milliseconds = test_seconds * 1000;

		json_parser jp;
		json proof_assertions = jp.create_object();
		proof_assertions.put_member("test_name", "locks");

		object_locker locker;

		std::vector<cob_key>  lock_keys = { { object_lock_types::lock_table, 0, 0 },
											{ object_lock_types::lock_object, 0, 42 } };

		HANDLE wait_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

		{
			scope_multilock lock = locker.lock(lock_keys);
			system_monitoring_interface::global_mon->log_information("1st lock on thread", __FILE__, __LINE__);

			scope_multilock lock2 = locker.lock(lock_keys);
			system_monitoring_interface::global_mon->log_information("2nd lock on same thread", __FILE__, __LINE__);

			proof_assertions.put_member("same_thread", true);

			global_job_queue->add_job([&locker, &proof_assertions, test_seconds]() -> void
				{
					timer tx2;
					system_monitoring_interface::global_mon->log_information("waiting for lock", __FILE__, __LINE__);
					scope_multilock locko = locker.lock({ object_lock_types::lock_table, 0, 0 });
					if (tx2.get_elapsed_seconds() < test_seconds) {
						system_monitoring_interface::global_mon->log_warning("thread skipped lock", __FILE__, __LINE__);
						proof_assertions.put_member("thread_sufficient", false);
					}
					else {
						proof_assertions.put_member("thread_sufficient", true);
					}
					system_monitoring_interface::global_mon->log_information("lock released", __FILE__, __LINE__);
				}, wait_handle);

			system_monitoring_interface::global_mon->log_information("waiting for job to get lock", __FILE__, __LINE__);
			::Sleep(test_milliseconds);
		}

		system_monitoring_interface::global_mon->log_information("should have released", __FILE__, __LINE__);
		WaitForSingleObject(wait_handle, INFINITE);
		if (tx.get_elapsed_seconds() < test_seconds) {
			proof_assertions.put_member("wait_sufficient", false);
		}
		else {
			proof_assertions.put_member("wait_sufficient", true);
		}

		_proof.put_member("locks", proof_assertions);
		system_monitoring_interface::global_mon->log_function_stop("lock proof", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

}

#endif
