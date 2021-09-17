#pragma once

#include <cstdint>
#include <exception>
#include <stdexcept>

namespace countrybit
{
	namespace database
	{
		typedef int32_t row_id_type;
		const row_id_type null_row = -1;

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
				return the_child;
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

		template <class T, int max_rows> 
		requires (std::is_standard_layout<T>::value)
		class table
		{
			T			rows[max_rows];
			row_id_type	last_row;

		public:

			table()
			{
				last_row = 0;
				for (row_id_type i = 0; i < max_rows; i++)
				{
					auto& r = rows[i];
					r = {};
				}
			}

			row_range create(row_id_type count)
			{
				auto x = last_row + count;
				if (x > max_rows)
					throw std::logic_error("table full");

				row_range rr;
				rr.start = last_row;
				rr.stop = x;
				last_row = x;
				return rr;
			}

			T& create(row_id_type count, row_range &rr)
			{
				auto x = last_row + count;
				if (x > max_rows)
					throw std::logic_error("table full");

				rr.start = last_row;
				rr.stop = x;
				last_row = x;
				return rows[rr.start];
			}

			T& insert(T& src, row_range& rr)
			{
				auto x = last_row + 1;
				if (x > max_rows)
					throw std::logic_error("table full");

				rr.start = last_row;
				rr.stop = x;
				last_row = x;
				rows[rr.start] = src;
				return rows[rr.start];
			}

			T& operator[](row_id_type & r)
			{
				if (r == null_row || r >= last_row)
					throw std::logic_error("invalid row id");

				return rows[r];
			}

			row_id_type size() const
			{
				return last_row;
			}

		};


		template <typename P, typename C, int max_rows, int avg_children_per_row>
		requires (std::is_standard_layout<P>::value && std::is_standard_layout<C>::value)
		class parent_child_table
		{
			struct parent_child
			{
				P parent;
				row_range children;
			};

			table<parent_child, max_rows> parents;
			table<C, avg_children_per_row * max_rows> children;

		public:

			void init()
			{
				parents.init();
				children.init();
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

		};



	}


}
