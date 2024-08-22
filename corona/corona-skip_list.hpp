/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_SKIP_LIST_H
#define CORONA_SKIP_LIST_H

namespace corona {

	const int MaxNumberOfLevels = 20;
	const int MaxLevel = MaxNumberOfLevels - 1;

	template <typename KEY, typename VALUE, int SORT_ORDER = 1> class skip_list
	{
	public:

		class node_type;

	public:


		skip_list()
			: header(MaxNumberOfLevels),
			level(MaxLevel),
			count(0),
			deleted(false),
			id(0)
		{
		}

		virtual ~skip_list()
		{
			clear();
		}

		skip_list(const skip_list& _src)
			: header(MaxNumberOfLevels),
			level(MaxLevel),
			count(0),
			deleted(false)
		{
			deleted = _src.deleted;
			auto n = _src.first_node();
			while (n != nullptr)
			{
				update_node(n->keyValue);
				n = _src.next_node(n);
			}
			check_id = _src.check_id;
			id = _src.id;
		}

		skip_list(skip_list&& _src)
		{
			level = _src.level;
			count = _src.count;
			header = _src.header;
			deleted = _src.deleted;
			for (int i = 0; i < _src.header.nlevels; i++) {
				_src.header.forward[i] = nullptr;
			}
			_src.level = MaxLevel;
			_src.count = 0;
			check_id = _src.check_id;
			id = _src.id;

		}

		skip_list& operator = (skip_list& _src)
		{
			if (this != &_src)
			{
				clear();
				skip_list::node_type temp(MaxNumberOfLevels);
				header = temp;
				auto n = _src.first_node();
				while (n != nullptr)
				{
					update_node(n->keyValue);
					n = _src.next_node(n);
				}
				check_id = _src.check_id;
				id = _src.id;
				deleted = _src.deleted;
			}
			return *this;
		}

		skip_list& operator = (skip_list&& _src)
		{
			if (this != &_src)
			{
				clear();
				level = _src.level;
				count = _src.count;
				header = _src.header;
				for (int i = 0; i < _src.header.nlevels; i++) {
					_src.header.forward[i] = nullptr;
				}
				_src.level = MaxLevel;
				_src.count = 0;
				check_id = _src.check_id;
				id = _src.id;
				deleted = _src.deleted;
			}
			return *this;
		}

		int64_t id;
		int64_t check_id;
		bool deleted;

		bool pop_front()
		{
			bool result = false;
			node_type* q;

			q = header.forward[0];

			if (q != nullptr)
			{
				result = true;
				remove_node(q->key);
			}
			return result;
		}

		void clear()
		{
			count = 0;
			for (auto nt = first_node(); nt != nullptr; )
			{
				auto temp = next_node(nt);
				delete nt;
				nt = temp;
			}
			for (int i = 0; i < header.nlevels; i++)
			{
				header.forward[i] = nullptr;
			}
		}

		class iterator
		{
			skip_list<KEY, VALUE, SORT_ORDER>* base;
			skip_list<KEY, VALUE, SORT_ORDER>::node_type* current;
			std::function<bool(std::pair<KEY, VALUE>&)> predicate;

		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = std::pair<KEY, VALUE>;
			using pointer = std::pair<KEY, VALUE>*;  // or also value_type*
			using reference = std::pair<KEY, VALUE>&;  // or also value_type&

			iterator(skip_list<KEY, VALUE, SORT_ORDER>* _base,
				skip_list<KEY, VALUE, SORT_ORDER>::node_type* _current,
				std::function<bool(std::pair<KEY, VALUE>&)> _predicate
			) :
				base(_base),
				current(_current),
				predicate(_predicate)
			{
				while (current and !predicate(current->data))
				{
					current = _base->next_link(current);
				}

			}

			iterator(skip_list<KEY, VALUE, SORT_ORDER>* _base,
				skip_list<KEY, VALUE, SORT_ORDER>::node_type* _current
			) :
				base(_base),
				current(_current)
			{
				predicate = [](skip_list<KEY, VALUE, SORT_ORDER>::node_type& a) { return true;  };
			}

			iterator() : base(nullptr), current(nullptr)
			{
				predicate = [](skip_list<KEY, VALUE, SORT_ORDER>::node_type& a) { return true;  };
			}

			iterator& operator = (const iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				predicate = _src.predicate;
				return *this;
			}

			inline std::pair<KEY, VALUE>& operator *()
			{
				return current->keyValue;
			}

			inline std::pair<KEY, VALUE>* operator->() const
			{
				return &current->keyValue;
			}

			inline KEY& get_key() const
			{
				return current->keyValue.first_link;
			}

			inline VALUE& get_value()
			{
				return current->keyValue.second;
			}

			inline iterator begin() const
			{
				return iterator(base, base->first_node());
			}

			inline iterator end() const
			{
				return iterator(base, nullptr);
			}

			inline iterator operator++()
			{
				do
				{
					current = base->next_node(current);
					if (not current)
						return iterator(base, nullptr, predicate);
				} while (!predicate(current->data));
				return iterator(base, current, predicate);
			}

			inline iterator operator++(int)
			{
				iterator tmp(*this);
				operator++();
				return tmp;
			}

			bool operator == (const iterator& _src) const
			{
				return _src.current == current;
			}

			bool operator != (const iterator& _src) const
			{
				return _src.current != current;
			}

		};

		skip_list<KEY, VALUE, SORT_ORDER>::iterator begin()
		{
			return iterator(this, first_node());
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator end()
		{
			return iterator(this, nullptr);
		}

		bool erase(skip_list<KEY, VALUE, SORT_ORDER>::iterator& _iter)
		{
			return this->remove_node(_iter->first_link);
		}

		bool erase(const KEY& key)
		{
			return this->remove_node(key);
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator at(const KEY& key)
		{
			return iterator(this, this->find_node(key));
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator first_like(const KEY& key)
		{
			return iterator(this, this->find_first_node(key));
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator operator[](const KEY& key)
		{
			return iterator(this, this->find_node(key));
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator get_by_index(int id)
		{
			return iterator(this, this->nodes_by_id[id]);
		}

		bool has(const KEY& key, VALUE& value)
		{
			auto n = this->find_node(key);
			return (n and n->keyValue.second == value);
		}

		bool has(const KEY& key, std::function<bool(VALUE& src)> pred)
		{
			auto n = this->find_node(key);
			return (n and pred(n->keyValue.second));
		}

		VALUE& first_value()
		{
			auto n = first_node();
			return n->keyValue.second;
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(std::pair<KEY, VALUE>& kvp)
		{
			auto modified_node = this->update_node(kvp);
			return iterator(this, modified_node);
		}

		inline skip_list<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(KEY key, VALUE value)
		{
			std::pair<KEY, VALUE> kvp(key, value);
			return insert_or_assign(kvp);
		}

		skip_list<KEY, VALUE, SORT_ORDER>::iterator put(std::pair<KEY, VALUE>& kvp)
		{
			auto modified_node = this->update_node(kvp);
			return iterator(this, modified_node);
		}

		inline skip_list<KEY, VALUE, SORT_ORDER>::iterator put(KEY key, VALUE value)
		{
			std::pair<KEY, VALUE> kvp(key, value);
			return insert_or_assign(kvp);
		}

		static bool test()
		{
			skip_list test;
			bool result = true;

			auto t1 = test.insert_or_assign(5, "hello");
			if (t1.get_key() != 5 or t1.get_value() != "hello" or t1->second != "hello")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}
			auto t2 = test.insert_or_assign(7, "goodbye");
			if (t2.get_key() != 7 or t2.get_value() != "goodbye" or t2->second != "goodbye")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}
			auto t3 = test.insert_or_assign(7, "something");
			if (t3.get_key() != 7 or t3->second != "something")
			{
				std::cout << "fail: wrong updated value." << std::endl;
				return false;
			}
			auto t4 = test[7];
			if (t4.get_key() != 7 or t4->second != "something")
			{
				std::cout << "fail: wrong [] access." << std::endl;
				return false;
			}
			auto t5 = test[6];
			if (t5 != std::end(test))
			{
				std::cout << "fail: wrong null access." << std::endl;
				return false;
			}

			int count = 0;

			for (auto t : test)
			{
				count++;
			}

			if (count != 2)
			{
				std::cout << "fail: wrong number of iterations." << std::endl;
				return false;
			}

			auto t6 = test.put(2, "hello super");
			if (t6.get_key() != 2 or t6.get_value() != "hello super")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			auto t7 = test.put(1, "first");
			if (t7.get_key() != 1 or t7->second != "first")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			t7 = test.put(1, "second");
			if (t7.get_key() != 1 or t7->second != "second")
			{
				std::cout << "fail: wrong inserted value." << std::endl;
				return false;
			}

			count = std::count_if(test.begin(), test.end(), [](auto) {
				return true;
				});

			if (count != 4)
			{
				std::cout << "fail: wrong number of iterations." << std::endl;
				return false;
			}

			std::cout << "first loop" << std::endl;

			for (auto& item : test)
			{
				std::cout << item.first_link << " " << item.second << std::endl;
			}

			std::cout << "starting later loop" << std::endl;

			for (auto& item : test[2])
			{
				std::cout << item.first_link << " " << item.second << std::endl;
			}

			auto x = test[1];
			test.erase(x);
			test.erase(7);

			std::cout << "erased loop" << std::endl;

			for (auto& item : test)
			{
				std::cout << item.first_link << " " << item.second << std::endl;
			}

			return result;
		}

	private:

		node_type header;
		int level;
		int count;
		std::vector<node_type*> nodes_by_id;

		int randomLevel()
		{
			double r = ((double)rand() / (RAND_MAX));
			int level = (int)(log(1. - r) / log(1. - .5));
			if (level < 1) level = 0;
			else if (level >= MaxLevel) level = MaxLevel;
			return level;
		}

	public:

		inline int size() { return count; }

		class node_type
		{

			void create(int nLevels)
			{
				forward = new node_type * [nlevels + 1];
				for (int i = 0; i <= nlevels; i++)
					forward[i] = nullptr;
			}

			void destroy()
			{
				if (forward)
				{
					delete[] forward;
				}
				nlevels = 0;
				forward = nullptr;
				//					std::cout << "c destroyed " << this << std::endl;
			}

		public:

			std::pair<KEY, VALUE> keyValue;
			int			nlevels;
			node_type** forward;
			int			node_index;

			node_type() : forward(nullptr), nlevels(0)
			{
			}

			node_type(int _nlevels, std::pair<KEY, VALUE> _keyValue) :
				forward(nullptr),
				nlevels(_nlevels),
				keyValue(_keyValue),
				node_index(0)
			{
				create(nlevels);
			}

			node_type(int _nlevels) :
				forward(nullptr),
				nlevels(_nlevels),
				node_index(0)
			{
				create(nlevels);
			}

			node_type(const node_type& src) :
				forward(nullptr),
				nlevels(src.nlevels),
				node_index(src.node_index)
			{
				keyValue = src.keyValue;
				forward = new node_type * [nlevels + 1];
				if (forward) {
					for (int i = 0; i < nlevels; i++)
					{
						forward[i] = src.forward[i];
					}
				}
			}

			node_type(node_type&& src)
			{
				nlevels = src.nlevels;
				keyValue = src.keyValue;
				forward = src.forward;
				node_index = src.node_index;
				src.forward = nullptr;
				src.nlevels = 0;
			}

			node_type& operator=(const node_type& src)
			{
				if (&src != this) {
					destroy();
					keyValue = src.keyValue;
					nlevels = src.nlevels;
					node_index = src.node_index;
					forward = new node_type * [nlevels + 1];
					if (forward) {
						for (int i = 0; i < nlevels; i++)
						{
							forward[i] = src.forward[i];
						}
					}
				}
				return *this;
			}

			node_type& operator=(const node_type&& src)
			{
				if (&src != this) {
					destroy();
					keyValue = src.keyValue;
					forward = src.forward;
					node_index = src.node_index;
					src.forward = nullptr;
					src.nlevels = 0;
				}
				return *this;
			}

			virtual ~node_type()
			{
				destroy();
			}
		};

		bool contains(skip_list<KEY, VALUE>& _src)
		{
			bool all = true;
			for (auto& f : _src)
			{
				if (not find_node(f.first_link))
				{
					all = false;
					break;
				}
			}
			return all;
		}

	private:

		// compare a node to a key for equality

		inline int compare(node_type* _node, const KEY& key)
		{
			if (_node != nullptr)
			{
				if (_node->keyValue.first_link < key)
					return -SORT_ORDER;
				else if (_node->keyValue.first_link > key)
					return SORT_ORDER;
				else
					return 0;
			}
			else
			{
				return 1;
			}
		}

		node_type* find_node(node_type** update, const KEY& key)
		{
			node_type* p, * q, * found = nullptr;

			for (int k = level; k >= 0; k--)
			{
				p = &header;
				q = p->forward[k];
				auto comp = compare(q, key);
				while (comp < 0)
				{
					p = q;
					q = q->forward[k];
					comp = compare(q, key);
				}
				if (comp == 0)
					found = q;
				update[k] = p;
			}

			return found;
		}

		node_type* find_first(node_type** update, const KEY& key)
		{
			node_type* p, * q, * found = nullptr, * last_link = nullptr;

			for (int k = level; k >= 0; k--)
			{
				p = &header;
				q = p->forward[k];
				last_link = q;
				auto comp = compare(q, key);
				while (comp < 0)
				{
					p = q;
					last_link = q;
					q = q->forward[k];
					comp = compare(q, key);
				}
				if (comp == 0)
					found = q;
				else if (comp < 0)
					found = last_link;
				update[k] = p;
			}

			return found;
		}


		node_type* update_node(const std::pair<KEY, VALUE>& kvp)
		{
			int k;
			node_type* update[MaxNumberOfLevels], * p;

			node_type* q = find_node(update, kvp.first_link);

			if (q)
			{
				q->keyValue.second = kvp.second;
				return q;
			}

			k = randomLevel();
			if (k > level) {
				k = ++level;
				update[k] = &header;
			};

			q = new node_type(k, kvp);

			if (q) {

				q->node_index = nodes_by_id.size();
				nodes_by_id.push_back(q);

				count++;

				do {
					p = update[k];
					q->forward[k] = p->forward[k];
					p->forward[k] = q;
				} while (--k >= 0);

				return q;
			}

			return nullptr;
		}

		bool remove_node(const KEY& key)
		{
			int k, m;
			node_type* update[MaxNumberOfLevels];
			node_type* p;

			m = level;
			node_type* q = find_node(update, key);

			if (q) {
				k = 0;
				p = update[k];
				while (k <= m and p->forward[k] == q)
				{
					p->forward[k] = q->forward[k];
					k++;
					p = update[k];
				}
				count--;
				while (header.forward[m] == nullptr and m > 0) {
					m--;
				}
				level = m;
				//					std::cout << "c removed " << (long long)q << std::endl;
				delete q;
				return true;
			}
			else {
				return false;
			}
		}

		node_type* find_node(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			node_type* update[MaxNumberOfLevels];
			return find_node(update, key);
		}

		node_type* find_first_node(const KEY& key)
		{
#ifdef	TIME_SKIP_LIST
			benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
			node_type* update[MaxNumberOfLevels];
			return find_first(update, key);
		}

		node_type* first_node() const
		{
			return header.forward != nullptr ? header.forward[0] : nullptr;
		}

		node_type* next_node(node_type* _node) const
		{
			if (_node) {
				_node = _node->forward[0];
				return _node;
			}
			return nullptr;
		}

	};


	template <typename KEY, typename VALUE, int SORT_ORDER> std::ostream& operator <<(std::ostream& output, skip_list<KEY, VALUE, SORT_ORDER>& src)
	{
		bool space = false;
		for (auto& l : src)
		{
			if (space)
				output << "  ";
			space = true;
			output << l.second;
		}
		return output;
	}
}

#endif
