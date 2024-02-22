#ifndef CORONA_QUEUE_H
#define CORONA_QUEUE_H

#include "corona-windows-lite.h"
#include "corona-store_box.hpp"
#include "corona-constants.hpp"

#include <coroutine>
#include <vector>
#include <thread>
#include <atomic>
#include <compare>
#include <functional>

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
		virtual job_notify execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success);
		friend class job_queue;
	};

	using runnable = std::function<void()>;

	class general_job : public job
	{
	public:
		runnable function_to_run;
		job_container container;
		general_job();
		general_job(runnable _runnable);
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

	// simple wrapper for interlocked increment

	class lockable
	{
		CRITICAL_SECTION section;
		int lock_count;
		void initialize();

	public:

		lockable();

		lockable(const lockable& _src) = delete;
		lockable(lockable&& _src) = delete;
		lockable& operator = (const lockable& _src) = delete;
		lockable& operator = (lockable&& _src) = delete;

		virtual ~lockable();

		void lock();
		void unlock();
		bool locked();
	};

	class scope_lock
	{
		lockable* locked;

	public:

		scope_lock(lockable& _lockable);
		virtual ~scope_lock();
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
		void add_job(job* _jobMessage);
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
		;
	}

	general_job::general_job(runnable _runnable)
	{
		function_to_run = _runnable;
	}

	general_job::~general_job()
	{
		;
	}

	job_notify general_job::execute(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
	{
		job_notify jobNotify;

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

	// -------------------------------------------------------------------------------
	// simple wrapper for interlocked increment

	lockable::lockable()
	{
		::InitializeCriticalSection(&section);
		lock_count = 0;
	}

	void lockable::initialize()
	{
		::InitializeCriticalSection(&section);
		lock_count = 0;
	}

	lockable::~lockable()
	{
		::DeleteCriticalSection(&section);
	}

	void lockable::lock()
	{
		::EnterCriticalSection(&section);
		lock_count++;
	}

	void lockable::unlock()
	{
		lock_count--;
		::LeaveCriticalSection(&section);
	}

	bool lockable::locked()
	{
		return lock_count > 0;
	}

	// ---

	scope_lock::scope_lock(lockable& _lockable)
	{
		locked = &_lockable;
		locked->lock();
	}

	scope_lock::~scope_lock()
	{
		locked->unlock();
	}

	// ---

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

		if (_numThreads > maxWorkerThreads || _numThreads == 0) _numThreads = threadCount;

		if (!ioCompPort) {

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
		++num_outstanding_jobs;
		ResetEvent(empty_queue_event);
		result = PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(&_jobMessage->container));
	}

	void job_queue::post_ui_message(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto success = ::PostThreadMessage(thread_id, msg, wparam, lparam);		
	}

	unsigned int job_queue::jobQueueThread(job_queue* jobQueue)
	{

		LPOVERLAPPED lpov;
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
			if (success && lpov) {
				container = (job_container*)lpov;
				if (container) {
					waiting_job = container->jobdata;

					if (waiting_job) {
						jobNotify = waiting_job->execute(jobQueue, bytesTransferred, success);
						jobNotify.notify();
						if (jobNotify.repost && (!jobQueue->wasShutDownOrdered())) {
							jobQueue->add_job(waiting_job);
						}
					}

					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}

				LONG numJobs = --jobQueue->num_outstanding_jobs;

				if (!numJobs) {
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

}

#endif
