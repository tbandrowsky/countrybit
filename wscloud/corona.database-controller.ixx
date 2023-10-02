module;

#include <memory>
#include <string>
#include <vector>
#include <list>
#include "windows.h"

export module corona.database:controller;
import :constants;
import :direct2dcontext;
import :visual;
import :store_box;
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


	virtual void redraw() = 0;

	virtual void setColorCapture(int _iconResourceId) = 0;

	virtual bool getSaveFilename(std::string& _saveFileName, const char* _pathExtensions, const char* _defaultExtension) = 0;
	virtual rectangle getWindowClientPos() = 0;
	virtual rectangle getWindowPos(int ddlControlId) = 0;
	virtual rectangle toPixelsFromDips(const rectangle& r) = 0;
	virtual void setWindowPos(int ddlControlId, rectangle rect) = 0;
	virtual void setMinimumWindowSize(point size) = 0;

	virtual bool drawFrame(direct2dContext& _ctx) = 0;
	virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

	virtual void keyDown(std::shared_ptr<direct2dWindow>& win, short _key) = 0;
	virtual void keyUp(std::shared_ptr<direct2dWindow>& win, short _key) = 0;
	virtual void mouseMove(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;
	virtual void mouseLeftDown(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;
	virtual void mouseLeftUp(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;
	virtual void mouseRightDown(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;
	virtual void mouseRightUp(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;
	virtual void pointSelected(std::shared_ptr<direct2dWindow>& win, point* _point, color* _color) = 0;
	virtual LRESULT ncHitTest(std::shared_ptr<direct2dWindow>& win, point* _point) = 0;

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

