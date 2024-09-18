/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT LICENSE

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_STORE_BOX_H
#define CORONA_STORE_BOX_H

namespace corona {

	template <typename box_class>
	concept box_data = requires(box_class c, char* b, int x) {
		x = c.size();
		b = c.data();
	};

#undef min
#undef max

	struct box_block
	{
		relative_ptr_type		location;
		relative_ptr_type		allocated_length;
		relative_ptr_type		payload_length;
		relative_ptr_type		block_type_id;
		bool					deleted;
		time_t					updated;
		char					data[1];
		template <typename T>   T* get_data() { return (T*)data[0]; }
	};

	template <typename box_class>
	concept box_implementation = requires(box_block * bb, box_class bc, relative_ptr_type loc, corona_size_t sz, int x, char* c, bool tf, int64_t i) {
		bc.init(x);
		bc.adjust(x);
		sz = bc.size();
		x = bc.top();
		bb = bc.reserve(sz);
		i = bc.free();
		bb = bc.allocate(sz, x, loc);
		bb = bc.get_object(loc);
		loc = bc.create_object(loc, c, x);
		loc = bc.put_object(c, x);
		bc.commit();
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

		serialized_box_implementation()
		{
			;
		}

		virtual ~serialized_box_implementation()
		{
			;
		}

		virtual void init(corona_size_t _length) = 0;
		virtual void adjust(corona_size_t _length) = 0;
		virtual corona_size_t size() const = 0;
		virtual relative_ptr_type top() const = 0;
		virtual corona_size_t free() const = 0;
		virtual void clear() = 0;
		virtual box_block* reserve(corona_size_t length) = 0;
		virtual box_block* allocate(int64_t sizeofobj, size_t length) = 0;
		virtual box_block* get_object(relative_ptr_type _src) = 0;
		virtual relative_ptr_type create_object(char* _src, size_t _length) = 0;
		virtual relative_ptr_type update_object(relative_ptr_type _location, char* _src, size_t _length) = 0;
		virtual bool delete_object(relative_ptr_type _location) = 0;
		virtual relative_ptr_type copy_object(relative_ptr_type _location) = 0;
		virtual relative_ptr_type commit() = 0;
	};

	class serialized_box_memory_implementation : public serialized_box_implementation
	{
		serialized_box_data* sbdata;

		serialized_box_memory_implementation(char* _data)
		{
			sbdata = serialized_box_data::from(_data);
			sbdata->_box_id = block_id::box_id();
		}

	public:

		serialized_box_memory_implementation()
		{
			sbdata = nullptr;
		}

		static serialized_box_memory_implementation* from(char* _data)
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
			sbdata = _bx.sbdata;
		}

		template <typename bx>
			requires (box_data<bx>)
		serialized_box_memory_implementation operator = (const bx& _src)
		{
			int64_t new_size = _src.size();
			if (sbdata->_size < new_size)
				throw std::invalid_argument("target box too small");
			sbdata->_top = _src.top();
			sbdata->_size = new_size;
			memcpy(sbdata->_data, _src.data(), new_size);
			return *this;
		}

		virtual void init(corona_size_t _length)
		{
			sbdata->_top = 0;
			sbdata->_size = _length;
			sbdata->_box_id = block_id::box_id();
		}

		virtual void adjust(corona_size_t _length)
		{
			sbdata->_size = _length;
		}

		virtual corona_size_t size() const
		{
			return sbdata->_size;
		}

		virtual relative_ptr_type top() const
		{
			return sbdata->_top;
		}

		virtual corona_size_t free() const
		{
			return size() - top();
		}

		virtual void clear()
		{
			sbdata->_top = 0;
		}

		virtual box_block* reserve(corona_size_t length)
		{
			relative_ptr_type placement;
			auto ac = allocate(1, length);
			return ac;
		}

		virtual box_block* allocate(int64_t sizeofobj, size_t length)
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

			relative_ptr_type start = sbdata->_top + ((alignment - sbdata->_top % alignment) % alignment);
			relative_ptr_type stop = start + sizeofobj * length + sizeof(box_block);

			//				std::cout << "pack:" << start << " " << stop << " " << _size << std::endl;

			if (stop > sbdata->_size)
			{
				return nullptr;
			}

			box_block* destptr = (box_block*)(sbdata->_data + start);
			destptr->allocated_length = stop - start;
			destptr->payload_length = length;
			destptr->location = start;
			destptr->deleted = false;
			destptr->block_type_id = 0;
			sbdata->_top = stop;

