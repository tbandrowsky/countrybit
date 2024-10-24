/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
A concept lock is a lock designed to be used around a 
business process.

Notes

For Future Consideration
*/


#ifndef CORONA_CONCEPT_LOCK_H
#define CORONA_CONCEPT_LOCK_H

namespace corona
{


	class concept_lock;

	class lockable_concept
	{

		CRITICAL_SECTION holder_lock;

	public:

		friend class concept_lock;

		std::string name;

		lockable_concept(std::string _name)
		{
			::InitializeCriticalSection(&holder_lock);
			name = _name;
		}

		lockable_concept()
		{
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept(const lockable_concept& _holder)
		{
			name = _holder.name;
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept(lockable_concept&& _holder)
		{
			name = std::move(_holder.name);
			::InitializeCriticalSection(&holder_lock);
		}

		~lockable_concept()
		{
			::DeleteCriticalSection(&holder_lock);
		}

		lockable_concept operator=(const lockable_concept& _holder)
		{
			name = _holder.name;
			return *this;
		}

		lockable_concept& operator=(lockable_concept&& _holder)
		{
			name = std::move(_holder.name);
			return *this;
		}

	};

	class concept_lock
	{
		lockable_concept* locked;

		friend class lockable_concept;

	public:

		concept_lock(lockable_concept* _locked)
		{
			locked = _locked;
			::EnterCriticalSection(&locked->holder_lock);
		}

		concept_lock(const concept_lock& _lock) = delete;
		concept_lock& operator =(const concept_lock& _lock) = delete;

		concept_lock()
		{
			locked = nullptr;
		}

		concept_lock(concept_lock&& _lock)
		{
			locked = _lock.locked;
			_lock.locked = nullptr;
		}

		concept_lock& operator =(concept_lock&& _lock)
		{
			locked = _lock.locked;
			_lock.locked = nullptr;
			return *this;
		}

		~concept_lock()
		{
			if (locked) ::LeaveCriticalSection(&locked->holder_lock);
			locked = nullptr;
		}


	};

}

#endif
