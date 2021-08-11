
#include "process.h"
#include "queue.h"

// #define COM_INITIALIZATION

namespace countrybit
{
	namespace system
	{

		void job_notify::notify()
		{
			switch (notification) {
			case postmessage:
				::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
				break;
			case setevent:
				SetEvent((HANDLE)msg.lParam);
				break;
			case coroutine:
				handle();
				break;
			}
		}

		job_notify::job_notify()
		{
			repost = false;
			notification = none;
			shouldDelete = false;
		}

		job_notify::job_notify(const job_notify& _src)
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

		void job_notify::setCoroutine(std::coroutine_handle<> _handle)
		{
			notification = coroutine;
			handle = _handle;
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
			::ZeroMemory(&ovp, sizeof(ovp));
		}

		job::~job()
		{
			;
		}

		job_notify job::on_completed(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
		{
			job_notify t;

			return t;
		}

		// -------------------------------------------------------------------------------

		finish_job::finish_job()
		{
			handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		finish_job::~finish_job()
		{
			::CloseHandle(handle);
		}

		job_notify finish_job::on_completed(job_queue* _callingQueue, DWORD _bytesTransferred, BOOL _success)
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
			ioCompPort = NULL;
			num_outstanding_jobs = 0;
			empty_queue_event = ::CreateEvent(NULL, FALSE, TRUE, NULL);
		}

		job_queue::~job_queue()
		{
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
						threads[i] = std::thread(jobQueueThread, this);
						threadHandles[i] = ::OpenThread(THREAD_ALL_ACCESS, FALSE, (DWORD)threads[i].native_handle());
					}
				}
			}
		}

		void job_queue::postJobMessage(job* _jobMessage)
		{
			LONG result;
			::InterlockedIncrement((LONG*)&num_outstanding_jobs);
			::ResetEvent(empty_queue_event);
			result = ::PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(_jobMessage));
		}

		void job_queue::shutDown()
		{
			int i;
			DWORD timeout;

			// keep sending shutdown messages until all of the queues terminate!

			shutDownOrdered = true;

			for (timeout = WAIT_TIMEOUT;
				timeout == WAIT_TIMEOUT;
				timeout = WaitForMultipleObjects(numWorkerThreads, threadHandles, 200, FALSE))
			{
				postJobMessage(&shutDownJob);
			}

			for (i = 0; i < numWorkerThreads; i++) {
				threads[i].detach();
				::CloseHandle(threadHandles[i]);
				threadHandles[i] = 0;
			}

			::CloseHandle(ioCompPort);

			ioCompPort = NULL;
		}

		void job_queue::kill()
		{
			int i;

			shutDownOrdered = true;

			if (ioCompPort) {
				for (i = 0; i < numWorkerThreads; i++) {
					threads[i].detach();
					::TerminateThread(threadHandles[i], 0);
					::CloseHandle(threadHandles[i]);
					threadHandles[i] = 0;
				}
				::CloseHandle(ioCompPort);
			}

			ioCompPort = NULL;
		}

		unsigned int job_queue::jobQueueThread(job_queue* jobQueue)
		{

			LPOVERLAPPED lpov;
			job* waiting_job;

			BOOL success, shuttingDown;
			DWORD bytesTransferred;
			ULONG_PTR compKey;

			shuttingDown = FALSE;
			job_notify jobNotify;

#ifdef COM_INITIALIZATION
			//	::CoInitializeEx( NULL, COINIT_MULTITHREADED );
			CoInitialize(NULL);
#endif

			while (!shuttingDown) {
				success = ::GetQueuedCompletionStatus(jobQueue->ioCompPort, &bytesTransferred, &compKey, &lpov, INFINITE);
				waiting_job = (job*)lpov;

				if (waiting_job == &(jobQueue->shutDownJob)) {
					shuttingDown = true;
				}
				else if (waiting_job) {
					jobNotify = waiting_job->on_completed(jobQueue, bytesTransferred, success);
					jobNotify.notify();
					if (jobNotify.repost && (!jobQueue->wasShutDownOrdered())) {
						jobQueue->postJobMessage(waiting_job);
					}
					if (jobNotify.shouldDelete) {
						delete waiting_job;
					}
				}

				LONG numJobs = ::InterlockedDecrement((LONG*)(&jobQueue->num_outstanding_jobs));

				if (!numJobs) {
					::SetEvent(jobQueue->empty_queue_event);
				}

			}

#ifdef COM_INITIALIZATION
			::CoUninitialize();
#endif

			return 0;
		};

		void job_queue::waitForThreadFinished()
		{
			finish_job finishJob;

			postJobMessage(&finishJob);
			::WaitForSingleObject(finishJob.handle, INFINITE);
		}

		void job_queue::waitForEmptyQueue()
		{
			::WaitForSingleObject(empty_queue_event, INFINITE);
		}

		int job_queue::numberOfProcessors()
		{
			SYSTEM_INFO si;
			::GetSystemInfo(&si);
			int threadCount = si.dwNumberOfProcessors;
			return threadCount;
		}

	}

}
