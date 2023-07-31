

#pragma once

#include "corona.h"

#define ACTOR_OBJECT_CHECKING 0

namespace corona
{
	namespace database
	{

		bool init_collection_id(collection_id_type& collection_id);

		class jcollection_ref
		{
		public:

			object_name			collection_name;
			object_path			collection_file_name;
			collection_id_type	collection_id;

			relative_ptr_type	collection_location;

			object_name			model_name;
			uint32_t			max_objects;
			uint64_t			collection_size_bytes;

			relative_ptr_type	objects_id;

			std::shared_ptr<persistent_box>		data;

			jcollection_ref() : 
				collection_name(""),
				collection_file_name(""),
				collection_location(null_row),
				model_name(""),
				max_objects(0),
				collection_size_bytes(0),
				objects_id(null_row),
				data(nullptr)
			{
				;
			}
		};

		struct jschema_map
		{
			block_id		  block;
			relative_ptr_type fields_table_id;
			relative_ptr_type classes_table_id;
			relative_ptr_type classes_by_name_id;
			relative_ptr_type models_by_name_id;
			relative_ptr_type fields_by_name_id;
			relative_ptr_type models_id;
		};

		class jschema;

		class jobject
		{
			const jschema *schema;
			relative_ptr_type class_id;
			char* bytes;
			jclass the_class;

			serialized_box_container *box;
			relative_ptr_type location;

			size_t get_offset(int field_idx, jtype _type = jtype::type_null);

			template <typename T> T get_boxed(int field_idx, jtype _type = jtype::type_null)
			{
				size_t offset = get_offset(field_idx, _type);
				T b = get_bytes() + offset;
				return b;
			}

			template <typename T> T get_boxed_ex(int field_idx, jtype _type = jtype::type_null)
			{
				size_t offset = get_offset(field_idx);
				T b(get_bytes() + offset, schema, &the_class, this, field_idx);
				return b;
			}

			std::partial_ordering compare_express(jtype _type, char* c1, char* c2);

		public:

			jobject();
			jobject(const jschema *_schema, relative_ptr_type _class_id, char* _bytes);
			jobject(const jschema *_schema, relative_ptr_type _class_id, serialized_box_container *_box, relative_ptr_type _location);

			jobject(const jobject& src);
			jobject operator =(const jobject& src);

			jobject(jobject&& src);
			jobject& operator =(jobject&& src);

			void construct();

			jclass get_class() const;
			const jschema *get_schema();

			relative_ptr_type get_class_id() const 
			{
				return class_id;
			}

			relative_ptr_type get_base_class_id() const 
			{
				return get_class().pitem()->base_class_id;
			}

			int get_field_index_by_name(const object_name& name);
			int get_field_index_by_id(relative_ptr_type field_id);
			const jfield& get_field_by_id(relative_ptr_type field_id)  const;
			const jclass_field& get_class_field(int field_idx) const;
			const jfield& get_field(int field_idx)  const;
			relative_ptr_type get_primary_key();
			relative_ptr_type get_primary_key_value();
			const char* get_name(relative_ptr_type field_id);

			bool has_field(const object_name& name);
			bool has_field(relative_ptr_type field_id);
			bool is_class(relative_ptr_type class_id);

			int8_box get_int8(int field_idx, bool _use_id = false);
			int16_box get_int16(int field_idx, bool _use_id = false);
			int32_box get_int32(int field_idx, bool _use_id = false);
			int64_box get_int64(int field_idx, bool _use_id = false);
			float_box get_float(int field_idx, bool _use_id = false);
			double_box get_double(int field_idx, bool _use_id = false);
			time_box get_time(int field_idx, bool _use_id = false);
			string_box get_string(int field_idx, bool _use_id = false);
			image_box get_image(int field_idx, bool _use_id = false);
			wave_box get_wave(int field_idx, bool _use_id = false);
			midi_box get_midi(int field_idx, bool _use_id = false);

			int8_box get_int8(object_name field_name);
			int16_box get_int16(object_name field_name);
			int32_box get_int32(object_name field_name);
			int64_box get_int64(object_name field_name);
			float_box get_float(object_name field_name);
			double_box get_double(object_name field_name);
			time_box get_time(object_name field_name);
			string_box get_string(object_name field_name);
			image_box get_image(object_name field_name);
			wave_box get_wave(object_name field_name);
			midi_box get_midi(object_name field_name);

			bool matches(const char* str);

