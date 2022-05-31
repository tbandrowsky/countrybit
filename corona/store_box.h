#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <numeric>

#include "constants.h"

namespace corona
{
	namespace database
	{

		template <typename box_class>
		concept box_data = requires(box_class c, char* b, int x) {
			x = c.size();
			b = c.data();
		};

		template <typename box_class, typename data>
		concept box = requires(box_class c, data d, data * pd, relative_ptr_type l, corona_size_t s, int x, char* b) {
			s = c.size();
			x = c.top();
			b = c.data();
			l = c.pack(d);
			l = c.reserve(s);
			l = c.pack(d, x);
			l = c.pack(&d, x);
//			pd = c.unpack(l);
		};

		class serialized_box 
		{
			block_id		 _box_id;
			corona_size_t _size;
			corona_size_t _top;
			char _data[1];

		public:

			serialized_box()
			{
				_box_id = block_id::box_id();
			}

			template <typename bx> 
			requires (box_data<bx>)
			serialized_box(const bx& _src)
			{
				int64_t new_size = _src.size();
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
				int64_t new_size = _src.size();
				if (_size < new_size)
					throw std::invalid_argument("target box too small");
				_top = _src.top();
				_size = new_size;
				memcpy(_data, _src.data(), new_size);
				return *this;
			}

			void init(corona_size_t _length)
			{
				_top = 0;
				_size = _length;
				_box_id = block_id::box_id();
			}

			void adjust(corona_size_t _length)
			{
				_size = _length;
			}

			relative_ptr_type top()
			{
				return _top;
			}

			corona_size_t size()
			{
				return _size;
			}

			char* data()
			{
				return &_data[0];
			}

