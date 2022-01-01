
#include "json_parse.h"
#include <charconv>

namespace countrybit
{
	namespace system
	{
		void parser::skip_whitespace() 
		{
			while (get_space());
		}

		int parser::match(int start_index, int num_groups, match_group* group)
		{
			int original_start = start_index;

			while (num_groups)
			{
				int count = 0;
				int start = start_index;

				group->match = 0;
				switch (group->search_type)
				{
				case match_group::search_types::digits:
					while (std::isdigit(at(start_index))) {
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::digits_and_seps:
					char c = at(start_index);
					while (std::isdigit(c) || c == '_')
					{
						start_index++;
						c = at(start_index);
						count++;
					}
					break;
				case match_group::search_types::period:
					while (at(start_index) == '.')
					{
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::space:
					while (at(start_index) <= 32)
					{
						if (c == '\n')
							line++;
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::plusminus:
					char c = at(start_index);
					while (c == '+' || c == '-' || c <= 32)
					{
						if (c == '\n')
							line++;
						start_index++;
						c = at(start_index);
						count++;
					}
					break;
				case match_group::search_types::E:
					while (at(start_index) == 'E')
					{
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::dollar:
					while (at(start_index) == '$')
					{
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::comma:
					while (at(start_index) == ',')
					{
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::alpha:
					while (std::isalpha(at(start_index)))
					{
						start_index++;
						count++;
					}
					break;
				case match_group::search_types::identifier:
					char c = at(start_index);
					while (std::isalpha(c) || std::isdigit(c) || c == '_')
					{
						start_index++;
						c = at(start_index);
						count++;
					}
					break;
				}

				switch (group->search_counts)
				{
				case match_group::search_counts::search_optional_one:
					if (count < 0 || count > 1) {
						return 0;
					}
					else {
						group->match = start;
					}
					break;
				case match_group::search_counts::search_one:
					if (count != 1) {
						return 0;
					}
					else {
						group->match = start;
					}
					break;
				case match_group::search_counts::search_optional_many:
					if (count < 0) {
						return 0;
					}
					else {
						group->match = start;
					}
					break;
				case match_group::search_counts::search_many:
					if (count < 1) {
						return 0;
					}
					else {
						group->match = start;
					}
					break;
				}
				num_groups--;
				group++;
			}

			return original_start;
		}
		
		get_number_result parser::get_number()
		{
			get_number_result result;

			result.value = 0.0;
			result.success = false;

			result.line_number = line;
			result.char_offset = index;

			/*				enum class search_counts
				{
					search_optional_one,
					search_one,
					search_optional_many,
					search_many
				} search_counts;

				enum class search_types 
				{
					digits,
					digits_and_seps,
					period,
					plusminus,
					E,
					dollar,
					comma,
					alpha,
					space
				} search_type;

				int match;
*/

			match_group groups[5] = { 
				{ match_group::search_counts::search_optional_one, match_group::search_types::plusminus, 0 }, 
				{ match_group::search_counts::search_optional_many, match_group::search_types::space, 0 },
				{ match_group::search_counts::search_many, match_group::search_types::digits, 0 },
				{ match_group::search_counts::search_optional_one, match_group::search_types::period, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::digits, 0 }
			};

			int match_end = match(index, 5, groups);
			if (match_end) 
			{
				const char* first = view.data() + index;
				const char* last = view.data() + match_end;
				auto fcr = std::from_chars( view.data() + index, view.data() + match_end, result.value );
				if (fcr.ptr == first) 
				{
					result.success = true;
					index = match_end;
				}
				else 
				{
					result.message = "Invalid number";
				}
			}
			else 
			{
				result.message = "Expected number";
			}

			return result;
		}

		get_identifier_result parser::get_identifier()
		{
			get_identifier_result result;
			result.success = false;

			match_group groups[2] = {
				{ match_group::search_counts::search_one, match_group::search_types::alpha, 0 },
				{ match_group::search_counts::search_optional_many, match_group::search_types::identifier, 0 }
			};

			result.line_number = line;
			result.char_offset = index;

			int match_end = match(index, 2, groups);
			if (match_end)
			{
				int l = (match_end - index) + 1;
				int dest = data.reserve(l);
				char* c = data.unpack<char>(dest);
				strncpy(c, view.data() + index, l);
				result.success = true;
				result.value = c;
				index = match_end;
			}
			else 
			{
				result.message = "Expected identifier";
				result.line_number = line;
				result.char_offset = index;
			}

			return result;
		}

		get_string_result parser::get_string()
		{
			get_string_result result;
			std::string buff;
			result.success = false;
			result.line_number = line;
			result.char_offset = index;

			char c = get_quote();
			if (!c) {
				result.message = "Expected string";
				return result;
			}
			int maximum_string_size = 1 << 22;
		
			while ((c = get_quote()) && maximum_string_size--)
			{
				if (c = get_escaped_quote())
				{
					buff += c;
				}
				else if (c = get_escaped_char())
				{
					buff += c;
				}
				else 
				{
					buff += c;
				}
			}

			if (!maximum_string_size) {
				result.message = "string too large";
			}

			return result;
		}

		get_object_result parser::get_object()
		{
			get_object_result result;

			return result;
		}

		get_array_result parser::get_array()
		{
			get_array_result result;

			return result;
		}

	}
}

