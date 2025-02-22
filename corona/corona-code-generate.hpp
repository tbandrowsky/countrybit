#ifndef CORONA_CODE_GENERATE_HPP
#define CORONA_CODE_GENERATE_HPP

namespace corona
{

	class corona_code_generate_request
	{
	public:
		std::string class_name;
		std::vector<std::string> search_fields;
		std::vector<std::string> list_columns;
		std::map<std::string, bool> ignore_edit_fields;

		corona_code_generate_request(const corona_code_generate_request& _src) = default;
		corona_code_generate_request(corona_code_generate_request&& _src) = default;

		corona_code_generate_request(const corona_code_generate_request& _src) = default;
		corona_code_generate_request(corona_code_generate_request&& _src) = default;
	};

	class corona_code_generator_base
	{
	public:
		std::string generator_name;
		virtual json generate(corona_database_interface* _database, const corona_code_generate_request& _request) = 0;
	};

	class corona_winui_generator : public corona_code_generator_base
	{
	public:
		virtual json generate(corona_database_interface* _database, const corona_code_generate_request& _request) override;
	};
}

#endif
