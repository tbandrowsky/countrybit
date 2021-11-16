
#include "jobject.h"
#include "combaseapi.h"

namespace countrybit
{
	namespace database
	{

		int compare(const dimensions_type& a, const dimensions_type& b)
		{
			if (a.z != b.z) return a.z - b.z;
			if (a.y != b.y) return a.y - b.y;
			if (a.x != b.x) return a.x - b.x;
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

		jarray jcollection::create_object(row_id_type _class_field_id)
		{
			auto myclassfield = schema->get_field(_class_field_id);
			auto myclass = schema->get_class(myclassfield.object_properties.class_id);
			auto bytes_to_allocate = myclass.parent().class_size_bytes;
			auto new_object = objects.create(bytes_to_allocate);
			new_object.parent().oid.collection_id = collection_id;
			new_object.parent().oid.row_id = new_object.row_id();
			new_object.parent().class_field_id = _class_field_id;
			jarray ja(schema, _class_field_id, new_object.pchild());

			return ja;
		}

		size_t jslice::get_offset(jtype field_type_id, int field_idx)
		{
			if (schema == nullptr || class_field_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}
			auto the_class = schema->get_class(class_field_id);
			jclass_field& jcf = the_class.child(field_idx);
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != field_type_id) 
			{
				throw std::invalid_argument("Invalid field type " + std::to_string(field_type_id) + " for field idx " + std::to_string(field_idx));
			}
			return jcf.offset;
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
			return get_boxed<double_box>(jtype::type_float64, field_idx);
		}

		time_box jslice::get_time(int field_idx)
		{
			return get_boxed<time_box>(jtype::type_datetime, field_idx);
		}

		jstring jslice::get_string(int field_idx)
		{
			return get_boxed<jstring>(jtype::type_string, field_idx);
		}

		jarray jslice::get_object(int field_idx)
		{
			if (schema == nullptr || class_field_id == null_row || bytes == nullptr) {
				throw std::logic_error("slice is not initialized");
			}

			auto the_class = schema->get_class(class_field_id);
			jclass_field& jcf = the_class.child(field_idx);
			jfield jf = schema->get_field(jcf.field_id);
			if (jf.type_id != jtype::type_object) {
				throw std::invalid_argument("Invalid field type " + std::to_string(jtype::type_object) + " for field idx " + std::to_string(field_idx));
			}

			char *b = &bytes[jcf.offset];
			jarray jerry(schema, field_idx, b);
			return jerry;
		}

		int jslice::size()
		{
			auto the_class = schema->get_class(class_field_id);
			return the_class.size();
		}

		dimensions_type jarray::dimensions()
		{
			jfield& field = schema->get_field(class_field_id);
			jclass the_class = schema->get_class(field.type_id);
			dimensions_type& dim = field.object_properties.dim;
			return dim;
		}

		jslice jarray::get_slice(int x, int y, int z)
		{
			dimensions_type dims;
			dims.x = x;
			dims.y = y;
			dims.z = z;
			return get_slice(dims.x, dims.y, dims.z);
		}

		jslice jarray::get_slice(dimensions_type dims)
		{
			jfield& field = schema->get_field(class_field_id);
			jclass the_class = schema->get_class(field.type_id);
			dimensions_type dim = field.object_properties.dim;
			if ((dims.x >= dim.x) ||
				(dims.y >= dim.y) ||
				(dims.z >= dim.z)) {
				throw std::invalid_argument("field " + field.name + " out of range.");
			}
			char* b = &bytes[ ((dims.z * dim.y * dim.x) + (dims.y * dim.x) + dims.x ) * field.object_properties.class_size_bytes ];
			jslice slice(schema, field.field_id, b, dims);
		}

