
#ifndef CORONA_PRESENTATION_CONTROLS_CALICO_H
#define CORONA_PRESENTATION_CONTROLS_CALICO_H

namespace corona
{

	struct calico_button_onclick_options
	{
		corona_client*					corona_client;
		std::string						function_name;
		json							function_data;
		json							credentials;
	};

	class calico_button_control : public pushbutton_control
	{
	public:
		using control_base::id;

		std::shared_ptr<call_status>	status;
		calico_button_onclick_options	options;

		using windows_control::enable;
		using windows_control::disable;

		calico_button_control(container_control_base* _parent, int _id) : pushbutton_control(_parent, _id)
		{
			init();
		}

		calico_button_control(const calico_button_control& _src) : pushbutton_control(_src) 
		{
			init();
			status = _src.status;
			options = _src.options;
		}

		virtual ~calico_button_control() { ; }

		void init();
		virtual double get_font_size() { return text_style.fontSize; }

		virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

	};

	void calico_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
	{
		_page->on_command(this->id, [this, _presentation, _page](command_event evt)
			{
				json_parser jp;
				json data;
				if (options.function_data.is_object())
				{
					if (options.function_data.has_member("SourceControlId")) {
						int64_t control_id = options.function_data["SourceControlId"];
						control_base* fvc = this->find(control_id);
						data = fvc->get_data();
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

	void calico_button_control::init()
	{
		set_origin(0.0_px, 0.0_px);
		set_size(100.0_px, 30.0_px);
	}


}

#endif
