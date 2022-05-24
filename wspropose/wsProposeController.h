#pragma once

namespace corona
{
	namespace win32
	{
		enum currentWindowViews {
			viewEmpty,
			viewLoading,
			viewProcessing,
			viewRunning
		};

		struct yankeeRinoAdvertisement {
			std::string url;
			std::string description;
		};

		class wsProposalController : public win32::controller {

			currentWindowViews currentWindowView;
			std::string currentImageStatus;
			std::vector<std::string> yankeerinoStuff;
			std::vector<yankeeRinoAdvertisement> yankeerinoAdvertisements;

			bool enableEditMessages;
			bool showUpdate;
			bool previewMode;
			int magnification;

			void fromImage();
			void clearErrors(errorDto* _error);
			void addError(errorDto* _error);
			void setScrollBars();

			// for sprite sheets
			void exportBitmap(const char* _filenameImage);
			void exportBitmapRectangles(const char* _filenameImage, const char* _templateFile);

			pointDto currentScroll;

			void randomAdvertisement();

		public:

			wsProposalController(viewStyle* _vs);
			virtual ~wsProposalController();

			virtual void loadController();
			virtual void keyDown(short _key);
			virtual void keyUp(short _key);
			virtual void mouseMove(pointDto* _point);
			virtual void mouseClick(pointDto* _point);
			virtual void drawFrame();
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void onInit();
			virtual void onCommand(int buttonId);
			virtual void onTextChanged(int textControlId);
			virtual void onDropDownChanged(int dropDownId);
			virtual void onListViewChanged(int listViewId);
			virtual int onHScroll(int controlId, scrollTypes scrollType);
			virtual int onVScroll(int controlId, scrollTypes scrollType);
			virtual int onResize(const rectDto& newSize);
			virtual int onSpin(int controlId, int newPosition);
			virtual void pointSelected(pointDto* _point, colorDto* _color);

		};
	}

}
