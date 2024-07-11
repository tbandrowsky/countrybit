/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_BASE_H
#define CORONA_PRESENTATION_BASE_H

namespace corona
{
	class presentation;
	class page;

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
				std::cout << "list_data is missing:" << std::endl;
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					std::cout << s << std::endl;
					});
				std::cout << "source json:" << std::endl;
				std::cout << _src.to_json() << std::endl;
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
		std::string json_field;
		int width;
		visual_alignment alignment;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("display_name", display_name);
			_dest.put_member("json_field", json_field);
			_dest.put_member("width", width);
			corona::get_json(_dest, "alignment", alignment);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "display_name", "json_field", "width" })) {
				std::cout << "table_column is missing:" << std::endl;
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					std::cout << s << std::endl;
					});
				std::cout << "source json:" << std::endl;
				std::cout << _src.to_json() << std::endl;
				return;
			}


			display_name = _src["display_name"];
			json_field = _src["json_field"];
			width = (int)_src["width"];
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
				std::cout << "table_data is missing:" << std::endl;
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					std::cout << s << std::endl;
					});
				std::cout << "source json:" << std::endl;
				std::cout << _src.to_json() << std::endl;
				return;
			}

			json jcolumns = _src["columns"];

			if (jcolumns.is_array()) 
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


	class mini_table
	{
		std::shared_ptr<dynamic_box> box;
		std::vector<relative_ptr_type> indeces;
		int row_size;
		int max_rows;
	public:

		mini_table()
		{
			row_size = 1;
			max_rows = 1;
			box = std::make_shared<dynamic_box>();
		}

		void init(int _row_size, int _max_rows)
		{
			max_rows = _max_rows;
			row_size = _row_size;
			indeces.clear();
			int c = (max_rows + 1) * row_size;
			if (indeces.size() < max_rows)
			{
				indeces.resize(c);
			}
			int b = c * 100;
			box->init(b);
		}

		char* set(int i, int j, std::string& src)
		{
			int idx = j * row_size + i;

			indeces[idx] = box->put_null_terminated(src.c_str(), 0);
			char* temp = box->get_object<char>(indeces[idx]);
			return temp;
		}
	};

}

#endif
