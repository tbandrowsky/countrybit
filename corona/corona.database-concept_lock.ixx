module;

#include <map>

export module corona.database:concept_lock;

import :stdapi;

class lockable_concept;

export class concept_lock
{
	lockable_concept* locked;

	friend class lockable_concept;
	concept_lock(lockable_concept* _locked);

public:

	concept_lock();

	concept_lock(concept_lock&& _lock);
	concept_lock& operator =(concept_lock&& _lock);

	concept_lock(const concept_lock& _lock) = delete;
	concept_lock& operator =(const concept_lock& _lock) = delete;

	~concept_lock();
};

export class lockable_concept
{

	CRITICAL_SECTION holder_lock;

public:

	friend class concept_lock;

	std::string name;

	lockable_concept(std::string _name);
	lockable_concept();

	lockable_concept(const lockable_concept& _holder);
	lockable_concept operator=(const lockable_concept& _holder);
	lockable_concept(lockable_concept&& _holder);
	lockable_concept& operator=(lockable_concept&& _holder);

	virtual ~lockable_concept();

	concept_lock lock();
};

export class concept_locker
{

private:

	lockable_concept map_lock;
	std::map<std::string, lockable_concept> locks;

public:

	concept_locker();
	virtual ~concept_locker();

	concept_lock checkout(std::string _name);
};


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
	locked = _lock.locked;
	_lock.locked = nullptr;
}

concept_lock& concept_lock::operator =(concept_lock&& _lock) 
{
	locked = _lock.locked;
	_lock.locked = nullptr;
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
