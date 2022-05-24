
#pragma once

namespace corona
{
	namespace win32
	{
		class viewStyle;

		enum scrollTypes
		{
			ScrollPageUp,
			ScrollPageDown,
			ScrollLineUp,
			ScrollLineDown,
			ThumbTrack
		};

		class controller
		{

		protected:

			controllerHost* host;
			viewStyle* style;

		public:

			colorDto backgroundColor;

			controller(viewStyle* _style);
			virtual ~controller();

			inline controllerHost* getHost() { return host; }
			inline drawableHost* getDrawable() { return host->getDrawable(0); }
			virtual void attach(controllerHost* _host);

			void baseLoadController();

			// these are for 

			virtual void loadController() = 0;
			virtual void keyDown(short _key) = 0;
			virtual void keyUp(short _key) = 0;
			virtual void mouseMove(pointDto* _point) = 0;
			virtual void mouseClick(pointDto* _point) = 0;
			virtual void pointSelected(pointDto* _point, colorDto* _color) = 0;
			virtual void drawFrame() = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onInit() = 0;
			virtual void onCreated(const rectDto& newSize) = 0;
			virtual void onCommand(int buttonId) = 0;
			virtual void onTextChanged(int textControlId) = 0;
			virtual void onDropDownChanged(int dropDownId) = 0;
			virtual void onListViewChanged(int listViewId) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onResize(const rectDto& newSize) = 0;
			virtual int onSpin(int controlId, int newPosition) = 0;

			// helper stuff for views
			void getH1Styles(textInstance2dDto* _dto);
			void getH2Styles(textInstance2dDto* _dto);
			void getH3Styles(textInstance2dDto* _dto);
			void getLabelStyles(textInstance2dDto* _dto);
			void getDataStyles(textInstance2dDto* _dto);

		};

	}
}