			void set_value(const jvariant& _member_assignment);

			void set(std::initializer_list<jvariant> var)
			{
				for (auto item : var) {
					set_value(item);
				}
			}

			jvariant get(relative_ptr_type field_id);
			jvariant operator[](relative_ptr_type field_idx);

			void update(jobject& _src_slice);

			std::partial_ordering compare(int _dst_idx, jobject& _src_slice, int _src_idx);
			std::partial_ordering compare(jobject& _src_slice);
			std::partial_ordering compare(jobject& _src_slice, const std::vector<relative_ptr_type>& join_fields);

			jobject convert(serialized_box_container* _box, relative_ptr_type _class_id);

			template <typename boxed> boxed get(int field_idx)
			{
				size_t offset = get_offset(field_idx);
				char *src = get_bytes() + offset;
				boxed temp(src);
				return temp;
			}

			int size();
			char* get_bytes() { return box ? box->get_object<char>(location) : bytes;  };
			bool is_null() {
				return get_bytes() == nullptr;
			}
			relative_ptr_type size_bytes() { return get_class().item().class_size_bytes; };

			std::partial_ordering operator<=>(jobject& src) {
				return compare(src);
			}
		};


		class collection_object_type
		{
		public:
			object_id_type			oid;
			jtype					otype;
			relative_ptr_type		class_id;
			time_t					last_modified;
			bool					deleted;
		};

		class create_object_request
		{
		public:
			collection_id_type	collection_id;
			relative_ptr_type	class_id;
			relative_ptr_type	item_id;
			relative_ptr_type	template_item_id;
			bool				select_on_create;
			object_name			create_prompt;
			collection_id_type get_collection_id() { return collection_id; }
		};

		using object_collection = item_details_table<collection_object_type, char>;

		class jcollection
		{

			jschema* schema;
			jcollection_ref* ref;

			collection_id_type		collection_id;
			object_collection		objects;

		public:


			using collection_type = jcollection;
			using iterator_item_type = value_object<jobject>;
			using iterator_type = filterable_iterator<jobject, collection_type, iterator_item_type>;

			jcollection() : schema( nullptr ), ref( nullptr )
			{
				;
			}

			jcollection(jschema* _schema, jcollection_ref *_ref) :
				schema(_schema),
				ref(_ref),
				collection_id(_ref->collection_id)
			{
				if (!ref || ref->data == nullptr) {
					throw std::invalid_argument("jcollection ref must have data initialized");
				}
				objects = object_collection::get_table(_ref->data, _ref->objects_id );
			}

			jcollection(jcollection& _src) :
				schema(_src.schema),
				ref(_src.ref),
				collection_id(_src.collection_id)
			{
				if (!ref || ref->data == nullptr) {
					throw std::invalid_argument("jcollection ref must have data initialized");
				}
				objects = object_collection::get_table(ref->data, ref->objects_id);
			}

			jcollection(jcollection&& _src) :
				schema(_src.schema),
				ref(_src.ref),
				collection_id(_src.collection_id)
			{
				if (!ref || ref->data == nullptr) {
					throw std::invalid_argument("jcollection ref must have data initialized");
				}
				objects = object_collection::get_table(ref->data, ref->objects_id);
			}

			jcollection operator =(jcollection&& _src) 
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			jcollection operator =(jcollection& _src)
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			collection_id_type get_collection_id()
			{
				return collection_id;
			}

			jobject create_object(relative_ptr_type _class_id, relative_ptr_type& _object_id);
			jobject create_object(relative_ptr_type _class_id, relative_ptr_type& _object_id, std::initializer_list<jvariant> var);
			jobject get_object(relative_ptr_type _object_id);
			jobject update_object(relative_ptr_type _object_id, jobject _slice);
			collection_object_type &get_object_reference(relative_ptr_type _object_id);

			jobject get_at(relative_ptr_type _object_id);
			relative_ptr_type get_class_id(relative_ptr_type _object_id);
			relative_ptr_type get_base_id(relative_ptr_type _object_id);

			bool object_is_class(relative_ptr_type _object_id, relative_ptr_type _class_id);
			bool object_is_class(const jobject& obj, relative_ptr_type _class_id);
			bool object_is_class(relative_ptr_type _object_id, std::vector<relative_ptr_type> _class_ids);
			bool object_is_class(relative_ptr_type _object_id, class_list& _class_ids);
			bool object_is_class(const jobject& obj, std::vector<relative_ptr_type> _class_ids);
			bool object_is_class(const jobject& obj, class_list& _class_ids);

