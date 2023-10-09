#ifndef CORONA_JOBJECT_H
#define CORONA_JOBJECT_H

#include "corona-windows-lite.h"

#include "corona-constants.hpp"
#include "corona-store_box.hpp"
#include "corona-color_box.hpp"
#include "corona-float_box.hpp"
#include "corona-function.hpp"
#include "corona-int_box.hpp"
#include "corona-layout_rect_box.hpp"
#include "corona-list_box.hpp"
#include "corona-midi_box.hpp"
#include "corona-point_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-string_box.hpp"
#include "corona-image_box.hpp"
#include "corona-time_box.hpp"
#include "corona-wave_box.hpp"
#include "corona-collection_id_box.hpp"
#include "corona-jfield.hpp"
#include "corona-jvalue.hpp"
#include "corona-store_box_file.hpp"

#include <memory>
#include <exception>
#include <compare>
#include <vector>
#include <initializer_list>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <format>

namespace corona {

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
		const jschema* schema;
		relative_ptr_type class_id;
		char* bytes;
		jclass the_class;

		serialized_box_container* box;
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
		jobject(const jschema* _schema, relative_ptr_type _class_id, char* _bytes);
		jobject(const jschema* _schema, relative_ptr_type _class_id, serialized_box_container* _box, relative_ptr_type _location);

		jobject(const jobject& src);
		jobject operator =(const jobject& src);

		jobject(jobject&& src);
		jobject& operator =(jobject&& src);

		void construct();

		jclass get_class() const;
		const jschema* get_schema();

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
			char* src = get_bytes() + offset;
			boxed temp(src);
			return temp;
		}

		int size();
		char* get_bytes() { return box ? box->get_object<char>(location) : bytes; };
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

		jcollection() : schema(nullptr), ref(nullptr)
		{
			;
		}

		jcollection(jschema* _schema, jcollection_ref* _ref) :
			schema(_schema),
			ref(_ref),
			collection_id(_ref->collection_id)
		{
			if (!ref || ref->data == nullptr) {
				throw std::invalid_argument("jcollection ref must have data initialized");
			}
			objects = object_collection::get_table(_ref->data, _ref->objects_id);
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
		collection_object_type& get_object_reference(relative_ptr_type _object_id);

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
				.where(_predicate);
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

		jfield					active;

	public:

		jschema() = default;
		~jschema() = default;

		const jfield& get_empty() const
		{
			return active;
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
			jschema_map schema_map, * pschema_map;
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
			schema.active.field_id = null_row;
			schema.active.type_id = jtype::type_null;
			schema.active.name = "empty";
			schema.active.description = "indicates a field not found";
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

		void bind_field(const char* _field_name, relative_ptr_type& _field_id)
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


		void build_class_members(field_array& pcr, int64_t& class_size_bytes, member_field_collection& mfs)
		{
			int field_idx = 0;
			corona_size_t sz = mfs.size();
			class_size_bytes = 0;

			for (int i = 0; i < sz; i++)
			{
				auto& field = mfs[i];

				relative_ptr_type fid;
				if (field.field_name.size() > 0) {
					fid = fields_by_name[field.field_name].second;
				}
				else
				{
					fid = field.field_id;
				}
				jclass_field* ref = pcr.append();
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

		jfield& get_field(relative_ptr_type field_id) const
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

		bool reserve_collection(jcollection_ref* ref)
		{
			uint64_t total_size = ref->collection_size_bytes;

			ref->objects_id = object_collection::reserve_table(ref->data, ref->max_objects, total_size, true);

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

	bool init_collection_id(collection_id_type& collection_id)
	{
		GUID gidReference;
		HRESULT hr = ::CoCreateGuid((GUID*)&collection_id);
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

		auto new_object = objects.create_item(&co, total_bytes, nullptr);
#if	_TRACE_GET_OBJECT
		std::cout << "created obj, objects size:" << objects.size() << " collections size:" << size() << std::endl;
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

	collection_object_type& jcollection::get_object_reference(relative_ptr_type _object_id)
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
			sma = jvariant(_field_idx, (int64_t)get_int8(_field_idx));
			break;
		case jtype::type_int16:
			sma = jvariant(_field_idx, (int64_t)get_int16(_field_idx));
			break;
		case jtype::type_int32:
			sma = jvariant(_field_idx, (int64_t)get_int32(_field_idx));
			break;
		case jtype::type_int64:
			sma = jvariant(_field_idx, (int64_t)get_int64(_field_idx));
			break;
		case jtype::type_float32:
			sma = jvariant(_field_idx, (double)get_float(_field_idx));
			break;
		case jtype::type_float64:
			sma = jvariant(_field_idx, (double)get_double(_field_idx));
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

	jobject::jobject(const jschema* _schema, relative_ptr_type _class_id, char* _bytes)
		:
		schema(_schema),
		class_id(_class_id),
		bytes(_bytes),
		box(nullptr),
		location(null_row)
	{
		the_class = schema->get_class(_class_id);
	}

	jobject::jobject(const jschema* _schema, relative_ptr_type _class_id, serialized_box_container* _box, relative_ptr_type _location) :
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

	const jschema* jobject::get_schema()
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
	std::partial_ordering jobject::compare_express(jtype _type, char* c1, char* c2)
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
				b = image_instance{};
			}
			break;
			case jtype::type_wave:
			{
				wave_box b(c);
				b = wave_instance{};
			}
			break;
			case jtype::type_midi:
			{
				midi_box b(c);
				b = midi_instance{};
			}
			break;
			}
		}
	}

	const jfield& jobject::get_field(int field_idx) const
	{
		jclass_field& jcf = the_class.detail(field_idx);
		jfield& jf = schema->get_field(jcf.field_id);
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
		return schema && get_field_index_by_name(name) > -1;
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
		char* b = get_bytes() + offset;
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
				auto& fld_source = _src_slice.get_field(fis);
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
					throw std::invalid_argument("Invalid field index");
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
			{ { jtype::type_int32, "string_length", "Max Length of String Field" }, 0, 1 << 20 },
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


}

#endif
