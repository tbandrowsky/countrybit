
#include "jobject.h"
#include "combaseapi.h"

namespace countrybit
{
	namespace database
	{

		int compare(const dimensions_type& a, const dimensions_type& b)
		{
			int t = a.z - b.z;
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

		jarray jcollection::create_object(row_id_type _class_id)
		{
			auto myclass = schema->get_class(_class_id);

			object_name composed_class_field_name;
			dimensions_type d{ 1, 1, 1 };
			schema->get_class_field_name(composed_class_field_name, myclass.parent().name, d);
			auto find_field_id = schema->find_field(composed_class_field_name);
			if (find_field_id == null_row) 
			{
				put_object_field_request porf;
				porf.name.name = composed_class_field_name;
				porf.options.class_id = _class_id;
				porf.options.class_name = myclass.parent().name;
				porf.options.class_size_bytes = myclass.parent().class_size_bytes;
				porf.options.dim = d;
				find_field_id = schema->put_object_field(porf);
			}
			auto bytes_to_allocate = myclass.parent().class_size_bytes;
			auto new_object = objects.create(bytes_to_allocate);
			new_object.parent().oid.collection_id = collection_id;
			new_object.parent().oid.row_id = new_object.row_id();
			new_object.parent().class_field_id = find_field_id;	
			jarray ja(schema, find_field_id, new_object.pchild());

			for (auto jai : ja) 
			{
				jai.construct();
			}

			return ja;
		}

		jslice::jslice() : schema(nullptr), class_id(null_row), bytes(nullptr)
		{
			;
		}

		jslice::jslice(jschema* _schema, row_id_type _class_id, char* _bytes, dimensions_type _dim) : schema(_schema), class_id(_class_id), bytes(_bytes), dim(_dim)
		{
			the_class = schema->get_class(_class_id);
		}

		size_t jslice::get_offset(jtype field_type_id, int field_idx)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.child(field_idx);
#if _DEBUG
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != field_type_id) 
			{
				throw std::invalid_argument("Invalid field type " + std::to_string(field_type_id) + " for field idx " + std::to_string(field_idx));
			}
#endif
			return jcf.offset;
		}

		dimensions_type jslice::get_dim() 
		{
			return dim; 
		}

		void jslice::construct()
		{
			for (int i = 0; i < the_class.size(); i++)
			{
				jclass_field& jcf = the_class.child(i);
				jfield jf = schema->get_field(jcf.field_id);
				int offset = jcf.offset;
				char* c = &bytes[offset];
				switch (jf.type_id) 
				{
				case jtype::type_null:
					break;
				case jtype::type_int8:
					{
						boxed<__int8> b(c);
						b = 0;
					}
					break;
				case jtype::type_int16:
					{
						boxed<__int16> b(c);
						b = 0;
					}
					break;
				case jtype::type_int32:
					{
						boxed<__int32> b(c);
						b = 0;
					}
					break;
				case jtype::type_int64:
					{
						boxed<__int64> b(c);
						b = 0;
					}
					break;
				case jtype::type_float32:
					{
						boxed<float> b(c);
						b = 0.0;
					}
					break;
				case jtype::type_float64:
					{
						boxed<float> b(c);
						b = 0.0;
					}
					break;
				case jtype::type_datetime:
					{
						boxed<time_t> b(c);
						b = 0.0;
					}
					break;
				case jtype::type_object:
					{
						jarray ja(schema, jcf.field_id, c);

						for (auto jai : ja)
						{
							jai.construct();
						}
					}
					break;
				case jtype::type_list:
					{
						jlist jax(schema, jcf.field_id, c, true);
					}
					break;
				case jtype::type_object_id:
					break;
				case jtype::type_string:
					{
						string_box::create(c, jf.string_properties.length);
					}
					break;
				case jtype::type_point:
					{
						boxed<point> b(c);
						b = point { };
					}
					break;
				case jtype::type_rectangle:
					{
						boxed<rectangle> b(c);
						b = rectangle {};
					}
					break;
				case jtype::type_color:
					{
						boxed<color> b(c);
						b = color{};
					}
					break;
				case jtype::type_image:
					{
						boxed<image_instance> b(c);
						b = image_instance {};
					}
					break;
				case jtype::type_wave:
					{
						boxed<wave_instance> b(c);
						b = wave_instance {};
					}
					break;
				case jtype::type_midi:
					{
						boxed<midi_instance> b(c);
						b = midi_instance {};
					}
					break;
				case jtype::type_query:
					{
						boxed<query_instance> b(c);
						b = query_instance{};
					}
					break;
				case jtype::type_sql:
					{
						boxed<sql_remote_instance> b(c);
						b = sql_remote_instance{};
					}
					break;
				case jtype::type_file:
					{
						boxed<file_remote_instance> b(c);
						b = file_remote_instance{};
					}
					break;
				case jtype::type_http:
					{
						boxed<http_remote_instance> b(c);
						b = http_remote_instance{};
					}
					break;
				}
			}
		}

