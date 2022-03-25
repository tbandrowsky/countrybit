#pragma once

#include <cstdint>
#include <exception>
#include <stdexcept>
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

		template <typename P, typename C> class parent_child_holder
		{
			P* the_parent;
			C* the_children;
			row_id_type id;
			row_id_type length;
			row_id_type base;

		public:

			parent_child_holder(P* _parent, C* _children, row_id_type _id, row_id_type _length, row_id_type _begin) :
				the_parent(_parent),
				the_children(_children),
				id(_id),
				length(_length),
				base(_begin)
			{
				;
			}

			parent_child_holder() :
				the_parent(nullptr),
				the_children(nullptr),
				id(null_row),
				length(0),
				base(0)
			{
				;
			}

			P& parent()
			{
				if (is_null())
					throw std::invalid_argument("is null");
				return *the_parent;
			}

			C& child(row_id_type idx)
			{
				if (is_null())
					throw std::invalid_argument("is null");
				if (idx >= length)
					throw std::invalid_argument("idx out of range");
				return the_children[idx];
			}

			P* pparent()
			{
				return the_parent;
			}

			C* pchild()
			{
				return the_children;
			}

			bool is_null() const
			{
				return !the_parent;
			}

			row_id_type row_id() const
			{
				return id;
			}

			size_t get_base() const
			{
				return base;
			}

			size_t size() const
			{
				return length;
			}

			bool success() const
			{
				return the_parent != nullptr && the_children != nullptr;
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
					t.hdr->rows[i] = {};
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
				offset = reserve_table(_b, _max_rows);
				table t = _b->unpack(offset);
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

			T& operator[](row_id_type & r)
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

			class iterator
			{
				table<T>* base;
				row_id_type current;

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
					if (current >= size())
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

			table<T>::iterator begin()
			{
				return iterator(this, 0);
			}

			table<T>::iterator end()
			{
				return iterator(this, null_row);
			}
		};

		template <typename P, typename C>
		requires (std::is_standard_layout<P>::value && std::is_standard_layout<C>::value)
		class parent_child_table
		{

			struct actors
			{
				P parent;
				row_range children;
			};

			struct parent_child_table_header
			{
				row_id_type parents;
				row_id_type children;
			};

			table<actors> parents;
			table<C> children;

			bool move_child(row_id_type location, int shift)
			{
				auto& pcr = parents.get_at(location);
				auto& pc = parents[pcr.children.start];

				row_range new_pos{ pc.children.start + shift, pc.children.stop + shift, pc.children.reserved_stop + shift };

				if (new_pos.reserved_stop >= children.size())
				{
					return false;
				}

				bool success = children.copy_rows(pc.children.start, pc.children.stop, shift);
				if (success)
				{
					pc.children = new_pos;
				}
				return success;
			}

		public:

			parent_child_table()
			{
				;
			}

			template <typename B>
				requires (box<B, P>&& box<B, parent_child_table<P, C>::actors>)
			static row_id_type reserve_table(B* b, int parent_rows, int child_rows)
			{
				parent_child_table_header hdr;
				hdr.parents = null_row;
				hdr.children = null_row;
				row_id_type r = b->pack(hdr);
				auto* phdr = b->unpack<parent_child_table_header>(r);
				phdr->parents = table< parent_child_table<P, C>::actors >::reserve_table(b, parent_rows);
				phdr->children = table< C >::reserve_table(b, child_rows);
				return r;
			}

			template <typename B>
				requires (box<B, P>&& box<B, parent_child_table<P, C>::actors>)
			static parent_child_table get_table(B* b, row_id_type row)
			{
				parent_child_table pct;
				parent_child_table_header* hdr;
				hdr = b->unpack<parent_child_table<P, C>::parent_child_table_header>(row);
				pct.parents = table< parent_child_table<P, C>::actors >::get_table(b, hdr->parents);
				pct.children = table< C >::get_table(b, hdr->children);
				return pct;
			}

			template <typename B>
				requires (box<B, P>&& box<B, parent_child_table<P, C>::actors>)
			static parent_child_table create_table(B* b, int parent_rows, int child_rows, row_id_type& row)
			{
				parent_child_table pct;
				row = reserve_table(b, parent_rows, child_rows);
				pct = get_table(b, row);
				return pct;
			}

			parent_child_holder<P, C> create(row_id_type child_count)
			{
				auto pcr = parents.create(1);
				if (pcr.success())
				{
					auto& pc = parents[pcr.start];
					pc.children = children.create(child_count);
					return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], pcr.start, child_count, 0);
				}
				else
				{
					return parent_child_holder<P, C>(nullptr, nullptr, null_row, null_row, null_row);
				}
			}

			parent_child_holder<P, C> create_at(row_id_type location, row_id_type child_count)
			{
				if (location == null_row)
				{
					return create(child_count);
				}
				auto& pc = parents.get_at(location);
				pc.children = children.create(child_count);
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], location, child_count, 0);
			}

			parent_child_holder<P, C> clone(row_id_type location)
			{
				auto& src = parents.get_at(location);
				auto dest = create(src.children.reserved_size());
				int shift = dest.parent().children.start - src.children.start;
				children.copy_rows(src.children.start, src.children.reserved_stop, shift );
			}

			parent_child_holder<P, C> put_at(row_id_type location, row_id_type child_count)
			{
				if (location == null_row)
				{
					return create(child_count);
				}

				auto& pc = parents.get_at(location);

				if (child_count > pc.children.size()) 
				{
					int add_count = child_count - pc.children.size();
					return append_child(location, add_count);
				}
				else 
				{
					pc.children.stop = pc.children.start + child_count;
					return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], location, child_count, 0);
				}
			}

			parent_child_holder<P, C> append_child(row_id_type location, int add_child_count)
			{
				if (location == null_row)
				{
					return create(add_child_count);
				}

				auto& pc = parents.get_at(location);

				if (pc.children.start == 0 && pc.children.stop == 0 && pc.children.reserved_stop == 0)
				{
					throw std::invalid_argument("can't extend an uncreated");
				}

				row_id_type new_base = pc.children.stop;
				row_id_type new_start = pc.children.stop;

				row_id_type capacity_in_node = pc.children.reserved_size();
				if (capacity_in_node >= add_child_count)
				{
					pc.children.stop = pc.children.stop + add_child_count;
				}
				else
				{
					int capacity_ask = capacity_in_node + add_child_count;
					int capacity_allocate = 1;
					while (capacity_allocate < capacity_ask)
						capacity_allocate *= 2;

					row_id_type new_stop = pc.children.start + capacity_allocate;
					row_id_type shift = new_stop - pc.children.reserved_stop;

					if (new_stop < children.max()) 
					{
						for (row_id_type i = size()-1; i > location; i--) 
						{
							move_child(i, shift);
						}

						pc.children.reserved_stop = new_stop;
						pc.children.stop += add_child_count;
					}
					else 
					{
						return parent_child_holder<P,C>(nullptr, nullptr, null_row, null_row, null_row);
					}
				}

				int new_size = pc.children.stop - pc.children.start;

				return parent_child_holder<P, C>( &pc.parent, &children[pc.children.start], location, new_size, new_start);
			}

			void erase_child(row_id_type location, int child_index)
			{
				auto& pc = parents.get_at(location);
				children.copy_rows(pc.children.start + child_index, pc.children.stop, -1);
			}

			void erase(row_id_type location)
			{
				auto& pc = parents.get_at(location);
				for (row_id_type i = location; i > location; i--)
				{
					move_child(i, -1);
				}
				parents.erase({ location, location });
			}

			parent_child_holder<P, C> operator[](row_id_type row_id)
			{
				parent_child_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = parents[ row_id ];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start],  row_id, pc.children.size(), 0);
			}

			parent_child_holder<P, C> get(row_id_type row_id)
			{
				parent_child_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = parents[row_id];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], row_id, pc.children.size(), 0);
			}

			static int get_box_size(int _parent_rows, int _child_rows)
			{
				return table<actors>::get_box_size(_parent_rows) + table<C>::get_box_size(_child_rows);
			}

			row_id_type size() const
			{
				return parents.size();
			}

			row_id_type max() const
			{
				return parents.max();
			}

		};

		bool table_tests();

	}
}
