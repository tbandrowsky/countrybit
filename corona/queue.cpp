
#include "corona.h"

// #define COM_INITIALIZATION

namespace corona
{
	namespace database
	{

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
			ZeroMemory(&msg, sizeof(msg));
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

		finish_job::finish_job() : job()
		{
			handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		finish_job::~finish_job()
		{
			::CloseHandle(handle);
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
			ioCompPort = NULL;
			num_outstanding_jobs = 0;
			empty_queue_event = ::CreateEvent(NULL, FALSE, TRUE, NULL);
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
						threads.push_back( std::thread(jobQueueThread, this) );
					}
				}
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

		void job_queue::postJobMessage(job* _jobMessage)
		{
			LONG result;
			::InterlockedIncrement((LONG*)&num_outstanding_jobs);
			::ResetEvent(empty_queue_event);
			result = ::PostQueuedCompletionStatus(ioCompPort, 0, 0, (LPOVERLAPPED)(&_jobMessage->container));
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
								jobQueue->postJobMessage(waiting_job);
							}
						}

						if (jobNotify.shouldDelete) {
							delete waiting_job;
							delete container;
						}
					}

					LONG numJobs = ::InterlockedDecrement((LONG*)(&jobQueue->num_outstanding_jobs));

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
			postJobMessage(&fj);
			::WaitForSingleObject(fj.handle, INFINITE);
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
