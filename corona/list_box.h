#pragma once

#include "corona.h"

namespace corona
{
	namespace database
	{
		template <typename item_type>
		class list_box
		{

			struct list_box_data
			{
			public:
				block_id			id;
				uint32_t			length;
				relative_ptr_type	root_item;
				relative_ptr_type	last_item;

				list_box_data();
			};

			struct list_link
			{
			public:
				item_type   data;
				relative_ptr_type next_link;
				relative_ptr_type previous_link;
			};

			relative_ptr_type		header_loc;
			serialized_box_container* box;

			list_box_data* get_hdr()
			{
				return box->unpack<list_box_data>(header_loc);
			}

			list_link* first_link()
			{
				list_box_data* hdr = get_hdr();
				return box->unpack<list_link>(hdr->root_item);
			}

			list_link* last_link()
			{
				list_box_data* hdr = get_hdr();
				return box->unpack<list_link>(hdr->last_item);
			}

			list_link* next_link(list_link* n)
			{
				return box->unpack<list_link>(n->next_link);
			}

			list_link* previous_link(list_link* n)
			{
				return box->unpack<list_link>(n->previous_link);
			}

			list_link* put(const item_type& _src)
			{
				list_box_data* hdr = get_hdr();

				list_link temp, *n;
				temp.data = _src;
				temp.next_link = null_row;
				temp.previous_link = null_row;

				relative_ptr_type loc = box->pack(temp);

				if (loc == null_row) {
					return nullptr;
				}

				if (hdr->last_item != null_row) 
				{
					list_link* last_link = box->unpack<list_link>(hdr->last_item);
					n = box->unpack<list_link>(loc);
					n->previous_link = hdr->last_item;
					last_link->next_link = loc;
					hdr->last_item = loc;
				}
				else
				{
					hdr->root_item = loc;
					hdr->last_item = loc;
				}
				
				hdr->length++;
				list_link *l = box->unpack<list_link>(loc);
				return l;
			}

			void copy(int ls, const item_type* s)
			{
				list_box_data* hdr = get_hdr();
				int l = 0;

				while (l < ls)
				{
					put(*s);
					l++;
					s++;
				}

				hdr->length += l;
			}

		public:

			list_box() : box(nullptr), header_loc(null_row)
			{
			}

			list_box(const list_box& _src) : box(_src.box), header_loc(_src.header_loc)
			{

			}

			list_box operator = ( const list_box& _src )
			{
				box = _src.box;
				header_loc = _src.header_loc;
				return *this;
			}

			relative_pointer_type get_location()
			{
				return header_loc;
			}

			static relative_ptr_type reserve(serialized_box_container* b)
			{
				list_box temp;
				list_box_data hdr;
				hdr.length = 0;
				hdr.root_item = null_row;
				hdr.last_item = null_row;
				auto location = b->pack<list_box_data>(hdr);
				return location;
			}

			static list_box get(serialized_box_container* b, relative_ptr_type location)
			{
				list_box temp;
				temp.box = b;
				temp.header_loc = location;
				return temp;
			}

			static list_box create(serialized_box_container* b)
			{
				relative_ptr_type loc = reserve(b);
				list_box temp = get(b, loc);
				return temp;
			}

			list_box& operator += (std::vector<item_type>& _src)
			{
				copy(_src.size(), _src.data());
				return *this;
			}

			list_box& operator += (const array_box<item_type>& _src)
			{
				copy(_src.size(), _src.data());
				return *this;
			}

			uint16_t size()
			{
				list_box_data* hdr = get_hdr();
				return hdr->length;
			}

			void push_back(const item_type& t)
			{
				put(t);
			}

			class iterator
			{
				list_box<item_type>* base;
				list_box<item_type>::list_link *current;
				std::function<bool(item_type&)> predicate;

			public:

				using iterator_category = std::bidirectional_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = item_type;
				using pointer = item_type*;  // or also value_type*
				using reference = item_type&;  // or also value_type&

				iterator(list_box<item_type>* _base,
					list_box<item_type>::list_link* _current,
					std::function<bool(item_type&)> _predicate) :
					base(_base),
					current(_current),
					predicate(_predicate)
				{
					while (current && !predicate(current->data))
					{
						current = _base->next_link(current);
					}
				}