			template <typename T>
			T* pack_start(int length, relative_ptr_type& dest)
			{
				relative_ptr_type alignment = sizeof(T);

				if (sizeof(T) < 8)
				{
					alignment = sizeof(T);
				}
				else
				{
					alignment = 8;
				}

				relative_ptr_type start = _top + ((alignment - _top % alignment) % alignment);
				relative_ptr_type stop = start + sizeof(T) * length;

//				std::cout << "pack:" << start << " " << stop << " " << _size << std::endl;

				if (stop > _size) 
				{
					dest = null_row;
					return nullptr;
				}

				T *destptr = (T*)(data() + start);
				_top = stop;
				dest = start;

				return destptr;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* unpack(relative_ptr_type offset, T* dummy = nullptr)
			{
				if (offset == null_row) {
					return nullptr;
				}
				T* item = (T*)&_data[offset];
				return item;
			}

			template <typename T> 
			requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack(T& src)
			{
				relative_ptr_type placement;
				T* item = pack_start<T>(1, placement);
				if (!item) return placement;
				*item = src;
				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack(const T* src, int length)
			{
				relative_ptr_type placement;
				T* item = pack_start<T>(length, placement);
				if (!item) return placement;
				while (length)
				{
					*item = *src;
					src++;
					item++;
					length--;
				}
				return placement;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type fill(T src, int length)
			{
				relative_ptr_type placement;
				T* item = pack_start<T>(length, placement);
				if (!item) return placement;
				while (length)
				{
					*item = src;
					item++;
					length--;
				}
				return placement;
			}

			template <typename T> 
			requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack_slice(const T* base, int start, int stop, bool terminate = true)
			{
				relative_ptr_type placement;
				T* item = pack_start<T>((stop - start) + 1, placement);
				if (!item) return placement;

				int i = start;
				while (i < stop)
				{
					*item = base[i];
					item++;
					i++;
				}

				if (terminate) 
				{
					T temp = {};
					*item = temp;
				}

				return placement;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack_terminated(const T* base, int start)
			{
				int length = 0;
				T defaulto = {};

				while (base[start + length] != defaulto)
				{
					length++;
				}
				length++;

				relative_ptr_type placement;
				T* item = pack_start<T>(length, placement);
				if (!item) return placement;

				while (length) 
				{
					*item = base[start];
					length--;
					start++;
					item++;
				}

				*item = defaulto;

				return placement;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start)
			{
				corona_size_t l = pack_terminated(base, start);
				return unpack<T>(l);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, int stop, bool terminate = true)
			{
				corona_size_t l = pack_slice(base, start, stop, terminate);
				return unpack<T>(l);
			}

			template <typename T>
			char* place()
			{
				relative_ptr_type placement;
				T* item = pack_start<T>(1, placement);
				return (char *)item;
			}

			template <typename T>
			T* clone(T& source)
			{
				// TODO: the cloned item here needs to have its destructor called,
				// so, we shall have to track this with a list some kind down the road to 
				// use this facility, and ideally create a special box
				relative_ptr_type placement;
				T* item = pack_start<T>(1, placement);
				if (!item) return item;				
				item = new (item) T(source);
				return item;
			}

			template <typename T>
			requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				relative_ptr_type placement;
				T* item = pack_start<T>(count, placement);
				if (!item) return item;

				T* it = item;
				while (count) 
				{
					*it = {};
					count--;
					it++;
				}
				return item;
			}

			relative_ptr_type reserve(corona_size_t length)
			{
				corona_size_t sz = length;
				corona_size_t placement = _top;
				corona_size_t new_top = placement + sz;
				if (new_top > _size)
					return -1;
				_top = new_top;
				return placement;
			}

			relative_ptr_type reserve_all_free()
			{
				corona_size_t new_top = _size;
				corona_size_t r = _top;
				_top = new_top;
				return r;
			}

			corona_size_t free()
			{
				return size() - top();
			}

		};

		class serialized_box_container
		{
		public:
			virtual serialized_box* get_box() { return nullptr; }
			virtual serialized_box* check(corona_size_t _bytes) { return nullptr; }

			relative_ptr_type top()
			{
				return get_box()->top();
			}

			corona_size_t free()
			{
				return get_box()->size();
			}

			corona_size_t size()
			{
				return get_box()->size();
			}

			char* data()
			{
				return get_box()->data();
			}

			char* move_ptr(serialized_box* _src, char* _srcp)
			{
				corona_size_t offset = _srcp - _src->data();
				char* t = data() + offset;
				return t;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* unpack(corona_size_t offset)
			{
				return get_box()->unpack<T>(offset);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack(T& src)
			{
				check(sizeof(T));
				return get_box()->pack<T>(src);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type fill(const T& src, int length)
			{
				check(sizeof(T) * length);
				return get_box()->fill<T>(src, length);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack(const T* src, int length)
			{
				check(sizeof(T) * length);
				return get_box()->pack<T>(src, length);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack_terminated(const T* src, int start, bool terminate = true)
			{
				return get_box()->pack_terminated<T>(src, start, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type pack_slice(const T* src, int start, int stop, bool terminate = true)
			{
				check(sizeof(T) * (stop - start + 1));
				return get_box()->pack_slice<T>(src, start, stop, terminate);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T *copy(const T* src, int start, int stop, bool terminate = true)
			{
				corona_size_t t = pack_slice<T>(src, start, stop, terminate);
				return unpack<T>(t);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start)
			{
				return get_box()->copy(base, start);
			}

			template <typename T>
			T* clone(T& base)
			{
				return get_box()->clone(base);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				check(sizeof(T) * count);
				return get_box()->allocate<T>(count);
			}

			template <typename T>
			char* place()
			{
				check(sizeof(T));
				return get_box()->place<T>();
			}

			relative_ptr_type reserve(corona_size_t length)
			{
				if (!length)
					length = get_box()->free();
				check(length);
				return get_box()->reserve(length);
			}

		};


		template <size_t bytes> 
		class static_box  : public serialized_box_container
		{
			static const int length = bytes;
			char stuff[length];

		public:

			static_box()
			{
				init();
			}

			virtual serialized_box* get_box() { return (serialized_box*)&stuff; }
			virtual serialized_box* check(int _bytes) { return _bytes < get_box()->free() ? get_box() : nullptr; }

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

		};

		class inline_box : public serialized_box_container
		{
			char* stuff;
			int64_t length;

		public:

			inline_box() : stuff(nullptr), length(0)
			{
				
			}

			inline_box(char* _stuff, int _length) : stuff(_stuff), length(_length)
			{
				get_box()->init(length - sizeof(serialized_box));
			}

			inline_box(const inline_box& _src) : stuff(_src.stuff), length(_src.length)
			{

			}

			inline_box& operator =(const inline_box& _src) 
			{
				stuff = _src.stuff;
				length = _src.length;
				return *this;
			}

			virtual serialized_box* get_box() { return (serialized_box*)stuff; }
			virtual serialized_box* check(int _bytes) { return _bytes < get_box()->free() ? get_box() : nullptr; }

		};

		class dynamic_box : public serialized_box_container
		{
			char *stuff;
			corona_size_t stuff_size;
			bool own_the_data;

			void resize(corona_size_t new_size)
			{
				corona_size_t new_stuff_size = new_size + sizeof(serialized_box);
				if (new_stuff_size < stuff_size)
					return;
				char* temp = new char[new_stuff_size];
				if (!temp) {
					throw std::exception("Out of memory");
				}
				if (stuff) {
					memcpy(temp, stuff, stuff_size);
					if (own_the_data) {
						delete[] stuff;
					}
				}
				stuff = temp;
				stuff_size = new_size;
				own_the_data = true;
			}

		public:

			dynamic_box() : stuff(nullptr), stuff_size(0)
			{
				own_the_data = true;
			}

			~dynamic_box()
			{
				if (own_the_data && stuff) 
				{
					delete[] stuff;
					stuff = nullptr;
				}
			}

			dynamic_box& operator = (const dynamic_box& _src)
			{
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = false;
				return *this;
			}

			dynamic_box& operator = (dynamic_box&& _src)
			{
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = true;
				_src.stuff = nullptr;
				_src.stuff_size = 0;
				_src.own_the_data = false;
				return *this;
			}

			dynamic_box(const dynamic_box& _src)
			{
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = false;
			}

			dynamic_box(dynamic_box&& _src)
			{
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = true;
				_src.stuff = nullptr;
				_src.stuff_size = 0;
				_src.own_the_data = false;
			}

			virtual serialized_box* get_box() 
			{
				return (serialized_box*)stuff;
			}

			virtual serialized_box* check(int _bytes) 
			{
				serialized_box* ob, *nb;
				ob = get_box();
				nb = ob;
				if (_bytes > ob->free())
				{
					corona_size_t s = ob->size();
					corona_size_t d = ob->size() * 2;
					corona_size_t a = d - ob->free();
					while (a < _bytes) {
						d *= 2;
						a = d - ob->free();
					}
					resize(d);
					std::cout << "resized from " << s << " to " << d << std::endl;
					serialized_box *nb = get_box();
					nb->adjust(d);
				}
				return nb;
			}

			void init(corona_size_t _length, serialized_box *_src = nullptr)
			{
				resize(_length);
				get_box()->init(_length);
				if (_src) {
					std::copy(_src->data(), _src->data() + _length, get_box()->data());
				}
			}

			void copy_box(serialized_box* _src = nullptr)
			{
				resize(_src->size());
				get_box()->init(_src->size());
				std::copy(_src->data(), _src->data() + _src->size(), get_box()->data());
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box(const bx& _src)
			{
				corona_size_t new_size = _src.size();
				init(new_size);
				memcpy(data(), _src.data(), new_size);
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box operator = (const bx& _src)
			{
				corona_size_t new_size = _src.size();
				init(new_size);
				memcpy(data(), _src.data(), new_size);
				return *this;
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