		jfield& jslice::get_field(int field_idx)
		{
			jclass_field& jcf = the_class.child(field_idx);
			jfield &jf = schema->get_field(jcf.field_id);
			return jf;
		}

		int8_box jslice::get_int8(int field_idx)
		{
			return get_boxed<int8_box>(jtype::type_int8, field_idx);
		}

		int16_box jslice::get_int16(int field_idx)
		{
			return get_boxed<int16_box>(jtype::type_int16, field_idx);
		}

		int32_box jslice::get_int32(int field_idx)
		{
			return jslice::get_boxed<int32_box>(jtype::type_int32, field_idx);
		}

		int64_box jslice::get_int64(int field_idx)
		{
			return jslice::get_boxed<int64_box>(jtype::type_int64, field_idx);
		}

		float_box jslice::get_float(int field_idx)
		{
			return jslice::get_boxed<float_box>(jtype::type_float32, field_idx);
		}

		double_box jslice::get_double(int field_idx)
		{
			return jslice::get_boxed<double_box>(jtype::type_float64, field_idx);
		}

		time_box jslice::get_time(int field_idx)
		{
			return get_boxed<time_box>(jtype::type_datetime, field_idx);
		}

		point_box jslice::get_point(int field_idx)
		{
			return get_boxed<point_box>(jtype::type_datetime, field_idx);
		}

		rectangle_box jslice::get_rectangle(int field_idx)
		{
			return get_boxed<rectangle_box>(jtype::type_datetime, field_idx);
		}

		image_box jslice::get_image(int field_idx)
		{
			return get_boxed<image_box>(jtype::type_datetime, field_idx);
		}

		wave_box jslice::get_wave(int field_idx)
		{
			return get_boxed<wave_box>(jtype::type_datetime, field_idx);
		}

		midi_box jslice::get_midi(int field_idx)
		{
			return get_boxed<midi_box>(jtype::type_datetime, field_idx);
		}

		color_box jslice::get_color(int field_idx)
		{
			return get_boxed<color_box>(jtype::type_datetime, field_idx);
		}

		query_box jslice::get_query(int field_idx)
		{
			return get_boxed_ex<query_box>(jtype::type_query, field_idx);
		}

		sql_remote_box jslice::get_sql_remote(int field_idx)
		{
			return get_boxed_ex<sql_remote_box>(jtype::type_sql, field_idx);
		}

		http_remote_box jslice::get_http_remote(int field_idx)
		{
			return get_boxed_ex<http_remote_box>(jtype::type_http, field_idx);
		}

		file_remote_box jslice::get_file_remote(int field_idx)
		{
			return get_boxed_ex<file_remote_box>(jtype::type_file, field_idx);
		}

		string_box jslice::get_string(int field_idx)
		{
			size_t offset = get_offset(jtype::type_string, field_idx);
			char *b = &bytes[offset];
			auto temp = string_box::get(b);
			return temp;
		}

		jarray jslice::get_object(int field_idx)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.child(field_idx);
#if _DEBUG
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != jtype::type_object) {
				throw std::invalid_argument("Invalid field type " + std::to_string(jtype::type_object) + " for field idx " + std::to_string(field_idx));
			}
