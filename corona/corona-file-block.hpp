#pragma once

#ifndef CORONA_FILE_BLOCK
#define CORONA_FILE_BLOCK

namespace corona
{

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

		void eat(file_buffer* _fb)
		{
			;
		}

	};

	class file_block
	{
		std::vector<file_buffer> buffers;
		std::shared_ptr<file> fp;

		void merge_buffers()
		{
			std::sort(buffers.begin(), buffers.end(), [](file_buffer& fb1, file_buffer& fb2) {
				return fb1.start < fb2.start;
				});

			bool must_merge = false;
			int64_t last_stop = 0;
			for (auto fb : buffers) {
				if (fb.start < last_stop) {
					must_merge = true;
					break;
				}
			}

			if (must_merge) 
			{
				std::vector<file_buffer> new_buffers;
				file_buffer buffer;

				auto bi = buffers.begin();
				auto b0 = bi;
				bi++;
				while (bi != std::end(buffers))
				{
					auto& bfi = *bi;
					auto& bf0 = *b0;

					if (bfi.start < bf0.stop) {
						
					}
				}

			}
		}

	public:

		file_block(std::shared_ptr<file> _fp)
		{
			fp = _fp;
		}

		file_block(const file_block& _src) = delete;
		file_block& operator = (const file_block& _src) = delete;

		file_command_result write(int64_t location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("write length < 0");

			auto found = std::find_if(buffers.begin(), buffers.end(), [this, location, _buffer_length](file_buffer& fb) {
				return fb.use(nullptr, location, _buffer_length);
				});

			if (found != std::end(buffers)) 
			{
				file_buffer& fb = *found;

				fb.is_dirty = true;

				unsigned char* src = (unsigned char*)_buffer;
				unsigned char* dest = (unsigned char*)fb.buff.get_ptr() + location - fb.start;
				std::copy(src, src + _buffer_length, dest);

				result.buffer = (const char *)_buffer;
				result.bytes_transferred = _buffer_length;
				result.location = location;
				result.success = true;
				result.result = os_result(0);
			}
			else 
			{
				int64_t end_point;

				end_point = location + _buffer_length;

				file_buffer fb(location, end_point, false);

				fb.is_dirty = true;

				unsigned char* src = (unsigned char*)_buffer;
				unsigned char* dest = (unsigned char*)fb.buff.get_ptr() + location - fb.start;
				std::copy(src, src + _buffer_length, dest);

				result.buffer = (const char*)_buffer;
				result.bytes_transferred = _buffer_length;
				result.location = location;
				result.success = true;
				result.result = os_result(0);

				buffers.push_back(fb);
			}

			return result;
		}

		file_command_result read(int64_t location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

			if (_buffer_length < 0)
				throw std::logic_error("read length < 0");

			auto found = std::find_if(buffers.begin(), buffers.end(), [this, location, _buffer_length](file_buffer& fb) {
				return fb.use(fp, location, _buffer_length);
				});

			if (found != std::end(buffers))
			{
				file_buffer& fb = *found;

				unsigned char* src = (unsigned char*)fb.buff.get_ptr() + location - fb.start;
				unsigned char* dest = (unsigned char*)_buffer;
				std::copy(src, src + _buffer_length, dest);

				result.buffer = (const char*)_buffer;
				result.bytes_transferred = _buffer_length;
				result.location = location;
				result.success = true;
				result.result = os_result(0);
			}
			else
			{

				result = fp->read(location, _buffer, _buffer_length);

				if (result.success) {

					int64_t end_point;

					end_point = location + _buffer_length;

					file_buffer fb(location, end_point, false);

					unsigned char* dest = (unsigned char*)fb.buff.get_ptr() + location - fb.start;
					unsigned char* src = (unsigned char*)_buffer;
					std::copy(src, src + _buffer_length, dest);

					result.buffer = (const char*)_buffer;
					result.bytes_transferred = _buffer_length;
					result.location = location;
					result.success = true;
					result.result = os_result(0);

					buffers.push_back(fb);
				}
				else {
					system_monitoring_interface::global_mon->log_warning("Physical read failed", __FILE__, __LINE__);
				}
			}

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
				file_buffer& fb = *found;

				location = fb.add( _bytes_to_add );

				if (location < 0) {
					int64_t buffer_size = _bytes_to_add;
					if (buffer_size < 65536)
						buffer_size = 65536;

					location = fp->add(buffer_size);

					int64_t end_point;

					end_point = location + buffer_size;

					if (location == fb.stop) 
					{
						fb.grow(buffer_size);
					}
					else 
					{
						file_buffer fb(location, end_point, true);
						location = fb.add(_bytes_to_add);
						buffers.push_back(fb);
					}
				}
			}

			if (location < 0)
			{
				int64_t buffer_size = _bytes_to_add;
				if (buffer_size < 65536)
					buffer_size = 65536;

				location = fp->add(buffer_size);
				int64_t end_point;

				end_point = location + buffer_size;

				file_buffer fb(location, end_point, true);
				location = fb.add(_bytes_to_add);
				buffers.push_back(fb);
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

		void begin(int64_t location, int64_t _size)
		{
			file_buffer fb(location, location + _size, false);
			fp->read(location, fb.buff.get_ptr(), _size);
			buffers.push_back(fb);
		}

		void commit()
		{
			for (auto& buff : buffers) {
				if (buff.is_dirty) {
					buff.is_dirty = false;
					fp->write(buff.start, buff.buff.get_ptr(), buff.stop - buff.start);
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
