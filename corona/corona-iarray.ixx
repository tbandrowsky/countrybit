
module;

#include <functional>
#include <algorithm>
#include <vector>
#include <initializer_list>
#include <memory>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <compare>

export module corona:iarray;
import :constants;
import :assert_if;
import :filterable_iterator;
import :store_box;

export template <typename item_type, int max_items>
struct iarray
{
	item_type data[max_items];
	corona_size_t length;

	using collection_type = iarray<item_type, max_items>;
	using iterator_item_type = value_reference<item_type>;
	using iterator_type = filterable_iterator<item_type, collection_type, iterator_item_type>;

	iarray() 
	{
		length = 0;
		for (corona_size_t i = 0; i < max_items; i++)
		{
			data[i] = {};
		}
	};

	iarray(iterator_type& src)
	{
		length = 0;
		for (auto srci : src)
		{
			push_back(srci.item);
		}
		for (corona_size_t i = length; i < max_items; i++)
		{
			data[i] = {};
		}
	}

	iarray(const std::vector<item_type>& src)
	{
		copy(src.data(), src.size());
	}

	iarray(const std::initializer_list<item_type>& src)
	{
		length = 0;
		for (auto itm : src)
		{
			push_back(itm);
		}
		for (corona_size_t i = length; i < max_items; i++)
		{
			data[i] = {};
		}
	}

	iarray& operator = (const std::vector<item_type>& src)
	{
		copy(src.data(), src.size());
		return *this;
	}

	iarray& operator += (iarray& src)
	{
		for (int i = 0; i < src.size(); i++)
		{
			push_back(src[i]);
		}
		return *this;
	}

	const item_type* value() const
	{
		return &data[0];
	}

	corona_size_t size() const
	{
		return length;
	}

	int get_index(item_type* _item)
	{
		return _item - data;
	}

	const item_type& operator[](corona_size_t idx) const
	{
		return data[idx];
	}

	item_type& operator[](corona_size_t idx)
	{
		return data[idx];
	}

	item_type& get_at(corona_size_t idx)
	{
		return data[idx];
	}

	item_type* get_ptr(corona_size_t idx)
	{
		return &data[idx];
	}

	void copy(const item_type* src, corona_size_t srclength)
	{
		item_type* d = &data[0];
		length = 0;

		while (length < max_items && srclength)
		{
			*d = *src;
			length++;
			src++;
			d++;
			srclength--;
		}
	}

	void clear()
	{
		length = 0;
	}

	void push_back(const item_type& t)
	{
		if (length < max_items) {
			data[length] = t;
			length++;
		}
	}