			bool class_has_base(relative_ptr_type _class_id, relative_ptr_type _base_id);
			int64_t get_class_count(relative_ptr_type _class_id);

			relative_ptr_type size()
			{
				return objects.size();
			}

			iterator_type begin()
			{
				return iterator_type(this, first_row).where([this](const iterator_item_type& it) {
					return objects[it.location].item().deleted == false;
					});
			}

			iterator_type end()
			{
				return iterator_type(this, null_row);
			}

			auto where(std::function<bool(const iterator_item_type&)> _predicate)
			{
				return iterator_type(this, [this](const iterator_item_type& it) {
					return objects[it.location].item().deleted == false;					
					})
					.where( _predicate );
			}

			auto where(relative_ptr_type _class_id)
			{
				return iterator_type(this, [this, _class_id](const iterator_item_type& it) { return objects[it.location].item().deleted == false && object_is_class(it.location, _class_id); });
			}

			jobject first_value(std::function<bool(const iterator_item_type&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					throw std::logic_error("sequence has no elements");
				}
				return w.get_object().item;
			}

			relative_ptr_type first_index(std::function<bool(const iterator_item_type&)> predicate)
			{
				auto w = this->where(predicate);
				if (w == end()) {
					return null_row;
				}
				return w.get_object().location;
			}

			bool any_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::any_of(begin(), end(), predicate);
			}

			bool all_of(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::all_of(begin(), end(), predicate);
			}

			corona_size_t count_if(std::function<bool(const iterator_item_type&)> predicate)
			{
				return std::count_if(begin(), end(), predicate);
			}

		};

		class jschema
		{

		protected:

			using field_store_type = table<jfield>;
			using class_store_type = item_details_table<jclass_header, jclass_field>;
			using class_index_type = sorted_index<object_name, relative_ptr_type>;
			using field_index_type = sorted_index<object_name, relative_ptr_type>;

			field_store_type		fields;
			class_store_type		classes;
			class_index_type		classes_by_name;
			field_index_type		fields_by_name;

			jfield					empty;

		public:	

			jschema() = default;
			~jschema() = default;

			const jfield &get_empty() const
			{
				return empty;
			}

			virtual void on_update(jfield& f) { ; }
			virtual void on_update(jclass& f) { ; }

			std::vector<relative_ptr_type> get_classes_with_primary_key(relative_ptr_type _field_id)
			{
				std::vector<relative_ptr_type> classes;

				for (int i = 0; i < classes.size(); i++)
				{
					auto pk = get_class_primary_key(i);
					if (pk == _field_id) {
						classes.push_back(i);
					}
				}
				return classes;
			}

			static relative_ptr_type reserve_schema(std::shared_ptr<serialized_box_container> _b, int _num_classes, int _num_fields, int _total_class_fields, bool _use_standard_fields)
			{
				jschema_map schema_map, *pschema_map;
				schema_map.block = block_id::collection_id();
				schema_map.fields_table_id = null_row;
				schema_map.classes_table_id = null_row;
				schema_map.classes_by_name_id = null_row;
				schema_map.fields_by_name_id = null_row;

				relative_ptr_type rit = _b->put_object(schema_map);
				schema_map.fields_table_id = field_store_type::reserve_table(_b, _num_fields);
				schema_map.classes_table_id = class_store_type::reserve_table(_b, _num_classes, _total_class_fields);
				schema_map.classes_by_name_id = class_index_type::reserve_sorted_index(_b);
				schema_map.fields_by_name_id = field_index_type::reserve_sorted_index(_b);
				pschema_map = _b->get_object<jschema_map>(rit);
				*pschema_map = schema_map;
				return rit;
			} 

			static jschema get_schema(std::shared_ptr<serialized_box_container> _b, relative_ptr_type _row)
			{
				jschema schema;
				jschema_map* pschema_map;
				pschema_map = _b->get_object<jschema_map>(_row);
				if (!pschema_map->block.is_collection()) {
					throw std::logic_error("collection read wrong");
				}
				schema.fields = field_store_type::get_table(_b, pschema_map->fields_table_id);
				schema.classes = class_store_type::get_table(_b, pschema_map->classes_table_id);
				schema.classes_by_name = class_index_type::get_sorted_index(_b, pschema_map->classes_by_name_id);
				schema.fields_by_name = field_index_type::get_sorted_index(_b, pschema_map->fields_by_name_id);
				schema.empty.field_id = null_row;
				schema.empty.type_id = jtype::type_null;
				schema.empty.name = "empty";
				schema.empty.description = "indicates a field not found";
				return schema;
			}