			return destptr;
		}

		virtual box_block* get_object(relative_ptr_type _src)
		{
			if (_src == null_row) {
				return nullptr;
			}
			box_block* item = (box_block*)&sbdata->_data[_src];
			return item;
		}

		virtual relative_ptr_type create_object(char* _src, size_t _length)
		{
			box_block* item = allocate(1, _length);
			if (not item) return null_row;
			memcpy(&item->data[0], _src, _length);
			return item->location;
		}

		virtual relative_ptr_type update_object(relative_ptr_type _placement, char* _src, size_t _length)
		{
			box_block* item = (box_block*)&sbdata->_data[_placement];
			char* dest;
			dest = &item->data[0];
			std::copy(_src, _src + _length, dest);
			return item->location;
		}

		virtual bool delete_object(relative_ptr_type _location)
		{
			box_block* item = (box_block*)&sbdata->_data[_location];
			item->deleted = true;
			return true;
		}

		virtual relative_ptr_type copy_object(relative_ptr_type _location)
		{
			relative_ptr_type dest_location = null_row;
			box_block* dest_block;
			box_block* item = (box_block*)&sbdata->_data[_location];
			dest_block = allocate(1, item->payload_length);
			if (dest_block) {
				char* p = &item->data[0];
				std::copy(p, p + item->payload_length, &dest_block->data[0]);
			}
			return dest_location;
		}

		virtual relative_ptr_type commit()
		{
			return null_row;
		}

	};

	class serialized_box
	{

		serialized_box_implementation* boxi;

	public:

		serialized_box() : boxi(nullptr)
		{
			;
		}

		serialized_box(serialized_box_implementation* _boxi) : boxi(_boxi)
		{
			;
		}

		virtual ~serialized_box()
		{
			;
		}

		void adjust(corona_size_t _length)
		{
			boxi->adjust(_length);
		}

		relative_ptr_type reserve(corona_size_t length)
		{
			return boxi->reserve(length)->location;
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

		char* allocate(int64_t sizeofobj, size_t length, relative_ptr_type& dest)
		{
			auto allocation = boxi->allocate(sizeofobj, length);
			dest = allocation->location;
			return allocation ? &allocation->data[0] : nullptr;
		}

		box_block* get_object(relative_ptr_type _src)
		{
			return boxi->get_object(_src);
		}

		relative_ptr_type create_object(char* _src, size_t _length)
		{
			return boxi->create_object(_src, _length);
		}

		virtual relative_ptr_type update_object(relative_ptr_type _placement, char* _src, size_t _length)
		{
			return boxi->update_object(_placement, _src, _length);
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

		serialized_box_container()
		{
			;
		}

		virtual ~serialized_box_container()
		{
			;
		}

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
		T* allocate(size_t length, relative_ptr_type& dest)
		{
			auto obj = get_box()->allocate(sizeof(T), length, dest);
			return (T*)obj;
		}

		template <typename T>
			requires (std::is_standard_layout<T>::value)
		T* get_object(relative_ptr_type offset, T* dummy = nullptr)
		{
			auto b = get_box()->get_object(offset);
			return (T*)b->data;
		}

		template <typename T>
			requires (std::is_standard_layout<T>::value)
		relative_ptr_type put_object(T& src)
		{
			relative_ptr_type placement;
			T* item = allocate<T>(1, placement);
			if (not item) return placement;
			*item = src;
			return placement;
		}

		template <typename T>
			requires (std::is_standard_layout<T>::value)
		relative_ptr_type put_object(const T* src, size_t length)
		{
			relative_ptr_type placement;
			T* item = allocate<T>(length, placement);
			if (not item) return placement;
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
		relative_ptr_type fill(T src, size_t length)
		{
			relative_ptr_type placement;
			T* item = allocate<T>(length, placement);
			if (not item) return placement;
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
		relative_ptr_type pack_slice(const T* base, size_t start, size_t stop, bool terminate = true)
		{
			relative_ptr_type placement;
			T* item = allocate<T>((stop - start) + 1, placement);
			if (not item) return placement;

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
		relative_ptr_type put_null_terminated(const T* base, size_t start)
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
			if (not item) return placement;

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
		T* copy(const T* base, size_t start)
		{
			if (not base) return nullptr;
			corona_size_t l = put_null_terminated(base, start);
			return get_object<T>(l);
		}

		template <typename T>
			requires (std::is_standard_layout<T>::value)
		T* copy(const T* base, size_t start, size_t stop, bool terminate = true)
		{
			if (not base) return nullptr;
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
			if (not item) return item;
			item = new (item) T(source);
			return item;
		}

		template <typename T>
			requires (std::is_standard_layout<T>::value)
		T* allocate(size_t count)
		{
			relative_ptr_type placement;
			T* item = allocate<T>(count, placement);
			if (not item) return item;

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
	class static_box : public serialized_box_container
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
			memcpy(stuff, _src.data(), new_size);
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

		inline_box(char* _stuff, size_t _length)
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

		virtual ~inline_box()
		{
			;
		}

		virtual serialized_box* get_box() { return &box; }
		virtual const serialized_box* get_box_const() const {
			return box.get_address();
		}
		virtual serialized_box* check(int _bytes) { return _bytes < get_box()->free() ? get_box() : nullptr; }

		inline_box& operator =(inline_box& _src)
		{
			size_t new_size = _src.size();
			if (length < new_size)
				throw std::invalid_argument("target box too small");
			clear();
			auto base = _src.get_object<char>(0);
			pack_slice<char>(base, 0, new_size, false);
			return *this;
		}
	};

	class dynamic_box : public serialized_box_container
	{
		char* stuff;
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
			if (not temp) {
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

		void ownership_delete(char* other_stuff)
		{
			if (stuff != other_stuff and own_the_data and stuff)
			{
				delete[] stuff;
				stuff = nullptr;
			}
		}

	public:

		virtual serialized_box* get_box() { return &box; }
		virtual const serialized_box* get_box_const() const { return &box; }

		dynamic_box() : stuff(nullptr), stuff_size(0), own_the_data(true)
		{
#if	TRACE_DYNAMIC_BOX
			std::cout << "box:" << this << " " << (void*)stuff << " create " << std::endl;
#endif
		}

		dynamic_box(int64_t _size) : stuff(nullptr), stuff_size(0), own_the_data(true)
		{
#if	TRACE_DYNAMIC_BOX
			std::cout << "box:" << this << " " << (void*)stuff << " create " << std::endl;
#endif
			init(_size);
		}

		virtual ~dynamic_box()
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

		void init(corona_size_t _length, serialized_box* _src = nullptr)
		{
			corona_size_t new_length = _src ? std::max(_length, _src->size()) : _length;
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
			if (not _src) return;
			corona_size_t new_length = std::max(size(), _src->size());
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
	using basic_time_box = boxed<DATE>;
	using basic_currency_box = boxed<CY>;
	using basic_collection_id_box = boxed<collection_id_type>;
	using basic_object_id_box = boxed<object_id_type>;

	bool test_box(serialized_box_container* b)
	{
		struct test_struct {
			int i1,
				i2,
				i3;
			double d;
			char s[8];
		};

		test_struct tests[3] = { { 1, 2, 3, 4.5, "alpha"}, { 10, 20, 30, 45.5, "beta"}, { 100, 200, 300, 455.5, "gamma"} };
		int locations[3] = { -1, -1, -1 };
		relative_ptr_type l = 0;
		int c = 0;

		// pack the box until it is full

		while (l >= 0)
		{
			int i = c % 3;
			l = b->put_object(tests[i]);
			if (l != null_row) {
				locations[i] = l;
				c++;
			}
		}

		while (c >= 0)
		{
			int i = c % 3;
			test_struct* ts = b->get_object<test_struct>(locations[i]);
			test_struct& item = tests[i];
			if (ts->i1 != item.i1 ||
				ts->i2 != item.i2 ||
				ts->i3 != item.i3 ||
				ts->d != item.d ||
				strcmp(ts->s, item.s))
			{
				std::cout << __LINE__ << ": pack " << c << " did not match unpack" << std::endl;
				return false;
			}
			c--;
		}

		return true;
	}

	bool box_tests()
	{
		dynamic_box dbox;
		dbox.init(500);
		if (not test_box(&dbox))
		{
			std::cout << __LINE__ << ": dynamic box failed" << std::endl;
			return false;
		}

		static_box<500> sbox;
		if (not test_box(&sbox))
		{
			std::cout << __LINE__ << ": static box failed" << std::endl;
			return false;
		}
		return true;
	}

}

#endif
