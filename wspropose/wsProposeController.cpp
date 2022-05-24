
#pragma once


#include "pch.h"
#include "resource.h"

const int IDC_DIRECT2D = -1;

namespace corona
{
	namespace win32
	{
		wsProposalController::wsProposalController(viewStyle* _vs) :
			controller(_vs),
			showUpdate(false),
			currentWindowView(currentWindowViews::viewEmpty),
			previewMode(false),
			magnification(100)
		{
			;
		}

		wsProposalController::~wsProposalController()
		{
			;
		}

		void wsProposalController::loadController()
		{
			;
		}

		void wsProposalController::onInit()
		{
			enableEditMessages = false;

			auto pos = host->getWindowPos(0);
			sizeIntDto curSize(pos.width, pos.height);
			host->setMinimumWindowSize(curSize);

			enableEditMessages = true;

		}

		void wsProposalController::randomAdvertisement()
		{
		}

		void wsProposalController::keyDown(short _key)
		{
			;
		}

		void wsProposalController::keyUp(short _key)
		{
			;
		}

		void wsProposalController::mouseMove(pointDto* _point)
		{
			;
		}

		int wsProposalController::onHScroll(int controlId, scrollTypes scrollType)
		{
			if (controlId != IDC_DIRECT2D)
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

		int wsProposalController::onVScroll(int controlId, scrollTypes scrollType)
		{
			if (controlId != -1)
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

		int wsProposalController::onSpin(int controlId, int newPosition)
		{
			double newPositionF = newPosition / 100.0;
			return 0;
		}

		int wsProposalController::onResize(const rectDto& newSize)
		{
			rectDto r = host->getWindowPos(IDC_DIRECT2D);

			r.width = newSize.width - (r.left + 32);
			r.height = newSize.height - (r.top + 32);

			host->setWindowPos(IDC_DIRECT2D, r);

			setScrollBars();

			return 0;
		}

		void wsProposalController::setScrollBars()
		{
		}

		void wsProposalController::mouseClick(pointDto* _point)
		{
		}

		void wsProposalController::drawFrame()
		{
			try
			{

				auto frameLayout = host->getWindowPos(IDC_DIRECT2D);
				frameLayout.left = 0.0;
				frameLayout.top = 0.0;
				float f = 192.0 / 255.0;
				float paperMargin = 8.0;
				float shadowSize = 4.0;
				dtoFactory fact;

				switch (currentWindowView) {
				case currentWindowViews::viewEmpty:
				{
					int mx = 260, my = 80;
					fact.colorMake(f, f, f, 1.0);
					host->getDrawable(0)->clear(&fact.color);

					pathImmediateDto pim;
					auto size = model.getBitmapSize();
					fact.rectangleMake(&pim, (frameLayout.width - mx) / 2.0 + shadowSize, (frameLayout.height - my) / 2.0 + shadowSize, mx, my, "back", style->getBlackWashBrushName(), NULL, 0.0);
					host->getDrawable(0)->drawPath(&pim);
					fact.rectangleMake(&pim, (frameLayout.width - mx) / 2.0, (frameLayout.height - my) / 2.0, mx, my, "border", style->getWhiteBrushName(), style->getGreyBrushName(), 1.0);
					host->getDrawable(0)->drawPath(&pim);

					textInstance2dDto txt;
					getH1Styles(&txt);
					txt.position.x = (frameLayout.width - mx) / 2.0 + 25;
					txt.position.y = (frameLayout.height - my) / 2.0 + 25;
					txt.rotation = 0.0;
					txt.text = "No Folder Selected.";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);
				}
				break;

			}
			catch (directException exc)
			{
				;
			}
		}

		void wsProposalController::exportBitmap(const char* _filenameImage)
		{
			dtoFactory fact;

			auto bm = host->getDrawable(0)->createBitmap(model.getBitmapSize());

			auto instances = model.getImagesArranged();

			colorDto color;
			color.alpha = color.red = color.green = color.blue = 0.0;

			bm->beginDraw();
			bm->clear(&color);

			for (auto i = instances.begin(); i != instances.end(); i++) {
				bitmapInstanceDto dto = *i;
				bm->drawBitmap(&dto);
			}

			bm->endDraw();

			bm->save(_filenameImage);
			delete bm;
		}

		void wsProposalController::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
		{
			;
		}

		bool wsProposalController::update(double _elapsedSeconds, double _totalSeconds)
		{
			bool oldShowUpdate = showUpdate;
			showUpdate = false;
			return oldShowUpdate;
		}

		void wsProposalController::onCommand(int buttonId)
		{

			switch (buttonId) {
			}
			host->redraw();

		}

		void wsProposalController::onTextChanged(int textControlId)
		{
		}

		void wsProposalController::onDropDownChanged(int dropDownId)
		{
		}

		void wsProposalController::onListViewChanged(int listViewId)
		{
		}

		void wsProposalController::fromImage()
		{
			std::string temp;
			bool tempB;
			int tempI;

			enableEditMessages = false;

			enableEditMessages = true;
		}

		void wsProposalController::pointSelected(pointDto* _point, colorDto* _color)
		{

		}

		void wsProposalController::clearErrors(errorDto* _error)
		{
			;
		}

		void wsProposalController::addError(errorDto* _error)
		{
			;
		}
	}
}

