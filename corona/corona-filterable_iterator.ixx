module;

#include <algorithm>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <iterator>
#include <compare>

export module corona:filterable_iterator;

import :constants;

export template <typename item_type>
class value_reference 
{
public:
	item_type&		  item;
	relative_ptr_type location;
};

export template <typename item_type>
class value_object
{
public:
	item_type item;
	relative_ptr_type location;
};

export class index_mapper
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

export template <typename itemType> class and_functions {
public:

	using boolean_fn = std::function<bool(const itemType&)>;

private:

	std::vector<boolean_fn> functions;

public:

	and_functions()
	{
	}

	and_functions(boolean_fn fn)
	{
		functions.push_back(fn);
	}

	and_functions operator = (const and_functions& srcs)
	{
		functions = srcs.functions;
		return *this;
	}

	and_functions& operator = (and_functions&& srcs)
	{
		functions = std::move(srcs.functions);
		return *this;
	}

	and_functions(const and_functions& srcs)
	{
		functions = srcs.functions;
	}

	and_functions(and_functions&& srcs)
	{
		functions = std::move(srcs.functions);
	}

	void and_fn(boolean_fn fn)
	{
		functions.push_back(fn);
	}

	bool operator()(const itemType& x)
	{
		return std::all_of(functions.begin(), functions.end(), [x](boolean_fn fn) { return fn(x); });
	}
};

export template
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
		while (current != null_row)
		{
			const auto& obj = get_object(current);
			if (predicate(obj))
				break;
			current++;
			if (current >= base->size()) {
				current = null_row;
				break;
			}
		}
	}

	auto make_value_ref(value_reference<item_type>* ref, relative_ptr_type location)
	{
		return value_reference<item_type> { get_at_ref(mapper, location), location };
	}

	auto make_value_ref(value_object<item_type>* ref, relative_ptr_type location)
	{
		return value_object<item_type> { get_at_obj(mapper, location), location };
	}

	auto make_value_ref(item_type *ref, relative_ptr_type location)
	{
		return get_at_ref(mapper, location);
	}

	item_type& get_at_ref(void *mapper, relative_ptr_type location)
	{
		return base->get_at(location);
	}

	item_type& get_at_ref(index_mapper* mapper, relative_ptr_type location)
	{
		relative_ptr_type target = mapper->map(location);
		return base->get_at(target);
	}

	item_type get_at_obj(void* mapper, relative_ptr_type location)
	{
		return base->get_at(location);
	}

	item_type get_at_obj(index_mapper* mapper, relative_ptr_type location)
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
		;
	}

	filterable_iterator(const filterable_iterator* _src) :
		base(_src->base),
		predicate(_src->predicate),
		mapper(_src->mapper),
		current(0)
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

	auto operator *()
	{
		value_ref *ref = nullptr;
		return make_value_ref(ref, current);
	}

	auto get_object()
	{
		value_ref* ref = nullptr;
		return make_value_ref(ref, current);
	}

	auto get_object(relative_ptr_type _idx)
	{
		value_ref* ref = nullptr;
		return make_value_ref(ref, current);
	}

	relative_ptr_type get_index()
	{
		return current;
	}

	filterable_iterator begin() const
	{
		return filterable_iterator(this);
	}

	filterable_iterator end() const
	{
		return filterable_iterator(this, null_row);
	}

	filterable_iterator operator++()
	{
		if (current == null_row)
			return end();

		current++;
		while (current < base->size())
		{
			const auto& obj = get_object(current);
			if (predicate(obj)) {
				return filterable_iterator(this, current);
			}
			current++;
		}

		current = null_row;

		return end();
	}

	filterable_iterator operator++(int)
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

	int count()
	{
		return std::count_if(begin(), end(), [](const auto& kp) { return true;  });
	}

	/*

	template <typename new_key> grouped<new_key, item_type> group_by(serialized_box_container *_box, std::function<new_key(const value_type&)> _transform)
	{
		return create_grouped<value_type, value_type, filterable_iterator>(_box, begin(), end(), _transform);
	}

	template <typename new_key, typename old_value_ref> list_box<new_key> select(serialized_box_container* _box, std::function<new_key(const old_value_ref&)> _transform)
	{
		return create_list<new_key, old_value_ref, filterable_iterator>(_box, begin(), end(), _transform);
	}

	auto order_by(serialized_box_container* _box, std::function<int(value_type&, value_type&)> _compare)
	{
		auto results = create_array<value_type, value_type, filterable_iterator>(_box, begin(), end(), [](value_type& a) { return a; });
		results.sort(_compare);
		return results;
	}
*/
};

