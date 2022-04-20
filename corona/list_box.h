#pragma once

#include "store_box.h"
#include "array_box.h"
#include <ostream>
#include <bit>
#include <functional>
#include <algorithm>

namespace countrybit
{
	namespace database
	{
		template <typename item_type>
		class list_box
		{
			struct list_box_data
			{
			public:
				uint32_t	length;
				row_id_type root_item;
				row_id_type last_item;
			};

			struct list_link
			{
			public:
				item_type   data;
				row_id_type next;
			};

			row_id_type		header_loc;
			serialized_box* box;

			list_box_data* get_hdr()
			{
				return box->unpack<list_box_data>(header_loc);
			}

			list_link* first()
			{
				list_box_data* hdr = get_hdr();
				return box->unpack<list_link>(hdr->root_item);
			}

			list_link* next(list_link* n)
			{
				return box->unpack<list_link>(n->next);
			}

			list_link* put(const item_type& _src)
			{
				list_link temp;
				list_box_data* hdr = get_hdr();

				temp.data = _src;
				temp.next = null_row;

				box->expand_check(sizeof(list_link));

				row_id_type loc = box->pack(temp);

				if (hdr->last_item != null_row) 
				{
					list_link* last = box->unpack<list_link>(hdr->last_item);
					last->next = loc;
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

			static row_id_type create(serialized_box* b)
			{
				list_box temp;
				box->expand_check(sizeof(list_box_data));
				auto location = b->pack<char>(0, sizeof(list_box_data));
				temp.hdr = b->unpack<list_box_data>(location);
				temp.hdr->length = 0;
				temp.header_loc = location;
				temp.hdr->root_item = null_row;
				temp.hdr->last_item = null_row;
				temp.box = b;
				return location;
			}

			static list_box get(serialized_box* b, int location)
			{
				list_box temp;
				temp.box = b;
				temp.hdr = b->unpack<list_box_data>(location);
				temp.header_loc = location;
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

			uint16_t size() const
			{
				list_box_data* hdr = get_hdr();
				return hdr->length;
			}

			void push_back(item_type& t)
			{
				put(t);
			}

			class iterator
			{
				list_box<item_type>* base;
				list_box<item_type>::list_link *current;
				std::function<bool(item_type&)> predicate;

			public:

				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = item_type;
				using pointer = item_type*;  // or also value_type*
				using reference = item_type&;  // or also value_type&

				iterator(array_box<item_type>* _base,
					list_box<item_type>::list_link* _current,
					std::function<bool(item_type&)> _predicate) :
					base(_base),
					current(_current),
					predicate(_predicate)
				{
					if (current != null_row) {
						while (!predicate(base->get_at(current)))
						{
							current++;
							if (current >= base->size()) {
								current = null_row;
							}
						}
					}
				}

				iterator(array_box<item_type>* _base,
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
					return current->item;
				}

				inline item_type* operator ->()
				{
					return current ? current->item : nullptr;
				}

				inline item_type* get_value()
				{
					return current ? &current->item : nullptr;;
				}

				inline iterator begin() const
				{
					return iterator(base, current, predicate);
				}

				inline iterator end() const
				{
					return iterator(base, nullptr, predicate);
				}

				inline iterator operator++()
				{
					do
					{
						current = base->next(current);
						if (!current)
							return iterator(base, nullptr, predicate);
					} while (!predicate(base->get_at(current)));
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

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			array_box<item_type>::iterator begin()
			{
				list_box_data* hdr = get_hdr();
				return iterator(this, hdr->root_item);
			}

			array_box<item_type>::iterator end()
			{
				list_box_data* hdr = get_hdr();
				return iterator(this, null_row);
			}

			auto where(std::function<bool(item_type&)> predicate)
			{
				list_box_data* hdr = get_hdr();
				return iterator(this, hdr->root_item, predicate);
			}
		};

		bool list_box_tests();
	}
}

