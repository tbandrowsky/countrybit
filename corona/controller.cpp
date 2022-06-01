
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
			_dto->fillBrushName = viewStyle::H1Fill;
			_dto->styleName = viewStyle::H1Text;
			_dto->backgroundBrushName = viewStyle::H1Background;
		}

		void controller::getH2Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = viewStyle::H2Fill;
			_dto->styleName = viewStyle::H2Text;
			_dto->backgroundBrushName = viewStyle::H2Background;
		}

		void controller::getH3Styles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = viewStyle::H3Fill;
			_dto->styleName = viewStyle::H3Text;
			_dto->backgroundBrushName = viewStyle::H3Background;
		}

		void controller::getLabelStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = viewStyle::LabelFill;
			_dto->styleName = viewStyle::LabelText;
			_dto->backgroundBrushName = viewStyle::LabelBackground;
		}

		void controller::getDataStyles(textInstance2dDto* _dto)
		{
			_dto->fillBrushName = viewStyle::DataFill;
			_dto->styleName = viewStyle::DataText;
			_dto->backgroundBrushName = viewStyle::DataBackground;
		}

		corona_controller::~corona_controller()
		{
			;
		}

		void corona_controller::onCreated(const rectDto& newSize)
		{
			if (style) style->setCommonStyles(this);
			state = this->program_chart.get_actor_state(this->sample_actor.actor_id, -1, "onCreated State");
			stateChanged(newSize);
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
				pos -= r.w;
				if (pos < 0) pos = 0;
				break;
			case ScrollPageDown:
				pos += r.w;
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
				pos -= r.h;
				if (pos < 0) pos = 0;
				break;
			case ScrollPageDown:
				pos += r.h;
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

			r.w = newSize.w - (r.x + 32);
			r.h = newSize.h - (r.y + 32);

			host->setWindowPos(canvasWindowsId, r);

			setScrollBars();

			state = program_chart.get_actor_state(sample_actor.actor_id, database::null_row, "state");
			stateChanged(newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(pointDto* _point)
		{
			auto select_item_iter = pg.where([this, _point](const auto& pi) { return pi.item.is_drawable() && corona::database::rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });
			auto size = host->getWindowPos(0);

			if (select_item_iter != std::end(pg)) 
			{
				auto select_item = select_item_iter.get_object();
				std::cout << std::format("{} selected", select_item.item.id);
				state = this->program_chart.select_object(select_item.item.select_request);
				stateChanged(size);
			}
			else 
			{
				corona::database::select_object_request request;
				request.collection_id = program_chart.get_collection_id();
				request.actor_id = sample_actor.actor_id;
				request.object_id = 0;
				state = this->program_chart.select_object(request);
				stateChanged(size);
			}
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
			auto command_item = pg.where([this, buttonId](const auto& pi) { return pi.item.id == buttonId; })
				.get_object();

			std::cout << "Create " << buttonId << std::endl;

			state = this->program_chart.create_object(command_item.item.create_request, "Create Item");
			auto size = host->getWindowPos(0);

			stateChanged(size);
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

		void corona_controller::for_each(std::function<bool(const database::actor_view_collection::iterator_item_type& _item)> selector, std::function<bool(database::actor_view_object& avo, database::jslice& slice)> updator)
		{
			auto selections = state.view_objects.where(selector);
			for (auto selection : selections)
			{
				database::jslice slice = program_chart.get_object(selection.second.object_id);
				updator(selection.second, slice);
			}
		}

		void corona_controller::for_each(database::relative_ptr_type class_id, std::function<bool(const database::actor_view_object& avo, database::jslice& slice)>  updator)
		{
			auto selections = state.view_objects.where([class_id](auto& kp) {return kp.second.class_id == class_id; });
			for (auto selection : selections)
			{
				database::jslice slice = program_chart.get_object(selection.second.object_id);
				updator(selection.second, slice);
			}
		}
	}
}

#endif