			static int64_t get_box_size(int _num_classes, int _num_fields, int _total_class_fields, bool _use_standard_fields)
			{
				int64_t field_size = field_store_type::get_box_size(_num_fields);
				int64_t class_size = class_store_type::get_box_size(_num_classes, _total_class_fields);
				int64_t classes_by_name_size = class_index_type::get_box_size();
				int64_t fields_by_name_size = field_index_type::get_box_size();
				int64_t total_size = field_size +
					class_size +
					classes_by_name_size +
					fields_by_name_size;
				return total_size;
			}

			static jschema create_schema(std::shared_ptr<serialized_box_container> _b, int _num_classes, bool _use_standard_fields, relative_ptr_type& _location)
			{
				int _num_fields = _num_classes * 10 + (_use_standard_fields ? 200 : 0);
				int _total_class_fields = _num_classes * 100;
				auto total_size = jschema::get_box_size(_num_classes, _num_fields, _total_class_fields, _use_standard_fields);
				_location = reserve_schema(_b, _num_classes, _num_fields, _total_class_fields, _use_standard_fields);
				jschema schema = get_schema(_b, _location);
				schema.add_standard_fields();
				return schema;
			}

			void add_standard_fields();

			bool is_empty(jfield& fld)
			{
				return fld.field_id == null_row && fld.type_id == jtype::type_null;
			}

			relative_ptr_type new_field_id()
			{
				relative_ptr_type field_id;
				fields.create(1, field_id);
				return field_id;
			}

			relative_ptr_type put_field(put_field_request_base& _base, int64_t _size_bytes, std::function<void(jfield& _set_options)> _options)
			{
				relative_ptr_type field_id = find_field(_base.name);

				if (_base.field_id == null_row)
				{
					_base.field_id = new_field_id();
				}

				auto& jf = fields[_base.field_id];

				jf.field_id = _base.field_id;
				jf.type_id = _base.type_id;
				jf.name = _base.name;
				jf.description = _base.description;
				jf.enumeration_class_id = _base.enumeration_class_id;
				jf.enumeration_display_field_id = _base.enumeration_display_field_id;
				jf.enumeration_value_field_id = _base.enumeration_value_field_id;
				jf.size_bytes = _size_bytes;
				jf.is_key = _base.is_key;

				_options(jf);

				fields_by_name.insert_or_assign(jf.name, jf.field_id);

				return jf.field_id;
			}

			layout_rect get_layout(relative_ptr_type _field_idx);

			relative_ptr_type put_string_field(put_string_field_request request)
			{
				request.name.type_id = jtype::type_string;
				return put_field(request.name, string_box::get_box_size(request.options.length), [request](jfield& _field)
					{
						_field.string_properties = request.options;
					});
			}

			relative_ptr_type put_time_field(put_time_field_request request)
			{
				request.name.type_id = jtype::type_datetime;
				return put_field(request.name, sizeof(DATE), [request](jfield& _field)
					{
						_field.time_properties = request.options;
					});
			} 

			relative_ptr_type put_integer_field(put_integer_field_request request)
			{
				int size_bytes = 0;
				switch (request.name.type_id)
				{
				case jtype::type_int8:
					size_bytes = sizeof(int8_t);
					break;
				case jtype::type_int16:
					size_bytes = sizeof(int16_t);
					break;
				case jtype::type_int32:
					size_bytes = sizeof(int32_t);
					break;
				case jtype::type_int64:
					size_bytes = sizeof(int64_t);
					break;
				default:
					throw std::invalid_argument("Invalid integer type for field name:" + request.name.name);
				}
				return put_field(request.name, size_bytes, [request](jfield& _field)
					{
						_field.int_properties = request.options;
					});

			}

			relative_ptr_type put_double_field(put_double_field_request request)
			{
				int size_bytes = 0;
				switch (request.name.type_id)
				{
				case type_float32:
					size_bytes = sizeof(float);
					break;
				case type_float64:
					size_bytes = sizeof(double);
					break;
				default:
					throw std::invalid_argument("Invalid floating point type for field name:" + request.name.name);
				}
				return put_field(request.name, size_bytes, [request](jfield& _field)
					{
						_field.double_properties = request.options;
					});
			}

