#pragma once

#include "jfield.h"
#include "store_box.h"
#include "list_box.h"
#include "collection_id_box.h"
#include "object_id_box.h"
#include "point_box.h"
#include "rectangle_box.h"
#include "image_box.h"
#include "wave_box.h"
#include "midi_box.h"
#include "color_box.h"
#include "sql_remote_box.h"
#include "http_remote_box.h"
#include "file_remote_box.h"
#include "sorted_index.h"
#include "query_box.h"
#include "float_box.h"

#include <cassert>

namespace countrybit
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

			row_id_type			collection_location;

			object_name			model_name;
			uint32_t			max_actors;
			uint32_t			max_objects;
			uint64_t			collection_size_bytes;

			row_id_type			actors_id;
			row_id_type			objects_id;
			row_id_type			model_id;

			dynamic_box*		data;
		};

		struct jschema_map
		{
			row_id_type fields_table_id;
			row_id_type classes_table_id;
			row_id_type classes_by_name_id;
			row_id_type models_by_name_id;
			row_id_type fields_by_name_id;
			row_id_type	query_properties_id;
			row_id_type sql_properties_id;
			row_id_type file_properties_id;
			row_id_type http_properties_id;
			row_id_type models_id;
		};

		using actor_id_type = row_id_type;

		class jschema;
		class jarray;
		class jlist;

		class jslice
		{
			jslice* parent;
			jschema* schema;
			row_id_type class_id;
			char* bytes;
			dimensions_type dim;
			jclass the_class;

			serialized_box* box;
			row_id_type location;

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
			jslice(jslice* _parent, jschema* _schema, row_id_type _class_id, char* _bytes, dimensions_type _dim);
			jslice(jslice* _parent, jschema* _schema, row_id_type _class_id, serialized_box *_box, row_id_type _location, dimensions_type _dim);

			void construct();

			jslice& get_parent_slice();
			jclass get_class();

			dimensions_type get_dim();

			int get_field_index_by_id(row_id_type field_id);
			jfield& get_field_by_id(row_id_type field_id);
			jclass_field& get_class_field(int field_idx);
			jfield& get_field(int field_idx);

			int8_box get_int8(int field_idx);
			int16_box get_int16(int field_idx);
			int32_box get_int32(int field_idx);
			int64_box get_int64(int field_idx);
			float_box get_float(int field_idx);
			double_box get_double(int field_idx);
			time_box get_time(int field_idx);
			string_box get_string(int field_idx);
			jarray get_object(int field_idx);
			jlist get_list(int field_idx);
			collection_id_box get_collection_id(int field_idx);
			object_id_box get_object_id(int field_idx);
			point_box get_point(int field_idx);
			rectangle_box get_rectangle(int field_idx);
			image_box get_image(int field_idx);
			wave_box get_wave(int field_idx);
			midi_box get_midi(int field_idx);
			color_box get_color(int field_idx);
			query_box get_query(int field_idx);
			sql_remote_box get_sql_remote(int field_idx);
			http_remote_box get_http_remote(int field_idx);
			file_remote_box get_file_remote(int field_idx);

			bool set_filters(filter_element_collection& _src, jslice& _parameters);
			bool filter(filter_element_collection& _src, jslice& _parameters);
			bool set_filters(filter_element *_src, int _count, jslice& _parameters);
			bool filter(filter_element* _src, int _count, jslice& _parameters);
			bool set_updates(update_element_collection& _src, jslice& _parameters);
			bool update(update_element_collection& _src, jslice& _parameters);
			void update(jslice& _src_slice);

			std::partial_ordering compare(projection_element_collection& collection, jslice& _dest_slice);
			bool set_projection(projection_element_collection& collection);
			std::partial_ordering compare(int _dst_idx, jslice& _src_slice, int _src_idx);
			std::partial_ordering compare(jslice& _src_slice);

			template <typename boxed> boxed get(int field_idx)
			{
				size_t offset = get_offset(field_idx);
				char *src = get_bytes() + offset;
				boxed temp(src);
				return temp;
			}

			int size();
			char* get_bytes() { return box ? box->unpack<char>(location) : bytes;  };
			row_id_type size_bytes() { return get_class().item().class_size_bytes; }
		};

		class jarray
		{
			jschema* schema;
			row_id_type class_field_id;
			char* bytes;
			jslice* item;

		public:

			jarray();
			jarray(jslice* _parent, jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init = false);
			jarray(dynamic_box& _dest, jarray& _src);
			dimensions_type dimensions();

			jslice get_slice(int x, int y = 0, int z = 0);
			jslice get_slice(dimensions_type dims);
			uint64_t get_size_bytes();
			char* get_bytes();
		
			class iterator
			{
				jarray* base;
				dimensions_type current;
				dimensions_type maxd;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = jslice;
				using pointer = jslice*;  // or also value_type*
				using reference = jslice&;  // or also value_type&

				iterator(jarray* _base, dimensions_type _current) :
					base(_base),
					current(_current)
				{
					maxd = base->dimensions();
				}

				iterator() : base(nullptr), current({ 0, 0, 0 }), maxd( { 0, 0, 0 })
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jslice operator *()
				{
					return base->get_slice(current);
				}

				inline jslice operator->()
				{
					return base->get_slice(current);
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end()
				{
					return iterator(base, maxd);
				}

				inline iterator operator++()
				{
					current.x++;
					if (current.x >= maxd.x) {
						current.y++;
						current.x = 0;
						if (current.y >= maxd.y) {
							current.z++;
							if (current.z >= maxd.z) {
								current = maxd;
							}
							else 
							{
								current.y = 0;
							}
						}
					}
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			inline iterator begin()
			{
				dimensions_type dt = { 0, 0, 0 };

				return iterator(this, dt);
			}

			inline iterator end()
			{
				auto temp = this->dimensions();
				return iterator(this, temp);
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

		class jlist
		{
			jschema* schema;
			serialized_box* model_box;
			row_id_type class_field_id;
			jlist_state data;
			jslice* item;

		public:

			jlist();
			jlist(jslice* _parent, jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init = false);
			jlist(dynamic_box& _dest, jlist& _src);

			uint32_t capacity();
			uint32_t size();

			jslice get_slice_direct(int idx);
			jslice get_slice(int x);
			bool erase_slice(int x);
			bool chop();

			jslice append_slice();
			bool select_slice(int x);
			bool deselect_slice(int x);
			void deselect_all();
			void select_all();
			void clear();

			uint64_t get_size_bytes();
			char* get_bytes();

			void sort(projection_element_collection& projections);

			class iterator
			{
				jlist* base;
				uint32_t current;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = jslice;
				using pointer = jslice*;  // or also value_type*
				using reference = jslice&;  // or also value_type&

				iterator(jlist* _base, uint32_t _current) :
					base(_base),
					current(_current)
				{
					;
				}

				iterator() : base(nullptr), current(0)
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jslice operator *()
				{
					return base->get_slice(current);
				}

				inline jslice operator->()
				{
					return base->get_slice(current);
				}

				inline iterator begin() const
				{
					return iterator(base, current);
				}

				inline iterator end()
				{
					return iterator(base, base->size());
				}

				inline iterator operator++()
				{
					current++;
					if (current > base->size()) 
					{
						current = base->size();
					}
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			inline iterator begin()
			{
				return iterator(this, 0);
			}

			inline iterator end()
			{
				return iterator(this, capacity());
			}

		};

		class actor_type
		{
		public:
			collection_id_type		collection_id;
			actor_id_type			actor_id;
			object_name				actor_name;
			selections_collection	selections;
		};

		using jactor = actor_type;

		class collection_object_type
		{
		public:
			object_id_type	oid;
			jtype			otype;
			row_id_type		class_id;
			row_id_type		class_field_id;
			actor_id_type	actor_id;
			row_id_type		item_id;
			bool			deleted;
		};

		class actor_select_object
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			row_id_type			object_id;
			bool				extend;
		};

		class actor_create_object
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			row_id_type			class_id;
			row_id_type			item_id;
			bool				select_on_create;
			jslice				item;
		};

		class actor_update_object
		{
		public:
			collection_id_type	collection_id;
			actor_id_type		actor_id;
			row_id_type			object_id;
			bool				selected;
			jslice				item;
		};

		class actor_view_object
		{
		public:
			collection_id_type	collection_id;
			row_id_type			object_id;
			bool				selected;
			jslice				item;
		};

		using actor_create_option_collection = list_box<actor_create_object>;
		using actor_update_option_collection = list_box<actor_update_object>;
		using actor_select_option_collection = list_box<actor_select_object>;
		using actor_view_collection = list_box<actor_view_object>;

		class actor_command_response
		{
			row_id_type									create_options_id;
			row_id_type									update_options_id;
			row_id_type									select_options_id;
			row_id_type									view_objects_id;
			dynamic_box									data;

		public:

			collection_id_type							collection_id;
			row_id_type									actor_id;
			actor_create_option_collection				create_options;
			actor_update_option_collection				update_options;
			actor_select_option_collection				select_options;
			actor_view_collection						view_objects;

			actor_command_response()
			{
				;
			}

			actor_command_response(int _size)
			{
				data.init(_size);
				create_options_id = actor_create_option_collection::create(data.get_box());
				update_options_id = actor_update_option_collection::create(data.get_box());
				select_options_id = actor_select_option_collection::create(data.get_box());
				view_objects_id = actor_view_collection::create(data.get_box());
			}

			actor_command_response(actor_command_response&& _src)
			{
				data = std::move(_src.data);
			}

			actor_command_response& operator=(actor_command_response&& _src)
			{
				data = std::move(_src.data);
				return *this;
			}

			actor_command_response operator=(const actor_command_response& _src) = delete;
			actor_command_response(const actor_command_response& _src) = delete;

			jslice create_object(jschema* _schema, row_id_type _class_id);
			jslice copy_object(jschema* _schema, jslice& _src);

		};

		using actor_collection = table<actor_type>;
		using object_collection = item_details_table<collection_object_type, char>;

		class jcollection
		{

			jschema* schema;
			jcollection_ref* ref;

			collection_id_type		collection_id;
			row_id_type				model_id;
			actor_collection		actors;
			object_collection		objects;

		public:

			jcollection() : schema( nullptr )
			{
				;
			}

			jcollection(jschema* _schema, jcollection_ref *_ref) :
				schema(_schema),
				ref(_ref),
				collection_id(_ref->collection_id),
				model_id(_ref->model_id)
			{
				actors = actor_collection::get_table(_ref->data, _ref->actors_id );
				objects = object_collection::get_table(_ref->data, _ref->objects_id );
			}

			jcollection(jcollection& _src) :
				schema(_src.schema),
				ref(_src.ref),
				collection_id(_src.collection_id),
				model_id(_src.model_id)
			{
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
			}

			jcollection(jcollection&& _src) :
				schema(_src.schema),
				ref(_src.ref),
				collection_id(_src.collection_id),
				model_id(_src.model_id)
			{
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
			}

			jcollection operator =(jcollection&& _src) 
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				model_id = _src.model_id;
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			jcollection operator =(jcollection& _src)
			{
				schema = _src.schema;
				ref = _src.ref;
				collection_id = _src.collection_id;
				model_id = _src.model_id;
				actors = actor_collection::get_table(ref->data, ref->actors_id);
				objects = object_collection::get_table(ref->data, ref->objects_id);
				return *this;
			}

			actor_type create_actor(actor_type _actor);
			actor_id_type put_actor(actor_type _actor);
			actor_type get_actor(actor_id_type _actor_id);
			actor_id_type find_actor(object_name& name);
			actor_type update_actor(actor_type _actor);

			actor_command_response get_command_result(row_id_type _actor);
			actor_command_response select_object(const actor_select_object& _select);
			actor_command_response create_object(actor_create_object& _create);
			actor_command_response update_object(actor_update_object& _update);

			jslice create_object(row_id_type _item_id, row_id_type _actor_id, row_id_type _class_id, row_id_type& object_id);
			jslice get_object(row_id_type _object_id);
			jslice update_object(row_id_type _object_id, jslice _slice);

			bool selector_applies(selector_collection* _selector, actor_id_type& _actor);

			row_id_type size()
			{
				return objects.size();
			}

			class iterator
			{
				jcollection* base;
				row_id_type current;

			public:
				using iterator_category = std::forward_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = row_id_type;
				using pointer = row_id_type*;  // or also value_type*
				using reference = row_id_type&;  // or also value_type&

				iterator(jcollection* _base, row_id_type _current) :
					base(_base),
					current(_current)
				{

				}

				iterator() 
					: base(nullptr), 
					current(null_row)
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jslice operator *()
				{
					return base->get_object(current);
				}

				inline jslice operator->()
				{
					return base->get_object(current);
				}

				iterator begin() const
				{
					return iterator(base, current);
				}

				iterator end() const
				{
					return iterator(base, base->size());
				}

				inline iterator operator++()
				{
					current++;
					if (current > base->size())
						current = base->size();
					return iterator(base, current);
				}

				inline iterator operator++(int)
				{
					iterator tmp(*this);
					operator++();
					return tmp;
				}

				bool operator == (const iterator& _src) const
				{
					return _src.current == current;
				}

				bool operator != (const iterator& _src)
				{
					return _src.current != current;
				}

			};

			inline iterator begin()
			{
				return iterator(this, 0);
			}

			inline iterator end()
			{
				return iterator(this, this->size());
			}

		};

		class jschema
		{

		protected:

			using field_store_type = table<jfield>;
			using class_store_type = item_details_table<jclass_header, jclass_field>;
			using class_index_type = sorted_index<object_name, row_id_type>;
			using field_index_type = sorted_index<object_name, row_id_type>;
			using model_index_type = sorted_index<object_name, row_id_type>;
			using query_store_type = table<query_definition_type>;
			using sql_store_type = table<sql_definition_type>;
			using file_store_type = table<file_definition_type>;
			using http_store_type = table<http_definition_type>;
			using model_store_type = table<model_type>;

			field_store_type		fields;
			class_store_type		classes;
			class_index_type		classes_by_name;
			field_index_type		fields_by_name;
			model_index_type		models_by_name;
			query_store_type		queries;
			sql_store_type			sql_remotes;
			file_store_type			file_remotes;
			http_store_type			http_remotes;
			model_store_type		models;

			jfield					empty;

		public:

			jschema() = default;
			~jschema() = default;

			jfield& get_empty()
			{
				return empty;
			}

			static row_id_type reserve_schema(serialized_box* _b, int _num_classes, int _num_models, bool _use_standard_fields)
			{
				int _num_class_fields = _num_classes * 64 + ( _use_standard_fields ? 200 : 0 );
				int _num_queries = _num_classes * 4;
				int _num_sql_remotes = _num_classes * 4;
				int _num_http_remotes = _num_classes * 4;
				int _num_file_remotes = _num_classes * 4;

				jschema_map schema_map, *pschema_map;
				schema_map.fields_table_id = null_row;
				schema_map.classes_table_id = null_row;
				schema_map.classes_by_name_id = null_row;
				schema_map.fields_by_name_id = null_row;
				schema_map.models_by_name_id = null_row;
				schema_map.query_properties_id = null_row;
				schema_map.sql_properties_id = null_row;
				schema_map.file_properties_id = null_row;
				schema_map.http_properties_id = null_row;
				schema_map.models_id = null_row;

				auto total_size = jschema::get_box_size(_num_classes, _num_models, _use_standard_fields);
				_b->expand_check(total_size);

				row_id_type rit = _b->pack(schema_map);
				pschema_map = _b->unpack<jschema_map>(rit);
				pschema_map->fields_table_id = field_store_type::reserve_table(_b, _num_class_fields);
				pschema_map->classes_table_id = class_store_type::reserve_table(_b, _num_classes, _num_class_fields);
				pschema_map->classes_by_name_id = field_index_type::reserve_sorted_index(_b, _num_classes);
				pschema_map->fields_by_name_id = class_index_type::reserve_sorted_index(_b, _num_class_fields);
				pschema_map->models_by_name_id = model_index_type::reserve_sorted_index(_b, _num_models);
				pschema_map->query_properties_id = query_store_type::reserve_table(_b, _num_queries);
				pschema_map->sql_properties_id = sql_store_type::reserve_table(_b, _num_sql_remotes);
				pschema_map->file_properties_id = file_store_type::reserve_table(_b, _num_file_remotes);
				pschema_map->http_properties_id = http_store_type::reserve_table(_b, _num_http_remotes);
				pschema_map->models_id = model_store_type::reserve_table(_b, _num_models);
				return rit;
			}

			static jschema get_schema(serialized_box* _b, row_id_type _row)
			{
				jschema schema;
				jschema_map* pschema_map;
				pschema_map = _b->unpack<jschema_map>(_row);
				schema.fields = field_store_type::get_table(_b, pschema_map->fields_table_id);
				schema.classes = class_store_type::get_table(_b, pschema_map->classes_table_id);
				schema.classes_by_name = class_index_type::get_sorted_index(_b, pschema_map->classes_by_name_id);
				schema.fields_by_name = field_index_type::get_sorted_index(_b, pschema_map->fields_by_name_id);
				schema.models_by_name = model_index_type::get_sorted_index(_b, pschema_map->models_by_name_id);
				schema.queries = query_store_type::get_table(_b, pschema_map->query_properties_id);
				schema.sql_remotes = sql_store_type::get_table(_b, pschema_map->sql_properties_id);
				schema.file_remotes = file_store_type::get_table(_b, pschema_map->file_properties_id);
				schema.http_remotes = http_store_type::get_table(_b, pschema_map->http_properties_id);
				schema.models = model_store_type::get_table(_b, pschema_map->models_id);
				schema.empty.field_id = null_row;
				schema.empty.type_id = jtype::type_null;
				schema.empty.name = "empty";
				schema.empty.description = "indicates a field not found";
				return schema;
			}

			static int64_t get_box_size(int _num_classes, int _num_models, bool _use_standard_fields)
			{
				int _num_class_fields = _num_classes * 64 + (_use_standard_fields ? 200 : 0);
				int _num_queries = _num_classes * 4;
				int _num_sql_remotes = _num_classes * 4;
				int _num_http_remotes = _num_classes * 4;
				int _num_file_remotes = _num_classes * 4;

				int64_t field_size = field_store_type::get_box_size(_num_class_fields);
				int64_t class_size = class_store_type::get_box_size(_num_classes, _num_class_fields);
				int64_t classes_by_name_size = class_index_type::get_box_size(_num_classes);
				int64_t fields_by_name_size = field_index_type::get_box_size(_num_class_fields);
				int64_t models_by_name_size = field_index_type::get_box_size(_num_class_fields);
				int64_t query_size = query_store_type::get_box_size(_num_queries);
				int64_t sql_size = sql_store_type::get_box_size(_num_sql_remotes);
				int64_t file_size = file_store_type::get_box_size(_num_file_remotes);
				int64_t http_size = http_store_type::get_box_size(_num_http_remotes);
				int64_t model_size = model_store_type::get_box_size(_num_models);
				int64_t total_size = field_size + class_size + models_by_name_size + classes_by_name_size + fields_by_name_size + query_size + sql_size + http_size + file_size;
				return total_size * 3 / 2;
			}

			static jschema create_schema(serialized_box* _b, int _num_classes, int _num_fields, bool _use_standard_fields, row_id_type& _location)
			{
				_location = reserve_schema(_b, _num_classes, _num_fields, _use_standard_fields);
				jschema schema = get_schema(_b, _location);
				schema.add_standard_fields();
				return schema;
			}

			void add_standard_fields();

			bool is_empty(jfield& fld)
			{
				return fld.field_id == null_row && fld.type_id == jtype::type_null;
			}

			row_id_type new_field_id()
			{
				return fields.create(1).start;
			}

			row_id_type put_field(
				row_id_type _field_id,
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

				jf.size_bytes = _size_bytes;

				fields_by_name.insert_or_assign(jf.name, _field_id);
				return _field_id;
			}

			row_id_type put_string_field(put_string_field_request request)
			{
				return put_field(request.name.field_id, jtype::type_string, request.name.name, request.name.description, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, string_box::get_box_size(request.options.length));
			}

			row_id_type put_time_field(put_time_field_request request)
			{
				return put_field(request.name.field_id, type_datetime, request.name.name, request.name.description, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(time_t));
			}

			row_id_type put_integer_field(put_integer_field_request request)
			{
				switch (request.name.type_id)
				{
				case jtype::type_int8:
					return put_field(request.name.field_id, type_int8, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int8_t));
				case jtype::type_int16:
					return put_field(request.name.field_id, type_int16, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int16_t));
				case jtype::type_int32:
					return put_field(request.name.field_id, type_int32, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int32_t));
				case jtype::type_int64:
					return put_field(request.name.field_id, type_int64, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int64_t));
				default:
					throw std::invalid_argument("Invalid integer type for field name:" + request.name.name);
				}
			}

			row_id_type put_double_field(put_double_field_request request)
			{
				switch (request.name.type_id)
				{
				case type_float32:
					return put_field(request.name.field_id, type_float32, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(float));
				case type_float64:
					return put_field(request.name.field_id, type_float64, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(double));
				default:
					throw std::invalid_argument("Invalid floating point type for field name:" + request.name.name);
				}
			}

			void get_class_field_name(object_name& _dest, object_name _class_name, dimensions_type& _dim)
			{
				_dest = _class_name + "[" + std::to_string(_dim.x) + "," + std::to_string(_dim.y) + "," + std::to_string(_dim.z) + "]";
			}

			row_id_type put_object_field(put_object_field_request request)
			{
				auto pcr = classes[request.options.class_id];
				auto& p = pcr.item();
				int64_t sizeb = pcr.item().class_size_bytes;
				request.options.class_size_bytes = sizeb;
				if (request.options.dim.x == 0) request.options.dim.x = 1;
				if (request.options.dim.y == 0) request.options.dim.y = 1;
				if (request.options.dim.z == 0) request.options.dim.z = 1;
				object_name field_name;
				get_class_field_name(field_name, pcr.pitem()->name, request.options.dim);
				request.options.total_size_bytes = request.options.dim.x * request.options.dim.y * request.options.dim.z * sizeb;
				return put_field(request.name.field_id, type_object, field_name, request.name.description, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, request.options.total_size_bytes);
			}

			row_id_type put_list_field(put_object_field_request request)
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
				return put_field(request.name.field_id, type_list, field_name, request.name.description, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, request.options.total_size_bytes);
			}

			const char* invalid_comparison = "Invalid comparison";
			const char* invalid_parameter_field = "Invalid parameter field";
			const char* invalid_target_field = "Invalid target field";
			const char* invalid_projection_field = "Invalid projection field";

			void bind_field(object_name& _field_name, row_id_type& _field_id)
			{
				auto fiter = fields_by_name[_field_name];
				if (fiter != std::end(fields_by_name)) {
					_field_id = fiter->second;
					return;
				}
				_field_id = null_row;
				throw std::logic_error("[" + _field_name + "] not found.");
			}

			void bind_class(object_name& _class_name, row_id_type& _class_id)
			{
				auto fiter = classes_by_name[_class_name];
				if (fiter != std::end(classes_by_name)) {
					_class_id = fiter->second;
					return;
				}
				_class_id = null_row;
				throw std::logic_error("[" + _class_name + "] not found.");
			}

			void bind_projection_operation(operation_name& _operation, projection_operations& _nt)
			{
				_nt = projection_operations::group_by;

				if (_operation == "group_by")
				{
					_nt = projection_operations::group_by;
				}
				else if (_operation == "calc_min")
				{
					_nt = projection_operations::calc_min;
				}
				else if (_operation == "calc_max")
				{
					_nt = projection_operations::calc_max;
				}
				else if (_operation == "calc_count")
				{
					_nt = projection_operations::calc_count;
				}
				else if (_operation == "calc_stddev")
				{
					_nt = projection_operations::calc_stddev;
				}
				else 
				{
					throw std::logic_error("[" + _operation + "] not found.");
				}
			}

			void bind_filter_comparison_type(operation_name& _comparison, filter_comparison_types& _fct)
			{
				_fct = filter_comparison_types::eq;
				if (_comparison == "eq")
				{
					_fct = filter_comparison_types::eq;
				}
				else if (_comparison == "ls")
				{
					_fct = filter_comparison_types::ls;
				}
				else if (_comparison == "gt")
				{
					_fct = filter_comparison_types::gt;
				}
				else if (_comparison == "lseq")
				{
					_fct = filter_comparison_types::lseq;
				}
				else if (_comparison == "gteq")
				{
					_fct = filter_comparison_types::gteq;
				}
				else if (_comparison == "contains")
				{
					_fct = filter_comparison_types::contains;
				}
				else if (_comparison == "inlist")
				{
					_fct = filter_comparison_types::inlist;
				}
				else if (_comparison == "distance")
				{
					_fct = filter_comparison_types::distance;
				}
				else {
					throw std::logic_error("[" + _comparison + "] not found.");
				}
			}

			row_id_type put_query_field(put_named_query_field_request request)
			{
				query_properties_type options;
				row_range rr;
				put_class_request pcr;

				pcr.class_name = request.options.result_class_name;

				auto& path = request.options.source_path;
				bind_class(path.root.class_name, path.root.class_id);

				row_id_type member_index = 0;

				for (auto nd : path.nodes) 
				{
					auto &ndi = nd.item;
					bind_field(ndi.member_name, ndi.member_id);
				}

				auto& projections = request.options.projection;

				for (auto proj : projections)
				{
					auto& proji = proj.item;
					bind_field(proji.field_name, proji.field_id);
					bind_projection_operation(proji.projection_name, proji.projection);
					member_field mf(proji.field_id);
					mf.field_name = proji.field_name;
					pcr.member_fields.push_back(mf);
					member_index++;
				}

				auto& filter = request.options.filter;
				for (auto fil : filter)
				{
					auto& fili = fil.item;
					bind_filter_comparison_type(fili.comparison_name, fili.comparison);
					bind_field(fili.parameter_field_name, fili.parameter_field_id);
					bind_field(fili.target_field_name, fili.target_field_id);
				}

 				request.options.result_class_id = put_class(pcr);
				if (request.options.result_class_id == null_row)
					return null_row;

				put_object_field_request porf;
				porf.name.name = request.options.result_class_name;
				porf.name.type = jtype::type_list;
				porf.options.dim = { request.options.max_result_objects, 1, 1 };
				porf.options.class_id = request.options.result_class_id;
				porf.options.class_name = request.options.result_class_name;

				queries.append(request.options, rr);
				options.properties_id = rr.start;

				return put_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(query_instance));
			}

			row_id_type put_sql_remote_field(put_named_sql_remote_field_request request)
			{
				sql_properties_type options;
				row_range rr;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				sql_remotes.append(request.options, rr);
				options.properties_id = rr.start;

				return put_field(request.name.field_id, type_sql, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, sizeof(sql_remote_instance));
			}

			row_id_type put_http_remote_field(put_named_http_remote_field_request request)
			{
				http_properties_type options;
				row_range rr;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				http_remotes.append(request.options, rr);
				options.properties_id = rr.start;

				return put_field(request.name.field_id, type_http, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, sizeof(http_remote_instance));
			}

			row_id_type put_file_remote_field(put_named_file_remote_field_request request)
			{
				file_properties_type options;
				row_range rr;

				auto& params = request.options.parameters;
				for (auto param : params) {
					auto& pi = param.item;
					bind_field(pi.corona_field, pi.corona_field_id);
				}

				file_remotes.append(request.options, rr);
				options.properties_id = rr.start;

				return put_field(request.name.field_id, type_sql, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, sizeof(file_remote_instance));
			}

			row_id_type put_point_field(put_point_field_request request)
			{
				return put_field(request.name.field_id, type_point, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(point));
			}

			row_id_type put_rectangle_field(put_rectangle_field_request request)
			{
				return put_field(request.name.field_id, type_rectangle, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(rectangle));
			}

			row_id_type put_image_field(put_image_field_request request)
			{
				return put_field(request.name.field_id, type_image, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(image_instance));
			}

			row_id_type put_wave_field(put_wave_field_request request)
			{
				return put_field(request.name.field_id, type_wave, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(wave_instance));
			}

			row_id_type put_midi_field(put_midi_field_request request)
			{
				return put_field(request.name.field_id, type_midi, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, sizeof(midi_instance));
			}

			row_id_type put_color_field(put_color_field_request request)
			{
				return put_field(request.name.field_id, type_color, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, sizeof(color));
			}

			row_id_type put_list_field(row_id_type class_id, int max_rows )
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

			row_id_type get_result_field_class(jfield fld, row_id_type &_max_result_rows)
			{
				row_id_type result_class_id = null_row;
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

			row_id_type build_class_members(jclass pcr, member_field_collection &mfs)
			{
				int field_idx = 0;
				int sz = mfs.size();
				auto& p = pcr.item();

				for (int i = 0; i < sz; i++)
				{
					auto& field = mfs[i];

					switch (field.membership_type)
					{
					case member_field_types::member_field:
					{
						row_id_type fid;
						if (!field.use_id) {
							auto fname = fields_by_name[field.field_name];
							if (fname == std::end(fields_by_name)) {
								throw std::logic_error("[" + field.field_name + " ] not found");
							}
							fid = fname->second;
						}
						else
						{
							fid = field.field_id;
						}
						auto& existing_field = fields[fid];
						auto& ref = pcr.detail(field_idx);
						field_idx++;
						ref.field_id = fid;
						ref.offset = p.class_size_bytes;
						p.class_size_bytes += existing_field.size_bytes;

						if (existing_field.is_data_generator()) {
							row_id_type max_result_rows = 0;
							row_id_type result_class_field = this->get_result_field_class(existing_field, max_result_rows);
							auto& ref = pcr.detail(field_idx);
							field_idx++;
							ref.field_id = result_class_field;
							ref.offset = p.class_size_bytes;
							p.class_size_bytes += existing_field.size_bytes;
						}
					}
					break;
					case member_field_types::member_class:
					{
						put_object_field_request porf;
						if (!field.use_id) {
							auto class_name = classes_by_name[field.field_name];
							if (class_name == std::end(classes_by_name)) {
								throw std::logic_error("[" + field.field_name + " ] not found");
							}
							porf.name.name = class_name.get_key();
							porf.name.field_id = null_row;
							porf.name.type_id = jtype::type_object;
							porf.options.class_name = class_name.get_key();
							porf.options.class_id = class_name.get_value();
							porf.options.class_size_bytes = classes[class_name.get_value()].pitem()->class_size_bytes;
							porf.options.dim = field.dimensions;
						}
						else
						{
							auto class_cls = classes[field.class_id];
							porf.name.name = class_cls.item().name;
							porf.name.field_id = null_row;
							porf.name.type_id = jtype::type_object;
							porf.options.class_name = class_cls.item().name;
							porf.options.class_id = field.class_id;
							porf.options.class_size_bytes = classes[field.class_id].pitem()->class_size_bytes;
							porf.options.dim = field.dimensions;
						}
						auto class_field_id = put_object_field(porf);
						if (class_field_id == null_row) {
							return null_row;
						}
						auto& existing_field = fields[class_field_id];
						auto& ref = pcr.detail(field_idx);
						field_idx++;
						ref.field_id = class_field_id;
						ref.offset = p.class_size_bytes;
						p.class_size_bytes += existing_field.size_bytes;
					}
					break;
					}

				}
			}

			row_id_type put_class(put_class_request request)
			{
				row_id_type class_id = find_class(request.class_name);
				request.class_id = class_id;

				auto& mfs = request.member_fields;
				auto sz = mfs.size();
				int num_integration_fields = mfs.count_if([this](member_field& src) {
					auto& f = this->get_field(src.field_id);
					return f.is_data_generator();
					});

				auto pcr = classes.put_item(request.class_id, sz);

				build_class_members(pcr, request.member_fields);

				auto& p = pcr.item();
				p.class_id = pcr.row_id();
				p.name = request.class_name;
				p.description = request.class_description;
				p.class_size_bytes = 0;
				p.is_model = false;

				return p.class_id;
			}

			bool class_has_field(row_id_type class_id, row_id_type field_id)
			{
				auto cls = classes[class_id];
				for (int i = 0; i < cls.size(); i++)
				{
					if (cls.detail(i).field_id == field_id) return true;
				}
				return false;
			}

			row_id_type put_model(jmodel request)
			{
				row_id_type model_id = find_model(request.model_name);
				request.model_id = model_id;

				for (auto opt : request.create_options)
				{
					for (auto sel : opt.item.selectors.rules) 
					{
						bind_class(sel.item.class_name, sel.item.class_id);
					}
					bind_class(opt.item.create_class_name, opt.item.create_class_id);
					bind_class(opt.item.item_id_class_name, opt.item.item_id_class);
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

				if (request.model_id != null_row) 
				{
					models[request.model_id] = request;
					model_id = request.model_id;
				}
				else 
				{
					row_range rr;
					models.append(request, rr);
					model_id = rr.start;
					request.model_id = model_id;
					models[model_id].model_id = model_id;
					models_by_name.insert_or_assign(request.model_name, model_id);
				}
				return model_id;
			}

			row_id_type find_class(const object_name& class_name)
			{
				auto citer = classes_by_name[class_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			row_id_type find_field(const object_name& field_name)
			{
				auto citer = fields_by_name[field_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			row_id_type find_model(const object_name& field_name)
			{
				auto citer = models_by_name[field_name];
				if (citer != std::end(citer)) {
					return citer->second;
				}
				return null_row;
			}

			jmodel get_model(row_id_type model_id)
			{
				auto the_model = models[model_id];
				return the_model;
			}

			jclass get_class(row_id_type class_id)
			{
				auto the_class = classes[class_id];
				return the_class;
			}

			jfield &get_field(row_id_type field_id)
			{
				auto& the_field = fields[field_id];
				return the_field;
			}

			const query_definition_type& get_query_definition(row_id_type field_id)
			{
				auto& f = fields[field_id];
				auto xid = f.query_properties.properties_id;
				return queries[xid];
			}

			const sql_definition_type& get_sql_definition(row_id_type field_id)
			{
				auto& f = fields[field_id];
				auto xid = f.sql_properties.properties_id;
				return sql_remotes[xid];
			}

			const file_definition_type& get_file_definition(row_id_type field_id)
			{
				auto& f = fields[field_id];
				auto xid = f.file_properties.properties_id;
				return file_remotes[xid];
			}

			const http_definition_type& get_http_definition(row_id_type field_id)
			{
				auto& f = fields[field_id];
				auto xid = f.http_properties.properties_id;
				return http_remotes[xid];
			}

			uint64_t get_max_object_size(row_id_type* _class_ids)
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

			int64_t get_collection_size(jcollection_ref *ref, row_id_type* _class_ids)
			{
				int64_t max_size = get_max_object_size(_class_ids);

				int64_t total_size = 0;
				total_size += actor_collection::get_box_size(ref->max_actors);
				total_size += object_collection::get_box_size(ref->max_objects, max_size * ref->max_objects);
				return total_size;
			}

			bool reserve_collection(jcollection_ref *ref, row_id_type* _class_ids)
			{
				uint64_t total_size = get_collection_size(ref, _class_ids);

				ref->data->expand_check(total_size);

				uint64_t max_size = get_max_object_size(_class_ids);

				ref->actors_id = actor_collection::reserve_table(ref->data, ref->max_actors);
				ref->objects_id =  object_collection::reserve_table(ref->data, ref->max_objects, max_size * ref->max_objects);

				return ref->actors_id != null_row && ref->objects_id != null_row;
			}

			jcollection get_collection(jcollection_ref* ref)
			{
				jcollection collection(this, ref);
				return collection;
			}

			jcollection create_collection(jcollection_ref* ref, row_id_type* _class_ids)
			{
				bool reserved = reserve_collection(ref, _class_ids);
				jcollection tmp;
				if (reserved) {
					tmp = get_collection(ref);
				}
				return tmp;
			}
		};

		class jarray;

		bool schema_tests();
		bool collection_tests();
		bool array_tests();

	}
}
