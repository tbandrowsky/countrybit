#pragma once

namespace corona
{
	namespace database
	{

		template <typename item_type>
		class value_reference 
		{
		public:
			item_type&		  item;
			relative_ptr_type location;
		};

		template <typename item_type>
		class value_object
		{
		public:
			item_type item;
			relative_ptr_type location;
		};

		class index_mapper
		{
			std::vector<relative_ptr_type> index_map;

		public:

			index_mapper()
			{
				;
			}

			void clear()
			{
				index_map.clear();
			}

			void add(relative_ptr_type _target)
			{
				index_map.push_back(_target);
			}

			relative_ptr_type map(relative_ptr_type _index)
			{
				return index_map[_index];
			}
		};

		template 
			<typename item_type, 
			typename collection_type, 
			typename value_ref= value_reference<item_type>,
			typename mapper_type=void>
			class filterable_iterator
		{
		public:

			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = value_ref;
			using pointer = value_ref*;  // or also value_type*
			using reference = value_ref&;  // or also value_type&

		private:
			collection_type* base;
			relative_ptr_type current;
			and_functions<value_ref> predicate;
			mapper_type* mapper;

			void move_first()
			{
				if (base->size() == 0) current = null_row;
				while (current != null_row && !predicate(get_object(current)))
				{
					current++;
					if (current >= base->size()) {
						current = null_row;
						break;
					}
				}
			}

			inline auto make_value_ref(value_reference<item_type>* ref, relative_ptr_type location)
			{
				return value_reference<item_type> { get_at_ref(mapper, location), location };
			}

			inline auto make_value_ref(value_object<item_type>* ref, relative_ptr_type location)
			{
				return value_object<item_type> { get_at_obj(mapper, location), location };
			}

			inline auto make_value_ref(item_type *ref, relative_ptr_type location)
			{
				return get_at_ref(mapper, location);
			}

			inline item_type& get_at_ref(void *mapper, relative_ptr_type location)
			{
				return base->get_at(location);
			}

			inline item_type& get_at_ref(index_mapper* mapper, relative_ptr_type location)
			{
				relative_ptr_type target = mapper->map(location);
				return base->get_at(target);
			}

			inline item_type get_at_obj(void* mapper, relative_ptr_type location)
			{
				return base->get_at(location);
			}

			inline item_type get_at_obj(index_mapper* mapper, relative_ptr_type location)
			{
				relative_ptr_type target = mapper->map(location);
				return base->get_at(target);
			}

		public:

			filterable_iterator(const filterable_iterator* _src,
				std::function<bool(const value_ref&)> _predicate) :
				base(_src->base),
				predicate(_src->predicate),
				mapper(_src->mapper),
				current(0)
			{
				predicate.and_fn(_predicate);
				move_first();
			}

			filterable_iterator(const filterable_iterator* _src, relative_ptr_type _current) :
				base(_src->base),
				predicate(_src->predicate),
				mapper(_src->mapper),
				current(_current)
			{
				move_first();
			}

			filterable_iterator(collection_type *_base, std::function<bool(const value_ref&)> _predicate, mapper_type *_mapper = nullptr ):
				base(_base),
				current(0),
				predicate(_predicate),
				mapper( _mapper )
			{
				move_first();
			}

			filterable_iterator(collection_type* _base, relative_ptr_type _current, mapper_type *_mapper = nullptr) :
				base(_base),
				current(_current),
				mapper( _mapper )
			{
				move_first();
			}

			filterable_iterator() :
				base(nullptr),
				current(null_row),
				mapper(nullptr)
			{
				predicate = [](const value_ref& a) { return true;  };
			}

			filterable_iterator(const filterable_iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				predicate = _src.predicate;
				mapper = _src.mapper;
			}

			filterable_iterator& operator = (const filterable_iterator& _src)
			{
				base = _src.base;
				current = _src.current;
				predicate = _src.predicate;
				return *this;
			}

			inline auto operator *()
			{
				value_ref *ref = nullptr;
				return make_value_ref(ref, current);
			}

			inline auto get_object()
			{
				value_ref* ref = nullptr;
				return make_value_ref(ref, current);
			}

			inline auto get_object(relative_ptr_type _idx)
			{
				value_ref* ref = nullptr;
				return make_value_ref(ref, current);
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
				while (current < base->size() && !predicate(get_object(current)))
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

			filterable_iterator where(std::function<bool(const value_ref&)> _predicate)
			{
				return filterable_iterator(this, _predicate);
			}

			template <typename new_key> grouping<new_key, item_type> group_by(serialized_box_container *_box, std::function<new_key(value_ref&)> _predicate)
			{
				grouping<new_key, value_ref> grp = grouping<new_key, value_ref>::create_grouping(box);

				for (auto i = begin(); i != end(); i++)
				{
					auto f = i.get_object();
					auto k = _predicate(f);
					grp.insert_or_assign(k, f);
				}

				return grp;
			}

			bool any_of(std::function<bool(const value_ref&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp.item) && _predicate(kp.item); };
				return std::any_of(begin(), end(), new_predicate);
			}

			bool all_of(std::function<bool(const value_ref&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp.item) && _predicate(kp.item); };
				return std::all_of(begin(), end(), new_predicate);
			}

			int count_if(std::function<bool(const value_ref&)> _predicate)
			{
				auto new_predicate = [this, _predicate](auto& kp) { return predicate(kp.item) && _predicate(kp.item); };
				return std::count_if(begin(), end(), new_predicate);
			}

		};

	}
}

