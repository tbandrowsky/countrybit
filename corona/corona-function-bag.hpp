
/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
A bag of std::function each of which with their own types.
Notes

For Future Consideration
*/

#ifndef CORONA_FUNCTION_BAG_H
#define CORONA_FUNCTION_BAG_H

namespace corona
{
	template <typename control_base> class function_bag
	{
		std::map<std::string, std::function<void(control_base* ctrl)>> functions;

	public:

		template <typename control_type> void set(std::function<void(control_type&)> _fn)
		{
			std::function<void(control_base*)> fnwrap = [](control_base *_ctrl)
			{
				auto ptr = dynamic_cast<control_type>(_ctrl);
				if (ptr) {
					_fn(*ptr);
				}
			};

			std::string fn_type_name = typeid(control_type).name();
			functions.insert_or_assign(fn_type_name, fnwrap);
		}

		template <typename control_type> void call(control_type& _ctrl)
		{
			std::string fn_type_name = typeid(control_type).name();
			if (functions.contains(fn_type_name)) {
				auto fx = functions[fn_type_name];
				fx(&_ctrl);
			}
		}
	};

}

#endif

