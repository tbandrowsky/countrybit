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
		const int AvgNumberOfLevels = 10;
		const int MaxLevel = MaxNumberOfLevels - 1;

		template <typename KEY, typename VALUE> class index_node_hdr 
		{
		public:

			std::pair<KEY, VALUE>	key_value;
			row_id_type				node_index;
		};

		using index_ref = row_id_type;

		template <typename KEY, typename VALUE, int MAX_ROWS, int SORT_ORDER = 1> class index 
			: public parent_child_table<index_node_hdr<KEY,VALUE>, index_ref, MAX_ROWS, AvgNumberOfLevels>
		{
		public:

			using index_node = parent_child_holder<index_node_hdr<KEY, VALUE>, index_ref>;

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
					return me == nullptr || nd.row_id() == 0;
				}

				std::pair<KEY, VALUE>& keyValue()
				{
					return nd.parent().key_value;
				}

				KEY& key()
				{
					return nd.parent().key_value.first;
				}

				VALUE& value()
				{
					return nd.parent().key_value.second;
				}

				node_type forward(int idx)
				{
					return node_type( me, nd.child(idx) );
				}

				row_id_type row_id() const
				{
					return nd.row_id();
				}

				bool operator == (node_type& nt)
				{
					return nd.row_id() == nt.nd.row_id();
				}
			};

		private:

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

				auto hdr = parent_child_table::create(MaxLevel);
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
				index<KEY, VALUE, MAX_ROWS, SORT_ORDER>* base;
				index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::node_type current;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = std::pair<KEY, VALUE>;
				using pointer = std::pair<KEY, VALUE>*;  // or also value_type*
				using reference = std::pair<KEY, VALUE>&;  // or also value_type&

				iterator(index<KEY, VALUE, MAX_ROWS, SORT_ORDER>* _base, const index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::node_type& _current) :
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
					return current.keyValue();
				}

				inline std::pair<KEY, VALUE>* operator->()
				{
					return &current.keyValue();
				}

				inline KEY& get_key()
				{
					return current.keyValue().first;
				}

				inline VALUE& get_value()
				{
					return current.keyValue().second;
				}

				inline iterator begin() const
				{
					index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::node_type omg;
					omg = current;
					return iterator(base, omg);
				}

				inline iterator end() const
				{
					index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::node_type empty;
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

				bool operator == (const iterator& _src)
				{
					return _src.current.row_id() == current.row_id();
				}

				bool operator != (const iterator& _src)
				{
					return _src.current.row_id() != current.row_id();
				}

			};

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator begin()
			{
				return iterator(this, first_node());
			}

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator end()
			{
				index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::node_type empty;
				return iterator(this, empty);
			}

			bool erase(index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator& _iter)
			{
				return this->remove_node(_iter->first);
			}

			bool erase(const KEY& key)
			{
				return this->remove_node(key);
			}

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator at(const KEY& key)
			{
				return iterator(this, this->find_node(key));
			}

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator first_like(const KEY& key)
			{
				return iterator(this, this->find_first_node(key));
			}

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator operator[](const KEY& key)
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

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator insert_or_assign(std::pair<KEY, VALUE>& kvp)
			{
				auto modified_node = this->update_node(kvp);
				return iterator(this, modified_node);
			}

			inline index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator insert_or_assign(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
			}

			index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator put(std::pair<KEY, VALUE>& kvp)
			{
				auto modified_node = this->update_node(kvp);
				return iterator(this, modified_node);
			}

			inline index<KEY, VALUE, MAX_ROWS, SORT_ORDER>::iterator put(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
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

			template <int MAX_ROWS, int SORT_ORDER> bool contains(index<KEY, VALUE, MAX_ROWS, SORT_ORDER>& _src)
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
					p = header();
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

				if (!q.is_null())
				{
					q.value() = kvp.second;
					return q;
				}

				k = randomLevel();
				if (k > level) {
					k = ++level;
					update[k] = header();
				};

				auto new_node_base = parent_child_table<index_node_hdr<KEY, VALUE>, index_ref, MAX_ROWS, AvgNumberOfLevels>::create(k);
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
						p = update[k];
					}
					count--;
					while (header().forward(m).is_null() && m > 0) {
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

			node_type first_node()
			{
				return header().forward(0);
			}

			node_type next_node(node_type _node)
			{
				node_type empty;
				if (!_node.is_null()) {
					_node = _node.forward(0);
					return _node;
				}
				return empty;
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

		bool test_index();
	}
}

