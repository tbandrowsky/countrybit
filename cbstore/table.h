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

			T& operator[](row_id_type & r)
			{
				if (r == null_row || r >= hdr->max_rows)
					throw std::logic_error("invalid row id");

				if (r > hdr->last_row)
				{
					hdr->last_row = r;
				}

				return hdr->rows[r];
			}

			row_id_type size() const
			{
				return hdr->last_row;
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

	}
}
