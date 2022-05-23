
pragma once


#include "pch.h"
#include "resource.h"

namespace corona
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

		host->setPictureIcon(IDC_SOURCE_PICTURE, dtoIconId::STOCKICO_FOLDER);
		host->setPictureIcon(IDC_DEST_PICTURE, dtoIconId::STOCKICO_IMAGEFILES);
		host->setButtonIcon(IDC_SOURCE_FOLDER, dtoIconId::STOCKICO_FOLDEROPEN);
		host->setButtonIcon(IDC_SOURCE_READ, dtoIconId::STOCKICO_IMAGEFILES);
		host->setButtonIcon(IDC_PREVIEW_STRIP, dtoIconId::STOCKICO_FIND);
		host->setButtonIcon(IDC_SAVE, dtoIconId::STOCKICO_DRIVECD);
		host->setPictureIcon(IDC_STATUS_PICTURE, dtoIconId::STOCKICO_INFO);

		std::list<std::pair<std::string, int>> bitmapSizes;
		bitmapSizes = model.getBitmapWidths();

		for (auto sz = bitmapSizes.begin(); sz != bitmapSizes.end(); sz++)
		{
			host->addComboItem(IDC_CREATE_SIZE, sz->first, sz->second);
		}
		host->setComboSelectedIndex(IDC_CREATE_SIZE, 0);

		std::list<std::pair<std::string, int>> projectTypes;
		projectTypes = model.getProjectTypes();

		for (auto sz = projectTypes.begin(); sz != projectTypes.end(); sz++)
		{
			host->addComboItem(IDC_CREATE_TYPE, sz->first, sz->second);
		}
		host->setComboSelectedIndex(IDC_CREATE_TYPE, 0);

		/*
			std::list<std::pair<std::string,int>> imageSets;
			imageSets = model.getSettings();

			host->clearComboItems( IDC_SETTINGS_SET );

			for (auto sz = imageSets.begin(); sz != imageSets.end(); sz++)
			{
				host->addComboItem( IDC_SETTINGS_SET, sz->first, sz->second );
			}
			host->setComboSelectedIndex( IDC_SETTINGS_SET, 0 );
			*/

		host->addComboItem(IDC_COMBO_MAGNIFY, "25%", 25);
		host->addComboItem(IDC_COMBO_MAGNIFY, "50%", 50);
		host->addComboItem(IDC_COMBO_MAGNIFY, "100%", 100);
		host->addComboItem(IDC_COMBO_MAGNIFY, "125%", 125);
		host->addComboItem(IDC_COMBO_MAGNIFY, "150%", 150);
		host->addComboItem(IDC_COMBO_MAGNIFY, "175%", 175);
		host->addComboItem(IDC_COMBO_MAGNIFY, "200%", 200);
		host->addComboItem(IDC_COMBO_MAGNIFY, "250%", 250);
		host->addComboItem(IDC_COMBO_MAGNIFY, "300%", 300);
		host->addComboItem(IDC_COMBO_MAGNIFY, "350%", 350);
		host->addComboItem(IDC_COMBO_MAGNIFY, "400%", 400);

		host->setComboSelectedValue(IDC_COMBO_MAGNIFY, 100);

		std::list<std::pair<std::string, int>> rotations;
		rotations = model.getRotations();

		for (auto sz = rotations.begin(); sz != rotations.end(); sz++)
		{
			host->addComboItem(IDC_COMBO_FLIP, sz->first, sz->second);
		}

		std::list<std::string> sizes;
		sizes = model.getSizes();
		for (auto sz = sizes.begin(); sz != sizes.end(); sz++)
		{
			host->addComboItem(IDC_COMBO_SIZE1, *sz, 0);
			host->addComboItem(IDC_COMBO_SIZE2, *sz, 0);
			host->addComboItem(IDC_COMBO_SIZE3, *sz, 0);
		}

		host->setComboSelectedIndex(IDC_EDIT_SPRITE_ORIENTATION, 0);
		host->addPicturesFoldersToCombo(IDC_DIRECTORY_SELECT);

		host->setSpinRange(IDC_SPIN_BRIGHT, -100, 100);
		host->setSpinRange(IDC_SPIN_CONTRAST, -100, 100);
		host->setSpinRange(IDC_SPIN_HTHRESHOLD, -100, 100);

