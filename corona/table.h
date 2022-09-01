#pragma once

namespace corona
{
	namespace database
	{

		struct row_range 
		{
			relative_ptr_type			start;
			relative_ptr_type			stop;
			relative_ptr_type			reserved_stop;

			relative_ptr_type size() { return stop - start;  }
			relative_ptr_type reserved_size() { return reserved_stop - start; }
			relative_ptr_type free() { return reserved_stop - stop; }

			bool success() const 
			{
				return start != null_row && stop != null_row;
			}
		};

		template <typename P, typename C> class item_details_holder
		{
			P* the_object;
			C* the_details;
			relative_ptr_type id;
			relative_ptr_type length;

		public:

			item_details_holder(relative_ptr_type _id, P* _parent, C* _children, relative_ptr_type _length) :
				id(_id),
				the_object(_parent),
				the_details(_children),
				length(_length)
			{
				;
			}

			item_details_holder() :
				the_object(nullptr),
				the_details(nullptr),
				id(null_row),
				length(0)
			{
				;
			}

			item_details_holder(const item_details_holder& _src) :
				the_object(_src.the_object),
				the_details(_src.the_details),
				id(_src.id),
				length(_src.length)
			{
				;
			}

			item_details_holder operator = (const item_details_holder& _src) 
			{
				the_object = _src.the_object;
				the_details = _src.the_details;
				id = _src.id;
				length = _src.length;
				return *this;
			}

			P& item()
			{
				if (is_null())
					throw std::invalid_argument("is null");
				return *the_object;
			}

			C& detail(relative_ptr_type idx)
			{
				if (is_null())
					throw std::invalid_argument("is null");
				if (idx >= length)
					throw std::invalid_argument("idx out of range");
				return the_details[idx];
			}

			P* pitem()
			{
				return the_object;
			}

			C* pdetails()
			{
				return the_details;
			}

			bool is_null() const
			{
				return !the_object;
			}

			relative_ptr_type row_id() const
			{
				return id;
			}

			size_t size() const
			{
				return length;
			}

			bool success() const
			{
				return the_object != nullptr && the_details != nullptr;
			}
		};

		template <typename T> 
		requires (std::is_standard_layout<T>::value)
		class table
		{

			struct table_header
			{
				block_id block;
				relative_ptr_type
					id;
				relative_ptr_type
					max_rows,
					last_row;
				bool dynamic;
				T   rows[1];
			};

			table_header* hdr;
			serialized_box_container* box;

			using collection_type = table<T>;
			using iterator_item_type = value_reference<T>;
			using iterator_type = filterable_iterator<T, collection_type, iterator_item_type>;


		public:

			table(  ) : 
				hdr(nullptr),
				box(nullptr)
			{
				
			}

			table(const table& src) : hdr(src.hdr), box(src.box)
			{

			}

			static int get_box_size( int _rows )
			{
				return sizeof(table_header) + sizeof(T) * (_rows + 8);
			}

			static relative_ptr_type reserve_table(serialized_box_container* _b, int _max_rows, bool _dynamic = false)
			{
				table t;

				relative_ptr_type hdr_id;

				relative_ptr_type bytes_size = get_box_size(_max_rows);

				char c = 0;
				hdr_id = _b->fill(c, bytes_size);

				if (hdr_id == null_row)
					return hdr_id;

				t.hdr = _b->get_object<table_header>(hdr_id);
				t.hdr->block = block_id::table_id();
				t.hdr->id = hdr_id;
				t.hdr->dynamic = _dynamic;
				t.hdr->max_rows = _max_rows;
				t.hdr->last_row = 0;

				for (int i = 0; i < _max_rows; i++) {
					T item = {};
					t.hdr->rows[i] = item;
				}

				return hdr_id;
			}

			static table get_table(serialized_box_container* _b, relative_ptr_type offset)
			{
				table t;

				t.hdr = _b->get_object<table_header>(offset);
				if (!t.hdr->block.is_table())
				{
					throw std::logic_error("Did not read table correctly.");
				}
				t.box = _b;

				return t;
			}

