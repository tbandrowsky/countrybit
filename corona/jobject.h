#pragma once

#include "jfield.h"

#include <cassert>

namespace countrybit
{
	namespace database
	{


		// a store id is in fact, a guid

		class jclass_header
		{
		public:
			row_id_type						class_id;
			object_name						name;
			object_description				description;
			uint64_t						class_size_bytes;
		};

		class jclass_field
		{
		public:
			row_id_type				field_id;
			uint64_t				offset;
		};

		using jclass_table = parent_child_table<jclass_header, jclass_field>;
		using jclass = parent_child_holder<jclass_header, jclass_field>;

		struct jschema_map
		{
			row_id_type fields_table_id;
			row_id_type classes_table_id;
			row_id_type classes_by_name_id;
			row_id_type fields_by_name_id;
		};

		class jcollection_map
		{
		public:
			collection_id_type collection_id;
			row_id_type table_id;
		};

		class jobject_header
		{
		public:
			object_id_type oid;
			row_id_type class_field_id;
		};

		class jschema;
		class jarray;

		class jslice
		{
			jschema* schema;
			row_id_type class_field_id;
			char* bytes;
			dimensions_type dim;
			jfield* class_field;
			jclass the_class;

			size_t get_offset(jtype field_type_id, int field_idx);

			template <typename T> T get_boxed(jtype jt, int field_idx)
			{
				size_t offset = get_offset(jt, field_idx);
				T b = &bytes[offset];
				return b;
			}

		public:

			jslice();
			jslice(jschema* _schema, row_id_type _class_field_id, char* _bytes, dimensions_type _dim);

			void construct();
			dimensions_type get_dim();
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
			collection_id_box get_collection_id(int field_idx);
			object_id_box get_object_id(int field_idx);
			point_box get_point(int field_idx);
			rectangle_box get_rectangle(int field_idx);
			image_box get_image(int field_idx);
			wave_box get_wave(int field_idx);
			midi_box get_midi(int field_idx);
			color_box get_color(int field_idx);
			int size();
		};

		class jarray
		{
			jschema* schema;
			row_id_type class_field_id;
			char* bytes;

		public:

			jarray() : schema(nullptr), class_field_id(null_row), bytes(nullptr)
			{
				;
			}

			jarray(jschema* _schema, row_id_type _class_field_id, char* _bytes) : schema( _schema ), class_field_id( _class_field_id ), bytes( _bytes )
			{

			}

			dimensions_type dimensions();

			jslice get_slice(int x, int y = 0, int z = 0);
			jslice get_slice(dimensions_type dims);

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

		class jcollection
		{

			jschema* schema;
			collection_id_type collection_id;
			parent_child_table<jobject_header, char> objects;

		public:

			jcollection() : schema( nullptr )
			{
				;
			}

			jcollection(jschema* _schema, collection_id_type _collection_id, parent_child_table<jobject_header, char>& _objects) :
				schema(_schema),
				collection_id(_collection_id),
				objects(_objects)
			{
				;
			}

			jarray create_object(row_id_type _class_field_id);

			jarray get_object(row_id_type _object_id)
			{
				auto new_object = objects.get(_object_id);
				return jarray(schema, new_object.parent().class_field_id, new_object.pchild());
			}

			object_id_type get_object_id(row_id_type _object_id)
			{
				auto new_object = objects.get(_object_id);
				return new_object.parent().oid;
			}

			int size()
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
				using value_type = jarray;
				using pointer = jarray*;  // or also value_type*
				using reference = jarray&;  // or also value_type&

				iterator(jcollection* _base, row_id_type _current) :
					base(_base),
					current(_current)
				{

				}

				iterator() : base(nullptr), current(null_row)
				{

				}

				iterator& operator = (const iterator& _src)
				{
					base = _src.base;
					current = _src.current;
					return *this;
				}

				inline jarray operator *()
				{
					return base->get_object(current);
				}

				inline jarray operator->()
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
			using class_store_type = parent_child_table<jclass_header, jclass_field>;
			using class_index_type = sorted_index<object_name, row_id_type>;
			using field_index_type = sorted_index<object_name, row_id_type>;
			using projection_store_type = parent_child_table<row_id_type, projection_element>;
			using filter_store_type = parent_child_table<row_id_type, filter_element>;

