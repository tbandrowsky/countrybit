module;

#include <compare>

export module corona.database:image_box;
import :store_box;
import :rectangle_box;

export struct image_instance
{
	rectangle	source;
};

export class image_box : public boxed<image_instance>
{
public:
	image_box(char* t) : boxed<image_instance>(t)
	{
		;
	}

	image_box operator = (const image_box& _src)
	{
		boxed<image_instance>::set_data(_src);
		return *this;
	}

	image_box operator = (image_instance _src)
	{
		boxed<image_instance>::set_value(_src);
		return *this;
	}

	operator image_instance& ()
	{
		image_instance& t = boxed<image_instance>::get_data_ref();
		return t;
	}

	image_instance* operator->()
	{
		image_instance& t = boxed<image_instance>::get_data_ref();
		return &t;
	}

	image_instance value() const { return boxed<image_instance>::get_value(); }

};
