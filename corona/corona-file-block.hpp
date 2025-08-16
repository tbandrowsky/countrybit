#pragma once

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This contains a file block, which implements buffers on top.

Notes
The big todo is whenn commmit happens, all the i/os should be issued 
in parallel.  The onnly thing really is that you need to hack up the run
into a run_async or something like that, all the hard i/o completion 
port stuff is done, but what's needed is to issue all the writes
then wait all them together.

For Future Consideration
*/

#ifndef CORONA_FILE_BLOCK
#define CORONA_FILE_BLOCK

namespace corona
{

	enum buffer_access_type {
		access_read = 1,
		access_write = 2
	};


	class file_buffer
	{
	public:
		int64_t start;
		int64_t stop;
		int64_t top;
		buffer  buff;
		bool is_dirty;
        date_time last_accessed;

		file_buffer() 
		{
			start = stop = top = {};
			is_dirty = false;
		}

		file_buffer(int64_t _start, int64_t _stop, bool _is_append = false)
		{
			start = _start;
			stop = _stop;
			top = _is_append ? start : stop;
			buff.init(stop - start);
		}

		file_buffer(const file_buffer& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			is_dirty = _src.is_dirty;
			buff = _src.buff;
			last_accessed = _src.last_accessed;
		}

		file_buffer(file_buffer&& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			is_dirty = _src.is_dirty;
			buff = std::move(_src.buff);
			last_accessed = _src.last_accessed;
		}

		file_buffer& operator =(file_buffer&& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			buff = std::move(_src.buff);
			is_dirty = _src.is_dirty;
			last_accessed = _src.last_accessed;
			return *this;
		}

		file_buffer& operator =(const file_buffer& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			buff = _src.buff;
			is_dirty = _src.is_dirty;
			last_accessed = _src.last_accessed;
			return *this;
		}

		int64_t add(int64_t _length)
		{
			int64_t new_top = top + _length;
			if (new_top > stop)
				return -1;
			int64_t old_top = top;
			top = new_top;
			return old_top;
		}

		void grow(int64_t _additional_bytes)
		{
			buffer new_buff;

			int64_t new_start = start;
			int64_t new_stop = stop + _additional_bytes;

			new_buff.init(new_stop - new_start);

			unsigned char* s = buff.get_uptr();
			unsigned char* p = new_buff.get_uptr();

			std::copy(s, s + buff.get_size(), p);

			buff = std::move(new_buff); // TODO, use move here

			start = new_start;
			stop = new_stop;
		}


	};

	class trans_commit_job;

	class file_block_interface
	{
	public:
		virtual file_command_result write(int64_t _location, void* _buffer, int _buffer_length) = 0;
		virtual file_command_result read(int64_t _location, void* _buffer, int _buffer_length) = 0;
		virtual file_command_result append(void* _buffer, int _buffer_length) = 0;

		virtual relative_ptr_type allocate_space(int64_t _size, int64_t* _actual_size) = 0;
		virtual void free_space(int64_t _location) = 0;
		virtual int64_t add(int _bytes_to_add) = 0;
		virtual bool is_free_capable() = 0;

		virtual file* get_fp() = 0;

		virtual void commit() = 0;

		virtual int buffer_count() = 0;
		virtual void clear() = 0;
		virtual int64_t size() = 0;

	};

	
	class file_block : public file_block_interface
	{
		std::shared_ptr<file> fp;
		std::vector<std::shared_ptr<file_buffer>> buffers;
		std::shared_ptr<file_buffer> append_buffer;
		int64_t block_size = 65536;
		int64_t append_size = 65536 * 16;

		lockable buffer_lock;

