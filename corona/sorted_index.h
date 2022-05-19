#pragma once

/**********************************************************
Skip List Based on Implementation of design by Bill Pugh.
***********************************************************/

namespace corona
{
	namespace database
	{
		const int MaxNumberOfLevels = 20;
		const int MaxLevel = MaxNumberOfLevels - 1;

		using index_ref = relative_ptr_type;

		template <typename KEY, typename VALUE> 
		class key_value_pair
		{
		public:
			KEY first;
			VALUE second;

			KEY& get_key() 
			{
				return first;
			}
			VALUE& get_value() 
			{
				return second;
			}
		};

		template <typename KEY, typename VALUE, int SORT_ORDER = 1> class sorted_index
		{
		private:

			struct index_header_type
			{
				block_id block;
				int count;
				int level;
				relative_ptr_type header_id;
			};

			class index_node_holder
			{
			public:
				block_id							block;
				relative_ptr_type					header_id;
				relative_ptr_type					details_id;
			};

		public:

			using forward_pointer_collection = array_box<relative_ptr_type>;
			using data_pair = key_value_pair<KEY, VALUE>;
			using collection_type = sorted_index<KEY, VALUE, SORT_ORDER>;
			using iterator_item_type = data_pair;
			using iterator_type = filterable_iterator<data_pair, collection_type, iterator_item_type, index_mapper>;

			class index_node
			{
			public:
				relative_ptr_type					id;
				data_pair							*data;
				forward_pointer_collection			details;

				relative_ptr_type					row_id()
				{
					return id;
				}

				inline relative_ptr_type& detail(int idx)
				{
					return details.get_at(idx);
				}

				inline data_pair&			item()
				{
					return *data;
				}
			};

		private:

			relative_ptr_type header_location;
			serialized_box_container* box;
			index_mapper mapper;
			bool mapper_dirty;

			index_header_type* get_index_header()
			{
				index_header_type* t;
				t = box->unpack<index_header_type>(header_location);
				if (!t->block.is_sorted_index()) {
					throw std::logic_error("did not read sorted index correctly");
				}
				return t;
			};

			index_node get_header()
			{
				auto hdr = get_index_header();
				index_node in = get_node(hdr->header_id);
				return in;
			}

			static index_node create_node(serialized_box_container* box, int _max_level)
			{
				index_node_holder holder, * hd;
				data_pair dp;
				relative_ptr_type r = box->pack<index_node_holder>(holder);

				int level_bounds = _max_level + 1;

				if (r == null_row)
				{
					throw std::logic_error("sorted index exhausted.");
				}

				hd = box->unpack<index_node_holder>(r);
				hd->block = block_id::sorted_index_node_id();
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
					relative_ptr_type rit = null_row;
					node.details.push_back(rit);
				}
			
				return node;
			}

			index_node create_node(int _max_level)
			{
				mapper_dirty = true;
				return create_node(box, _max_level);
			}

			static index_node get_node(serialized_box_container* box, relative_ptr_type _node_id)
			{
				index_node_holder *hd;
				hd = box->unpack<index_node_holder>(_node_id);
				if (!hd->block.is_sorted_index_node()) {
					throw std::logic_error("did not read sorted index node correctly");
				}
				index_node		  node;
				node.data = box->unpack<data_pair>(hd->header_id);
				node.details = forward_pointer_collection::get(box, hd->details_id);
				return node;
			}

			index_node get_node(relative_ptr_type _node_id)
			{
				return get_node(box, _node_id);
			}

			void mapper_check()
			{
				if (!mapper_dirty)
					return;

				mapper.clear();

				auto nd = first_node();
				while (nd != null_row) {
					mapper.add(nd);
					nd = next_node(nd);
				}

				mapper_dirty = false;
			}


		public:

			sorted_index() : header_location(null_row), box(nullptr), mapper_dirty(true)
			{
			}

			sorted_index(serialized_box_container* _box, relative_ptr_type _location) : box(_box), header_location(_location), mapper_dirty(true)
			{
			}

			sorted_index(const sorted_index& _src) : box(_src.box), header_location(_src.header_location), mapper_dirty(true)
			{
				
			}

			sorted_index operator = (const sorted_index& _src) 
			{
				box = _src.box;
				header_location = _src.header_location;
				mapper_dirty = true;
				return *this;
			}

			static relative_ptr_type reserve_sorted_index(serialized_box_container *_b)
			{
				index_header_type hdr, *phdr;

				hdr.count = 0;
				hdr.level = 0;
				hdr.header_id = null_row;
				hdr.block = block_id::sorted_index_id();

				relative_ptr_type header_location = _b->pack(hdr);			
				phdr = _b->unpack<index_header_type>(header_location);

				index_node new_node = create_node(_b, MaxNumberOfLevels);
				phdr->header_id = new_node.row_id();

				return header_location;
			}