			const char* invalid_comparison = "Invalid comparison";
			const char* invalid_parameter_field = "Invalid parameter field";
			const char* invalid_target_field = "Invalid target field";
			const char* invalid_projection_field = "Invalid projection field";

			void bind_field(const char *_field_name, relative_ptr_type& _field_id)
			{
				object_name fn = _field_name;
				bind_field(fn, _field_id);
			}

			void bind_field(object_name& _field_name, relative_ptr_type& _field_id)
			{
				if (_field_id != null_row)
				{
					_field_name = fields[_field_id].name;
					return;
				}

				_field_id = fields_by_name[_field_name].second;
			}

			void bind_class(object_name& _class_name, relative_ptr_type& _class_id, bool _optional = false)
			{
				if (_optional && _class_name.size() == 0 && _class_id == null_row)
					return;

				if (_class_id != null_row)
				{
					_class_name = classes[_class_id].pitem()->name;
					return;
				}

				auto fiter = classes_by_name[_class_name].second;
			}

			void bind_class(const char* _class_name, relative_ptr_type& _class_id)
			{
				object_name fn = _class_name;
				bind_class(fn, _class_id);
			}

			relative_ptr_type put_image_field(put_image_field_request request)
			{
				request.name.type_id = jtype::type_image;
				auto query_location = put_field(request.name, sizeof(image_instance), [request](jfield& _field)
					{
						_field.image_properties = request.options;
					});
				return query_location;
			}

			relative_ptr_type put_wave_field(put_wave_field_request request)
			{
				request.name.type_id = jtype::type_wave;
				auto query_location = put_field(request.name, sizeof(wave_instance), [request](jfield& _field)
					{
						_field.wave_properties = request.options;
					});
				return query_location;
			}

			relative_ptr_type put_midi_field(put_midi_field_request request)
			{
				request.name.type_id = jtype::type_midi;
				auto query_location = put_field(request.name, sizeof(midi_instance), [request](jfield& _field)
					{
						_field.midi_properties = request.options;
					});
				return query_location;
			}

			relative_ptr_type put_currency_field(put_currency_field_request request)
			{
				request.name.type_id = jtype::type_currency;
				auto query_location = put_field(request.name, sizeof(CY), [request](jfield& _field)
					{
						_field.currency_properties = request.options;
					});
				return query_location;
			}


			void build_class_members(field_array& pcr, int64_t& class_size_bytes, member_field_collection &mfs)
			{
				int field_idx = 0;
				corona_size_t sz = mfs.size();
				class_size_bytes = 0;

				for (int i = 0; i < sz; i++)
				{
					auto& field = mfs[i];

					relative_ptr_type fid;
					if (field.field_name.size()>0) {
						fid = fields_by_name[field.field_name].second;
					}
					else
					{
						fid = field.field_id;
					}
					jclass_field *ref = pcr.append();
					ref->field_id = fid;
				}

				for (int i = 0; i < pcr.size(); i++)
				{
					auto& fld = pcr[i];
					fld.offset = class_size_bytes;
					class_size_bytes += get_field(fld.field_id).size_bytes;
				}
			}