			field_store_type fields;
			class_store_type classes;
			class_index_type classes_by_name;
			field_index_type fields_by_name;
			projection_store_type query_projections;
			filter_store_type	  query_filters;

		public:


			class add_class_request {
			public:
				std::string class_name;
				std::string class_description;
				std::vector<row_id_type> field_ids;
			};

			jschema() = default;
			~jschema() = default;

			template <typename B>
			requires (box<B, jfield>
				&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static row_id_type reserve_schema(B *_b, int _num_classes, int _num_fields, int _num_class_fields)
			{
				jschema_map schema_map, *pschema_map;
				schema_map.classes_table_id = null_row;
				schema_map.fields_table_id = null_row;
				schema_map.classes_by_name_id = null_row;
				schema_map.fields_by_name_id = null_row;

				row_id_type rit = _b->pack(schema_map);
				pschema_map = _b->unpack<jschema_map>(rit);
				pschema_map->fields_table_id = field_store_type::reserve_table(_b, _num_fields);
				pschema_map->classes_by_name_id = field_index_type::reserve_sorted_index(_b, _num_classes);
				pschema_map->classes_table_id = class_store_type::reserve_table(_b, _num_classes, _num_class_fields );
				pschema_map->fields_by_name_id = class_index_type::reserve_sorted_index(_b, _num_fields);
				return rit;
			}

			template <typename B>
			requires (box<B, jfield>
				&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static jschema get_schema(B* _b, row_id_type _row)
			{
				jschema schema;
				jschema_map *pschema_map;
				pschema_map = _b->unpack<jschema_map>(_row);
				schema.classes = class_store_type::get_table(_b, pschema_map->classes_table_id);
				schema.fields = field_store_type::get_table(_b, pschema_map->fields_table_id);
				schema.classes_by_name = class_index_type::get_sorted_index(_b, pschema_map->classes_by_name_id);
				schema.fields_by_name = field_index_type::get_sorted_index(_b, pschema_map->fields_by_name_id);
				return schema;
			}

			template <typename B>
				requires (box<B, jfield>
			&& box<B, jclass_header>
				&& box<B, jclass_field>
				&& box<B, object_name>
				)
			static jschema create_schema(B* _b, int _num_classes, int _num_fields, int _num_class_fields, row_id_type& _row)
			{
				_row = reserve_schema(_b, _num_classes, _num_fields, _num_class_fields);
				jschema schema = get_schema(_b, _row);
				return schema;
			}

			void add_standard_fields();

			row_id_type add_field()
			{
				return fields.create(1).start;
			}

			row_id_type add_field(
				row_id_type _field_id,
				jtype _field_type,
				object_name& _name,
				object_description& _description,
				string_properties_type *_string_properties,
				int_properties_type *_int_properties,
				double_properties_type *_double_properties,
				time_properties_type *_time_properties,
				object_properties_type *_object_properties,
				query_properties_type* _query_properties,
				point_properties_type* _point_properties,
				rectangle_properties_type* _rectangle_properties,
				image_properties_type* _image_properties,
				wave_properties_type* _wave_properties,
				midi_properties_type* _midi_properties,
				color_properties_type* _color_properties,
				int64_t _size_bytes)
			{
				if (_field_id == null_row) {
					_field_id = add_field();
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

				jf.size_bytes = _size_bytes;

				fields_by_name.insert_or_assign(jf.name, _field_id);
				return _field_id;
			}

			row_id_type add_string_field(add_string_field_request request)
			{
				return add_field(request.name.field_id, jtype::type_string, request.name.name, request.name.description, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, string_box::get_box_size(request.options.length));
			}

			row_id_type add_time_field(add_time_field_request request)
			{
				return add_field(request.name.field_id, type_datetime, request.name.name, request.name.description, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(time_t));
			}

			row_id_type add_integer_field(add_integer_field_request request)
			{
				switch (request.name.type_id)
				{
					case jtype::type_int8:
						return add_field(request.name.field_id, type_int8, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int8_t));
					case jtype::type_int16:
						return add_field(request.name.field_id, type_int16, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int16_t));
					case jtype::type_int32:
						return add_field(request.name.field_id, type_int32, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int32_t));
					case jtype::type_int64:
						return add_field(request.name.field_id, type_int64, request.name.name, request.name.description, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(int64_t));
					default:
						throw std::invalid_argument("Invalid integer type for field name:" + request.name.name);
				}
			}

