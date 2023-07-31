#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <numeric>

#include "constants.h"
#include "store_box.h"
#include "sorted_index.h"
#include "file.h"

#define TRACE_DYNAMIC_BOX 0

namespace corona
{
	namespace database
	{

		using io_block_store = sorted_index<relative_ptr_type, box_block *>;

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

			corona::database::task<os_result> get_header_async();
			corona::database::task<os_result> put_header_async();

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

			corona::database::task<os_result> init_task(corona_size_t _length);
			corona::database::task<os_result> adjust_task(corona_size_t _length);
			corona::database::task<os_result> clear_task();
			corona::database::task<box_block *> reserve_task(corona_size_t _length);
			corona::database::task<box_block *> allocate_task(int64_t sizeofobj, int length);
			corona::database::task<box_block *> get_object_task(relative_ptr_type _length);
			corona::database::task<relative_ptr_type> create_object_task(char* _src, int _length);
			corona::database::task<relative_ptr_type> update_object_task(relative_ptr_type _destination, char *_src, int _length);
			corona::database::task<bool> delete_object_task(relative_ptr_type _location);
			corona::database::task<relative_ptr_type> copy_object_task(relative_ptr_type _location);
			corona::database::task<relative_ptr_type> commit_task();

			corona::database::sync<os_result> init_async(corona_size_t _length);
			corona::database::sync<os_result> adjust_async(corona_size_t _length);
			corona::database::sync<os_result> clear_async();
			corona::database::sync<box_block*> reserve_async(corona_size_t _length);
			corona::database::sync<box_block*> allocate_async(int64_t sizeofobj, int length);
			corona::database::sync<box_block*> get_object_async(relative_ptr_type _length);
			corona::database::sync<relative_ptr_type> create_object_async(char* _src, int _length);
			corona::database::sync<relative_ptr_type> update_object_async(relative_ptr_type _destination, char* _src, int _length);
			corona::database::sync<bool> delete_object_async(relative_ptr_type _location);
			corona::database::sync<relative_ptr_type> copy_object_async(relative_ptr_type _location);
			corona::database::sync<relative_ptr_type> commit_async();

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

			void open(application *_the_application, object_path _the_file_name)
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

	}
}
