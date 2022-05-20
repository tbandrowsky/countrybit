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
		class group_list
		{
		public:
			using grouped_items = list_box<VALUE>;

			KEY key;
			grouped_items items;
		};

		template <typename KEY, typename VALUE> class grouping : protected sorted_index<KEY, relative_ptr_type, 1>
		{
		protected:
			using group_by_collection = sorted_index<KEY, relative_ptr_type, 1>;
			using inner_type = group_by_collection::data_pair;
			using grouped_items = list_box<VALUE>;
			using group = group_list<KEY, VALUE>;

			using collection_type = grouping<KEY, VALUE>;
			using iterator_item_type = group;
			using iterator_type = filterable_iterator<iterator_item_type, collection_type, iterator_item_type, index_mapper>;

		public:

			grouping()
			{
			}

			grouping(serialized_box_container* _box, relative_ptr_type _location) :
				group_by_collection(_box,_location)
			{
			}

			grouping(const grouping& _src) :
				group_by_collection(_src)
			{

			}

			grouping operator = (const grouping& _src)
			{
				group_by_collection::operator=(_src);
				return *this;
			}

			static relative_ptr_type reserve_grouping(serialized_box_container* _b)
			{
				relative_ptr_type location;
				location = group_by_collection::reserve_sorted_index(_b);
				return location;
			}

			static grouping get_grouping(serialized_box_container* _b, relative_ptr_type _header_location)
			{
				auto si = group_by_collection::get_sorted_index(_b, _header_location);
				return si;
			}

			static grouping create_grouping(serialized_box_container* _b, relative_ptr_type& _header_location)
			{
				_header_location = reserve_grouping(_b);
				grouping new_index = get_grouping(_b, _header_location);
				return new_index;
			}

			group insert_or_assign(const KEY& _key, const VALUE& _value)
			{
				relative_ptr_type list_location;
				grouped_items lst;
				group grp;
				
				grp.key = _key;

				if (contains(_key)) 
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

			group get_at(KEY _key)
			{
				group g;

				g.key = _key;
				relative_ptr_type location = group_by_collection::get(_key).second;
				g.items = grouped_items::get(box, location);

				return g;
			}

			group get_at(relative_ptr_type ptr)
			{
				grouped_items lst;
				group grp;

				auto dp = group_by_collection::get(ptr);
				grp.key = dp.first;
				relative_ptr_type location = dp.second;
				grp.items = grouped_items::get(box, location);
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

			auto where(std::function<bool(const group&)> _predicate)
			{
				group_by_collection::mapper_check();
				return iterator_type(this, _predicate, &group_by_collection::mapper);
			}

			bool any_of(std::function<bool(const group&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const group&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(const group&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

		};

		void query_tests();
	}
}
