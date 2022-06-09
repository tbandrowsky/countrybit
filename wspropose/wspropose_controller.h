#pragma once

namespace proposal
{
	using namespace corona::win32;

	class wsproposal_controller : public corona_controller 
	{
	public:

		corona::database::relative_ptr_type idhome_class;
		corona::database::relative_ptr_type idhome;

		corona::database::relative_ptr_type idclient_root_class;
		corona::database::relative_ptr_type idclient_root;

		corona::database::relative_ptr_type idclient_class;
		corona::database::relative_ptr_type idclient;
		corona::database::relative_ptr_type idclient_name;

		corona::database::relative_ptr_type idcarrier_root_class;
		corona::database::relative_ptr_type idcarrier_root;

		corona::database::relative_ptr_type idcarrier_class;
		corona::database::relative_ptr_type idcarrier;
		corona::database::relative_ptr_type idcarrier_name;

		corona::database::relative_ptr_type idcoverage_root_class;
		corona::database::relative_ptr_type idcoverage_root;

		corona::database::relative_ptr_type idcoverage_class;
		corona::database::relative_ptr_type idcoverage;
		corona::database::relative_ptr_type idcoverage_name;

		corona::database::relative_ptr_type idprogram_class;
		corona::database::relative_ptr_type idprogram;
		corona::database::relative_ptr_type idprogram_name;
		corona::database::relative_ptr_type idprogram_description;
		corona::database::relative_ptr_type idprogram_view;

		corona::database::relative_ptr_type idproperty_list;
		corona::database::relative_ptr_type idwc_list;
		corona::database::relative_ptr_type idvehicle_list;
		corona::database::relative_ptr_type idaircraft_list;

		corona::database::relative_ptr_type idpolicy_property_class;
		corona::database::relative_ptr_type idpolicy_wc_class;
		corona::database::relative_ptr_type idpolicy_vehicles_class;
		corona::database::relative_ptr_type idpolicy_aircraft_class;
		corona::database::relative_ptr_type idpolicy_umbrella_class;

		corona::database::relative_ptr_type idpolicy;
		corona::database::relative_ptr_type idpolicy_name;
		corona::database::relative_ptr_type idinception;
		corona::database::relative_ptr_type idexpiration;
		corona::database::relative_ptr_type idstatus;
		corona::database::relative_ptr_type idattachment;
		corona::database::relative_ptr_type idlimit;
		corona::database::relative_ptr_type iddeductible;
		corona::database::relative_ptr_type idshare;

		corona::database::relative_ptr_type idpolicy_coverage_class;

		corona::database::relative_ptr_type idslide_heading1;
		corona::database::relative_ptr_type idslide_heading2;

		corona::database::relative_ptr_type idslide_title_class;
		corona::database::relative_ptr_type idslide_program_chart_class;
		corona::database::relative_ptr_type idslide_demo_cart_class;

		corona::database::relative_ptr_type idcomment;

		wsproposal_controller();
		virtual ~wsproposal_controller();

		void render_client_view(const rectangle& newSize);
		void render_carrier_view(const rectangle& newSize);
		void render_coverage_view(const rectangle& newSize);
		void render_program_view(const rectangle& newSize);

		virtual void render(const rectangle& newSize);
	};

}
