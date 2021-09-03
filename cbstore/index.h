#pragma once

#include "table.h"

/*********************************************************************
  Countrybit SAT
  (c) 2021 TJ Bandrowsky
  MIT License
**********************************************************************/

#pragma once

/**********************************************************
Implementation of design by Bill Pugh.
***********************************************************/

#include <functional>
#include <string>
#include <iterator> // For std::forward_iterator
#include <cstddef>  // For std::ptrdiff_t
#include <algorithm>
#include <iostream>
#include <cmath>

namespace countrybit
{
	namespace database
	{
		const int MaxNumberOfLevels = 20;
		const int MaxLevel = MaxNumberOfLevels - 1;

		template <typename KEY, typename VALUE, int SORT_ORDER = 1> class index_node_hdr 
		{
		public:

			std::pair<KEY, VALUE>	key_value;
			row_id_type				node_index;
		};

		using index_ref = row_id_type;

		template <typename KEY, typename VALUE, int SORT_ORDER = 1> class index : public parent_child_table<index_node_hdr<KEY,VALUE>, index_ref>
		{
		public:

			using index_node = parent_child_holder<index_node_hdr<KEY, VALUE>, index_ref>;

		private:

			class node_type 
			{
				index* me;
				index_node nd;

			public:

				node_type() : me(nullptr)
				{
					;
				}

				node_type(index* _me, index_node _nd) 
					: me(_me),
					nd(_nd)
				{
					;
				}

				node_type(index* _me, row_id_type r)
					: me(_me)
				{
					nd = me->get( r );
				}

				bool is_null()
				{
					return nd.row_id() == 0;
				}

				std::pair<KEY, VALUE>& keyValue()
				{
					return nd.parent().key_value;
				}

				KEY& key()
				{
					return nd.parent().key_value.first;
				}

				VALUE& key()
				{
					return nd.parent().key_value.second;
				}

				node_type foward(int idx)
				{
					return node_type( me, nd.child(idx) );
				}
			};

			bool deleted;
			int level;
			int count;
			row_id_type header_id;

			node_type header()
			{
				return node_type(this, header_id);
			}

		public:

			void init()
			{
				parent_child_table::init();

				deleted = false;
				level = MaxLevel;
				count = 0;

				auto hdr = create(MaxLevel);	
				header_id = hdr.row_id();
			}

			bool pop_front()
			{
				bool result = false;
				node_type q;

				q = header().forward(0);

				if (q != nullptr)
				{
					result = true;
					remove_node(q->key);
				}
				return result;
			}

			class iterator
			{
				index<KEY, VALUE, SORT_ORDER>* base;
				index<KEY, VALUE, SORT_ORDER>::node_type current;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = std::pair<KEY, VALUE>;
				using pointer = std::pair<KEY, VALUE>*;  // or also value_type*
				using reference = std::pair<KEY, VALUE>&;  // or also value_type&

				iterator(index<KEY, VALUE, SORT_ORDER>* _base, index<KEY, VALUE, SORT_ORDER>::node_type& _current) :
					base(_base),
					current(_current)
				{

				}

				iterator() : base(nullptr), current(nullptr)
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline std::pair<KEY, VALUE>& operator *()
				{
					return current.parent().keyValue;
				}

				inline std::pair<KEY, VALUE>* operator->() const
				{
					return &current.parent().keyValue;
				}

				inline KEY& get_key() const
				{
					return current.parent().keyValue.first;
				}

				inline VALUE& get_value()
				{
					return current.parent().keyValue.second;
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end() const
				{
					index<KEY, VALUE, SORT_ORDER>::node_type empty;
					return iterator(base, empty);
				}

				inline iterator operator++()
				{
					current = base->next_node(current);
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current.row_id() == current.row_id();
				}

				bool operator != (const iterator& _src) const
				{
					return _src.current.row_id() != current.row_id();
				}

			};

			index<KEY, VALUE, SORT_ORDER>::iterator begin()
			{
				return iterator(this, first_node());
			}

			index<KEY, VALUE, SORT_ORDER>::iterator end()
			{
				return iterator(this, nullptr);
			}

			bool erase(index<KEY, VALUE, SORT_ORDER>::iterator& _iter)
			{
				return this->remove_node(_iter->first);
			}

			bool erase(const KEY& key)
			{
				return this->remove_node(key);
			}

			index<KEY, VALUE, SORT_ORDER>::iterator at(const KEY& key)
			{
				return iterator(this, this->find_node(key));
			}

			index<KEY, VALUE, SORT_ORDER>::iterator first_like(const KEY& key)
			{
				return iterator(this, this->find_first_node(key));
			}

			index<KEY, VALUE, SORT_ORDER>::iterator operator[](const KEY& key)
			{
				return iterator(this, this->find_node(key));
			}

			bool has(const KEY& key, VALUE& value)
			{
				auto n = this->find_node(key);
				return (n && n->keyValue.second == value);
			}

			bool has(const KEY& key, std::function<bool(VALUE& src)> pred)
			{
				auto n = this->find_node(key);
				return (n && pred(n->keyValue.second));
			}

			VALUE& first_value()
			{
				auto n = first_node();
				return n->keyValue.second;
			}

			index<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(std::pair<KEY, VALUE>& kvp)
			{
				auto modified_node = this->update_node(kvp);
				return iterator(this, modified_node);
			}

			inline index<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
			}

			index<KEY, VALUE, SORT_ORDER>::iterator put(std::pair<KEY, VALUE>& kvp)
			{
				auto modified_node = this->update_node(kvp);
				return iterator(this, modified_node);
			}

			inline index<KEY, VALUE, SORT_ORDER>::iterator put(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
			}

			static bool test()
			{
				index test;
				bool result = true;

				auto t1 = test.insert_or_assign(5, "hello");
				if (t1.get_key() != 5 || t1.get_value() != "hello" || t1->second != "hello")
				{
					std::cout << "fail: wrong inserted value." << std::endl;
					return false;
				}
				auto t2 = test.insert_or_assign(7, "goodbye");
				if (t2.get_key() != 7 || t2.get_value() != "goodbye" || t2->second != "goodbye")
				{
					std::cout << "fail: wrong inserted value." << std::endl;
					return false;
				}
				auto t3 = test.insert_or_assign(7, "something");
				if (t3.get_key() != 7 || t3->second != "something")
				{
					std::cout << "fail: wrong updated value." << std::endl;
					return false;
				}
				auto t4 = test[7];
				if (t4.get_key() != 7 || t4->second != "something")
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
				if (t6.get_key() != 2 || t6.get_value() != "hello super")
				{
					std::cout << "fail: wrong inserted value." << std::endl;
					return false;
				}

				auto t7 = test.put(1, "first");
				if (t7.get_key() != 1 || t7->second != "first")
				{
					std::cout << "fail: wrong inserted value." << std::endl;
					return false;
				}

				t7 = test.put(1, "second");
				if (t7.get_key() != 1 || t7->second != "second")
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
					std::cout << item.first << " " << item.second << std::endl;
				}

				std::cout << "starting later loop" << std::endl;

				for (auto& item : test[2])
				{
					std::cout << item.first << " " << item.second << std::endl;
				}

				auto x = test[1];
				test.erase(x);
				test.erase(7);

				std::cout << "erased loop" << std::endl;

				for (auto& item : test)
				{
					std::cout << item.first << " " << item.second << std::endl;
				}

				return result;
			}