			static table create_table(serialized_box_container* _b, int _max_rows, relative_ptr_type& offset, bool _dynamic = false)
			{
				table t;
				offset = reserve_table(_b, _max_rows, _dynamic);
				t.hdr = _b->get_object<table_header>(offset);
				t.box = _b;
				return t;
			}

			bool storage_check(corona_size_t new_rows)
			{
				relative_ptr_type new_end_row = hdr->last_row + new_rows;
				if (new_end_row < hdr->max_rows)
				{
					return true;
				}
				else if (hdr->dynamic)
				{
					relative_ptr_type demand_bytes = new_rows * sizeof(T);
					if (box->check(demand_bytes)) {
						hdr->max_rows += new_rows;
						return true;
					}
					throw std::logic_error("insufficient collection / table space, box full");
				}
				else {
					throw std::logic_error("insufficient collection / table space");
				}
				return false;
			}

			relative_ptr_type create(corona_size_t count, T* items)
			{
				relative_ptr_type new_row;
				if (!storage_check(count))
					return null_row;
				auto x = hdr->last_row + count;
				new_row = hdr->last_row;
				hdr->last_row = x;
				if (items) {
					for (auto i = 0; i < count; i++)
					{
						hdr->rows[i + new_row] = items[i];
					}
				}
				else {
					for (auto i = 0; i < count; i++)
					{
						hdr->rows[i + new_row] = {};
					}
				}
				return new_row;
			}

			T* create(corona_size_t count, relative_ptr_type& _new_row)
			{
				if (!storage_check(count)) {
					_new_row = null_row;
					return nullptr;
				}
				auto x = hdr->last_row + count;
				_new_row = hdr->last_row;
				hdr->last_row = x;
				return &hdr->rows[_new_row];
			}

			T* insert(relative_ptr_type index, corona_size_t count)
			{
				if (!storage_check(count))
					return nullptr;

				if (!check(index) || count < 0 || size()==0) return nullptr;

				relative_ptr_type dest_idx = hdr->last_row + count;
				relative_ptr_type source_idx = hdr->last_row;

				while (source_idx >= index) 
				{
					hdr->rows[dest_idx] = hdr->rows[source_idx];
					dest_idx--;
					source_idx--;
				}

				hdr->last_row += count;

				return get_ptr(index);
			}

			T *erase(relative_ptr_type index, uint32_t count)
			{
				if (!check(index) || count < 0 || count > hdr->last_row) return nullptr;

				relative_ptr_type dest_idx = index;
				relative_ptr_type source_idx = index + count;

				/*
					* rows = 5
					dest	source
					0		1
					1		2
					2		3
					3		4
					*/

				while (dest_idx <= hdr->last_row) {
					hdr->rows[dest_idx] = hdr->rows[source_idx];
					dest_idx++;
					source_idx++;
				}
				
				hdr->last_row -= count;

				return get_ptr(index);
			}

			bool check(int index)
			{
				if (index == null_row || index >= size() || index < 0)
				{
					return false;
				}
				return true;
			}

			// src here should be const
			T& append(T& src, row_range& rr)
			{
				if (!storage_check(1)) {
					rr = { null_row, null_row, null_row };
					return hdr->rows[0];
				}
				auto x = hdr->last_row + 1;
				rr.start = hdr->last_row;
				rr.stop = x;
				rr.reserved_stop = x;
				hdr->last_row = x;
				hdr->rows[rr.start] = src;
				return hdr->rows[rr.start];
			}

			T* get_ptr(relative_ptr_type& r)
			{
				if (r == null_row || r >= hdr->max_rows || r < 0)
					throw std::invalid_argument("invalid row id");

				if (r > hdr->last_row)
				{
					hdr->last_row = r;
				}

				return &hdr->rows[r];
			}

			T& get_at(relative_ptr_type& r)
			{
				if (r == null_row || r >= hdr->max_rows || r < 0)
					throw std::invalid_argument("invalid row id");

				if (r > hdr->last_row)
				{
					hdr->last_row = r;
				}

				return hdr->rows[r];
			}

			T& operator[](relative_ptr_type r)
			{
				return get_at(r);
			}

			relative_ptr_type size() const
			{
				return hdr->last_row;
			}

			relative_ptr_type capacity() const
			{
				return hdr->max_rows;
			}

