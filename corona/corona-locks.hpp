/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
A concept lock is a lock designed to be used around a 
business process.

Notes

For Future Consideration
*/

#pragma once

#ifndef CORONA_LOCKS_HPP
#define CORONA_LOCKS_HPP

namespace corona
{
	// simple wrapper for interlocked increment

	class lockable
	{
		CRITICAL_SECTION section;
		int lock_count;

		void initialize()
		{
			::InitializeCriticalSection(&section);
			lock_count = 0;
		}

	public:

		lockable()
		{
			::InitializeCriticalSection(&section);
			lock_count = 0;
		}

		lockable(const lockable& _src) = delete;
		lockable(lockable&& _src) = delete;
		lockable& operator = (const lockable& _src) = delete;
		lockable& operator = (lockable&& _src) = delete;

		~lockable()
		{
			::DeleteCriticalSection(&section);
		}

		void lock()
		{
			::EnterCriticalSection(&section);
			lock_count++;
		}

		void unlock()
		{
			lock_count--;
			::LeaveCriticalSection(&section);
		}

		bool locked()
		{
			return lock_count > 0;
		}
	};

	class scope_lock
	{
		lockable* locked;

	public:

		scope_lock(lockable& _lockable)
		{
			locked = &_lockable;
			locked->lock();
		}

		~scope_lock()
		{
			locked->unlock();
		}

	};

	class shared_lockable
	{
		SRWLOCK lock;

	public:

		shared_lockable()
		{
			InitializeSRWLock(&lock);
		}

		inline void write_lock()
		{
			::AcquireSRWLockExclusive(&lock);
		}

		inline void write_unlock()
		{
			::ReleaseSRWLockExclusive(&lock);
		}

		inline void read_lock()
		{
			::AcquireSRWLockShared(&lock);
		}

		inline void read_unlock()
		{
			::ReleaseSRWLockShared(&lock);
		}
	};

	class read_scope_lock
	{
		shared_lockable* locked;

	public:

		read_scope_lock(shared_lockable& _lockable)
		{
			locked = &_lockable;
			locked->read_lock();
		}

		~read_scope_lock()
		{
			locked->read_unlock();
		}
	};

	class write_scope_lock
	{
		shared_lockable* locked;

	public:

		write_scope_lock(shared_lockable& _lockable)
		{
			locked = &_lockable;
			locked->write_lock();
		}

		~write_scope_lock()
		{
			locked->write_unlock();
		}
	};

	template<typename T>
	concept read_lockable_type = requires(T a)
	{
		{ a.read_lock() };
		{ a.read_unlock() };
	};

	template<typename T>
	concept write_lockable_type = requires(T a)
	{
		{ a.write_lock() };
		{ a.write_unlock() };
	};

	template <read_lockable_type T>
	class read_locked_sp
	{
		std::shared_ptr<T> data;
	public:
		read_locked_sp(std::shared_ptr<T>& _data) :
			data(_data)
		{
			data->read_lock();
		}

		read_locked_sp(const read_locked_sp& _src) = delete;
		read_locked_sp& operator =(const read_locked_sp& _src) = delete;

		read_locked_sp(read_locked_sp&& _src)
		{
			std::swap(data, _src.data);
		}

		read_locked_sp& operator =(read_locked_sp&& _src)
		{
			std::swap(data, _src.data);
			return *this;
		}

		virtual ~read_locked_sp()
		{
			if (data) 
			{
				data->read_unlock();
			}
		}

		T* get()
		{
			return data.get();
		}

		T* operator->()
		{
			return data.get();
		}

		operator bool()
		{
			return data.get() != nullptr;
		}

		bool exists()
		{
			return data.get() != nullptr;
		}

	};

	template <write_lockable_type T>
	class write_locked_sp
	{
		std::shared_ptr<T> data;
	public:
		write_locked_sp(std::shared_ptr<T>& _data) :
			data(_data)
		{
			data->write_lock();
		}

		write_locked_sp(const write_locked_sp& _src) = delete;
		write_locked_sp& operator =(const write_locked_sp& _src) = delete;

		write_locked_sp(write_locked_sp&& _src)
		{
			std::swap(data, _src.data);
		}

		write_locked_sp& operator =(write_locked_sp&& _src)
		{
			std::swap(data, _src.data);
			return *this;
		}

		virtual ~write_locked_sp()
		{
			if (data)
				data->write_unlock();
		}

		operator bool()
		{
			return data.get() != nullptr;
		}

		T* get()
		{
			return data.get();
		}

		T* operator->()
		{
			return data.get();
		}

		bool exists()
		{
			return data.get() != nullptr;
		}

	};

	class scope_multilock 
	{
		std::vector<HANDLE> signals;

	public:

		scope_multilock(std::vector<HANDLE> &&_signals)
		{
			signals = std::move(_signals);
			for (auto signal : signals) 
			{
				::WaitForSingleObject(signal, INFINITE);
			}
		}

		void add_signal(HANDLE _signal)
		{
			signals.push_back(_signal);
			::WaitForSingleObject(_signal, INFINITE);
		}

		scope_multilock(scope_multilock& _src)
		{
			signals = _src.signals;
		}