#if false
		try {
			yankeerinoStuff = host->readInternet("www.yankeerino.com", "sitemap.bhs?details=1");
			for (auto ys = yankeerinoStuff.begin(); ys != yankeerinoStuff.end(); ys++) {
				std::vector<std::string> splits = convertDto::split(*ys, "\t");
				yankeeRinoAdvertisement yra;
				yra.url = splits[0];
				yra.description = splits[1];
				yankeerinoAdvertisements.push_back(yra);
			}
		}
		catch (errorDto dto)
		{
			host->setEditText(IDC_EDIT_STATUS_MSG, dto.message);
		}
#endif

		enableEditMessages = true;

	}

	void wsProposalController::randomAdvertisement()
	{
		char buffer[16384];
		if (yankeerinoAdvertisements.size() > 0) {
			int index = rand() % yankeerinoAdvertisements.size();
			auto& ad = yankeerinoAdvertisements[index];
			sprintf(buffer, "This Program is by CountryBit - Check Out This Story: <a href=\"%s\">%s</a>", ad.url.c_str(), ad.description.c_str());
			host->setSysLinkText(IDC_ADVERTISING_LINK, buffer);
		}
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
		if (controlId != IDC_DIRECT2D)
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
		std::string converted = convertDto::toString(newPositionF);
		switch (controlId) {
		case IDC_SPIN_BRIGHT:
			host->setEditText(IDC_EDIT_BRIGHT, converted);
			break;
		case IDC_SPIN_CONTRAST:
			host->setEditText(IDC_EDIT_CONTRAST, converted);
			break;
		case IDC_SPIN_HTHRESHOLD:
			host->setEditText(IDC_EDIT_CHROMA_THRESHOLD, converted);
			break;
		}

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
		model.arrangeImages();
		auto rect = host->getWindowPos(IDC_DIRECT2D);
		auto bmSize = model.getBitmapSize();

		bool bSave = enableEditMessages;
		enableEditMessages = false;
		host->setComboSelectedValue(IDC_CREATE_SIZE, bmSize.width);
		enableEditMessages = bSave;

		int w = (bmSize.width * (magnification / 100.0) - rect.width) + 16 * (magnification / 100.0);
		if (w < 0) w = 0;
		if (w == 0)
			currentScroll.x = 0;
		host->setScrollWidth(IDC_DIRECT2D, w);
		int h = (bmSize.height * (magnification / 100.0) - rect.height) + 16 * (magnification / 100.0);
		if (h < 0) h = 0;
		if (h == 0)
			currentScroll.y = 0;
		host->setScrollHeight(IDC_DIRECT2D, h);
	}

	void wsProposalController::mouseClick(pointDto* _point)
	{
		pointDto p = *_point;
		p.x /= (magnification / 100.0);
		p.y /= (magnification / 100.0);

		p.x += currentScroll.x / (magnification / 100.0);
		p.y += currentScroll.y / (magnification / 100.0);

		auto instances = model.getImagesArranged();

		for (auto i = instances.begin(); i != instances.end(); i++) {
			if (i->contains(p)) {
				host->setListViewSelectedText(IDC_LIST_IMAGES, i->bitmapName);
				break;
			}
		}
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
			case currentWindowViews::viewLoading:
			{
				int mx = 400, my = 110;
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
				txt.text = "Loading Images, Please Wait.";
				txt.layout = frameLayout;
				host->getDrawable(0)->drawText(&txt);
				txt.position.y += 30;
				txt.rotation = 0.0;
				txt.text = currentImageStatus;
				txt.layout = frameLayout;
				host->getDrawable(0)->drawText(&txt);
			}
			break;
			case currentWindowViews::viewProcessing:
			case currentWindowViews::viewRunning:
			{
				fact.colorMake(f, f, f, 1.0);
				host->getDrawable(0)->clear(&fact.color);

				pointDto camera;
				camera.x = -currentScroll.x / (magnification / 100.0);
				camera.y = -currentScroll.y / (magnification / 100.0);
				host->getDrawable(0)->pushCamera(&camera, 0.0, magnification / 100.0);

				// draw the paper background
				pathImmediateDto pim;
				auto size = model.getBitmapSize();
				fact.rectangleMake(&pim, paperMargin - 1, paperMargin - 1, size.width + 1, size.height + 1, "border", style->getWhiteBrushName(), style->getGreyBrushName(), 1.0);
				host->getDrawable(0)->drawPath(&pim);

				// then the bitmaps on top
				auto instances = model.getImagesArranged();

				for (auto i = instances.begin(); i != instances.end(); i++) {
					bitmapInstanceDto dto = *i;
					dto.x += paperMargin;
					dto.y += paperMargin;
					host->getDrawable(0)->drawBitmap(&dto);

					if (!this->previewMode) {
						/*					fact.rectangleMake( &pim, dto.x, dto.y + dto.height - 16, dto.width, 16, "black", style->getBlackWashBrushName(), NULL, 0 );
											host->drawPath( &pim );

											textInstance2dDto txt;
											this->getDataStyles( &txt );
											txt.fillBrushName = style->getWhiteBrushName();
											txt.position.x = dto.x;
											txt.position.y = dto.y + dto.height - 16;
											txt.rotation = 0.0;
											txt.text = dto.bitmapName;
											txt.layout = frameLayout;
											host->drawText( &txt );
						*/
						if (!dto.selected) {
							fact.rectangleMake(&pim, dto.x, dto.y, dto.width, dto.height, "border", style->getWhiteWashBrushName(), NULL, 0);
							host->getDrawable(0)->drawPath(&pim);
						}
					}
				}
				host->getDrawable(0)->popCamera();

				if (currentWindowView == currentWindowViews::viewProcessing) {
					int mx = 400, my = 110;

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
					txt.text = "Processing Images, Please Wait.";
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);
					txt.position.y += 30;
					txt.rotation = 0.0;
					txt.text = currentImageStatus;
					txt.layout = frameLayout;
					host->getDrawable(0)->drawText(&txt);
				}
			}
			}

			pathImmediateDto border;
			auto sizeWindow = this->host->getDrawable(0)->getSize();
			fact.rectangleMake(&border, 0.0, 0.0, sizeWindow.width, sizeWindow.height, "border", NULL, style->getBlackBrushName(), 1);
			host->getDrawable(0)->drawPath(&border);

		}
		catch (directException exc)
		{
			host->setEditText(IDC_EDIT_STATUS_MSG, exc.message);
		}
	}

	void wsProposalController::spriteSheetExportBitmap(const char* _filenameImage)
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

	void wsProposalController::spriteSheetExportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
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
		bool tempB;
		errorDto err;
		std::list< std::pair<bitmapDto*, spriteStripFilter> > modifiedBitmapsTakei;
		std::list<bitmapDto*> modifiedBitmaps;
		std::string saveFileName;

		switch (buttonId) {
		case IDC_BTN_CHROMA_EDIT:
			host->setColorCapture(IDC_CURSOR_COLOR_PICK);
			err.message = "Select a color to use for the chroma key.";
			break;
		case IDC_CHECK_CROP:
			tempB = host->getButtonChecked(IDC_CHECK_CROP);
			host->setEnable(IDC_EDIT_CROP_MARGINS, tempB);
			err = model.setCropEnable(tempB, modifiedBitmaps);
			for (auto i = modifiedBitmaps.begin(); i != modifiedBitmaps.end(); i++) {
				host->getDrawable(0)->setBitmapSizes(*i, true);
			}
			break;
		case IDC_CHROMA_ENABLE:
			tempB = host->getButtonChecked(IDC_CHROMA_ENABLE);
			host->setEnable(IDC_EDIT_CHROMA_COLOR, tempB);
			host->setEnable(IDC_EDIT_CHROMA_THRESHOLD, tempB);
			host->setEnable(IDC_BTN_CHROMA_EDIT, tempB);
			err = model.setChromaEnable(tempB, modifiedBitmapsTakei);
			for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
				host->getDrawable(0)->setBitmapFilter(i->first, i->second);
			}
			break;
		case IDC_BUTTON_TOGGLE_PREVIEW:
			this->previewMode = !this->previewMode;
			if (this->previewMode)
				host->setEditText(IDC_BUTTON_TOGGLE_PREVIEW, "Show Selection");
			else
				host->setEditText(IDC_BUTTON_TOGGLE_PREVIEW, "Show Preview");
			break;
		case IDC_SAVE:
			if (host->getSaveFilename(saveFileName, "png image (*.png)\0*.png\0", "png")) {
				this->spriteSheetExportBitmap(saveFileName.c_str());
				err.message = saveFileName + " saved.";
				err.error = false;
				errorDto error2 = model.exportRectangleFile(saveFileName.c_str());
				if (error2.error)
					err = error2;
			}
		}
		host->redraw();
		host->setEditText(IDC_EDIT_STATUS_MSG, err.message);

	}

	void wsProposalController::onTextChanged(int textControlId)
	{
		if (textControlId == IDC_EDIT_STATUS_MSG ||
			!enableEditMessages)
			return;

		errorDto err;
		std::string temp;
		std::list<bitmapDto*> modifiedBitmaps;
		std::list< std::pair<bitmapDto*, spriteStripFilter> > modifiedBitmapsTakei;

		showUpdate = true;

		switch (textControlId) {

		case IDC_EDIT_BRIGHT:
			temp = host->getEditText(IDC_EDIT_BRIGHT);
			err = model.setBrightAdjust(temp, modifiedBitmapsTakei);
			if (!err.error) {
				for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
					host->getDrawable(0)->setBitmapFilter(i->first, i->second);
				}
				auto spinPos = (int)(model.getBrightAdjustFloat() * 100);
				host->setSpinPos(IDC_SPIN_BRIGHT, spinPos);
			}
			break;

		case IDC_EDIT_CONTRAST:
			temp = host->getEditText(IDC_EDIT_CONTRAST);
			err = model.setContrastAdjust(temp, modifiedBitmapsTakei);
			if (!err.error) {
				for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
					host->getDrawable(0)->setBitmapFilter(i->first, i->second);
				}
				auto spinPos = (int)(model.getContrastAdjustFloat() * 100);
				host->setSpinPos(IDC_SPIN_CONTRAST, spinPos);
			}
			break;

		case IDC_EDIT_CROP_MARGINS:
			temp = host->getEditText(IDC_EDIT_CROP_MARGINS);
			err = model.setCropMargins(temp, modifiedBitmaps);
			if (!err.error) {
				for (auto i = modifiedBitmaps.begin(); i != modifiedBitmaps.end(); i++) {
					host->getDrawable(0)->setBitmapSizes(*i, true);
				}
			}
			setScrollBars();
			break;

		case IDC_EDIT_CHROMA_COLOR:
			temp = host->getEditText(IDC_EDIT_CHROMA_COLOR);
			err = model.setChromaY(temp, modifiedBitmapsTakei);
			if (!err.error) {
				for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
					host->getDrawable(0)->setBitmapFilter(i->first, i->second);
				}
			}
			break;

		case IDC_EDIT_CHROMA_THRESHOLD:
			temp = host->getEditText(IDC_EDIT_CHROMA_THRESHOLD);
			err = model.setChromaThreshold(temp, modifiedBitmapsTakei);
			if (!err.error) {
				for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
					host->getDrawable(0)->setBitmapFilter(i->first, i->second);
				}
				auto spinPos = (int)(model.getChromaThresholdFloat() * 100);
				host->setSpinPos(IDC_SPIN_HTHRESHOLD, spinPos);
			}
			fromImage();
			break;

		case IDC_EDIT_SPRITE_NAME:
			temp = host->getEditText(IDC_EDIT_SPRITE_NAME);
			break;

		case IDC_EDIT_SPRITE_STATE:
			temp = host->getEditText(IDC_EDIT_SPRITE_STATE);
			break;

		case IDCE_EDIT_SPRITE_FRAME:
			temp = host->getEditText(IDCE_EDIT_SPRITE_FRAME);
			break;

		case IDCE_EDIT_SPRITE_DELAY:
			temp = host->getEditText(IDCE_EDIT_SPRITE_DELAY);
			break;
		}

		host->setEditText(IDC_EDIT_STATUS_MSG, err.message);

	}

	void wsProposalController::onDropDownChanged(int dropDownId)
	{
		std::string directory;
		int ctr = 0;
		std::string temp;
		std::list<std::pair<std::string, int>> imageSets;

		errorDto err;
		bool tempB;
		int tempI;
		std::list<bitmapDto*> modifiedBitmaps;
		std::list< std::pair<bitmapDto*, spriteStripFilter> > modifiedBitmapsTakei;
		std::string errorMessage = "";
		int counter = 0;
		int percent = 0;
		char buff[256];

		showUpdate = true;

		switch (dropDownId) {
		case IDC_DIRECTORY_SELECT:
		{
			currentWindowView = currentWindowViews::viewLoading;
			host->setScrollPos(IDC_DIRECT2D, pointDto(0, 0));
			currentScroll.x = 0;
			currentScroll.y = 0;
			currentImageStatus = "";

			host->getDrawable(0)->clearBitmapsAndBrushes();
			directory = host->getComboSelectedText(IDC_DIRECTORY_SELECT);
			model.loadImages(directory.c_str());
			std::list<std::string> list = model.getImageNames();
			host->clearListView(IDC_LIST_IMAGES);
			for (auto i = list.begin(); i != list.end(); i++) {
				host->addListViewItem(IDC_LIST_IMAGES, *i, ctr++);
			}
			host->setListViewSelectedIndex(IDC_LIST_IMAGES, 0);
			std::list<bitmapDto*> images = model.getImages();
			for (auto i = images.begin(); i != images.end(); i++) {
				host->getDrawable(0)->addBitmap(*i);
				counter++;
				percent = counter * 100 / images.size();
				sprintf(buff, "%d%% complete", percent);
				currentImageStatus = buff;
				host->redraw();
			}
			tempI = host->getComboSelectedValue(IDC_CREATE_SIZE);
			model.setBitmapWidth(tempI);
			model.arrangeImages();
			setScrollBars();
			currentWindowView = currentWindowViews::viewRunning;
			err.message = "Images Loaded";
			host->setFocus(IDC_LIST_IMAGES);
		}
		randomAdvertisement();
		break;

		case IDC_CREATE_SIZE:
			tempI = host->getComboSelectedValue(IDC_CREATE_SIZE);
			err = model.setBitmapWidth(tempI);
			model.arrangeImages();
			setScrollBars();
			randomAdvertisement();
			break;

		case IDC_CREATE_TYPE:
			tempI = host->getComboSelectedValue(IDC_CREATE_TYPE);
			err = model.selectProjectType((spriteStripProjectTypes)tempI);

			/*
			imageSets = model.getSettings();
			host->clearComboItems( IDC_SETTINGS_SET );
			for (auto sz = imageSets.begin(); sz != imageSets.end(); sz++)
			{
				host->addComboItem( IDC_SETTINGS_SET, sz->first, sz->second );
			}
			host->setComboSelectedIndex( IDC_SETTINGS_SET, 0 );
			*/
			model.arrangeImages();
			setScrollBars();
			randomAdvertisement();
			fromImage();
			break;

		case IDC_COMBO_FLIP:
			tempI = host->getComboSelectedValue(IDC_COMBO_FLIP);
			err = model.setRotation(tempI, modifiedBitmapsTakei);
			for (auto i = modifiedBitmapsTakei.begin(); i != modifiedBitmapsTakei.end(); i++) {
				host->getDrawable(0)->setBitmapFilter(i->first, i->second);
			}
			break;

		case IDC_COMBO_SIZE1:
			currentWindowView = currentWindowViews::viewProcessing;
			host->setScrollPos(IDC_DIRECT2D, pointDto(0, 0));
			currentScroll.x = 0;
			currentScroll.y = 0;
			currentImageStatus = "";
			temp = host->getComboSelectedText(IDC_COMBO_SIZE1);
			err = model.setSize1(temp, modifiedBitmaps);
			for (auto i = modifiedBitmaps.begin(); i != modifiedBitmaps.end(); i++) {
				host->getDrawable(0)->setBitmapSizes(*i, false);
				model.arrangeImages();
				counter++;
				percent = counter * 100 / modifiedBitmaps.size();
				sprintf(buff, "%d%% complete", percent);
				currentImageStatus = buff;
				host->redraw();
			}
			setScrollBars();
			currentWindowView = currentWindowViews::viewRunning;
			host->redraw();
			break;


			/* get 'em in the next release!
		case IDC_SET_SELECT:
			{
				int newSet = host->getComboSelectedValue( IDC_SET_SELECT );
				model.selectSettings( newSet );
				fromImage();
				host->redraw();
				err.error = false;
				err.message = "Selected set.";
			}
			break;
			*/

		case IDC_COMBO_MAGNIFY:
		{
			int newMagnification = host->getComboSelectedValue(IDC_COMBO_MAGNIFY);
			currentScroll.x *= newMagnification / (float)magnification;
			currentScroll.y *= newMagnification / (float)magnification;
			magnification = newMagnification;
			setScrollBars();
			host->redraw();
			err.error = false;
			err.message = "Magnification changed.";
		}
		break;

		}

		host->setEditText(IDC_EDIT_STATUS_MSG, err.message);

	}

	void wsProposalController::onListViewChanged(int listViewId)
	{
		std::list< std::string> temp;
		errorDto errMsg;
		switch (listViewId) {
		case IDC_LIST_IMAGES:
			temp = host->getListViewSelectedTexts(IDC_LIST_IMAGES);
			errMsg = model.selectImages(temp);
			fromImage();
			host->setEditText(IDC_EDIT_STATUS_MSG, errMsg.message);
			setScrollBars();
			host->redraw();
			break;
		}
	}

	void spriteStripArtistController::fromImage()
	{
		std::string temp;
		bool tempB;
		int tempI;

		enableEditMessages = false;

		temp = model.getImageName();
		host->setEditText(IDC_EDIT_FILENAME, temp);

		tempI = model.getRotation();
		host->setComboSelectedValue(IDC_COMBO_FLIP, tempI);

		temp = model.getSize1();
		host->setComboSelectedText(IDC_COMBO_SIZE1, temp);

		temp = model.getBrightAdjust();
		host->setEditText(IDC_EDIT_BRIGHT, temp);

		temp = model.getContrastAdjust();
		host->setEditText(IDC_EDIT_CONTRAST, temp);

		tempB = model.getCropEnable();
		host->setButtonChecked(IDC_CHECK_CROP, tempB);
		host->setEnable(IDC_EDIT_CROP_MARGINS, tempB);
		host->setEnable(IDC_BTN_CROP_EDIT, tempB);

		temp = model.getCropMargins();
		host->setEditText(IDC_EDIT_CROP_MARGINS, temp);

		tempB = model.getChromaEnable();
		host->setButtonChecked(IDC_CHROMA_ENABLE, tempB);
		host->setEnable(IDC_EDIT_CHROMA_COLOR, tempB);
		host->setEnable(IDC_EDIT_CHROMA_THRESHOLD, tempB);
		host->setEnable(IDC_BTN_CHROMA_EDIT, tempB);

		temp = model.getChromaY();
		host->setEditText(IDC_EDIT_CHROMA_COLOR, temp);

		temp = model.getChromaThreshold();
		host->setEditText(IDC_EDIT_CHROMA_THRESHOLD, temp);

		temp = model.getSpriteName();
		host->setEditText(IDC_EDIT_SPRITE_NAME, temp);

		temp = model.getSpriteState();
		host->setEditText(IDC_EDIT_SPRITE_STATE, temp);

		temp = model.getSpriteFrameId();
		host->setEditText(IDC_EDIT_SPRITE_FRAME, temp);

		temp = model.getSpriteFrameDelay();
		host->setEditText(IDC_EDIT_SPRITE_DELAY, temp);

		enableEditMessages = true;
	}

	void wsProposalController::pointSelected(pointDto* _point, colorDto* _color)
	{
		std::string colorString = convertDto::toString(_color);
		host->setEditText(IDC_EDIT_CHROMA_COLOR, colorString);
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

