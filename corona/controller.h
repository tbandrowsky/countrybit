
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

			std::weak_ptr<directApplicationWin32> host;

		public:

			controller()
			{
			}

			virtual ~controller()
			{

			}

			inline void setHost(std::weak_ptr<directApplicationWin32> _host) { host = _host; }
			inline std::shared_ptr<directApplicationWin32> getHost() { return host.lock(); }

			virtual void keyDown(std::shared_ptr<win32::direct2dWindow>& win, short _key) = 0;
			virtual void keyUp(std::shared_ptr<win32::direct2dWindow>& win, short _key) = 0;
			virtual void mouseMove(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
			virtual void mouseClick(std::shared_ptr<win32::direct2dWindow>& win, point* _point) = 0;
			virtual void pointSelected(std::shared_ptr<win32::direct2dWindow>& win, point* _point, color* _color) = 0;
			virtual bool drawFrame(CComPtr<ID2D1DeviceContext>& _context) = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onCreated() = 0;
			virtual void onCommand(int buttonId) = 0;
			virtual void onTextChanged(int textControlId) = 0;
			virtual void onDropDownChanged(int dropDownId) = 0;
			virtual void onListBoxChanged(int dropDownId) = 0;
			virtual void onListViewChanged(int listViewId) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onResize(const rectangle& newSize, double d2dScale) = 0;
			virtual int onSpin(int controlId, int newPosition) = 0;
		};

	}
}

