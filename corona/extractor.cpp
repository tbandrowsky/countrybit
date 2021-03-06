

#include "corona.h"

// #define SHOW_ERROR_MESSAGES

namespace corona
{
	namespace database
	{
		bool string_extractor::skip_whitespace()
		{
			char c, l;
			while (c = get_space()) {
				l = c;
			}
			return c == '\n';
		}

		int string_extractor::get_pattern_count(int& start_index, const std::function<bool(char c)>& matches)
		{
			int count = 0;
			char c = at(start_index);
			while (matches(c)) {
				start_index++;
				count++;
				c = at(start_index);
			}
			return count;
		}

		string_extractor::match_result string_extractor::match(int start_index, int num_groups, match_group* group)
		{
			int original_start = start_index;

			match_result result(view.data(), num_groups, group, start_index);

			while (num_groups)
			{
				int count = 0;
				int start = start_index;

				group->match = 0;
				group->count = 0;

				switch (group->search_type)
				{
				case match_group::search_types::digits:
					count = get_pattern_count(start_index, [](char c) {return std::isdigit(c); });
					break;
				case match_group::search_types::digits_and_seps:
					count = get_pattern_count(start_index, [](char c) {return std::isdigit(c) || c == '_'; });
					break;
				case match_group::search_types::period:
					count = get_pattern_count(start_index, [](char c) {return c == '.'; });
					break;
				case match_group::search_types::space:
					count = get_pattern_count(start_index, [this](char c) { if (c == '\n') line++;  return c <= 32; });
					break;
				case match_group::search_types::plusminus:
					count = get_pattern_count(start_index, [](char c) { return c == '+' || c == '-'; });
					break;
				case match_group::search_types::E:
					count = get_pattern_count(start_index, [](char c) { return c == 'E'; });
					break;
				case match_group::search_types::dollar:
					count = get_pattern_count(start_index, [](char c) { return c == '$'; });
					break;
				case match_group::search_types::comma:
					count = get_pattern_count(start_index, [](char c) { return c == ','; });
					break;
				case match_group::search_types::alpha:
					count = get_pattern_count(start_index, [](char c) { return std::isalpha(c); });
					break;
				case match_group::search_types::identifier:
					count = get_pattern_count(start_index, [](char c) { return std::isalnum(c) || c == '_'; });
					break;
				case match_group::search_types::datesep:
					count = get_pattern_count(start_index, [](char c) { return c == '/' || c == '_' || c == '-' || c == ':' || c == '.'; });
					break;
				case match_group::search_types::operchars:
					count = get_pattern_count(start_index, [](char c) { return c == '+' || c == '^' || c == '&' || c == '!' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '.' || c == '=' || c == '<' || c == '>'; });
					break;
				case match_group::search_types::hex:
					count = get_pattern_count(start_index, [](char c) { return std::isdigit(c) || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f'; });
					break;
				case match_group::search_types::pound:
					count = get_pattern_count(start_index, [](char c) { return c == '#'; });
					break;
				}

				switch (group->search_counts)
				{
				case match_group::search_counts::search_optional_one:
					if (count < 0 || count > 1) {
						return match_result::empty();
					}
					else {
						group->match = start;
						group->count = count;
					}
					break;
				case match_group::search_counts::search_one:
					if (count != 1) {
						return match_result::empty();
					}
					else {
						group->match = start;
						group->count = count;
					}
					break;
				case match_group::search_counts::search_optional_many:
					if (count < 0) {
						return match_result::empty();
					}
					else {
						group->match = start;
						group->count = count;
					}
					break;
				case match_group::search_counts::search_many:
					if (count < 1) {
						return match_result::empty();
					}
					else {
						group->match = start;
						group->count = count;
					}
					break;
				case match_group::search_counts::search_exact:
					if (count != group->search_exact_count) {
						return match_result::empty();
					}
					else {
						group->match = start;
						group->count = count;
					}
					break;
				}
				num_groups--;
				group++;
			}

			result.end_index = start_index;

			return result;
		}

		const char* error_invalid_number = "Bad Number.  The parser thought this was a number, but, this text could not be converted to a double.";
		const char* error_expected_number = "Expected Number.  A number was expected at this point, yet, sadly, other things were found.";

		get_number_result string_extractor::get_number()
		{
			get_number_result result;

			result.value = 0.0;
			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[5] = {
				{ match_group::search_counts::search_optional_one, match_group::search_types::plusminus, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_one, match_group::search_types::period, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 5, groups);
			if (!matches.is_empty())
			{
				const char* first_link = matches.begin();
				const char* last_link = matches.end();
				auto fcr = std::from_chars(first_link, last_link, result.value);
				if (fcr.ptr == last_link)
				{
					result.success = true;
					index = matches.end_index;
				}
				else
				{
					result.message = error_invalid_number;
				}
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		const char* error_expected_identifer = "Expected identifier.  A token of the form 'SomeVariable_Name' was expected at this point, but instead, something else was found.";
		const char* error_expected_operator = "An operator was expected, but somehow, you bolluxed that up.";

		get_identifier_result string_extractor::get_identifier()
		{
			get_identifier_result result;
			result.success = false;

			match_group groups[2] = {
				{ match_group::search_counts::search_many, match_group::search_types::alpha, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::identifier, 0 }
			};

			result.line_number = line;
			result.char_offset = index;

			auto matches = match(index, 2, groups);
			if (!matches.is_empty())
			{
				char *c = data.copy(view.data(), index, matches.end_index, true);
				result.success = true;
				result.value = c;
				index = matches.end_index;
				result.success = true;
			}
			else
			{
				result.message = error_expected_identifer;
				result.line_number = line;
				result.char_offset = index;
			}

			return result;
		}

		get_operator_result string_extractor::get_operator()
		{
			get_operator_result result;
			result.success = false;

			match_group groups[1] = {
				{ match_group::search_counts::search_many, match_group::search_types::operchars, 0 }
			};

			result.line_number = line;
			result.char_offset = index;

			auto matches = match(index, 1, groups);
			if (!matches.is_empty())
			{
				char* c = data.copy(view.data(), index, matches.end_index, true);
				result.success = true;
				result.value = c;
				index = matches.end_index;
				result.success = true;
			}
			else
			{
				result.message = error_expected_identifer;
				result.line_number = line;
				result.char_offset = index;
			}

			return result;
		}

		get_dimension_result string_extractor::get_dimensions()
		{
			get_dimension_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[9] = {
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },			
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 9, groups);
			if (!matches.is_empty())
			{
				result.x = matches.get_number(0);
				result.y = matches.get_number(4);
				result.z = matches.get_number(7);
				result.success = true;
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		get_datetime_result string_extractor::get_date()
		{
			get_datetime_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[9] = {
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::datesep, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::datesep, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 9, groups);
			if (!matches.is_empty())
			{
				result.months = matches.get_number(0);
				result.days = matches.get_number(4);
				result.years = matches.get_number(8);
				result.success = true;
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		get_color_result string_extractor::get_color_alpha()
		{
			get_color_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[2] = {
				{ match_group::search_counts::search_one, match_group::search_types::pound, 0, 0, 0 },
				{ match_group::search_counts::search_exact, match_group::search_types::hex, 8, 0, 0 }
			};

			auto matches = match(index, 2, groups);
			if (!matches.is_empty())
			{
				auto str_view = matches.get_string(1);

				uint32_t result_hex;

				struct color_block 
				{
					uint8_t a, b, g, r;
				} *cb;

				cb = (color_block*)&result_hex;

				auto [ptr, ec] { std::from_chars(str_view.data(), str_view.data() + str_view.size(), result_hex, 16) };

				if (ec == std::errc())
				{
					result.red = (cb->r) / 255.0;
					result.green = (cb->g) / 255.0;
					result.blue = (cb->b) / 255.0;
					result.alpha = (cb->a) / 255.0;
					result.success = true;
				}
				else if (ec == std::errc::invalid_argument)
				{
					result.success = false;
					result.message = error_expected_number;
				}
				else if (ec == std::errc::result_out_of_range)
				{
					result.success = false;
					result.message = "This number is larger than an int.\n";
				}
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		get_color_result string_extractor::get_color()
		{
			get_color_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[2] = {
				{ match_group::search_counts::search_one, match_group::search_types::pound, 0, 0, 0 },
				{ match_group::search_counts::search_exact, match_group::search_types::hex, 6, 0, 0 }
			};

			auto matches = match(index, 2, groups);
			if (!matches.is_empty())
			{
				auto str_view = matches.get_string(1);

				uint32_t result_hex;

				struct color_block
				{
					uint8_t a, b, g, r;
				} *cb;

				cb = (color_block*)&result_hex;
				cb->a = 0xff;

				auto [ptr, ec] { std::from_chars(str_view.data(), str_view.data() + str_view.size(), result_hex, 16) };

				if (ec == std::errc())
				{
					result.red = (cb->r) / 255.0;
					result.green = (cb->g) / 255.0;
					result.blue = (cb->b) / 255.0;
					result.alpha = (cb->a) / 255.0;
					result.success = true;
				}
				else if (ec == std::errc::invalid_argument)
				{
					result.success = false;
					result.message = error_expected_number;
				}
				else if (ec == std::errc::result_out_of_range)
				{
					result.success = false;
					result.message = "This number is larger than an int.\n";
				}
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}


		get_rectangle_result string_extractor::get_rectangle()
		{
			get_rectangle_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[16] = {
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 13, groups);
			if (!matches.is_empty())
			{
				result.x = matches.get_number(0);
				result.y = matches.get_number(4);
				result.w = matches.get_number(8);
				result.h = matches.get_number(12);
				result.success = true;
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		get_point_result string_extractor::get_point()
		{
			get_point_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[5] = {
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 5, groups);
			if (!matches.is_empty())
			{
				result.x = matches.get_number(0);
				result.y = matches.get_number(4);
				result.z = 0;
				result.success = true;
			}
			else
			{
				result.message = error_expected_number;
			}

			return result;
		}

		get_audio_result string_extractor::get_audio()
		{
			get_audio_result result;

			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			match_group groups[17] = {
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_one, match_group::search_types::comma, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },

				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 }
			};

			auto matches = match(index, 8, groups);
			if (!matches.is_empty())
			{
				result.start_seconds = matches.get_number(0);
				result.stop_seconds = matches.get_number(4);
				result.pitch_adjust = matches.get_number(8);
				result.volume_adjust = matches.get_number(12);
				result.playing = matches.get_number(16);
				result.success = true;
			}
			else
			{
				result.message = error_expected_number;
			}
			return result;
		}

		const char* error_expected_string = "Expected string.";
		const char* error_unknown_directive = "Unknown $ directive.";
		const char* error_no_memory_for_string = "No memory. There was not enough space in the box for this string.  Make the parser memory larger.";
		const char* error_string_too_long = "String too large.  The string was simply too big for us to accept.  Max size is 1 << 25 chars.";
		const char* error_string_not_terminated = "String too large.  The string was simply too big for us to accept.  Max size is 1 << 25 chars.";

		get_string_result string_extractor::get_string()
		{
			get_string_result result;

			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			char c = get_quote();
			if (!c)
			{
				result.message = error_expected_string;
				result.line_number = line;
				result.char_offset = index;
				return result;
			}

			int maximum_string_size = 1 << 25;

			result.value = nullptr;

			auto cx = get_string_char();
			while ((cx.c && cx.c != '"') || (cx.c == '"' && cx.escaped == true))
			{
				int l = data.pack(cx.c);
				if (l<0) {
					result.line_number = line;
					result.char_offset = index;
					result.message = error_no_memory_for_string;
					return result;
				}

				if (!result.value)
				{
					result.value = data.unpack<char>(l);
				}
				maximum_string_size--;
				if (!maximum_string_size) {
					result.message = error_string_too_long;
					result.line_number = line;
					result.char_offset = index;
					return result;
				}
				cx = get_string_char();
			}

			char null = 0;
			int lx = data.pack(null);

			if (!result.value)
			{
				result.value = data.unpack<char>(lx);
			}

			if (cx.c != '"' || cx.escaped != false)
			{
				result.success = false;
				result.line_number = line;
				result.char_offset = index;
				result.message = error_string_not_terminated;
				return result;
			}

			result.success = true;
			return result;
		}

		const char* error_expected_value = "Expected a value after the colon - either a string, another object, a number, or an array.";


		parse_json_value_result string_extractor::parse_json_value()
		{

			parse_json_value_result result;

			result.line_number = line;
			result.char_offset = index;
			result.success = false;

			pvalue* value = data.allocate<pvalue>(1);
			value->next_link = nullptr;
			result.value = value;
			value->line = line;
			value->index = index;

			skip_whitespace();

			char c = at(index);

			switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '+':
			case '-':
			{
				value->pvalue_type = pvalue::pvalue_types::double_value;
				get_number_result gnr = get_number();
				if (gnr.success) {
					value->double_value = gnr.value;
				}
				else {
					result.message = gnr.message;
					result.line_number = gnr.line_number;
					result.char_offset = gnr.char_offset;
					return result;
				}
			}
			break;
			case '"':
			{
				value->pvalue_type = pvalue::pvalue_types::string_value;
				get_string_result gnr = get_string();
				if (gnr.success) {
					value->string_value = gnr.value;
				}
				else {
					result.message = gnr.message;
					result.line_number = gnr.line_number;
					result.char_offset = gnr.char_offset;
					return result;
				}
			}
			break;
			case '{':
			{
				value->pvalue_type = pvalue::pvalue_types::object_value;
				parse_json_object_result pjor = parse_json_object();
				if (pjor.success) {
					value->object_value = pjor.value;
				}
				else {
					result.message = pjor.message;
					result.line_number = pjor.line_number;
					result.char_offset = pjor.char_offset;
					return result;
				}
			}
			break;
			case '[':
			{
				value->pvalue_type = pvalue::pvalue_types::array_value;
				parse_json_array_result pjar = parse_json_array();
				if (pjar.success) {
					value->array_value = pjar.value;
				}
				else {
					result.message = pjar.message;
					result.line_number = pjar.line_number;
					result.char_offset = pjar.char_offset;
					return result;
				}
			}
			break;
			case '$':
			{
				this->get_char();
				auto gir = get_identifier();
				if (gir.success)
				{
					if (strcmp(gir.value, "matrix") == 0) 
					{
						value->pvalue_type = pvalue::pvalue_types::array_value;
						parse_json_array_result pjar = parse_matrix();
						if (pjar.success) {
							value->array_value = pjar.value;
						}
						else {
							result.message = pjar.message;
							result.line_number = pjar.line_number;
							result.char_offset = pjar.char_offset;
							return result;
						}
					}
					else if (strcmp(gir.value, "csv") == 0)
					{
						parse_json_array_result pjar = parse_csv();
						if (pjar.success) {
							value->array_value = pjar.value;
						}
						else {
							result.message = pjar.message;
							result.line_number = pjar.line_number;
							result.char_offset = pjar.char_offset;
							return result;
						}
					}
					else
					{
						result.message = error_unknown_directive;
						result.line_number = line;
						result.char_offset = index;
						return result;
					}
				}
				else 
				{
					result.message = gir.message;
					result.line_number = gir.line_number;
					result.char_offset = gir.char_offset;
					return result;
				}
			}
			break;
			default:
				result.message = error_expected_value;
				result.line_number = line;
				result.char_offset = index;
				return result;
			}
			result.success = true;
			return result;
		}

		const char* error_expected_object = "Expected an object, one of those chumpies with a { and }.";
		const char* error_expected_object_end = "Expected either a comma to add another member, or a } to conclude the object.";
		const char* error_expected_colon = "Expected a colon here.  A key value pair takes the form ""key"" : value.";

		parse_json_object_result string_extractor::parse_json_object()
		{
			parse_json_object_result result;

			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			skip_whitespace();
			char c = get_object_start();

			if (c)
			{
				pobject* obj = data.allocate<pobject>(1);
				obj->first_link = nullptr;
				obj->num_members = 0;
				obj->line = line;
				obj->index = index;

				skip_whitespace();
				c = at(index);
				while (c != '}')
				{
					pmember* member = data.allocate<pmember>(1);
					obj->add(member);
					member->name = nullptr;
					member->value = nullptr;
					member->next_link = nullptr;

					skip_whitespace();

					auto gsr = get_string();
					if (!gsr.success) {
						result.message = gsr.message;
						result.line_number = gsr.line_number;
						result.char_offset = gsr.char_offset;
						return result;
					}
					member->name = gsr.value;
					if (type_member_name && !strcmp(member->name, type_member_name)) {
						obj->type_member = member;
					}

					skip_whitespace();

					c = get_colon();
					if (c) {
						skip_whitespace();

						auto valresult = parse_json_value();
						if (!valresult.success) {
							result.line_number = valresult.line_number;
							result.char_offset = valresult.char_offset;
							result.message = valresult.message;
							return result;
						}
						member->value = valresult.value;

						skip_whitespace();
						
						if (c = get_comma()) {
							c = at(index);
						}
						else if (!(c = get_object_stop())) {
							result.line_number = line;
							result.char_offset = index;
							result.message = error_expected_object_end;
							return result;
						}
					}
					else {
						result.line_number = line;
						result.char_offset = index;
						result.message = error_expected_colon;
						return result;
					}
				}
				result.success = true;
				result.value = obj;
			}
			else {
				result.line_number = line;
				result.char_offset = index;
				result.message = error_expected_object;
			}
			return result;
		}

		const char* error_expected_array = "Expected an array, one of those chumpies with a [ and ].";
		const char* error_expected_array_end = "Expected either a comma to add another element, or a ] to conclude the array.";

		parse_json_array_result string_extractor::parse_json_array()
		{
			parse_json_array_result result;

			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			skip_whitespace();
			char c = get_array_start();

			if (c)
			{
				parray* pa = data.allocate<parray>(1);
				pa->num_elements = 0;
				pa->first_link = nullptr;
				pa->line = line;
				pa->index = index;

				skip_whitespace();
				while (c != ']' && c)
				{
					skip_whitespace();					
					c = at(index);
					if (c == ']') 
					{
						index++;
						break;
					}

					auto valresult = parse_json_value();
					if (valresult.success) {
						pvalue* v = valresult.value;
						pa->add(v);
					}
					else {
						result.line_number = valresult.line_number;
						result.char_offset = valresult.char_offset;
						result.message = valresult.message;
						return result;
					}
					skip_whitespace();
					if (c = get_comma()) {
						skip_whitespace();
					}
					else if (!(c = get_array_stop())) {
						result.line_number = line;
						result.char_offset = index;
						result.message = error_expected_array_end;
						return result;
					}
				}
				result.success = true;
				result.value = pa;
			}
			else {
				result.line_number = line;
				result.char_offset = index;
				result.message = error_expected_array;
			}

			return result;
		}

		parse_json_array_result string_extractor::parse_matrix()
		{
			parse_json_array_result result;

			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			skip_whitespace();
			char c = get_array_start();

			if (c)
			{
				parray* pa = data.allocate<parray>(1);
				pa->num_elements = 0;
				pa->first_link = nullptr;
				pa->line = line;
				pa->index = index;

				skip_whitespace();

				auto object_result = parse_json_object();

				if (!object_result.success)
				{
					result.success = false;
					result.line_number = object_result.line_number;
					result.char_offset = object_result.char_offset;
					result.message = "missing matrix object";
					return result;
				}

				int64_t x=1, y=1, z=1;

				auto xm = object_result.value->get_member("x");
				auto ym = object_result.value->get_member("y");
				auto zm = object_result.value->get_member("z");
				auto mapm = object_result.value->get_member("map");
				const pobject* mm = nullptr;

				if (mapm) 
				{
					mm = mapm->value->as_object();
					if (!mm) {
						result.success = false;
						result.line_number = object_result.line_number;
						result.char_offset = object_result.char_offset;
						result.message = "an object map, if supplied, must be an object.  But if you tried to use an array, I get your point and will fix it eventually.";
						return result;
					}
				}

				if (!xm) {
					result.success = false;
					result.line_number = object_result.line_number;
					result.char_offset = object_result.char_offset;
					result.message = "missing x dimension";
					return result;
				}

				const double *xmd = xm->value->as_double();
				if (!xmd) {
					result.success = false;
					result.line_number = object_result.line_number;
					result.char_offset = object_result.char_offset;
					result.message = "x dimensions must be numbers";
					return result;
				}

				if (ym) {
					const double* ymd = ym->value->as_double();
					if (!ymd) {
						result.success = false;
						result.line_number = object_result.line_number;
						result.char_offset = object_result.char_offset;
						result.message = "y dimensions must be numbers";
						return result;
					}
					y = (int64_t)*ymd;
					if (y < 1) y = 1;
				}

				if (zm) {
					const double* zmd = zm->value->as_double();
					if (!zmd) {
						result.success = false;
						result.line_number = object_result.line_number;
						result.char_offset = object_result.char_offset;
						result.message = "z dimensions must be numbers";
						return result;
					}
					z = (int64_t)*zmd;
					if (z < 1) z = 1;
				}

				int64_t ix = 0, iy = 0, iz = 0;

				while (c != ']' && c)
				{
					skip_whitespace();
					c = at(index);
					if (c == ']')
					{
						index++;
						break;
					}

					auto valresult = parse_json_value();
					if (valresult.success) {
						pa->num_elements++;
						pvalue* v = valresult.value;

						if (mm && !v->as_array() && !v->as_object())
						{
							
							istring<2048> temp;

							const char* s = v->as_string();

							// cast the value to a member of the map

							if (!s)
							{
								const double* d = v->as_double();
								if (d)
								{
									temp = *d;
									s = temp.c_str();
								}
							}

							if (!s)
							{
								result.line_number = valresult.line_number;
								result.char_offset = valresult.char_offset;
								result.message = "value must be a castable as a member to be mapped";
								return result;
							}

							auto mbr = mm->get_member(s);

							if (!mbr)
							{
								result.line_number = valresult.line_number;
								result.char_offset = valresult.char_offset;
								result.message = "value not found as a member of the map";
								return result;
							}

							// we make a shallow copy of the map member...

							char* pv_bytes = data.place<pvalue>();
							v = new (pv_bytes) pvalue(*mbr->value);
						}

						v->x = ix;
						v->y = iy;
						v->z = iz;

						pa->add(v);
					}
					else {
						result.line_number = valresult.line_number;
						result.char_offset = valresult.char_offset;
						result.message = valresult.message;
						return result;
					}
					skip_whitespace();

					ix++;
					if (ix == x) {
						ix = 0;
						iy++;
						if (iy == y) {
							iy = 0;
							iz++;
						}
					}
				}

				result.success = true;
				result.value = pa;
			}
			else {
				result.line_number = line;
				result.char_offset = index;
				result.message = error_expected_array;
			}

			return result;
		}

		parse_json_array_result string_extractor::parse_csv()
		{
			parse_json_array_result result;

			transformer ts(data);

			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			skip_whitespace();
			char c = get_array_start();

			result.value = data.allocate<parray>(1);

			if (c)
			{
				parray* pa_header = data.allocate<parray>(1);
				parray* pa_current = nullptr;
				pobject* po_header = nullptr;

				pa_header->num_elements = 0;
				pa_header->first_link = nullptr;
				pa_header->line = line;
				pa_header->index = index;

				enum parse_states 
				{
					building_object_template,
					building_objects
				} parse_state;

				parse_state = parse_states::building_object_template;

				while (c != ']' && c)
				{
					auto pvresult = parse_json_value();
					if (!pvresult.success) {
						result.success = false;
						result.message = pvresult.message;
						result.line_number = pvresult.line_number;
						result.char_offset = pvresult.char_offset;
						return result;
					}
					auto pv = pvresult.value;
					bool eol = skip_whitespace();
					switch (parse_state) {
					case parse_states::building_object_template:
						pa_header->add(pv);
						if (eol) 
						{
							parse_state = parse_states::building_objects;
							po_header = ts.array_to_object_template(pa_header);
							pa_current = data.allocate<parray>(1);
						}
						break;
					case parse_states::building_objects:
						pa_header->add(pv);
						if (eol)
						{
							parse_state = parse_states::building_objects;
							pobject* tempo = ts.array_to_object(po_header, pa_current);
							pvalue* tempv = data.allocate<pvalue>(1);
							tempv->object_value = tempo;
							tempv->pvalue_type = pvalue::pvalue_types::object_value;
							tempv->index = index;
							tempv->line = line;
							result.value->add(tempv);
							pa_current = data.allocate<parray>(1);
						}
						break;
					}
					c = get_char();
					if (c == ',') {
						eol = skip_whitespace();
						if (eol) {
							result.success = false;
							result.message = "end of line with trailing comma not allowed";
							result.line_number = line;
							result.char_offset = index;
							return result;
						}
					}
					else if (c != ']') 
					{
						result.success = false;
						result.message = "expected end of array or comma for next item";
						result.line_number = line;
						result.char_offset = index;
						return result;
					}
				}
			}
			else {
				result.line_number = line;
				result.char_offset = index;
				result.message = error_expected_array;
			}

			return result;
		}

		get_expression_result string_extractor::parse_expression()
		{
			get_expression_result ger;

			ger.start();

			skip_whitespace();

			char c = at(index);
			bool accepted = true;

			while (c && accepted)
			{
				if (std::isdigit(c)) 
				{
					auto gxr = get_number();
					accepted = ger.accept(gxr);
				}
				else if (std::isalpha(c))
				{
					auto gxr = get_identifier();
					accepted = ger.accept(gxr);
				}
				else if (std::isdigit(c))
				{
					auto gxr = get_number();
					accepted = ger.accept(gxr);
				}
				else if (c == '"')
				{
					auto gxr = get_string();
					accepted = ger.accept(gxr);
				}

				skip_whitespace();
			}

			ger.stop();

			return ger;
		}

		bool string_extractor::test_basics()
		{
			int count = 0;
			std::string pattern_test1 = "12345";
			string_extractor p1(pattern_test1, 200, "type");
			count = p1.get_pattern_count(p1.index, [](char c) {return std::isdigit(c); });
			if (count != 5 || p1.index != 5) {
				std::cout << __LINE__ << ":digit pattern count failed" << std::endl;
				return false;
			}

			std::string pattern_test2 = "abc123";
			string_extractor p2(pattern_test2, 200, "type");
			count = p2.get_pattern_count(p2.index, [](char c) {return std::isalpha(c); });

			if (count != 3 || p2.index != 3) {
				std::cout << __LINE__ << ":alpha pattern count failed" << std::endl;
				return false;
			}

			if (p2.get_char() != '1') {
				std::cout << __LINE__ << ":alpha pattern next failed" << std::endl;
				return false;
			}

			std::string good_identifier = "  Alpha42_Something  ";
			string_extractor p3(good_identifier, 2000, "type");
			p3.skip_whitespace();
			if (p3.at(p3.index) != 'A') {
				std::cout << __LINE__ << ":expected to hit A in test" << std::endl;
				return false;
			}

			auto p3r = p3.get_identifier();
			if (!p3r.success || strcmp("Alpha42_Something", p3r.value)) {
				std::cout << __LINE__ << ":could not extract identifier" << std::endl;
				return false;
			}

			std::string bad_identifier = "  0Alpha42_Something  ";
			string_extractor p4(bad_identifier, 2000, "type");
			p4.skip_whitespace();
			auto p4r = p4.get_identifier();
			if (p4r.success) {
				std::cout << __LINE__ << ":should not have extracted identifier" << std::endl;
				return false;
			}

			std::string good_string1 = "   \"Alpha42_\\\"stuff\\\"Something\"  ";
			string_extractor p5(good_string1, 2000, "type");
			p5.skip_whitespace();
			auto p5r = p5.get_string();
			if (!p5r.success || strcmp("Alpha42_\"stuff\"Something", p5r.value)) {
				std::cout << __LINE__ << ":extract string 1 failed" << std::endl;
				return false;
			}

			std::string good_string2 = "   \"Alpha42\\\"_Some\"thing  ";
			string_extractor p6(good_string2, 2000, "type");
			p6.skip_whitespace();
			auto p6r = p6.get_string();
			if (!p6r.success || strcmp("Alpha42\"_Some", p6r.value)) {
				std::cout << __LINE__ << ":extract string 2 failed" << std::endl;
				return false;
			}

			std::string bad_string1 = "\"stuff";
			string_extractor p7(bad_string1, 2000, "type");
			auto p7r = p7.get_string();
			if (p7r.success) {
				std::cout << __LINE__ << ":should not have extracted string" << std::endl;
				return false;
			}

			std::string sampleJsonObject1 = R"^(
{ "name_user" : "todd",
  "age" : 42 }
  )^";

			string_extractor p8(sampleJsonObject1, 4000, "type");
			auto p8r = p8.parse_json_object();
			if (!p8r.success) {
				std::cout << __LINE__ << ":should have extracted json object" << std::endl;
				return false;
			}

			if (p8r.value->num_members != 2) {
				std::cout << __LINE__ << ":wrong member count" << std::endl;
				return false;
			}

			auto member1 = p8r.value->first_link;
			if (strcmp(member1->name, "name_user")) {
				std::cout << __LINE__ << ":wrong member name 1" << std::endl;
				return false;
			}

			if (member1->value->pvalue_type != pvalue::pvalue_types::string_value) {
				std::cout << __LINE__ << ":wrong type member 1" << std::endl;
				return false;
			}

			if (strcmp(member1->value->string_value, "todd")) {
				std::cout << __LINE__ << ":wrong value member 1" << std::endl;
				return false;
			}

			auto member2 = member1->next_link;
			if (strcmp(member2->name, "age")) {
				std::cout << __LINE__ << ":wrong name member 2" << std::endl;
				return false;
			}

			if (member2->value->pvalue_type != pvalue::pvalue_types::double_value) {
				std::cout << __LINE__ << ":wrong type member 2" << std::endl;
				return false;
			}

			if (member2->value->double_value != 42.0) {
				std::cout << __LINE__ << ":wrong value member 2" << std::endl;
				return false;
			}

			return true;
		}

		bool string_extractor::test_json(int _case_line, const std::string& _src, int _expected_failure_line)
		{
			std::string_view sv(_src);
			string_extractor p8(sv, 8000, "type");
			auto p8r = p8.parse_json_object();
			if (_expected_failure_line) {
				if (p8r.success) {
					std::cout << _case_line << ": test " << __LINE__ << ": should have failed object" << std::endl;
					return false;
				}
				if (p8r.line_number != _expected_failure_line)
				{
					std::cout << _case_line << ": test " << __LINE__ << ": should have failed at line " << _expected_failure_line << " but failed at " << p8r.line_number << std::endl;
					return false;
				}
#ifdef SHOW_ERROR_MESSAGES
				std::cout << _case_line << ": pass (message: " << p8r.line_number << ":" << p8r.message << ")" << std::endl;
#endif

			}
			else if (!p8r.success)
			{
				std::cout << _case_line << ": test " << __LINE__ << ": object should have passed but failed with :" << p8r.message << " at " << p8r.line_number << std::endl;
				return false;
			}
			return true;
		}

		bool string_extractor::test_json()
		{
			std::string sampleJsonArrayGood = R"^(
{ "people" : [ 
{ 
	"name_user" : "todd",
	"age" : 42 
},
{ 
	"name_user" : "bob",
	"age" : 35 
},
{ 
	"name_user" : "sam",
	"age" : 41 
}
] }
  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonArrayGood, 0))
				return false;

			std::string sampleJsonArrayBad = R"^(
{ "people" : [ 
{ 
	"name_user" : "todd",
	"age" : 42 
},
[ 
	"name_user" : "bob",
	"age" : 35 
},
{ 
	"name_user" : "sam",
	"age" : 41 
}
] }
  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonArrayBad, 8))
				return false;

			std::string sampleJsonArrayBad2 = R"^(
{ "people" : [ 
{ 
	name_user : "todd",
	"age" : 42 
},
{
	name_user : "bob",
	"age" : 35 
},
{ 
	"name_user" : "sam",
	"age" : 41 
}
]}
  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonArrayBad2, 4))
				return false;

			std::string sampleJsonArrayBad3 = R"^(
{
	"name_user"  "todd",
	"age" : 42 
  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonArrayBad3, 3))
				return false;

			std::string sampleJsonArrayBad4 = R"^(
{
	"name_user" : "todd",
	"age"  42 
  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonArrayBad4, 4))
				return false;

			std::string sampleJsonArrayGoodEmpty = R"^(
{
	"name_user" : [ ]
}  )^";


			if (!string_extractor::test_json(__LINE__, sampleJsonArrayGoodEmpty, 0))
				return false;

			std::string sampleJsonObjectGoodEmpty = R"^(
{
	"name_user" : { }
}  )^";