				iterator(list_box<item_type>* _base,
					list_box<item_type>::list_link* _current) :
					base(_base),
					current(_current)
				{
					predicate = [](item_type& a) { return true;  };
				}

				iterator() :
					base(nullptr),
					current(nullptr)
				{
					predicate = [](item_type& a) { return true;  };
				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline item_type& operator *()
				{
					return current->data;
				}

				inline item_type* operator ->()
				{
					return &current->data;
				}

				inline item_type& get_value()
				{
					return current->data;
				}

				inline iterator begin() const
				{
					return iterator(base, base->first_link(), predicate);
				}

				inline iterator end() const
				{
					return iterator(base, nullptr, predicate);
				}

				inline iterator operator++()
				{
					do
					{
						current = base->next_link(current);
						if (!current)
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

				inline iterator operator--()
				{
					do
					{
						current = base->previous_link(current);
						if (!current)
							return iterator(base, nullptr, predicate);
					} while (!predicate(current->data));
					return iterator(base, current, predicate);
				}

				inline iterator operator--(int)
				{
					iterator tmp(*this);
					operator--();
					return tmp;
				}

				inline iterator operator[](int idx)
				{
					iterator temp(*this);
					if (idx > 0) while (idx--) temp++;
					else if (idx < 0) while (idx++) temp--;
					return temp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

				item_type& first(std::function<bool(item_type&)> _predicate)
				{
					auto new_predicate = [this, _predicate](item_type& kp) { return this->predicate(kp) && _predicate(kp); };
					auto it = iterator(base, first_link(), new_predicate);
					return *it;
				}

				auto where(std::function<bool(item_type&)> _predicate)
				{
					auto new_predicate = [this, _predicate](item_type& kp) { return this->predicate(kp) && _predicate(kp); };
					return iterator(base, first_link(), new_predicate);
				}

				bool any_of(std::function<bool(item_type&)> _predicate)
				{
					auto new_predicate = [this, _predicate](item_type& kp) { return this->predicate(kp) && _predicate(kp); };
					return std::any_of(begin(), end(), new_predicate);
				}

				bool all_of(std::function<bool(item_type&)> _predicate)
				{
					auto new_predicate = [this, _predicate](item_type& kp) { return this->predicate(kp) && _predicate(kp); };
					return std::all_of(begin(), end(), new_predicate);
				}

				corona_size_t count_if(std::function<bool(item_type&)> _predicate)
				{
					auto new_predicate = [this, _predicate](item_type& kp) { return this->predicate(kp) && _predicate(kp); };
					return std::count_if(begin(), end(), new_predicate);
				}

			};

			list_box<item_type>::iterator begin()
			{
				return iterator(this, first_link());
			}

			list_box<item_type>::iterator end()
			{
				return iterator(this, nullptr);
			}

			list_box<item_type>::iterator rbegin()
			{
				return iterator(this, last_link());
			}

			item_type &first()
			{
				auto t = first_link();
				if (!t)
					throw std::invalid_argument("list is empty");
				return t->data;
			}

			item_type& last()
			{
				auto t = last_link();
				if (!t)
					throw std::invalid_argument("list is empty");
				return t->data;
			}

			item_type& operator[](int idx)
			{
				if (idx < 0) 
				{
					auto itr = rbegin();
					idx++;
					while (idx) {
						itr--;
						idx++;
					}
					if (itr == std::end(*this))
						throw std::invalid_argument("index out of range for list");
					return *itr;
				}
				else
				{
					auto itr = begin();
					while (idx) {
						itr++;
						idx--;
					}
					if (itr == std::end(*this))
						throw std::invalid_argument("index out of range for list");
					return *itr;
				}
			}

			item_type& first(std::function<bool(item_type&)> predicate)
			{
				auto it = iterator(this, first_link(), predicate);
				return *it;
			}

			auto where(std::function<bool(item_type&)> predicate)
			{
				return iterator(this, first_link(), predicate);
			}

			bool any_of(std::function<bool(item_type&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(item_type&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(item_type&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}
		};

		bool list_box_tests();
	}
}

