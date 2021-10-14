
#include "jobject.h"

namespace countrybit
{
	namespace database
	{
		const static int

			field_none = 0,
			// string units
			field_fullname = 1,
			field_firstname = 2,
			field_sublastname = 3,
			field_lastname = 4,
			field_middlename = 5,
			field_ssn = 6,
			field_email = 7,
			field_title = 8,
			field_street = 9,
			field_substreet = 10,
			field_city = 11,
			field_state = 12,
			field_postal = 13,
			field_countryname = 14,
			field_countrycode = 15,
			field_longname = 16,
			field_shortname = 17,
			field_unit = 18,
			field_code_symbol = 19,
			field_code_operator = 20,
			field_windows_path = 21,
			field_linux_path = 22,
			field_drive_letter = 23,
			field_user_name = 24,
			field_password = 25,

			// date units
			field_birthday = 26,
			field_scheduled = 27,

			// int units
			field_count = 28,

			// float units
			field_money = 29,
			field_quantity = 30,
			field_latitude = 31,
			field_longitude = 32,
			field_meters = 33,
			field_feet = 34,
			field_kilograms = 35,
			field_pounds = 36,
			field_seconds = 37,
			field_minutes = 38,
			field_hours = 39,
			field_amperes = 40,
			field_kelvin = 41,
			field_mole = 42,
			field_candela = 43,

			// object units
			field_address = 44,
			field_geocode = 45,

			// more string units
			field_institution_name = 46,
			field_subtitle = 47,
			field_caption = 48,
			field_paragraph = 49,

			field_mime_type = 50,
			field_image_base64 = 51;


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
			jclass& the_class = schema.get_class(field.type_id);
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

	}
}