			row_id_type add_double_field(add_double_field_request request)
			{
				switch (request.name.type_id)
				{
					case type_float32:
						return add_field(request.name.field_id, type_float32, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(float));
					case type_float64:
						return add_field(request.name.field_id, type_float64, request.name.name, request.name.description, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(double));
					default:
						throw std::invalid_argument("Invalid floating point type for field name:" + request.name.name);
				}
			}

			row_id_type add_object_field(add_object_field_request request)
			{
				auto pcr = classes[ request.options.class_id ];
				auto& p = pcr.parent();
				int64_t sizeb = pcr.parent().class_size_bytes;
				request.options.class_size_bytes = sizeb;
				if (request.options.dim.x == 0) request.options.dim.x = 1;
				if (request.options.dim.y == 0) request.options.dim.y = 1;
				if (request.options.dim.z == 0) request.options.dim.z = 1;
				request.options.total_size_bytes = request.options.dim.x * request.options.dim.y * request.options.dim.z * sizeb ;
				return add_field(request.name.field_id, type_object, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, request.options.total_size_bytes);
			}

			row_id_type add_query_field(add_query_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(collection_id_type));
			}

			const char* invalid_comparison = "Invalid comparison";
			const char* invalid_parameter_field = "Invalid parameter field";
			const char* invalid_target_field = "Invalid target field";
			const char* invalid_projection_field = "Invalid projection field";

			bool prepare_query_field_request(add_named_query_field_request request)
			{
				bool valid = true;

				int filterSize = request.options.filter.size();

				for (int i = 0; i < filterSize; i++)
				{
					auto& filter = request.options.filter[i];

					if (filter.comparison_name == "$eq") {
						filter.comparison = filter_comparison_types::eq;
					}
					else if (filter.comparison_name == "$gte") {
						filter.comparison = filter_comparison_types::gteq;
					}
					else if (filter.comparison_name == "$lte") {
						filter.comparison = filter_comparison_types::lseq;
					}
					else if (filter.comparison_name == "$gt") {
						filter.comparison = filter_comparison_types::gt;
					}
					else if (filter.comparison_name == "$lt") {
						filter.comparison = filter_comparison_types::ls;
					}
					else if (filter.comparison_name == "$inside") {
						filter.comparison = filter_comparison_types::distance;
					}
					else if (filter.comparison_name == "$in") {
						filter.comparison = filter_comparison_types::inlist;
					}
					else if (filter.comparison_name == "$contains") {
						filter.comparison = filter_comparison_types::contains;
					}
					else
					{
						filter.error_message = invalid_comparison;
						valid = false;
					}

					auto piter = fields_by_name[filter.parameter_field_name];
					if (piter != std::end(fields_by_name)) 
					{
						filter.parameter_field_id = piter->second;
					}
					else
					{
						filter.error_message = invalid_parameter_field;
					}

					auto titer = fields_by_name[filter.target_field_name];
					if (titer != std::end(fields_by_name)) 
					{
						filter.target_field_id = titer->second;
					}
					else
					{
						filter.error_message = invalid_target_field;
					}
				}

				int projectionSize = request.options.filter.size();

				for (int i = 0; i < projectionSize; i++)
				{
					auto& projection = request.options.projection[i];

					auto titer = fields_by_name[projection.field_name];
					if (titer != std::end(fields_by_name))
					{
						projection.field_id = titer->second;
					}
					else
					{
						projection.error_message = invalid_target_field;
						valid = false;
						continue;
					}
				}

				return valid;
			}

