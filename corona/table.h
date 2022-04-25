#pragma once

#include <iostream>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include "store_box.h"

namespace countrybit
{
	namespace database
	{

		struct row_range 
		{
			row_id_type			start;
			row_id_type			stop;
			row_id_type			reserved_stop;

			row_id_type size() { return stop - start;  }
			row_id_type reserved_size() { return reserved_stop - start; }
			row_id_type free() { return reserved_stop - stop; }

			bool success() const 
			{
				return start != null_row && stop != null_row;
			}
		};

		template <typename P, typename C> class item_details_holder
		{
			P* the_object;
			C* the_details;
			row_id_type id;
			row_id_type length;

		public:

			item_details_holder(P* _parent, C* _children, row_id_type _id, row_id_type _length) :
				the_object(_parent),
				the_details(_children),
				id(_id),
				length(_length)
			{
				;
			}

			item_details_holder() :
				the_object(nullptr),
				the_details(nullptr),
				id(null_row),
				length(0)
			{
				;
			}

			item_details_holder(const item_details_holder& _src) :
				the_object(_src.the_object),
				the_details(_src.the_details),
				id(_src.id),
				length(_src.length)
			{
				;
			}

			item_details_holder operator = (const item_details_holder& _src) 
			{
				item_details_holder temp(_src);
				return temp;
			}

			P& item()
			{
				if (is_null())
					throw std::invalid_argument("is null");
				return *the_object;
			}

			C& detail(row_id_type idx)
			{
				if (is_null())
					throw std::invalid_argument("is null");
				if (idx >= length)
					throw std::invalid_argument("idx out of range");
				return the_details[idx];
			}

			P* pitem()
			{
				return the_object;
			}

			C* pdetails()
			{
				return the_details;
			}

			bool is_null() const
			{
				return !the_object;
			}

			row_id_type row_id() const
			{
				return id;
			}

			size_t size() const
			{
				return length;
			}

			bool success() const
			{
				return the_object != nullptr && the_details != nullptr;
			}
		};

		template <typename T> 
		requires (std::is_standard_layout<T>::value)
		class table
		{

			struct table_header
			{
				row_id_type
					max_rows,
					last_row;
				T   rows[1];
			};

			table_header* hdr;

		public:

			table(  ) : 
				hdr(nullptr)
			{
				
			}

			table(const table& src) : hdr(src.hdr)
			{

			}

			static int get_box_size( int _rows )
			{
				return sizeof(table_header) + sizeof(T) * _rows;
			}

			template <typename B>
			requires (box<B, T>&& box<B, table_header>)
			static row_id_type reserve_table(B* _b, int _max_rows)
			{
				table t;

				row_id_type hdr_offset;

				row_id_type bytes_size = get_box_size(_max_rows);

				char c = 0;
				hdr_offset = _b->pack(c, bytes_size);

				t.hdr = _b->unpack<table_header>(hdr_offset);

				t.hdr->max_rows = _max_rows;
				t.hdr->last_row = 0;

				for (int i = 0; i < _max_rows; i++) {
					T item = {};
					t.hdr->rows[i] = item;
				}

				return hdr_offset;
			}

			template <typename B>
			requires (box<B, T>&& box<B, table_header>)
			static table get_table(B* _b, row_id_type offset)
			{
				table t;

				t.hdr = _b->unpack<table_header>(offset);

				return t;
			}

			template <typename B>
			requires (box<B, T>&& box<B, table_header>)
			static table create_table(B* _b, int _max_rows, row_id_type& offset)
			{
				table t;
				offset = reserve_table(_b, _max_rows);
				t.hdr = _b->unpack<table_header>(offset);
				return t;
			}

			row_range create(uint32_t count)
			{
				auto x = hdr->last_row + count;
				if (x > hdr->max_rows) 
				{
					row_range err = { null_row, null_row, null_row };
					return err;
				}

				row_range rr;
				rr.start = hdr->last_row;
				rr.stop = x;
				rr.reserved_stop = x;
				hdr->last_row = x;
				return rr;
			}

