
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

			virtual jobject get_style_sheet(int _idx) = 0;
			virtual void keyDown(direct2dWindow *win, short _key) = 0;
			virtual void keyUp(direct2dWindow* win, short _key) = 0;
			virtual void mouseMove(direct2dWindow* win, point* _point) = 0;
			virtual void mouseClick(direct2dWindow* win, point* _point) = 0;
			virtual void pointSelected(direct2dWindow *win, point* _point, color* _color) = 0;
			virtual bool drawFrame() = 0;
			virtual bool drawCanvas(int _id) = 0;
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

		struct advertisement 
		{
			std::string url;
			std::string description;
		};

		struct table_column 
		{
		public:
			object_name			title;
			object_name			subtitle;
			double				width;
			relative_ptr_type	field_id;
		};

		struct table_options 
		{
		public:
			view_query				  *data;
			bool					  alternating_row;
			measure					  header_height;
			measure					  row_height;
			std::vector<table_column> columns;
			relative_ptr_type		  header_config_id;
			jobject					  header_config;
			relative_ptr_type		  sort_field_id;

			table_options() : data(nullptr)
				, alternating_row(false)
				, header_height({ 1.2_fontgr })
				, row_height({ 1.2_fontgr })
				, header_config_id(null_row)
				, sort_field_id(null_row)
			{
				;
			}
		};

		struct edit_field
		{
		public:
			object_name				  label;
			relative_ptr_type		  field_id;
			int						  column_id;

			edit_field() : field_id(null_row), column_id(0)
			{
				;
			}

			edit_field( const char *_label ) : label(_label), field_id(null_row), column_id(0)
			{
				;
			}

			edit_field(const char* _label, relative_ptr_type _field_id) : label(_label), field_id(_field_id), column_id(0)
			{
				;
			}

			edit_field(const char* _label, relative_ptr_type _field_id, int _column) : label(_label), field_id(_field_id), column_id(_column)
			{
				;
			}
		};

		struct edit_options
		{
		public:
			view_query* data;
			object_name				  form_title;
			std::vector<edit_field> fields;
		};

		class corona_controller;

		std::function<void(drawableHost* _cont, page_item* _item)> render_handler;

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

			virtual jobject get_style_sheet(int _index);

			// methods to use in render

			virtual void clear();

			page_item* row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 1.0_remaining }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* hcenter(page_item* _parent, relative_ptr_type _style_id);
			page_item* absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });
			page_item* space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });
			page_item* canvas2d_row(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* canvas2d_column(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* canvas2d_absolute(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px }, measure _item_space = { 0.0, measure_units::pixels }, visual_alignment _alignment = visual_alignment::align_near);
			page_item* navigate(page_item* _parent, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box);
			page_item* text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });
			page_item* table_header(page_item* _parent, actor_state* _state, const char* _caption, relative_ptr_type object_id, jobject slice, relative_ptr_type field_id, relative_ptr_type sort_field_id, relative_ptr_type _style_id, layout_rect _box);
			page_item* table_cell(page_item* _parent, actor_state* _state, relative_ptr_type object_id, jobject slice, relative_ptr_type field_id, relative_ptr_type _style_id, layout_rect _box);
			page_item* set(page_item* _parent, actor_state* _state, const object_member_path path, int field_id, dynamic_value dv, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });
			page_item* select(page_item* _parent, actor_state* _state, relative_ptr_type object_id, relative_ptr_type _id_name, jobject slice, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 1.0_remaining, 100.0_px });

			virtual void breadcrumbs(page_item* _parent, std::function<const char* (jobject& slice)> _captioner, layout_rect _item_box = { 0.0_px, 0.0_px, 200.0_px, 100.0_px });

			virtual page_item *edit_fields(page_item* _parent, const object_member_path& _omp, field_layout _layout, const edit_options& _fields);
			virtual page_item* create_buttons(page_item* _parent, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 250.0_px, 2.0_fontgr });
			virtual page_item* selectable_items(page_item* _parent, view_query& _vq, relative_ptr_type _style_id, layout_rect _box = { 0.0_px, 0.0_px, 250.0_px, 2.0_fontgr });
			virtual void edit_form(page_item* _navigation, page_item* _frame, relative_ptr_type _title_uiid, const object_member_path& _omp, edit_options _fields);
			virtual void search_form(page_item* _navigation, page_item* _frame, relative_ptr_type _title_uiid, relative_ptr_type _table_uiid, relative_ptr_type _search_class_id, table_options& _options, edit_options _search_options);

			void table(page_item* _parent, table_options& _options);

			void arrange(double width, double height, jobject& _style_sheet, double padding = 0.0);

			// end of render methods

			virtual void render(const rectangle& newSize) = 0;
			virtual void render_item(drawableHost *_host, page_item* _item);

			virtual bool drawFrame();
			virtual bool drawCanvas(int _id);

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

