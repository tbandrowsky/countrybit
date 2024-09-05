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
			buff.init(stop - start);
		}

		file_buffer(file_buffer& _src)
		{
			start = _src.start;
			stop = _src.stop;
			top = _src.top;
			is_append = _src.is_append;
			is_dirty = _src.is_dirty;
			buff = _src.buff;
		}

		file_buffer& operator =(file_buffer& _src)
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
			if (top + _length > stop)
				return -1;
			int64_t old_top = top;
			top += _length;
			return old_top;
		}

		bool use(file *_fp, int64_t _location, int64_t _size)
		{
			bool can_use = false;

			int64_t start_point, end_point;

			start_point = _location;
			end_point = _location + _size;

			if (start_point >= start and end_point <= stop) 
			{
				can_use = true;
				buffer new_buff;

				int64_t new_start = start;
				int64_t new_stop = _location + _size;

				if (new_stop > stop) 
				{

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
			}
			else if (end_point >= start and end_point <= stop)
			{
				can_use = true;
				buffer new_buff;

				int64_t new_start = _location;
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

			return can_use;
		}
	};

	class file_block
	{
		std::vector<file_buffer> buffers;
		file* fp;

	public:

		file_block(file *_fp)
		{
			fp = _fp;
		}

		file_block(const file_block& _src) = delete;
		file_block& operator = (const file_block& _src) = delete;

		file_command_result write(uint64_t location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

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

		file_command_result read(uint64_t location, void* _buffer, int _buffer_length)
		{
			file_command_result result;

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
				int64_t end_point;

				end_point = location + _buffer_length;

				file_buffer fb(location, end_point, false);

				unsigned char* src = (unsigned char*)fb.buff.get_ptr() + location - fb.start;
				unsigned char* dest = (unsigned char*)_buffer;
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

		int64_t add(int _buffer_length)
		{
			file_command_result result;
			int64_t location = 0;

			auto found = std::find_if(buffers.begin(), buffers.end(), [this, _buffer_length](file_buffer& fb) {
				return fb.is_append;
				});

			if (found != std::end(buffers))
			{
				file_buffer& fb = *found;

				location = fb.add( _buffer_length);
			}

			if (location == 0)
			{
				if (_buffer_length < 65536)
					_buffer_length = 65536;

				int64_t location = fp->add(_buffer_length);
				int64_t end_point;

				end_point = location + _buffer_length;

				file_buffer fb(location, end_point, end_point);
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

		void begin(uint64_t location, int64_t _size)
		{
			file_buffer fb(location, location + _size, false);
			fp->read(location, fb.buff.get_ptr(), _size);
			buffers.push_back(fb);
		}

		void commit()
		{
			for (auto& buff : buffers) {
				if (buff.is_dirty) {
					fp->write(buff.start, buff.buff.get_ptr(), buff.stop - buff.start);
				}
			}
		}

		int64_t size()
		{
			return fp->size();
		}
	};

}

#endif
