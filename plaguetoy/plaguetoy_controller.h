#pragma once

namespace proposal
{
	using namespace corona::win32;

	class plaguetoy_controller : public corona_controller
	{
	public:

		corona::database::relative_ptr_type idc_home;
		corona::database::relative_ptr_type idf_home;

		plaguetoy_controller();
		virtual ~plaguetoy_controller();

		void render_client_view(const rectangle& newSize);
		void render_carrier_view(const rectangle& newSize);
		void render_coverage_view(const rectangle& newSize);
		void render_program_view(const rectangle& newSize);

		virtual void render(const rectangle& newSize);
	};

}
