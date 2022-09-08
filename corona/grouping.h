#pragma once

namespace corona
{
	namespace database
	{

		class grouping_header_type
		{
		public:
			block_id			block;
			relative_ptr_type	group_by_items;
		};

		template <typename KEY, typename VALUE>
		class group
		{
		public:
			using grouped_items = list_box<VALUE>;

			KEY key;
			grouped_items items;
		};

		template <typename KEY, typename VALUE> class grouped : protected sorted_index<KEY, relative_ptr_type, 1>
		{
		protected:
			using group_by_collection = sorted_index<KEY, relative_ptr_type, 1>;
			using inner_type = group_by_collection::data_pair;
			using grouped_items = list_box<VALUE>;

			using collection_type = grouped<KEY, VALUE>;
			using iterator_item_type = group<KEY, VALUE>;
			using iterator_type = filterable_iterator<iterator_item_type, collection_type, iterator_item_type, index_mapper>;

		public:

			grouped()
			{
			}

			grouped(serialized_box_container* _box, relative_ptr_type _location) :
				group_by_collection(_box, _location)
			{
			}

			grouped(const grouped& _src) :
				group_by_collection(_src)
			{

			}

			grouped operator = (const grouped& _src)
			{
				group_by_collection::operator=(_src);
				return *this;
			}

			static relative_ptr_type reserve_grouped(serialized_box_container* _b)
			{
				relative_ptr_type location;
				location = group_by_collection::reserve_sorted_index(_b);
				return location;
			}

			static grouped get_grouped(serialized_box_container* _b, relative_ptr_type _header_location)
			{
				grouped grp(_b, _header_location);
				return grp;
			}

			static grouped create_grouped(serialized_box_container* _b, relative_ptr_type& _header_location)
			{
				_header_location = reserve_grouped(_b);
				grouped new_index = get_grouped(_b, _header_location);
				return new_index;
			}

			iterator_item_type insert_or_assign(const KEY& _key, const VALUE& _value)
			{
				relative_ptr_type list_location;
				grouped_items lst;
				iterator_item_type grp;

				grp.key = _key;

				auto box = group_by_collection::get_box();

				if (group_by_collection::contains(_key))
				{
					inner_type& dp = group_by_collection::get(_key);
					list_location = dp.second;
					lst = grouped_items::get(box, list_location);
				}
				else
				{
					lst = grouped_items::create(box);
					list_location = lst.get_location();
					group_by_collection::insert_or_assign(_key, list_location);
				}

				grp.items = lst;
				grp.items.push_back(_value);

				return grp;
			}

			iterator_item_type get_at(KEY _key)
			{
				iterator_item_type g;

				g.key = _key;
				relative_ptr_type location = group_by_collection::get(_key).second;
				g.items = grouped_items::get(this->get_box(), location);

				return g;
			}

			iterator_item_type get_at(relative_ptr_type ptr)
			{
				grouped_items lst;
				iterator_item_type grp;

				auto dp = group_by_collection::get(ptr);
				grp.key = dp.first;
				relative_ptr_type location = dp.second;
				grp.items = grouped_items::get(this->get_box(), location);
				return grp;
			}

			iterator_type begin()
			{
				return iterator_type(this, 0, &group_by_collection::mapper);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row, &group_by_collection::mapper);
			}

			auto where(std::function<bool(const iterator_item_type&)> _predicate)
			{
				group_by_collection::mapper_check();
				return iterator_type(this, _predicate, &group_by_collection::mapper);
			}

			bool any_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

		};

		template <typename new_type, typename item_type, typename iter_type> 
		grouped<new_type, item_type> create_grouped(serialized_box_container *_box, iter_type begin_iter, iter_type end_iter, std::function<new_type(item_type&)> _transform )
		{
			relative_ptr_type header_location;
			auto new_group = grouped<new_type, item_type>::create_grouped(_box, header_location);
			for (auto iter = begin_iter; iter != end_iter; iter++) {
				auto obj = iter.get_object();
				auto key = _transform(obj);
				new_group.insert_or_assign(key, obj);
			}
			return new_group;
		}

		void query_tests();
	}
}
