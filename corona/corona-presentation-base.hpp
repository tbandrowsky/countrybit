#ifndef CORONA_PRESENTATION_BASE_H
#define CORONA_PRESENTATION_BASE_H

namespace corona
{
	class presentation;
	class page;

	presentation_style_factory styles;

	class list_data
	{
	public:

		std::string id_field;
		std::string text_field;
		std::string selected_field;

		json items;
	};

	class table_column
	{
	public:
		std::string display_name;
		std::string json_field;
		int width;
		visual_alignment alignment;
	};

	class table_data
	{
	public:
		std::vector<table_column> columns;
		std::string id_field;
		json items;
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