#endif
			char *b = &bytes[jcf.offset];
			jarray jerry(schema, jcf.field_id, b);
			return jerry;
		}

		jlist jslice::get_list(int field_idx)
		{
#if _DEBUG
			if (schema == nullptr || class_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
#endif
			jclass_field& jcf = the_class.child(field_idx);
#if _DEBUG
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != jtype::type_object) {
				throw std::invalid_argument("Invalid field type " + std::to_string(jtype::type_object) + " for field idx " + std::to_string(field_idx));
			}
#endif
			char* b = &bytes[jcf.offset];
			jlist jerry(schema, jcf.field_id, b);
			return jerry;
		}

		collection_id_box jslice::get_collection_id(int field_idx)
		{
			return jslice::get_boxed<collection_id_box>(jtype::type_collection_id, field_idx);
		}

		object_id_box jslice::get_object_id(int field_idx)
		{
			return jslice::get_boxed<object_id_box>(jtype::type_object_id, field_idx);
		}

		int jslice::size()
		{
			auto the_class = schema->get_class(class_id);
			return the_class.size();
		}

		jarray::jarray() : schema(nullptr), class_field_id(null_row), bytes(nullptr)
		{
			;
		}

		jarray::jarray(jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init) : schema(_schema), class_field_id(_class_field_id), bytes(_bytes)
		{

		}

		jarray::jarray(dynamic_box& _dest, jarray& _src)
		{
			schema = _src.schema;
			class_field_id = _src.class_field_id;
			auto fld = schema->get_field(class_field_id);
			_dest.init(fld.size_bytes);
			bytes = _dest.allocate<char>(fld.size_bytes);
			std::copy(_src.bytes, _src.bytes + fld.size_bytes, bytes);
		}

		dimensions_type jarray::dimensions()
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim;
		}

		jslice jarray::get_slice(int x, int y, int z)
		{
			dimensions_type dims;
			dims.x = x;
			dims.y = y;
			dims.z = z;
			return get_slice(dims);
		}

		jslice jarray::get_slice(dimensions_type pos)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((pos.x >= dim.x) ||
				(pos.y >= dim.y) ||
				(pos.z >= dim.z)) {
					return jslice(schema, field.object_properties.class_id, nullptr, dim);
			}
			char* b = &bytes[ ((pos.z * dim.y * dim.x) + (pos.y * dim.x) + pos.x ) * field.object_properties.class_size_bytes ];
			jslice slice(schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		uint64_t jarray::get_size_bytes()
		{
			jfield& field = schema->get_field(class_field_id);
			return field.size_bytes;
		}

		jarray_container::jarray_container()
		{
			;
		}

		jarray_container::jarray_container(collection_id_type& _collection, jarray& _objects)
		{
			set(_collection, _objects);
		}

		void jarray_container::set(collection_id_type& _collection, jarray& _objects)
		{
			collection = _collection;
			objects = jarray(data, _objects);
		}

		jarray& jarray_container::get()
		{
			return objects;
		}

		//

		jlist::jlist() : schema(nullptr), class_field_id(null_row)
		{
			;
		}

		jlist::jlist(jschema* _schema, row_id_type _class_field_id, char* _bytes, bool _init) 
			: schema(_schema), class_field_id(_class_field_id)
		{
			auto& field_def = schema->get_field(_class_field_id);
			jclass model_class_def = schema->get_class(field_def.object_properties.class_id);
			auto box_size = field_def.size_bytes;

			data.instance = nullptr;

			if (_init)
			{
				model_box->init(box_size);
				data.instance = model_box->allocate<jlist_instance>(1);
				data.instance->selection_offset = array_box<row_id_type>::create(model_box, field_def.object_properties.dim.x);
				data.instance->slice_offset = model_box->reserve(box_size);
				data.instance->allocated = 0;
			}
			else
			{
				data.instance = model_box->unpack<jlist_instance>(0);
			}

			data.list_bytes = model_box->unpack<char>(data.instance->slice_offset);
			data.selections = array_box<row_id_type>::get(model_box, data.instance->selection_offset);
		}

		jlist::jlist(dynamic_box& _dest, jlist& _src)
			: schema(_src.schema), class_field_id(_src.class_field_id)
		{
			auto& field_def = schema->get_field(_src.class_field_id);
			auto box_size = field_def.size_bytes;

			model_box = _dest.get_box();
			_dest.init(box_size);
			std::copy((char*)_src.data.instance, (char*)_src.data.instance + box_size, (char*)data.instance);
			data.instance = model_box->unpack<jlist_instance>(0);
			data.list_bytes = model_box->unpack<char>(data.instance->slice_offset);
			data.selections = array_box<row_id_type>::get(model_box, data.instance->selection_offset);
		}

		uint32_t jlist::capacity()
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim.x;
		}

		uint32_t jlist::size()
		{
			return data.instance->allocated;
		}

		jslice jlist::get_slice(int idx)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= data.instance->allocated) || (idx < 0)) {
				return jslice(schema, field.object_properties.class_id, nullptr, dim);
			}
			dimensions_type pos = { idx, 0, 0 };
			char* b = &data.list_bytes[idx * field.object_properties.class_size_bytes];
			jslice slice(schema, field.object_properties.class_id, b, pos);
			return slice;
		}

		bool jlist::erase_slice(int idx)
		{
			jfield& field = schema->get_field(class_field_id);
			dimensions_type dim = field.object_properties.dim;
			if ((idx >= dim.x) || (idx < 0)) 
			{
				return false;
			}
			else if (data.instance->allocated <= 0)
			{
				data.instance->allocated = 0;
				return false;
			}
			else if (idx >= data.instance->allocated)
			{
				data.instance->allocated--;
			}
			else 
			{
				auto class_size = field.object_properties.class_size_bytes;
				char* b1 = &data.list_bytes[idx * class_size];
				char* b2 = &data.list_bytes[(idx + 1) * class_size];
				int32_t length_objects = data.instance->allocated - idx;
				int32_t length_bytes = length_objects * class_size;
				std::copy(b2, b2 + length_bytes, b1);
				data.instance->allocated--;
			}
			return true;
		}

		jslice jlist::append_slice()
		{
			if (data.instance->allocated < capacity()) {
				auto index = data.instance->allocated;
				data.instance->allocated++;
				jslice new_slice = get_slice(index);
				new_slice.construct();
				return new_slice;
			}
			return get_slice(-1);
		}

		bool jlist::select_slice(int idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			data.selections[idx] = 1;
			return true;
		}

		bool jlist::deselect_slice(int idx)
		{
			if (idx < 0 || idx >= data.instance->allocated)
				return false;
			data.selections[idx] = 0;
			return true;
		}

		void jlist::deselect_all()
		{
			for (int i = 0; i < data.instance->allocated; i++) 
			{
				data.selections[i] = 0;
			}
		}

		void jlist::select_all()
		{
			for (int i = 0; i < data.instance->allocated; i++)
			{
				data.selections[i] = 1;
			}
		}

		char* jlist::get_bytes()
		{
			return (char *)model_box;
		}

		uint64_t jlist::get_size_bytes()
		{
			jfield& field = schema->get_field(class_field_id);
			return field.size_bytes;
		}

		// - jmodel

		jmodel::jmodel() : 
			schema(nullptr), 
			class_id(null_row), 
			model_box(nullptr)
		{
			;
		}

		jmodel::jmodel(jschema* _schema, row_id_type _class_id, char* _bytes, bool _init) : 
			schema(_schema), 
			class_id(_class_id),
			model_box( (serialized_box*) _bytes )
		{

			jclass model_class_def = schema->get_class(_class_id);
			jclass_header *model_class = model_class_def.pparent();
			auto box_size = model_class->class_size_bytes;
			auto num_actors = model_class->number_actors;

			data.instance = nullptr;

			if (_init) 
			{
				model_box->init(box_size);
				data.instance = model_box->allocate<jmodel_instance>(1);
				data.instance->selection_offset = array_box<row_id_type>::create(model_box, num_actors);
				data.instance->slice_offset = model_box->reserve(box_size);
				data.instance->actor_id = 0;
			}
			else 
			{
				data.instance = model_box->unpack<jmodel_instance>(0);
			}

			data.model_bytes = model_box->unpack<char>(data.instance->slice_offset);
			data.selections = array_box<row_id_type>::get(model_box, data.instance->selection_offset);
		}

		jmodel::jmodel(dynamic_box& _dest, jmodel& _src) 
		{
			jclass model_class_def = _src.schema->get_class(_src.class_id);
			jclass_header* model_class = model_class_def.pparent();
			auto box_size = model_class->class_size_bytes;
			auto num_actors = model_class->number_actors;

			model_box = _dest.get_box();
			model_box->init(box_size);
			_dest.copy(_src.model_box);
			data.instance = model_box->unpack<jmodel_instance>(0);
			data.model_bytes = model_box->unpack<char>(data.instance->slice_offset);
			data.selections = array_box<row_id_type>::get(model_box, data.instance->selection_offset);
		}

		size_t jmodel::estimate_size(put_model_request& _request, uint32_t base_class_bytes)
		{
			size_t estimated_size = base_class_bytes;
			estimated_size += sizeof(jmodel_instance);
			estimated_size += 32;
			return estimated_size;
		}

		size_t jmodel::size()
		{
			return data.number_of_actors;
		}

		row_id_type	jmodel::get_field_index(row_id_type state_field_id)
		{
			jclass model_class_def = schema->get_class(class_id);
			for (int i = 0; i < model_class_def.size(); i++)
			{
				auto& fld = model_class_def.child(i);
				if (fld.field_id == state_field_id && fld.model_state) {
					return i;
				}
			}
			return null_row;
		}

		jslice jmodel::get_model()
		{
			jslice slice(schema, class_id, data.model_bytes, { 0, 0, 0 } );
			return slice;
		}

		jlist jmodel::get_state(row_id_type state_field_id)
		{
			auto slice = get_model();
			auto field_idx = get_field_index(state_field_id);
			slice.get_list(field_idx);
		}

		row_id_type jmodel::create_actor()
		{
			data.instance->actor_id++;
			return data.instance->actor_id;
		}

		jslice jmodel::set_actor(row_id_type state_field_id, row_id_type actor_id)
		{
			;
		}

		jslice jmodel::get_actor(row_id_type state_field_id, row_id_type actor_id)
		{
			jlist state = get_state(state_field_id);

		}

		void jmodel::delete_actor(row_id_type state_field_id, row_id_type actor_id)
		{
			;
		}

		void jmodel::move_actor(row_id_type source_state_field_id, row_id_type dest_state_field_id, row_id_type actor_id)
		{
			;
		}

		void jmodel::delete_actor(row_id_type state_field_id)
		{
			;
		}

		char* jmodel::get_bytes()
		{
			return (char*)model_box;
		}

		uint64_t jmodel::get_size_bytes()
		{
			jclass model_class_def = schema->get_class(class_id);
			return model_class_def.parent().class_size_bytes;
		}

		void jschema::add_standard_fields() 
		{
			put_string_field_request string_fields[33] = {
				{ { field_full_name, jtype::type_string , "fullName", "Full Name" }, { 75, "", "" } },
				{ { field_first_name, jtype::type_string , "firstName", "First Name" }, { 50, "", "" } },
				{ { field_last_name, jtype::type_string , "lastName", "Last Name" }, { 50, "", "" } },
				{ { field_middle_name, jtype::type_string , "middleName", "Middle Name" }, { 50, "", "" } },
				{ { field_ssn, jtype::type_string , "ssn", "SSN" }, { 10, "", "" }},
				{ { field_email, jtype::type_string, "email", "eEmail" }, { 200, "", ""  }},
				{ { field_title, jtype::type_string, "title", "Title" }, { 200, "", "" } },
				{ { field_street, jtype::type_string, "street", "Street" },{  200, "", "" } },
				{ { field_substreet, jtype::type_string, "suiteapt", "Suite/Apt" }, { 100, "", ""  }},
				{ { field_city, jtype::type_string, "city", "City" }, { 100, "", "" } },
				{ { field_state, jtype::type_string, "state", "State" }, { 100, "", "" } },
				{ { field_postal, jtype::type_string, "postal", "Postal Code" }, { 50, "", ""  }},
				{ { field_country_name, jtype::type_string, "countryName", "Country Name" }, { 50, "", "" } },
				{ { field_country_code, jtype::type_string, "countryCode", "Country Code" }, { 3, "", ""  }},
				{ { field_institution_name, jtype::type_string, "institutionName", "Institution Name" }, { 100, "", "" } },
				{ { field_longname, jtype::type_string, "longName", "Long Name" }, { 200, "", ""  }},
				{ { field_shortname, jtype::type_string, "shortName", "Short Name" },{  50, "", ""  }},
				{ { field_unit, jtype::type_string, "unit", "Unit" }, { 10, "", "" } },
				{ { field_code_symbol, jtype::type_string, "symbol", "Symbol" }, { 10, "", "" } },
				{ { field_code_operator, jtype::type_string, "operator", "Operator" }, { 10, "", ""  }},
				{ { field_windows_path, jtype::type_string, "windowsPath", "Windows Path" }, { 512, "", ""  }},
				{ { field_linux_path, jtype::type_string, "linuxPath", "Linux Path" }, { 512, "", "" } },
				{ { field_url, jtype::type_string, "url", "Url" }, { 512, "", "" } },
				{ { field_user_name, jtype::type_string, "userName", "User Name" }, { 100, "", ""  }},
				{ { field_password, jtype::type_string, "passWord", "Password" }, { 100, "", ""  }},
				{ { field_document_title, jtype::type_string, "docTitle", "Document Title" }, { 200, "", "" } },
				{ { field_section_title, jtype::type_string, "sectionTitle", "Section Title" }, { 200, "", "" } },
				{ { field_block_title, jtype::type_string, "blockTitle", "Block Title" }, { 200, "", "" } },
				{ { field_caption, jtype::type_string, "caption", "Caption" }, { 200, "", "" } },
				{ { field_paragraph, jtype::type_string, "paragraph", "Paragraph" }, { 4000, "", "" } },
				{ { field_mime_type, jtype::type_string, "mimeType", "MimeType" }, { 100, "", "" } },
				{ { field_base64_block, jtype::type_string, "base64", "Base64" }, { 100, "", "" } },
				{ { field_file_name, jtype::type_string, "fileName", "fileName" }, { 512, "", "" } }
			};

			put_time_field_request time_fields[2] = {
				{ { field_birthday, jtype::type_datetime, "birthday", "Birthday" }, 0, INT64_MAX },
				{ { field_scheduled, jtype::type_datetime, "scheduled", "Scheduled" }, 0, INT64_MAX },
			};

			put_integer_field_request int_fields[1] = {
				{ { field_count, jtype::type_int64, "count", "Count" }, 0, INT64_MAX },
			};

			put_double_field_request double_fields[22] = {
				{ { field_quantity, jtype::type_float64, "quantity", "Quantity" }, -1E40, 1E40 },
				{ { field_latitude, jtype::type_float64, "latitude", "Latitude" }, -90, 90 },
				{ { field_longitude, jtype::type_float64, "longitude", "Longitude" }, -180, 180 },
				{ { field_meters, jtype::type_float64, "meters", "Meters" }, -1E40, 1E40 },
				{ { field_feet, jtype::type_float64, "feet", "Feet" }, -1E40, 1E40 },
				{ { field_kilograms, jtype::type_float64, "kilograms", "Kilograms" }, -1E40, 1E40 },
				{ { field_pounds, jtype::type_float64, "pounds", "Pounds" }, -1E40, 1E40 },
				{ { field_seconds, jtype::type_float64, "seconds", "Seconds" }, -1E40, 1E40 },
				{ { field_minutes, jtype::type_float64, "minutes", "Minutes" }, -1E40, 1E40 },
				{ { field_hours, jtype::type_float64, "hours", "Hours" }, -1E40, 1E40 },
				{ { field_amperes, jtype::type_float64, "amperes", "Amperes" }, -1E40, 1E40 },
				{ { field_kelvin, jtype::type_float64, "kelvin", "Kelvin" }, -1E40, 1E40 },
				{ { field_mole, jtype::type_float64, "moles", "Moles" }, -1E40, 1E40 },
				{ { field_height, jtype::type_float32, "height", "Height" }, 0, 100000 },
				{ { field_width, jtype::type_float32, "width", "Width" }, 0, 100000 },
				{ { field_x, jtype::type_float32, "x", "X" }, -100000, 100000 },
				{ { field_y, jtype::type_float32, "y", "Y" }, -100000, 100000 },
				{ { field_z, jtype::type_float32, "z", "Z" }, -100000, 100000 },
				{ { field_red, jtype::type_float32, "red", "red" }, 0, 1 },
				{ { field_green, jtype::type_float32, "green", "green" }, 0, 1 },
				{ { field_blue, jtype::type_float32, "blue", "blue" }, 0, 1 },
				{ { field_alpha, jtype::type_float32, "alpha", "alpha" }, 0, 1 }
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
			dynamic_box box;
			box.init(1 << 21);

			jschema schema;
			row_id_type schema_id;

			schema = jschema::create_schema(&box, 10, 200, 500, 20, 20, 20, 20, schema_id);
			schema.add_standard_fields();

			row_id_type quantity_field_id = schema.find_field("quantity");
			row_id_type last_name_field_id = schema.find_field("lastName");
			row_id_type count_field_id = schema.find_field("count");

			if (quantity_field_id == null_row) {
				std::cout << __LINE__ << ":find row failed:" << std::endl;
				return false;
			}

			if (quantity_field_id != field_quantity) {
				std::cout << __LINE__ << ":field placement failed:" <<  std::endl;
				return false;
			}

			if (last_name_field_id == null_row) {
				std::cout << __LINE__ << ":find row failed" << std::endl;
				return false;
			}

			if (last_name_field_id != field_last_name) {
				std::cout << __LINE__ << ":field placement failed" << std::endl;
				return false;
			}

			if (count_field_id == null_row) {
				std::cout << __LINE__ << ":find row failed" << std::endl;
				return false;
			}

			if (count_field_id != field_count) {
				std::cout << __LINE__ << ":field placement failed:" << std::endl;
				return false;
			}

			row_id_type failed_field_id = schema.find_field("badFieldName");

			if (failed_field_id != null_row) {
				std::cout << __LINE__ << ":find row failed" << std::endl;
				return false;
			}

			countrybit::database::put_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.member_fields = { field_last_name, field_first_name, field_birthday, field_title, field_count, field_quantity };
			row_id_type person_class_id = schema.put_class(person);

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
				auto& fldref = person_class.child(i);
				auto& fld = schema.get_field(fldref.field_id);
//				std::cout << fld.name << " " << fld.description << " " << fldref.offset << " " << fld.size_bytes << std::endl;
				if (offset_start && offset_start != fldref.offset) {
					std::cout << __LINE__ << ":class alignment failed" << std::endl;

				}
				offset_start += fld.size_bytes;
			}

			countrybit::database::put_class_request company;
			company.class_name = "company";
			company.class_description = "a company is a collection of people";
			company.member_fields = { field_institution_name, member_field(person_class_id, dimensions_type { 10, 1, 1 })};
			row_id_type company_class_id = schema.put_class(company);

			if (company_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			return true;
		}

		bool collection_tests()
		{
			dynamic_box box;
			box.init(1 << 21);

			jschema schema;
			row_id_type schema_id;

			schema = jschema::create_schema( &box, 10, 200, 500, 20, 20, 20, 20, schema_id);
			schema.add_standard_fields();

			countrybit::database::put_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.member_fields = { field_last_name, field_first_name, field_birthday, field_count, field_quantity };
			row_id_type person_class_id = schema.put_class(person);

			if (person_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			collection_id_type colid;
			init_collection_id(colid);
			
			jcollection people = schema.create_collection(&box, colid, 50, person_class_id);

			jarray pa;

			int birthdaystart = 1941;
			int countstart = 12;
			double quantitystart = 10.22;
			int increment = 5;
			
			pa = people.create_object(person_class_id);
			auto sl = pa.get_slice(0);
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

			pa = people.create_object(person_class_id);
			sl = pa.get_slice(0);
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

			pa = people.create_object(person_class_id);
			sl = pa.get_slice(0);
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

			pa = people.create_object(person_class_id);
			sl = pa.get_slice(0);
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

			pa = people.create_object(person_class_id);
			sl = pa.get_slice(0);
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

			for (auto pers : people)
			{
				sl = pers.get_slice(0);
				last_name = sl.get_string(0);
				if (!last_name.starts_with("last")) {
					std::cout << __LINE__ << ":last name failed" << std::endl;
					return false;
				}
				first_name = sl.get_string(1);
				if (!first_name.starts_with("first")) {
					std::cout << __LINE__ << ":first name failed" << std::endl;
					return false;
				}
				birthday = sl.get_time(2);
				count = sl.get_int64(3);
				qty = sl.get_double(4);

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

		bool array_tests()
		{
			dynamic_box box;
			box.init(1 << 21);

			jschema schema;
			row_id_type schema_id;

			schema = jschema::create_schema(&box, 10, 200, 500, 20, 20, 20, 20, schema_id);
			schema.add_standard_fields();

			countrybit::database::put_class_request sprite_frame_request;

			sprite_frame_request.class_name = "spriteframe";
			sprite_frame_request.class_description = "sprite frame";
			sprite_frame_request.member_fields = { field_shortname, field_x, field_y, field_width, field_height };
			row_id_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

			if (sprite_frame_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			countrybit::database::put_object_field_request of;
			of.name.field_id = null_row;
			of.options.class_id = sprite_frame_class_id;
			of.options.dim = { 10, 10, 1 };
			of.name.name = "spriteframe20";
			of.name.description = "spriteframe20";

			row_id_type sprite_frame_field_id = schema.put_object_field(of);

			if (sprite_frame_field_id == null_row) {
				std::cout << __LINE__ << ":object field create failed" << std::endl;
				return false;
			}

			countrybit::database::put_class_request sprite_class_request;
			sprite_class_request.class_name = "sprite";
			sprite_class_request.class_description = "sprite";
			sprite_class_request.member_fields = { field_shortname, field_width, field_height, sprite_frame_field_id };
			row_id_type sprite_class_id = schema.put_class(sprite_class_request);

			if (sprite_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			collection_id_type colid;

			init_collection_id(colid);

			jcollection sprites = schema.create_collection(&box, colid, 50, sprite_class_id);

			for (int i = 0; i < 10; i++) {
				auto new_object = sprites.create_object(sprite_class_id);
				auto slice = new_object.get_slice(0);

				auto image_name = slice.get_string(0);
				auto frame_width = slice.get_float(1);
				auto frame_height = slice.get_float(2);

				auto frame_array = slice.get_object(3);

				image_name = std::format("{} #{}", "image", i);
				frame_width = 1000;
				frame_height = 1000;

				for (auto frame : frame_array) 
				{
					auto dim = frame.get_dim();
					auto frame_name = frame.get_string(0);
					auto x = frame.get_float(1);
					auto y = frame.get_float(2);
					auto width = frame.get_float(3);
					auto height = frame.get_float(4);
					frame_name = std::format("{} #{}", "frame", dim.x);
					x = dim.x * 100.0;
					y = dim.y * 100.0;
					width = 100.0;
					height = 100.0;
				}
			}

			int scount = 0;

			for (auto item : sprites)
			{
				auto slice = item.get_slice(0);

				auto frame_width = slice.get_float(1);
				auto frame_height = slice.get_float(2);

				if (frame_width != 1000 || frame_height != 1000) {

					std::cout << __LINE__ << ":array failed" << std::endl;
					return false;
				}

				auto frames = slice.get_object(3);

				int fcount = 0;
				for (auto frame : frames)
				{
					if (frame_width != 1000 || frame_height != 1000) {

						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}
//					std::cout << std::format("{} {}x{} - {}x{}", frame.get_string(0).value(), frame.get_float(1).value(), frame.get_float(2).value(), frame.get_float(3).value(), frame.get_float(4).value()) << std::endl;
					auto dim = frame.get_dim();
					auto x = frame.get_float(1);
					auto y = frame.get_float(2);
					auto width = frame.get_float(3);
					auto height = frame.get_float(4);
					if (x != dim.x * 100.0) {
						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					};
					if (y != dim.y * 100.0) {
						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}
					if (width != 100.0) {
						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;

					}
					if (height != 100.0) {
						std::cout << __LINE__ << ":array failed" << std::endl;
						return false;
					}
				}
			}

			return true;
		}

	}
}
