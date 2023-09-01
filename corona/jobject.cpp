
#include "corona.h"

#define _DETAIL 0
#define _TRACE_RULE 0
#define _TRACE_GET_OBJECT 0
#define _TRACE_SEARCH 1

namespace corona
{
	namespace database
	{

		int compare(const dimensions_type& a, const dimensions_type& b)
		{
			corona_size_t t = a.z - b.z;
			if (t) return t;
			t = a.y - b.y;
			if (t) return t;
			t = a.x - b.x;
			return t;
		}

		int operator<(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) < 0;
		}

		int operator>(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) > 0;
		}

		int operator>=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) >= 0;
		}

		int operator<=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) <= 0;
		}

		int operator==(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) == 0;
		}

		int operator!=(const dimensions_type& a, const dimensions_type& b)
		{
			return compare(a, b) != 0;
		}

		bool init_collection_id(collection_id_type &collection_id)
		{
			::GUID gidReference;
			HRESULT hr = ::CoCreateGuid((GUID *) &collection_id);
			return hr == S_OK;
		}


		jobject jcollection::create_object(relative_ptr_type _class_id, relative_ptr_type& _object_id)
		{
			auto myclass = schema->get_class(_class_id);
			auto total_bytes = myclass.item().class_size_bytes;

			collection_object_type co;
			co.oid.collection_id = collection_id;
			co.class_id = _class_id;
			co.deleted = false;

			auto new_object = objects.create_item( &co, total_bytes, nullptr);
			#if	_TRACE_GET_OBJECT
			std::cout << "created obj, objects size:"<< objects.size() << " collections size:" << size() << std::endl;
			#endif
			new_object.item().last_modified = std::time(nullptr);
			co.oid.row_id = new_object.row_id();
			_object_id = new_object.row_id();
			char* bytes = new_object.pdetails();
			jobject jobj(schema, _class_id, bytes);
			int pkidx = myclass.pitem()->primary_key_idx;

			if (pkidx > -1) {
				auto pkfield = jobj.get_int64(pkidx);
				pkfield = co.oid.row_id;
			}

			return jobj;
		}

		jobject jcollection::create_object(relative_ptr_type _class_id, relative_ptr_type& _object_id, std::initializer_list<jvariant> var)
		{
			jobject j = create_object(_class_id, _object_id);
			j.set(var);
			return j;
		}

		jobject jcollection::get_object(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);

			auto item_type_id = existing_object.pitem()->otype;
			auto deleted = existing_object.pitem()->deleted;

			#if	_TRACE_GET_OBJECT
				std::cout << "get_object id:" << _object_id << ", class_id:: " << item_type_id << ", deleted:" << deleted << std::endl;
			#endif

			if (!existing_object.pitem()->deleted) {
				jobject jax(schema, existing_object.item().class_id, existing_object.pdetails());
				return jax;
			}
			else 
			{
				jobject active;
				return active;
			}
		}

		relative_ptr_type jcollection::get_class_id(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			relative_ptr_type cls_id = existing_object.pitem()->class_id;

#if _TRACE_RULE
			//std::cout << "object " << _object_id << " class " << cls_id << " " << schema->get_class(cls_id).pitem()->name << std::endl;
#endif

			return cls_id;
		}

		relative_ptr_type jcollection::get_base_id(relative_ptr_type _object_id)
		{
			relative_ptr_type class_id = get_class_id(_object_id);
			auto class_def = schema->get_class(class_id);
			return class_def.item().base_class_id;
		}

		bool jcollection::class_has_base(relative_ptr_type _class_id, relative_ptr_type _base_id)
		{
			if (_base_id == null_row || _class_id == null_row)
			{
				return false;
			}
			if (_base_id == _class_id) 
			{
				return true;
			}
			relative_ptr_type base_class_id = schema->get_class(_class_id).item().base_class_id;
			if (base_class_id == _base_id) {
				return true;
			}
			while (base_class_id != null_row) {
				auto new_class = schema->get_class(base_class_id);
				base_class_id = new_class.item().base_class_id;
				if (base_class_id == _base_id) {
					return true;
				}
			}
			return false;
		}

		int64_t jcollection::get_class_count(relative_ptr_type _class_id)
		{
			int64_t count = 0;
			int64_t index = 0;
			for (index = 0; index < objects.size(); index++)
			{
				auto object_class_id = objects[index].item().class_id;
				if (class_has_base(object_class_id, _class_id))
					count++;
			}
			return count;
		}

		bool jcollection::object_is_class(const jobject& obj, relative_ptr_type _class_id)
		{
			relative_ptr_type class_id = obj.get_class_id();
			return class_has_base(class_id, _class_id);
		}

		bool jcollection::object_is_class(relative_ptr_type _object_id, relative_ptr_type _class_id)
		{
			relative_ptr_type class_id = get_class_id(_object_id);
			return class_has_base(class_id, _class_id);
		}

		bool jcollection::object_is_class(relative_ptr_type _object_id, class_list& _class_ids)
		{
			for (auto cli : _class_ids)
			{
				if (object_is_class(_object_id, cli.item))
					return true;
			}
			return false;
		}

		bool jcollection::object_is_class(relative_ptr_type _object_id, std::vector<relative_ptr_type> _class_ids)
		{
			for (auto cli : _class_ids)
			{
				if (object_is_class(_object_id, cli))
					return true;
			}
			return false;
		}

		bool jcollection::object_is_class(const jobject& obj, std::vector<relative_ptr_type> _class_ids)
		{
			for (auto cli : _class_ids)
			{
				if (object_is_class(obj, cli))
					return true;
			}
			return false;
		}

		bool jcollection::object_is_class(const jobject& obj, class_list& _class_ids)
		{
			for (auto cli : _class_ids)
			{
				if (object_is_class(obj, cli.item))
					return true;
			}
			return false;
		}


		jobject jcollection::get_at(relative_ptr_type _object_id)
		{
			return get_object(_object_id);
		}

		collection_object_type &jcollection::get_object_reference(relative_ptr_type _object_id)
		{
			auto existing_object = objects.get_item(_object_id);
			return existing_object.item();
		}

		jobject jcollection::update_object(relative_ptr_type _object_id, jobject _slice)
		{
			auto existing_object = objects.get_item(_object_id);

			existing_object.item().last_modified = std::time(nullptr);
			jobject obj(schema, existing_object.item().class_id, existing_object.pdetails());
			obj.update(_slice);
			return obj;
		}

		relative_ptr_type jobject::get_primary_key()
		{
			relative_ptr_type pkfield = null_row;
			auto cls = get_class();
			int pkid = cls.pitem()->primary_key_idx;
			if (pkid > -1) {
				pkfield = cls.detail(pkid).field_id;
			}
			return pkfield;
		}

		relative_ptr_type jobject::get_primary_key_value()
		{
			relative_ptr_type pkfield = null_row;
			auto cls = get_class();
			int pkid = cls.pitem()->primary_key_idx;
			if (pkid > -1) {
				pkfield = get_int64(pkid, false);
			}
			return pkfield;
		}

		jvariant jobject::get(relative_ptr_type _field_id)
		{
			jvariant sma;

			if (_field_id < 0)
			{
				throw std::invalid_argument(std::format("get:invalid field id {}", _field_id));
			}

			int _field_idx = get_field_index_by_id(_field_id);

			if (_field_idx < 0)
			{
				throw std::invalid_argument(std::format("get:field id not found {}", _field_id));
			}

			const jfield& field = get_field(_field_idx);
			switch (field.type_id)
			{
			case jtype::type_int8:
				sma = jvariant( _field_idx, (int64_t)get_int8(_field_idx) );
				break;
			case jtype::type_int16:
				sma = jvariant(_field_idx, (int64_t)get_int16(_field_idx) );
				break;
			case jtype::type_int32:
				sma = jvariant(_field_idx, (int64_t)get_int32(_field_idx) );
				break;
			case jtype::type_int64:
				sma = jvariant(_field_idx, (int64_t)get_int64(_field_idx) );
				break;
			case jtype::type_float32:
				sma = jvariant(_field_idx, (double)get_float(_field_idx) );
				break;
			case jtype::type_float64:
				sma = jvariant(_field_idx, (double)get_double(_field_idx) );
				break;
			case jtype::type_datetime:
				sma = jvariant(_field_idx, (time_t)get_time(_field_idx));
				break;
			case jtype::type_image:
				break;
			case jtype::type_midi:
				break;
			case jtype::type_null:
				break;
			case jtype::type_string:
				{
				auto str_value = get_string(_field_idx);
				sma = jvariant(_field_idx, str_value.c_str());
				}
				break;
			case jtype::type_wave:
				break;
			}

			return sma;
		}

		jvariant jobject::operator[](relative_ptr_type field_id)
		{
			return get(field_id);
		}

		jobject::jobject() : schema(nullptr), class_id(null_row), bytes(nullptr), box(nullptr), location(null_row)
		{
			;
		}

		jobject::jobject(const jschema  * _schema , relative_ptr_type _class_id, char* _bytes)
			: 
			schema(_schema), 
			class_id(_class_id), 
			bytes(_bytes), 
			box(nullptr), 
			location(null_row)
		{
			the_class = schema->get_class(_class_id);
		}

		jobject::jobject(const jschema  * _schema, relative_ptr_type _class_id, serialized_box_container *_box, relative_ptr_type _location) :
			schema(_schema), 
			class_id(_class_id), 
			bytes(nullptr), 
			box(_box), 
			location(_location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject::jobject(const jobject& _src) :
			schema(_src.schema),
			class_id(_src.class_id),
			bytes(_src.bytes),
			box(_src.box),
			location(_src.location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject jobject::operator =(const jobject& _src)
		{
			schema = _src.schema;
			class_id = _src.class_id;
			bytes = _src.bytes;
			box = _src.box;
			location = _src.location;
			the_class = schema->get_class(class_id);
			return *this;
		}

		jobject::jobject(jobject&& _src) : 
			schema(_src.schema),
			class_id(_src.class_id),
			bytes(_src.bytes),
			box(_src.box),
			location(_src.location)
		{
			the_class = schema->get_class(class_id);
		}

		jobject& jobject::operator =(jobject&& _src)
		{
			schema = _src.schema;
			class_id = _src.class_id;
			bytes = _src.bytes;
			box = _src.box;
			location = _src.location;
			the_class = schema->get_class(class_id);
			return *this;
		}

		jclass jobject::get_class() const
		{
			return the_class;
		}

		const jschema *jobject::get_schema()
		{
			return schema;
		}

		size_t jobject::get_offset(int field_idx, jtype _type)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || get_bytes() == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.detail(field_idx);
#if _DEBUG
			if (_type != jtype::type_null) {
				jfield& f = schema->get_field(jcf.field_id);
				if (f.type_id != _type) {
					istring<100> error = f.name + " accessed incorrectly.";
					throw std::logic_error(error.c_str());
				}
			}
#endif
			return jcf.offset;
		}

		/*
					auto offset1 = get_offset(_type, _src_idx);
			auto offset2 = _src_slice.get_offset(_type, _dst_idx);
			char* c1 = bytes + offset1;
			char* c2 = bytes + offset2;
*/
		std::partial_ordering jobject::compare_express(jtype _type, char *c1, char *c2)
		{
			switch (_type)
			{
			case jtype::type_int8:
				{
					int8_box b1(c1);
					int8_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int16:
				{
					int16_box b1(c1);
					int16_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int32:
				{
					int32_box b1(c1);
					int32_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_int64:
				{
					int64_box b1(c1);
					int64_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_float32:
				{
					float_box b1(c1);
					float_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_float64:
				{
					double_box b1(c1);
					double_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_datetime:
				{
					time_box b1(c1);
					time_box b2(c2);
					return b1 <=> b2;
				}
				break;
			case jtype::type_string:
				{
					auto b1 = string_box::get(c1);
					auto b2 = string_box::get(c2);
					return b1 <=> b2;
				}
				break;
			default:
				return c1 <=> c2;
			}
		}

		void jobject::construct()
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				jfield jf = schema->get_field(jcf.field_id);
				int offset = jcf.offset;
				char* c = get_bytes() + offset;
				switch (jf.type_id) 
				{
				case jtype::type_null:
					break;
				case jtype::type_int8:
					{
						int8_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int16:
					{
						int16_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int32:
					{
						int32_box b(c);
						b = 0;
					}
					break;
				case jtype::type_int64:
					{
						int64_box b(c);
						b = 0;
					}
					break;
				case jtype::type_float32:
					{
						float_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_float64:
					{
						double_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_datetime:
					{
						time_box b(c);
						b = 0.0;
					}
					break;
				case jtype::type_string:
					{
						string_box::create(c, jf.string_properties.length);
					}
					break;
				case jtype::type_image:
					{
						image_box b(c);
						b = image_instance {};
					}
					break;
				case jtype::type_wave:
					{
						wave_box b(c);
						b = wave_instance {};
					}
					break;
				case jtype::type_midi:
					{
						midi_box b(c);
						b = midi_instance {};
					}
					break;
				}
			}
		}

		const jfield& jobject::get_field(int field_idx) const
		{
			jclass_field& jcf = the_class.detail(field_idx);
			jfield &jf = schema->get_field(jcf.field_id);
			return jf;
		}

		int jobject::get_field_index_by_id(relative_ptr_type field_id)
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				if (jcf.field_id == field_id)
				{
					return i;
				}
			}
			return -1;
		}

		int jobject::get_field_index_by_name(const object_name& name)
		{
			relative_ptr_type field_id = schema->find_class(name);
			return get_field_index_by_id(field_id);
		}

		const jclass_field& jobject::get_class_field(int field_idx) const
		{
			jclass_field& jcf = the_class.detail(field_idx);
			return jcf;
		}

		bool jobject::has_field(const object_name& name)
		{
			return schema && get_field_index_by_name( name ) > -1;
		}

		bool jobject::has_field(relative_ptr_type field_id)
		{
			return field_id > null_row && get_field_index_by_id(field_id) > -1;
		}

		bool jobject::is_class(relative_ptr_type class_id)
		{
			return the_class.pitem()->class_id == class_id;
		}

		const jfield& jobject::get_field_by_id(relative_ptr_type field_id) const
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.detail(i);
				if (jcf.field_id == field_id)
				{
					jfield& jf = schema->get_field(jcf.field_id);
					return jf;
				}
			}
			return schema->get_empty();
		}

		int8_box jobject::get_int8(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("int8 field {} not found", field_id));
			}
			return get_boxed<int8_box>(field_idx, jtype::type_int8);
		}

		int16_box jobject::get_int16(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("int16 field {} not found", field_id));

			}
			return get_boxed<int16_box>(field_idx, jtype::type_int16);
		}

		int32_box jobject::get_int32(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("int32 field {} not found", field_id));

			}
			return jobject::get_boxed<int32_box>(field_idx, jtype::type_int32);
		}

		int64_box jobject::get_int64(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("int64 field {} not found", field_id));
			}
			return jobject::get_boxed<int64_box>(field_idx, jtype::type_int64);
		}

		float_box jobject::get_float(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("float field {} not found", field_id));
			}
			return jobject::get_boxed<float_box>(field_idx, jtype::type_float32);
		}

		double_box jobject::get_double(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("double field {} not found", field_id));
			}
			return jobject::get_boxed<double_box>(field_idx, jtype::type_float64);
		}

		time_box jobject::get_time(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("time field {} not found", field_id));
			}
			return get_boxed<time_box>(field_idx, jtype::type_datetime);
		}

		image_box jobject::get_image(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("image field {} not found", field_id));
			}
			return get_boxed<image_box>(field_idx, jtype::type_image);
		}

		wave_box jobject::get_wave(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("wave field {} not found", field_id));
			}
			return get_boxed<wave_box>(field_idx, jtype::type_wave);
		}

		midi_box jobject::get_midi(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("midi field {} not found", field_id));
			}
			return get_boxed<midi_box>(field_idx, jtype::type_midi);
		}

		string_box jobject::get_string(int field_id, bool _use_id)
		{
			int field_idx = null_row;

			if (_use_id)
				field_idx = get_field_index_by_id(field_id);
			else
				field_idx = field_id;

			if (field_idx == null_row)
			{
				throw std::invalid_argument(std::format("string field {} not found", field_id));
			}

			size_t offset = get_offset(field_idx, jtype::type_string);
			char *b = get_bytes() + offset;
			auto temp = string_box::get(b);
			return temp;
		}

		int8_box jobject::get_int8(object_name field_name)
		{
			int index = get_field_index_by_name(field_name);
			return get_int8(index);
		}

		int16_box jobject::get_int16(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_int16(index);
		}

		int32_box jobject::get_int32(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_int32(index);
		}

		int64_box jobject::get_int64(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_int64(index);
		}

		float_box jobject::get_float(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_float(index);
		}

		double_box jobject::get_double(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_double(index);
		}

		time_box jobject::get_time(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_time(index);
		}

		string_box jobject::get_string(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);;
			return get_string(index);
		}



		image_box jobject::get_image(object_name field_name)
		{
			int  index = get_field_index_by_name(field_name);
			return get_image(index);
		}

		wave_box jobject::get_wave(object_name field_name) {
			int  index = get_field_index_by_name(field_name);
			return get_wave(index);
		}

		midi_box jobject::get_midi(object_name field_name) {
			int  index = get_field_index_by_name(field_name);
			return get_midi(index);
		}

		bool jobject::matches(const char* _str)
		{
			if (!_str) return true;
			if (*_str == 0) return true;
			for (int i = 0; i < size(); i++) {
				if (get_field(i).type_id == jtype::type_string) {
					auto str = get_string(i);
					if (str.has_any(_str)) {
						return true;
					}
				}
			};
			return false;
		}

		void jobject::set_value(const jvariant& _member_assignment)
		{
			int index = get_field_index_by_id(_member_assignment.field_id);

			if (index == -1)
				return;

			auto fld = get_field(index);

			switch (_member_assignment.variant_type)
			{
			case jtype::type_float64:
			case jtype::type_float32:
				switch (fld.type_id)
				{
					case jtype::type_float64:
					{
						auto fb = get_double(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_float32:
					{
						auto fb = get_float(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int64:
					{
						auto fb = get_int64(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int32:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int16:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_int8:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.double_value;
					}
					break;
					case jtype::type_string:
					{
						auto fb = get_string(index);
						fb = _member_assignment.double_value;
					}
					break;
				}
				break;
			case jtype::type_int64:
			case jtype::type_int32:
			case jtype::type_int16:
			case jtype::type_int8:
				switch (fld.type_id)
				{
					case jtype::type_float64:
					{
						auto fb = get_double(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_float32:
					{
						auto fb = get_float(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int64:
					{
						auto fb = get_int64(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int32:
					{
						auto fb = get_int32(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int16:
					{
						auto fb = get_int16(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_int8:
					{
						auto fb = get_int8(index);
						fb = _member_assignment.int_value;
						break;
					}
					case jtype::type_string:
					{
						auto fb = get_string(index);
						fb = _member_assignment.int_value;
						break;
					}
				}
				break;
			case jtype::type_datetime:
				switch (fld.type_id)
				{
					case jtype::type_datetime:
					{
						auto fb = get_time(index);
						fb = _member_assignment.time_value;
					}
				}
				break;
			case jtype::type_string:
				switch (fld.type_id)
				{
					case jtype::type_string:
						{
							auto fb = get_string(index);
							fb = _member_assignment.string_value.c_str();
						}
						break;
					case jtype::type_float64:
						{
							auto fb = get_double(index);
							fb = _member_assignment.string_value.to_double();
						}
						break;
					case jtype::type_float32:
						{
							auto fb = get_float(index);
							fb = _member_assignment.string_value.to_double();
						}
						break;
					case jtype::type_int64:
						{
							auto fb = get_int64(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int32:
						{
							auto fb = get_int32(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int16:
						{
							auto fb = get_int16(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
					case jtype::type_int8:
						{
							auto fb = get_int8(index);
							fb = _member_assignment.string_value.to_long();
						}
						break;
				}
			}
		}

		int jobject::size()
		{
			auto the_class = schema->get_class(class_id);
			return the_class.size();
		}

		void jobject::update(jobject& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				std::copy(_src_slice.get_bytes(), _src_slice.get_bytes() + size_bytes(), get_bytes());
			}
			else 
			{
				relative_ptr_type fis, fid, ssf;

				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto fld_source = _src_slice.get_field(fis);
					auto fld_dest_idx = get_field_index_by_id(fld_source.field_id);
					if (fld_dest_idx != null_row)
					{
						auto& sf = _src_slice.get_class_field(fis);
						auto& df = get_class_field(fld_dest_idx);
						std::copy(_src_slice.get_bytes() + sf.offset, _src_slice.get_bytes() + sf.offset + fld_source.size_bytes, get_bytes() + df.offset);
					}
				}
			}
		}

		const char* jobject::get_name(relative_ptr_type field_id)
		{
			int idx = get_field_index_by_id(field_id);
			if (idx > null_row) {
				return get_string(idx).c_str();
			}
			return get_class().item().description;
		}
		
		std::partial_ordering  jobject::compare(int _dst_idx, jobject& _src_slice, int _src_idx)
		{
			auto field_type = get_field(_dst_idx).type_id;
			auto offset1 = get_offset(_dst_idx);
			auto offset2 = _src_slice.get_offset(_src_idx);
			char* c1 = get_bytes() + offset1;
			char* c2 = _src_slice.get_bytes() + offset2;
			return compare_express(field_type, c1, c2);
		}

		std::partial_ordering jobject::compare(jobject& _src_slice)
		{
			if (_src_slice.class_id == class_id) 
			{
				relative_ptr_type fis, fid, ssf;
				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto &fld_source = _src_slice.get_field(fis);
					auto offset1 = get_offset(fis);
					auto offset2 = _src_slice.get_offset(fis);
					char* c1 = get_bytes() + offset1;
					char* c2 = _src_slice.get_bytes() + offset2;
					auto x = compare_express(fld_source.type_id, c1, c2);
					if (x != std::strong_ordering::equal) {
						return x;
					}
				}
				return std::strong_ordering::equal;
			}
			else 
			{
				relative_ptr_type fis, fid, ssf;
				ssf = _src_slice.size();
				for (fis = 0; fis < ssf; fis++)
				{
					auto fld_idx_source = fis;
					auto& fld_source = _src_slice.get_field(fld_idx_source);
					auto fld_idx_dest = get_field_index_by_id(fld_source.field_id);

					if (fld_idx_dest == null_row) 
					{
						throw std::invalid_argument( "Invalid field index");
					}

					auto& fld_dest = get_field(fld_idx_dest);

					auto offset1 = get_offset(fld_idx_dest, fld_source.type_id);
					auto offset2 = _src_slice.get_offset(fld_idx_source, fld_dest.type_id);
					char* c1 = get_bytes() + offset1;
					char* c2 = _src_slice.get_bytes() + offset2;
					auto x = compare_express(fld_source.type_id, c1, c2);
					if (x != std::strong_ordering::equal) {
						return x;
					}
				}
				return std::strong_ordering::equal;
			}
		}

		std::partial_ordering jobject::compare(jobject& _src_slice, const std::vector<relative_ptr_type>& _join_fields)
		{
			relative_ptr_type fid;
			for (auto iter : _join_fields)
			{
				auto fld_idx_source = _src_slice.get_field_index_by_id(iter);
				auto fld_idx_dest = get_field_index_by_id(iter);

				if (fld_idx_source == null_row) 
				{
					return std::partial_ordering::less;
				}
				else if (fld_idx_dest == null_row)
				{
					return std::partial_ordering::greater;
				}

				auto& fld_source = _src_slice.get_field(fld_idx_source);
				auto& fld_dest = get_field(fld_idx_dest);

				auto offset1 = get_offset(fld_idx_dest, fld_source.type_id);
				auto offset2 = _src_slice.get_offset(fld_idx_source, fld_dest.type_id);
				char* c1 = get_bytes() + offset1;
				char* c2 = _src_slice.get_bytes() + offset2;
				auto x = compare_express(fld_source.type_id, c1, c2);
				if (x != std::strong_ordering::equal) {
					return x;
				}
			}
			return std::strong_ordering::equal;
		}


		jobject jobject::convert(serialized_box_container* _box, relative_ptr_type _class_id)
		{
			if (_class_id == class_id)
				return *this;

			auto myclass = schema->get_class(_class_id);
			auto bytes_to_allocate = myclass.item().class_size_bytes;
			_box->allocate<char>(bytes_to_allocate, location);

			jobject ja(schema, _class_id, box, location);
			ja.construct();
			ja.update(*this);

			return ja;
		}


		void jschema::add_standard_fields() 
		{
			put_string_field_request string_fields[41] = {
				{ { jtype::type_string ,"full_name", "Full Name" }, { 75, "", "" } },
				{ { jtype::type_string ,"first_name", "First Name" }, { 50, "", "" } },
				{ { jtype::type_string ,"last_name", "Last Name" }, { 50, "", "" } },
				{ { jtype::type_string ,"middle_name", "Middle Name" }, { 50, "", "" } },
				{ { jtype::type_string ,"ssn", "SSN" }, { 10, "", "" }},
				{ { jtype::type_string, "email", "eMail" }, { 200, "", ""  }},
				{ { jtype::type_string, "title", "Title" }, { 200, "", "" } },
				{ { jtype::type_string, "street", "Street" },{  200, "", "" } },
				{ { jtype::type_string, "suiteapt", "Suite/Apt" }, { 100, "", ""  }},
				{ { jtype::type_string, "city", "City" }, { 100, "", "" } },
				{ { jtype::type_string, "state", "State" }, { 100, "", "" } },
				{ { jtype::type_string, "postal", "Postal Code" }, { 50, "", ""  }},
				{ { jtype::type_string, "country_name", "Country Name" }, { 50, "", "" } },
				{ { jtype::type_string, "country_code", "Country Code" }, { 3, "", ""  }},
				{ { jtype::type_string, "institution_name", "Institution Name" }, { 100, "", "" } },
				{ { jtype::type_string, "long_name", "Long Name" }, { 200, "", ""  }},
				{ { jtype::type_string, "short_name", "Short Name" },{  50, "", ""  }},
				{ { jtype::type_string, "unit", "Unit" }, { 10, "", "" } },
				{ { jtype::type_string, "symbol", "Symbol" }, { 10, "", "" } },
				{ { jtype::type_string, "operator", "Operator" }, { 10, "", ""  }},
				{ { jtype::type_string, "windows_path", "Windows Path" }, { 512, "", ""  }},
				{ { jtype::type_string, "linux_path", "Linux Path" }, { 512, "", "" } },
				{ { jtype::type_string, "url", "Url" }, { 512, "", "" } },
				{ { jtype::type_string, "username", "User Name" }, { 100, "", ""  }},
				{ { jtype::type_string, "password", "Password" }, { 100, "", ""  }},
				{ { jtype::type_string, "doc_title", "Document Title" }, { 200, "", "" } },
				{ { jtype::type_string, "section_title", "Section Title" }, { 200, "", "" } },
				{ { jtype::type_string, "block_title", "Block Title" }, { 200, "", "" } },
				{ { jtype::type_string, "caption", "Caption" }, { 200, "", "" } },
				{ { jtype::type_string, "paragraph", "Paragraph" }, { 4000, "", "" } },
				{ { jtype::type_string, "mime_type", "MimeType" }, { 100, "", "" } },
				{ { jtype::type_string, "base64", "Base64" }, { 100, "", "" } },
				{ { jtype::type_string, "font_name", "Font" }, { 32, "", "" } },
				{ { jtype::type_string, "name", "Name" }, { 32, "", "" } },
				{ { jtype::type_string, "field_name", "Field Name" }, { 64, "", "" } },
				{ { jtype::type_string, "field_description", "Field Display Name" }, { 64, "", "" } },
				{ { jtype::type_string, "field_format", "Field Format" }, { 64, "", "" } },
				{ { jtype::type_string, "string_validation_pattern", "Validation Pattern" }, { 64, "", "" } },
				{ { jtype::type_string, "string_validation_message", "Validation Message" }, { 64, "", "" } },
				{ { jtype::type_string, "user_class_class_name", "User Class Name" }, { 64, "", "" } },
				{ { jtype::type_string, "search_string", "Search String" }, { 250, "", "" } }
			};

			put_time_field_request time_fields[4] = {
				{ { jtype::type_datetime, "birthday", "Birthday" }, 0, INT64_MAX },
				{ { jtype::type_datetime, "scheduled", "Scheduled" }, 0, INT64_MAX },
				{ { jtype::type_datetime, "date_start", "Min Date" }, 0, INT64_MAX },
				{ { jtype::type_datetime, "date_stop", "Max Date" }, 0, INT64_MAX }
			};

			put_integer_field_request int_fields[25] = {
				{ { jtype::type_int64, "count", "Count" }, 0, INT64_MAX },
				{ { jtype::type_int8, "bold", "Bold" }, 0, INT8_MAX },
				{ { jtype::type_int8, "italic", "Italic" }, 0, INT8_MAX },
				{ { jtype::type_int8, "underline", "Underline" }, 0, INT8_MAX },
				{ { jtype::type_int8, "strike_through", "Strike Through" }, 0, INT8_MAX },
				{ { jtype::type_int8, "vertical_alignment", "Vertical alignment" }, 0, INT8_MAX },
				{ { jtype::type_int8, "horizontal_alignment", "Vertical alignment" }, 0, INT8_MAX }, 
				{ { jtype::type_int8, "wrap_text", "Wrap text" }, 0, INT8_MAX },
				{ { jtype::type_int64, "int_start", "Min. Value" }, 0, INT64_MAX },
				{ { jtype::type_int64, "int_stop", "Max. Value" }, 0, INT64_MAX },
				{ { jtype::type_int64, "text_style", "Text Style" }, 0, INT64_MAX },
				{ { jtype::type_int64, "style_sheet", "Style Sheet", true }, 0, INT64_MAX },
				{ { jtype::type_int64, "user_class_root", "User Classes", true }, 0, INT64_MAX },
				{ { jtype::type_int64, "user_class", "User Class", true }, 0, INT64_MAX },
				{ { jtype::type_int64, "user_field", "User Field", true }, 0, INT64_MAX },
				{ { jtype::type_int16, "field_type", "Field Type" }, 0, INT64_MAX },
				{ { jtype::type_int64, "user_class_class_id", "User Class Id" }, 0, INT64_MAX },
				{ { jtype::type_int64, "base_class_id", "Base Class Id" }, 0, INT64_MAX },
				{ { jtype::type_int32, "string_length", "Max Length of String Field" }, 0, 1<<20 },
				{ { jtype::type_int32, "object_x", "X Dim" }, 0, INT64_MAX },
				{ { jtype::type_int32, "object_y", "Y Dim" }, 0, INT64_MAX },
				{ { jtype::type_int32, "object_z", "Z Dim" }, 0, INT64_MAX },
				{ { jtype::type_int32, "object_w", "Width" }, 0, INT64_MAX },
				{ { jtype::type_int32, "object_h", "Height" }, 0, INT64_MAX },
				{ { jtype::type_int64, "style_id", "Style Id" }, 0, INT64_MAX },
			};

			put_double_field_request double_fields[20] = {
				{ { jtype::type_float64, "quantity", "Quantity" }, -1E40, 1E40 },
				{ { jtype::type_float64, "latitude", "Latitude" }, -90, 90 },
				{ { jtype::type_float64, "longitude", "Longitude" }, -180, 180 },
				{ { jtype::type_float64, "meters", "Meters" }, -1E40, 1E40 },
				{ { jtype::type_float64, "feet", "Feet" }, -1E40, 1E40 },
				{ { jtype::type_float64, "kilograms", "Kilograms" }, -1E40, 1E40 },
				{ { jtype::type_float64, "pounds", "Pounds" }, -1E40, 1E40 },
				{ { jtype::type_float64, "seconds", "Seconds" }, -1E40, 1E40 },
				{ { jtype::type_float64, "minutes", "Minutes" }, -1E40, 1E40 },
				{ { jtype::type_float64, "hours", "Hours" }, -1E40, 1E40 },
				{ { jtype::type_float64, "amperes", "Amperes" }, -1E40, 1E40 },
				{ { jtype::type_float64, "kelvin", "Kelvin" }, -1E40, 1E40 },
				{ { jtype::type_float64, "moles", "Moles" }, -1E40, 1E40 },
				{ { jtype::type_float32, "gradient_position", "Gradient stop position" }, 0, 1E3 },
				{ { jtype::type_float32, "font_size", "Font size" }, 0, 1E3 },
				{ { jtype::type_float32, "line_spacing", "Line Spacing" }, 0, 1E3 },
				{ { jtype::type_float32, "box_border_thickness", "Box Border Thickness" }, 0, 1E3 },
				{ { jtype::type_float32, "shape_border_thickness", "Shape Border Thickness" }, 0, 1E3 },
				{ { jtype::type_float64, "double_start", "Min. Value" }, -11E30, 1E30 },
				{ { jtype::type_float64, "double_stop", "Max. Value" }, -1E30, 1E30 }
			};

			for (int i = 0; i < sizeof(string_fields) / sizeof(string_fields[0]); i++) {
				put_string_field(string_fields[i]);
			}

			for (int i = 0; i < sizeof(time_fields) / sizeof(time_fields[0]); i++) {
				put_time_field(time_fields[i]);
			}

			for (int i = 0; i < sizeof(int_fields) / sizeof(int_fields[0]); i++) {
				put_integer_field(int_fields[i]);
			}

			for (int i = 0; i < sizeof(double_fields) / sizeof(double_fields[0]); i++) {
				put_double_field(double_fields[i]);
			}
		}

		bool schema_tests()
		{
			try {
				std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
				box->init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema( box, 20, true, schema_id );

				relative_ptr_type quantity_field_id = null_row;
				relative_ptr_type last_name_field_id = null_row;
				relative_ptr_type first_name_field_id = null_row;
				relative_ptr_type birthday_field_id = null_row;
				relative_ptr_type count_field_id = null_row;
				relative_ptr_type title_field_id = null_row;
				relative_ptr_type institution_field_id = null_row;

				schema.bind_field("quantity", quantity_field_id);
				schema.bind_field("lastName", last_name_field_id);
				schema.bind_field("firstName", first_name_field_id);
				schema.bind_field("birthday", birthday_field_id);
				schema.bind_field("count", count_field_id);
				schema.bind_field("title", title_field_id);
				schema.bind_field("institutionName", institution_field_id);

				if (quantity_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed:" << std::endl;
					return false;
				}

				if (last_name_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (first_name_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (birthday_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (title_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				if (count_field_id == null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				relative_ptr_type failed_field_id = schema.find_class("badFieldName");

				if (failed_field_id != null_row) {
					std::cout << __LINE__ << ":find row failed" << std::endl;
					return false;
				}

				put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, title_field_id, count_field_id, quantity_field_id };
				relative_ptr_type person_class_id = schema.put_class(person);

				if (person_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				jclass person_class = schema.get_class(person_class_id);

				if (person_class.size() != 6) {
					std::cout << __LINE__ << ":class size failed failed" << std::endl;
					return false;
				}

				int offset_start = 0;
				for (int i = 0; i < person_class.size(); i++) {
					auto& fldref = person_class.detail(i);
					auto& fld = schema.get_field(fldref.field_id);
					//				std::cout << fld.name << " " << fld.description << " " << fldref.offset << " " << fld.size_bytes << std::endl;
					if (offset_start && offset_start != fldref.offset) {
						std::cout << __LINE__ << ":class alignment failed" << std::endl;

					}
					offset_start += fld.size_bytes;
				}

			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}

			return true;
		}

		bool collection_tests()
		{

			try {

				corona::database::application app;

				std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
				box->init(1 << 21);

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema(box, 20, true, schema_id);

				relative_ptr_type quantity_field_id = null_row;
				relative_ptr_type last_name_field_id = null_row;
				relative_ptr_type first_name_field_id = null_row;
				relative_ptr_type birthday_field_id = null_row;
				relative_ptr_type count_field_id = null_row;
				relative_ptr_type title_field_id = null_row;
				relative_ptr_type institution_field_id = null_row;

				schema.bind_field("quantity", quantity_field_id);
				schema.bind_field("lastName", last_name_field_id);
				schema.bind_field("firstName", first_name_field_id);
				schema.bind_field("birthday", birthday_field_id);
				schema.bind_field("count", count_field_id);
				schema.bind_field("title", title_field_id);
				schema.bind_field("institutionName", institution_field_id);

				jcollection_ref ref;
				ref.data = std::make_shared<persistent_box>();
				ref.data->create(&app, "test.dat");
				ref.max_objects = 50;
				ref.collection_size_bytes = 1 << 19;

				init_collection_id(ref.collection_id);

				jcollection people = schema.create_collection(&ref);

				put_class_request person;

				person.class_name = "person";
				person.class_description = "a person";
				person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, count_field_id, quantity_field_id };
				relative_ptr_type person_class_id = schema.put_class(person);

				if (person_class_id == null_row)
				{
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				int birthdaystart = 1941;
				int countstart = 12;
				double quantitystart = 10.22;
				int increment = 5;

				relative_ptr_type people_object_id;

				auto sl = people.create_object(person_class_id, people_object_id);
				auto last_name = sl.get_string(0);
				auto first_name = sl.get_string(1);
				auto birthday = sl.get_time(2);
				auto count = sl.get_int64(3);
				auto qty = sl.get_double(4);
				last_name = "last 1";
				first_name = "first 1";
				birthday = birthdaystart + increment * 0;
				count = countstart + increment * 0;
				qty = quantitystart + increment * 0;

				sl = people.create_object(person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 2";
				first_name = "first 2";
				birthday = birthdaystart + increment * 1;
				count = countstart + increment * 1;
				qty = quantitystart + increment * 1;

				sl = people.create_object(person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 3";
				first_name = "first 3";
				birthday = birthdaystart + increment * 2;
				count = countstart + increment * 2;
				qty = quantitystart + increment * 2;

				sl = people.create_object(person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 4";
				first_name = "first 4";
				birthday = birthdaystart + increment * 3;
				count = countstart + increment * 3;
				qty = quantitystart + increment * 3;

				sl = people.create_object(person_class_id, people_object_id);
				last_name = sl.get_string(0);
				first_name = sl.get_string(1);
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);
				last_name = "last 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
				first_name = "first 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
				birthday = birthdaystart + increment * 4;
				count = countstart + increment * 4;
				qty = quantitystart + increment * 4;

				int inc_count = 0;

				for (auto sl : people)
				{
					last_name = sl.item.get_string(0);
					if (!last_name.starts_with("last")) {
						std::cout << __LINE__ << ":last name failed" << std::endl;
						return false;
					}
					first_name = sl.item.get_string(1);
					if (!first_name.starts_with("first")) {
						std::cout << __LINE__ << ":first name failed" << std::endl;
						return false;
					}
					birthday = sl.item.get_time(2);
					count = sl.item.get_int64(3);
					qty = sl.item.get_double(4);

					if (birthday != birthdaystart + increment * inc_count) {
						std::cout << __LINE__ << ":birthday failed" << std::endl;
						return false;
					}

					if (count != countstart + increment * inc_count) {
						std::cout << __LINE__ << ":count failed" << std::endl;
						return false;
					}

					if (qty != quantitystart + increment * inc_count) {

						std::cout << __LINE__ << ":qty failed" << std::endl;
						return false;
					}

					inc_count++;
				}

				return true;
			}
			catch (std::exception exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}
			
		}

		bool array_tests()
		{

			try {
				std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
				box->init(1 << 21);
				corona::database::application app;

				jschema schema;
				relative_ptr_type schema_id;

				schema = jschema::create_schema(box, 50, true, schema_id);

				put_class_request sprite_frame_request;

				sprite_frame_request.class_name = "spriteframe";
				sprite_frame_request.class_description = "sprite frame";
				sprite_frame_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h", "color" };
				relative_ptr_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

				if (sprite_frame_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				put_class_request sprite_class_request;
				sprite_class_request.class_name = "sprite";
				sprite_class_request.class_description = "sprite";
				sprite_class_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h" , member_field(sprite_frame_class_id) };
				relative_ptr_type sprite_class_id = schema.put_class(sprite_class_request);

				if (sprite_class_id == null_row) {
					std::cout << __LINE__ << ":class create failed failed" << std::endl;
					return false;
				}

				collection_id_type colid;

				init_collection_id(colid);

				relative_ptr_type classesb[2] = { sprite_class_id, null_row };

				jcollection_ref ref;
				ref.data = std::make_shared<persistent_box>();
				ref.data->create(&app, "test.dat");
				ref.max_objects = 50;
				ref.collection_size_bytes = 1 << 19;

				init_collection_id(ref.collection_id);

				jcollection sprites = schema.create_collection(&ref);

				for (int i = 0; i < 10; i++) {

					relative_ptr_type new_sprite_id;
					auto slice = sprites.create_object(sprite_class_id, new_sprite_id );
					auto image_name = slice.get_string(0);


					auto bx = slice.get_int32("object_x");
					auto by = slice.get_int32("object_y");
					auto bw = slice.get_int32("object_w");
					auto bh = slice.get_int32("object_h");

					rectangle image_rect;

					image_name = std::format("{} #{}", "image", i);
					bx = 0;
					by = 0;
					bw = 1000;
					bh = 1000;

#if _DETAIL
					std::cout << "before:" << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

				}

				int scount = 0;

				for (auto slice : sprites)
				{
					auto image_name = slice.item.get_string(0);
					auto bx = slice.item.get_int32("object_x");
					auto by = slice.item.get_int32("object_y");
					auto bw = slice.item.get_int32("object_w");
					auto bh = slice.item.get_int32("object_h");

#if _DETAIL
					std::cout << image_name << std::endl;
					std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

					if (bw != 1000 || bh != 1000) {

						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}

				}

				return true;
			}
			catch (std::exception& exc)
			{
				std::cout << exc.what() << std::endl;
				return false;
			}
		}

	}
}
