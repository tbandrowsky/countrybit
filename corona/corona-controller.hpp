#ifndef CORONA_CONTROLLER_H
#define CORONA_CONTROLLER_H

namespace corona
{

	class ui_task_result_base;

	enum scrollTypes
	{
		ScrollPageUp,
		ScrollPageDown,
		ScrollLineUp,
		ScrollLineDown,
		ThumbTrack
	};

	class controller {

	public:

		virtual bool drawFrame(direct2dContext& _ctx) = 0;
		virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

		virtual void keyPress(int _ctrl_id, short _key) = 0;
		virtual void keyDown(int _ctrl_id, short _key) = 0;
		virtual void keyUp(int _ctrl_id, short _key) = 0;
		virtual void mouseMove(point* _point) = 0;
		virtual void mouseLeftDown(point* _point) = 0;
		virtual void mouseLeftUp(point* _point) = 0;
		virtual void mouseRightDown(point* _point) = 0;
		virtual void mouseRightUp(point* _point) = 0;
		virtual void pointSelected(point* _point, ccolor* _color) = 0;
		virtual LRESULT ncHitTest(point* _point) = 0;
		virtual void setFocus(int ddlControlId) = 0;
		virtual void killFocus(int ddlControlId) = 0;
		virtual bool navigationKey(int _key) = 0;

		virtual void onHostCreated() = 0;
		virtual void onCreated() = 0;
		virtual void onCommand(int buttonId) = 0;
		virtual void onTextChanged(int textControlId) = 0;
		virtual void onDropDownChanged(int dropDownId) = 0;
		virtual void onListBoxChanged(int listBoxId) = 0;
		virtual void onListViewChanged(int listViewId) = 0;
		virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
		virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
		virtual int onResize(const rectangle& newSize, double d2dScale) = 0;
		virtual int onSpin(int controlId, int newPosition) = 0;
		virtual int layout() = 0;

		virtual void onDataChanged(json _params, data_lake* _api, data_function* _set) = 0;
		virtual void onLogged(data_lake* _api) = 0;

		virtual int getDefaultButtonId() = 0;
		virtual void hardwareChanged() = 0;

	};

}

#endif
