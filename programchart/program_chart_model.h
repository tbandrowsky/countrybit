#pragma once

namespace app_model 
{
	namespace cdb = corona::database;

	class program_chart_model
	{

		cdb::dynamic_box		box;
		cdb::jschema			schema;
		cdb::relative_ptr_type	schema_id;
		cdb::jmodel				model;
		cdb::jactor				actor;
		cdb::jcollection		program_chart;

		bool					ready;
		std::string				status;

		void set_status(const char* _status, bool _ready);

	public:

		program_chart_model();
		~program_chart_model();

		std::string get_status();
		bool get_ready();
	};
}