			T& create(uint32_t count, row_range &rr)
			{
				auto x = hdr->last_row + count;
				if (x > hdr->max_rows) {
					rr = { null_row, null_row, null_row };
					return hdr->rows[0];
				}

				rr.start = hdr->last_row;
				rr.stop = x;
				rr.reserved_stop = x;
				hdr->last_row = x;
				return hdr->rows[rr.start];
			}

			bool copy_rows(int start, int stop, int shift)
			{
				if (start > stop || start < 0 || stop < 0 || start > max() || stop > max())
				{
					return false;
				}

				int new_stop = stop + shift;

				if (new_stop > max())
					return false;
				if (new_stop < 0)
					return false;

				int i, k;

				if (shift > 0)
				{
					for (i = stop - 1, k = new_stop - 1; i >= start; i--, k--)
					{
						hdr->rows[k] = hdr->rows[i];
					}
				}
				else if (shift < 0)
				{
					for (i = start, k = start + shift; i < stop; i++, k++)
					{
						hdr->rows[k] = hdr->rows[i];
					}
				}

				if (new_stop > hdr->last_row) 
				{
					hdr->last_row = new_stop;
				}

				return true;
			}

			T& insert(row_id_type index, uint32_t count, row_range& rr)
			{
				auto x = hdr->last_row + count;
				if (x > hdr->max_rows) {
					rr = { null_row, null_row, null_row };
					return hdr->rows[0];
				}

				rr.start = index;
				rr.stop = index + count;
				rr.reserved_stop = index + count;

				copy_rows(index + 1, hdr->last_row + 1, count);

				hdr->last_row = x;
				return hdr->rows[rr.start];
			}

			void erase(row_range& rr)
			{
				row_id_type isize = rr.size();
				copy_rows(rr.start, rr.stop, -isize);
				hdr->last_row -= isize;
			}

			bool check(int index)
			{
				if (index == null_row || index >= size() || index < 0)
				{
					return false;
				}
				return true;
			}

			// src here should be const
			T& append(T& src, row_range& rr)
			{
				auto x = hdr->last_row + 1;
				if (x > hdr->max_rows) {
					rr = { null_row, null_row, null_row };
					return hdr->rows[0];
				}

				rr.start = hdr->last_row;
				rr.stop = x;
				rr.reserved_stop = x;
				hdr->last_row = x;
				hdr->rows[rr.start] = src;
				return hdr->rows[rr.start];
			}


			T& get_at(row_id_type& r)
			{
				if (r == null_row || r >= hdr->max_rows || r < 0)
					throw std::invalid_argument("invalid row id");

				if (r > hdr->last_row)
				{
					hdr->last_row = r;
				}

				return hdr->rows[r];
			}

			T& operator[](row_id_type r)
			{
				return get_at(r);
			}

			row_id_type size() const
			{
				return hdr->last_row;
			}

			row_id_type max() const
			{
				return hdr->max_rows;
			}

			void clear()
			{
				hdr->last_row = 0;
			}

			class iterator
			{
				table<T>* base;
				row_id_type current;
				std::function<bool(T&)> predicate;

			public:

				struct value_ref 
				{
					T& item;
					row_id_type location;
				};

				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = value_ref;
				using pointer = value_ref*;  // or also value_type*
				using reference = value_ref&;  // or also value_type&

				iterator(table<T>* _base, row_id_type _current) :
					base(_base),
					current(_current)
				{
					predicate = [](T& a) { return true;  };
				}

