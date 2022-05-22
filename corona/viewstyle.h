
#pragma once

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{

		class controller;

		class viewStyle {
			dtoFactory dtoPad;
		public:
			virtual void setCommonStyles(controller* _controller);

			inline const char* getH1TextStyleName() { return "h1text"; }
			inline const char* getH2TextStyleName() { return "h2text"; }
			inline const char* getH3TextStyleName() { return "h3text"; }
			inline const char* getLabelTextStyleName() { return "plabel"; }
			inline const char* getDataTextStyleName() { return "pdata"; }

			inline const char* getH1TextBrushName() { return "h1textbrush"; }
			inline const char* getH2TextBrushName() { return "h2textbrush"; }
			inline const char* getH3TextBrushName() { return "h3textbrush"; }
			inline const char* getLabelTextBrushName() { return "plabelbrush"; }
			inline const char* getDataTextBrushName() { return "pdatabrush"; }

			inline const char* getH1BackgroundBrushName() { return "h1backgroundbrush"; }
			inline const char* getH2BackgroundBrushName() { return "h2backgroundbrush"; }
			inline const char* getH3BackgroundBrushName() { return "h3backgroundbrush"; }
			inline const char* getLabelBackgroundBrushName() { return "plabelbackgroundbrush"; }
			inline const char* getDataBackgroundBrushName() { return "pdatabackgroundbrush"; }

			inline const char* getWhiteBrushName() { return "whiteBrush"; }
			inline const char* getBlackBrushName() { return "blackBrush"; }
			inline const char* getGreyBrushName() { return "greyBrush"; }
			inline const char* getRedBrushName() { return "redBrush"; }
			inline const char* getGreenBrushName() { return "greenBrush"; }
			inline const char* getBlueBrushName() { return "blueBrush"; }
			inline const char* getWhiteWashBrushName() { return "whiteWashBrush"; }
			inline const char* getBlackWashBrushName() { return "blackWashBrush"; }

		};

	}
}

#endif
