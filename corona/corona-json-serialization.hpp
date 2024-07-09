/*
*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024+ All Rights Reserved

About this File
This is the main header file.
It was structured to facilitate C++ modules but there
is much work to be done, and at the time, there were issues
with the compiler.

Notes

For Future Consideration

*/

#ifndef CORONA_JSON_SERIALIZATION_H
#define CORONA_JSON_SERIALIZATION_H

namespace corona
{
	void get_json(json& _dest, point& _src)
	{
		_dest.put_member("x", _src.x);
		_dest.put_member("y", _src.y);
		_dest.put_member("z", _src.z);
	}

	void put_json(point& _dest, json& _src)
	{
		_dest.x = (double)_src["x"];
		_dest.y = (double)_src["y"];
		_dest.z = (double)_src["z"];
	}

	void get_json(json& _dest, measure _src)
	{
		_dest.put_member("amount", _src.amount);
		switch (_src.units)
		{
		case measure_units::font:
			_dest.put_member("units", "font");
			break;
		case measure_units::font_golden_ratio:
			_dest.put_member("units", "fontgr");
			break;
		case measure_units::percent_aspect:
			_dest.put_member("units", "aspect");
			break;
		case measure_units::percent_child:
			_dest.put_member("units", "child");
			break;
		case measure_units::percent_container:
			_dest.put_member("units", "container");
			break;
		case measure_units::percent_remaining:
			_dest.put_member("units", "remaining");
			break;
		case measure_units::pixels:
			_dest.put_member("units", "pixels");
			break;
		case measure_units::text:
			_dest.put_member("units", "text");
			break;
		default:
			std::cout << "measure unit undefined" << std::endl;
			return;

		}
	}
	void put_json(measure& _dest, json& _src)
	{
		if (!_src.has_members({ "amount", "units" })) {
			std::cout << "measure must have amount and units" << std::endl;
			std::cout << _src.to_json() << std::endl;
		}

		_dest.amount = (double)_src["amount"];
		std::string sunits = _src["units"];
		if (sunits.empty())
			sunits = "pixels";

		if (sunits == "font")
		{
			_dest.units = measure_units::font;
		}
		else if (sunits == "fontgr")
		{
			_dest.units = measure_units::font_golden_ratio;
		}
		else if (sunits == "aspect")
		{
			_dest.units = measure_units::percent_aspect;
		}
		else if (sunits == "child")
		{
			_dest.units = measure_units::percent_child;
		}
		else if (sunits == "container")
		{
			_dest.units = measure_units::percent_container;
		}
		else if (sunits == "remaining")
		{
			_dest.units = measure_units::percent_remaining;
		}
		else if (sunits == "pixels")
		{
			_dest.units = measure_units::pixels;
		}
		else if (sunits == "text")
		{
			_dest.units = measure_units::pixels;
		}
		else
		{
			std::cout << "measure unit '" << sunits << "' undefined, pixels assumed." << std::endl;
			_dest.units = measure_units::pixels;
		}
	}

	void get_json(json& _dest, layout_rect& _src)
	{
		json_parser jp;
		json jx, jy, jwidth, jheight;
		jx = jp.create_object();
		get_json(jx, _src.x);
		jy = jp.create_object();
		get_json(jy, _src.y);
		jwidth = jp.create_object();
		get_json(jwidth, _src.width);
		jheight = jp.create_object();
		get_json(jheight, _src.height);
		_dest.put_member("x", jx);
		_dest.put_member("y", jy);
		_dest.put_member("width", jwidth);
		_dest.put_member("height", jheight);
	}

	void put_json(layout_rect& _dest, json& _src)
	{
		json jx, jy, jwidth, jheight;
		if (!_src.has_members({ "width", "height" })) {
			std::cout << "layout_rect must have width and height. x and y are optional." << std::endl;
			std::cout << _src.to_json() << std::endl;
			return;
		}
		jx = _src["x"];
		jy = _src["y"];
		jwidth = _src["width"];
		jheight = _src["height"];
		put_json(_dest.x, jx);
		put_json(_dest.y, jy);
		put_json(_dest.width, jwidth);
		put_json(_dest.height, jheight);
	}

	void get_json(json& _dest, rectangle& _src)
	{
		_dest.put_member("x", _src.x);
		_dest.put_member("y", _src.y);
		_dest.put_member("w", _src.w);
		_dest.put_member("h", _src.h);
	}

	void put_json(rectangle& _dest, json& _src)
	{
		if (!_src.has_members({ "w", "h" })) {
			std::cout << "rectangle must have w, and h.  x & y are optional." << std::endl;
			std::cout << _src.to_json() << std::endl;
			return;
		}

		_dest.x = (double)_src["x"];
		_dest.y = (double)_src["y"];
		_dest.w = (double)_src["w"];
		_dest.h = (double)_src["h"];
	}


}

#endif

