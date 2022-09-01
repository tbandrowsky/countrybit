#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <numeric>

#include "constants.h"

#define TRACE_DYNAMIC_BOX 0

namespace corona
{
	namespace database
	{

		template <typename box_class>
		concept box_data = requires(box_class c, char* b, int x) {
			x = c.size();
			b = c.data();
		};

		template <typename box_class>
		concept box_implementation = requires(box_class c, relative_ptr_type l, corona_size_t s, int x, char* b, bool v, int64_t i) {
			c.init(x);
			c.adjust(x);
			s = c.size();
			x = c.top();
			l = c.reserve(s);
			i = c.free();
			b = c.allocate(s, x, l);
			b = c.get_object(l);
			l = c.put_object(b, x);
			c.commit();
		};

		struct serialized_box_data
		{
			block_id		 _box_id;
			corona_size_t _size;
			corona_size_t _top;
			char _data[1];

			static serialized_box_data* from(char* p)
			{
				serialized_box_data* r = new (p) serialized_box_data();
				return r;
			}
		};

		class serialized_box_implementation
		{
		public:
			virtual void init(corona_size_t _length) = 0;
			virtual void adjust(corona_size_t _length) = 0;
			virtual corona_size_t size() const = 0;
			virtual relative_ptr_type top() const = 0;
			virtual corona_size_t free() const = 0;
			virtual void clear() = 0;
			virtual relative_ptr_type reserve(corona_size_t length) = 0;
			virtual char* allocate(int64_t sizeofobj, int length, relative_ptr_type& dest) = 0;
			virtual char* get_object(relative_ptr_type _src) = 0;
			virtual relative_ptr_type put_object(char* _src, int _length) = 0;
			virtual void commit() = 0;
		};

		class serialized_box_memory_implementation : public serialized_box_implementation
		{
			serialized_box_data* data;


			serialized_box_memory_implementation(char* _data)
			{
				data = serialized_box_data::from(_data);
				data->_box_id = block_id::box_id();
			}

		public:

			serialized_box_memory_implementation()
			{
				data = nullptr;
			}

			static serialized_box_memory_implementation *from(char* _data)
			{
				serialized_box_memory_implementation* nw = new (_data) serialized_box_memory_implementation(_data + sizeof(serialized_box_memory_implementation));
				return nw;
			}

			static serialized_box_memory_implementation* create(char* _data, int _length)
			{
				int reserved_size = sizeof(serialized_box_memory_implementation) + sizeof(serialized_box_data);
				serialized_box_memory_implementation* nw = new (_data) serialized_box_memory_implementation(_data + sizeof(serialized_box_memory_implementation));
				nw->init(_length - reserved_size);
				return nw;
			}

			serialized_box_memory_implementation operator = (serialized_box_memory_implementation& _bx)
			{
				data = _bx.data;
			}

			template <typename bx>
				requires (box_data<bx>)
			serialized_box_memory_implementation operator = (const bx& _src)
			{
				int64_t new_size = _src.size();
				if (data->_size < new_size)
					throw std::invalid_argument("target box too small");
				data->_top = _src.top();
				data->_size = new_size;
				memcpy(data->_data, _src.data(), new_size);
				return *this;
			}

			virtual void init(corona_size_t _length)
			{
				data->_top = 0;
				data->_size = _length;
				data->_box_id = block_id::box_id();
			}

			virtual void adjust(corona_size_t _length)
			{
				data->_size = _length;
			}

			virtual corona_size_t size() const
			{
				return data->_size;
			}

			virtual relative_ptr_type top() const
			{
				return data->_top;
			}

			virtual corona_size_t free() const
			{
				return size() - top();
			}

			virtual void clear()
			{
				data->_top = 0;
			}

			virtual relative_ptr_type reserve(corona_size_t length)
			{
				corona_size_t sz = length;
				corona_size_t placement = data->_top;
				corona_size_t new_top = placement + sz;
				if (new_top > data->_size)
					return -1;
				data->_top = new_top;
				return placement;
			}

			virtual char* allocate(int64_t sizeofobj, int length, relative_ptr_type& dest)
			{
				relative_ptr_type alignment = sizeof(sizeofobj);

				if (sizeofobj < 8)
				{
					alignment = sizeofobj;
				}
				else
				{
					alignment = 8;
				}

				relative_ptr_type start = data->_top + ((alignment - data->_top % alignment) % alignment);
				relative_ptr_type stop = start + sizeofobj * length;

				//				std::cout << "pack:" << start << " " << stop << " " << _size << std::endl;

				if (stop > data->_size)
				{
					dest = null_row;
					return nullptr;
				}

				char* destptr = (data->_data + start);
				data->_top = stop;
				dest = start;

				return destptr;
			}

			virtual char* get_object(relative_ptr_type _src)
			{
				if (_src == null_row) {
					return nullptr;
				}
				char* item = &data->_data[_src];
				return item;
			}

