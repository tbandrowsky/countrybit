
#pragma once

#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{


		void viewStyle::setCommonStyles(controller* _controller)
		{
			dtoPad.textStyleMake(_controller->getDrawable(), H1Text, "Arial", 20, false, false, 0.0, visual_alignment::align_near, visual_alignment::align_center, true)->
				textStyleMake(_controller->getDrawable(), H2Text, "Arial", 18, false, false, 0.0, visual_alignment::align_near, visual_alignment::align_center, true)->
				textStyleMake(_controller->getDrawable(), H3Text, "Arial", 14, false, false, 0.0, visual_alignment::align_near, visual_alignment::align_center, true)->
				textStyleMake(_controller->getDrawable(), LabelText, "Arial", 12, false, false, 0.0, visual_alignment::align_near, visual_alignment::align_center, true)->
				textStyleMake(_controller->getDrawable(), DataText, "Arial", 12, true, false, 0.0, visual_alignment::align_near, visual_alignment::align_center, true);

			// some stock brushes to go with our style

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H1Fill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H1Background, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H2Fill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H2Background, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H3Fill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), H3Background, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), LabelFill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), LabelBackground, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), DataFill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), DataBackground, true);

			// some basic colors

			dtoPad.colorMake(1.0, 1.0, 1.0, .5)
				->solidBrushMake(_controller->getDrawable(), WhiteWashFill, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, .5)
				->solidBrushMake(_controller->getDrawable(), BlackWashFill, true);

			dtoPad.colorMake(1.0, 1.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), WhiteFill, true);

			dtoPad.colorMake(0.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), BlackFill, true);

			dtoPad.colorMake(0.5, 0.5, 0.5, 1.0)
				->solidBrushMake(_controller->getDrawable(), GreyFill, true);

			dtoPad.colorMake(1.0, 0.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), RedFill, true);

			dtoPad.colorMake(0.0, 1.0, 0.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), GreenFill, true);

			dtoPad.colorMake(0.0, 0.0, 1.0, 1.0)
				->solidBrushMake(_controller->getDrawable(), BlueFill, true);

		}
	}
}

#endif
