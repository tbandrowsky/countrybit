
#pragma once

namespace corona
{
	namespace win32
	{
		class viewStyle;

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

		protected:

			controllerHost* host;
			viewStyle* style;

		public:

			colorDto backgroundColor;

			controller(viewStyle* _style);
			virtual ~controller();

			inline controllerHost* getHost() { return host; }
			inline drawableHost* getDrawable() { return host->getDrawable(0); }
			virtual void attach(controllerHost* _host);

			void baseLoadController();

			// these are for 

			virtual void loadController() = 0;
			virtual void keyDown(short _key) = 0;
			virtual void keyUp(short _key) = 0;
			virtual void mouseMove(pointDto* _point) = 0;
			virtual void mouseClick(pointDto* _point) = 0;
			virtual void pointSelected(pointDto* _point, colorDto* _color) = 0;
			virtual void drawFrame() = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onInit() = 0;
			virtual void onCreated(const rectDto& newSize) = 0;
			virtual void onCommand(int buttonId) = 0;
			virtual void onTextChanged(int textControlId) = 0;
			virtual void onDropDownChanged(int dropDownId) = 0;
			virtual void onListViewChanged(int listViewId) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onResize(const rectDto& newSize) = 0;
			virtual int onSpin(int controlId, int newPosition) = 0;

			// helper stuff for views
			void getH1Styles(textInstance2dDto* _dto);
			void getH2Styles(textInstance2dDto* _dto);
			void getH3Styles(textInstance2dDto* _dto);
			void getLabelStyles(textInstance2dDto* _dto);
			void getDataStyles(textInstance2dDto* _dto);

		};

		enum currentWindowViews {
			viewEmpty,
			viewLoading,
			viewProcessing,
			viewRunning
		};

		struct advertisement {
			std::string url;
			std::string description;
		};

		class corona_controller : public controller
		{
		protected:

			currentWindowViews currentWindowView;
			std::string currentImageStatus;
			std::vector<advertisement> advertisements;

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

			corona::database::dynamic_box box;
			corona::database::jschema schema;
			corona::database::relative_ptr_type schema_id;
			corona::database::jcollection program_chart;
			corona::database::jactor sample_actor;
			corona::database::page pg;
			corona::database::actor_state state;

			int canvasWindowsId;

		public:

			corona_controller(viewStyle* _style)
				: controller(_style),
				showUpdate(false),
				currentWindowView(currentWindowViews::viewEmpty),
				previewMode(false),
				magnification(100)
			{
				;
			}

			virtual ~corona_controller();

			virtual void for_each(std::function<bool(const database::actor_view_collection::iterator_item_type& _item)> selector, std::function<bool(database::actor_view_object& avo, database::jslice& slice)> updator);
			virtual void for_each(database::relative_ptr_type class_id, std::function<bool(const database::actor_view_object& avo, database::jslice& slice)>  updator);
			virtual void for_each(database::jslice _parent, database::relative_ptr_type *_join_fields, std::function<bool(const database::actor_view_object& avo, database::jslice& slice)>  updator);

			virtual void stateChanged(const rectDto& newSize) = 0;
			virtual void loadController() = 0;
			virtual void drawFrame() = 0;
			virtual void onInit() = 0;

			virtual void keyDown(short _key);
			virtual void keyUp(short _key);
			virtual void mouseMove(pointDto* _point);
			virtual void mouseClick(pointDto* _point);
			virtual void pointSelected(pointDto* _point, colorDto* _color);
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void onCreated(const rectDto& newSize);
			virtual void onCommand(int buttonId);
			virtual void onTextChanged(int textControlId);
			virtual void onDropDownChanged(int dropDownId);
			virtual void onListViewChanged(int listViewId);
			virtual int onHScroll(int controlId, scrollTypes scrollType);
			virtual int onVScroll(int controlId, scrollTypes scrollType);
			virtual int onResize(const rectDto& newSize);
			virtual int onSpin(int controlId, int newPosition);
		};

	}
}

