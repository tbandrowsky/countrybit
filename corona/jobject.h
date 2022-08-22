

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
			virtual jobject get_at(relative_ptr_type object_id) = 0;
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
			friend class jobject;

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

			bool is_integer(int64_t &v)
			{
				if ((this_type == jtype::type_int8 || this_type == jtype::type_int16 || this_type == jtype::type_int32 || this_type == jtype::type_int64))
				{
					v = int_value;
					return true;
				}
				return false;
			}

			bool is_double(double& _d)
			{
				if ((this_type == jtype::type_float32 || this_type == jtype::type_float64))
				{
					_d = double_value;
					return true;
				}
				return false;
			}

			bool compare(comparisons _comparion, dynamic_value& _target);

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
					string_value = "collection_id";
					break;
				case jtype::type_color:
					string_value = "color";
					break;
				case jtype::type_datetime:
					string_value = "datetime";
					break;
				case jtype::type_file:
					string_value = "file";
					break;
				case jtype::type_http:
					string_value = "http";
					break;
				case jtype::type_image:
					string_value = "image";
					break;
				case jtype::type_layout_rect:
					string_value = "layout_rect";
					break;
				case jtype::type_list:
					string_value = "list";
					break;
				case jtype::type_midi:
					string_value = "midi";
					break;
				case jtype::type_null:
					string_value = "(null type)";
					break;
				case jtype::type_object:
					string_value = "object";
					break;
				case jtype::type_object_id:
					string_value = "object_id";
					break;
				case jtype::type_point:
					string_value = "point";
					break;
				case jtype::type_query:
					string_value = "query";
					break;
				case jtype::type_rectangle:
					string_value = "rectangle";
					break;
				case jtype::type_sql:
					string_value = "sql";
					break;
				case jtype::type_string:
					break;
				case jtype::type_wave:
					string_value = "wave";
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

		class filter_term
		{
		public:
			dynamic_value	  src_value;
			relative_ptr_type target_field;
			comparisons		  comparison;
		};

		class filter_option
		{
		public:
			class_list classes;
			std::vector<filter_term> options;
		};

		class jobject
		{
			jobject* parent;
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

			jobject();
			jobject(jobject* _parent, jschema* _schema, relative_ptr_type _class_id, char* _bytes, dimensions_type _dim);
			jobject(jobject* _parent, jschema* _schema, relative_ptr_type _class_id, serialized_box_container *_box, relative_ptr_type _location, dimensions_type _dim);

			jobject(const jobject& src);
			jobject operator =(const jobject& src);

			jobject(jobject&& src);
			jobject& operator =(jobject&& src);

			void construct();

			jobject& get_parent_slice();
			jclass get_class() const;
			jschema* get_schema();

			void set_box_dangerous_hack(serialized_box_container* _box)
			{
#if ACTOR_OBJECT_CHECKING
				std::cout << "object box " << (void*)&box << " to " << (void *)_box << std::endl;
#endif
				box = _box;
			}

			relative_ptr_type get_class_id() const 
			{
				return class_id;
			}

			relative_ptr_type get_base_class_id() const 
			{
				return get_class().pitem()->base_class_id;
			}

			dimensions_type get_dim();

			int get_field_index_by_name(const object_name& name);
			int get_field_index_by_id(relative_ptr_type field_id);
			jfield& get_field_by_id(relative_ptr_type field_id);
			jclass_field& get_class_field(int field_idx);
			jfield& get_field(int field_idx);
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
			jarray get_object(int field_idx, bool _use_id = false);
			jarray get_object_by_class(relative_ptr_type _class_id);
			jobject get_object(int field_idx, dimensions_type _dim, bool _use_id = false);
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

			bool matches(const char* str);

			void set_value(const dynamic_value& _member_assignment);

			void set(std::initializer_list<dynamic_value> var)
			{
				for (auto item : var) {
					set_value(item);
				}
			}

			void set(std::initializer_list<relative_ptr_type> member_ids, std::initializer_list<dynamic_value> var)
			{
				jobject target_slice = *this;
				
				for (auto item : member_ids)
				{
					target_slice = target_slice.get_object(item, {0,0,0}, true);
				}

				for (auto item : var) 
				{
					target_slice.set_value(item);
				}
			}

			void set(relative_ptr_type _src_member_id, std::initializer_list<relative_ptr_type> member_ids, std::initializer_list<dynamic_value> var)
			{
				jobject target_slice = *this;
				jobject source_slice = *this;

				source_slice = get_object(_src_member_id, { 0,0,0 }, true);

				for (auto item : member_ids)
				{
					target_slice = target_slice.get_object(item, { 0,0,0 }, true);
				}

				target_slice.update(source_slice);

				for (auto item : var)
				{
					target_slice.set_value(item);
				}
			}

			dynamic_value get(relative_ptr_type field_id);
			dynamic_value operator[](relative_ptr_type field_idx);

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

			void* get_box_address() { return box; }

			int size();
			char* get_bytes() { return box ? box->unpack<char>(location) : bytes;  };
			bool is_null() {
				return get_bytes() == nullptr;
			}
			relative_ptr_type size_bytes() { return get_class().item().class_size_bytes; };

			std::partial_ordering operator<=>(jobject& src) {
				return compare(src);
			}
		};

		class jarray : public slice_enumerable
		{
			jschema* schema;
			relative_ptr_type class_field_id;
			char* bytes;
			jobject* item;

		public:

			using collection_type = jarray;
			using iterator_item_type = value_object<jobject>;
			using iterator_type = filterable_iterator<jobject, collection_type, iterator_item_type>;

			jarray();
			jarray(jobject* _parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init = false);
			jarray(dynamic_box& _dest, jarray& _src);
			dimensions_type dimensions();

			jobject get_object(int x, int y = 0, int z = 0);
			jobject get_object(dimensions_type dims);
			uint64_t get_size_bytes();
			char* get_bytes();

			jobject get_at(relative_ptr_type _index);
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
			jobject* item;
			inline_box model_box;

		public:

			using collection_type = jlist;
			using iterator_item_type = value_object<jobject>;
			using iterator_type = filterable_iterator<jobject, collection_type, iterator_item_type>;


			jlist();
			jlist(jobject* _parent, jschema* _schema, relative_ptr_type _class_field_id, char* _bytes, bool _init = false);
			jlist(serialized_box_container& _dest, jlist& _src);

			corona_size_t capacity();
			corona_size_t size();

			jobject get_at(relative_ptr_type x);
			bool erase(relative_ptr_type x);
			bool chop();

			jobject append_slice();
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
			view_options			view;
			object_name				last_rule_name;

			actor_type() : actor_id(null_row)
			{
				;
			}
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
			time_t					last_modified;
			bool					deleted;
		};

		class create_object_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	class_id;
			relative_ptr_type	item_id;
			relative_ptr_type	template_item_id;
			bool				select_on_create;
			object_name			create_prompt;
			collection_id_type get_collection_id() { return collection_id; }
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
			bool				deletable;
			jobject				object;
		};

		class select_object_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			relative_ptr_type	object_id;
			bool				extend;
			collection_id_type get_collection_id() { return collection_id; }
		};

		class delete_selected_request
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			collection_id_type get_collection_id() { return collection_id; }
		};

		class update_object_request
		{
		public:
			object_member_path	path;
			actor_id_type		actor_id;
			jobject				item;
			collection_id_type get_collection_id() { return path.object.collection_id; }
		};

		using filtered_object_id_list = list_box<relative_ptr_type>;
		using filtered_object_list = list_box<jobject>;
		using filtered_actor_view_object_list = list_box<actor_view_object>;
		using actor_view_collection = sorted_index<relative_ptr_type, actor_view_object>;
		using actor_create_collection = sorted_index<relative_ptr_type, create_object_request>;
		using filtered_objects_collection = sorted_index<object_name, relative_ptr_type>;

		class actor_state;

		class actor_object_option
		{
		public:
			actor_view_object	avo;
			jobject				slice;
		};

		class actor_state
		{
			dynamic_box									data;
			relative_ptr_type							create_objects_location;
			relative_ptr_type							view_objects_location;
			relative_ptr_type							filter_results_location;
			jschema* schema;

		public:

			collection_id_type							collection_id;
			relative_ptr_type							actor_id;
			actor_create_collection						create_objects;
			actor_view_collection						view_objects;
			filtered_objects_collection					filter_results;
			jactor										actor;

			relative_ptr_type							modified_object_id;
			jobject										modified_object;

			actor_state()
			{
				data.init(100000);
				create_objects = actor_create_collection::create_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::create_sorted_index(&data, view_objects_location);
				filter_results = filtered_objects_collection::create_sorted_index(&data, filter_results_location);
				modified_object_id = null_row;
				check_objects("empty ctor");
			}

			actor_state(actor_state&& _src)
			{
				data = std::move(_src.data);
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				filter_results_location = _src.filter_results_location;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location );
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location );
				filter_results = filtered_objects_collection::get_sorted_index(&data, filter_results_location);
				for (auto avo : view_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				actor = _src.actor;
				check_objects("move ctor");
			}

			actor_state& operator=(actor_state&& _src)
			{
				data = std::move(_src.data);
				collection_id = _src.collection_id;
				actor_id = _src.actor_id;
				create_objects_location = _src.create_objects_location;
				view_objects_location = _src.view_objects_location;
				modified_object_id = _src.modified_object_id;
				filter_results_location = _src.filter_results_location;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);
				filter_results = filtered_objects_collection::get_sorted_index(&data, filter_results_location);
				for (auto avo : view_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				actor = _src.actor;
				check_objects("move assign");
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
				filter_results_location = _src.filter_results_location;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);
				filter_results = filtered_objects_collection::get_sorted_index(&data, filter_results_location);
				for (auto avo : view_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				actor = _src.actor;
				check_objects("copy assign");
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
				filter_results_location = _src.filter_results_location;
				create_objects = actor_create_collection::get_sorted_index(&data, create_objects_location);
				view_objects = actor_view_collection::get_sorted_index(&data, view_objects_location);
				filter_results = filtered_objects_collection::get_sorted_index(&data, filter_results_location);
				for (auto avo : view_objects) {
					avo.second.object.set_box_dangerous_hack(&data);
				}
				actor = _src.actor;
				check_objects("copy ctor");
			}

			serialized_box_container* get_data() 
			{
				return &data;
			}

			jobject create_object(jschema* _schema, relative_ptr_type _class_id);
			jobject copy_object(jschema* _schema, jobject& _src);
			actor_view_object get_modified_object();
			jobject get_object(object_member_path _path);
			object_member_path get_object_by_class(relative_ptr_type _class_id, member_path _path);
			object_id_type get_object_by_class(relative_ptr_type _class_id);
			object_member_path get_selected( relative_ptr_type _class_id );

			create_object_request create_create_request(relative_ptr_type _class_id)
			{
				if (!create_objects.contains(_class_id)) {
					throw std::invalid_argument("class is not creatable");
				}
				create_object_request aco = create_objects[_class_id].second;
				check_objects("create_request");
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
				check_objects("select_request");
				return aso;
			}

			update_object_request create_update_request()
			{
				update_object_request uor;
				uor.actor_id = actor_id;
				uor.path.object.collection_id = collection_id;
				uor.path.object.row_id = modified_object_id;
				uor.item = modified_object;
				return uor;
			}

			void check_objects(const char *name)
			{
				serialized_box_container* sbd = static_cast<serialized_box_container*>(&data);
				for (auto avo : view_objects) {
					avo.second.object.set_box_dangerous_hack(sbd);
					view_objects.put(avo);
				}

#if ACTOR_OBJECT_CHECKING
				// this ensures that all the objects in this state have their data set to be provided from this state,
				void* my_data = (void*)(&data);
				std::cout << "actor state box " << name << ", data " << my_data << ", has_data: " << data.has_data() << std::endl;
				bool is_ok = true;
				for (auto avo : view_objects) 
				{
					void *object_data = avo.second.object.get_box_address();
					if (object_data != my_data) {
						is_ok = false;
						std::cout << "object " << avo.second.object_id << " " << object_data << " is incorrect " << std::endl;
					}
				}
#endif
			}

			filtered_actor_view_object_list get_view_query_avo(const object_name& _name)
			{
				filtered_actor_view_object_list ret_value;
				ret_value = filtered_actor_view_object_list::create(&data);
				if (filter_results.contains(_name))
				{
					auto fr_loc = filter_results[_name];
					auto list = filtered_object_id_list::get(&data, fr_loc.second);
					for (auto item : list) {
						auto avo = view_objects[item];
						ret_value.push_back(avo.second);
					}
				}
				else
				{
					std::cout << "ERR:" << _name << " query results not found";
				}
				return ret_value;
			}

			filtered_actor_view_object_list get_view_query_avo(view_query& vq)
			{
				return get_view_query_avo(vq.query_name);
			}

			filtered_object_list get_view_query_obj(const object_name& _name)
			{
				filtered_object_list ret_value;
				ret_value = filtered_object_list::create(&data);
				if (filter_results.contains(_name))
				{
					auto fr_loc = filter_results[_name];
					auto list = filtered_object_id_list::get(&data, fr_loc.second);
					for (auto item : list) {
						auto avo = view_objects[item];
						ret_value.push_back(avo.second.object);
					}
				}
				else 
				{
					std::cout << "ERR:" << _name << " query results not found";
				}
				return ret_value;
			}

			filtered_object_list get_view_query_obj(view_query& vq)
			{
				return get_view_query_obj(vq.query_name);
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
			actor_state delete_selected(const delete_selected_request& _select, const char* _trace_msg = nullptr);

			void print(const char *_trace, actor_state& acr);
			jobject create_object(relative_ptr_type _item_id, relative_ptr_type _actor_id, relative_ptr_type _class_id, relative_ptr_type& _object_id);
			jobject create_object(relative_ptr_type _item_id, relative_ptr_type _actor_id, relative_ptr_type _class_id, relative_ptr_type& _object_id, std::initializer_list<dynamic_value> var);
			jobject get_object(relative_ptr_type _object_id);
			jobject get_object(object_member_path _path);
			jobject update_object(relative_ptr_type _object_id, jobject _slice);
			jobject update_object(object_member_path _path, jobject _slice);
			collection_object_type &get_object_reference(relative_ptr_type _object_id);
			relative_ptr_type create_class_from_template(relative_ptr_type _target_class_id, relative_ptr_type _source_template_object);

			bool selector_applies(selector_collection* _selector, actor_id_type& _actor);

			jobject get_at(relative_ptr_type _object_id);
			relative_ptr_type get_class_id(relative_ptr_type _object_id);
			relative_ptr_type get_base_id(relative_ptr_type _object_id);
			jobject get_style_sheet();

			bool object_is_class(relative_ptr_type _object_id, relative_ptr_type _class_id);
			bool object_is_class(const jobject& obj, relative_ptr_type _class_id);
			bool object_is_class(relative_ptr_type _object_id, std::vector<relative_ptr_type> _class_ids);
			bool object_is_class(relative_ptr_type _object_id, class_list& _class_ids);
			bool object_is_class(const jobject& obj, std::vector<relative_ptr_type> _class_ids);
			bool object_is_class(const jobject& obj, class_list& _class_ids);

			bool class_has_base(relative_ptr_type _class_id, relative_ptr_type _base_id);
			int64_t get_class_count(relative_ptr_type _class_id);

			filter_option create_filter_from_view_query(view_query& vq, actor_type* pactor);

			filtered_object_id_list run_filter(serialized_box_container* _data, filter_option& _stuff);
			void update(filter_option& _stuff, std::function<bool(jobject&_src)> updateor);

			filtered_object_id_list run_filter(serialized_box_container* _data, view_query& vq, actor_type *pactor);
			void update(view_query& vq, actor_type* pactor, std::function<bool(jobject& _src)> updateor);

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

			relative_ptr_type put_user_class(jobject& _class_definition, time_t _version);

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
			
			relative_ptr_type idf_full_name;
			relative_ptr_type idf_first_name;
			relative_ptr_type idf_last_name;
			relative_ptr_type idf_middle_name;
			relative_ptr_type idf_ssn;
			relative_ptr_type idf_email;
			relative_ptr_type idf_title;
			relative_ptr_type idf_street;
			relative_ptr_type idf_suiteapt;
			relative_ptr_type idf_city;
			relative_ptr_type idf_state;
			relative_ptr_type idf_postal;
			relative_ptr_type idf_country_name;
			relative_ptr_type idf_country_code;
			relative_ptr_type idf_institution_name;
			relative_ptr_type idf_long_name;
			relative_ptr_type idf_short_name;
			relative_ptr_type idf_unit;
			relative_ptr_type idf_symbol;
			relative_ptr_type idf_operator;
			relative_ptr_type idf_windows_path;
			relative_ptr_type idf_linux_path;
			relative_ptr_type idf_url;
			relative_ptr_type idf_username;
			relative_ptr_type idf_password;
			relative_ptr_type idf_doc_title;
			relative_ptr_type idf_section_title;
			relative_ptr_type idf_block_title;
			relative_ptr_type idf_caption;
			relative_ptr_type idf_paragraph;
			relative_ptr_type idf_mimeType;
			relative_ptr_type idf_base64;
			relative_ptr_type idf_file_name;
			relative_ptr_type idf_font_name;
			relative_ptr_type idf_name;
			relative_ptr_type idf_search_string;

			relative_ptr_type idf_birthday;
			relative_ptr_type idf_scheduled;

			relative_ptr_type idf_count;

			relative_ptr_type idf_quantity;
			relative_ptr_type idf_latitude;
			relative_ptr_type idf_longitude;
			relative_ptr_type idf_meters;
			relative_ptr_type idf_feet;
			relative_ptr_type idf_kilograms;
			relative_ptr_type idf_pounds;
			relative_ptr_type idf_seconds;
			relative_ptr_type idf_minutes;
			relative_ptr_type idf_hours;
			relative_ptr_type idf_amperes;
			relative_ptr_type idf_kelvin;
			relative_ptr_type idf_moles;
			relative_ptr_type idf_gradient_position;
			relative_ptr_type idf_font_size;
			relative_ptr_type idf_line_spacing;
			relative_ptr_type idf_box_border_thickness;
			relative_ptr_type idf_shape_border_thickness;

			relative_ptr_type idf_color;
			relative_ptr_type idf_shape_fill_color;
			relative_ptr_type idf_box_fill_color;
			relative_ptr_type idf_shape_border_color;
			relative_ptr_type idf_box_border_color;
			relative_ptr_type idf_point;
			relative_ptr_type idf_position_point;
			relative_ptr_type idf_selection_point;
			relative_ptr_type idf_rectangle;
			relative_ptr_type idf_layout_rect;

			relative_ptr_type idf_bold;
			relative_ptr_type idf_italic;
			relative_ptr_type idf_underline;
			relative_ptr_type idf_strike_through;
			relative_ptr_type idf_vertical_alignment;
			relative_ptr_type idf_horizontal_alignment;
			relative_ptr_type idf_wrap_text;

			relative_ptr_type idc_solid_brush;
			relative_ptr_type idc_gradient_stop;
			relative_ptr_type idc_linear_gradient_brush;
			relative_ptr_type idc_round_gradient_brush;
			relative_ptr_type idc_bitmap_brush;
			relative_ptr_type idc_text_style;

			relative_ptr_type idf_view_background_style;
			relative_ptr_type idf_view_title_style;
			relative_ptr_type idf_view_subtitle_style;
			relative_ptr_type idf_view_section_style;
			relative_ptr_type idf_view_style;
			relative_ptr_type idf_disclaimer_style;
			relative_ptr_type idf_copyright_style;
			relative_ptr_type idf_panel_style;
			relative_ptr_type idf_h1_style;
			relative_ptr_type idf_h2_style;
			relative_ptr_type idf_h3_style;
			relative_ptr_type idf_column_number_head_style;
			relative_ptr_type idf_column_text_head_style;
			relative_ptr_type idf_column_number_style;
			relative_ptr_type idf_column_text_style;
			relative_ptr_type idf_column_data_style;
			relative_ptr_type idf_button_style;
			relative_ptr_type idf_label_style;
			relative_ptr_type idf_control_style;
			relative_ptr_type idf_chart_axis_style;
			relative_ptr_type idf_chart_legend_style;
			relative_ptr_type idf_chart_block_style;
			relative_ptr_type idf_tooltip_style;
			relative_ptr_type idf_breadcrumb_style;
			relative_ptr_type idf_error_style;
			relative_ptr_type idf_company_a1_style;
			relative_ptr_type idf_company_a2_style;
			relative_ptr_type idf_company_a3_style;
			relative_ptr_type idf_company_b1_style;
			relative_ptr_type idf_company_b2_style;
			relative_ptr_type idf_company_b3_style;
			relative_ptr_type idf_company_c1_style;
			relative_ptr_type idf_company_c2_style;
			relative_ptr_type idf_company_c3_style;
			relative_ptr_type idf_company_d1_style;
			relative_ptr_type idf_company_d2_style;
			relative_ptr_type idf_company_d3_style;
			relative_ptr_type idf_company_deductible_style;
			relative_ptr_type idf_navigation_style;
			relative_ptr_type idf_company_neutral1_style;
			relative_ptr_type idf_company_neutral2_style;
			relative_ptr_type idf_header_area_style;
			relative_ptr_type idf_title_bar_style;
			relative_ptr_type idf_subtitle_bar_style;
			relative_ptr_type idf_breadcrumb_bar_style;
			relative_ptr_type idf_album_title_style;
			relative_ptr_type idf_artist_title_style;
			relative_ptr_type idf_work_title1_style;
			relative_ptr_type idf_work_title2_style;
			relative_ptr_type idf_work_title3_style;
			relative_ptr_type idf_work_title4_style;
			relative_ptr_type idf_work_title5_style;
			relative_ptr_type idf_work_title6_style;
			relative_ptr_type idf_album_about_style;
			relative_ptr_type idf_artist_about_style;

			relative_ptr_type idc_style_sheet;
			relative_ptr_type idc_style_sheet_set;

			relative_ptr_type idf_style_id;

			relative_ptr_type idc_user_class_root;
			relative_ptr_type idf_user_class_root;

			relative_ptr_type idc_user_class;
			relative_ptr_type idf_user_class;
			relative_ptr_type idf_user_class_class_name;
			relative_ptr_type idf_user_class_class_id;
			relative_ptr_type idf_base_class_id;

			relative_ptr_type idc_user_field;
			relative_ptr_type idf_user_field;
			relative_ptr_type idf_user_field_list;

			relative_ptr_type idc_string_options;
			relative_ptr_type idc_double_options;
			relative_ptr_type idc_int_options;
			relative_ptr_type idc_date_options;
			relative_ptr_type idc_object_options;

			relative_ptr_type idf_string_options;
			relative_ptr_type idf_double_options;
			relative_ptr_type idf_int_options;
			relative_ptr_type idf_date_options;
			relative_ptr_type idf_object_options;

			relative_ptr_type idf_field_name;
			relative_ptr_type idf_field_description;
			relative_ptr_type idf_field_format;
			relative_ptr_type idf_field_type;

			relative_ptr_type idf_string_length;
			relative_ptr_type idf_string_validation_pattern;
			relative_ptr_type idf_string_validation_message;
			relative_ptr_type idf_string_full_text_editor;
			relative_ptr_type idf_string_rich_text_editor;

			relative_ptr_type idf_date_start;
			relative_ptr_type idf_date_stop;
			relative_ptr_type idf_date_format;

			relative_ptr_type idf_double_start;
			relative_ptr_type idf_double_stop;
			relative_ptr_type idf_double_format;

			relative_ptr_type idf_int_start;
			relative_ptr_type idf_int_stop;
			relative_ptr_type idf_int_format;

			relative_ptr_type idf_object_class_id;
			relative_ptr_type idf_object_is_list;
			relative_ptr_type idf_object_x;
			relative_ptr_type idf_object_y;
			relative_ptr_type idf_object_z;

			relative_ptr_type idf_template_parameter_field_id;
			relative_ptr_type idf_template_parameter_field_type;
			relative_ptr_type idf_template_parameter_field_dim;
			relative_ptr_type idc_template_parameter_field;

			jschema() = default;
			~jschema() = default;

			jfield& get_empty()
			{
				return empty;
			}

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
				return put_field(request.name, sizeof(time_t), [request](jfield& _field)
					{
						_field.time_properties= request.options;
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
				request.name.type_id = jtype::type_object;
				request.options.total_size_bytes = request.options.dim.x * request.options.dim.y * request.options.dim.z * sizeb;
				return put_field(request.name, request.options.total_size_bytes, [request](jfield& _field)
					{
						_field.object_properties = request.options;
					});
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
				return put_field(request.name, request.options.total_size_bytes, [request](jfield& _field)
					{
						_field.object_properties = request.options;
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

			relative_ptr_type put_query_field(put_named_query_field_request request)
			{
				query_properties_type options;

				request.name.type_id = jtype::type_query;
				auto query_location = put_field(request.name, sizeof(query_instance), [request](jfield& _field)
					{
						;
					});

				queries.insert_or_assign(query_location, request.options);

				return query_location;
			}

			relative_ptr_type put_sql_remote_field(put_named_sql_remote_field_request request)
			{
				sql_properties_type options;

				request.name.type_id = jtype::type_sql;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				auto query_location = put_field(request.name, sizeof(sql_remote_instance), [request](jfield& _field)
					{
						;
					});

				sql_remotes.insert_or_assign(query_location, request.options);
				return query_location;
			}

			relative_ptr_type put_http_remote_field(put_named_http_remote_field_request request)
			{
				http_properties_type options;

				request.name.type_id = jtype::type_http;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				auto query_location = put_field(request.name, sizeof(http_remote_instance), [request](jfield& _field)
					{
						;
					});

				http_remotes.insert_or_assign(query_location, request.options);

				return query_location;
			}

			relative_ptr_type put_file_remote_field(put_named_file_remote_field_request request)
			{
				file_properties_type options;

				request.name.type_id = jtype::type_file;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				auto query_location = put_field(request.name, sizeof(file_remote_instance), [request](jfield& _field)
					{
						;
					});

				file_remotes.insert_or_assign(query_location, request.options);
				return query_location;
			}

			relative_ptr_type put_point_field(put_point_field_request request)
			{
				request.name.type_id = jtype::type_point;
				auto query_location = put_field(request.name, sizeof(point), [request](jfield& _field)
					{
						_field.point_properties = request.options;
					});
				return query_location;
			}

			relative_ptr_type put_rectangle_field(put_rectangle_field_request request)
			{
				request.name.type_id = jtype::type_rectangle;
				auto query_location = put_field(request.name, sizeof(rectangle), [request](jfield& _field)
					{
						_field.rectangle_properties = request.options;
					});
				return query_location;
			}

			relative_ptr_type put_layout_rect_field(put_layout_rect_field_request request)
			{
				request.name.type_id = jtype::type_layout_rect;
				auto query_location = put_field(request.name, sizeof(rectangle), [request](jfield& _field)
					{
						_field.layout_rect_properties = request.options;
					});
				return query_location;
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

			relative_ptr_type put_color_field(put_color_field_request request)
			{
				request.name.type_id = jtype::type_color;
				auto query_location = put_field(request.name, sizeof(color), [request](jfield& _field)
					{
						_field.color_properties = request.options;
					});
				return query_location;
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

					int pk_field_idx = cls.item().primary_key_idx;

					if (pk_field_idx >= 0)
					{
						pk_field = cls.detail(pk_field_idx);
						if (pk_field.field_id != request.field_id_primary_key) {
							pk_field = { null_row, 0};
						}
					}

					put_object_field_request porf;
					porf.name.type_id = jtype::type_object;
					porf.options.class_id = base_class_id;
					porf.options.dim = { 1, 1, 1 };

					auto field_id = put_object_field(porf);
					temp_fields.push_back(field_id);
				}

				temp_fields += mfs;

				build_class_members(af, total_size_bytes, temp_fields);

				if (pk_field.field_id > -1) 
				{
					af.push_back(pk_field);
				}

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

			relative_ptr_type get_class_primary_key(relative_ptr_type class_id)
			{
				auto the_class = classes[class_id];
				relative_ptr_type pk_index = the_class.item().primary_key_idx;
				if (pk_index < 0)
					return null_row;
				relative_ptr_type pk_id = the_class.detail(pk_index).field_id;
				return pk_id;
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
					if (ref->create_style_sheet) {
						tmp.create_object(0, null_row, idc_style_sheet_set, ref->style_sheet_id);
					}
				}
				return tmp;
			}
		};

		bool schema_tests();
		bool collection_tests();
		bool array_tests();
		bool model_tests();
	}
}