			virtual relative_ptr_type put_object(char *_src, int _length)
			{
				relative_ptr_type placement;
				char* item = allocate(1, _length, placement);
				if (!item) return placement;
				memcpy(item,_src, _length);
				return placement;
			}

			virtual void commit()
			{
				;
			}
			
			char* data()
			{
				return data->_data;
			}

		};

		class serialized_box 
		{

			serialized_box_implementation *boxi;

		public:

			serialized_box() : boxi(nullptr)
			{
				;
			}

			serialized_box(serialized_box_implementation *_boxi) : boxi(_boxi)
			{
				;
			}

			void adjust(corona_size_t _length)
			{
				boxi->adjust(_length);
			}

			relative_ptr_type reserve(corona_size_t length)
			{
				return boxi->reserve(length);
			}

			corona_size_t top() const
			{
				return boxi->top();
			}

			const serialized_box* get_address() const
			{
				return this;
			}

			corona_size_t free() const
			{
				return boxi->free();
			}

			corona_size_t size() const
			{
				return boxi->size();
			}

			void clear()
			{
				boxi->clear();
			}

			char* allocate(int64_t sizeofobj, int length, relative_ptr_type& dest)
			{
				return boxi->allocate(sizeofobj, length, dest);
			}

			char* get_object(relative_ptr_type _src)
			{
				return boxi->get_object(_src);
			}

			relative_ptr_type put_object(char* _src, int _length)
			{
				return boxi->put_object(_src, _length);
			}

			void commit()
			{
				boxi->commit();
			}

		};


		class serialized_box_container
		{
		public:

			virtual serialized_box* get_box() { return nullptr; }
			virtual const serialized_box* get_box_const() const { return nullptr; }
			virtual serialized_box* check(corona_size_t _bytes) { return nullptr; }

			relative_ptr_type top() const
			{
				return get_box_const()->top();
			}

			corona_size_t free() const
			{
				return get_box_const()->size();
			}

			corona_size_t size() const
			{
				return get_box_const()->size();
			}

			void clear()
			{
				return get_box()->clear();
			}
		
