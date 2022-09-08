#pragma once

namespace corona 
{
	namespace database 
	{
		class lockable_concept;

		class concept_lock
		{
			lockable_concept* locked;

			friend class lockable_concept;
			concept_lock(lockable_concept* _locked);

		public:

			concept_lock();

			concept_lock(concept_lock&& _lock);
			concept_lock& operator =(concept_lock&& _lock);

			concept_lock(const concept_lock& _lock) = delete;
			concept_lock & operator =(const concept_lock & _lock) = delete;

			~concept_lock();
		};

		class lockable_concept
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

		class concept_locker
		{

		private:

			lockable_concept map_lock;
			std::map<std::string, lockable_concept> locks;

		public:

			concept_locker();
			virtual ~concept_locker();

			concept_lock checkout(std::string _name);
		};

	}
}
