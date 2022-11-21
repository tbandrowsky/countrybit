
#include "corona.h"

namespace corona
{
	namespace database
	{

		bool jvariant::compare(comparisons _comparison, jvariant& _target)
		{
			int64_t ithis, itarget;
			double dthis, dtarget;
			const char* sthis, * starget;
			auto& rthis = *this;

			switch (_comparison)
			{
			case comparisons::eq:
				if (is_integer(ithis) && _target.is_integer(itarget))
				{
					return ithis == itarget;
				}
				else if (is_double(dthis) && _target.is_double(dtarget))
				{
					return dthis == dtarget;
				}
				else if (is_double(dthis) && _target.is_integer(itarget))
				{
					return dthis == itarget;
				}
				else if (is_integer(ithis) && _target.is_double(dtarget))
				{
					return ithis == dtarget;
				}
				else
				{
					sthis = rthis;
					starget = _target;
					return strcmp(sthis, starget) == 0;
				}
				break;
			case comparisons::lt:
				if (is_integer(ithis) && _target.is_integer(itarget))
				{
					return ithis < itarget;
				}
				else if (is_double(dthis) && _target.is_double(dtarget))
				{
					return dthis < dtarget;
				}
				else if (is_double(dthis) && _target.is_integer(itarget))
				{
					return dthis < itarget;
				}
				else if (is_integer(ithis) && _target.is_double(dtarget))
				{
					return ithis < dtarget;
				}
				else
				{
					sthis = rthis;
					starget = _target;
					return strcmp(sthis, starget) < 0;
				}
				break;
			case comparisons::lte:
				if (is_integer(ithis) && _target.is_integer(itarget))
				{
					return ithis <= itarget;
				}
				else if (is_double(dthis) && _target.is_double(dtarget))
				{
					return dthis <= dtarget;
				}
				else if (is_double(dthis) && _target.is_integer(itarget))
				{
					return dthis <= itarget;
				}
				else if (is_integer(ithis) && _target.is_double(dtarget))
				{
					return ithis <= dtarget;
				}
				else
				{
					sthis = rthis;
					starget = _target;
					return strcmp(sthis, starget) <= 0;
				}
				break;
			case comparisons::gt:
				if (is_integer(ithis) && _target.is_integer(itarget))
				{
					return ithis > itarget;
				}
				else if (is_double(dthis) && _target.is_double(dtarget))
				{
					return dthis > dtarget;
				}
				else if (is_double(dthis) && _target.is_integer(itarget))
				{
					return dthis > itarget;
				}
				else if (is_integer(ithis) && _target.is_double(dtarget))
				{
					return ithis > dtarget;
				}
				else
				{
					sthis = rthis;
					starget = _target;
					return strcmp(sthis, starget) > 0;
				}
				break;
			case comparisons::gte:
				if (is_integer(ithis) && _target.is_integer(itarget))
				{
					return ithis >= itarget;
				}
				else if (is_double(dthis) && _target.is_double(dtarget))
				{
					return dthis >= dtarget;
				}
				else if (is_double(dthis) && _target.is_integer(itarget))
				{
					return dthis >= itarget;
				}
				else if (is_integer(ithis) && _target.is_double(dtarget))
				{
					return ithis >= dtarget;
				}
				else
				{
					sthis = rthis;
					starget = _target;
					return strcmp(sthis, starget) >= 0;
				}
				break;
			case comparisons::cont:
				sthis = rthis;
				starget = _target;
				return strstr(sthis, starget) != nullptr;
			}
			return false;
		}

		void jvariant::copy(const jvariant& _src)
		{
			field_id = _src.field_id;
			variant_type = _src.variant_type;
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				int_value = _src.int_value;
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				double_value = _src.double_value;
				break;
			case jtype::type_color:
				color_value = _src.color_value;
				break;
			case jtype::type_datetime:
				time_value = _src.time_value;
				break;
			case jtype::type_image:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_layout_rect:
				layout_rect_value = _src.layout_rect_value;
				break;
			case jtype::type_midi:
				throw std::logic_error("not implemented");
				break;
			case jtype::type_null:
				break;
			case jtype::type_point:
				point_value = _src.point_value;
				break;
			case jtype::type_rectangle:
				rectangle_value = _src.rectangle_value;
				break;
			case jtype::type_string:
				string_value = _src.string_value;
				break;
			case jtype::type_wave:
				break;
			}
		}

		jvariant::jvariant(const jvariant& _src)
		{
			copy(_src);
		}

		jvariant jvariant::operator =(const jvariant& _src)
		{
			copy(_src);
			return *this;
		}

		jvariant::operator std::string()
		{
			std::string z;
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				z = std::format("{}", int_value);
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				z = std::format("{}", double_value);
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to string");
				break;
			case jtype::type_string:
				z = string_value;
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to string");
				break;
			}
			return z;
		}

		jvariant::operator color ()
		{
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_color:
				return color_value;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to color");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to color");
				break;
			}
		}

		jvariant::operator point()
		{
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_point:
				return point_value;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to point");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to point");
				break;
			}
		}

		jvariant::operator rectangle()
		{
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_layout_rect:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_midi:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_rectangle:
				return rectangle_value;
			case jtype::type_string:
				throw std::logic_error("can't convert to rectangle");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to rectangle");
				break;
			}
		}

		jvariant::operator layout_rect()
		{
			switch (variant_type)
			{
			case jtype::type_int8:
			case jtype::type_int16:
			case jtype::type_int32:
			case jtype::type_int64:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_float32:
			case jtype::type_float64:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_color:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_datetime:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_image:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_layout_rect:
				return layout_rect_value;
			case jtype::type_midi:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_null:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_point:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_rectangle:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_string:
				throw std::logic_error("can't convert to layout_rect");
				break;
			case jtype::type_wave:
				throw std::logic_error("can't convert to layout_rect");
				break;
			}
		}

	}
}
