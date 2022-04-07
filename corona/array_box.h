#pragma once

#include "store_box.h"
#include <ostream>
#include <bit>
#include <functional>
#include <algorithm>

namespace countrybit
{
	namespace database
	{
		template <typename item_type, int max_items> 
		struct iarray
		{
			item_type data[max_items];
			uint32_t length;

			iarray() = default;

			iarray(const std::vector<item_type>& src)
			{
				copy(src.data(), src.size());
			}

			iarray& operator = (const std::vector<item_type>& src)
			{
				copy(src.data(), src.size());
				return *this;
			}

			const item_type* value() const
			{
				return &data[0];
			}

			uint16_t size() const
			{
				return length;
			}

			item_type& operator[](int idx)
			{
				return data[idx];
			}

			item_type& get_at(int idx)
			{
				return data[idx];
			}

			item_type* get_ptr(int idx)
			{
				return &data[idx];
			}

			void copy(const item_type* src, int srclength)
			{
				item_type* d = &data[0];
				length = 0;

				while (length < max_items && srclength)
				{
					*d = *src;
					length++;
					src++;
					d++;
					srclength--;
				}
			}

			void push_back(item_type& t)
			{
				if (length < max_items) {
					data[length] = t;
					length++;
				}
			}

			item_type* append()
			{
				item_type* d = nullptr;
				if (length < max_items) {
					d = &data[length];
					length++;
				}
				return d;
			}

			class iterator
			{
				iarray<item_type, max_items>* base;
				row_id_type current;

			public:

				struct value_ref
				{
					item_type& item;
					row_id_type location;
				};

				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = value_ref;
				using pointer = value_ref*;  // or also value_type*
				using reference = value_ref&;  // or also value_type&

				iterator(iarray<item_type, max_items>* _base, row_id_type _current) :
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

				inline value_ref operator *()
				{
					return value_ref{ base->get_at(current), current };
				}

				inline item_type *operator ->()
				{
					return &base->get_at(current);
				}

				inline row_id_type get_index()
				{
					return current;
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
					current++;
					if (current >= base->size())
						return iterator(base, null_row);
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

			iarray<item_type, max_items>::iterator begin()
			{
				return iterator(this, 0);
			}

			iarray<item_type, max_items>::iterator end()
			{
				return iterator(this, null_row);
			}

			bool any_of(std::function<bool(item_type&)> predicate)
			{
				return std::any_of(data, data + length, predicate);
			}

			int count_if(std::function<bool(item_type&)> predicate)
			{
				return std::count_if(data, data + length, predicate);
			}

			void sort(std::function<bool(item_type& a, item_type& b)> fn)
			{
				std::sort(data, data + length, fn);
			}

		};

		template <typename item_type>
		class array_box
		{

			struct array_box_data 
			{
			public:
				uint32_t length;
				uint32_t max_items;
				item_type data[1];
			};

			array_box_data* hdr;

			void copy(int ls, const item_type* s)
			{
				item_type* d = &hdr->data[0];
				int l = 0;

				while (l < hdr->max_items && l < ls)
				{
					*d = *s;
					l++;
					s++;
					d++;
				}

				if (l < hdr->max_items)
				{
					hdr->length = l;
				}
				else
				{
					hdr->length = hdr->max_items - 1;
				}
			}

		public:

			array_box() : hdr(nullptr)
			{
			}

			array_box(char* c) : hdr((array_box_data*)c)
			{
			}

			template <typename BOX>
				requires(box<BOX, char>)
			static row_id_type create(BOX* b, int items_length)
			{
				array_box temp;
				auto location = b->pack<char>(0, sizeof(array_box_data) + items_length * sizeof(item_type));
				temp.hdr = b->unpack<array_box_data>(location);
				temp.hdr->max_items = items_length;
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return location;
			}

			template <typename BOX>
				requires(box<BOX, char>)
			static array_box get(BOX* b, int location)
			{
				array_box temp;
				temp.hdr = b->unpack<array_box_data>(location);
				return temp;
			}

			static array_box create(char* b, int chars_length)
			{
				array_box temp;
				temp.hdr = (array_box_data*)(b);
				temp.hdr->max_items = ( chars_length - sizeof(array_box_data) ) / sizeof(item_type);
				temp.hdr->length = 0;
				return temp;
			}

			static array_box get(char* b)
			{
				array_box temp;
				temp.hdr = (array_box_data*)(b);
				return temp;
			}

			static int get_box_size(int length)
			{
				return sizeof(array_box_data) + sizeof(item_type) * length;
			}

			array_box& operator = (std::vector<item_type>& _src)
			{
				copy(_src.size(), _src.data());
				return *this;
			}

			array_box& operator = (const array_box& _src)
			{
				hdr = _src.hdr;
				return *this;
			}

			array_box& operator = (const char* s)
			{
				copy(s);
				return *this;
			}

			const item_type* data() const
			{
				return &hdr->data[0];
			}

			item_type value(int _idx) const
			{
				return hdr->data[_idx];
			}

			item_type& operator[](int _idx)
			{
				return hdr->data[_idx];
			}

			uint16_t max_size() const
			{
				return hdr->max_items;
			}

			uint16_t size() const
			{
				return hdr->length;
			}

			void push_back(item_type& t)
			{
				if (hdr->length < hdr->max_items) {
					hdr->data[hdr->length] = t;
					hdr->length++;
				}
			}

			bool any_of(std::function<bool(item_type&)> predicate)
			{
				return std::any_of(hdr->data, hdr->data + hdr->length, predicate);
			}

			int count_if(std::function<bool(item_type&)> predicate)
			{
				return std::count_if(hdr->data, hdr->data + hdr->length, predicate);
			}

			void sort(std::function<bool(item_type& a, item_type& b)> fn)
			{
				std::sort(hdr->data, hdr->data + hdr->length, fn);
			}

		};

		bool array_box_tests();
	}
}

