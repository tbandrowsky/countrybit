/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is the base class(es) for a control in a presentation.

Notes

For Future Consideration
*/

#ifndef CORONA_PRESENTATION_CONTROLS_CALICO_H
#define CORONA_PRESENTATION_CONTROLS_CALICO_H

namespace corona
{

	struct corona_button_onclick_options
	{
		corona_client*					corona_client;
		std::string						function_name;
		json							function_data;
		json							credentials;

		corona_button_onclick_options()
		{
			corona_client = nullptr;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("function_name", function_name);
			_dest.put_member("function_data", function_data);
			_dest.put_member("credentials", credentials);
		}

		virtual void put_json(json& _src)
		{
			if (!_src.has_members({ "function_name", "function_data", "credentials" }))
			{
				std::cout << "corona button must have function_name, function_data, and credentials" << std::endl;
				std::cout << "function_data can be an explicit json," << std::endl;
				std::cout << "				an id of a control," << std::endl;
				std::cout << "				form_parent," << std::endl;
			}
			function_name = _src["function_name"];
			function_data = _src["function_data"];
			credentials = _src["credentials"];
		}
	};

	class corona_button_control : public pushbutton_control
	{
	public:
		using control_base::id;

		std::shared_ptr<call_status>	status;
		corona_button_onclick_options	options;

		using windows_control::enable;
		using windows_control::disable;

		corona_button_control(container_control_base* _parent, int _id) : pushbutton_control(_parent, _id)
		{
			init();
		}

		corona_button_control(const corona_button_control& _src) : pushbutton_control(_src) 
		{
			init();
			status = _src.status;
			options = _src.options;
		}

		virtual ~corona_button_control() { ; }

		void init();
		virtual double get_font_size() { return text_style.fontSize; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

		virtual void get_json(json& _dest)
		{
			pushbutton_control::get_json(_dest);
			options.get_json(_dest);
		}
		virtual void put_json(json& _src)
		{
			pushbutton_control::put_json(_src);
			options.put_json(_src);
		}
	};

	void corona_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		_page->on_command(this->id, [this, _presentation, _page](command_event evt)
			{
				json_parser jp;
				json data;
				if (options.function_data.is_object())
				{
					std::string from_data = options.function_data.get_member("from");
					if (!from_data.empty())
					{
						if (std::isdigit(from_data[0])) {
							int64_t control_id = (int64_t)options.function_data["from"];
							control_base* fvc = this->find(control_id);
							data = fvc->get_data();
						}
						else if (from_data == "form")
						{
							for (control_base* p = dynamic_cast<control_base*>(parent); p; p = dynamic_cast<control_base*>(p->parent)) {
								form_control* fc = dynamic_cast<form_control*>(p);
								if (fc) {
									data = fc->get_data();
								}
							}
						}
					}
					else
					{
						data = options.function_data;
					}
				}
				if (options.corona_client) {
					options.corona_client->general_post_thread(id, options.function_name, options.credentials, data);
				}
			});
	}

	void corona_button_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(100.0_px, 30.0_px);
	}
}

#endif