			relative_ptr_type put_class(put_class_request request)
			{
				relative_ptr_type class_id = find_class(request.class_name);
				request.class_id = class_id;

				if (request.template_class_id < 0)
					request.template_class_id = null_row;
				if (request.base_class_id < 0)
					request.base_class_id = null_row;
				if (request.field_id_primary_key < 0)
					request.field_id_primary_key = null_row;

				if (request.auto_primary_key) 
				{
					put_integer_field_request pifr;
					pifr.name.type_id = jtype::type_int64;
					pifr.name.name = "key_" + request.class_name;
					pifr.name.description = "primary key for " + request.class_name;
					pifr.name.is_key = true;
					pifr.options.minimum_int = INT64_MIN;
					pifr.options.maximum_int = INT64_MAX;
					request.field_id_primary_key = put_integer_field(pifr);
					request.member_fields.push_back(request.field_id_primary_key);
				}

				auto& mfs = request.member_fields;

				for (auto mfi : mfs)
				{
					if (mfi.item.field_name.size() > 0) {
						bind_field(mfi.item.field_name, mfi.item.field_id);
					}
				}

				auto sz = mfs.size();
				field_array af;
				int64_t total_size_bytes = 0;

				member_field_collection temp_fields;
				
				jclass_field pk_field = { null_row, 0 };

				if (request.base_class_id != null_row)
				{
					relative_ptr_type base_class_id = request.base_class_id;

					jclass cls = get_class(base_class_id);

					for (int i = 0; i < cls.size(); i++)
					{
						auto base_field = cls.detail(i);
						af.push_back(base_field);
					}
				}

				temp_fields += mfs;

				build_class_members(af, total_size_bytes, temp_fields);

#if _DEBUG && false
				std::cout << std::endl << request.class_name << " layout" << std::endl;
				for (auto f : af)
				{
					auto& fld = fields[f.item.field_id];
					std::cout << " " << f.item.field_id << ") " << fld.name << " " << f.item.offset << " " << fld.size_bytes << std::endl;
				}
#endif

				auto pcr = classes.put_item(class_id, nullptr, af.size(), af.data);

				auto& p = pcr.item();
				p.class_id = class_id;
				p.name = request.class_name;
				p.description = request.class_description;
				p.class_size_bytes = total_size_bytes;
				p.primary_key_idx = -1;
				p.base_class_id = request.base_class_id;
				p.template_class_id = request.template_class_id;
				p.create_prompt = request.create_prompt;

				for (int i = 0; i < pcr.size(); i++)
				{
					auto field_id = pcr.detail(i).field_id;
					if (field_id == request.field_id_primary_key) {
						p.primary_key_idx = i;
						auto fld = get_field(field_id);
						if (!fld.is_key)
						{
							throw std::invalid_argument("Primary key field specified is not marked as a key field");
						}
					}
				}

				return class_id;
			}

			bool class_has_field(relative_ptr_type class_id, relative_ptr_type field_id)
			{
				auto cls = classes[class_id];
				for (int i = 0; i < cls.size(); i++)
				{
					if (cls.detail(i).field_id == field_id) return true;
				}
				return false;
			}

			relative_ptr_type get_primary_key(relative_ptr_type class_id)
			{
				auto cls = classes[class_id];
				relative_ptr_type key_idx = cls.item().primary_key_idx;
				if (key_idx < 0) return null_row;
				return cls.detail(key_idx).field_id;
			}

			relative_ptr_type find_class(const object_name& class_name) const
			{
				return classes_by_name.contains(class_name) ? classes_by_name[class_name].second : null_row;
			}

			relative_ptr_type find_field(const object_name& field_name)
			{
				return fields_by_name.contains(field_name) ? fields_by_name[field_name].second : null_row;
			}

			jclass get_class(relative_ptr_type class_id) const
			{
				auto the_class = classes[class_id];
				return the_class;
			}

			relative_ptr_type get_class_primary_key(relative_ptr_type class_id)
			{
				auto the_class = classes[class_id];
				relative_ptr_type pk_index = the_class.item().primary_key_idx;
				if (pk_index < 0)
					return null_row;
				relative_ptr_type pk_id = the_class.detail(pk_index).field_id;
				return pk_id;
			}

			jfield &get_field(relative_ptr_type field_id) const
			{
				auto& the_field = fields[field_id];
				return the_field;
			}

			uint64_t get_max_object_size(relative_ptr_type* _class_ids)
			{

				if (!_class_ids)
				{
					return default_collection_object_size;
				}

				int max_size = 0;
				while (*_class_ids != null_row)
				{
					auto myclassfield = get_class(*_class_ids);
					if (myclassfield.item().class_size_bytes > max_size) 
					{
						max_size = myclassfield.item().class_size_bytes;
					}
					_class_ids++;
				}

				if (max_size == 0)
				{
					max_size = default_collection_object_size;
				}

				return max_size;
			}

			bool reserve_collection(jcollection_ref *ref)
			{
				uint64_t total_size = ref->collection_size_bytes;

				ref->objects_id =  object_collection::reserve_table(ref->data, ref->max_objects, total_size, true);

				return ref->objects_id != null_row;
			}

			jcollection get_collection(jcollection_ref* ref)
			{
				jcollection collection(this, ref);
				return collection;
			}

			jcollection create_collection(jcollection_ref* ref)
			{
				bool reserved = reserve_collection(ref);
				jcollection tmp;
				if (reserved) {
					tmp = get_collection(ref);
				}
				return tmp;
			}
		};

		bool schema_tests();
		bool collection_tests();
		bool array_tests();
	}
}
