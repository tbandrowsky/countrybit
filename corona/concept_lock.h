#pragma once

namespace corona 
{
	namespace database 
	{

		class lockable_concept
		{

			CRITICAL_SECTION holder_lock;

		public:

			std::string name;

			lockable_concept(std::string _name);
			lockable_concept();

			lockable_concept(const lockable_concept& _holder);
			lockable_concept operator=(const lockable_concept& _holder);
			lockable_concept(lockable_concept&& _holder);
			lockable_concept& operator=(lockable_concept&& _holder);

			virtual ~lockable_concept();

			void lock_concept();
			void unlock_concept();
		};

		class concept_lock
		{
			lockable_concept* locked;

		public:

			concept_lock() : locked(nullptr)
			{
				;
			}

			concept_lock(lockable_concept* _locked) : locked(_locked)
			{
				if (locked) _locked->lock_concept();
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
			}

			concept_lock(const concept_lock& _lock) = delete;
			concept_lock& operator =(const concept_lock& _lock) = delete;

			~concept_lock()
			{
				if (locked) locked->unlock_concept();
				locked = nullptr;
			}
		};

		class concept_locker
		{

		private:

			CRITICAL_SECTION map_update_lock;
			std::map<std::string, lockable_concept> locks;

		public:

			concept_locker();
			virtual ~concept_locker();

			concept_lock checkout(std::string _name);
		};

	}
}