			row_id_type add_query_field(add_named_query_field_request request)
			{
				query_properties_type options;

				bool prepared = prepare_query_field_request(request);
				if (!prepared) {
					return null_row;
				}

				int filterSize = request.options.filter.size();
				auto filterHolder = query_filters.create(filterSize);

				options.filters = filterHolder.row_id();

				for (int fi = 0; fi < filterHolder.size(); fi++)
				{
					auto& fe_src = request.options.filter[fi];
					filter_element fe;
					fe.comparison = fe_src.comparison;
					fe.parameter_field_id = fe_src.parameter_field_id;
					fe.target_field_id = fe_src.target_field_id;
					fe.distance_threshold = fe_src.distance_threshold;
					filterHolder.child(fi) = fe;
				}

				int projectionSize = request.options.filter.size();
				auto projectionHolder = query_projections.create(projectionSize);

				options.projections = projectionHolder.row_id();

				for (int pi = 0; pi < projectionHolder.size(); pi++)
				{
					auto& pe_src = request.options.projection[pi];
					projection_element pe;
					pe.field_id = pe_src.field_id;
					projectionHolder.child(pi) = pe;
				}

				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, &options, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(collection_id_type));
			}

			row_id_type add_point_field(add_point_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, nullptr, sizeof(point));
			}

			row_id_type add_rectangle_field(add_rectangle_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, nullptr, sizeof(rectangle));
			}

			row_id_type add_image_field(add_image_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, nullptr, sizeof(image_instance));
			}

			row_id_type add_wave_field(add_wave_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, nullptr, sizeof(wave_instance));
			}

			row_id_type add_midi_field(add_midi_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, nullptr, sizeof(midi_instance));
			}

			row_id_type add_color_field(add_color_field_request request)
			{
				return add_field(request.name.field_id, type_query, request.name.name, request.name.description, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &request.options, sizeof(color));
			}

			row_id_type add_class( add_class_request request )
			{
				auto sz = request.field_ids.size();
				auto pcr = classes.create(sz);
				auto& p = pcr.parent();

				p.class_id = pcr.row_id();
				p.name = request.class_name;
				p.description = request.class_description;
				p.class_size_bytes = 0;

				for (int i = 0; i < pcr.size(); i++)
				{
					auto fid = request.field_ids[i];
					auto& field = fields[fid];
					auto& ref = pcr.child(i);
					ref.field_id = fid;
					ref.offset = p.class_size_bytes;
					p.class_size_bytes += field.size_bytes;
				}

				return p.class_id;
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

			template <typename B>
			requires (box<B, jcollection_map>)
			row_id_type reserve_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, int* _class_field_ids)
			{
				if (!_class_field_ids)
				{
					throw std::invalid_argument("cannot create collection with 0 class size");
				}

				int max_size = 0;
				while (*_class_field_ids != null_row)
				{
					auto myclassfield = get_field(*_class_field_ids);
					if (myclassfield.size_bytes > max_size) {
						max_size = myclassfield.size_bytes;
					}
					_class_field_ids++;
				}

				if (!max_size)
				{
					throw std::invalid_argument("cannot create collection with 0 class size");
				}

				jcollection_map jcm;
				jcm.collection_id = _collection_id;
				jcm.table_id = parent_child_table<jobject_header, char>::reserve_table(_b, _number_of_objects, max_size * _number_of_objects);
				row_id_type jcm_row = _b->pack(jcm);
				return jcm_row;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection get_collection(B* _b, row_id_type _location)
			{
				jcollection_map *jcm;
				jcm = _b->unpack<jcollection_map>(_location);
				auto obj = parent_child_table<jobject_header, char>::get_table(_b, jcm->table_id);
				jcollection collection(this, jcm->collection_id, obj);
				return collection;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection create_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, row_id_type* _class_field_ids)
			{
				auto reserved_id = reserve_collection(_b, _collection_id, _number_of_objects, _class_field_ids);
				jcollection tmp = get_collection(_b, reserved_id);
				return tmp;
			}

			template <typename B>
			requires (box<B, jcollection_map>)
			jcollection create_collection(B* _b, collection_id_type _collection_id, int _number_of_objects, row_id_type _class_field_id)
			{
				row_id_type class_field_ids[2] = { _class_field_id, null_row };
				row_id_type reserved_id = reserve_collection(_b, _collection_id, _number_of_objects, class_field_ids);
				jcollection tmp = get_collection(_b, reserved_id);
				return tmp;
			}

		};

		class jarray;

		bool schema_tests();
		bool collection_tests();
		bool array_tests();

	}
}
