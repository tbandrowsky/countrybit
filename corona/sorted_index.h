#pragma once

#include "table.h"
#include "array_box.h"

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
#include "table.h"

namespace countrybit
{
	namespace database
	{
		const int MaxNumberOfLevels = 20;
		const int MaxLevel = MaxNumberOfLevels - 1;

		using index_ref = row_id_type;

		template <typename KEY, typename VALUE, int SORT_ORDER = 1> class sorted_index
		{
		private:

			struct index_header_type
			{
				int count;
				int level;
				row_id_type header_id;
			};

			class index_node_holder
			{
			public:
				row_id_type					header_id;
				row_id_type					details_id;
			};

		public:

			using forward_pointer_collection = array_box<row_id_type>;
			using data_pair = std::pair<KEY, VALUE>;

			class index_node
			{
			public:
				row_id_type					id;
				data_pair					*data;
				forward_pointer_collection	details;

				row_id_type					row_id()
				{
					return id;
				}

				inline row_id_type& detail(int idx)
				{
					return details.get_at(idx);
				}

				inline data_pair&			item()
				{
					return *data;
				}
			};

		private:

			row_id_type header_location;
			serialized_box_container* box;

			index_header_type* get_index_header()
			{
				return box->unpack<index_header_type>(header_location);
			};

			index_node get_header()
			{
				auto hdr = get_index_header();
				index_node in = get_node(hdr->header_id);
				return in;
			}

			index_node create_node(int _max_level)
			{
				index_node_holder holder, * hd;
				data_pair dp;
				row_id_type r = box->pack<index_node_holder>(holder);

				int level_bounds = _max_level + 1;

				if (r == null_row)
				{
					throw std::logic_error("sorted index exhausted.");
				}

				hd = box->unpack<index_node_holder>(r);
				hd->header_id = box->pack<data_pair>(dp);
				hd->details_id = forward_pointer_collection::reserve(box, level_bounds);

				if (hd->header_id == null_row || hd->details_id == null_row)
				{
					throw std::logic_error("sorted index exhausted.");
				}

				index_node		  node;
				node.data = box->unpack<data_pair>(hd->header_id);
				node.details = forward_pointer_collection::get(box, hd->details_id);
				node.id = r;

				for (int i = 0; i < level_bounds; i++)
				{
					row_id_type rit = null_row;
					node.details.push_back(rit);
				}

				return node;
			}

			index_node get_node(row_id_type _node_id)
			{
				index_node_holder *hd;
				hd = box->unpack<index_node_holder>(_node_id);
				index_node		  node;
				node.data = box->unpack<data_pair>(hd->header_id);
				node.details = forward_pointer_collection::get(box, hd->details_id);
				return node;
			}

		public:

			sorted_index() : header_location(null_row), box(nullptr)
			{
			}

			sorted_index(serialized_box_container* _box, row_id_type _location) : box(_box), header_location(_location)
			{
				auto* ihdr = get_index_header();

				if (ihdr->header_id == null_row) {
					index_node hdr = create_node(MaxNumberOfLevels);
					ihdr->header_id = hdr.row_id();
				}
			}

			sorted_index(const sorted_index& _src) : box(_src.box), header_location(_src.header_location)
			{
				
			}

			sorted_index operator = (const sorted_index& _src) 
			{
				box = _src.box;
				header_location = _src.header_location;
				return *this;
			}

			static row_id_type reserve_sorted_index(serialized_box_container *_b)
			{
				index_header_type hdr, *phdr;

				hdr.count = 0;
				hdr.level = 0;
				hdr.header_id = null_row;

				row_id_type header_location = _b->pack(hdr);
				phdr = _b->unpack<index_header_type>(header_location);
				return header_location;
			}

			static sorted_index get_sorted_index(serialized_box_container* _b, row_id_type _header_location)
			{
				sorted_index si(_b, _header_location);
				return si;
			}

			static sorted_index create_sorted_index(serialized_box_container* _b, row_id_type& _header_location)
			{
				_header_location = reserve_sorted_index(_b);
				sorted_index new_index = get_sorted_index(_b, _header_location);
				return new_index;
			}

			static int64_t get_box_size()
			{
				return sizeof(index_header_type) + sizeof(row_id_type) * MaxNumberOfLevels + sizeof(data_pair);
			}

			class iterator
			{
				sorted_index<KEY, VALUE, SORT_ORDER>* base;
				row_id_type current;
				using index_node = item_details_holder<std::pair<KEY, VALUE>, index_ref>;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = std::pair<KEY, VALUE>;
				using pointer = std::pair<KEY, VALUE>*;  // or also value_type*
				using reference = std::pair<KEY, VALUE>&;  // or also value_type&

				iterator(sorted_index<KEY, VALUE, SORT_ORDER>* _base, row_id_type _current) :
					base(_base),
					current(_current)
				{

				}

				iterator() : base(nullptr), current(null_row)
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
					return base->get_node(current).item();
				}

				inline std::pair<KEY, VALUE>* operator->()
				{
					return &base->get_node(current).item();
				}

				inline KEY& get_key()
				{
					std::pair<KEY, VALUE>& p = base->get_node(current).item();
					return p.first;
				}

				inline VALUE& get_value()
				{
					std::pair<KEY, VALUE>& p = base->get_node(current).item();
					return p.second;
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end() const
				{
					return iterator(base, null_row);
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
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			bool pop_front()
			{
				bool result = false;
				
				index_node q = get_header();
				row_id_type qr;

				qr = q.detail(0);

				if (qr != null_row)
				{
					q = get_node(qr);
					result = true;
					remove_node(q.item().key);
				}
				return result;
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator begin()
			{
				index_node q = get_header();
				row_id_type qr = q.detail(0);

				return iterator(this, qr);
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator end()
			{
				return iterator(this, null_row);
			}

			bool erase(sorted_index<KEY, VALUE, SORT_ORDER>::iterator& _iter)
			{
				return this->remove_node(_iter->first);
			}

			bool erase(const KEY& key)
			{
				return this->remove_node(key);
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator at(const KEY& key)
			{
				return iterator(this, this->find_node(key));
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator first_like(const KEY& key)
			{
				return iterator(this, this->find_first_node(key));
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator operator[](const KEY& key)
			{
				return iterator(this, this->find_node(key));
			}

			bool contains(const KEY& key)
			{
				auto iter = iterator(this, this->find_node(key));
				return iter != std::end(*this);
			}

			data_pair& get(const KEY& key)
			{
				auto iter = iterator(this, this->find_node(key));
				return *iter;
			}

			bool has(const KEY& key, VALUE& value)
			{
				auto n = this->find_node(key);
				return (n != null_row && n.item().second == value);
			}

			bool has(const KEY& key, std::function<bool(VALUE& src)> pred)
			{
				auto n = this->find_node(key);
				return (n != null_row && pred(n.item().second));
			}

			VALUE& first_value()
			{
				auto n = first_node();
				return get_node(n).item().second;
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(std::pair<KEY, VALUE>& kvp)
			{
				row_id_type modified_node = this->update_node(kvp, [](VALUE& dest) { });
				return iterator(this, modified_node);
			}

			inline sorted_index<KEY, VALUE, SORT_ORDER>::iterator insert_or_assign(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
			}

			sorted_index<KEY, VALUE, SORT_ORDER>::iterator put(std::pair<KEY, VALUE>& kvp)
			{
				row_id_type modified_node = this->update_node(kvp);
				return iterator(this, modified_node);
			}

			inline sorted_index<KEY, VALUE, SORT_ORDER>::iterator put(KEY key, VALUE value)
			{
				std::pair<KEY, VALUE> kvp(key, value);
				return insert_or_assign(kvp);
			}

			inline sorted_index<KEY, VALUE, SORT_ORDER>::iterator put(KEY& key, VALUE& _default_value, std::function<void(VALUE& existing_value)> predicate)
			{
				std::pair<KEY, VALUE> kvp(key, _default_value);
				row_id_type modified_node = this->update_node(kvp, predicate);
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

			inline int size() { return get_index_header()->count; }

			template <int SORT_ORDER> bool contains(sorted_index<KEY, VALUE, SORT_ORDER>& _src)
			{
				bool all = true;
				for (auto& f : _src)
				{
					if (find_node(f.first) == null_row)
					{
						all = false;
						break;
					}
				}
				return all;
			}

		private:

			// compare a node to a key for equality

			inline int compare(row_id_type _node, const KEY& key)
			{
				if (_node != null_row)
				{
					auto nd = get_node(_node);
					auto ndkey = nd.item().first;

					if (ndkey < key)
						return -SORT_ORDER;
					else if (ndkey > key)
						return SORT_ORDER;
					else
						return 0;
				}
				else
				{
					return 1;
				}
			}

			row_id_type find_node(row_id_type* update, const KEY& key)
			{
				row_id_type found = null_row, p, q;
				auto hdr = get_header();

				for (int k = get_index_header()->level; k >= 0; k--)
				{
					p = get_index_header()->header_id;
					q = hdr.detail(k);
					auto comp = compare(q, key);
					while (comp < 0)
					{
						p = q;
						q = get_node(q).detail(k);
						comp = compare(q, key);
					}
					if (comp == 0)
						found = q;
					update[k] = p;
				}

				return found;
			}

			row_id_type find_first(row_id_type* update, const KEY& key)
			{
				row_id_type found = null_row, p, q, last;

				for (int k = get_index_header()->level; k >= 0; k--)
				{
					p = get_index_header()->header_id;
					q = get_node(p).detail(k);
					last = q;
					auto comp = compare(q, key);
					while (comp < 0)
					{
						p = q;
						last = q;
						q = get_node(q).detail(k);
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

			row_id_type update_node(data_pair& kvp, std::function<void(VALUE& existing_value)> predicate)
			{
				int k;
				row_id_type update[MaxNumberOfLevels];
				row_id_type q = find_node(update, kvp.first);
				index_node qnd;

				if (q != null_row)
				{
					qnd = get_node(q);
					predicate( qnd.item().second );
					return q;
				}

				k = randomLevel();
				if (k > get_index_header()->level)
				{
					k = ++(get_index_header()->level);
					update[k] = get_index_header()->header_id;
				}

				qnd = create_node(k);

				qnd.item() = kvp;
				predicate(qnd.item().second);
				get_index_header()->count++;

				do {
					auto pnd = get_node(update[k]);
					qnd.detail(k) = pnd.detail(k);
					pnd.detail(k) = qnd.row_id();
				} while (--k >= 0);

				return qnd.row_id();
			}

			bool remove_node(const KEY& key)
			{
				int k;
				row_id_type update[MaxNumberOfLevels], p;
				index_node qnd, pnd;

				row_id_type q = find_node(update, key);

				if (q != null_row)
				{
					k = 0;
					p = update[k];
					qnd = get_node(q);
					pnd = get_node(p);
					int m = get_index_header()->level;
					while (k <= m && pnd.detail(k) == q)
					{
						pnd.detail(k) = qnd.detail(k);
						k++;
						if (k <= m) {
							p = update[k];
							pnd = get_node(p);
						}
					}
					get_index_header()->count--;
					while (get_header().detail(m) == null_row && m > 0) {
						m--;
					}
					get_index_header()->level = m;
					return true;
				}
				else 
				{
					return false;
				}
			}

			row_id_type find_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				row_id_type update[MaxNumberOfLevels];
				return find_node(update, key);
			}

			row_id_type find_first_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				row_id_type update[MaxNumberOfLevels];
				return find_first(update, key);
			}

			row_id_type first_node()
			{
				return get_header().detail(0);
			}

			row_id_type next_node(row_id_type _node)
			{
				if (_node == null_row)
					return _node;

				auto nd = get_node(_node);
				_node = nd.detail(0);
				return _node;
			}
		};

		template <typename KEY, typename VALUE, int SORT_ORDER> std::ostream& operator <<(std::ostream& output, sorted_index<KEY, VALUE, SORT_ORDER>& src)
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

