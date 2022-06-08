
#pragma once

namespace corona
{
	namespace win32
	{

		using namespace database;

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

		public:

			color backgroundColor;

			controller();
			virtual ~controller();

			inline controllerHost* getHost() { return host; }
			inline drawableHost* getDrawable() { return host->getDrawable(0); }
			virtual void attach(controllerHost* _host);

			// these are for 

			virtual jslice getStyleSheet() = 0;
			virtual void keyDown(short _key) = 0;
			virtual void keyUp(short _key) = 0;
			virtual void mouseMove(point* _point) = 0;
			virtual void mouseClick(point* _point) = 0;
			virtual void pointSelected(point* _point, color* _color) = 0;
			virtual void drawFrame() = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onCreated(const rectangle& newSize) = 0;
			virtual void onCommand(int buttonId) = 0;
			virtual void onTextChanged(int textControlId) = 0;
			virtual void onDropDownChanged(int dropDownId) = 0;
			virtual void onListViewChanged(int listViewId) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType) = 0;
			virtual int onResize(const rectangle& newSize) = 0;
			virtual int onSpin(int controlId, int newPosition) = 0;
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
			void clearErrors(base_result* _error);
			void addError(base_result* _error);
			void setScrollBars();

			// for sprite sheets
			void exportBitmap(const char* _filenameImage);
			void exportBitmapRectangles(const char* _filenameImage, const char* _templateFile);

			point currentScroll;

			void randomAdvertisement();

			dynamic_box box;
			jschema schema;
			relative_ptr_type schema_id;
			jcollection program_chart;
			jactor sample_actor;
			page pg;
			actor_state state;

			int canvasWindowsId;

			void stateChanged(const rectangle& newSize);

		public:

			corona_controller()
				: 
				showUpdate(false),
				currentWindowView(currentWindowViews::viewEmpty),
				previewMode(false),
				magnification(100)
			{
				;
			}

			virtual ~corona_controller();

			virtual void clear();
			virtual jslice getStyleSheet();

			page_item* row(page_item* _parent, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			page_item* column(page_item* _parent, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct });
			page_item* absolute(page_item* _parent, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d(page_item* _parent, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h1(page_item* _parent, const char* _text, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h2(page_item* _parent, const char* _text, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* h3(page_item* _parent, const char* _text, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* space(page_item* _parent, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });

			virtual void for_each(std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector, std::function<bool(actor_view_object& avo, jslice& slice)> updator);
			virtual void for_each(relative_ptr_type class_id, std::function<bool(const actor_view_object& avo, jslice& slice)>  updator);
			virtual void for_each(jslice& _parent, relative_ptr_type* _join_fields, std::function<bool(const actor_view_object& avo, jslice& slice)>  updator);
			virtual void for_each(relative_ptr_type* _has_field_list, std::function<bool(const actor_view_object& avo, jslice& slice)>  updator);

			virtual page_item *row(page_item* _parent_ui, layout_rect _box, std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector);
			virtual page_item* row(page_item* _parent_ui, layout_rect _box, relative_ptr_type class_id);
			virtual page_item* row(page_item* _parent_ui, layout_rect _box, jslice& _parent, relative_ptr_type* _join_fields);
			virtual page_item* row(page_item* _parent_ui, layout_rect _box, relative_ptr_type* _has_field_list);

			virtual page_item* column(page_item* _parent_ui, layout_rect _box, std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector);
			virtual page_item* column(page_item* _parent_ui, layout_rect _box, relative_ptr_type class_id);
			virtual page_item* column(page_item* _parent_ui, layout_rect _box, jslice& _parent, relative_ptr_type* _join_fields);
			virtual page_item* column(page_item* _parent_ui, layout_rect _box, relative_ptr_type* _has_field_list);

			virtual void render(const rectangle& newSize) = 0;
			virtual void render_item(drawableHost *_host, page_item& _item) = 0;
			virtual void drawFrame();

			virtual void keyDown(short _key);
			virtual void keyUp(short _key);
			virtual void mouseMove(point* _point);
			virtual void mouseClick(point* _point);
			virtual void pointSelected(point* _point, color* _color);
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void onCreated(const rectangle& newSize);
			virtual void onCommand(int buttonId);
			virtual void onTextChanged(int textControlId);
			virtual void onDropDownChanged(int dropDownId);
			virtual void onListViewChanged(int listViewId);
			virtual int onHScroll(int controlId, scrollTypes scrollType);
			virtual int onVScroll(int controlId, scrollTypes scrollType);
			virtual int onResize(const rectangle& newSize);
			virtual int onSpin(int controlId, int newPosition);

		};

	}
}

