module;

#include <string>
#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <utility>
#include <functional>
#include <stdexcept>

export module corona.database:store_box_file;

import :store_box;
import :constants;
import :sorted_index;
import :file;
import :application;
import :function;

export using io_block_store = sorted_index<relative_ptr_type, box_block*>;

export class serialized_box_file_implementation : public serialized_box_implementation
{

	application* app;
	object_path box_name;
	file box_file;
	std::shared_ptr<dynamic_box> transaction_data;
	io_block_store transaction;
	relative_ptr_type transaction_header;

	void open_file();
	void create_file();

	struct file_header
	{
	public:
		object_name			branding;
		serialized_box_data sbd;
		time_t				last_grow;
		int64_t				bytes_added;
	} header;

	task<os_result> get_header_async();
	task<os_result> put_header_async();

public:

	serialized_box_file_implementation();
	serialized_box_file_implementation(application* _application, object_path _file_name);

	serialized_box_file_implementation(const serialized_box_file_implementation& _src) = delete;
	serialized_box_file_implementation operator =(const serialized_box_file_implementation& _src) = delete;

	serialized_box_file_implementation(serialized_box_file_implementation&& _src);
	serialized_box_file_implementation& operator =(serialized_box_file_implementation&& _src);

	static serialized_box_file_implementation* open(application* _application, object_path _file_name);
	static serialized_box_file_implementation* create(application* _application, object_path _file_name);

	virtual ~serialized_box_file_implementation();

	virtual corona_size_t size() const;
	virtual relative_ptr_type top() const;
	virtual corona_size_t free() const;

	task<os_result> init_task(corona_size_t _length);
	task<os_result> adjust_task(corona_size_t _length);
	task<os_result> clear_task();
	task<box_block*> reserve_task(corona_size_t _length);
	task<box_block*> allocate_task(int64_t sizeofobj, int length);
	task<box_block*> get_object_task(relative_ptr_type _length);
	task<relative_ptr_type> create_object_task(char* _src, int _length);
	task<relative_ptr_type> update_object_task(relative_ptr_type _destination, char* _src, int _length);
	task<bool> delete_object_task(relative_ptr_type _location);
	task<relative_ptr_type> copy_object_task(relative_ptr_type _location);
	task<relative_ptr_type> commit_task();

	sync<os_result> init_async(corona_size_t _length);
	sync<os_result> adjust_async(corona_size_t _length);
	sync<os_result> clear_async();
	sync<box_block*> reserve_async(corona_size_t _length);
	sync<box_block*> allocate_async(int64_t sizeofobj, int length);
	sync<box_block*> get_object_async(relative_ptr_type _length);
	sync<relative_ptr_type> create_object_async(char* _src, int _length);
	sync<relative_ptr_type> update_object_async(relative_ptr_type _destination, char* _src, int _length);
	sync<bool> delete_object_async(relative_ptr_type _location);
	sync<relative_ptr_type> copy_object_async(relative_ptr_type _location);
	sync<relative_ptr_type> commit_async();

	virtual void init(corona_size_t _length);
	virtual void adjust(corona_size_t _length);
	virtual void clear();
	virtual box_block* reserve(corona_size_t length);
	virtual box_block* allocate(int64_t sizeofobj, int length);
	virtual box_block* get_object(relative_ptr_type _src);
	virtual relative_ptr_type create_object(char* _src, int _length);
	virtual relative_ptr_type update_object(relative_ptr_type _rp, char* _src, int _length);
	bool delete_object(relative_ptr_type _location);
	relative_ptr_type copy_object(relative_ptr_type _location);
	virtual relative_ptr_type commit();
};

export class persistent_box : public serialized_box_container
{
	serialized_box box;

public:

	persistent_box()
	{
	}

	void open(application* _the_application, object_path _the_file_name)
	{
		box = serialized_box_file_implementation::open(_the_application, _the_file_name);
	}

	void create(application* _the_application, object_path _the_file_name)
	{
		box = serialized_box_file_implementation::create(_the_application, _the_file_name);
	}

	virtual serialized_box* get_box() { return &box; }
	virtual const serialized_box* get_box_const() const { return &box; }
	virtual serialized_box* check(int _bytes) { return _bytes < get_box()->free() ? get_box() : nullptr; }

	virtual ~persistent_box()
	{
		;
	}

};

void serialized_box_file_implementation::open_file()
{
	box_file = app->open_file(box_name, file_open_types::open_existing);
}

void serialized_box_file_implementation::create_file()
{
	box_file = app->open_file(box_name, file_open_types::create_new);
}

serialized_box_file_implementation::serialized_box_file_implementation() : app(nullptr)
{
	transaction_data = std::make_shared<dynamic_box>();
	transaction_data->init(1 << 20);
	transaction = io_block_store::create_sorted_index(transaction_data, transaction_header);
}

serialized_box_file_implementation::serialized_box_file_implementation(application* _application, object_path _file_name) :
	app(_application),
	box_name(_file_name)
{
	transaction_data = std::make_shared<dynamic_box>();
	transaction_data->init(1 << 20);
	transaction = io_block_store::create_sorted_index(transaction_data, transaction_header);
}