				iterator(table<T>* _base, row_id_type _current, std::function<bool(T&)> _predicate) :
					base(_base),
					current(_current),
					predicate(_predicate)
				{
					while (current != null_row && !predicate(base->get_at(current)))
					{
						current++;
						if (current >= base->size()) {
							current = null_row;
							break;
						}
					}
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

				inline row_id_type get_row_id()
				{
					return current;
				}

				inline T& get_value()
				{
					return base->get_at(current);
				}

				inline iterator begin() const
				{
					return iterator(base, current, predicate);
				}

				inline iterator end() const
				{
					return iterator(base, null_row, predicate);
				}

				inline iterator operator++()
				{
					if (current == null_row)
						return end();
					current++;
					while (current < base->size() && !predicate(base->get_at(current)))
					{
						current++;
					}

					if (current >= base->size()) {
						current = null_row;
					}

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

			table<T>::iterator begin()
			{
				return iterator(this, 0);
			}

			table<T>::iterator end()
			{
				return iterator(this, null_row);
			}

			auto where(std::function<bool(T&)> predicate)
			{
				return iterator(this, 0, predicate);
			}

			T& first(std::function<bool(T&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					throw std::logic_error("sequence has no elements");
				}
				return w->get_value();
			}

			row_id_type first_index(std::function<bool(T&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					return null_row;
				}
				return w.get_row_id();
			}

			bool any_of(std::function<bool(T&)> predicate)
			{
				return std::any_of(hdr->rows, hdr->rows + size(), predicate);
			}

			bool all_of(std::function<bool(T&)> predicate)
			{
				return std::all_of(hdr->rows, hdr->rows + size(), predicate);
			}

			int count_if(std::function<bool(T&)> predicate)
			{
				return std::count_if(hdr->rows, hdr->rows + size(), predicate);
			}

			void sort(std::function<bool(T& a, T& b)> fn)
			{
				std::sort(hdr->rows, hdr->rows + size(), fn);
			}

		};


		template <typename P, typename C>
		requires (std::is_standard_layout<P>::value && std::is_standard_layout<C>::value)
		class item_details_table
		{

			struct item_type
			{
				P item;
				row_range detail_range;
			};

			struct item_detail_table_header
			{
				row_id_type item_location;
				row_id_type detail_location;
			};

			table<item_type> item;
			table<C> details;

			bool move_details(row_id_type location, int shift)
			{
				auto& pcr = item[location];

				row_range new_pos{ pcr.detail_range.start + shift, pcr.detail_range.stop + shift, pcr.detail_range.reserved_stop + shift };

				if (new_pos.reserved_stop >= details.size())
				{
					return false;
				}

				bool success = details.copy_rows(pcr.detail_range.start, pcr.detail_range.stop, shift);
				if (success)
				{
					pcr.detail_range = new_pos;
				}
				return success;
			}

		public:

			item_details_table()
			{
				;
			}

			template <typename B>
				requires (box<B, P>&& box<B, item_details_table<P, C>::item_type>)
			static row_id_type reserve_table(B* b, int item_rows, int detail_rows)
			{
				item_detail_table_header hdr;
				hdr.item_location = null_row;
				hdr.detail_location = null_row;
				row_id_type r = b->pack(hdr);
				auto* phdr = b->unpack<item_detail_table_header>(r);
				phdr->item_location = table< item_details_table<P, C>::item_type >::reserve_table(b, item_rows);
				phdr->detail_location = table< C >::reserve_table(b, detail_rows);
				return r;
			}

			template <typename B>
				requires (box<B, P>&& box<B, item_details_table<P, C>::item_type>)
			static item_details_table get_table(B* b, row_id_type row)
			{
				item_details_table pct;
				item_detail_table_header* hdr;
				hdr = b->unpack<item_details_table<P, C>::item_detail_table_header>(row);
				pct.item = table< item_details_table<P, C>::item_type >::get_table(b, hdr->item_location);
				pct.details = table< C >::get_table(b, hdr->detail_location);
				return pct;
			}

			template <typename B>
				requires (box<B, P>&& box<B, item_details_table<P, C>::item_type>)
			static item_details_table create_table(B* b, int item_rows, int detail_rows, row_id_type& row)
			{
				item_details_table pct;
				row = reserve_table(b, item_rows, detail_rows);
				pct = get_table(b, row);
				return pct;
			}

			item_details_holder<P, C> create_item(int detail_count, C* new_details)
			{
				auto pcr = item.create(1);
				if (pcr.success())
				{
					auto& pc = item[pcr.start];
					pc.detail_range = details.create(detail_count);
					if (new_details) {
						append_detail(pcr.start, detail_count, new_details);
					}
					return item_details_holder<P, C>(&pc.item, &details[pc.detail_range.start], pcr.start, detail_count);
				}
				else
				{
					return item_details_holder<P, C>(nullptr, nullptr, null_row, null_row);
				}
			}

			item_details_holder<P, C> put_item(row_id_type location, int detail_count, C *detail_range)
			{
				if (location == null_row) 
				{
					auto pcr = create_item(detail_count, detail_range);
					location = pcr.row_id();
				}

				clear_details(location);
				if (detail_range) {
					append_detail(location, detail_count, detail_range);
				}
				return get_item(location);
			}

			item_details_holder<P, C> clone_item(row_id_type location)
			{
				auto& src = item.get_at(location);
				auto dest = create(src.detail_range.reserved_size());
				int shift = dest.item().detail_range.start - src.detail_range.start;
				details.copy_rows(src.detail_range.start, src.detail_range.reserved_stop, shift );
			}

			void clear_details(row_id_type location)
			{
				auto& pc = item.get_at(location);
				pc.detail_range.stop = pc.detail_range.start;
			}

			void append_detail(row_id_type location, int add_detail_count, C *new_details)
			{
				if (location == null_row)
				{
					auto parent_child = create_item(add_detail_count, new_details);
					return;
				}

				auto& pc = item.get_at(location);

				if (pc.detail_range.start == 0 && pc.detail_range.stop == 0 && pc.detail_range.reserved_stop == 0)
				{
					throw std::invalid_argument("can't extend an uncreated");
				}

				row_id_type new_base = pc.detail_range.stop;
				row_id_type new_start = pc.detail_range.stop;

				row_id_type capacity_in_node = pc.detail_range.free();
				if (capacity_in_node >= add_detail_count)
				{
					pc.detail_range.stop = pc.detail_range.stop + add_detail_count;
				}
				else
				{
					int capacity_ask = capacity_in_node + add_detail_count;
					int capacity_allocate = pc.detail_range.reserved_size() * 2;
					while (capacity_allocate < capacity_ask)
						capacity_allocate *= 2;

					row_id_type new_stop = pc.detail_range.start + capacity_allocate;
					row_id_type shift = new_stop - pc.detail_range.reserved_stop;

					if (new_stop < details.max()) 
					{
						for (row_id_type i = size()-1; i > location; i--) 
						{
							move_details(i, shift);
						}

						pc.detail_range.reserved_stop = new_stop;
						pc.detail_range.stop += add_detail_count;
					}
					else 
					{
						throw std::invalid_argument("can't extend an uncreated");
					}
				}

				for (row_id_type i = 0; i < add_detail_count; i++) {
					auto& new_detail = details[new_start + i];
					new_detail = new_details[i];
				}
			}

			void erase_detail(row_id_type location, int detail_index)
			{
				auto& pc = item.get_at(location);
				details.copy_rows(pc.detail_range.start + detail_index, pc.detail_range.stop, -1);
				pc.detail_range.stop--;
			}

			void erase_item(row_id_type location)
			{
				auto& pc = item.get_at(location);
				for (row_id_type i = location; i > location; i--)
				{
					move_details(i, -1);
				}
				item.erase({ location, location });
			}

			bool check(int index)
			{
				return item.check(index);
			}

			item_details_holder<P, C> operator[](row_id_type row_id)
			{
				item_details_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = item[ row_id ];
				return item_details_holder<P, C>(&pc.item, &details[pc.detail_range.start],  row_id, pc.detail_range.size());
			}

			item_details_holder<P, C> get_item(row_id_type row_id)
			{
				item_details_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = item[row_id];
				return item_details_holder<P, C>(&pc.item, &details[pc.detail_range.start], row_id, pc.detail_range.size());
			}

			static int get_box_size(int _parent_rows, int _child_rows)
			{
				return table<item_type>::get_box_size(_parent_rows) + table<C>::get_box_size(_child_rows);
			}

			row_id_type size() const
			{
				return item.size();
			}

			row_id_type max() const
			{
				return item.max();
			}

			void clear()
			{
				item.clear();
				details.clear();
			}

		};

		bool table_tests();

	}
}
