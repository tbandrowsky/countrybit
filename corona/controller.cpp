
#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{
		controller::controller(viewStyle* _style) : host(NULL), style(_style)
		{
			backgroundColor.alpha = 1.0;
			backgroundColor.red = 0.2;
			backgroundColor.green = 0.0;
			backgroundColor.blue = 0.0;
		}

		controller::~controller()
		{
			;
		}

		void controller::attach(controllerHost* _host)
		{
			host = _host;
		}


		void controller::baseLoadController()
		{
			if (style) style->setCommonStyles(this);
			loadController();
		}

		// helpers for views

		void controller::getH1Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH1TextBrushName();
			_dto->styleName = style->getH1TextStyleName();
			_dto->backgroundBrushName = style->getH1BackgroundBrushName();
		}

		void controller::getH2Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH2TextBrushName();
			_dto->styleName = style->getH2TextStyleName();
			_dto->backgroundBrushName = style->getH2BackgroundBrushName();
		}

		void controller::getH3Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH3TextBrushName();
			_dto->styleName = style->getH3TextStyleName();
			_dto->backgroundBrushName = style->getH3BackgroundBrushName();
		}

		void controller::getLabelStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getLabelTextBrushName();
			_dto->styleName = style->getLabelTextStyleName();
			_dto->backgroundBrushName = style->getLabelBackgroundBrushName();
		}

		void controller::getDataStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getDataTextBrushName();
			_dto->styleName = style->getDataTextStyleName();
			_dto->backgroundBrushName = style->getDataBackgroundBrushName();
		}

	}
}

#endif
