#pragma once

#include "constants.h"

namespace countrybit
{
	namespace database
	{
		template 
			<typename item_type, 
			typename collection_type> class filterable_iterator
		{
			collection_type* base;
			relative_ptr_type current;
			and_functions<item_type> predicate;

			void move_first()
			{
				if (base->size() == 0) current = null_row;
				while (current != null_row && !predicate(base->get_at(current)))
				{
					current++;
					if (current >= base->size()) {
						current = null_row;
						break;
					}
				}
			}

		public:

			struct value_ref
			{
				item_type& item;
				relative_ptr_type location;
			};

			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = value_ref;
			using pointer = value_ref*;  // or also value_type*
			using reference = value_ref&;  // or also value_type&

			filterable_iterator(const filterable_iterator* _src,
				std::function<bool(const item_type&)> _predicate) :
				base(_src->base),
				predicate(_src->predicate),
				current(0)
			{
				predicate.and_fn(_predicate);
				move_first();
			}

			filterable_iterator(const filterable_iterator* _src, relative_ptr_type _current) :
				base(_src->base),
				current(_current),
				predicate(_src->predicate)
			{
				move_first();
			}

			filterable_iterator(collection_type *_base, std::function<bool(const item_type&)> _predicate) :
				base(_base),
				current(0),
				predicate(_predicate)
			{
				move_first();
			}

			filterable_iterator(collection_type* _base, relative_ptr_type _current) :
				base(_base),
				current(_current)
			{
				move_first();
			}

			filterable_iterator() :
				base(nullptr),
				current(null_row)
			{
				predicate = [](item_type& a) { return true;  };
			}

			filterable_iterator(const filterable_iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				predicate = _src.predicate;
			}

			filterable_iterator& operator = (const filterable_iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				predicate = _src.predicate;
				return *this;
			}

			inline value_ref operator *()
			{
				return value_ref{ base->get_at(current), current };
			}

			inline item_type* operator ->()
			{
				return &base->get_at(current);
			}

			inline value_ref get_value()
			{
				return value_ref{ base->get_at(current), current };
			}

			inline relative_ptr_type get_index()
			{
				return current;
			}

			inline filterable_iterator begin() const
			{
				return filterable_iterator(this, 0);
			}

			inline filterable_iterator end() const
			{
				return filterable_iterator(this, null_row);
			}

			inline filterable_iterator operator++()
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

				return filterable_iterator(this, current);
			}

			inline filterable_iterator operator++(int)
			{
				filterable_iterator tmp(*this);
				operator++();
				return tmp;
			}

			bool operator == (const filterable_iterator& _src) const
			{
				return _src.current == current;
			}

			bool operator != (const filterable_iterator& _src)
			{
				return _src.current != current;
			}

			bool eoi()
			{
				return begin() == end();
			}

			bool exists()
			{
				return begin() != end();
			}

			filterable_iterator where(std::function<bool(const item_type&)> _predicate)
			{
				return filterable_iterator(this, _predicate);
			}

			bool any_of(std::function<bool(const item_type&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp) && _predicate(kp); };
				return std::any_of(begin(), end(), new_predicate);
			}

			bool all_of(std::function<bool(const item_type&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp) && _predicate(kp); };
				return std::all_of(begin(), end(), new_predicate);
			}

			int count_if(std::function<bool(const item_type&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp) && _predicate(kp); };
				return std::count_if(begin(), end(), new_predicate);
			}

		};

	}
}

