
#pragma once


#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

namespace proposal
{

	using namespace corona::database;

	wsproposal_controller::wsproposal_controller() : corona_controller()
	{
		/*

		In this example we create our schema and data store locally.
		In a remote application, we will wind up fetching and binding but that will be for a different example.

		*/

		/* first we initialize our box.  Our box can grow, but we start with a size of 16MB, which is more than enough
		* for demo purposes.
		*/

		box.init(1 << 23);

		/* with our box, we can then create a schema.  We'll reserve space to create roughly 75 classes,
		* and also include standard fields and classes.
		*/

		schema = jschema::create_schema(&box, 500, true, schema_id);

		/*
		Now, we can begin to flesh out schema for this application.  In Corona, a schema is a collection of fields,
		that are composed into classes.  There are many different types of fields, and only some are used below.
		Some C++ syntactical sugar makes the process less painful.  Note that the id of each field returned goes
		into a variable with the idf naming convention.
		*/

		put_class_request pcr;
		put_object_field_request porf;

		/*
		These fields are all custom integer fields that we will use to represent primary keys.
		Primary keys in Corona are like a cross between sequences and auto number.  The field itself
		is just an integer, but it will be assigned as a PK as part of its class membership.
		*/


		idf_home = schema.put_integer_field({ {  jtype::type_int64, "home_id", "Home Id", false, true }, { 0, INT64_MAX } });
		idf_client_root = schema.put_integer_field({ {  jtype::type_int64, "client_root", "Clients", false, true }, { 0, INT64_MAX } });
		idf_client = schema.put_integer_field({ {  jtype::type_int64, "client_id", "Client Id", false, true }, { 0, INT64_MAX } });
		idf_carrier_root = schema.put_integer_field({ {  jtype::type_int64, "carrier_root", "Carriers", false, true }, { 0, INT64_MAX } });
		idf_carrier = schema.put_integer_field({ {  jtype::type_int64, "carrier_id", "Carrier Id", false, true }, { 0, INT64_MAX } });
		idf_product_root = schema.put_integer_field({ {  jtype::type_int64, "product_root", "Products", false }, { 0, INT64_MAX } });
		idf_product = schema.put_integer_field({ {  jtype::type_int64, "product", "product_root", false, true }, { 0, INT64_MAX } });
		idf_coverage_root = schema.put_integer_field({ {  jtype::type_int64, "coverage_root", "Coverages", false, true }, { 0, INT64_MAX } });
		idf_coverage = schema.put_integer_field({ {  jtype::type_int64, "coverage_id", "Coverage Id", false, true }, { 0, INT64_MAX } });
		idf_system_root = schema.put_integer_field({ {  jtype::type_int64, "system_root", "System Settings", false, true }, { 0, INT64_MAX } });

		idf_carrier_name = schema.put_string_field({ {  jtype::type_string, "carrier_name", "Carrier Name", true }, { 100, "", "" } });
		idf_coverage_name = schema.put_string_field({ {  jtype::type_string, "coverage_name", "Coverage Name", true }, { 100, "", "" } });
		idf_prompt = schema.put_string_field({ {  jtype::type_string, "prompt_name", "Prompt/Label", true }, { 100, "", "" } });

		idf_product = schema.put_integer_field({ {  jtype::type_int64, "product_id", "Product Id", false, true }, { 0, INT64_MAX } });
		idf_product_code = schema.put_string_field({ {  jtype::type_string, "product_code", "Code", true }, { 100, "", "" } });
		idf_product_status = schema.put_string_field({ {  jtype::type_string, "product_status", "Status", true }, { 100, "", "" } });
		idf_product_edition = schema.put_string_field({ {  jtype::type_string, "product_edition", "Edition", true }, { 100, "", "" } });
		idf_product_template_type = schema.put_string_field({ {  jtype::type_string, "product_template_type", "Template Type", true }, { 100, "", "" } });
		idf_product_line_of_business = schema.put_string_field({ {  jtype::type_string, "product_line_of_business", "Line of Business", true }, { 100, "", "" } });
		idf_product_carrier = schema.put_string_field({ {  jtype::type_string, "product_carrier", "Carrier", true }, { 100, "", "" } });
		idf_product_updated_by = schema.put_string_field({ {  jtype::type_string, "product_updated_by", "Updated By", true }, { 100, "", "" } });
		idf_product_updated_date = schema.put_string_field({ {  jtype::type_string, "product_updated_date", "Updated Date", true }, { 100, "", "" } });
		idf_product_view = schema.put_integer_field({ {  jtype::type_int64, "product_id", "Product Id", false }, { 0, INT64_MAX } });

		idf_product_header = schema.put_integer_field({ {  jtype::type_int64, "product_header_id", "Product Header", false, true }, { 0, INT64_MAX } });
		idf_product_program_header = schema.put_integer_field({ {  jtype::type_int64, "program_header_id", "Program Header", false, true }, { 0, INT64_MAX } });
		idf_product_program_structure = schema.put_integer_field({ {  jtype::type_int64, "program_structure_id", "Program Structure", false, true }, { 0, INT64_MAX } });
		idf_product_coverage_header = schema.put_integer_field({ {  jtype::type_int64, "coverage_header_id", "Coverage Header", false, true }, { 0, INT64_MAX } });
		idf_product_coverage_structure = schema.put_integer_field({ {  jtype::type_int64, "coverage_structure_id", "Coverage Structure", false, true }, { 0, INT64_MAX } });
	
		idf_product_psi_base = schema.put_integer_field({ {  jtype::type_int64, "psi_base_id", "Product Structure Item Id", false }, { 0, INT64_MAX } });
		idf_product_phi_base = schema.put_integer_field({ {  jtype::type_int64, "phi_base_id", "Product Header Item Id", false }, { 0, INT64_MAX } });
		idf_product_csi_base = schema.put_integer_field({ {  jtype::type_int64, "csi_base_id", "Coverage Structure Item Id", false }, { 0, INT64_MAX } });
		idf_product_chi_base = schema.put_integer_field({ {  jtype::type_int64, "chi_base_id", "Coverage Header Item Id", false }, { 0, INT64_MAX } });

		/*
		* Here we are going to specify a set of fields that may be used to create a program item.
		* A model in Corona is a way to tie together objects that are affiliated but necessarily members per se, although syntactical
		* sugar will be introduced to make it -seem- that way.
		*/

		idf_inception = schema.put_time_field({ {  jtype::type_datetime, "inception", "Inception" }, { 0, INT64_MAX } });
		idf_expiration = schema.put_time_field({ {  jtype::type_datetime, "expiration", "Expiration" }, { 0, INT64_MAX } });
		idf_status = schema.put_integer_field({ {  jtype::type_int32, "status", "Status" }, { 0, INT64_MAX } });
		idf_attachment = schema.put_double_field({ {  jtype::type_float32, "attachment", "Attachment" }, { 0.0, 1E10 } });
		idf_limit = schema.put_double_field({ {  jtype::type_float32, "limit", "Limit" }, { 0.0, 1E10 } });
		idf_deductible = schema.put_double_field({ {  jtype::type_float32, "deductible", "Deductible" }, { 0.0, 1E10 } });
		idf_share = schema.put_double_field({ {  jtype::type_float32, "share", "Share %" }, { 0.0, 100 } });
		idf_comment = schema.put_string_field({ {  jtype::type_string, "comment", "Comment" }, { 500, "", "", } });

		/*
		Now that we have our fields, we can make our classes.
		*/

		/*
		We have a class object to represent our application states and objects with respect to this model.  We supply a class name, description, a list
		of member fields, and then, a primary key.  We call put_class to upsert the class, and the class id is returned for easy reference.
		The schema can also be searched by the class_name to find the class.
		*/

		/* This is the application home class.  It is just a place holder now */
		pcr.class_name = "home";
		pcr.class_description = "HOME";
		pcr.field_id_primary_key = idf_home;
		pcr.member_fields = { idf_home, schema.idf_style_id };
		idc_home = schema.put_class(pcr);

		/* This is the client root class. Objects of the class have a layout and accept the client home id as a relation.  There is also
		a standard search string field, for searching for clients and keeping the search state. */
		pcr.class_name = "client_root";
		pcr.class_description = "Client";
		pcr.field_id_primary_key = idf_client_root;
		pcr.member_fields = { idf_client_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_client_root = schema.put_class(pcr);

		/* This is the client class. Objects of the class have client things like name and address.  The primary key is the
		field id idf_client, which is populated when objects of this class are constructed. . */

		pcr.class_name = "client";
		pcr.class_description = "Client";
		pcr.field_id_primary_key = idf_client;
		pcr.member_fields = { idf_client, idf_client_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_client = schema.put_class(pcr);

		/* This is the carrier root class. Objects of the class have a layout and accept the idf_home as a relation and have a primary key
		that is auto-opulated at object load time.  There is also
			a standard search string field, for searching for clients and keeping the search state. */

		pcr.class_name = "carrier_root";
		pcr.class_description = "Carriers";
		pcr.field_id_primary_key = idf_carrier_root;
		pcr.member_fields = { idf_carrier_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_carrier_root = schema.put_class(pcr);

		/* This is the carrier class. Objects of the class have client things like name and address.  The primary key is the
field id idf_carrier, which is populated when objects of this class are constructed. . */

		pcr.class_name = "carrier";
		pcr.class_description = "Carrier";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_carrier, idf_carrier_root, schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal, schema.idf_email, schema.idf_url };
		idc_carrier = schema.put_class(pcr);

		pcr.class_name = "coverage_root";
		pcr.class_description = "Coverages";
		pcr.field_id_primary_key = idf_coverage_root;
		pcr.member_fields = { idf_coverage_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_coverage_root = schema.put_class(pcr);

		pcr.class_name = "coverage";
		pcr.class_description = "Coverage";
		pcr.field_id_primary_key = idf_carrier;
		pcr.member_fields = { idf_coverage, idf_coverage_root, schema.idf_name };
		idc_coverage = schema.put_class(pcr);

		pcr.class_name = "system_root";
		pcr.class_description = "System";
		pcr.field_id_primary_key = idf_system_root;
		pcr.member_fields = { idf_system_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_system_root = schema.put_class(pcr);

		pcr.class_name = "products";
		pcr.class_description = "Products";
		pcr.field_id_primary_key = idf_product_root;
		pcr.member_fields = { idf_product_root, idf_home, schema.idf_search_string, schema.idf_style_id };
		idc_product_root = schema.put_class(pcr);

		pcr.class_name = "product";
		pcr.class_description = "Product";
		pcr.field_id_primary_key = idf_product;
		pcr.member_fields = { idf_product_root,
							  idf_product };
		idc_product = schema.put_class(pcr);

		pcr.class_name = "product_header";
		pcr.class_description = "Product Header";
		pcr.field_id_primary_key = idf_product_header;
		pcr.member_fields = { idf_product_header, idf_product, schema.idf_name,
							  idf_product_code,
							  idf_product_status,
							  idf_product_edition,
							  idf_product_template_type,
							  idf_product_line_of_business,
							  idf_product_carrier,
							  idf_product_view,
							  idf_product_updated_by,
							  idf_product_updated_date };
		idc_product_header = schema.put_class(pcr);

		pcr.class_name = "product_program_header";
		pcr.class_description = "Program Header";
		pcr.field_id_primary_key = idf_product_program_header;
		pcr.member_fields = { idf_product_program_header, idf_product };
		idc_product_program_header = schema.put_class(pcr);

		pcr.class_name = "product_program_structure";
		pcr.class_description = "Program Structure";
		pcr.field_id_primary_key = idf_product_program_structure;
		pcr.member_fields = { idf_product_program_structure, idf_product };
		idc_product_program_structure = schema.put_class(pcr);

		pcr.class_name = "product_coverage_header";
		pcr.class_description = "Coverage Header";
		pcr.field_id_primary_key = idf_product_coverage_header;
		pcr.member_fields = { idf_product_coverage_header, idf_product };
		idc_product_coverage_header = schema.put_class(pcr);

		pcr.class_name = "product_coverage_structure";
		pcr.class_description = "Coverage Structure";
		pcr.field_id_primary_key = idf_product_coverage_structure;
		pcr.member_fields = { idf_product_coverage_structure, idf_product  };
		idc_product_coverage_structure = schema.put_class(pcr);

		pcr.class_name = "product_phi_base";
		pcr.class_description = "Program Header Item";
		pcr.field_id_primary_key = idf_product_phi_base;
		pcr.member_fields = { idf_product_phi_base, idf_product_program_header, idf_product, schema.idf_name, schema.idf_field_description };
		idc_product_phi_base = schema.put_class(pcr);

		pcr.class_name = "product_psi_base";
		pcr.class_description = "Program Structure Item";
		pcr.field_id_primary_key = idf_product_psi_base;
		pcr.member_fields = { idf_product_psi_base, idf_product_program_structure, idf_product, schema.idf_name, schema.idf_field_description };
		idc_product_psi_base = schema.put_class(pcr);

		pcr.class_name = "product_chi_base";
		pcr.class_description = "Coverage Header Item";
		pcr.field_id_primary_key = idf_product_chi_base;
		pcr.member_fields = { idf_product_chi_base, idf_product_coverage_header, idf_product, schema.idf_name, schema.idf_field_description };
		idc_product_chi_base = schema.put_class(pcr);

		pcr.class_name = "product_csi_base";
		pcr.class_description = "Coverage Structure Item";
		pcr.field_id_primary_key = idf_product_csi_base;
		pcr.member_fields = { idf_product_csi_base, idf_product_coverage_structure, idf_product, schema.idf_name, schema.idf_field_description };
		idc_product_csi_base = schema.put_class(pcr);

		relative_ptr_type bases[4] = { idc_product_phi_base, idc_product_psi_base, idc_product_chi_base, idc_product_csi_base  };
		relative_ptr_type pks[4] = { idf_product_phi_base, idf_product_psi_base, idf_product_chi_base, idf_product_csi_base };
		const char* base_names[4] = { "product_phi", "product_psi", "product_chi", "product_csi" };
		const char* item_descriptions[4] = { "Text Field", "Number Field", "Date Field", "Image Field" };
		const char* item_names[4] = { "_text", "_number", "_date", "_image" };

		for (int i = 0; i < 4; i++)
		{
			auto bid = bases[i];
			object_name name;
			name = base_names[i];
			for (int j = 0; j < 4; j++)
			{
				name += item_names[j];
				pcr.class_name = name;
				pcr.class_description = name;
				pcr.field_id_primary_key = pks[i];
				pcr.member_fields = { idf_prompt };
				pcr.base_class_id = bases[i];
				schema.put_class(pcr);
			}
		}

		jmodel jm;

		jm.update_when(&schema, {}, idc_home, {});
		jm.update_when(&schema, {}, idc_carrier_root, {});
		jm.update_when(&schema, {}, idc_carrier, {});
		jm.update_when(&schema, {}, idc_product_root, {});
		jm.update_when(&schema, {}, idc_product, {});
		jm.update_when(&schema, {}, idc_product_header, {});
		jm.update_when(&schema, {}, idc_product_program_header, {});
		jm.update_when(&schema, {}, idc_product_program_structure, {});
		jm.update_when(&schema, {}, idc_product_coverage_header, {});
		jm.update_when(&schema, {}, idc_product_coverage_structure, {});
		jm.update_when(&schema, {}, idc_product_phi_base, {});
		jm.update_when(&schema, {}, idc_product_chi_base, {});
		jm.update_when(&schema, {}, idc_product_psi_base, {});
		jm.update_when(&schema, {}, idc_product_csi_base, {});
		jm.update_when(&schema, {}, idc_coverage_root, {});
		jm.update_when(&schema, {}, idc_coverage, {});
		jm.update_when(&schema, {}, idc_client_root, {});
		jm.update_when(&schema, {}, idc_client, {});
		jm.update_when(&schema, {}, idc_system_root, {});

		vq_navigation.classes = { idc_home, idc_carrier_root, idc_product_root, idc_coverage_root, idc_client_root, idc_system_root };
		vq_navigation.query_name = "navigation";

		vq_carriers.classes = { idc_carrier };
		vq_carriers.query_name = "carriers";

		vq_products.classes = { idc_product };
		vq_products.query_name = "products";

		vq_clients.classes = { idc_client };
		vq_clients.query_name = "clients";

		vq_coverages.classes = { idc_coverage };
		vq_coverages.query_name = "coverages";

		vq_coverages.classes = { idc_product_header, idc_product_program_header, idc_product_program_structure, idc_product_coverage_header, idc_product_coverage_structure };
		vq_coverages.query_name = "product_header";

		view_options vo_home;
		vo_home.use_view = true;
		vo_home.view_class_id = idc_home;
		vo_home.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_home, {}, {}, vo_home);

		view_options vo_carrier_root;
		vo_carrier_root.use_view = true;
		vo_carrier_root.view_class_id = idc_carrier_root;
		vo_carrier_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_home }, idc_carrier_root, {}, {}, vo_carrier_root);

		view_options vo_carrier;
		vo_carrier.use_view = true;
		vo_carrier.view_class_id = idc_carrier;
		vo_carrier.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_carrier_root }, idc_carrier, {}, {}, vo_carrier);

