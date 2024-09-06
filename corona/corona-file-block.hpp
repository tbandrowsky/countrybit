#pragma once

#ifndef CORONA_FILE_BLOCK
#define CORONA_FILE_BLOCK

namespace corona
{

	enum feast_types {
		feast_read = 1,
		feast_write = 2,
		feast_add = 3
	};

	class file_buffer
	{
	public:
		int64_t start;
		int64_t stop;
		int64_t top;
		buffer  buff;
		bool is_append;
		bool is_dirty;

		file_buffer() 
		{
			start = stop = top = {};
			is_append = false;
			is_dirty = false;
		}

		file_buffer(int64_t _start, int64_t _stop, bool _is_append = false)
		{
			start = _start;
			stop = _stop;
			top = _is_append ? start : stop;
			is_append = _is_append;
			buff.init(stop - start);
		}

		file_buffer(const file_buffer& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			is_append = _src.is_append;
			is_dirty = _src.is_dirty;
			buff = _src.buff;
		}

		file_buffer(file_buffer&& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			is_append = _src.is_append;
			is_dirty = _src.is_dirty;
			buff = std::move(_src.buff);
		}

		file_buffer& operator =(file_buffer&& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			buff = std::move(_src.buff);
			is_append = _src.is_append;
			is_dirty = _src.is_dirty;
			return *this;
		}

		file_buffer& operator =(const file_buffer& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			buff = _src.buff;
			is_append = _src.is_append;
			is_dirty = _src.is_dirty;
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

			buff = new_buff; // TODO, use move here

			start = new_start;
			stop = new_stop;
		}

		bool use(std::shared_ptr<file> _fp, int64_t _location, int64_t _size)
		{
			bool can_use = false;

			int64_t start_point, end_point;

			start_point = _location;
			end_point = _location + _size;

			if (end_point < start_point)
				return false;

			if (start_point >= start and end_point <= stop) 
			{
				can_use = true;
			}
			else if (start_point >= start and end_point > stop)
			{
				buffer new_buff;

				int64_t new_start = start;
				int64_t new_stop = end_point;

				new_buff.init(new_stop - new_start);

				unsigned char* s = buff.get_uptr();
				unsigned char* p = new_buff.get_uptr();

				std::copy(s, s + buff.get_size(), p);
				if (_fp) {
					_fp->read(stop, p, new_stop - stop);
				}
				buff = new_buff; // TODO, use move here

				start = new_start;
				stop = new_stop;
			}
			else if (start_point < start and end_point <= stop)
			{
				can_use = true;
				buffer new_buff;

				int64_t new_start = start_point;
				int64_t new_stop = stop;

				new_buff.init(new_stop - new_start);

				int64_t delta = start - new_start;

				unsigned char* s = buff.get_uptr();
				unsigned char* p = new_buff.get_uptr() + delta;

				std::copy(s, s + buff.get_size(), p);
				if (_fp) {
					_fp->read(new_start, p, delta);
				}
				buff = new_buff; // TODO, use move here

				start = new_start;
				stop = new_stop;
			}
			else if (start_point < start and end_point < start_point)
			{
				can_use = false;
			}

			return can_use;
		}

	};

	class file_block
	{
		std::vector<std::shared_ptr<file_buffer>> buffers;
		std::shared_ptr<file> fp;

		struct feast_result
		{


		};

		std::shared_ptr<file_buffer> feast(int64_t _start, int64_t _length, feast_types _feast)
		{
			int64_t _stop = _start + _length;

			int64_t new_start = _start,
					new_stop = _stop;

			std::vector<std::shared_ptr<file_buffer>> eaten_buffers;
			std::vector<std::shared_ptr<file_buffer>> new_buffers;

			for (auto fb : buffers) 
			{
				if ((fb->start <= _start and fb->stop >= _start) or
					(fb->start <= _stop and fb->stop >= _stop))
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

			if (_feast == feast_types::feast_add) 
			{
				new_buffer = std::make_shared<file_buffer>(new_start, new_stop, true);
			}
			else 
			{
				new_buffer = std::make_shared<file_buffer>(new_start, new_stop, false);
			}

			int64_t track_start = new_start;

			if (_feast == feast_types::feast_read)
			{
				// read and fill the gaps!
				for (auto fb : eaten_buffers)
				{
					if (fb->start > track_start) {
						int64_t read_start = track_start;
						int64_t read_length = fb->start - track_start;
						unsigned char* dest = new_buffer->buff.get_uptr() + read_start - new_buffer->start;
						fp->read(read_start, dest, read_length);
					}
					track_start = fb->stop;
				}

				if (track_start < new_stop)
				{
					int64_t read_start = track_start;
					int64_t read_length = new_stop - track_start;
					unsigned char* dest = new_buffer->buff.get_uptr() + read_start - new_buffer->start;
					fp->read(read_start, dest, read_length);
				}
			}

			for (auto fb : eaten_buffers)
			{
				if (fb->is_dirty)
					new_buffer->is_dirty = true;
				unsigned char *dest = new_buffer->buff.get_uptr() + fb->start - new_buffer->start;
				unsigned char *src = fb->buff.get_uptr();
				int64_t fb_size = fb->stop - fb->start;
				std::copy(src, src + fb->stop - fb->start, dest);
			}

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

		file_command_result write(int64_t _location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("write length < 0");

			auto fb = feast(_location, _buffer_length, feast_types::feast_write);

			fb->is_dirty = true;

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

		file_command_result read(int64_t _location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("read length < 0");

			auto fb = feast(_location, _buffer_length, feast_types::feast_read);

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

		virtual relative_ptr_type allocate_space(int64_t _size)
		{
			if (_size < 0)
				throw std::logic_error("allocate_space < 0");

			return add(_size);
		}

		virtual void free_space(int64_t _location)
		{
			;
		}

		int64_t add(int _bytes_to_add)
		{

			if (_bytes_to_add < 0)
				throw std::logic_error("add < 0");

			file_command_result result;
			int64_t location = -1;

			auto found = std::find_if(buffers.begin(), buffers.end(), [this, _bytes_to_add](file_buffer& fb) {
				return fb.is_append;
				});

			if (found != std::end(buffers))
			{
				std::shared_ptr<file_buffer> fb = *found;
				location = fb->add( _bytes_to_add );
			}

			if (location < 0)
			{
				int64_t buffer_size = _bytes_to_add;
				if (buffer_size < 65536)
					buffer_size = 65536;

				location = fp->add(buffer_size);

				feast(location, buffer_size, feast_types::feast_add);
			}

			return location;
		}

		file_command_result append(void* _buffer, int _buffer_length)
		{
			int64_t file_position = add(_buffer_length);

			auto found = std::find_if(buffers.begin(), buffers.end(), [this, file_position, _buffer_length](file_buffer& fb) {
				return fb.use(nullptr, file_position, _buffer_length);
				});

			if (found == std::end(buffers))
			{
				system_monitoring_interface::global_mon->log_warning("Tried to append but couldn't find the buffer after add", __FILE__, __LINE__);
			}

			auto fc = write(file_position, _buffer, _buffer_length);

			return fc;
		}

		void commit()
		{
			for (auto& buff : buffers) {
				if (buff->is_dirty) {
					buff->is_dirty = false;
					fp->write(buff->start, buff->buff.get_ptr(), buff->stop - buff->start);
				}
			}
		}

		void clear()
		{
			buffers.clear();
		}

		int64_t size()
		{
			return fp->size();
		}
	};

}

#endif
