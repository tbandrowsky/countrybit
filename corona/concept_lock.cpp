#pragma once

#include "corona.h"

namespace corona
{
	namespace database
	{

		lockable_concept::lockable_concept(const lockable_concept& _holder)
		{
			::InitializeCriticalSection(&holder_lock);
			name = _holder.name;
		}

		lockable_concept::lockable_concept(lockable_concept&& _holder)
		{
			::InitializeCriticalSection(&holder_lock);
			name = std::move(_holder.name);
		}

		lockable_concept lockable_concept::operator=(const lockable_concept& _holder)
		{
			name = _holder.name;
			return *this;
		}

		lockable_concept& lockable_concept::operator=(lockable_concept&& _holder)
		{
			::InitializeCriticalSection(&holder_lock);
			name = std::move(_holder.name);
			return *this;
		}

		lockable_concept::lockable_concept(std::string _name)
		{
			::InitializeCriticalSection(&holder_lock);
			name = _name;
		}

		lockable_concept::lockable_concept()
		{
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept::~lockable_concept()
		{
			::DeleteCriticalSection(&holder_lock);
		}

		concept_lock::concept_lock()
		{
			;
		}

		concept_lock::concept_lock(lockable_concept* _locked)
		{
			if (locked) _locked->lock_concept();
		}

		concept_lock::concept_lock(concept_lock&& _lock)
		{
			locked = _lock.locked;
			_lock.locked = nullptr;
		}

		concept_lock& concept_lock::operator =(concept_lock&& _lock)
		{
			locked = _lock.locked;
			_lock.locked = nullptr;
		}

		concept_lock::~concept_lock()
		{
			if (locked) locked->unlock_concept();
			locked = nullptr;
		}

		concept_locker::concept_locker()
		{
			::InitializeCriticalSection(&map_update_lock);
		}

		concept_locker::~concept_locker()
		{
			::DeleteCriticalSection(&map_update_lock);
		}

		concept_lock concept_locker::checkout(std::string _name)
		{
			concept_lock lock;

			EnterCriticalSection(&this->map_update_lock);

			if (locks.contains(_name))
			{
				try 
				{				
					auto& l = locks[_name];				
					lock = concept_lock(&l);
				}
				catch (std::exception & exc)
				{
					;
				}
			}
			else 
			{
				try
				{
					lockable_concept lck(_name);
					locks.insert_or_assign(_name, lck);
					auto& l = locks[_name];
					lock = concept_lock(&l);					
				}
				catch (std::exception& exc)
				{
					;
				}
			}

			LeaveCriticalSection(&this->map_update_lock);

			return lock;
		}
	}
}
