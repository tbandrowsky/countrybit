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
			row_id_type start;
			row_id_type stop;

			row_id_type size() { return stop - start;  }
		};

		template <typename P, typename C> class parent_child_holder
		{
			P* the_parent;
			C* the_children;
			row_id_type id;
			row_id_type length;

		public:

			parent_child_holder(P* _parent, C* _children, row_id_type _id, row_id_type _length) :
				the_parent(_parent),
				the_children(_children),
				id(_id),
				length(_length)
			{
				;
			}

			parent_child_holder() :
				the_parent(nullptr),
				the_children(nullptr),
				id(null_row),
				length(0)
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

			size_t size() const
			{
				return length;
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

			row_range create(row_id_type count)
			{
				auto x = hdr->last_row + count;
				if (x > hdr->max_rows)
					throw std::logic_error("table full");

				row_range rr;
				rr.start = hdr->last_row;
				rr.stop = x;
				hdr->last_row = x;
				return rr;
			}

			T& create(row_id_type count, row_range &rr)
			{
				auto x = hdr->last_row + count;
				if (x > hdr->max_rows)
					throw std::logic_error("table full");

				rr.start = hdr->last_row;
				rr.stop = x;
				hdr->last_row = x;
				return hdr->rows[rr.start];
			}

			T& insert(T& src, row_range& rr)
			{
				auto x = hdr->last_row + 1;
				if (x > hdr->max_rows)
					throw std::logic_error("table full");

				rr.start = hdr->last_row;
				rr.stop = x;
				hdr->last_row = x;
				hdr->rows[rr.start] = src;
				return hdr->rows[rr.start];
			}

			T& get_at(row_id_type& r)
			{
				if (r == null_row || r >= hdr->max_rows)
					throw std::logic_error("invalid row id");

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

			struct parent_child
			{
				P parent;
				row_range children;
			};

			struct parent_child_table_header
			{
				row_id_type parents;
				row_id_type children;
			};

			table<parent_child> parents;
			table<C> children;

		public:

			parent_child_table()
			{
				;
			}

			template <typename B>
			requires (box<B, P> && box<B, parent_child_table<P, C>::parent_child>)
			static row_id_type reserve_table( B *b, int parent_rows, int child_rows )
			{
				parent_child_table_header hdr;
				hdr.parents = null_row;
				hdr.children = null_row;
				row_id_type r = b->pack(hdr);
				auto* phdr = b->unpack<parent_child_table_header>(r);
				phdr->parents = table< parent_child_table<P,C>::parent_child >::reserve_table(b, parent_rows);
				phdr->children = table< C >::reserve_table(b, child_rows);
				return r;
			}

			template <typename B>
			requires (box<B, P> && box<B, parent_child_table<P, C>::parent_child>)
			static parent_child_table get_table(B* b, row_id_type row)
			{
				parent_child_table pct;
				parent_child_table_header* hdr;
				hdr = b->unpack<parent_child_table<P, C>::parent_child_table_header>(row);
				pct.parents = table< parent_child_table<P, C>::parent_child >::get_table(b, hdr->parents);
				pct.children = table< C >::get_table (b, hdr->children);
				return pct;
			}

			template <typename B>
			requires (box<B, P>&& box<B, parent_child_table<P, C>::parent_child>)
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
				auto& pc = parents[pcr.start];
				pc.children = children.create(child_count);
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], pcr.start, child_count);
			}

			parent_child_holder<P, C> create_at(row_id_type location, row_id_type child_count)
			{
				if (location == null_row)
				{
					return create(child_count);
				}
				auto pcr = parents.get_at(location);
				auto& pc = parents[pcr.start];
				pc.children = children.create(child_count);
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], pcr.start, child_count);
			}

			parent_child_holder<P, C> operator[](row_id_type row_id)
			{
				parent_child_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = parents[ row_id ];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], row_id, pc.children.size());
			}

			parent_child_holder<P, C> get(row_id_type row_id)
			{
				parent_child_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				auto& pc = parents[row_id];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], row_id, pc.children.size());
			}

			static int get_box_size(int _parent_rows, int _child_rows)
			{
				return table<parent_child>::get_box_size(_parent_rows) + table<C>::get_box_size(_child_rows);
			}

			row_id_type size()
			{
				return parents.size();
			}

		};

		bool table_tests();

	}
}