			template <typename T>
			T* allocate(int length, relative_ptr_type& dest)
			{
				return (T*)get_box()->allocate(sizeof(T), length, dest);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* get_object(relative_ptr_type offset, T* dummy = nullptr)
			{
				return (T*)get_box()->get_object(offset);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type put_object(T& src)
			{
				relative_ptr_type placement;
				T* item = allocate<T>(1, placement);
				if (!item) return placement;
				*item = src;
				return placement;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			relative_ptr_type put_object(const T* src, int length)
			{
				relative_ptr_type placement;
				T* item = allocate<T>(length, placement);
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
				T* item = allocate<T>(length, placement);
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
				T* item = allocate<T>((stop - start) + 1, placement);
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
			relative_ptr_type put_null_terminated(const T* base, int start)
			{
				int length = 0;
				T defaulto = {};

				while (base[start + length] != defaulto)
				{
					length++;
				}
				length++;

				relative_ptr_type placement;
				T* item = allocate<T>(length, placement);
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
				if (!base) return nullptr;
				corona_size_t l = put_null_terminated(base, start);
				return get_object<T>(l);
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* copy(const T* base, int start, int stop, bool terminate = true)
			{
				if (!base) return nullptr;
				corona_size_t l = pack_slice(base, start, stop, terminate);
				return get_object<T>(l);
			}

			template <typename T>
			T* clone(T& source)
			{
				// TODO: the cloned item here needs to have its destructor called,
				// so, we shall have to track this with a list some kind down the road to 
				// use this facility, and ideally create a special box
				relative_ptr_type placement;
				T* item = allocate<T>(1, placement);
				if (!item) return item;
				item = new (item) T(source);
				return item;
			}

			template <typename T>
				requires (std::is_standard_layout<T>::value)
			T* allocate(int count)
			{
				relative_ptr_type placement;
				T* item = allocate<T>(count, placement);
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

			void commit()
			{
				get_box()->commit();
			}

		};


		template <size_t bytes> 
		class static_box  : public serialized_box_container
		{
			static const int length = bytes;
			serialized_box box;
			char stuff[length];

		public:

			static_box() 
			{
				box = serialized_box_memory_implementation::create(stuff, length);
			}

			virtual serialized_box* get_box() { return &box; }
			virtual const serialized_box* get_box_const() const { return &box; }
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
			static_box operator = (bx& _src)
			{
				int new_size = _src.size();
				if (length < new_size)
					throw std::invalid_argument("target box too small");
				clear();
				auto base = _src.get_object<char>(0);
				pack_slice<char>(base, 0, new_size, false);
				return *this;
			}

		};

		class inline_box : public serialized_box_container
		{
			char* stuff;
			int64_t length;
			serialized_box box;

		public:

			inline_box() : stuff(nullptr), length(0)
			{
				
			}

			inline_box(char* _stuff, int _length) 
				: 
				stuff(_stuff), 
				length(_length)
			{
				box = serialized_box_memory_implementation::create(stuff, length);
			}

			inline_box(const inline_box& _src)
			{
				box = serialized_box_memory_implementation::create(_src.stuff, _src.length);
			}

			virtual serialized_box* get_box() { return &box; }
			virtual const serialized_box* get_box_const() const { 
				return box.get_address();
			}
			virtual serialized_box* check(int _bytes) { return _bytes < get_box()->free() ? get_box() : nullptr; }

			inline_box& operator =(inline_box& _src) 
			{
				int new_size = _src.size();
				if (length < new_size)
					throw std::invalid_argument("target box too small");
				clear();
				auto base = _src.get_object<char>(0);
				pack_slice<char>(base, 0, new_size, false);
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

			serialized_box box;

			void resize(corona_size_t new_size)
			{
				corona_size_t new_stuff_size = new_size + sizeof(serialized_box) + sizeof(serialized_box_memory_implementation);
				if (new_stuff_size < stuff_size)
					return;

#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << std::endl;
#endif
				char* temp = new char[new_stuff_size];
				if (!temp) {
					throw std::exception("Out of memory");
				}
				if (stuff) {
					memcpy(temp, stuff, stuff_size);
					ownership_delete(nullptr);
					stuff = temp;
					stuff_size = new_size;
					box = serialized_box_memory_implementation::from(temp);
					box.adjust(stuff_size);
				}
				else 
				{
					stuff = temp;
					stuff_size = new_size;
					box = serialized_box_memory_implementation::create(temp, stuff_size);
				}
				own_the_data = true;
			}

			void ownership_delete(char *other_stuff)
			{
				if (stuff != other_stuff && own_the_data && stuff)
				{
					delete[] stuff;
					stuff = nullptr;
				}
			}

		public:

			dynamic_box() : stuff(nullptr), stuff_size(0), own_the_data(true)
			{
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " create " << std::endl;
#endif
			}

			~dynamic_box()
			{
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " delete " << std::endl;
#endif
				ownership_delete(nullptr);
			}

			dynamic_box& operator = (const dynamic_box& _src)
			{
				ownership_delete(nullptr);
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = false;
				box = serialized_box_memory_implementation::from(stuff);
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " copy " << std::endl;
#endif
				return *this;
			}

			dynamic_box& operator = (dynamic_box&& _src)
			{
				ownership_delete(nullptr);
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = true;
				_src.stuff = nullptr;
				_src.stuff_size = 0;
				_src.own_the_data = false;
				box = serialized_box_memory_implementation::from(stuff);
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " move " << std::endl;
#endif
				return *this;
			}

			dynamic_box(const dynamic_box& _src) : stuff(nullptr), stuff_size(0), own_the_data(true)
			{
				ownership_delete(nullptr);
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = false;
				box = serialized_box_memory_implementation::from(stuff);
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " copy ctor" << std::endl;
#endif
			}

			dynamic_box(dynamic_box&& _src) : stuff(nullptr), stuff_size(0), own_the_data(true)
			{
				ownership_delete(nullptr);
				stuff = _src.stuff;
				stuff_size = _src.stuff_size;
				own_the_data = true;
				_src.stuff = nullptr;
				_src.stuff_size = 0;
				_src.own_the_data = false;
				box = serialized_box_memory_implementation::from(stuff);
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " move ctor" << std::endl;
#endif
			}

			virtual bool has_data()
			{
				return stuff != nullptr;
			}

			virtual serialized_box* check(int _bytes) 
			{
				serialized_box* ob;
				ob = get_box();
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
				}
				return get_box();
			}

			void init(corona_size_t _length, serialized_box *_src = nullptr)
			{
				corona_size_t new_length = _src ? max(_length, _src->size()) : _length;
				resize(new_length);
				if (_src) {
					clear();
					copy(_src->get_object(0), 0, _src->size(), false);
				}
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " init " << std::endl;
#endif

			}

			void copy_box(serialized_box* _src = nullptr)
			{
				if (!_src) return;
				corona_size_t new_length = max(size(), _src->size());
				resize(new_length);
				if (_src) {
					clear();
					copy(_src->get_object(0), 0, _src->size(), false);
				}
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " copy box " << std::endl;
#endif
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box(const bx& _src) : stuff(nullptr), stuff_size(0), own_the_data(true)
			{
				corona_size_t new_size = _src.size();
				resize(new_size);
#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " copy data ctor " << std::endl;
#endif
			}

			template <typename bx>
			requires (box_data<bx>)
			dynamic_box operator = (const bx& _src)
			{
				corona_size_t new_size = _src.size();
				resize(new_size);
				clear();
				copy(_src.data(), 0, new_size, false);

#if	TRACE_DYNAMIC_BOX
				std::cout << "box:" << this << " " << (void*)stuff << " assign " << std::endl;
#endif
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