serialized_box_file_implementation::serialized_box_file_implementation(serialized_box_file_implementation&& _src)
{
	app = _src.app;
	box_file = std::move(box_file);
	box_name = std::move(box_name);
	transaction_data = std::move(_src.transaction_data);
	transaction = std::move(_src.transaction);
}

serialized_box_file_implementation& serialized_box_file_implementation::operator =(serialized_box_file_implementation&& _src)
{
	app = _src.app;
	box_file = std::move(box_file);
	box_name = std::move(box_name);
	transaction_data = std::move(_src.transaction_data);
	transaction = std::move(_src.transaction);
	return *this;
}

serialized_box_file_implementation* serialized_box_file_implementation::open(application* _application, object_path _file_name)
{
	serialized_box_file_implementation* sbfi = new serialized_box_file_implementation(_application, _file_name);
	sbfi->open_file();
	return sbfi;
}

serialized_box_file_implementation* serialized_box_file_implementation::create(application* _application, object_path _file_name)
{
	serialized_box_file_implementation* sbfi = new serialized_box_file_implementation(_application, _file_name);
	sbfi->open_file();
	return sbfi;
}

serialized_box_file_implementation::~serialized_box_file_implementation()
{
}

task<os_result> serialized_box_file_implementation::init_task(corona_size_t _length)
{
	os_result r;
	try
	{
		auto size_bytes = _length + sizeof(header);
		if (size_bytes < _length)
		{
			auto bytes_to_add = _length - size_bytes;
			box_file.add(_length);
		}
		header.branding = "Corona Database File";
		header.sbd._box_id = block_id::database_id();
		header.sbd._size = size_bytes;
		header.sbd._top = sizeof(header);
		auto ret = co_await box_file.write(0, &header, sizeof(header));
		r = ret.last_result;
	}
	catch (std::exception e)
	{
		co_return r;
	}
	co_return r;
}

task<os_result> serialized_box_file_implementation::clear_task()
{
	os_result r;
	try
	{
		auto ret_read = co_await box_file.read(0, &header, sizeof(header));
		header.sbd._top = sizeof(header);
		auto ret_write = co_await box_file.write(0, &header, sizeof(header));
		r = ret_write.last_result;
	}
	catch (std::exception e)
	{
		co_return r;
	}
	co_return r;
}

task<os_result> serialized_box_file_implementation::adjust_task(corona_size_t _length)
{
	os_result r;
	try
	{
		auto size_bytes = box_file.size();
		if (size_bytes < _length)
		{
			auto bytes_to_add = _length - size_bytes;
			box_file.add(_length);
			header.sbd._box_id = block_id::database_id();
			header.sbd._size = size_bytes;
			header.sbd._top = sizeof(header);
			auto ret = co_await box_file.write(0, &header, sizeof(header));
			r = ret.last_result;
		}
	}
	catch (std::exception e)
	{
		co_return r;
	}
	co_return r;
}

task<box_block *> serialized_box_file_implementation::reserve_task(corona_size_t _length)
{
	box_block* bb = nullptr;
	try
	{
		time_t current_time;
		time(&current_time);

		co_await box_file.read(0, &header, sizeof(header));
		int total_length = free() - _length;

		if (total_length < 0)
		{
			double factor = 1.0;
			int64_t bytes_to_add = _length;
			int64_t elapsed_seconds = current_time - header.last_grow;

			if (elapsed_seconds > 0)
			{
				int64_t bytes_added_per_second = header.bytes_added / elapsed_seconds;
				int64_t bytes_added_per_day = bytes_added_per_second / 86400;
				if (bytes_added_per_day < 0) {
					bytes_to_add = 7 * bytes_added_per_day;
				}
			}
			else
			{
				bytes_to_add = size() + _length;
			}

			box_file.add(bytes_to_add);

		}

		total_length = _length + sizeof(box_block);
		bb = (box_block*)transaction_data->allocate<char>(total_length);
		bb->deleted = false;
		bb->allocated_length = total_length;
		bb->payload_length = _length;
		bb->location = header.sbd._top;

		transaction.insert_or_assign(bb->location, bb);

		header.sbd._top += _length;
		auto ret = co_await box_file.write(0, &header, sizeof(header));
	}
	catch (std::exception e)
	{
		co_return bb;
	}

	co_return bb;
}

task<box_block *> serialized_box_file_implementation::allocate_task(int64_t sizeofobj, int length)
{
	box_block *atr;
	relative_ptr_type l = sizeofobj * length;
	atr = co_await reserve_task(l + sizeof(box_block));
	co_return atr;
}

task<box_block*> serialized_box_file_implementation::get_object_task(relative_ptr_type location)
{
	box_block* bb = nullptr;
	if (transaction.contains(location))
	{
		bb = transaction[location].get_value();
	}
	else 
	{
		box_block temp, *actual;
		co_await box_file.read(location, &temp, sizeof(box_block));
		actual = (box_block *)transaction_data->allocate<char>(temp.payload_length );
		co_await box_file.read(location, actual, actual->allocated_length);
	}
	co_return bb;
}

