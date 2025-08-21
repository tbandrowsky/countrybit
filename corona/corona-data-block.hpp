#pragma once

#ifndef CORONA_DATABLOCK_HPP
#define CORONA_DATABLOCK_HPP

namespace corona
{

	const int ENABLE_JSON_LOGGING = 0;

	// data blocks
	class data_block;

	// poco nodes
	template <typename T> class poco_node;

	struct allocation_index
	{
		int		index;
		int64_t size;
	};

	// root data block.  Everything is a block.  It's almost like minecraft, except, it makes money
	// and thus far, I do not.
	class data_block
	{
	public:

		data_block_struct				header;

		data_block()
		{
			header = {};
			header.block_location = null_row;
			header.data_location = null_row;
		}

		data_block(const data_block& _src) = default;
		data_block& operator = (const data_block& _src) = default;

		virtual char* before_write(int32_t* _size) = 0;
		virtual char* before_write(int _offset, int _size)
		{
			return nullptr;
		}

		virtual void after_write(char* _buff)
		{
			;
		}


		virtual char* before_read(int32_t _size) = 0;
		virtual void after_read(char* _bytes) = 0;

		virtual void finished_io(char* _bytes) = 0;

		relative_ptr_type read(file_block* _file, relative_ptr_type location)
		{
			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_start("block", "read block", start_time, __FILE__, __LINE__);
			}

			file_command_result header_result = _file->read(location, &header, sizeof(header));

