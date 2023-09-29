module;

#include <vector>
#include <functional>
#include <string>
#include <iostream>

export module corona.database:list_box;

import :store_box;
import :stdapi;
import :assert_if;

export struct list_box_data
		{
		public:
			block_id			id;
			uint32_t			length;
			relative_ptr_type	root_item;
			relative_ptr_type	last_item;

			list_box_data();
		};

export template <typename item_type>
		class list_box
		{
			struct list_link
			{
			public:
				item_type   data;
				relative_ptr_type next_link;
				relative_ptr_type previous_link;
			};

			relative_ptr_type		header_loc;
			serialized_box_container* box;
			index_mapper mapper;
			bool mapper_dirty;

			list_box_data* get_hdr()
			{
				return box->get_object<list_box_data>(header_loc);
			}

			list_link* first_link()
			{
				list_box_data* hdr = get_hdr();
				return box->get_object<list_link>(hdr->root_item);
			}

			list_link* last_link()
			{
				list_box_data* hdr = get_hdr();
				return box->get_object<list_link>(hdr->last_item);
			}

			list_link* next_link(list_link* n)
			{
				return box->get_object<list_link>(n->next_link);
			}

			list_link* previous_link(list_link* n)
			{
				return box->get_object<list_link>(n->previous_link);
			}

			list_link* put(const item_type& _src)
			{
				mapper_dirty = true;
				list_box_data* hdr = get_hdr();

				list_link temp, * n;
				temp.data = _src;
				temp.next_link = null_row;
				temp.previous_link = null_row;

				relative_ptr_type loc = box->put_object(temp);

				if (loc == null_row) {
					return nullptr;
				}

				if (hdr->last_item != null_row)
				{
					list_link* last_link = box->get_object<list_link>(hdr->last_item);
					n = box->get_object<list_link>(loc);
					n->previous_link = hdr->last_item;
					last_link->next_link = loc;
					hdr->last_item = loc;
				}
				else
				{
					hdr->root_item = loc;
					hdr->last_item = loc;
				}

				hdr->length++;
				list_link* l = box->get_object<list_link>(loc);
				return l;
			}

			void copy(int ls, const item_type* s)
			{
				list_box_data* hdr = get_hdr();
				int l = 0;

				while (l < ls)
				{
					put(*s);
					l++;
					s++;
				}

				hdr->length += l;
			}

			void mapper_check()
			{
				if (!mapper_dirty)
					return;

				mapper.clear();
				auto lbd = get_hdr();
				lbd->length = 0;
				relative_ptr_type ln = lbd->root_item;
				while (ln != null_row) {
					mapper.add(ln);
					auto nd = box->get_object<list_link>(ln);
					ln = nd->next_link;
					lbd->length++;
				}

				mapper_dirty = false;
			}

		public:

			using collection_type = list_box<item_type>;
			using iterator_item_type = item_type;
			using iterator_type = filterable_iterator<item_type, collection_type, iterator_item_type, index_mapper>;

			list_box() : box(nullptr), header_loc(null_row), mapper_dirty(true)
			{
			}

			list_box(const list_box& _src) : box(_src.box), header_loc(_src.header_loc), mapper_dirty(true)
			{

			}

			list_box operator = (const list_box& _src)
			{
				box = _src.box;
				header_loc = _src.header_loc;
				return *this;
			}

			relative_ptr_type get_location()
			{
				return header_loc;
			}

			static relative_ptr_type reserve(serialized_box_container* b)
			{
				list_box temp;
				list_box_data hdr;
				hdr.length = 0;
				hdr.root_item = null_row;
				hdr.last_item = null_row;
				auto location = b->put_object<list_box_data>(hdr);
				return location;
			}

			static list_box get(serialized_box_container* b, relative_ptr_type location)
			{
				list_box temp;
				temp.box = b;
				temp.header_loc = location;
				return temp;
			}

			static list_box create(serialized_box_container* b)
			{
				relative_ptr_type loc = reserve(b);
				list_box temp = get(b, loc);
				return temp;
			}

			list_box& operator += (std::vector<item_type>& _src)
			{
				copy(_src.size(), _src.data());
				return *this;
			}

			list_box& operator += (const array_box<item_type>& _src)
			{
				copy(_src.size(), _src.data());
				return *this;
			}

			uint16_t size()
			{
				list_box_data* hdr = get_hdr();
				return hdr->length;
			}

			void push_back(const item_type& t)
			{
				put(t);
			}

			iterator_type begin()
			{
				mapper_check();
				return iterator_type(this, 0, &mapper);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row, &mapper);
			}

			item_type& get_at(relative_ptr_type offset)
			{
				list_link* l = box->get_object<list_link>(offset);
				return l->data;
			}

			auto where(std::function<bool(const item_type&)> _predicate)
			{
				mapper_check();
				return iterator_type(this, _predicate, &mapper);
			}

			bool any_of(std::function<bool(const item_type&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const item_type&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(const item_type&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

			item_type& first()
			{
				auto t = first_link();
				if (!t)
					throw std::invalid_argument("list is empty");
				return t->data;
			}

			item_type& last()
			{
				auto t = last_link();
				if (!t)
					throw std::invalid_argument("list is empty");
				return t->data;
			}

			item_type& operator[](int idx)
			{
				mapper_check();
				int base_idx = 0;
				int sz = size();
				if (idx < 0)
				{
					base_idx = sz + idx;
				}
				else
				{
					base_idx = idx;
				}
				if (base_idx < 0 || base_idx >= size()) {
					throw std::invalid_argument("idx out of range, and come on, we even let you do idx < 0 and you still got it wrong");
				}
				auto map_idx = mapper.map(base_idx);
				auto link = box->get_object<list_link>(map_idx);
				return link->data;
			}
		};

		bool list_box_tests();

		template <typename new_type, typename value_ref, typename iter_type>
		list_box<new_type> create_list(serialized_box_container* _box, iter_type begin_iter, iter_type end_iter, std::function<new_type(value_ref&)> _transform)
		{
			auto new_list = list_box<new_type>::create(_box);
			for (auto iter = begin_iter; iter != end_iter; iter++) {
				auto obj = iter.get_object();
				auto key = _transform(obj);
				new_list.push_back(key);
			}
			return new_list;
		}

		list_box_data::list_box_data() : 
			length(0),
			root_item(null_row),
			last_item(null_row)
		{
			id = block_id::list_id();
		}

		export bool list_box_tests()
		{
			dynamic_box box;

			box.init(100000);

			using int_list = list_box<int>;

			int_list ix = int_list::create(&box);

			for (auto item : ix)
			{
				std::cout << __LINE__ << ": list_box iterator failed empty" << std::endl;
				return false;
			}

			for (int i = 0; i < 10; i++)
			{
				ix.push_back(i);
			}
			corona_size_t count = 0;
			for (auto its : ix)
			{
				if (its != count) {
					std::cout << __LINE__ << ": list_box data wrong" << std::endl;
					return false;
				}
				count++;
			}

			int check_prev1 = ix[-1];
			int check_prev2 = ix[-2];

			if (check_prev1 != 9) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			if (check_prev2 != 8) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			int check_tail0 = ix[-1];
			int check_tail1 = ix[-2];
			int check_head0 = ix[0];
			int check_head1 = ix[1];

			if (check_tail0 != 9) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			if (check_tail1 != 8) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			if (check_head0 != 0) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			if (check_head1 != 1) {
				std::cout << __LINE__ << ": list_box reverse array read data wrong" << std::endl;
				return false;
			}

			count = ix.count_if([](auto& t) { return t < 3;  });
			assert_if([count]() { return count == 3; }, "Wrong count");

			auto items = ix.where([](auto& t) { return t < 3;  });

			count = 0;
			for (auto r : items)
			{
				assert_if([r]() { return r < 3;  }, "Wrong values");
				count++;
			}
			assert_if([count]() { return count == 3; }, "Wrong count");


			return true;

		}