		std::shared_ptr<file_buffer> acquire_buffer(int64_t _start, int64_t _length, buffer_access_type _feast)
		{
			int64_t _stop = _start + _length;

			int64_t new_start = _start,
					new_stop = _stop;

			std::vector<std::shared_ptr<file_buffer>> eaten_buffers;
			std::vector<std::shared_ptr<file_buffer>> new_buffers;

			for (auto fb : buffers) 
			{
				if (fb->start <= _start and fb->stop >= _stop)
					return fb;
				else if 
					((fb->start <= _start and fb->stop >= _start) or
					(fb->start <= _stop and fb->stop >= _stop) or 
					(_start <= fb->start and _stop >= fb->stop))
				{
					if (fb->start < new_start) 
					{
						new_start = fb->start;
					}
					if (fb->stop > new_stop)
					{
						new_stop = fb->stop;
					}
					eaten_buffers.push_back(fb);
				}
				else 
				{
					new_buffers.push_back(fb);
				}
			}

			std::shared_ptr<file_buffer> new_buffer;
			new_buffer = std::make_shared<file_buffer>(new_start, new_stop, false);

			int64_t track_start = new_start;

			io_fence fence;

			if (_feast == buffer_access_type::access_read)
			{
				// read and fill the gaps!
				// but we can't do this, until everything in flight is written to the disk.
				// clear would be an otherwise disaster.
				
				for (auto fb : eaten_buffers)
				{
					if (fb->start > track_start) {
						int64_t read_start = track_start;
						int64_t read_length = fb->start - track_start;
						unsigned char* dest = new_buffer->buff.get_uptr() + read_start - new_buffer->start;
						fp->read(read_start, dest, read_length, &fence);
					}
					track_start = fb->stop;
				}

				if (track_start < new_stop)
				{
					int64_t read_start = track_start;
					int64_t read_length = new_stop - track_start;
					unsigned char* dest = new_buffer->buff.get_uptr() + read_start - new_buffer->start;
					fp->read(read_start, dest, read_length, &fence);
				}
			}

			fence.wait();

			for (auto fb : eaten_buffers)
			{
				if (fb->is_dirty)
					new_buffer->is_dirty = true;
				unsigned char *dest = new_buffer->buff.get_uptr() + fb->start - new_buffer->start;
				unsigned char *src = fb->buff.get_uptr();
				int64_t fb_size = fb->stop - fb->start;
				std::copy(src, src + fb->stop - fb->start, dest);
			}

			new_buffer->last_accessed = date_time::now();

			buffers = new_buffers;
			buffers.push_back(new_buffer);

			return new_buffer;
		}

	public:

		file_block(std::shared_ptr<file> _fp)
		{
			fp = _fp;
		}

		file_block(const file_block& _src) = delete;
		file_block& operator = (const file_block& _src) = delete;

		virtual ~file_block()
		{
			;
		}

		virtual file* get_fp() override
		{
			return fp.get();
		}

		virtual file_command_result write(int64_t _location, void* _buffer, int _buffer_length) override
		{
			scope_lock lockme(buffer_lock);

			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("write length < 0");

			int64_t _location_end = _location + _buffer_length;
			std::shared_ptr<file_buffer> fb;

			if (append_buffer and 
				append_buffer->start <= _location and 
				append_buffer->stop >= _location_end) {
				fb = append_buffer;
			}
			else 
			{
				fb = acquire_buffer(_location, _buffer_length, buffer_access_type::access_write);
			}

			fb->is_dirty = true;
            fb->last_accessed = date_time::now();

			unsigned char* src = (unsigned char*)_buffer;
			unsigned char* dest = (unsigned char*)fb->buff.get_ptr() + _location - fb->start;
			std::copy(src, src + _buffer_length, dest);

			result.buffer = (const char *)_buffer;
			result.bytes_transferred = _buffer_length;
			result.location = _location;
			result.success = true;
			result.result = os_result(0);

			return result;
		}

