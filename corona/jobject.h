#pragma once

#include "corona.h"

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
			uint32_t			max_actors;
			uint32_t			max_objects;
			uint64_t			collection_size_bytes;

			bool				create_style_sheet;

			relative_ptr_type	style_sheet_id;
			relative_ptr_type	actors_id;
			relative_ptr_type	objects_id;

			dynamic_box*		data;

			jcollection_ref() : 
				collection_name(""),
				collection_file_name(""),
				collection_location(null_row),
				model_name(""),
				max_actors(0),
				max_objects(0),
				collection_size_bytes(0),
				actors_id(null_row),
				objects_id(null_row),
				style_sheet_id(null_row),
				create_style_sheet(false),
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
			relative_ptr_type query_properties_id;
			relative_ptr_type sql_properties_id;
			relative_ptr_type file_properties_id;
			relative_ptr_type http_properties_id;
			relative_ptr_type models_id;
		};

		using actor_id_type = relative_ptr_type;

		class jschema;
		class jarray;
		class jlist;

		class slice_enumerable {
		public:
			virtual jslice get_at(relative_ptr_type object_id) = 0;
			virtual corona_size_t size() = 0;
		};

		class dynamic_value
		{
			relative_ptr_type		field_id;
			jtype					this_type;
			object_description		string_value;
			double					double_value;
			corona_size_t			int_value;
			time_t					time_value;
			rectangle				rectangle_value;
			layout_rect				layout_rect_value;
			point					point_value;
			color					color_value;

			void copy(const dynamic_value& _src);

		public:
			friend class jslice;

			dynamic_value() : field_id(null_row), this_type(jtype::type_null), string_value(""), double_value(0.0), int_value(0), time_value(0)
			{
				;
			}

			dynamic_value(const dynamic_value& _src);
			dynamic_value operator =(const dynamic_value& _src);

			template <explicit_char_ptr t> dynamic_value(relative_ptr_type _field_id, t _string_value) : field_id(_field_id), string_value(_string_value), this_type(jtype::type_string)
			{
				;
			}

			dynamic_value(relative_ptr_type _field_id, object_description& _string_value) : field_id(_field_id), string_value(_string_value), this_type(jtype::type_string)
			{
				;
			}

			template <explicit_double t> dynamic_value(relative_ptr_type _field_id, t _double_value) : field_id(_field_id), double_value(_double_value), this_type(jtype::type_float64)
			{
				;
			}

			template <explicit_int64 t> dynamic_value(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), this_type(jtype::type_int64)
			{
				;
			}

			template <explicit_int32 t> dynamic_value(relative_ptr_type _field_id, t _int_value) : field_id(_field_id), int_value(_int_value), this_type(jtype::type_int64)
			{
				;
			}

			template <explicit_bool t> dynamic_value(relative_ptr_type _field_id, t _bool_value) : field_id(_field_id), int_value(_bool_value), this_type(jtype::type_int64)
			{
				;
			}

			dynamic_value(relative_ptr_type _field_id, rectangle _rectangle_value) : field_id(_field_id), rectangle_value(_rectangle_value), this_type(jtype::type_rectangle)
			{
				;
			}

			dynamic_value(relative_ptr_type _field_id, layout_rect _layout_rect_value) : field_id(_field_id), layout_rect_value(_layout_rect_value), this_type(jtype::type_layout_rect)
			{
				;
			}

			dynamic_value(relative_ptr_type _field_id, point _point_value) : field_id(_field_id), point_value(_point_value), this_type(jtype::type_point)
			{
				;
			}

			dynamic_value(relative_ptr_type _field_id, color _color_value) : field_id(_field_id), color_value(_color_value), this_type(jtype::type_color)
			{
				;
			}

			template <explicit_floating_point t> operator t()
			{
				double r;
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					r = int_value;
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					r = double_value;
					break;
				case jtype::type_collection_id:
					throw std::logic_error("can't convert collection id to double");
					break;
				case jtype::type_color:
					throw std::logic_error("can't convert color to double");
					break;
				case jtype::type_datetime:
					r = time_value;
					break;
				case jtype::type_file:
					throw std::logic_error("can't convert file to double");
					break;
				case jtype::type_http:
					throw std::logic_error("can't convert http to double");
					break;
				case jtype::type_image:
					throw std::logic_error("can't convert image to double");
					break;
				case jtype::type_layout_rect:
					throw std::logic_error("can't convert layout_rect to double");
					break;
				case jtype::type_list:
					throw std::logic_error("can't convert list to double");
					break;
				case jtype::type_midi:
					throw std::logic_error("can't convert midi to double");
					break;
				case jtype::type_null:
					throw std::logic_error("can't convert null to double");
					break;
				case jtype::type_object:
					throw std::logic_error("can't convert object to double");
					break;
				case jtype::type_object_id:
					throw std::logic_error("can't convert object_id to double");
					break;
				case jtype::type_point:
					throw std::logic_error("can't convert point to double");
					break;
				case jtype::type_query:
					throw std::logic_error("can't convert query to double");
					break;
				case jtype::type_rectangle:
					throw std::logic_error("can't convert rectangle to double");
					break;
				case jtype::type_sql:
					throw std::logic_error("can't convert sql to double");
					break;
				case jtype::type_string:
					r = string_value.to_double();
					break;
				case jtype::type_wave:
					throw std::logic_error("can't convert wave to double");
					break;
				}
				return r;
			}

			template <explicit_int t> operator t()
			{
				int64_t z;
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					z = int_value;
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					z = double_value;
					break;
				case jtype::type_collection_id:
					throw std::logic_error("can't convert to int");
					break;
				case jtype::type_color:
					throw std::logic_error("can't convert to int");
					break;
				case jtype::type_datetime:
					z = time_value;
					break;
				case jtype::type_file:
					throw std::logic_error("can't convert file to int");
					break;
				case jtype::type_http:
					throw std::logic_error("can't convert http to int");
					break;
				case jtype::type_image:
					throw std::logic_error("can't convert image to int");
					break;
				case jtype::type_layout_rect:
					throw std::logic_error("can't convert layout_rect to int");
					break;
				case jtype::type_list:
					throw std::logic_error("can't convert list to int");
					break;
				case jtype::type_midi:
					throw std::logic_error("can't convert midi to int");
					break;
				case jtype::type_null:
					throw std::logic_error("can't convert null to int");
					break;
				case jtype::type_object:
					throw std::logic_error("can't convert object to int");
					break;
				case jtype::type_object_id:
					throw std::logic_error("can't convert object_id to int");
					break;
				case jtype::type_point:
					throw std::logic_error("can't convert point to int");
					break;
				case jtype::type_query:
					throw std::logic_error("can't convert query to int");
					break;
				case jtype::type_rectangle:
					throw std::logic_error("can't convert rectangle to int");
					break;
				case jtype::type_sql:
					throw std::logic_error("can't convert sql to int");
					break;
				case jtype::type_string:
					z = string_value.to_long();
					break;
				case jtype::type_wave:
					throw std::logic_error("can't convert wave to int");
					break;
				}
				return z;
			}

			template <explicit_bool t> operator t()
			{
				return (int)*this;
			}

			template <explicit_char_ptr t> operator t()
			{
				switch (this_type)
				{
				case jtype::type_int8:
				case jtype::type_int16:
				case jtype::type_int32:
				case jtype::type_int64:
					string_value = std::format("{}", int_value);
					break;
				case jtype::type_float32:
				case jtype::type_float64:
					string_value = std::format("{}", double_value);
					break;
				case jtype::type_collection_id:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_color:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_datetime:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_file:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_http:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_image:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_layout_rect:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_list:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_midi:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_null:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_object:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_object_id:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_point:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_query:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_rectangle:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_sql:
					throw std::logic_error("can't convert to string");
					break;
				case jtype::type_string:
					break;
				case jtype::type_wave:
					throw std::logic_error("can't convert to string");
					break;
				}
				return string_value.c_str();
			}

			operator std::string();
			operator color ();
			operator point();
			operator rectangle();
			operator layout_rect();

		};

		class jslice
		{
			jslice* parent;
			jschema* schema;
			relative_ptr_type class_id;
			char* bytes;
			dimensions_type dim;
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

			jslice();
			jslice(jslice* _parent, jschema* _schema, relative_ptr_type _class_id, char* _bytes, dimensions_type _dim);
			jslice(jslice* _parent, jschema* _schema, relative_ptr_type _class_id, serialized_box_container *_box, relative_ptr_type _location, dimensions_type _dim);

			jslice(const jslice& src);
			jslice operator =(const jslice& src);

			jslice(jslice&& src);
			jslice& operator =(jslice&& src);

			void construct();

			jslice& get_parent_slice();
			jclass get_class();
			jschema* get_schema();

			relative_ptr_type get_class_id() const 
			{
				return class_id;
			}

			dimensions_type get_dim();

			int get_field_index_by_name(const object_name& name);
			int get_field_index_by_id(relative_ptr_type field_id);
			jfield& get_field_by_id(relative_ptr_type field_id);
			jclass_field& get_class_field(int field_idx);
			jfield& get_field(int field_idx);

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
			jarray get_object(int field_idx, bool _use_id = false);
			jslice get_slice(int field_idx, dimensions_type _dim, bool _use_id = false);
			jlist get_list(int field_idx, bool _use_id = false);
			collection_id_box get_collection_id(int field_idx, bool _use_id = false);
			object_id_box get_object_id(int field_idx, bool _use_id = false);
			point_box get_point(int field_idx, bool _use_id = false);
			rectangle_box get_rectangle(int field_idx, bool _use_id = false);
			layout_rect_box get_layout_rect(int field_idx, bool _use_id = false);
			image_box get_image(int field_idx, bool _use_id = false);
			wave_box get_wave(int field_idx, bool _use_id = false);
			midi_box get_midi(int field_idx, bool _use_id = false);
			color_box get_color(int field_idx, bool _use_id = false);
			query_box get_query(int field_idx, bool _use_id = false);
			sql_remote_box get_sql_remote(int field_idx, bool _use_id = false);
			http_remote_box get_http_remote(int field_idx, bool _use_id = false);
			file_remote_box get_file_remote(int field_idx, bool _use_id = false);

			int8_box get_int8(object_name field_name);
			int16_box get_int16(object_name field_name);
			int32_box get_int32(object_name field_name);
			int64_box get_int64(object_name field_name);
			float_box get_float(object_name field_name);
			double_box get_double(object_name field_name);
			time_box get_time(object_name field_name);
			string_box get_string(object_name field_name);
			jarray get_object(object_name field_name);
			jlist get_list(object_name field_name);
			collection_id_box get_collection_id(object_name field_name);
			object_id_box get_object_id(object_name field_name);
			point_box get_point(object_name field_name);
			rectangle_box get_rectangle(object_name field_name);
			layout_rect_box get_layout_rect(object_name field_name);
			image_box get_image(object_name field_name);
			wave_box get_wave(object_name field_name);
			midi_box get_midi(object_name field_name);
			color_box get_color(object_name field_name);
			query_box get_query(object_name field_name);
			sql_remote_box get_sql_remote(object_name field_name);
			http_remote_box get_http_remote(object_name field_name);
			file_remote_box get_file_remote(object_name field_name);

			void set_value(const dynamic_value& _member_assignment);

			void set(std::initializer_list<dynamic_value> var)
			{
				for (auto item : var) {
					set_value(item);
				}
			}

			void set(std::initializer_list<relative_ptr_type> member_ids, std::initializer_list<dynamic_value> var)
			{
				jslice target_slice = *this;
				
				for (auto item : member_ids)
				{
					target_slice = target_slice.get_slice(item, {0,0,0}, true);
				}

				for (auto item : var) 
				{
					target_slice.set_value(item);
				}
			}

			dynamic_value get(relative_ptr_type field_id);
			dynamic_value operator[](relative_ptr_type field_idx);

			void update(jslice& _src_slice);

			std::partial_ordering compare(int _dst_idx, jslice& _src_slice, int _src_idx);
			std::partial_ordering compare(jslice& _src_slice);
			std::partial_ordering compare(jslice& _src_slice, relative_ptr_type *field_ids);

			jslice convert(serialized_box_container* _box, relative_ptr_type _class_id);

			template <typename boxed> boxed get(int field_idx)
			{
				size_t offset = get_offset(field_idx);
				char *src = get_bytes() + offset;
				boxed temp(src);
				return temp;
			}

			int size();
			char* get_bytes() { return box ? box->unpack<char>(location) : bytes;  };
			relative_ptr_type size_bytes() { return get_class().item().class_size_bytes; };

			std::partial_ordering operator<=>(jslice& src) {
				return compare(src);
			}
		};

		class jarray : public slice_enumerable
		{
			jschema* schema;
			relative_ptr_type class_field_id;
			char* bytes;
			jslice* item;

		public:

			using collection_type = jarray;
			using iterator_item_type = value_object<jslice>;
			using iterator_type = filterable_iterator<jslice, collection_type, iterator_item_type>;

			jarray();
			jarray(jslice* _parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init = false);
			jarray(dynamic_box& _dest, jarray& _src);
			dimensions_type dimensions();

			jslice get_slice(int x, int y = 0, int z = 0);
			jslice get_slice(dimensions_type dims);
			uint64_t get_size_bytes();
			char* get_bytes();

			jslice get_at(relative_ptr_type _index);
			corona_size_t size();

			inline iterator_type begin()
			{
				return iterator_type(this, 0);
			}

			inline iterator_type end()
			{
				return iterator_type(this, null_row);
			}
		};

		class jarray_container
		{
			collection_id_type	collection;
			dynamic_box			data;
			jarray				objects;

		public:

			jarray_container();
			jarray_container(collection_id_type& _collection, jarray& _objects);
			void set(collection_id_type& _collection, jarray& _objects);
			jarray& get();
		};

		using selection_flag_type = uint32_t;

		class jlist : public slice_enumerable
		{
			jschema* schema;
			relative_ptr_type class_field_id;
			jlist_state data;
			jslice* item;
			inline_box model_box;

		public:

			using collection_type = jlist;
			using iterator_item_type = value_object<jslice>;
			using iterator_type = filterable_iterator<jslice, collection_type, iterator_item_type>;


			jlist();
			jlist(jslice* _parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init = false);
			jlist(serialized_box_container& _dest, jlist& _src);

			corona_size_t capacity();
			corona_size_t size();

			jslice get_at(relative_ptr_type x);
			bool erase(relative_ptr_type x);
			bool chop();

			jslice append_slice();
			bool select_slice(relative_ptr_type x);
			bool deselect_slice(relative_ptr_type x);
			void deselect_all();
			void select_all();
			void clear();

			uint64_t get_size_bytes();
			char* get_bytes();

			inline iterator_type begin()
			{
				return iterator_type(this, 0);
			}

			inline iterator_type end()
			{
				return iterator_type(this, null_row);
			}
		};

		class actor_type
		{
		public:
			collection_id_type		collection_id;
			actor_id_type			actor_id;
			object_name				actor_name;
			selections_collection	selections;
			selections_collection	breadcrumb;
		};

		using jactor = actor_type;

		class collection_object_type
		{
		public:
			object_id_type			oid;
			jtype					otype;
			relative_ptr_type		class_id;
			relative_ptr_type		class_field_id;
			actor_id_type			actor_id;
			relative_ptr_type		item_id;
			bool					deleted;
		};

		class create_object_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	class_id;
			relative_ptr_type	item_id;
			bool				select_on_create;
		};

		class actor_view_object
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	object_id;
			relative_ptr_type	class_id;
			bool				selectable;
			bool				updatable;
			bool				selected;
		};

		class select_object_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	object_id;
			bool				extend;
		};

		class update_object_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	object_id;
			jslice				item;
		};

		using actor_view_collection = sorted_index<relative_ptr_type, actor_view_object>;
		using actor_create_collection = sorted_index<relative_ptr_type, create_object_request>;

		class actor_object_option
		{
		public:
			actor_view_object	avo;
			jslice				slice;
		};

		class actor_query_base 
		{
		public:
			slice_enumerable *objects;
			actor_view_collection& view;

			using collection_type = actor_query_base;
			using iterator_item_type = value_object<actor_object_option>;
			using iterator_type = filterable_iterator<actor_object_option, collection_type, iterator_item_type>;

			actor_query_base(actor_view_collection& _view, slice_enumerable* _objects):
				view(_view),
				objects(_objects)
			{
				;
			}

			corona_size_t size()
			{
				return objects->size();
			}

			actor_object_option get_at(relative_ptr_type _id)
			{
				actor_object_option aoo;
				aoo.slice = objects->get_at(_id);
				aoo.avo = view.get(_id).second;
				return aoo;
			}

			iterator_type begin()
			{
				return iterator_type(this, first_row);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row);
			}

			auto where(std::function<bool(const iterator_item_type&)> _predicate)
			{
				return iterator_type(this, _predicate);
			}
		};

		class actor_state
		{
			dynamic_box									data;
			relative_ptr_type							create_objects_location;
			relative_ptr_type							view_objects_location;
			jschema* schema;

		public:

			collection_id_type							collection_id;
			relative_ptr_type							actor_id;
			actor_create_collection						create_objects;
			actor_view_collection						view_objects;

			int											modified_object_level;
			relative_ptr_type							modified_object_id;
			jslice										modified_object;

			actor_state()
			{
				data.init(100000);
				create_objects = actor_create_collection::create_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::create_sorted_index(&data, view_objects_location);
				modified_object_id = null_row;
			}

			actor_state(actor_state&& _src)
			{
				data = std::move(_src.data);
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location );
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location );
			}

			actor_state& operator=(actor_state&& _src)
			{
				data = std::move(_src.data);
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);

				return *this;
			}

			actor_state operator=(const actor_state& _src)
			{
				data = _src.data;
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);
				return *this;
			}

			actor_state(const actor_state& _src)
			{
				data = _src.data;
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);
			}

			jslice create_object(jschema* _schema, relative_ptr_type _class_id);
			jslice copy_object(jschema* _schema, jslice& _src);
			actor_view_object get_modified_object();

			actor_query_base query(slice_enumerable *collection)
			{
				actor_query_base new_base(view_objects, collection);
				return new_base;
			}

			create_object_request create_create_request(relative_ptr_type _class_id)
			{
				if (!create_objects.contains(_class_id)) {
					throw std::invalid_argument("class is not creatable");
				}
				create_object_request aco = create_objects[_class_id].second;
				return aco;
			}

			select_object_request create_select_request(relative_ptr_type _object_id, bool _extend)
			{
				if (!view_objects.contains(_object_id)) {
					throw std::invalid_argument("object not found");
				}
				actor_view_object avo = view_objects[_object_id].second;
				select_object_request aso;
				aso.collection_id = collection_id;
				aso.actor_id = actor_id;
				aso.extend = _extend;
				aso.object_id = _object_id;
				return aso;
			}

			update_object_request create_update_request()
			{
				update_object_request uor;
				uor.actor_id = actor_id;
				uor.collection_id = collection_id;
				uor.object_id = modified_object_id;
				uor.item = modified_object;
				return uor;
			}

		};

		using actor_collection = table<actor_type>;
		using object_collection = item_details_table<collection_object_type, char>;

		class jcollection
		{

			jschema* schema;
			jcollection_ref* ref;

			collection_id_type		collection_id;
			actor_collection		actors;
			object_collection		objects;

		public:


			using collection_type = jcollection;
			using iterator_item_type = value_object<jslice>;
			using iterator_type = filterable_iterator<jslice, collection_type, iterator_item_type>;

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
				actors = actor_collection::get_table(_ref->data, _ref->actors_id );
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
				actors = actor_collection::get_table(ref->data, ref->actors_id);
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
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
			}

			jcollection operator =(jcollection&& _src) 
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			jcollection operator =(jcollection& _src)
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			collection_id_type get_collection_id()
			{
				return collection_id;
			}

			actor_type create_actor(actor_type _actor);
			actor_id_type put_actor(actor_type _actor);
			actor_type get_actor(actor_id_type _actor_id);
			actor_id_type find_actor(object_name& name);
			actor_type update_actor(actor_type _actor);

			actor_state get_actor_state(relative_ptr_type _actor, relative_ptr_type _last_modified_object = null_row, const char *_trace_msg = nullptr);
			actor_state select_object(const select_object_request& _select, const char* _trace_msg = nullptr);
			actor_state create_object(create_object_request& _create, const char* _trace_msg = nullptr);
			actor_state update_object(update_object_request& _update, const char* _trace_msg = nullptr);

			void print(const char *_trace, actor_state& acr);

			jslice create_object(relative_ptr_type _item_id, relative_ptr_type _actor_id, relative_ptr_type _class_id, relative_ptr_type& object_id);
			jslice get_object(relative_ptr_type _object_id);
			jslice update_object(relative_ptr_type _object_id, jslice _slice);
			collection_object_type &get_object_reference(relative_ptr_type _object_id);

			bool selector_applies(selector_collection* _selector, actor_id_type& _actor);

			jslice get_at(relative_ptr_type _object_id);
			relative_ptr_type get_class_id(relative_ptr_type _object_id);

			relative_ptr_type size()
			{
				return objects.size();
			}

			iterator_type begin()
			{
				return iterator_type(this, first_row);
			}

			iterator_type end()
			{
				return iterator_type(this, null_row);
			}

			auto where(std::function<bool(const iterator_item_type&)> _predicate)
			{
				return iterator_type(this, _predicate);
			}

			auto where(relative_ptr_type _class_id)
			{
				return iterator_type(this, [_class_id](const iterator_item_type& it) { return it.item.get_class_id() == _class_id; });
			}

			jslice first_value(std::function<bool(const iterator_item_type&)> predicate)
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
			using query_store_type = sorted_index<relative_ptr_type, query_definition_type>;
			using sql_store_type = sorted_index<relative_ptr_type, sql_definition_type>;
			using file_store_type = sorted_index<relative_ptr_type, file_definition_type>;
			using http_store_type = sorted_index<relative_ptr_type, http_definition_type>;
			using model_store_type = sorted_index<object_name, model_type>;

			field_store_type		fields;
			class_store_type		classes;
			class_index_type		classes_by_name;
			field_index_type		fields_by_name;
			query_store_type		queries;
			sql_store_type			sql_remotes;
			file_store_type			file_remotes;
			http_store_type			http_remotes;
			model_store_type		models;

			jfield					empty;

		public:
			
			relative_ptr_type idfull_name;
			relative_ptr_type idfirst_name;
			relative_ptr_type idlast_name;
			relative_ptr_type idmiddle_name;
			relative_ptr_type idssn;
			relative_ptr_type idemail;
			relative_ptr_type idtitle;
			relative_ptr_type idstreet;
			relative_ptr_type idsuiteapt;
			relative_ptr_type idcity;
			relative_ptr_type idstate;
			relative_ptr_type idpostal;
			relative_ptr_type idcountry_name;
			relative_ptr_type idcountry_code;
			relative_ptr_type idinstitution_name;
			relative_ptr_type idlong_name;
			relative_ptr_type idshort_name;
			relative_ptr_type idunit;
			relative_ptr_type idsymbol;
			relative_ptr_type idoperator;
			relative_ptr_type idwindows_path;
			relative_ptr_type idlinux_path;
			relative_ptr_type idurl;
			relative_ptr_type idusername;
			relative_ptr_type idpassword;
			relative_ptr_type iddoc_title;
			relative_ptr_type idsection_title;
			relative_ptr_type idblock_title;
			relative_ptr_type idcaption;
			relative_ptr_type idparagraph;
			relative_ptr_type idmimeType;
			relative_ptr_type idbase64;
			relative_ptr_type idfile_name;
			relative_ptr_type idfont_name;
			relative_ptr_type idname;

			relative_ptr_type idbirthday;
			relative_ptr_type idscheduled;

			relative_ptr_type idcount;

			relative_ptr_type idquantity;
			relative_ptr_type idlatitude;
			relative_ptr_type idlongitude;
			relative_ptr_type idmeters;
			relative_ptr_type idfeet;
			relative_ptr_type idkilograms;
			relative_ptr_type idpounds;
			relative_ptr_type idseconds;
			relative_ptr_type idminutes;
			relative_ptr_type idhours;
			relative_ptr_type idamperes;
			relative_ptr_type idkelvin;
			relative_ptr_type idmoles;
			relative_ptr_type idgradient_position;
			relative_ptr_type idfont_size;
			relative_ptr_type idline_spacing;
			relative_ptr_type idbox_border_thickness;
			relative_ptr_type idshape_border_thickness;

			relative_ptr_type idcolor;
			relative_ptr_type idshape_fill_color;
			relative_ptr_type idbox_fill_color;
			relative_ptr_type idshape_border_color;
			relative_ptr_type idbox_border_color;
			relative_ptr_type idpoint;
			relative_ptr_type idposition_point;
			relative_ptr_type idselection_point;
			relative_ptr_type idrectangle;
			relative_ptr_type idlayout_rect;

			relative_ptr_type idbold;
			relative_ptr_type iditalic;
			relative_ptr_type idunderline;
			relative_ptr_type idstrike_through;
			relative_ptr_type idvertical_alignment;
			relative_ptr_type idhorizontal_alignment;
			relative_ptr_type idwrap_text;

			relative_ptr_type id_solid_brush;
			relative_ptr_type id_gradient_stop;
			relative_ptr_type id_linear_gradient_brush;
			relative_ptr_type id_round_gradient_brush;
			relative_ptr_type id_bitmap_brush;
			relative_ptr_type id_text_style;

			relative_ptr_type id_view_background;
			relative_ptr_type id_view_title;
			relative_ptr_type id_view_subtitle;
			relative_ptr_type id_view_section;
			relative_ptr_type id_view;
			relative_ptr_type id_disclaimer;
			relative_ptr_type id_copyright;
			relative_ptr_type id_h1;
			relative_ptr_type id_h2;
			relative_ptr_type id_h3;
			relative_ptr_type id_column_number_head;
			relative_ptr_type id_column_text_head;
			relative_ptr_type id_column_data;
			relative_ptr_type id_label;
			relative_ptr_type id_control;
			relative_ptr_type id_chart_axis;
			relative_ptr_type id_chart_legend;
			relative_ptr_type id_chart_block;
			relative_ptr_type id_tooltip;
			relative_ptr_type id_breadcrumb;

			relative_ptr_type id_style_sheet;

			jschema() = default;
			~jschema() = default;

			jfield& get_empty()
			{
				return empty;
			}

			static relative_ptr_type reserve_schema(serialized_box_container* _b, int _num_classes, int _num_fields, int _total_class_fields, bool _use_standard_fields)
			{
				jschema_map schema_map, *pschema_map;
				schema_map.block = block_id::collection_id();
				schema_map.fields_table_id = null_row;
				schema_map.classes_table_id = null_row;
				schema_map.classes_by_name_id = null_row;
				schema_map.fields_by_name_id = null_row;
				schema_map.query_properties_id = null_row;
				schema_map.sql_properties_id = null_row;
				schema_map.file_properties_id = null_row;
				schema_map.http_properties_id = null_row;
				schema_map.models_id = null_row;

				relative_ptr_type rit = _b->pack(schema_map);
				schema_map.fields_table_id = field_store_type::reserve_table(_b, _num_fields);
				schema_map.classes_table_id = class_store_type::reserve_table(_b, _num_classes, _total_class_fields);
				schema_map.classes_by_name_id = class_index_type::reserve_sorted_index(_b);
				schema_map.fields_by_name_id = field_index_type::reserve_sorted_index(_b);
				schema_map.query_properties_id = query_store_type::reserve_sorted_index(_b);
				schema_map.sql_properties_id = sql_store_type::reserve_sorted_index(_b);
				schema_map.file_properties_id = file_store_type::reserve_sorted_index(_b);
				schema_map.http_properties_id = http_store_type::reserve_sorted_index(_b);
				schema_map.models_id = model_store_type::reserve_sorted_index(_b);
				pschema_map = _b->unpack<jschema_map>(rit);
				*pschema_map = schema_map;
				return rit;
			} 

			static jschema get_schema(serialized_box_container* _b, relative_ptr_type _row)
			{
				jschema schema;
				jschema_map* pschema_map;
				pschema_map = _b->unpack<jschema_map>(_row);
				if (!pschema_map->block.is_collection()) {
					throw std::logic_error("collection read wrong");
				}
				schema.fields = field_store_type::get_table(_b, pschema_map->fields_table_id);
				schema.classes = class_store_type::get_table(_b, pschema_map->classes_table_id);
				schema.classes_by_name = class_index_type::get_sorted_index(_b, pschema_map->classes_by_name_id);
				schema.fields_by_name = field_index_type::get_sorted_index(_b, pschema_map->fields_by_name_id);
				schema.queries = query_store_type::get_sorted_index(_b, pschema_map->query_properties_id);
				schema.sql_remotes = sql_store_type::get_sorted_index(_b, pschema_map->sql_properties_id);
				schema.file_remotes = file_store_type::get_sorted_index(_b, pschema_map->file_properties_id);
				schema.http_remotes = http_store_type::get_sorted_index(_b, pschema_map->http_properties_id);
				schema.models = model_store_type::get_sorted_index(_b, pschema_map->models_id);
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
				int64_t query_size = query_store_type::get_box_size();
				int64_t sql_size = sql_store_type::get_box_size();
				int64_t file_size = file_store_type::get_box_size();
				int64_t http_size = http_store_type::get_box_size();
				int64_t model_size = model_store_type::get_box_size();
				int64_t total_size = field_size + 
					class_size + 
					classes_by_name_size + 
					fields_by_name_size + 
					query_size + 
					sql_size + 
					file_size + 
					http_size + 
					model_size;
				return total_size;
			}

			static jschema create_schema(serialized_box_container* _b, int _num_classes, bool _use_standard_fields, relative_ptr_type& _location)
			{
				int _num_fields = _num_classes * 5 + _use_standard_fields ? 200 : 0;
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

			relative_ptr_type put_field(
				relative_ptr_type _field_id,
				jtype _field_type,
				object_name& _name,
				object_description& _description,
				string_properties_type* _string_properties,
				int_properties_type* _int_properties,
				double_properties_type* _double_properties,
				time_properties_type* _time_properties,
				object_properties_type* _object_properties,
				query_properties_type* _query_properties,
				point_properties_type* _point_properties,
				rectangle_properties_type* _rectangle_properties,
				image_properties_type* _image_properties,
				wave_properties_type* _wave_properties,
				midi_properties_type* _midi_properties,
				color_properties_type* _color_properties,
				sql_properties_type* _sql_properties,
				file_properties_type* _file_properties,
				http_properties_type* _http_properties,
				layout_rect_properties_type* _layout_rect_properties,
				int64_t _size_bytes)
			{

				_field_id = find_field(_name);

				if (_field_id == null_row)
				{
					_field_id = new_field_id();
				}

				auto& jf = fields[_field_id];

				jf.field_id = _field_id;
				jf.type_id = _field_type;
				jf.name = _name;
				jf.description = _description;

				if (_string_properties)
					jf.string_properties = *_string_properties;
				if (_int_properties)
					jf.int_properties = *_int_properties;
				if (_double_properties)
					jf.double_properties = *_double_properties;
				if (_time_properties)
					jf.time_properties = *_time_properties;
				if (_object_properties)
					jf.object_properties = *_object_properties;
				if (_query_properties)
					jf.query_properties = *_query_properties;
				if (_point_properties)
					jf.point_properties = *_point_properties;
				if (_rectangle_properties)
					jf.rectangle_properties = *_rectangle_properties;
				if (_image_properties)
					jf.image_properties = *_image_properties;
				if (_wave_properties)
					jf.wave_properties = *_wave_properties;
				if (_midi_properties)
					jf.midi_properties = *_midi_properties;
				if (_color_properties)
					jf.color_properties = *_color_properties;
				if (_sql_properties)
					jf.sql_properties = *_sql_properties;
				if (_http_properties)
					jf.http_properties = *_http_properties;
				if (_file_properties)
					jf.file_properties = *_file_properties;
				if (_layout_rect_properties)
					jf.layout_rect_properties = *_layout_rect_properties;

				jf.size_bytes = _size_bytes;

				fields_by_name.insert_or_assign(jf.name, _field_id);
				return _field_id;
			}

			relative_ptr_type put_string_field(put_string_field_request request)
			{
				return put_field(request.name.field_id, jtype::type_string, request.name.name, request.name.description, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, string_box::get_box_size(request.options.length));
			}

			relative_ptr_type put_time_field(put_time_field_request request)
			{
				return put_field(request.name.field_id, type_datetime, request.name.name, request.name.description, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(time_t));
			}

			relative_ptr_type put_integer_field(put_integer_field_request request)
			{
				switch (request.name.type_id)
				{
				case jtype::type_int8:
					return put_field(request.name.field_id, type_int8, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int8_t));
				case jtype::type_int16:
					return put_field(request.name.field_id, type_int16, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int16_t));
				case jtype::type_int32:
					return put_field(request.name.field_id, type_int32, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int32_t));
				case jtype::type_int64:
					return put_field(request.name.field_id, type_int64, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int64_t));
				default:
					throw std::invalid_argument("Invalid integer type for field name:" + request.name.name);
				}
			}

			relative_ptr_type put_double_field(put_double_field_request request)
			{
				switch (request.name.type_id)
				{
				case type_float32:
					return put_field(request.name.field_id, type_float32, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(float));
				case type_float64:
					return put_field(request.name.field_id, type_float64, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(double));
				default:
					throw std::invalid_argument("Invalid floating point type for field name:" + request.name.name);
				}
			}

			void get_class_field_name(object_name& _dest, object_name _class_name, dimensions_type& _dim)
			{
				_dest = _class_name + "[" + std::to_string(_dim.x) + "," + std::to_string(_dim.y) + "," + std::to_string(_dim.z) + "]";
			}

			relative_ptr_type put_object_field(put_object_field_request request)
			{
				auto pcr = classes[request.options.class_id];
				auto& p = pcr.item();
				int64_t sizeb = pcr.item().class_size_bytes;
				request.options.class_size_bytes = sizeb;
				if (request.options.dim.x == 0) request.options.dim.x = 1;
				if (request.options.dim.y == 0) request.options.dim.y = 1;
				if (request.options.dim.z == 0) request.options.dim.z = 1;
				object_name field_name;
				if (request.name.name.size() == 0) 
				{
					get_class_field_name(field_name, pcr.pitem()->name, request.options.dim);
				}
				else 
				{
					field_name = request.name.name;
				}
				request.options.total_size_bytes = request.options.dim.x * request.options.dim.y * request.options.dim.z * sizeb;
				return put_field(request.name.field_id, type_object, field_name, request.name.description, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, request.options.total_size_bytes);
			}

			relative_ptr_type put_list_field(put_object_field_request request)
			{
				auto pcr = classes[request.options.class_id];
				auto& p = pcr.item();
				int64_t sizeb = pcr.item().class_size_bytes;
				request.options.class_size_bytes = sizeb;
				if (request.options.dim.x == 0) request.options.dim.x = 1;
				int64_t power_of_two = 1;
				while (request.options.dim.x < power_of_two) {
					power_of_two *= 2;
				}
				request.options.dim.x = power_of_two;
				request.options.dim.y = 1;
				request.options.dim.z = 1;
				object_name field_name;
				get_class_field_name(field_name, pcr.pitem()->name + "list", request.options.dim);
				request.options.total_size_bytes = request.options.dim.x * request.options.dim.y * request.options.dim.z * sizeb + sizeof(jlist_instance) + sizeof(selection_flag_type) * request.options.dim.x + 32;
				return put_field(request.name.field_id, type_list, field_name, request.name.description, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, request.options.total_size_bytes);
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

			relative_ptr_type put_query_field(put_named_query_field_request request)
			{
				query_properties_type options;

				relative_ptr_type query_location = put_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(query_instance));
				queries.insert_or_assign(query_location, request.options);

				return query_location;
			}

			relative_ptr_type put_sql_remote_field(put_named_sql_remote_field_request request)
			{
				sql_properties_type options;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				relative_ptr_type remote_location = put_field(request.name.field_id, type_sql, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, nullptr, sizeof(sql_remote_instance));
				sql_remotes.insert_or_assign(remote_location, request.options);
				return remote_location;
			}

			relative_ptr_type put_http_remote_field(put_named_http_remote_field_request request)
			{
				http_properties_type options;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				relative_ptr_type remote_location = put_field(request.name.field_id, type_http, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, sizeof(http_remote_instance));
				http_remotes.insert_or_assign(remote_location, request.options);

				return remote_location;
			}

			relative_ptr_type put_file_remote_field(put_named_file_remote_field_request request)
			{
				file_properties_type options;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				relative_ptr_type remote_location = put_field(request.name.field_id, type_sql, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, sizeof(file_remote_instance));
				file_remotes.insert_or_assign(remote_location, request.options);
				return remote_location;
			}

			relative_ptr_type put_point_field(put_point_field_request request)
			{
				return put_field(request.name.field_id, type_point, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(point));
			}

			relative_ptr_type put_rectangle_field(put_rectangle_field_request request)
			{
				return put_field(request.name.field_id, type_rectangle, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(rectangle));
			}

			relative_ptr_type put_layout_rect_field(put_layout_rect_field_request request)
			{
				return put_field(request.name.field_id, type_layout_rect, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, sizeof(layout_rect));
			}

			relative_ptr_type put_image_field(put_image_field_request request)
			{
				return put_field(request.name.field_id, type_image, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(image_instance));			}

			relative_ptr_type put_wave_field(put_wave_field_request request)
			{
				return put_field(request.name.field_id, type_wave, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(wave_instance));
			}

			relative_ptr_type put_midi_field(put_midi_field_request request)
			{
				return put_field(request.name.field_id, type_midi, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(midi_instance));
			}

			relative_ptr_type put_color_field(put_color_field_request request)
			{
				return put_field(request.name.field_id, type_color, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, sizeof(color));
			}

			relative_ptr_type put_list_field(relative_ptr_type class_id, int max_rows )
			{
				object_name result_class_name;

				put_object_field_request porf;
				auto class_cls = classes[class_id];
				porf.name.name = class_cls.item().name;
				porf.name.field_id = null_row;
				porf.name.type_id = jtype::type_list;
				porf.options.class_name = class_cls.item().name;
				porf.options.class_id = class_id;
				porf.options.class_size_bytes = class_cls.pitem()->class_size_bytes;
				porf.options.dim = { max_rows, 1, 1 };
				auto class_field_id = put_object_field(porf);
				if (class_field_id == null_row) {
					return null_row;
				}
				return class_field_id;
			}

			relative_ptr_type get_result_field_class(jfield fld, relative_ptr_type &_max_result_rows)
			{
				relative_ptr_type result_class_id = null_row;
				switch (fld.type_id) {
				case jtype::type_query:
					{
						auto qd = get_query_definition(fld.field_id);
						result_class_id = qd.result_class_id;
						_max_result_rows = qd.max_result_objects;
						break;
					}
				case jtype::type_file:
					{
						auto fd = get_file_definition(fld.field_id);
						result_class_id = fd.result_class_id;
						_max_result_rows = fd.max_result_objects;
						break;
					}
					break;
				case jtype::type_http:
					{
						auto hd = get_http_definition(fld.field_id);
						result_class_id = hd.result_class_id;
						_max_result_rows = hd.max_result_objects;
						break;
					}
					break;
				case jtype::type_sql:
					{
						auto sd = get_sql_definition(fld.field_id);
						result_class_id = sd.result_class_id;
						_max_result_rows = sd.max_result_objects;
						break;
					}
					break;
				}
				return result_class_id;
			}


			relative_ptr_type build_class_members(field_array& pcr, int64_t& class_size_bytes, member_field_collection &mfs)
			{
				int field_idx = 0;
				corona_size_t sz = mfs.size();
				class_size_bytes = 0;

				for (int i = 0; i < sz; i++)
				{
					auto& field = mfs[i];

					switch (field.membership_type)
					{
					case member_field_types::member_field:
					{
						relative_ptr_type fid;
						if (field.field_name.size()>0) {
							fid = fields_by_name[field.field_name].second;
						}
						else
						{
							fid = field.field_id;
						}
						auto& existing_field = fields[fid];
						jclass_field *ref = pcr.append();
						ref->field_id = fid;
						ref->offset = class_size_bytes;
						field_idx++;
						class_size_bytes += existing_field.size_bytes;

						if (existing_field.is_data_generator()) {
							relative_ptr_type max_result_rows = 0;
							relative_ptr_type result_class_field = this->get_result_field_class(existing_field, max_result_rows);

							jclass_field* ref = pcr.append();
							ref->field_id = result_class_field;
							ref->offset = class_size_bytes;
							field_idx++;
							auto& result_field = fields[result_class_field];
							class_size_bytes += result_field.size_bytes;
						}
					}
					break;
					case member_field_types::member_class:
					{
						put_object_field_request porf;
						auto class_cls = classes[field.class_id];

						if (field.field_name.size() > 0) {
							porf.name.name = field.field_name;
						}
						else 
						{
							porf.name.name = class_cls.item().name;
						}

						porf.name.field_id = null_row;
						porf.name.type_id = jtype::type_object;
						porf.options.class_name = class_cls.item().name;
						porf.options.class_id = field.class_id;
						porf.options.class_size_bytes = classes[field.class_id].pitem()->class_size_bytes;
						porf.options.dim = field.dimensions;

						auto class_field_id = put_object_field(porf);
						if (class_field_id == null_row) {
							return null_row;
						}
						auto& existing_field = fields[class_field_id];

						jclass_field* ref = pcr.append();
						ref->field_id = class_field_id;
						ref->offset = class_size_bytes;
						field_idx++;
						class_size_bytes += existing_field.size_bytes;
					}
					break;
					}

				}
			}

			relative_ptr_type put_class(put_class_request request)
			{
				relative_ptr_type class_id = find_class(request.class_name);
				request.class_id = class_id;

				auto& mfs = request.member_fields;

				for (auto mfi : mfs)
				{
					if (mfi.item.field_name.size() > 0) {
						bind_field(mfi.item.field_name, mfi.item.field_id);
					}
				}

				auto sz = mfs.size();
				int num_integration_fields = mfs.count_if([this](auto& src) {
					auto& f = this->get_field(src.item.field_id);
					return f.is_data_generator();
					});

				field_array af;
				int64_t total_size_bytes = 0;

				build_class_members(af, total_size_bytes, mfs);

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
				for (int i = 0; i < pcr.size(); i++)
				{
					if (pcr.detail(i).field_id == request.field_id_primary_key) {
						p.primary_key_idx = i;
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

			void put_model(jmodel request)
			{
				for (auto opt : request.create_options)
				{
					for (auto sel : opt.item.selectors.rules) 
					{
						bind_class(sel.item.class_name, sel.item.class_id);
					}
					bind_class(opt.item.create_class_name, opt.item.create_class_id);
					bind_class(opt.item.item_id_class_name, opt.item.item_id_class, true);
				}

				for (auto opt : request.select_options)
				{
					for (auto sel : opt.item.selectors.rules)
					{
						bind_class(sel.item.class_name, sel.item.class_id);
					}
					bind_class(opt.item.select_class_name, opt.item.select_class_id);
				}

				for (auto opt : request.update_options)
				{
					for (auto sel : opt.item.selectors.rules)
					{
						bind_class(sel.item.class_name, sel.item.class_id);
					}
					bind_class(opt.item.update_class_name, opt.item.update_class_id);
				}

				models.insert_or_assign(request.name, request);
			}

			relative_ptr_type find_class(const object_name& class_name)
			{
				return classes_by_name.contains(class_name) ? classes_by_name[class_name].second : null_row;
			}

			relative_ptr_type find_field(const object_name& field_name)
			{
				return fields_by_name.contains(field_name) ? fields_by_name[field_name].second : null_row;
			}

			jmodel get_model(object_name model_name)
			{
				return models[model_name].second;
			}

			jclass get_class(relative_ptr_type class_id)
			{
				auto the_class = classes[class_id];
				return the_class;
			}

			jfield &get_field(relative_ptr_type field_id)
			{
				auto& the_field = fields[field_id];
				return the_field;
			}

			const query_definition_type& get_query_definition(relative_ptr_type field_id)
			{
				auto& f = fields[field_id];
				return queries[field_id].second;
			}

			const sql_definition_type& get_sql_definition(relative_ptr_type field_id)
			{
				auto& f = fields[field_id];
				return sql_remotes[field_id].second;
			}

			const file_definition_type& get_file_definition(relative_ptr_type field_id)
			{
				auto& f = fields[field_id];
				return file_remotes[field_id].second;
			}

			const http_definition_type& get_http_definition(relative_ptr_type field_id)
			{
				auto& f = fields[field_id];
				return http_remotes[field_id].second;
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

				ref->actors_id = actor_collection::reserve_table(ref->data, ref->max_actors);
				ref->objects_id =  object_collection::reserve_table(ref->data, ref->max_objects, total_size, true);

				return ref->actors_id != null_row && ref->objects_id != null_row;
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
					create_object_request cor;
					cor.class_id = id_style_sheet;
					cor.actor_id = null_row;
					cor.select_on_create = false;
					cor.collection_id = tmp.get_collection_id();
					if (ref->create_style_sheet) {
						tmp.create_object(0, null_row, id_style_sheet, ref->style_sheet_id);
					}
				}
				return tmp;
			}
		};

		class jarray;

		bool schema_tests();
		bool collection_tests();
		bool array_tests();
		bool model_tests();
	}
}