		view_options vo_product_root;
		vo_product_root.use_view = true;
		vo_product_root.view_class_id = idc_product_root;
		vo_product_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, { idc_home }, idc_product_root, {}, {}, vo_product_root);

		view_options vo_product;
		vo_product.use_view = true;
		vo_product.view_class_id = idc_product;
		vo_product.view_queries.push_back(vq_navigation);
		vo_product.view_queries.push_back(vq_product_header);
		jm.select_when(&schema, { idc_product_root }, idc_product, {}, {}, {});

		jm.select_when(&schema, { idc_product }, idc_product_header, {}, {}, {});
		jm.select_when(&schema, { idc_product }, idc_product_program_header, {}, {}, {});
		jm.select_when(&schema, { idc_product }, idc_product_program_structure, {}, {}, {});
		jm.select_when(&schema, { idc_product }, idc_product_coverage_header, {}, {}, {});
		jm.select_when(&schema, { idc_product }, idc_product_coverage_structure, {}, {}, {});
		jm.select_when(&schema, {}, idc_product_phi_base, {}, {}, {});
		jm.select_when(&schema, {}, idc_product_chi_base, {}, {}, {});
		jm.select_when(&schema, {}, idc_product_psi_base, {}, {}, {});
		jm.select_when(&schema, {}, idc_product_csi_base, {}, {}, {});

		view_options vo_coverage_root;
		vo_coverage_root.use_view = true;
		vo_coverage_root.view_class_id = idc_coverage_root;
		vo_coverage_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_coverage_root, {}, {}, vo_coverage_root);

		view_options vo_coverage;
		vo_coverage.use_view = true;
		vo_coverage.view_class_id = idc_coverage;
		vo_coverage.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_coverage, {}, {}, vo_coverage);

		view_options vo_client_root;
		vo_client_root.use_view = true;
		vo_client_root.view_class_id = idc_client_root;
		vo_client_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_client_root, {}, {}, vo_client_root);

		view_options vo_client;
		vo_client.use_view = true;
		vo_client.view_class_id = idc_client;
		vo_client.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_client, {}, {}, vo_client);

		view_options vo_system_root;
		vo_product_root.use_view = true;
		vo_product_root.view_class_id = idc_system_root;
		vo_product_root.view_queries.push_back(vq_navigation);
		jm.select_when(&schema, {}, idc_system_root, {}, {}, {});

		jm.delete_when(&schema, {}, idc_carrier, {});
		jm.delete_when(&schema, {}, idc_product, {});
		jm.delete_when(&schema, {}, idc_product_program_header, {});
		jm.delete_when(&schema, {}, idc_product_program_structure, {});
		jm.delete_when(&schema, {}, idc_product_coverage_header, {});
		jm.delete_when(&schema, {}, idc_product_coverage_structure, {});
		jm.delete_when(&schema, {}, idc_product_phi_base, {});
		jm.delete_when(&schema, {}, idc_product_chi_base, {});
		jm.delete_when(&schema, {}, idc_product_psi_base, {});
		jm.delete_when(&schema, {}, idc_product_csi_base, {});
		jm.delete_when(&schema, {}, idc_coverage, {});
		jm.delete_when(&schema, {}, idc_client, {});

		jm.create_when(&schema, { }, idc_home, null_row, true, false, 0, { idc_carrier_root, idc_coverage_root, idc_client_root, idc_product_root });
		jm.create_when(&schema, { idc_home }, idc_carrier_root, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_home }, idc_coverage_root, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_home }, idc_client_root, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_home }, idc_product_root, null_row, true, false, 0, {  });
		jm.create_when(&schema, { idc_carrier_root }, idc_carrier, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_coverage_root }, idc_coverage, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_client_root }, idc_client, null_row, true, false, 0, {});
		jm.create_when(&schema, { idc_product_root }, idc_product, null_row, true, false, 0, 
			{ 
			idc_product_program_header, 
			idc_product_program_structure, 
			idc_product_coverage_header, 
			idc_product_coverage_structure });

		jm.create_when(&schema, { idc_product }, idc_product_program_header, null_row, false, false, 1, {});
		jm.create_when(&schema, { idc_product }, idc_product_program_structure, null_row, false, false, 1, {});
		jm.create_when(&schema, { idc_product }, idc_product_coverage_header, null_row, false, false, 1, {});
		jm.create_when(&schema, { idc_product }, idc_product_coverage_structure, null_row, false, false, 1, {});
		jm.create_when(&schema, { idc_product_program_header }, idc_product_phi_base, null_row, false, false, 0, {});
		jm.create_when(&schema, { idc_product_program_structure }, idc_product_psi_base, null_row, false, false, 0, {});
		jm.create_when(&schema, { idc_product_coverage_header }, idc_product_chi_base, null_row, false, false, 0, {});
		jm.create_when(&schema, { idc_product_coverage_structure }, idc_product_csi_base, null_row, false, false, 0, {});

		schema.put_model(jm);

		jcollection_ref ref;
		ref.data = &box;
		ref.model_name = jm.name;
		ref.max_actors = 2;
		ref.max_objects = 100;
		ref.collection_size_bytes = 1 << 19;
		ref.create_style_sheet = true;

		if (!init_collection_id(ref.collection_id))
		{
			std::cout << __LINE__ << "collection id failed" << std::endl;
		}

		user_collection = schema.create_collection(&ref);

		jactor sample_actor;
		sample_actor.actor_name = "sample actor";
		sample_actor.actor_id = null_row;
		sample_actor = user_collection.create_actor(sample_actor);

		actor_id = sample_actor.actor_id;

		set_style_sheet();

		create_object_request cor;

		cor.actor_id = actor_id;
		cor.class_id = idc_home;
		cor.collection_id = user_collection.get_collection_id();
		cor.item_id = -1;
		cor.select_on_create = true;
		cor.template_item_id = -1;

		// create our initial object, and get our state.
		state = user_collection.create_object(cor, "create home");

	}

	wsproposal_controller::~wsproposal_controller()
	{
		;
	}

	void wsproposal_controller::render_navigation_frame(std::function<void(page_item* navigation_contents, page_item* _frame)> _contents)
	{
		clear();

		const char* _title = application_title;
		const char* _subtitle = application_author;

		auto page_column = column(nullptr, null_row, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });

		auto title_bar = canvas2d_row(id_canvas_header, page_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 45.0_px });
		text(title_bar, schema.idf_album_title_style, _title);

		auto main_row = row(page_column, null_row, { 0.0_px, 15.0_px, 100.0_pct, 100.0_pct });
		auto navigation_contents = canvas2d_column(id_canvas_navigation, main_row, null_row, { 0.0_px, 0.0_px, 200.0_px, 100.0_pct });
		breadcrumbs(navigation_contents, [this](jobject& _item) {
			return _item.get_name(schema.idf_name);
			}, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });

		std::vector<relative_ptr_type> navigation_objects = {idc_client_root, idc_product_root, idc_carrier_root, idc_coverage_root, idc_system_root};
		//selects_by_class(navigation_contents, schema.idf_navigation_style, schema.idf_navigation_selected_style, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px }, schema.idf_name, navigation_objects);

		auto form_contents = column(main_row, null_row, { 16.0_px, 0.0_px, 100.0_pct, 100.0_pct });
		_contents(navigation_contents, form_contents);

		auto footer_bar = canvas2d_row(id_canvas_footer, page_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
		text(footer_bar, schema.idf_artist_title_style, _subtitle);
	}

	void wsproposal_controller::render_form(page_item* _navigation, page_item *_frame, const char *_form_title)
	{
		_frame->windowsRegion = true;
		add_update_fields(_frame, field_layout::label_on_left, _form_title);
		space(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		text(_navigation, schema.idf_label_style, "Create", {0.0_px, 0.0_px, 100.0_pct, 32.0_px});
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wsproposal_controller::render_search_page(page_item* _navigation, page_item* _contents, relative_ptr_type _class_id, const char* _form_title, int count_fields, relative_ptr_type *_field_ids)
	{
		auto form_search = row(_contents, null_row, { 0.0_px, 0.0_px, 100.0_pct, 25.0_px });
		add_update_fields(form_search, field_layout::label_on_left, _form_title);
		auto form_table = canvas2d_column(id_canvas_form_table_a, _contents, schema.idf_view_background_style);
		search_table(form_table, _class_id, _field_ids, count_fields);
		text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wsproposal_controller::render_home()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_home_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_client_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_client_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_client()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_client_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_coverage_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_coverage_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_coverage()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_coverage_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_product_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_product_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_product()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_product_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_carrier_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_carrier_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_system_root()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_system_root_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_carrier()
	{
		render_navigation_frame([this](page_item* _navigation, page_item* _contents) { render_carrier_contents(_navigation, _contents);  });
	}

	void wsproposal_controller::render_home_contents(page_item* _navigation, page_item* _contents)
	{
	}

	void wsproposal_controller::render_client_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		render_search_page(_navigation, _contents, idc_client, "Clients", 5, field_ids);
	}

	void wsproposal_controller::render_client_contents(page_item* _navigation, page_item* _contents)
	{
		const char* object_title = nullptr;
		object_title = schema.get_class(state.actor.view.view_class_id).item().description;

		auto edit_body = row(_contents, null_row, { 0.0_px,0.0_px,100.0_pct,100.0_pct });

		auto control = column(edit_body, schema.idf_view_background_style, { 0.0_pct, 0.0_px, 30.0_pct, 100.0_pct });
		auto children = canvas2d_column(id_canvas_form_table_a, edit_body, schema.idf_view_background_style, { 0.0_px, 0.0_px, 65.0_pct, 100.0_pct });

		add_update_fields(control, field_layout::label_on_top, "Client Details");
		space(control, schema.idf_button_style, { 0.0_px, 0.0_px, 1.0_fntgr, 1.0_fntgr });

		add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
	}

	void wsproposal_controller::render_coverage_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[1] = { schema.idf_name };
		render_search_page(_navigation, _contents, idc_coverage, "Coverages", 1, field_ids);
	}

	void wsproposal_controller::render_system_root_contents(page_item* _navigation, page_item* _contents)
	{

		}

	void wsproposal_controller::render_coverage_contents(page_item* _navigation, page_item* _contents)
	{
		render_form(_navigation, _contents, "Coverage");
	}

	void wsproposal_controller::render_product_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[9] = { 
							  schema.idf_name,
							  idf_product_code,
							  idf_product_status,
							  idf_product_edition,
							  idf_product_template_type,
							  idf_product_line_of_business,
							  idf_product_carrier,
							  idf_product_updated_by,
							  idf_product_updated_date };
		render_search_page(_navigation, _contents, idc_product, "Products", 9, field_ids);
	}

	void wsproposal_controller::render_product_contents(page_item* _navigation, page_item* _contents)
	{
		if (state.actor.current_subview_class_id == -1) 
		{
			auto content_column = column( _contents, null_row, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			auto tab_row = canvas2d_row(id_canvas_products_a, content_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
			relative_ptr_type product_objects[5] = { idc_product, idc_product_program_header, idc_product_program_structure, idc_product_coverage_header, idc_product_coverage_structure };
//			selects(tab_row, schema.idf_navigation_style, schema.idf_navigation_selected_style, { 0.0_px, 0.0_px, 150.0_px, 30.0_px }, schema.idf_name, product_objects, 5);
			render_form(_navigation, content_column, "Product");
		}
		else
		{
			auto content_column = canvas2d_column(id_canvas_products_b, _contents, null_row, { 0.0_px, 0.0_px, 100.0_pct, 90.0_pct });
			auto tab_row = row(content_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
			relative_ptr_type product_objects[5] = { idc_product, idc_product_program_header, idc_product_program_structure, idc_product_coverage_header, idc_product_coverage_structure };
	//		selects(tab_row, schema.idf_navigation_style, schema.idf_navigation_selected_style, { 0.0_px, 0.0_px, 150.0_px, 30.0_px }, schema.idf_name, product_objects, 5);

			auto title_row = row(content_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 30.0_px });
			auto body_row = row(content_column, null_row, { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			relative_ptr_type search_fields[2] = {schema.idf_name, schema.idf_field_description };

			if (state.actor.current_subview_class_id == idc_product_program_header)
			{
				std::cout << "Product Program Header Clicked" << std::endl;
				text(title_row, schema.idf_label_style, "Program Header");
				search_table(body_row, idc_product_phi_base, search_fields, 2 );
			}
			else if (state.actor.current_subview_class_id == idc_product_program_structure)
			{
				std::cout << "Product Program Structure Clicked" << std::endl;
				text(title_row, schema.idf_label_style, "Program Structure");
				search_table(body_row, idc_product_psi_base, search_fields, 2);
			}
			else if (state.actor.current_subview_class_id == idc_product_coverage_header)
			{
				std::cout << "Product Coverage Header  Clicked" << std::endl;
				text(title_row, schema.idf_label_style, "Coverage Header");
				search_table(body_row, idc_product_chi_base, search_fields, 2);
			}
			else if (state.actor.current_subview_class_id == idc_product_coverage_structure)
			{
				std::cout << "Product Coverage Structure  Clicked" << std::endl;
				text(title_row, schema.idf_label_style, "Coverage Structure");
				search_table(body_row, idc_product_csi_base, search_fields, 2);
			}

			add_create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		}
	}

	void wsproposal_controller::render_carrier_root_contents(page_item* _navigation, page_item* _contents)
	{
		relative_ptr_type field_ids[5] = { schema.idf_name, schema.idf_street, schema.idf_city, schema.idf_state, schema.idf_postal };
		render_search_page(_navigation, _contents, idc_carrier, "Carriers", 5, field_ids);
	}

	void wsproposal_controller::render_carrier_contents(page_item* _navigation, page_item* _contents)
	{
		render_form(_navigation, _contents, "Carrier");
	}

	void wsproposal_controller::render(const rectangle& newSize)
	{
		if (state.actor.view.view_class_id == idc_home) 
		{
			render_home();
		} 
		else if (state.actor.view.view_class_id == idc_client_root)
		{
			render_client_root();
		}
		else if (state.actor.view.view_class_id == idc_client)
		{
			render_client();
		}
		else if (state.actor.view.view_class_id == idc_product_root)
		{
			render_product_root();
		}
		else if (state.actor.view.view_class_id == idc_product)
		{
			render_product();
		}
		else if (state.actor.view.view_class_id == idc_coverage_root)
		{
			render_coverage_root();
		}
		else if (state.actor.view.view_class_id == idc_coverage)
		{
			render_coverage();
		}
		else if (state.actor.view.view_class_id == idc_carrier_root)
		{
			render_carrier_root();
		}
		else if (state.actor.view.view_class_id == idc_carrier)
		{
			render_carrier();
		}
		else if (state.actor.view.view_class_id == idc_system_root)
		{
			render_system_root();
		}

		auto style_sheet = getStyleSheet();
		arrange(newSize.w, newSize.h, style_sheet, 16);
		canvasWindowsId = host->renderPage(pg, &schema, state, user_collection);
		host->redraw();
	}

	void wsproposal_controller::set_style_sheet()
	{
		auto style_sheet = getStyleSheet();

		const char* fontName = "Open Sans,Arial";

		style_sheet.set(
			{ schema.idf_view_background_style },
			{
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#ffffffff" }
			}
			);

		style_sheet.set(
			schema.idf_view_background_style,
			{ schema.idf_view_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#ffffffff" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_view_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_view_subtitle_style },
			{
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_bold, false },
			}
			);

		auto color_check_src = style_sheet.get_slice(schema.idf_view_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);
		auto color_check_dest = style_sheet.get_slice(schema.idf_view_subtitle_style, { 0,0,0 }, true).get(schema.idf_shape_fill_color);

		style_sheet.set(
			schema.idf_view_style, 
			{ schema.idf_view_section_style },
			{
				{ schema.idf_font_name, fontName},
				{ schema.idf_font_size, 16.0 },
				{ schema.idf_box_fill_color, "#DEE1E1FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#C1C6C8FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_disclaimer_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_copyright_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h1_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 30.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h2_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_h3_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 20.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_number_head_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#1F2A44FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#1F2A44FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_text_head_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#1F2A44FF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#1F2A44FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_number_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_far },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_text_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_column_data_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 12.0 },
				{ schema.idf_bold, false },
				{ schema.idf_italic, false },
				{ schema.idf_underline, false },
				{ schema.idf_strike_through, false },
				{ schema.idf_line_spacing, 0.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_near },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_near },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#dfdfdfFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_label_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_control_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_bold, true },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_axis_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_legend_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_chart_block_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_tooltip_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_error_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_client_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_carrier_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_coverage_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_product_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_system_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#78BE20FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_home_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_login_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_a3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_b3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_c3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d1_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d2_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_d3_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_company_deductible_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_header_area_style },
			{
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#ffffffFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "#ffffffFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_title_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_shape_fill_color, "#319B42FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 0 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_subtitle_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_breadcrumb_bar_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_breadcrumb_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#5284A3FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#DEE1E1FF" },
				{ schema.idf_box_border_thickness, 4 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_button_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#FFFFFFFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#319B42FF" },
				{ schema.idf_box_border_thickness, 2 },
				{ schema.idf_box_border_color, "#319B42FF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 40.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
				{ schema.idf_shape_fill_color, "#000000FF" },
				{ schema.idf_shape_border_thickness, 4 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_title_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 24.0 },
				{ schema.idf_horizontal_alignment, (int)(visual_alignment::align_center) },
				{ schema.idf_shape_fill_color, "#001100FF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_album_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_artist_about_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title1_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title2_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title3_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title4_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title5_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_work_title6_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_shape_fill_color, "#0000CCFF" },
				{ schema.idf_shape_border_thickness, 0 },
				{ schema.idf_shape_border_color, "" },
				{ schema.idf_box_fill_color, "#FFFFFFFF" },
				{ schema.idf_box_border_thickness, 1 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_navigation_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#BFBFBFFF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

		style_sheet.set(
			schema.idf_view_style,
			{ schema.idf_navigation_selected_style },
			{
				{ schema.idf_font_name, fontName },
				{ schema.idf_font_size, 14.0 },
				{ schema.idf_horizontal_alignment, (int)visual_alignment::align_center },
				{ schema.idf_vertical_alignment, (int)visual_alignment::align_center },
				{ schema.idf_shape_fill_color, "#ffffffFF" },
				{ schema.idf_shape_border_thickness, 8 },
				{ schema.idf_shape_border_color, "#000000FF" },
				{ schema.idf_box_fill_color, "#002060FF" },
				{ schema.idf_box_border_thickness, 8 },
				{ schema.idf_box_border_color, "#FFFFFFFF" }
			}
			);

	}

}