			void clear()
			{
				hdr->last_row = 0;
			}

			iterator_type begin()
			{
				return iterator_type(this, first_row);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row);
			}

			auto where(std::function<bool(const iterator_item_type&)> _predicate)
			{
				return iterator_type(this, _predicate);
			}

			T& first_item(std::function<bool(const iterator_item_type&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					throw std::logic_error("sequence has no elements");
				}
				return w.get_value();
			}

			relative_ptr_type first_index(std::function<bool(const iterator_item_type&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					return null_row;
				}
				return w.get_index();
			}

			bool any_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			int count_if(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

			void sort(std::function<bool(T& a, T& b)> fn)
			{
				std::sort(hdr->rows, hdr->rows + size(), fn);
			}

		};


		template <typename P, typename C>
		requires (std::is_standard_layout<P>::value && std::is_standard_layout<C>::value)
		class item_details_table
		{

			struct item_type
			{
				P item;
				row_range detail_range;
			};

			struct item_detail_table_header
			{
				relative_ptr_type item_location;
				relative_ptr_type detail_location;
			};

			table<item_type> item;
			table<C> details;

		public:

			item_details_table()
			{
				;
			}

			static relative_ptr_type reserve_table(serialized_box_container* b, int item_rows, int detail_rows, bool dynamic = false)
			{
				item_detail_table_header hdr;
				hdr.item_location = null_row;
				hdr.detail_location = null_row;
				relative_ptr_type r = b->put_object(hdr);
				auto* phdr = b->get_object<item_detail_table_header>(r);
				phdr->item_location = table< item_details_table<P, C>::item_type >::reserve_table(b, item_rows);
				phdr->detail_location = table< C >::reserve_table(b, detail_rows, dynamic);
				return r;
			}

			static item_details_table get_table(serialized_box_container* b, relative_ptr_type row)
			{
				item_details_table pct;
				item_detail_table_header* hdr;
				hdr = b->get_object<item_details_table<P, C>::item_detail_table_header>(row);
				pct.item = table< item_details_table<P, C>::item_type >::get_table(b, hdr->item_location);
				pct.details = table<C>::get_table(b, hdr->detail_location);
				return pct;
			}

			static item_details_table create_table(serialized_box_container* b, int item_rows, int detail_rows, relative_ptr_type& row, bool dynamic = false)
			{
				item_details_table pct;
				row = reserve_table(b, item_rows, detail_rows, dynamic);
				pct = get_table(b, row);
				return pct;
			}

			item_details_holder<P, C> create_item(P* _item, int detail_count, C* _new_details)
			{
				relative_ptr_type new_row;
				item_type *it = item.create(1, new_row);

				if (it)
				{
					if (_item) {
						it->item = *_item;
					}
					else {
						it->item = {};
					}
					C *dest_detail = details.create(detail_count, it->detail_range.start);
					if (dest_detail) {
						it->detail_range.reserved_stop = it->detail_range.start + detail_count;
						it->detail_range.stop = it->detail_range.start;
						if (_new_details)
						{
							append_detail(new_row, detail_count, _new_details);
						}
						return item_details_holder<P, C>(new_row, &it->item, details.get_ptr(it->detail_range.start), detail_count);
					}
					else 
					{
						return item_details_holder<P, C>(null_row, nullptr, nullptr, null_row);
					}
				}
				else
				{
					return item_details_holder<P, C>(null_row, nullptr, nullptr, null_row);
				}
			}

			item_details_holder<P, C> put_item(relative_ptr_type& location, P* _item, int detail_count, C *mod_details)
			{
				if (location == null_row) 
				{
					auto pcr = create_item(_item, detail_count, mod_details);
					location = pcr.row_id();
				}
				else 
				{
					item[location].item = *_item;
				}

				clear_details(location);
				if (mod_details) {
					append_detail(location, detail_count, mod_details);
				}

				return get_item(location);
			}

			item_details_holder<P, C> clone_item(relative_ptr_type location)
			{
				auto src = item.get_at(location);
				auto new_item = create_item(src.pitem(), src.detail_range.reserved_size(), src.pdetail());
				return new_item;
			}

