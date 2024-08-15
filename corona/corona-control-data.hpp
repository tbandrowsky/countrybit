/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_CONTROL_DATA_H
#define CORONA_CONTROL_DATA_H

namespace corona
{
	class list_data : public json_serializable
	{
	public:

		std::string id_field;
		std::string text_field;
		std::string selected_field;

		json items;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("id_field", id_field);
			_dest.put_member("text_field", text_field);
			_dest.put_member("selected_field", selected_field);
			_dest.put_member("items", items);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "id_field", "text_field", "selected_field" })) {
				system_monitoring_interface::global_mon->log_warning("list_data is missing");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			id_field = _src.get_member("id_field");
			text_field = _src.get_member("text_field");
			selected_field = _src.get_member("selected_field");
			items = _src.get_member("items");
		}

	};

	class table_column : public json_serializable
	{
	public:
		std::string display_name;
		std::string json_field_name;
		std::string format;
		int width;
		visual_alignment alignment;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("label_text", display_name);
			_dest.put_member("json_field_name", json_field_name);
			_dest.put_member("width", width);
			_dest.put_member("format", format);
			corona::get_json(_dest, "alignment", alignment);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "label_text", "json_field_name", "width" })) {
				system_monitoring_interface::global_mon->log_warning("table_column is missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			display_name = _src["label_text"];
			json_field_name = _src["json_field_name"];
			width = (int)_src["width"];
			format = _src["format"];
			corona::put_json(alignment, _src, "alignment");
		}

	};

	class table_data : public json_serializable
	{
	public:
		std::vector<table_column> columns;
		std::string id_field;
		json items;

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			json jcolumns = jp.create_array();

			for (auto tc : columns)
			{
				json tcitem = jp.create_object();
				tc.get_json(tcitem);
				jcolumns.push_back(tcitem);
			}

			_dest.put_member("id_field", id_field);
			_dest.put_member("items", items);
			_dest.put_member("columns", jcolumns);
		}

		virtual void put_json(json& _src)
		{

			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "columns", "id_field", "items" })) {
				system_monitoring_interface::global_mon->log_warning("table_data missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			json jcolumns = _src["columns"];

			if (jcolumns.array()) 
			{
				for (auto jtc : jcolumns)
				{
					table_column tc;
					tc.put_json(jtc);
					columns.push_back(tc);
				}
			}

			id_field = _src["id_field"];
			items = _src["items"];
		}

	};

	class id_counter
	{
	public:
		static int id;
		static int next();
		static int check(int _id);
		static int status_bar_id;
		static int status_text_title_id;
		static int status_text_subtitle_id;
	};

	class layout_context
	{
	public:
		point flow_origin;
		point container_origin;
		point container_size;
		point remaining_size;
		point space_amount;
	};
}

#endif
