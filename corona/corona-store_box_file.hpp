#ifndef CORONA_STORE_BOX_FILE
#define CORONA_STORE_BOX_FILE

#include "corona-store_box.hpp"
#include "corona-constants.hpp"
#include "corona-sorted_index.hpp"
#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-file.hpp"
#include "corona-application.hpp"

#include <string>
#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <utility>
#include <functional>
#include <stdexcept>
#include <compare>
#include <coroutine>

namespace corona {

	using io_block_store = sorted_index<relative_ptr_type, box_block*>;

	class serialized_box_file_implementation : public serialized_box_implementation
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

		os_result get_header_async();
		os_result put_header_async();

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

	class persistent_box : public serialized_box_container
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
		box_file = file(global_job_queue.get(), box_name, file_open_types::open_existing);
	}

	void serialized_box_file_implementation::create_file()
	{
		box_file = file(global_job_queue.get(), box_name, file_open_types::create_new);
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

	void serialized_box_file_implementation::init(corona_size_t _length)
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
//			file_result ret = box_file.write(0, &header, sizeof(header));
//			r = ret.last_result;
		}
		catch (std::exception e)
		{
			os_result rx;
		}
	}

	void serialized_box_file_implementation::clear()
	{
		os_result r;
		try
		{
			//file_result ret_read = box_file.read(0, &header, sizeof(header));
			header.sbd._top = sizeof(header);
			//file_result ret_write = box_file.write(0, &header, sizeof(header));
		}
		catch (std::exception e)
		{
			os_result rx;
		}
		return;
	}

	void serialized_box_file_implementation::adjust(corona_size_t _length)
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
				//file_result ret = box_file.write(0, &header, sizeof(header));
				//r = ret.last_result;
			}
		}
		catch (std::exception e)
		{
			os_result rx;
		}
	}

	box_block*serialized_box_file_implementation::reserve(corona_size_t _length)
	{
		box_block* bb = nullptr;
		try
		{
			time_t current_time;
			time(&current_time);

			box_file.read(0, &header, sizeof(header));
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
			auto ret = box_file.write(0, &header, sizeof(header));
		}
		catch (std::exception e)
		{
			return bb;
		}

		return bb;
	}

	box_block* serialized_box_file_implementation::allocate(int64_t sizeofobj, int length)
	{
		box_block* atr;
		relative_ptr_type l = sizeofobj * length;
		atr = reserve(l + sizeof(box_block));
		return atr;
	}

	box_block* serialized_box_file_implementation::get_object(relative_ptr_type location)
	{
		box_block* bb = nullptr;
		if (transaction.contains(location))
		{
			bb = transaction[location].get_value();
		}
		else
		{
			box_block temp, * actual;
			box_file.read(location, &temp, sizeof(box_block));
			actual = (box_block*)transaction_data->allocate<char>(temp.payload_length);
			box_file.read(location, actual, actual->allocated_length);
		}
		return bb;
	}

	relative_ptr_type serialized_box_file_implementation::create_object(char* _src, int _length)
	{
		auto atr = allocate(1, _length);
		if (atr != nullptr)
		{
			char* d = &atr->data[0];
			if (_src != d) {
				std::copy(_src, _src + _length, &atr->data[0]);
			}
			return atr->location;
		}
		return null_row;
	}

	relative_ptr_type serialized_box_file_implementation::update_object(relative_ptr_type _location, char* _src, int _length)
	{
		relative_ptr_type r = null_row;
		if (transaction.contains(_location))
		{
			r = transaction[_location].second->location;
		}
		else
		{
			r = create_object(_src, _length);
		}
		return r;
	}

	bool serialized_box_file_implementation::delete_object(relative_ptr_type _location)
	{
		auto block = get_object(_location);
		if (block) {
			block->deleted = true;
			memset(&block->data[0], 0, block->payload_length);
			return block->deleted;
		}
		else
		{
			return false;
		}
	}

	relative_ptr_type serialized_box_file_implementation::copy_object(relative_ptr_type _location)
	{
		relative_ptr_type new_location = null_row;
		auto block = get_object(_location);
		if (block) {
			auto new_block = allocate(1, block->payload_length);
			char* s = &block->data[0];
			char* d = &new_block->data[0];
			std::copy(s, s + block->payload_length, d);
			new_location = new_block->location;
		}
		return new_location;
	}

	relative_ptr_type serialized_box_file_implementation::commit()
	{
		relative_ptr_type count_bytes = 0;
		for (auto trans : transaction)
		{
			auto io = trans.second;
			box_file.write(io->location, io, io->allocated_length);
			count_bytes += io->allocated_length;
		}
		return count_bytes;
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
}

#endif

