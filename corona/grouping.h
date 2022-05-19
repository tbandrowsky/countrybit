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

		template <typename KEY, typename VALUE> class grouping
		{
			using group_by_collection = sorted_index<KEY, relative_ptr_type, 1>;
			using group_by_collection_iterator = group_by_collection::iterator;
			using grouped_items = list_box<VALUE>;
			using group = group_list<KEY, VALUE>;

			relative_ptr_type			header_location;
			serialized_box_container* box;

			grouping_header_type* get_group_header()
			{
				grouping_header_type* t;
				t = box->unpack<grouping_header_type>(header_location);
				if (!t->block.is_group()) {
					throw std::logic_error("did not read grouping correctly");
				}
				return t;
			};

			group_by_collection get_group_by()
			{
				auto hdr = get_group_header();
				return group_by_collection::get_sorted_index(box, hdr->group_by_items);
			};

		public:

			grouping() : header_location(null_row), box(nullptr)
			{
			}

			grouping(serialized_box_container* _box, relative_ptr_type _location) : box(_box), header_location(_location)
			{
			}

			grouping(const grouping& _src) : box(_src.box), header_location(_src.header_location)
			{

			}

			grouping operator = (const grouping& _src)
			{
				box = _src.box;
				header_location = _src.header_location;
				return *this;
			}

			static relative_ptr_type reserve_grouping(serialized_box_container* _b)
			{
				grouping_header_type hdr, *phdr;

				hdr.block = block_id::grouping_id();
				relative_ptr_type header_location = _b->pack(hdr);
				phdr = _b->unpack<grouping_header_type>(header_location);
				phdr->group_by_items = group_by_collection::reserve_sorted_index(_b);
				return header_location;
			}

			static grouping get_grouping(serialized_box_container* _b, relative_ptr_type _header_location)
			{
				grouping si(_b, _header_location);
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
				group_by_collection index = get_group_by();
				group_by_collection_iterator iter = index[_key];
				group g;

				g.key = _key;

				if (iter != std::end(index))
				{
					relative_ptr_type location = iter->get_value();
					g.items = grouped_items::get(box, location);
				}
				else 
				{
					g.items = grouped_items::create(box);
					index.insert_or_assign(_key, g.items.get_location());
				}

				g.items.push_back(_value);
				return g;
			}

			group get_at(KEY _key)
			{
				group g;

				group_by_collection index = get_group_by();
				group_by_collection_iterator iter = index[_key];

				g.key = _key;
				relative_ptr_type location = iter->get_value();
				g.items = grouped_items::get(box, location);

				return g;
			}
		};

		void query_tests();
	}
}
