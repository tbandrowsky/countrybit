
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

			inline static const  char* H1Text = "h1text";
			inline static const  char* H2Text = "h2text";
			inline static const  char* H3Text = "h3text";
			inline static const  char* LabelText = "plabel";
			inline static const  char* DataText = "pdata";

			inline static const  char* H1Fill = "h1textbrush";
			inline static const  char* H2Fill = "h2textbrush";
			inline static const  char* H3Fill = "h3textbrush";
			inline static const  char* LabelFill = "plabelbrush";
			inline static const  char* DataFill = "pdatabrush";

			inline static const  char* H1Background = "h1backgroundbrush";
			inline static const  char* H2Background = "h2backgroundbrush";
			inline static const  char* H3Background = "h3backgroundbrush";
			inline static const  char* LabelBackground = "plabelbackgroundbrush";
			inline static const  char* DataBackground = "pdatabackgroundbrush";

			inline static const  char* WhiteFill = "whiteBrush";
			inline static const  char* BlackFill = "blackBrush";
			inline static const  char* GreyFill = "greyBrush";
			inline static const  char* RedFill = "redBrush";
			inline static const  char* GreenFill = "greenBrush";
			inline static const  char* BlueFill = "blueBrush";
			inline static const  char* WhiteWashFill = "whiteWashBrush";
			inline static const  char* BlackWashFill = "blackWashBrush";
		};

	}
}

#endif
