#pragma once

#include <ostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <numeric>

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
			l = c.reserve(s);
			l = c.pack(d, x);
			l = c.pack(&d, x);
//			pd = c.unpack(l);
		};

		class serialized_box;

		class expandable_box
		{
		public:
			virtual serialized_box *expand_check(int _bytes) { return nullptr; }
		};

		class serialized_box 
		{
			int _size;
			int _top;
			expandable_box* owner;
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

			void init(int _length, expandable_box *_owner = nullptr)
			{
				_top = 0;
				_size = _length;
				owner = _owner;
			}

			void adjust(int _length)
			{
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

			serialized_box *expand_check(int _extra_bytes)
			{
				if (!owner) return nullptr;
				return owner->expand_check(_extra_bytes);
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
				if (new_top >= _size)
					return -1;
				T* item = unpack<T>(_top);
				*item = src;
				_top = new_top;
				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			int pack(const T* src, int length)
			{
				size_t sz = sizeof(T) * length;
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top >= _size)
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
			int pack_extracted(const T* base, int start, int stop, bool terminate = true)
			{
				int length = stop - start;
				size_t sz = sizeof(T) * length;
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top >= _size)
					return -1;
				int i = start;
				while (i < stop)
				{
					T* item = unpack<T>(_top);
					*item = base[i];
					i++;
					_top += sizeof(T);
				}
				if (terminate) {
					T temp = {};
					pack(temp);
				}

				return placement;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			int pack_extracted(const T* base, int start, bool terminate = true)
			{
				T defaulto = {};
				size_t placement = _top;
				int i = start;
				while (base[i] != defaulto)
				{
					T* item = unpack<T>(_top);
					*item = base[i];
					i++;
					_top += sizeof(T);
					if (_top >= _size) {
						_top = placement;
						return -1;
					}
				}
				if (terminate) {
					pack(defaulto);
				}

				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, bool terminate = true)
			{
				int l = pack_extracted(base, start, terminate);
				return unpack<char>(l);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, int stop, bool terminate = true)
			{
				int l = pack_extracted(base, start, stop, terminate);
				return unpack<char>(l);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			int pack(T src, int length)
			{
				size_t sz = sizeof(T) * length;
				size_t placement = _top;
				size_t new_top = placement + sz;
				if (new_top >= _size)
					return -1;
				while (_top < new_top)
				{
					T* item = unpack<T>(_top);
					*item = src;
					_top += sizeof(T);
				}
				return placement;
			}

			template <typename T>
			char* place()
			{
				int sz = sizeof(T);
				int l = size() - top();

				if (l < sz) 
				{
					return nullptr;
				}

				char* szn = data();
				_top += sz;
				return szn;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				int loc = -1;
				T dummy = {};
				count--;
				if (count >= 0) {
					loc = pack(dummy);
				}
				while (count) 
				{
					pack(dummy);
					count--;
				}
				return unpack<T>(loc);
			}

			int reserve(int length)
			{
				int sz = length;
				int placement = _top;
				int new_top = placement + sz;
				if (new_top > _size)
					return -1;
				_top = new_top;
				return placement;
			}

			int reserve_all_free()
			{
				int new_top = _size;
				int r = _top;
				_top = new_top;
				return r;
			}

			int free()
			{
				return size() - top();
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
				init();
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

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			int pack_extracted(T* src, int start, int stop, bool terminate = true)
			{
				return get_box()->pack_extracted<T>(src, start, stop, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, bool terminate = true)
			{
				return get_box()->copy(base, start, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, int stop, bool terminate = true)
			{
				return get_box()->copy(base, start, stop, terminate);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				return get_box()->allocate<T>(count);
			}

			template <typename T>
			char* place()
			{
				return get_box()->place<T>();
			}

			int reserve(int length)
			{
				return get_box()->reserve(length);
			}
		};

		class dynamic_box : public expandable_box
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

			void init(int _length, serialized_box *_src = nullptr)
			{
				stuff.resize(_length + sizeof(serialized_box));
				get_box()->init(_length, this);
				if (_src) {
					std::copy(_src->data(), _src->data() + _length, get_box()->data());
				}
			}

			void copy(serialized_box* _src = nullptr)
			{
				stuff.resize(_src->size());
				get_box()->init(_src->size());
				std::copy(_src->data(), _src->data() + _src->size(), get_box()->data());
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

			size_t free()
			{
				return get_box()->size();
			}

			char* data()
			{
				return get_box()->data();
			}

			char* move_ptr(serialized_box* _src, char* _srcp)
			{
				int offset = _srcp - _src->data();
				char* t = data() + offset;
				return t;
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
			int pack_extracted(T* src, int start, int stop, bool terminate = true)
			{
				return get_box()->pack_extracted(src, start, stop, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, bool terminate = true)
			{
				if (!base) return nullptr;
				return get_box()->copy(base, start, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, int stop, bool terminate = true)
			{
				if (!base) return nullptr;
				return get_box()->copy(base, start, stop, terminate);
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
				int pack(T* src, int length)
			{
				return get_box()->pack<T>(src, length);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				return get_box()->allocate<T>(count);
			}

			template <typename T>
			char* place()
			{
				return get_box()->place<T>();
			}

			size_t reserve(int length)
			{
				return get_box()->reserve(length);
			}

			size_t reserve_all_free()
			{
				return get_box()->reserve_all_free();
			}

			virtual serialized_box *expand_check(int _bytes)
			{
				serialized_box *b, temp;
				b = get_box();
				if (_bytes > b->free())
				{
					temp = *b;
					int d = b->size() * 2;
					while ((d - b->top()) < _bytes)
						d *= 2;
					stuff.resize(d + sizeof(serialized_box));
					b = get_box();
					*b = temp;
					b->adjust(d);
					return b;
				}
				return nullptr;
			}
		};

		template <typename T>
		requires (std::is_standard_layout<T>::value)
		class boxed 
		{
		protected:

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

			void set_data(const boxed& _src)
			{
				data = _src.data;
			}

			void set_value(const T& _src)
			{
				*data = _src;
			}

			T get_value() const { return *data; }
			T* get_data() { return data; }
			T& get_data_ref() { return *data; }
			const T& get_data_ref() const { return *data; }
		};

		template<typename T> std::ostream& operator <<(std::ostream& output, boxed<T>& src)
		{
			output << (T)src;
			return output;
		}

		using basic_int8_box = boxed<int8_t>;
		using basic_int16_box = boxed<int16_t>;
		using basic_int32_box = boxed<int32_t>;
		using basic_int64_box = boxed<int64_t>;
		using basic_float_box = boxed<float>;
		using basic_double_box = boxed<double>;
		using basic_time_box = boxed<time_t>;
		using basic_collection_id_box = boxed<collection_id_type>;
		using basic_object_id_box = boxed<object_id_type>;

		bool box_tests();
	}
}

