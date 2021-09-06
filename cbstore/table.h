#pragma once

#include <cstdint>
#include <exception>
#include <stdexcept>

namespace countrybit
{
	namespace database
	{
		typedef uint32_t row_id_type;

		struct row_range 
		{
			row_id_type start;
			row_id_type stop;

			row_id_type size() { return stop - start;  }
		};

		template <class T, int max_rows> 
		requires (std::is_standard_layout<T>::value)
		class table
		{
			T			rows[max_rows];
			row_id_type	top_row;

		public:

			table()
			{
				;
			}

			void init()
			{
				top_row = 1;
				for (row_id_type i = 0; i < max_rows; i++)
				{
					auto& r = rows[i];
					r.item = {};
					r.id = i;
				}
			}

			row_range create(row_id_type count)
			{
				auto x = top_row + count;
				if (x > max_rows)
					throw std::logic_error("table full");

				row_range rr;
				rr.start = top_row;
				rr.stop = rr.start + count;
				top_row = x;
				return rr;
			}

			T& operator[](row_id_type & r)
			{
				if (r >= max_rows)
					throw std::logic_error("invalid row id");

				if (r > top_row) 
				{
					top_row = r + 1;
				}

				return rows[r];
			}
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
				id(0),
				length(0)
			{
				;
			}

			P& parent()
			{
				if (!the_parent)
					throw std::invalid_argument("no parent");
				return *the_parent;
			}

			C& child(uint32_t idx)
			{
				if (idx >= length)
					throw std::invalid_argument("idx out of range");
				return the_children[idx];
			}

			int row_id() const
			{
				return id;
			}

			size_t size() const
			{
				return length;
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
				auto& pc = parents[ row_id ];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], row_id, pc.children.size());
			}

			parent_child_holder<P, C> get(row_id_type row_id)
			{
				auto& pc = parents[row_id];
				return parent_child_holder<P, C>(&pc.parent, &children[pc.children.start], row_id, pc.children.size());
			}

		};

	}
}
