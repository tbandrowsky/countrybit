/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
These are the layout specification rectangles for the presentation system.

Notes

For Future Consideration
*/


#ifndef CORONA_LAYOUT_RECT_H
#define CORONA_LAYOUT_RECT_H

namespace corona {

	enum class measure_units
	{
		percent_container = 0,
		percent_remaining = 1,
		percent_aspect = 2,
		pixels = 3,
		font = 4,
		font_golden_ratio = 5,
		percent_child = 6,
		text = 7
	};

	class measure 
	{
	public:
		long double amount;
		measure_units units;

		measure() : amount(0), units(measure_units::pixels) { ; }
		measure(long double _amount, measure_units _units) : amount(_amount), units(_units) { ; }

	};

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
		}
	}
	void put_json(measure& _dest, json& _src)
	{
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
			_dest.units = measure_units::pixels;
		}
	}


	measure operator ""_px(long double px);
	measure operator ""_container(long double pct);
	measure operator ""_remaining(long double pct);
	measure operator ""_children(long double pct);
	measure operator ""_aspect(long double pct);
	measure operator ""_font(long double fnt);
	measure operator ""_fontgr(long double fnt);
	measure operator ""_text(long double fnt);
	measure operator -(const measure& _src);

	class layout_rect 
	{
	public:
		measure x, y, width, height;

		layout_rect() { ; }
		layout_rect(measure _x, measure _y, measure _width, measure _height) :
			x(_x), y(_y), width(_width), height(_height)
		{
			;
		}
		layout_rect(measure _width, measure _height) :
			x(0.0_px), y(0.0_px), width(_width), height(_height)
		{
			;
		}


	};

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
		jx = _src["x"];
		jy = _src["y"];
		jwidth = _src["width"];
		jheight = _src["height"];
		put_json(_dest.x, jx);
		put_json(_dest.y, jy);
		put_json(_dest.width, jwidth);
		put_json(_dest.height, jheight);
	}

	class layout_rect_box : protected boxed<layout_rect>
	{
	public:
		layout_rect_box(char* t) : boxed<layout_rect>(t)
		{
			;
		}

		layout_rect_box operator = (const layout_rect_box& _src)
		{
			boxed<layout_rect>::operator =(_src);
			return *this;
		}

		layout_rect_box operator = (layout_rect_box _src)
		{
			set_data(_src);
			return *this;
		}

		layout_rect_box operator = (layout_rect _src)
		{
			set_value(_src);
			return *this;
		}

		layout_rect* operator->()
		{
			layout_rect& t = boxed<layout_rect>::get_data_ref();
			return &t;
		}

		operator layout_rect& ()
		{
			layout_rect& t = boxed<layout_rect>::get_data_ref();
			return t;
		}

		layout_rect value() const { return boxed<layout_rect>::get_value(); }

	};

	measure operator ""_px(long double px)
	{
		return measure(px, measure_units::pixels);
	}

	measure operator ""_remaining(long double pct)
	{
		return measure(pct, measure_units::percent_remaining);
	}

	measure operator ""_container(long double pcc)
	{
		return measure(pcc, measure_units::percent_container);
	}

	measure operator ""_aspect(long double pct)
	{
		return measure(pct, measure_units::percent_aspect);
	}

	measure operator ""_children(long double pct)
	{
		return measure(pct, measure_units::percent_child);
	}

	measure operator ""_font(long double fnt)
	{
		return measure(fnt, measure_units::font);
	}

	measure operator ""_text(long double fnt)
	{
		return measure(fnt, measure_units::text);
	}

	measure operator ""_fontgr(long double fnt)
	{
		return measure(fnt, measure_units::font_golden_ratio);
	}

	measure operator -(const measure& _src)
	{
		return measure(-_src.amount, _src.units);
	}

}

#endif
