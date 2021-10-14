#pragma once

#include <vector>
#include "constants.h"

namespace countrybit
{
	namespace database
	{

		template <typename box_class>
		concept box_data = requires(box_class c, char* b, int x) {
			x = c.size();
			b = c.data();
		};

		template <typename box_class, typename data>
		concept box = requires(box_class c, data d, data * pd, row_id_type l, size_t s, int x, char* b) {
			s = c.size();
			x = c.top();
			b = c.data();
			l = c.pack(d);
			l = c.pack(d, x);
			l = c.pack(&d, x);
//			pd = c.unpack(l);
		};

		class serialized_box 
		{
			int _size;
			int _top;
			char _data[1];

		public:

			serialized_box()
			{
				;
			}

			template <typename bx> 
			requires (box_data<bx>)
			serialized_box(const bx& _src)
			{
				int new_size = _src.size();
				if (_size < new_size)
					throw std::invalid_argument("target box too small");
				_top = _src.top();
				_size = new_size;
				memcpy(_data, _src.data(), new_size);
			}

			template <typename bx>
			requires (box_data<bx>)
			serialized_box operator = (const bx& _src)
			{
				int new_size = _src.size();
				if (_size < new_size)
					throw std::invalid_argument("target box too small");
				_top = _src.top();
				_size = new_size;
				memcpy(_data, _src.data(), new_size);
				return *this;
			}

			void init(int _length)
			{
				_top = 0;
				_size = _length;
			}

			int top()
			{
				return _top;
			}

			int size()
			{
				return _size;
			}

			char* data()
			{
				return &_data[0];
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* unpack(int offset, T* dummy = nullptr)
			{
				T* temptress = (T*)&_data[offset];
				return temptress;
			}

			template <typename T> 
			requires (std::is_standard_layout<T>::value)
			int pack(T& src)
			{
				size_t sz = sizeof(T);
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top > _size)
					return -1;
				T* item = *unpack<T>(_top);
				*item = src;
				_top = new_top;
				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			int pack(T* src, int length)
			{
				size_t sz = sizeof(T) * length;
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top > _size)
					return -1;
				while (_top < new_top) 
				{
					T *item = unpack<T>(_top);
					*item = *src;
					src++;
					_top += sizeof(T);
				}
				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T src, int length)
			{
				size_t sz = sizeof(T) * length;
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top > _size)
					return -1;
				while (_top < new_top)
				{
					T* item = unpack<T>(_top);
					*item = src;
					_top += sizeof(T);
				}
				return placement;
			}

		};

		template <size_t bytes> 
		class static_box 
		{
			static const int length = bytes;
			char stuff[length];

		public:

			static_box()
			{
			}

			inline serialized_box* get_box()
			{
				return (serialized_box*)&stuff;
			}

			template <typename bx>
			requires (box_data<bx>)
			static_box(const bx& _src)
			{
				int new_size = _src.size();
				if (length < new_size)
					throw std::invalid_argument("target box too small");
				memcpy(data(), _src.data(), new_size);
			}

			template <typename bx>
			requires (box_data<bx>)
			static_box operator = (const bx& _src)
			{
				int new_size = _src.size();
				if (length < new_size)
					throw std::invalid_argument("target box too small");
				memcpy(data(), _src.data(), new_size);
				return *this;
			}

			void init()
			{
				get_box()->init(length - sizeof(serialized_box));
			}

			size_t top()
			{
				return get_box()->top();
			}

			size_t size()
			{
				return get_box()->size();
			}

			char* data()
			{
				return get_box()->data();
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				T* unpack(int offset)
			{
				return get_box()->unpack<T>(offset);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T& src)
			{
				return get_box()->pack<T>(src);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T& src, int length)
			{
				return get_box()->pack<T>(src, length);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T* src, int length)
			{
				return get_box()->pack<T>(src, length);
			}
		};


		class dynamic_box
		{
			std::vector<char> stuff;

		public:

			dynamic_box() 
			{
			}

			inline serialized_box* get_box()
			{
				return (serialized_box*)stuff.data();
			}

			void init(int _length)
			{
				stuff.resize(_length + sizeof(serialized_box));
				get_box()->init(_length);
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box(const bx& _src)
			{
				int new_size = _src.size();
				init(new_size);
				memcpy(data(), _src.data(), new_size);
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box operator = (const bx& _src)
			{
				int new_size = _src.size();
				init(new_size);
				memcpy(data(), _src.data(), new_size);
				return *this;
			}

			size_t top()
			{
				return get_box()->top();
			}

			size_t size()
			{
				return get_box()->size();
			}

			char* data()
			{
				return get_box()->data();
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				T* unpack(int offset)
			{
				return get_box()->unpack<T>(offset);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T& src)
			{
				return get_box()->pack<T>(src);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T& src, int length)
			{
				return get_box()->pack<T>(src, length);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T* src, int length)
			{
				return get_box()->pack<T>(src, length);
			}
		};

		template <typename T>
		requires (std::is_standard_layout<T>::value)
		class boxed 
		{
			T* data;

		public:

			boxed(char* _data)
			{
				data = (T*)_data;
			}

			boxed(const boxed& _src)
			{
				data = _src.data;
			}

			boxed operator = (const boxed& _src)
			{
				data = _src.data;
				return *this;
			}

			operator T&() { return *data; }
			T* get_data() { return data; }
		};

		template<typename T> int compare(const boxed<T>& a, const boxed<T>& b)
		{
			return (T)a <=> (T)b;
		}

		template<typename T> int operator<(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const boxed<T>& a, const boxed<T>& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> int compare(const boxed<T>& a, T& b)
		{
			return compare(a, boxed(b));
		}

		template<typename T> int operator<(const boxed<T>& a, T& b)
		{
			return compare(a, b) < 0;
		}

		template<typename T> int operator>(const boxed<T>& a, T& b)
		{
			return compare(a, b) > 0;
		}

		template<typename T> int operator>=(const boxed<T>& a, T& b)
		{
			return compare(a, b) >= 0;
		}

		template<typename T> int operator<=(const boxed<T>& a, T& b)
		{
			return compare(a, b) <= 0;
		}

		template<typename T> int operator==(const boxed<T>& a, T& b)
		{
			return compare(a, b) == 0;
		}

		template<typename T> int operator!=(const boxed<T>& a, T& b)
		{
			return compare(a, b) != 0;
		}

		template<typename T> std::ostream& operator <<(std::ostream& output, const boxed<T>& src)
		{
			output << (T)src;
			return output;
		}

		using int8_box = boxed<int8_t>;
		using int16_box = boxed<int16_t>;
		using int32_box = boxed<int32_t>;
		using int64_box = boxed<int64_t>;
		using float_box = boxed<float>;
		using double_box = boxed<double>;
		using time_box = boxed<time_t>;

	}
}