task<relative_ptr_type> serialized_box_file_implementation::create_object_task(char* _src, int _length)
{
	auto atr = co_await allocate_task(1, _length);
	if (atr != nullptr) 
	{
		char* d = &atr->data[0];
		if (_src != d) {
			std::copy(_src, _src + _length, &atr->data[0]);
		}
		co_return atr->location;
	}
	co_return null_row;
}

task<relative_ptr_type> serialized_box_file_implementation::update_object_task(relative_ptr_type _location, char* _src, int _length)
{
	relative_ptr_type r = null_row;
	if (transaction.contains(_location))
	{
		r = transaction[_location].second->location;
	}
	else
	{
		r = create_object_task(_src, _length );
	}
	co_return r;
}

task<bool> serialized_box_file_implementation::delete_object_task(relative_ptr_type _location)
{
	auto block = co_await get_object_task(_location);
	if (block) {
		block->deleted = true;
		::ZeroMemory(&block->data[0], block->payload_length);
		co_return block->deleted;
	}
	else 
	{
		co_return false;
	}
}

task<relative_ptr_type> serialized_box_file_implementation::copy_object_task(relative_ptr_type _location)
{
	relative_ptr_type new_location = null_row;
	auto block = co_await get_object_task(_location);
	if (block) {
		auto new_block = allocate(1, block->payload_length);
		char* s = &block->data[0];
		char* d = &new_block->data[0];
		std::copy(s, s + block->payload_length, d);
		new_location = new_block->location;
	}
	co_return new_location;
}

task<relative_ptr_type> serialized_box_file_implementation::commit_task()
{
	relative_ptr_type count_bytes = 0;
	for (auto trans : transaction)
	{
		auto io = trans.second;
		co_await box_file.write(io->location, io, io->allocated_length);       
		count_bytes += io->allocated_length;
	}
	co_return count_bytes;
}

sync<os_result> serialized_box_file_implementation::init_async(corona_size_t _length)
{
	co_return init_task(_length);
}

sync<os_result> serialized_box_file_implementation::adjust_async(corona_size_t _length)
{
	co_return adjust_task(_length);
}

sync<os_result> serialized_box_file_implementation::clear_async()
{
	co_return clear_task();
}

sync<box_block*> serialized_box_file_implementation::reserve_async(corona_size_t _length)
{
	co_return reserve_task(_length);
}

sync<box_block*> serialized_box_file_implementation::allocate_async(int64_t sizeofobj, int length)
{
	co_return allocate_task(sizeofobj, length);
}

sync<box_block*> serialized_box_file_implementation::get_object_async(relative_ptr_type _position)
{
	co_return get_object_task(_position);
}

sync<relative_ptr_type> serialized_box_file_implementation::create_object_async(char* _src, int _length)
{
	co_return create_object_task(_src, _length);
}

sync<relative_ptr_type> serialized_box_file_implementation::update_object_async(relative_ptr_type _destination, char* _src, int _length)
{
	co_return update_object_task(_destination, _src, _length);
}

sync<bool> serialized_box_file_implementation::delete_object_async(relative_ptr_type _location)
{
	co_return delete_object_task(_location);
}

sync<relative_ptr_type> serialized_box_file_implementation::copy_object_async(relative_ptr_type _location)
{
	co_return copy_object_task(_location);
}

sync<relative_ptr_type> serialized_box_file_implementation::commit_async()
{
	co_return commit_task();
}

void serialized_box_file_implementation::init(corona_size_t _length)
{
	init_async(_length);
}

void serialized_box_file_implementation::adjust(corona_size_t _length)
{
	adjust_async(_length);
}

void serialized_box_file_implementation::clear()
{
	clear_async();
}

box_block *serialized_box_file_implementation::reserve(corona_size_t length)
{
	return reserve_async(length);
}

box_block* serialized_box_file_implementation::allocate(int64_t sizeofobj, int length)
{
	return allocate_async(sizeofobj, length);
}

box_block* serialized_box_file_implementation::get_object(relative_ptr_type _src)
{
	return get_object_async(_src);
}

relative_ptr_type serialized_box_file_implementation::create_object(char* _src, int _length)
{
	return create_object_async(_src, _length);
}

relative_ptr_type serialized_box_file_implementation::update_object(relative_ptr_type _rp, char* _src, int _length)
{
	return update_object_async(_rp, _src, _length);
}

bool serialized_box_file_implementation::delete_object(relative_ptr_type _location)
{
	return delete_object_async(_location);
}

relative_ptr_type serialized_box_file_implementation::copy_object(relative_ptr_type _location)
{
	return copy_object_async(_location);
}

relative_ptr_type serialized_box_file_implementation::commit()
{
	return commit_async();
}

corona_size_t serialized_box_file_implementation::size() const
{
	return header.sbd._size;
}

relative_ptr_type serialized_box_file_implementation::top() const
{
	return header.sbd._top;
}

corona_size_t serialized_box_file_implementation::free() const
{
	return header.sbd._size - header.sbd._top;
}
