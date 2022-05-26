
#include "pch.h"

#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{
		controller::controller(viewStyle* _style) : host(NULL), style(_style)
		{
			backgroundColor.alpha = 1.0;
			backgroundColor.red = 0.2;
			backgroundColor.green = 0.0;
			backgroundColor.blue = 0.0;
		}

		controller::~controller()
		{
			;
		}

		void controller::attach(controllerHost* _host)
		{
			host = _host;
		}


		void controller::baseLoadController()
		{
			if (style) style->setCommonStyles(this);
			loadController();
		}

		// helpers for views

		void controller::getH1Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH1TextBrushName();
			_dto->styleName = style->getH1TextStyleName();
			_dto->backgroundBrushName = style->getH1BackgroundBrushName();
		}

		void controller::getH2Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH2TextBrushName();
			_dto->styleName = style->getH2TextStyleName();
			_dto->backgroundBrushName = style->getH2BackgroundBrushName();
		}

		void controller::getH3Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getH3TextBrushName();
			_dto->styleName = style->getH3TextStyleName();
			_dto->backgroundBrushName = style->getH3BackgroundBrushName();
		}

		void controller::getLabelStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getLabelTextBrushName();
			_dto->styleName = style->getLabelTextStyleName();
			_dto->backgroundBrushName = style->getLabelBackgroundBrushName();
		}

		void controller::getDataStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = style->getDataTextBrushName();
			_dto->styleName = style->getDataTextStyleName();
			_dto->backgroundBrushName = style->getDataBackgroundBrushName();
		}

		void corona_controller::onCreated(const rectDto& newSize)
		{
			auto state = this->program_chart.get_actor_state(this->sample_actor.actor_id);
			updateState(state, newSize);
		}

		void corona_controller::randomAdvertisement()
		{
		}

		void corona_controller::keyDown(short _key)
		{
			;
		}

		void corona_controller::keyUp(short _key)
		{
			;
		}

		void corona_controller::mouseMove(pointDto* _point)
		{
			;
		}

		int corona_controller::onHScroll(int controlId, scrollTypes scrollType)
		{
			if (controlId != canvasWindowsId)
				return 0;

			rectDto r = host->getWindowPos(controlId);
			int pos = host->getScrollPos(controlId).x;
			int max = host->getScrollRange(controlId).width;

			switch (scrollType) {
			case ScrollPageUp:
				pos -= r.width;
				if (pos < 0) pos = 0;
				break;
			case ScrollPageDown:
				pos += r.width;
				if (pos > max) pos = max;
				break;
			case ScrollLineUp:
				pos -= 16;
				if (pos < 0) pos = 0;
				break;
			case ScrollLineDown:
				pos += 16;
				if (pos > max) pos = max;
				break;
			case ThumbTrack:
				pos = host->getScrollTrackPos(controlId).x;
				break;
			}

			currentScroll.x = pos;
			host->redraw();

			return pos;
		}

		int corona_controller::onVScroll(int controlId, scrollTypes scrollType)
		{
			if (controlId != canvasWindowsId)
				return 0;

			rectDto r = host->getWindowPos(controlId);
			int pos = host->getScrollPos(controlId).y;
			int max = host->getScrollRange(controlId).height;

			switch (scrollType) {
			case ScrollPageUp:
				pos -= r.height;
				if (pos < 0) pos = 0;
				break;
			case ScrollPageDown:
				pos += r.height;
				if (pos > max) pos = max;
				break;
			case ScrollLineUp:
				pos -= 16;
				if (pos < 0) pos = 0;
				break;
			case ScrollLineDown:
				pos += 16;
				if (pos > max) pos = max;
				break;
			case ThumbTrack:
				pos = host->getScrollTrackPos(controlId).y;
				break;
			}

			currentScroll.y = pos;
			host->redraw();

			return pos;
		}

		int corona_controller::onSpin(int controlId, int newPosition)
		{
			double newPositionF = newPosition / 100.0;
			return 0;
		}

		int corona_controller::onResize(const rectDto& newSize)
		{
			rectDto r = host->getWindowPos(canvasWindowsId);

			r.width = newSize.width - (r.left + 32);
			r.height = newSize.height - (r.top + 32);

			host->setWindowPos(canvasWindowsId, r);

			setScrollBars();

			auto result = program_chart.get_actor_state(sample_actor.actor_id, null_row, "state");
			updateState(result, newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(pointDto* _point)
		{
		}

		void corona_controller::drawFrame()
		{

		}

		void corona_controller::exportBitmap(const char* _filenameImage)
		{
			dtoFactory fact;

			sizeIntDto dt = { 100, 100 };
			auto bm = host->getDrawable(0)->createBitmap(dt);

			colorDto color;
			color.alpha = color.red = color.green = color.blue = 0.0;

			bm->beginDraw();
			bm->clear(&color);

			bm->endDraw();

			bm->save(_filenameImage);

			delete bm;
		}

		void corona_controller::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
		{
			;
		}

		bool corona_controller::update(double _elapsedSeconds, double _totalSeconds)
		{
			bool oldShowUpdate = showUpdate;
			showUpdate = false;
			return oldShowUpdate;
		}

		void corona_controller::onCommand(int buttonId)
		{

			switch (buttonId) {
			}
			host->redraw();

		}

		void corona_controller::onTextChanged(int textControlId)
		{
		}

		void corona_controller::onDropDownChanged(int dropDownId)
		{
		}

		void corona_controller::onListViewChanged(int listViewId)
		{
		}

		void corona_controller::fromImage()
		{
			std::string temp;
			bool tempB;
			int tempI;

			enableEditMessages = false;

			enableEditMessages = true;
		}

		void corona_controller::pointSelected(pointDto* _point, colorDto* _color)
		{

		}

		void corona_controller::clearErrors(errorDto* _error)
		{
			;
		}

		void corona_controller::addError(errorDto* _error)
		{
			;
		}

	}
}

#endif

