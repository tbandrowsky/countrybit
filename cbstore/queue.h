#pragma once

#include "windows.h"
#include "windowsx.h"
#include <thread>
#include <coroutine>

namespace countrybit
{
	namespace system
	{
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

		class job
		{
		protected:

			OVERLAPPED ovp;

		public:
			job();
			virtual ~job();
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

		const int maxWorkerThreads = MAXIMUM_WAIT_OBJECTS;

		class job_queue : public lockable
		{

			HANDLE ioCompPort;

			std::vector<std::thread> threads;
			int numWorkerThreads;

			bool shutDownOrdered;

			int num_outstanding_jobs;
			HANDLE empty_queue_event;

		public:

			inline HANDLE getPort() { return ioCompPort; }
			inline DWORD getThreadCount() { return numWorkerThreads; }
			inline bool wasShutDownOrdered() { return shutDownOrdered; }

			job_queue();
			virtual ~job_queue();

			void start(int _numThreads);
			void postJobMessage(job* _jobMessage);
			void shutDown();
			void kill();

			static unsigned int jobQueueThread(job_queue* jobQueue);
			void waitForThreadFinished();
			void waitForEmptyQueue();

			int numberOfProcessors();

		};

	}

}