		scope_multilock(scope_multilock&& _src)
		{
			std::swap(signals, _src.signals);
		}

		scope_multilock& operator =(scope_multilock& _src)
		{
			signals = _src.signals;
			return *this;
		}

		scope_multilock&operator =(scope_multilock&& _src)
		{
			std::swap(signals, _src.signals);
			return *this;
		}

		virtual ~scope_multilock()
		{
			for (auto signal : signals)
			{
				::ReleaseMutex(signal);
			}
		}
	};

	template <typename key_type>
	concept comparable_key = requires(key_type a, key_type b)
	{
		{ a < b };
		{ b < a };
	};

	template <typename key_type>  
	requires comparable_key<key_type>
	class concept_locker 
	{

	private:
		std::map<key_type, HANDLE>	locks;
		lockable					lock_guard;

		void load_signals(std::vector<HANDLE>& _signals, const std::vector<key_type>& _keys)
		{
			scope_lock lockme(lock_guard);

			for (auto key : _keys)
			{
				HANDLE signal = nullptr;
				auto foundkeys = locks.find(key);
				if (foundkeys != std::end(locks))
				{
					signal = foundkeys->second;
				}
				else
				{
					signal = ::CreateMutex(NULL, FALSE, NULL);
					locks.insert_or_assign(key, signal);
				}
				_signals.push_back(signal);
			}
		}

	public:

		concept_locker()
		{
			;
		}

		concept_locker(concept_locker& _src)
		{
			locks = _src.locks;
		}

		concept_locker(concept_locker&& _src)
		{
			std::swap(locks, _src.locks);
		}

		virtual ~concept_locker()
		{
			for (auto& lock : locks) 
			{
				CloseHandle(lock.second);
				lock.second = nullptr;
			}
		}

		concept_locker &operator =(concept_locker& _src)
		{
			locks = _src.locks;
			return *this;
		}

		concept_locker& operator = (concept_locker&& _src)
		{
			std::swap(locks, _src.locks);
			return *this;
		}

		scope_multilock lock()
		{
			std::vector<HANDLE> signals;
			// now, if my calculations are correct we return a scope_multilock with the array of signals
			// the lock owner will wait and lock the signals, so the thread waiting for the locks 
			// will wait, but consumers of this lock manager itself will not.
			return scope_multilock(std::move(signals));
		}

		scope_multilock lock(const key_type& _key)
		{
			std::vector<HANDLE> signals;
			std::vector<key_type> keys;
			keys.push_back(_key);
			// signals and keys are passed by reference.
			// the load_signals keeps the locks map guarded,
			// so we can find the signals we want to lock,
			// or create them.
			load_signals(signals, keys);

			// now, if my calculations are correct we return a scope_multilock with the array of signals
			// the lock owner will wait and lock the signals, so the thread waiting for the locks 
			// will wait, but consumers of this lock manager itself will not.
			return scope_multilock(std::move(signals));
		}

		scope_multilock lock(const std::vector<key_type>& _keys)
		{
			std::vector<HANDLE> signals;

			// signals and keys are passed by reference.
			// the load_signals keeps the locks map guarded,
			// so we can find the signals we want to lock,
			// or create them.
			load_signals(signals, _keys);
			
			// now, if my calculations are correct we return a scope_multilock with the array of signals
			// the lock owner will wait and lock the signals, so the thread waiting for the locks 
			// will wait, but consumers of this lock manager itself will not.
			return scope_multilock(std::move(signals));
		}

		void add_lock(scope_multilock& _owner, const key_type& _key)
		{
			std::vector<HANDLE> signals;
			std::vector<key_type> keys;
			keys.push_back(_key);
			load_signals(signals, keys);
			_owner.add_signal(signals[0]);
		}
	};

	enum class object_lock_types
	{
		lock_none = 0,
		lock_table = 1,
		lock_object = 2,
		lock_block = 3
	};

	class cob_key
	{
	public:
		object_lock_types	lock_type;
		int64_t				class_id;
		int64_t				object_id;

		cob_key() : class_id(0), object_id(0)
		{
			;
		}

		cob_key(object_lock_types _lock_type, int64_t _class_id, int64_t _object_id) :
			lock_type(_lock_type), class_id(_class_id), object_id(_object_id)
		{
			;
		}

		cob_key(const cob_key& _src)
		{
			lock_type = _src.lock_type;
			class_id = _src.class_id;
			object_id = _src.object_id;
		}

		cob_key(cob_key&& _src)
		{
			std::swap(lock_type, _src.lock_type);
			std::swap(class_id, _src.class_id);
			std::swap(object_id, _src.object_id);
		}

		cob_key& operator = (const cob_key& _src)
		{
			lock_type = _src.lock_type;
			class_id = _src.class_id;
			object_id = _src.object_id;
			return *this;
		}

		cob_key& operator = (cob_key&& _src)
		{
			std::swap(lock_type, _src.lock_type);
			std::swap(class_id, _src.class_id);
			std::swap(object_id, _src.object_id);
			return *this;
		}

		bool operator<(const cob_key& _src) const
		{
			return std::tie(lock_type, class_id, object_id) < std::tie(lock_type, _src.class_id, _src.object_id);
		}

	};

	using object_locker = concept_locker<cob_key>;


}

#endif
