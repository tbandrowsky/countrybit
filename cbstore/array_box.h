#pragma once

#include "store_box.h"
#include <ostream>
#include <bit>

namespace countrybit
{
	namespace database
	{
		template <typename item_type, int length_items> struct iarray
		{
			item_type data[length_items];
			uint32_t length;

			iarray() = default;

			iarray(const std::vector<item_type>& src)
			{
				copy(src.data(), src.size());
			}

			iarray& operator = (const std::vector<item_type>& src)
			{
				copy(src.data(), src.size());
			}

			const item_type* value() const
			{
				return &data[0];
			}

			uint16_t size() const
			{
				return length;
			}

			void copy(int srclength, item_type* src)
			{
				int last_char = length_items - 1;

				query_element* d = &data[0];
				length = 0;

				while (length <= last_char && srclength)
				{
					*d = *s;
					length++;
					s++;
					d++;
					srclength--;
				}
			}

		};

		template <typename item_type>
		class array_box
		{

			struct array_box_data 
			{
			public:
				uint32_t length;
				int last_item;
				item_type data[1];
			};

			array_box_data* hdr;

			void copy(int ls, const item_type* s)
			{
				item_type* d = &hdr->data[0];
				int l = 0;

				while (l <= hdr->last_item && l < ls)
				{
					*d = *s;
					l++;
					s++;
					d++;
				}

				if (l <= hdr->last_item)
				{
					hdr->length = l;
				}
				else
				{
					hdr->length = hdr->last_item;
				}
			}

		public:

			array_box() : hdr(nullptr)
			{
			}

			array_box(char* c) : hdr((query_box_data*)c)
			{
			}

			template <typename BOX>
				requires(box<BOX, char>)
			static row_id_type create(BOX* b, int items_length)
			{
				query_box temp;
				auto location = b->pack<char>(sizeof(query_box_data) + items_length);
				temp.hdr = b->unpack<query_box_data>(location);
				temp.hdr->last_item = items_length - 1;
				temp.hdr->length = 0;
				temp.hdr->data[0] = 0;
				return location;
			}

			template <typename BOX>
				requires(box<BOX, char>)
			static array_box get(BOX* b, int location)
			{
				array_box temp;
				temp.hdr = b->unpack<query_box_data>(location);
				return temp;
			}

			static array_box create(char* b, int chars_length)
			{
				query_box temp;
				temp.hdr = (query_box_data*)(b);
				temp.hdr->last_item = chars_length - (sizeof(query_box_data) - 1);
				temp.hdr->length = 0;
				return temp;
			}

			static array_box get(char* b)
			{
				query_box temp;
				temp.hdr = (query_box_data*)(b);
				return temp;
			}

			static int get_box_size(int length)
			{
				return sizeof(query_box_data) + sizeof(query_element) * length;
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

			std::string value() const
			{
				return std::string(c_str());
			}

			item_type& operator(int idx)
			{
				return data() + idx;
			}

			uint16_t size() const
			{
				return hdr->length;
			}
		};


	}
}