		private:


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


			bool contains(index<KEY, VALUE>& _src)
			{
				bool all = true;
				for (auto& f : _src)
				{
					if (!find_node(f.first))
					{
						all = false;
						break;
					}
				}
				return all;
			}

		private:

			// compare a node to a key for equality

			inline int compare(node_type& _node, const KEY& key)
			{
				if (!_node.is_null())
				{
					if (_node.key() < key)
						return -SORT_ORDER;
					else if (_node.key() > key)
						return SORT_ORDER;
					else
						return 0;
				}
				else
				{
					return 1;
				}
			}

			node_type find_node(node_type* update, const KEY& key)
			{
				node_type p, q, found;

				for (int k = level; k >= 0; k--)
				{
					p = header;
					q = p.forward(k);
					auto comp = compare(q, key);
					while (comp < 0)
					{
						p = q;
						q = q.forward(k);
						comp = compare(q, key);
					}
					if (comp == 0)
						found = q;
					update[k] = p;
				}

				return found;
			}

			node_type find_first(node_type* update, const KEY& key)
			{
				node_type p, q, found, last;

				for (int k = level; k >= 0; k--)
				{
					p = header;
					q = p.forward(k);
					last = q;
					auto comp = compare(q, key);
					while (comp < 0)
					{
						p = q;
						last = q;
						q = q.forward(k);
						comp = compare(q, key);
					}
					if (comp == 0)
						found = q;
					else if (comp < 0)
						found = last;
					update[k] = p;
				}

				return found;
			}

			node_type update_node(const std::pair<KEY, VALUE>& kvp)
			{
				int k;
				node_type null_node;
				node_type update[MaxNumberOfLevels], p;

				node_type q = find_node(update, kvp.first);

				if (q)
				{
					q.value() = kvp.second;
					return q;
				}

				k = randomLevel();
				if (k > level) {
					k = ++level;
					update[k] = header;
				};

				auto new_node_base = create(k);
				q = node_type(this, new_node_base);

				if (!q.is_null()) {

					count++;

					do {
						p = update[k];
						q.forward(k) = p.forward(k);
						p.forward(k) = q;
					} while (--k >= 0);

					return q;
				}

				return null_node;
			}

			bool remove_node(const KEY& key)
			{
				int k, m;
				node_type update[MaxNumberOfLevels];
				node_type p;

				m = level;
				node_type q = find_node(update, key);

				if (!q.is_null()) {
					k = 0;
					p = update[k];
					while (k <= m && p.forward(k) == q)
					{
						p.forward(k) = q.forward(k);
						k++;
						p = update(k);
					}
					count--;
					while (header.forward(m).is_null() && m > 0) {
						m--;
					}
					level = m;
					return true;
				}
				else {
					return false;
				}
			}

			node_type find_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				node_type update[MaxNumberOfLevels];
				return find_node(update, key);
			}

			node_type find_first_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				node_type update[MaxNumberOfLevels];
				return find_first(update, key);
			}

			node_type first_node() const
			{
				return header.forward != nullptr ? header.forward[0] : nullptr;
			}

			node_type next_node(node_type _node) const
			{
				if (_node) {
					_node = _node.forward(0);
					return _node;
				}
				return nullptr;
			}
		};

		template <typename KEY, typename VALUE, int SORT_ORDER> std::ostream& operator <<(std::ostream& output, index<KEY, VALUE, SORT_ORDER>& src)
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
}