		void jschema::create_standard_fields() 
		{
			create_string_field_request string_fields[32] = {
				{ field_full_name, jtype::type_string , "fullName", "Full Name", 75, "", "" },
				{ field_first_name, jtype::type_string , "firstName", "First Name", 50, "", "" },
				{ field_last_name, jtype::type_string , "lastName", "Last Name", 50, "", "" },
				{ field_middle_name, jtype::type_string , "middleName", "Middle Name", 50, "", "" },
				{ field_ssn, jtype::type_string , "ssn", "SSN", 10, "", "" },
				{ field_email, jtype::type_string, "email", "eEmail", 200, "", "" },
				{ field_title, jtype::type_string, "title", "Title", 200, "", "" },
				{ field_street, jtype::type_string, "street", "Street", 200, "", "" },
				{ field_substreet, jtype::type_string, "suiteapt", "Suite/Apt", 100, "", "" },
				{ field_city, jtype::type_string, "city", "City", 100, "", "" },
				{ field_state, jtype::type_string, "state", "State", 100, "", "" },
				{ field_postal, jtype::type_string, "postal", "Postal Code", 50, "", "" },
				{ field_country_name, jtype::type_string, "countryName", "Country Name", 50, "", "" },
				{ field_country_code, jtype::type_string, "countryCode", "Country Code", 3, "", "" },
				{ field_institution_name, jtype::type_string, "institutionName", "Institution Name", 100, "", "" },
				{ field_longname, jtype::type_string, "longName", "Long Name", 200, "", "" },
				{ field_shortname, jtype::type_string, "shortName", "Short Name", 50, "", "" },
				{ field_unit, jtype::type_string, "unit", "Unit", 10, "", "" },
				{ field_code_symbol, jtype::type_string, "symbol", "Symbol", 10, "", "" },
				{ field_code_operator, jtype::type_string, "operator", "Operator", 10, "", "" },
				{ field_windows_path, jtype::type_string, "windowsPath", "Windows Path", 512, "", "" },
				{ field_linux_path, jtype::type_string, "linuxPath", "Linux Path", 512, "", "" },
				{ field_url, jtype::type_string, "url", "Url", 512, "", "" },
				{ field_user_name, jtype::type_string, "userName", "User Name", 100, "", "" },
				{ field_password, jtype::type_string, "passWord", "Password", 100, "", "" },
				{ field_document_title, jtype::type_string, "docTitle", "Document Title", 200, "", "" },
				{ field_section_title, jtype::type_string, "sectionTitle", "Section Title", 200, "", "" },
				{ field_block_title, jtype::type_string, "blockTitle", "Block Title", 200, "", "" },
				{ field_caption, jtype::type_string, "caption", "Caption", 200, "", "" },
				{ field_paragraph, jtype::type_string, "paragraph", "Paragraph", 4000, "", "" },
				{ field_mime_type, jtype::type_string, "mimeType", "MimeType", 100, "", "" },
				{ field_base64_block, jtype::type_string, "base64", "Base64", 100, "", "" }
			};

			create_time_field_request time_fields[2] = {
				{ field_birthday, jtype::type_datetime, "birthday", "Birthday", 0, INT64_MAX },
				{ field_scheduled, jtype::type_datetime, "scheduled", "Scheduled", 0, INT64_MAX },
			};

			create_integer_field_request int_fields[1] = {
				{ field_count, jtype::type_int64, "count", "Count", 0, INT64_MAX },
			};

			create_double_field_request double_fields[14] = {
				{ field_quantity, jtype::type_float64, "quantity", "Quantity", -1E40, 1E40 },
				{ field_latitude, jtype::type_float64, "latitude", "Latitude", -90, 90 },
				{ field_longitude, jtype::type_float64, "longitude", "Longitude", -180, 180 },
				{ field_meters, jtype::type_float64, "meters", "Meters", -1E40, 1E40 },
				{ field_feet, jtype::type_float64, "feet", "Feet", -1E40, 1E40 },
				{ field_kilograms, jtype::type_float64, "kilograms", "Kilograms", -1E40, 1E40 },
				{ field_pounds, jtype::type_float64, "pounds", "Pounds", -1E40, 1E40 },
				{ field_seconds, jtype::type_float64, "seconds", "Seconds", -1E40, 1E40 },
				{ field_minutes, jtype::type_float64, "minutes", "Minutes", -1E40, 1E40 },
				{ field_hours, jtype::type_float64, "hours", "Hours", -1E40, 1E40 },
				{ field_amperes, jtype::type_float64, "amperes", "Amperes", -1E40, 1E40 },
				{ field_kelvin, jtype::type_float64, "kelvin", "Kelvin", -1E40, 1E40 },
				{ field_mole, jtype::type_float64, "moles", "Moles", -1E40, 1E40 },
				{ field_candela, jtype::type_float64, "candels", "Candels", -1E40, 1E40 }
			};

			for (int i = 0; i < sizeof(string_fields) / sizeof(string_fields[0]); i++) {
				create_string_field(string_fields[i]);
			}

			for (int i = 0; i < sizeof(time_fields) / sizeof(time_fields[0]); i++) {
				create_time_field(time_fields[i]);
			}

			for (int i = 0; i < sizeof(int_fields) / sizeof(int_fields[0]); i++) {
				create_integer_field(int_fields[i]);
			}

			for (int i = 0; i < sizeof(double_fields) / sizeof(double_fields[0]); i++) {
				create_double_field(double_fields[i]);
			}

		}