			static sorted_index get_sorted_index(serialized_box_container* _b, relative_ptr_type _header_location)
			{
				sorted_index si(_b, _header_location);
				return si;
			}

			static sorted_index create_sorted_index(serialized_box_container* _b, relative_ptr_type& _header_location)
			{
				_header_location = reserve_sorted_index(_b);
				sorted_index new_index = get_sorted_index(_b, _header_location);
				return new_index;
			}

			static int64_t get_box_size()
			{
				return sizeof(index_header_type) + sizeof(relative_ptr_type) * MaxNumberOfLevels + sizeof(data_pair);
			}

			bool pop_front()
			{
				bool result = false;
				
				index_node q = get_header();
				relative_ptr_type qr;

				qr = q.detail(0);

				if (qr != null_row)
				{
					q = get_node(qr);
					result = true;
					remove_node(q.item().key);
				}
				return result;
			}

			iterator_type begin()
			{
				mapper_check();
				return iterator_type(this, 0, &mapper);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row, &mapper);
			}

			data_pair& get_at(relative_ptr_type offset)
			{
				return get_node(offset).item();
			}

			auto where(std::function<bool(const data_pair&)> _predicate)
			{
				mapper_check();
				return iterator_type(this, _predicate, &mapper);
			}

			bool any_of(std::function<bool(const data_pair&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const data_pair&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(const data_pair&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

			bool erase(iterator_type& _iter)
			{
				return this->remove_node(_iter.get_object().get_key());
			}

			bool erase(const KEY& key)
			{
				return this->remove_node(key);
			}

			data_pair& operator[](const KEY& key)
			{
				relative_ptr_type n = find_node(key);
				if (n == null_row) {
					throw std::invalid_argument("bad key");
				}
				return get_node( n ).item();
			}

			bool contains(const KEY& key)
			{
				return this->find_node(key) != null_row;
			}

			data_pair& get(const KEY& key)
			{
				mapper_check();
				auto n = this->find_node(key);
				return get_node(n).item();
			}

			bool has(const KEY& key, VALUE& value)
			{
				auto n = this->find_node(key);
				return (n != null_row && get_node(n).item().second == value);
			}

			bool has(const KEY& key, std::function<bool(VALUE& src)> pred)
			{
				auto n = this->find_node(key);
				return (n != null_row && pred(get_node(n).item().second));
			}

			VALUE& first_value()
			{
				auto n = first_node();
				return get_node(n).item().second;
			}

			void insert_or_assign(data_pair& kvp)
			{
				relative_ptr_type modified_node = this->update_node(kvp, [kvp](VALUE& dest) { dest = kvp.second; });
			}

			void insert_or_assign(KEY key, VALUE value)
			{
				data_pair kvp(key, value);
				insert_or_assign(kvp);
			}

			void put(data_pair& kvp)
			{
				relative_ptr_type modified_node = this->update_node(kvp);
			}

			void put(KEY key, VALUE value)
			{
				data_pair kvp(key, value);
				insert_or_assign(kvp);
			}

			void put(const KEY& key, VALUE& _default_value, std::function<void(VALUE& existing_value)> predicate)
			{
				data_pair kvp(key, _default_value);
				relative_ptr_type modified_node = this->update_node(kvp, predicate);
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

			inline int compare(relative_ptr_type _node, const KEY& key)
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

			relative_ptr_type find_node(relative_ptr_type* update, const KEY& key)
			{
				relative_ptr_type found = null_row, p, q;
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

			relative_ptr_type find_first(relative_ptr_type* update, const KEY& key)
			{
				relative_ptr_type found = null_row, p, q, last_link;

				for (int k = get_index_header()->level; k >= 0; k--)
				{
					p = get_index_header()->header_id;
					q = get_node(p).detail(k);
					last_link = q;
					auto comp = compare(q, key);
					while (comp < 0)
					{
						p = q;
						last_link = q;
						q = get_node(q).detail(k);
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

			relative_ptr_type update_node(data_pair& kvp, std::function<void(VALUE& existing_value)> predicate)
			{
				int k;
				relative_ptr_type update[MaxNumberOfLevels];
				relative_ptr_type q = find_node(update, kvp.first);
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
				relative_ptr_type update[MaxNumberOfLevels], p;
				index_node qnd, pnd;

				relative_ptr_type q = find_node(update, key);

				mapper_dirty = true;

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

			relative_ptr_type find_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				relative_ptr_type update[MaxNumberOfLevels];
				return find_node(update, key);
			}

			relative_ptr_type find_first_node(const KEY& key)
			{
#ifdef	TIME_SKIP_LIST
				benchmark::auto_timer_type methodtimer("skip_list_type::search");
#endif
				relative_ptr_type update[MaxNumberOfLevels];
				return find_first(update, key);
			}

			relative_ptr_type first_node()
			{
				return get_header().detail(0);
			}

			relative_ptr_type next_node(relative_ptr_type _node)
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

