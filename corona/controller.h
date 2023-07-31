
#pragma once

namespace corona
{
	namespace win32
	{

		using namespace database;

		class page;

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

			win32ControllerHost *host;

		public:

			color backgroundColor;

			controller(win32ControllerHost *_host) 
				: host(_host)
			{
				backgroundColor.a = 1.0;
				backgroundColor.r = 1.0;
				backgroundColor.g = 1.0;
				backgroundColor.b = 1.0;
			}

			virtual ~controller()
			{

			}

			inline win32ControllerHost *getHost() { return host; }

			virtual jobject get_style_sheet(int _idx) = 0;
			virtual void keyDown(direct2dWindow *win, short _key) = 0;
			virtual void keyUp(direct2dWindow* win, short _key) = 0;
			virtual void mouseMove(direct2dWindow* win, point* _point) = 0;
			virtual void mouseClick(direct2dWindow* win, point* _point) = 0;
			virtual void pointSelected(direct2dWindow *win, point* _point, color* _color) = 0;
			virtual bool drawFrame() = 0;
			virtual bool drawCanvas(int _id) = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onCreated() = 0;
			virtual void onCommand(int buttonId) = 0;
			virtual void onTextChanged(int textControlId) = 0;
			virtual void onDropDownChanged(int dropDownId) = 0;
			virtual void onListViewChanged(int listViewId) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onResize(const rectangle& newSize, double d2dScale) = 0;
			virtual int onSpin(int controlId, int newPosition) = 0;
		};

	}
}