		virtual file_command_result read(int64_t _location, void* _buffer, int _buffer_length) override
		{
			scope_lock lockme(buffer_lock);

			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("read length < 0");

			int64_t _location_end = _location + _buffer_length;
			std::shared_ptr<file_buffer> fb;

			if (append_buffer and
				append_buffer->start <= _location and
				append_buffer->stop >= _location_end) {
				fb = append_buffer;
			}
			else
			{
				int64_t block_start = (_location / block_size) * block_size;
				int64_t block_end = (_location_end / block_size) * block_size;
				if (_location_end % block_size)
					block_end += block_size;

				int64_t final_length = block_end - block_start;

				fb = acquire_buffer(block_start, final_length, buffer_access_type::access_read);
			}

			fb->last_accessed = date_time::now();

			unsigned char* src = (unsigned char*)fb->buff.get_ptr() + _location - fb->start;
			unsigned char* dest = (unsigned char*)_buffer;
			std::copy(src, src + _buffer_length, dest);

			result.buffer = (const char*)_buffer;
			result.bytes_transferred = _buffer_length;
			result.location = _location;
			result.success = true;
			result.result = os_result(0);

			return result;
		}

		virtual relative_ptr_type allocate_space(int64_t _size, int64_t* _actual_size) override
		{

			scope_lock lockme(buffer_lock);

			if (_size < 0)
				throw std::logic_error("allocate_space < 0");

			*_actual_size = _size;

			return add(_size);
		}

		virtual void free_space(int64_t _location) override
		{
			;
		}

		virtual int64_t add(int _bytes_to_add) override
		{
			scope_lock lockme(buffer_lock);

			if (_bytes_to_add < 0)
				throw std::logic_error("add < 0");

			file_command_result result;
			int64_t location = -1;

			int64_t buffer_size = append_size;

			while (buffer_size < _bytes_to_add)
				buffer_size *= 2;

			if (!append_buffer) {

				location = fp->add(buffer_size);
				append_buffer = std::make_shared<file_buffer>(location, location + buffer_size, true);
			}

			location = append_buffer->add( _bytes_to_add );

			if (location < 0)
			{
				location = fp->add(buffer_size);
				if (location != append_buffer->stop) {
					throw std::logic_error("Someone else grew the file so I have no clue what to do now.");
				}
				append_buffer->grow(buffer_size);
				location = append_buffer->add(_bytes_to_add);
			}

			return location;
		}

		virtual file_command_result append(void* _buffer, int _buffer_length) override
		{
			scope_lock lockme(buffer_lock);

			int64_t file_position = add(_buffer_length);

			auto fc = write(file_position, _buffer, _buffer_length);

			return fc;
		}

		virtual bool is_free_capable()
		{
			return false;
		}

		virtual void commit() override
		{
		
			commit_job_master cjm;

			{
				scope_lock lockme(buffer_lock);

				if (append_buffer and append_buffer->is_dirty) {
					dirty_buffers.push_back(append_buffer);
					append_buffer->is_dirty = false;
				}

				std::vector<std::shared_ptr<file_buffer>> buffers_to_keep;

				for (auto bf : buffers) {
					if (bf->is_dirty) {
						dirty_buffers.push_back(bf);
						bf->is_dirty = false;
					}
					date_time expiration = bf->last_accessed + time_span(5, time_models::seconds);
					if (expiration >= date_time::now()) {
						buffers_to_keep.push_back(bf);
					}
				}

				buffers.clear();
				for (auto& bf : buffers_to_keep) {
					buffers.push_back(bf);
				}
			}

			io_fence fence;

			if (dirty_buffers.size() > 0) {
				int i = 0;
				while (i < dirty_buffers.size())
				{
					auto& trans_buff = dirty_buffers[i];
					get_fp()->write(trans_buff->start, trans_buff->buff.get_ptr(), trans_buff->stop - trans_buff->start, &fence);
					i++;
				}
			}
			fence.wait();
		}

		virtual int buffer_count() override
		{
			return buffers.size();
		}

		virtual void clear() override
		{
			scope_lock lockme(buffer_lock);
			append_buffer = nullptr;
			buffers.clear();
		}

		virtual int64_t size() override
		{
			return fp->size();
		}

	};

}

#endif