			if (header_result.success)
			{
				char* bytes = before_read(header.data_size);
				file_command_result data_result = _file->read(header.data_location, bytes, header.data_size);

				if (data_result.success)
				{
					after_read(bytes);
					finished_io(bytes);
					if (ENABLE_JSON_LOGGING) {
						system_monitoring_interface::active_mon->log_block_stop("block", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					}
					return header_result.location; // want to make this 0 or -1 if error
				}
				else
				{
					finished_io(bytes);
					if (ENABLE_JSON_LOGGING) {
						system_monitoring_interface::active_mon->log_function_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
			}

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_stop("block", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return -1i64;
		}

		relative_ptr_type write_piece(file_block* _file, int _offset, int _size)
		{
			if (header.block_location < 0)
			{
				throw std::invalid_argument("cannot append a partial write of a block");
			}

			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_start("block", "write piece", start_time, __FILE__, __LINE__);
			}
			char* bytes = before_write(_offset, _size);

			file_command_result data_result = _file->write(header.data_location + _offset, bytes, _size);

			if (data_result.success)
			{
				after_write(bytes);
				finished_io(bytes);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_block_stop("block", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				return data_result.location;
			}
			else {
				finished_io(bytes);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_block_stop("block", "write failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
			return -1i64;
		}

		relative_ptr_type write(file_block* _file)
		{

			if (header.block_location < 0)
			{
				throw std::invalid_argument("use append for new blocks");
			}

			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_start("block", "write block", start_time, __FILE__, __LINE__);
			}

			int32_t size;
			char* bytes = before_write(&size);

			if (size > header.data_capacity)
			{
				// the header.data_length is the max size of the block.  Since there's stuff past that in the file, then,
				// there's not going to be a way we can write this, so we must have another block.
				_file->free_space(header.data_location);
				header.data_size = size;
				header.data_capacity = 0;
				int64_t actual_size;
				header.data_location = _file->allocate_space(size, &actual_size);
				header.data_capacity = actual_size;
			}
			else
			{
				header.data_size = size;
			}

			file_command_result data_result = _file->write(header.data_location, bytes, size);

			if (data_result.success)
			{
				file_command_result header_result = _file->write(header.block_location, &header, sizeof(header));
				after_write(bytes);
				finished_io(bytes);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_block_stop("block", "write complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				return header_result.location;
			}
			else
			{
				finished_io(bytes);
				if (ENABLE_JSON_LOGGING) {
					system_monitoring_interface::active_mon->log_block_stop("block", "write failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
			return -1i64;
		}

		relative_ptr_type append(file_block* _file)
		{

			int32_t size;
			char* bytes;
			bytes = before_write(&size);

			date_time start_time = date_time::now();
			timer tx;

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_start("block", "append", start_time, __FILE__, __LINE__);
			}

			int64_t actual_size = 0;
			header.block_location = _file->allocate_space(sizeof(header), &actual_size);
			header.data_size = size;
			header.data_location = _file->allocate_space(header.data_size, &actual_size);
			header.data_capacity = actual_size;

			if (header.block_location < 0 or header.data_location < 0)
				return -1;

			auto hdr_status = _file->write(header.block_location, &header, sizeof(header));
			auto data_status = _file->write(header.data_location, bytes, size);

			if (ENABLE_JSON_LOGGING) {
				system_monitoring_interface::active_mon->log_block_stop("block", "append", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			after_write(bytes);
			finished_io(bytes);

			if (not (hdr_status.success and data_status.success)) {
				return -1;
			}

			return header.block_location;
		}

		void erase(file_block* _fb)
		{
			_fb->free_space(header.data_location);
			_fb->free_space(header.block_location);
		}

		int64_t size()
		{
			return sizeof(header) + header.data_size;
		}
	};

	class string_block : public data_block
	{
	public:

		std::string data;

		string_block()
		{
		}

		bool is_empty()
		{
			return data.empty();
		}

		void clear()
		{
			data.clear();
		}

		virtual char* before_read(int32_t size) override
		{
			data.resize(size, 0);
			return (char*)data.c_str();
		}

		virtual void after_read(char* _bytes) override
		{
			;
		}

		virtual char* before_write(int32_t* _size) override
		{
			*_size = strlen(data.c_str());
			return (char*)data.c_str();
		}

		virtual void after_write(char* _bytes) override
		{
			;
		}

		virtual void finished_io(char* _bytes) override
		{
			;
		}

	};

	class json_data_block : public data_block
	{
	public:

		json							data;
		std::string						bytes;

		json_data_block()
		{
		}

		bool is_empty()
		{
			return data.empty();
		}

		void clear()
		{
			json_parser jp;
			data = jp.create_object();
		}

		virtual char* before_read(int32_t _size)  override
		{
			bytes.resize(_size);
			return (char*)bytes.c_str();
		}

		virtual void after_read(char* _bytes) override
		{
			const char* contents = _bytes;
			if (contents) {
				json_parser jp;
				if (*contents == '[') {
					data = jp.parse_array(contents);
				}
				else {
					data = jp.parse_object(contents);
				}
			}
		}

		virtual void finished_io(char* _bytes) override
		{
			;
		}

		virtual char* before_write(int32_t* _size) override
		{
			std::stringstream buff;

			data.serialize(buff);
			bytes = buff.str();
			*_size = bytes.size();
			return (char*)bytes.c_str();
		}

		virtual void after_write(char* _t) override
		{

		}

	};


	template <typename poco_type> class poco_node : public data_block
	{
	public:

		poco_node()
		{
			clear();
		}

		void clear()
		{
			data = {};
		}

		poco_type				data;

		virtual char* before_read(int32_t size) override
		{
			char* io_bytes = (char*)&data;
			return io_bytes;
		}

		virtual void after_read(char* _bytes) override
		{
		}


		virtual char* before_write(int32_t* _size) override
		{
			*_size = sizeof(data);
			char* io_bytes = (char*)&data;
			return io_bytes;
		}

		virtual char* before_write(int _offset, int _size)
		{
			return (char*)(&data) + _offset;
		}

		virtual void after_write(char* _bytes) override
		{
			;
		}

		virtual void finished_io(char* _bytes)  override
		{

		}

		poco_node& operator = (const poco_type& _src)
		{
			data = _src;
			return *this;
		}

	};
}

#endif
