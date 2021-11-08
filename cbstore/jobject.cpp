
#include "jobject.h"

namespace countrybit
{
	namespace database
	{


		jslice::jslice(jclass& _the_class, jschema& _schema, char* _bytes) :
			schema(_schema),
			the_class(_the_class),
			bytes(_bytes)
		{

		}

		int8_box jslice::get_int8(int field_idx)
		{
			return get_boxed<int8_box>(field_idx, jtype::type_int8);
		}

		int16_box jslice::get_int16(int field_idx)
		{
			return get_boxed<int16_box>(field_idx, jtype::type_int16);
		}

		int32_box jslice::get_int32(int field_idx)
		{
			return jslice::get_boxed<int32_box>(field_idx, jtype::type_int32);
		}

		int64_box jslice::get_int64(int field_idx)
		{
			return jslice::get_boxed<int64_box>(field_idx, jtype::type_int64);
		}

		float_box jslice::get_float(int field_idx)
		{
			return jslice::get_boxed<float_box>(field_idx, jtype::type_float32);
		}

		double_box jslice::get_double(int field_idx)
		{
			return get_boxed<double_box>(field_idx, jtype::type_float64);
		}

		time_box jslice::get_time(int field_idx)
		{
			return get_boxed<time_box>(field_idx, jtype::type_float64);
		}

		jstring jslice::get_string(int field_idx)
		{
			return get_boxed<jstring>(field_idx, jtype::type_string);
		}

		jarray jslice::get_object(int field_idx)
		{
			jclass_field& jcf = the_class.child(field_idx);
			jfield jf = schema.get_field(jcf.field_id);

			char *b = &bytes[jcf.offset];
			jarray jerry(schema, field_idx, b);
			return jerry;
		}

		int jslice::size()
		{
			return the_class.size();
		}

		jarray::jarray(jschema& _schema, row_id_type& _field_id, char* _bytes) :
			schema(_schema),
			class_field_id(_field_id),
			bytes(_bytes)
		{
			
			
		}

		jslice jarray::get_slice(int x, int y, int z)
		{
			jfield& field = schema.get_field(class_field_id);
			jclass the_class = schema.get_class(field.type_id);
			dimensions_type& dim = field.object_properties.dim;
			if ((x >= dim.x) ||
				(y >= dim.y) ||
				(z >= dim.z)) {
				throw std::invalid_argument("field " + field.name + " out of range.");
			}
			char* b = &bytes[(z * dim.y * dim.x) + (y * dim.x) + x];
			return jslice(the_class, schema, b);
		}

		jslice jarray::get_slice(dimensions_type dims)
		{
			return get_slice(dims.x, dims.y, dims.z);
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

			row_id_type schema_id = jschema::create_schema(&box, 10, 200, 500);
			schema = jschema::get_schema(&box, schema_id);
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

			jclass person_class = schema.get_class(person_class_id);
			
			if (person_class.size() != 4) {
				std::cout << "class size failed failed" << __LINE__ << std::endl;
				return false;
			}

			for (int i = 0; i < person_class.size(); i++) {
				auto &fldref = person_class.child(i);
				auto &fld = schema.get_field(fldref.field_id);
				std::cout << fld.name << " " << fld.description << " " << fldref.offset << " " << fld.size_bytes << std::endl;
			}

			return true;
		}

		bool collection_tests()
		{
			
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
