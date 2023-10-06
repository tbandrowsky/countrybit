module;

#include <memory>
#include <string>
#include <vector>
#include <list>

export module corona:controller;

import :constants;
import :direct2dcontext;
import :rectangle_box;
import :point_box;
import :datatransfer;

export enum scrollTypes
{
	ScrollPageUp,
	ScrollPageDown,
	ScrollLineUp,
	ScrollLineDown,
	ThumbTrack
};

export class controller {

public:

	virtual bool drawFrame(direct2dContext& _ctx) = 0;
	virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

	virtual void keyDown(short _key);
	virtual void keyUp(short _key);
	virtual void mouseMove(point* _point);
	virtual void mouseLeftDown(point* _point);
	virtual void mouseLeftUp(point* _point);
	virtual void mouseRightDown(point* _point);
	virtual void mouseRightUp(point* _point);
	virtual void pointSelected(point* _point, color* _color);
	virtual LRESULT ncHitTest(point* _point);

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
};

