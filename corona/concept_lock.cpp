#pragma once

#include "corona.h"

namespace corona
{
	namespace database
	{

		lockable_concept::lockable_concept(std::string _name)
		{
			::InitializeCriticalSection(&holder_lock);
			name = _name;
		}

		lockable_concept::lockable_concept()
		{
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept::lockable_concept(const lockable_concept& _holder)
		{
			name = _holder.name;
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept::lockable_concept(lockable_concept&& _holder)
		{
			name = std::move(_holder.name);
			::InitializeCriticalSection(&holder_lock);
		}

		lockable_concept::~lockable_concept()
		{
			::DeleteCriticalSection(&holder_lock);
		}

		lockable_concept lockable_concept::operator=(const lockable_concept& _holder)
		{
			name = _holder.name;
			return *this;
		}

		lockable_concept& lockable_concept::operator=(lockable_concept&& _holder)
		{
			name = std::move(_holder.name);
			return *this;
		}

		concept_lock lockable_concept::lock()
		{
			concept_lock locko(this);

			return locko;
		}

		concept_lock::concept_lock()
		{
			locked = nullptr;
		}

		concept_lock::concept_lock(lockable_concept* _locked)
		{
			locked = _locked;
			::EnterCriticalSection(&locked->holder_lock);
		}

		concept_lock::concept_lock(concept_lock&& _lock)
		{
			locked = nullptr;
			std::swap(locked,_lock.locked);
		}

		concept_lock& concept_lock::operator =(concept_lock&& _lock)
		{
			locked = nullptr;
			std::swap(locked, _lock.locked);
			return *this;
		}

		concept_lock::~concept_lock()
		{
			if (locked) ::LeaveCriticalSection(&locked->holder_lock);
			locked = nullptr;
		}

		concept_locker::concept_locker()
		{
			;
		}

		concept_locker::~concept_locker()
		{
			;
		}

		concept_lock concept_locker::checkout(std::string _name)
		{
			concept_lock map_locked = map_lock.lock();
			concept_lock check_out_lock;

			if (locks.contains(_name))
			{
				try 
				{				
					auto& l = locks[_name];				
					check_out_lock = l.lock();
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
					check_out_lock = l.lock();
				}
				catch (std::exception& exc)
				{
					;
				}
			}

			return check_out_lock;
		}
	}
}