			if (!string_extractor::test_json(__LINE__, sampleJsonObjectGoodEmpty, 0))
				return false;


			std::string sampleJsonArrayGoodEmpty2 = R"^(
{ "people" : [ 
{ 
	"name_user" : "todd",
	"age" : 42,
	"friends": [ "bob", "ted" ]
},
{ 
	"name_user" : "bob",
	"age" : 35,
	"friends": [ "todd", "jan", "dave" ]
},
{ 
	"name_user" : "roger",
	"age" : 41,
	"friends": [ ]
}
] }
  )^";
			if (!string_extractor::test_json(__LINE__, sampleJsonArrayGoodEmpty2, 0))
				return false;

			std::string sampleJsonArrayGood3 = R"^(
{ "people" : [ 
{ 
	"name_user" : "todd",
	"age" : 42,
	"friends": [ "bob", "ted", { "name": "sam", "age": 42 } ]
},
{ 
	"name_user" : "bob",
	"age" : 35,
	"friends": [ { "name": "sam", "age": 42 }, "jan", "dave" ]
},
{ 
	"name_user" : "roger",
	"age" : 41,
	"friends": [ { "name": "sam", "age": 42 } ]
},
{ 
	"name_user" : "roger",
	"age" : 45,
	"friends": [ { "name": "wilson", "age": 52, "shoes": [ "red", "green", "black", "blue"] } ]
}
] }
  )^";
			if (!string_extractor::test_json(__LINE__, sampleJsonArrayGood3, 0))
				return false;

			return true;
		}
	}
}