	item_type* append()
	{
		item_type* d = nullptr;
		if (length < max_items) {
			d = &data[length];
			d = new (d) item_type();
			length++;
		}
		return d;
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

	item_type& first_value(std::function<bool(const iterator_item_type&)> predicate)
	{
		iterator_type w = where(predicate);
		if (w == end()) {
			throw std::logic_error("sequence has no elements");
		}
		item_type&t = get_at( w.get_index() );
		return t;
	}

	relative_ptr_type first_index(std::function<bool(const iterator_item_type&)> predicate)
	{
		auto w = this->where(predicate);
		if (w == end()) {
			return null_row;
		}
		return w->get_row_id();
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

	void sort(std::function<bool(const item_type& a, const item_type& b)> fn)
	{
		std::sort(data, data + length, fn);
	}

};

export template <typename item_type>
class array_box
{
	struct array_box_data 
	{
	public:
		corona_size_t length;
		corona_size_t max_items;
		item_type data[1];
	};

	array_box_data* hdr;

	void copy(corona_size_t ls, const item_type* s)
	{
		item_type* d = &hdr->data[0];
		corona_size_t l = 0;

		while (l < hdr->max_items && l < ls)
		{
			*d = *s;
			l++;
			s++;
			d++;
		}

		if (l < hdr->max_items)
		{
			hdr->length = l;
		}
		else
		{
			hdr->length = hdr->max_items - 1;
		}
	}

public:
	using collection_type = array_box<item_type>;
	using iterator_item_type = value_reference<item_type>;
	using iterator_type = filterable_iterator<item_type, collection_type, iterator_item_type>;

	array_box() : hdr(nullptr)
	{
	}

	array_box(char* c) : hdr((array_box_data*)c)
	{
	}

	static relative_ptr_type reserve(std::shared_ptr<serialized_box_container> b, corona_size_t items_length)
	{
		array_box temp;
		auto location = b->fill<char>(0, sizeof(array_box_data) + items_length * sizeof(item_type));
		temp.hdr = b->get_object<array_box_data>(location);
		temp.hdr->max_items = items_length;
		temp.hdr->length = 0;
		temp.hdr->data[0] = {};
		return location;
	}

	static array_box get(std::shared_ptr<serialized_box_container> b, corona_size_t location)
	{
		array_box temp;
		temp.hdr = b->get_object<array_box_data>(location);
		return temp;
	}

	static array_box create(std::shared_ptr<serialized_box_container> b, corona_size_t items_length, relative_ptr_type& _dest)
	{
		_dest = reserve(b, items_length);
		auto temp = get(b, _dest);
		return temp;
	}

	static array_box create(char* b, corona_size_t chars_length)
	{
		array_box temp;
		temp.hdr = (array_box_data*)(b);
		temp.hdr->max_items = ( chars_length - sizeof(array_box_data) ) / sizeof(item_type);
		temp.hdr->length = 0;
		return temp;
	}

	static array_box get(char* b)
	{
		array_box temp;
		temp.hdr = (array_box_data*)(b);
		return temp;
	}

	static corona_size_t get_box_size(corona_size_t length)
	{
		return sizeof(array_box_data) + sizeof(item_type) * length;
	}

	array_box& operator = (std::vector<item_type>& _src)
	{
		copy(_src.size(), _src.data());
		return *this;
	}

	array_box& operator = (const array_box& _src)
	{
		hdr = _src.hdr;
		return *this;
	}

	array_box& operator = (const char* s)
	{
		copy(s);
		return *this;
	}

	const item_type* data() const
	{
		return &hdr->data[0];
	}

	item_type value(corona_size_t _idx) const
	{
		return hdr->data[_idx];
	}

	item_type& operator[](corona_size_t _idx)  const
	{
		if (_idx < 0 || _idx >= hdr->max_items)
			throw std::invalid_argument("range error");
		return hdr->data[_idx];
	}

	item_type& get_at(corona_size_t _idx) const
	{
		if (_idx < 0 || _idx >= hdr->max_items)
			throw std::invalid_argument("range error");
		return hdr->data[_idx];
	}

	corona_size_t max_size() const
	{
		return hdr->max_items;
	}

	corona_size_t size() const
	{
		return hdr->length;
	}

	void push_back(item_type& t)
	{
		if (hdr->length < hdr->max_items) {
			hdr->data[hdr->length] = t;
			hdr->length++;
		}
	}

	iterator_type begin()
	{
		return iterator_type(this, first_row);
	}

	iterator_type end()
	{
		return iterator_type(this, null_row);
	}

	auto where(std::function<bool(iterator_item_type&)> predicate)
	{
		return iterator(this, 0, predicate);
	}

	item_type& first_value(std::function<bool(iterator_item_type&)> predicate)
	{
		auto w = this->where(predicate);
		if (w == end()) {
			throw std::logic_error("sequence has no elements");
		}
		return w->get_index();
	}

	relative_ptr_type first_index(std::function<bool(iterator_item_type&)> predicate)
	{
		auto w = this->where(predicate);
		if (w == end()) {
			return null_row;
		}
		return w->get_index();
	}

	bool any_of(std::function<bool(item_type&)> predicate)
	{
		return std::any_of(hdr->data, hdr->data + hdr->length, predicate);
	}

	bool all_of(std::function<bool(item_type&)> predicate)
	{
		return std::all_of(hdr->data, hdr->data + hdr->length, predicate);
	}

	int count_if(std::function<bool(item_type&)> predicate)
	{
		return std::count_if(hdr->data, hdr->data + hdr->length, predicate);
	}

	void sort(std::function<bool(item_type& a, item_type& b)> fn)
	{
		std::sort(hdr->data, hdr->data + hdr->length, fn);
	}

};

export template <typename new_type, typename value_ref, typename iter_type>
array_box<new_type> create_array(serialized_box_container* _box, iter_type begin_iter, iter_type end_iter, std::function<new_type(value_ref&)> _transform)
{
	relative_ptr_type dest;
	int c = begin_iter.count();
	auto new_list = array_box<new_type>::create(_box, c, dest);
	for (auto iter = begin_iter; iter != end_iter; iter++) {
		auto obj = iter.get_object();
		auto key = _transform(obj);
		new_list.push_back(key);
	}
	return new_list;
}

export bool array_box_tests()
{
	char box[8192];

	using int_array = array_box<int>;

	int_array ix = int_array::create(box, 4000);

	for (auto item : ix)
	{
		std::cout << __LINE__ << ": array_box iterator failed empty" << std::endl;
		return false;
	}

	for (int k = 4000; k < sizeof(box); k++)
	{
		box[k] = 0;
	}

	for (int i = 0; i < ix.max_size(); i++)
	{
		ix.push_back(i);
	}

	for (int i = 0; i < ix.size(); i++)
	{
		if (ix[i] != i) {
			std::cout << __LINE__ << ": array box failed" << std::endl;
			return false;
		}
	}

	for (int k = 4000; k < sizeof(box); k++)
	{
		if (box[k] != 0) {
			std::cout << __LINE__ << ": array box failed" << std::endl;
			return false;
		}
	}

	iarray<int, 100> test_iarray;

	for (auto item : test_iarray)
	{
		std::cout << __LINE__ << ": irray iterator failed empty" << std::endl;
		return false;
	}

	for (int i = 0; i < 10; i++)
		test_iarray.push_back(i);

	int iteration_count = 0;

	for (auto r : test_iarray)
	{
		if (r.item != r.location || test_iarray[r.location] != r.item) {
			std::cout << __LINE__ << ": iarray failed" << std::endl;
			return false;
		}
		iteration_count++;
	}
	assert_if(iteration_count == 10, "Wrong count");

	auto count = test_iarray.count_if([](auto& t) { return t.item < 3;  });
	assert_if(count == 3, "Wrong count");

	auto items = test_iarray.where([](auto& t) { return t.item < 5;  });

	count = 0;
	for (auto r : items)
	{
		auto rti = r.item;
		assert_if(rti < 5, "Wrong values");
		count++;
	}
	assert_if(count == 5, "Wrong count");

	items = test_iarray.where([](auto& t) { return true;  });

	items = items.where([](auto& t) {
		return t.item < 5;
		});

	items = items.where([](auto& t) {
		auto v = (t.item % 2);
		return v == 0;
		});

	count = 0;
	for (auto r : items)
	{
		auto rti = r.item;
		assert_if(rti < 5 && rti % 2 == 0, "Wrong values");
		count++;
	}
	assert_if(count == 3, "Wrong count");

	return true;
}
