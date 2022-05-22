
#pragma once

#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{


		void viewStyle::setCommonStyles(controller* _controller)
		{
			dtoPad.textStyleMake(_controller->getDrawable(), getH1TextStyleName(), "Arial", 18, false, false)->
				textStyleMake(_controller->getDrawable(), getH2TextStyleName(), "Arial", 14, false, false)->
				textStyleMake(_controller->getDrawable(), getH3TextStyleName(), "Arial", 12, false, false)->
				textStyleMake(_controller->getDrawable(), getLabelTextStyleName(), "Arial", 10, false, false)->
				textStyleMake(_controller->getDrawable(), getDataTextStyleName(), "Arial", 10, false, false);

			// some stock brushes to go with our style

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH1TextBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH1BackgroundBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH2TextBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH2BackgroundBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH3TextBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getH3BackgroundBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getLabelTextBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getLabelBackgroundBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getDataTextBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getDataBackgroundBrushName(), true);

			// some basic colors

			dtoPad.colorMake(1.0, 1.0, 1.0, .5)
				->solidBrushMake(_controller->getDrawable(), getWhiteWashBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, .5)
				->solidBrushMake(_controller->getDrawable(), getBlackWashBrushName(), true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getWhiteBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getBlackBrushName(), true);

			dtoPad.colorMake(0.5, 0.5, 0.5, 1.0)
				->solidBrushMake(_controller->getDrawable(), getGreyBrushName(), true);

			dtoPad.colorMake(1.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getRedBrushName(), true);

			dtoPad.colorMake(0.0, 1.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getGreenBrushName(), true);

			dtoPad.colorMake(0.0, 0.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), getBlueBrushName(), true);

		}
	}
}

#endif
