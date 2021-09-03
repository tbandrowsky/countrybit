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
		};

		template <class T, int max_rows> 
		requires (std::is_trivial<T>::value)
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
			row_id_type length;
			row_id_type id;

		public:

			parent_child_holder(P* _parent, C* _children, row_id_type _length, row_id_type _id) :
				the_parent(_parent),
				the_children(_children),
				length(_length),
				id(_id)
			{
				;
			}

			P& parent()
			{
				return *the_parent;
			}

			C& child(uint32_t idx)
			{
				if (idx >= length)
					throw std::invalid_argument("idx out of range");
				return the_children[idx];
			}

			int row_id()
			{
				return id;
			}

			size_t size()
			{
				return length;
			}
		};

		template <typename P, typename C, int max_rows, int max_child_rows>
		requires (std::is_trivial<P>::value && std::is_trivial<C>::value)
		class parent_child_table
		{
			struct parent_child
			{
				P parent;
				row_range children;
			};

			table<parent_child, max_rows> parents;
			table<C, max_child_rows> children;

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
				return parent_child_holder(&pc.parent, &children[pc.children.start], pcr.start);
			}

			parent_child_holder<P, C> operator[](row_id_type row_id)
			{
				auto& pcr = parents[ row_id ];
				auto& pc = parents[pcr.start];
				return parent_child_holder(&pc.parent, &children[pc.children.start]);
			}

			parent_child_holder<P, C> operator[](row_id_type row_id)
			{
				auto& pcr = parents[row_id];
				auto& pc = parents[pcr.start];
				return parent_child_holder(&pc.parent, &children[pc.children.start]);
			}

		};

	}
}
