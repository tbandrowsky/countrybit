
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
			inline drawableHost* getDrawable(int _id) { return host->getDrawable(_id); }
			virtual void attach(controllerHost* _host);

			// these are for 

			virtual jobject getStyleSheet() = 0;
			virtual void keyDown(direct2dWindow *win, short _key) = 0;
			virtual void keyUp(direct2dWindow* win, short _key) = 0;
			virtual void mouseMove(direct2dWindow* win, point* _point) = 0;
			virtual void mouseClick(direct2dWindow* win, point* _point) = 0;
			virtual void pointSelected(direct2dWindow *win, point* _point, color* _color) = 0;
			virtual bool drawFrame() = 0;
			virtual bool drawItem(int _id) = 0;
			virtual bool update(double _elapsedSeconds, double _totalSeconds) = 0;

			virtual void onCreated() = 0;
			virtual void onCommand(int buttonId, page_item pi) = 0;
			virtual void onTextChanged(int textControlId, page_item pi) = 0;
			virtual void onDropDownChanged(int dropDownId, page_item pi) = 0;
			virtual void onListViewChanged(int listViewId, page_item pi) = 0;
			virtual int onHScroll(int controlId, scrollTypes scrollType, page_item pi) = 0;
			virtual int onVScroll(int controlId, scrollTypes scrollType, page_item pi) = 0;
			virtual int onResize(const rectangle& newSize, double d2dScale) = 0;
			virtual int onSpin(int controlId, int newPosition, page_item pi) = 0;
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
			jcollection user_collection;
			page pg;
			actor_state state;
			actor_id_type actor_id;
			analytics_kit analytics;

			int canvasWindowsId;

			void stateChanged(const rectangle& newSize);
			const char* style_id(relative_ptr_type _style_field_id);

		public:

			corona_controller()
				: 
				showUpdate(false),
				currentWindowView(currentWindowViews::viewEmpty),
				previewMode(false),
				magnification(100)
			{
				analytics = analytics_kit(&schema, &state, &user_collection);
			}

			virtual ~corona_controller();

			virtual jobject getStyleSheet();

			// methods to use in render

			virtual void clear();

			page_item* row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct }, measure _item_space = { 0.0, measure_units::pixels });
			page_item* column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_pct }, measure _item_space = { 0.0, measure_units::pixels });
			page_item* absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d_row(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d_column(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* canvas2d_absolute(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* navigate(page_item* _parent, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box);
			page_item* text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box = { 0.0_px, 0.0_px, 100.0_pct, 100.0_px });
			page_item* select_cell(page_item* _parent, actor_state* _state, int object_id, jobject slice, const char* _caption, relative_ptr_type _style_id, layout_rect _box);

			virtual void breadcrumbs(page_item* _parent, std::function<const char* (jobject& slice)> _captioner, layout_rect _item_box = { 0.0_px, 0.0_px, 200.0_px, 100.0_px });

			virtual page_item *add_update_fields(page_item* _parent, field_layout _layout, const char* _object_title);
			virtual page_item* add_create_buttons(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 250.0_px, 2.0_fntgr });
			virtual page_item* add_select_items(page_item* _parent);

			void search_table(page_item* _parent, relative_ptr_type _idc_class_id, relative_ptr_type* _idf_child_fields, int _num_child_fields);

			void arrange(double width, double height, jobject& _style_sheet, double padding = 0.0);

			// end of render methods

			virtual void render(const rectangle& newSize) = 0;
			virtual void render_item(drawableHost *_host, page_item& _item);
			virtual bool drawFrame();
			virtual bool drawItem(int _id);

			virtual void keyDown(direct2dWindow* win, short _key);
			virtual void keyUp(direct2dWindow* win, short _key);
			virtual void mouseMove(direct2dWindow* win, point* _point);
			virtual void mouseClick(direct2dWindow* win, point* _point);
			virtual void pointSelected(direct2dWindow* win, point* _point, color* _color);
			virtual bool update(double _elapsedSeconds, double _totalSeconds);

			virtual void onCreated();
			virtual void onCommand(int buttonId, page_item pi);
			virtual void onTextChanged(int textControlId, page_item pi);
			virtual void onDropDownChanged(int dropDownId, page_item pi);
			virtual void onListViewChanged(int listViewId, page_item pi);
			virtual int onHScroll(int controlId, scrollTypes scrollType, page_item pi);
			virtual int onVScroll(int controlId, scrollTypes scrollType, page_item pi);
			virtual int onResize(const rectangle& newSize, double dpi);
			virtual int onSpin(int controlId, int newPosition, page_item pi);

		};

	}
}