			void clear_details(relative_ptr_type location)
			{
				auto& pc = item.get_at(location);
				pc.detail_range.stop = pc.detail_range.start;
			}

			void append_detail(relative_ptr_type location, int add_detail_count, C *new_details)
			{
				auto& pc = item[location];

				if (pc.detail_range.start == 0 && pc.detail_range.stop == 0 && pc.detail_range.reserved_stop == 0)
				{
					throw std::invalid_argument("can't extend an uncreated");
				}

#if DETAILS
				std::cout << "append_detail " << location << ", start " << pc.detail_range.start << ", stop " << pc.detail_range.stop << ", reserved stop " << pc.detail_range.reserved_stop << std::endl;
#endif

				relative_ptr_type new_details_location = pc.detail_range.stop;
				relative_ptr_type existing_end = pc.detail_range.reserved_stop;
				relative_ptr_type minimum_end = new_details_location + add_detail_count;

				if (minimum_end <= existing_end)
				{
					pc.detail_range.stop += add_detail_count;
				}
				else
				{
					relative_ptr_type capacity_allocate = pc.detail_range.reserved_size() * 2;
					relative_ptr_type allocation_end = capacity_allocate + pc.detail_range.start;

					while (allocation_end < minimum_end) {
						capacity_allocate *= 2;
						allocation_end = capacity_allocate + pc.detail_range.start;
					}

					relative_ptr_type insert_rows = allocation_end - existing_end;

#if DETAILS
					std::cout << "new loc " << new_details_location << ", reserved end " << existing_end << ", new end " << allocation_end << ", insert count " << insert_rows << std::endl;
#endif

					details.insert(allocation_end, insert_rows);

					pc.detail_range.stop += add_detail_count;
					pc.detail_range.reserved_stop = allocation_end;

					for (relative_ptr_type i = location+1; i < size(); i++)
					{
						auto& pcx = item[i];
						pcx.detail_range.start += insert_rows;
						pcx.detail_range.stop += insert_rows;
						pcx.detail_range.reserved_stop += insert_rows;
					}

					for (relative_ptr_type i = pc.detail_range.stop; i < pc.detail_range.reserved_stop; i++) {
						C temp = {};
						details[i] = temp;
					}
				}

				for (relative_ptr_type i = 0; i < add_detail_count; i++) {
					auto& new_detail = details[new_details_location];
					if (new_details) {
						new_detail = new_details[i];
					}
					else {
						C temp = {};
						new_detail = temp;
					}
					new_details_location++;
				}

			}

			void erase_item(relative_ptr_type location)
			{
				auto& pc = item.get_at(location);
				relative_ptr_type shift = pc.detail_range.reserved_size();
				details.erase(pc.detail_range.reserved_stop, shift);
				for (relative_ptr_type i = location + 1; i < size(); i++)
				{
					auto& pcx = item[location];
					pcx.detail_range.start -= shift;
					pcx.detail_range.stop -= shift;
					pcx.detail_range.reserved_stop -= shift;
				}
				item.erase(location, 1);
			}

			bool check(int index)
			{
				return item.check(index);
			}

			item_details_holder<P, C> operator[](relative_ptr_type row_id)
			{
				return get_item(row_id);
			}

			item_details_holder<P, C> get_item(relative_ptr_type row_id)
			{
				item_details_holder<P, C> nullpc;
				if (row_id == null_row) return nullpc;
				if (!item.check(row_id)) {
					throw std::invalid_argument("get_item out of range" + std::to_string(row_id));
				}
				auto& pc = item[row_id];
				relative_ptr_type lsize = pc.detail_range.size();
				relative_ptr_type lstart = pc.detail_range.start;
				return item_details_holder<P, C>(row_id, &pc.item, details.get_ptr(lstart), lsize);
			}

			static int get_box_size(int _parent_rows, int _child_rows)
			{
				return table<item_type>::get_box_size(_parent_rows) + table<C>::get_box_size(_child_rows);
			}

			relative_ptr_type size() const
			{
				return item.size();
			}

			relative_ptr_type capacity() const
			{
				return item.capacity();
			}

			void clear()
			{
				item.clear();
				details.clear();
			}

		};

		bool table_tests();

	}
}