		bool schema_tests()
		{
			dynamic_box box;
			box.init(1 << 21);

			jschema schema;
			row_id_type schema_id;

			schema = jschema::create_schema(&box, 10, 200, 500, schema_id);
			schema.create_standard_fields();

			row_id_type quantity_field_id = schema.find_field("quantity");
			row_id_type last_name_field_id = schema.find_field("lastName");
			row_id_type count_field_id = schema.find_field("count");

			if (quantity_field_id == null_row) {
				std::cout << "find row failed:" << __LINE__ << std::endl;
				return false;
			}

			if (quantity_field_id != field_quantity) {
				std::cout << "field placement failed:" << __LINE__ << std::endl;
				return false;
			}

			if (last_name_field_id == null_row) {
				std::cout << "find row failed" << __LINE__ << std::endl;
				return false;
			}

			if (last_name_field_id != field_last_name) {
				std::cout << "field placement failed:" << __LINE__ << std::endl;
				return false;
			}

			if (count_field_id == null_row) {
				std::cout << "find row failed" << __LINE__ << std::endl;
				return false;
			}

			if (count_field_id != field_count) {
				std::cout << "field placement failed:" << __LINE__ << std::endl;
				return false;
			}

			row_id_type failed_field_id = schema.find_field("badFieldName");

			if (failed_field_id != null_row) {
				std::cout << "find row failed" << __LINE__ << std::endl;
				return false;
			}

			countrybit::database::jschema::create_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.field_ids = { field_last_name, field_first_name, field_birthday, field_title };
			row_id_type person_class_id = schema.create_class(person);

			if (person_class_id == null_row) {
				std::cout << "class create failed failed" << __LINE__ << std::endl;
				return false;
			}

			jclass person_class = schema.get_class(person_class_id);

			if (person_class.size() != 4) {
				std::cout << "class size failed failed" << __LINE__ << std::endl;
				return false;
			}

			for (int i = 0; i < person_class.size(); i++) {
				auto& fldref = person_class.child(i);
				auto& fld = schema.get_field(fldref.field_id);
				std::cout << fld.name << " " << fld.description << " " << fldref.offset << " " << fld.size_bytes << std::endl;
			}

			countrybit::database::jschema::create_object_field_request people;
			people.class_id = person_class_id;
			people.description = "People";
			people.name = "people";
			people.dim = { 100, 1, 1 };
			people.field_id = schema.create_field();
			row_id_type people_field = schema.create_object_field(people);

			if (people_field == null_row) {
				std::cout << "field create failed failed" << __LINE__ << std::endl;
				return false;
			}

			countrybit::database::jschema::create_class_request company;
			company.class_name = "company";
			company.class_description = "a company is a collection of people";
			company.field_ids = { field_last_name, field_first_name, field_birthday, people_field };
			row_id_type company_class_id = schema.create_class(company);

			if (company_class_id == null_row) {
				std::cout << "class create failed failed" << __LINE__ << std::endl;
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

			schema = jschema::create_schema(&box, 10, 200, 500, schema_id);
			schema.create_standard_fields();

			countrybit::database::jschema::create_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.field_ids = { field_last_name, field_first_name, field_birthday, field_title };
			row_id_type person_class_id = schema.create_class(person);

			if (person_class_id == null_row) {
				std::cout << "class create failed failed" << __LINE__ << std::endl;
				return false;
			}

			collection_id_type colid;

			init_collection_id(colid);
			
			jcollection people = schema.create_collection(&box, colid, 50, person_class_id);

			jarray pa;
			
			pa = people.create_object(person_class_id);

			pa = people.create_object(person_class_id);

			pa = people.create_object(person_class_id);

			pa = people.create_object(person_class_id);

			return true;
		}

		bool array_tests()
		{
			return true;
		}

		bool slice_tests()
		{
			return true;
		}

	}
}
