
#include "corona.h"

#ifdef WINDESKTOP_GUI

#define TRACE_CONTROLLER 1
#define TRACE_RENDER 0

namespace corona
{
	namespace win32
	{
		using namespace database;

		controller::controller()
		{
			backgroundColor.alpha = 1.0;
			backgroundColor.red = 1.0;
			backgroundColor.green = 1.0;
			backgroundColor.blue = 1.0;
		}

		controller::~controller()
		{
			;
		}

		void controller::attach(controllerHost* _host)
		{
			host = _host;
		}

		corona_controller::~corona_controller()
		{
			;
		}

		jobject corona_controller::get_style_sheet(int _index)
		{
			return user_collection.get_style_sheet()
				.get_object_by_class(schema.idc_style_sheet)
				.get_at(_index);
		}

		void corona_controller::onCreated()
		{
			enableEditMessages = false;

			auto pos = host->getWindowClientPos();

//			host->setMinimumWindowSize(point{ pos.w - pos.x, pos.h - pos.y });

			state = user_collection.get_actor_state(actor_id);

			stateChanged(pos);

			enableEditMessages = true;
		}

		void corona_controller::randomAdvertisement()
		{
		}

		void corona_controller::keyDown(direct2dWindow* win, short _key)
		{
			;
		}

		void corona_controller::keyUp(direct2dWindow* win, short _key)
		{
			;
		}

		void corona_controller::mouseMove(direct2dWindow* win, point* _point)
		{
			for (auto pgi : pg) {
				pgi.item.mouse_over = false;
			}

			auto over_items = pg.where([this, _point](const auto& pi) { 
				return pi.item.is_command() && 
				rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });

			for (auto ov : over_items)
			{
				ov.item.mouse_over = true;
			}
			host->redraw();
		}

		int corona_controller::onHScroll(int controlId, scrollTypes scrollType, page_item pi)
		{
			if (controlId != canvasWindowsId)
				return 0;

			rectangle r = host->getWindowPos(controlId);
			int pos = host->getScrollPos(controlId).x;
			int max = host->getScrollRange(controlId).x;

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

		int corona_controller::onVScroll(int controlId, scrollTypes scrollType, page_item pi)
		{
			if (controlId != canvasWindowsId)
				return 0;

			rectangle r = host->getWindowPos(controlId);
			int pos = host->getScrollPos(controlId).y;
			int max = host->getScrollRange(controlId).y;

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

		int corona_controller::onSpin(int controlId, int newPosition, page_item pi)
		{
			double newPositionF = newPosition / 100.0;
			return 0;
		}

		int corona_controller::onResize(const rectangle& newSize, double d2dScale)
		{
			setScrollBars();

			state = user_collection.get_actor_state(state.actor_id, null_row, "state");
			stateChanged(newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(direct2dWindow *win, point* _point)
		{
			auto clicked_items = pg.where([this, _point](const auto& pi) { return pi.item.is_command() && rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });
			auto size = host->getWindowClientPos();

#if TRACE_CONTROLLER
			std::cout << std::format("clicked {},{}", _point->x, _point->y) << std::endl;
#endif 

			if (clicked_items != std::end(pg))
			{
				auto clicked_item = clicked_items.get_object();

#if TRACE_CONTROLLER
				std::cout << std::format("{} clicked", clicked_item.item.id) << std::endl;
#endif 

				if (clicked_item.item.is_select())
				{

#if TRACE_CONTROLLER
					state = this->user_collection.select_object(clicked_item.item.select_request, "selected via mouse click");
#else
					state = this->program_chart.select_object(clicked_item.item.select_request);
#endif
				}
				else if (clicked_item.item.is_create())
				{
#if TRACE_CONTROLLER
					state = this->user_collection.create_object(clicked_item.item.create_request, "created via mouse click");
#else
					state = this->program_chart.create_object(clicked_item.item.create_request);
#endif
				}
				else if (clicked_item.item.is_set())
				{
					auto obj = this->user_collection.get_object(clicked_item.item.object_path);
					obj.set({ clicked_item.item.dest_value });
					update_object_request uor;
					uor.path = clicked_item.item.object_path;
					uor.item = obj;
					uor.actor_id = this->actor_id;
#if TRACE_CONTROLLER
					state = this->user_collection.update_object( uor, "updated via mouse click");
#else
					state = this->program_chart.update_object(uor);
#endif
				}

				stateChanged(size);
			}
		}

		void corona_controller::exportBitmap(const char* _filenameImage)
		{
			point dt = { 100, 100 };
			auto bm = host->getDrawable(0)->createBitmap(dt);

			color c;
			c.alpha = 1.0;
			c.red = c.green = c.blue = 0.0;

			bool blown_away;

			bm->beginDraw(blown_away);
			bm->clear(&c);
			bm->endDraw(blown_away);

			bm->save(_filenameImage);

			delete bm;
		}

		void corona_controller::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
		{
			;
		}

		page_item* corona_controller::navigate(page_item* _parent, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box)
		{
			return pg.navigate( _parent, &state, object_id, _style_id, _caption, _box);
		}

		void corona_controller::breadcrumbs(page_item* _breadcrumb_container, std::function<const char* (jobject& slice)> _captioner, layout_rect _item_box)
		{
			for (auto bc : state.actor.breadcrumb)
			{
				jobject obj = user_collection.get_object(bc.item);
				const char* caption = _captioner(obj);
				navigate(_breadcrumb_container, bc.item, schema.idf_breadcrumb_style, caption, _item_box);
			}
		}

		bool corona_controller::update(double _elapsedSeconds, double _totalSeconds)
		{
			bool oldShowUpdate = showUpdate;
			showUpdate = false;
			return oldShowUpdate;
		}

		void corona_controller::onCommand(int buttonId, page_item pi)
		{
			auto command_item = pi.create_request;

#if TRACE_CONTROLLER
			std::cout << "Create " << buttonId << std::endl;
#endif

			state = this->user_collection.create_object(command_item, "Create Item");
			auto size = host->getWindowClientPos();

			stateChanged(size);
		}

		void corona_controller::onTextChanged(int textControlId, page_item pi)
		{
			if (pi.object_path.object.row_id != null_row) 
			{
				auto text = host->getEditText(textControlId);
				pi.slice.set({ { pi.field->field_id, text.c_str()}});
				update_object_request uor;
				uor.actor_id = state.actor.actor_id;
				uor.path = pi.object_path;
				uor.item = pi.slice;
				state = this->user_collection.update_object(uor);
				auto size = host->getWindowClientPos();
				stateChanged(size);
			}
		}

		void corona_controller::onDropDownChanged(int dropDownId, page_item pi)
		{
			if (pi.object_path.object.row_id != null_row)
			{
				auto text = host->getComboSelectedText(dropDownId);
				pi.slice.set({ { pi.field->field_id, text.c_str()} });
				update_object_request uor;
				uor.actor_id = state.actor.actor_id;
				uor.path = pi.object_path;
				uor.item = pi.slice;
				state = this->user_collection.update_object(uor);
				auto size = host->getWindowClientPos();
				stateChanged(size);
			}
		}

		void corona_controller::onListViewChanged(int listViewId, page_item pi)
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

		void corona_controller::pointSelected(direct2dWindow* win, point* _point, color* _color)
		{

		}

		void corona_controller::clearErrors(base_result* _error)
		{
			;
		}

		void corona_controller::addError(base_result* _error)
		{
			;
		}

		void corona_controller::clear()
		{
			analytics.init(1 << 23);
			showUpdate = true;
			pg.clear();
		}

		const char* corona_controller::style_id(relative_ptr_type _style_field_id)
		{
			const char *r = nullptr;
			if (_style_field_id > 0) {
				r = schema.get_field(_style_field_id).name;
			}
			return r;
		}

		page_item* corona_controller::row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space)
		{
			return pg.row(_parent, _style_id, _box, _item_space);
		}

		page_item* corona_controller::column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space)
		{
			return pg.column(_parent, _style_id, _box, _item_space);
		}

		page_item* corona_controller::absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.absolute(_parent, _style_id, _box);
		}

		page_item* corona_controller::space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.space(_parent, _style_id, _box);
		}

		page_item* corona_controller::text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box)
		{
			return pg.text(_parent, _style_id, _text, _box);
		}

		page_item* corona_controller::select_cell(page_item* _parent, actor_state* _state, int object_id, jobject slice, const char *_caption, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.select_cell(_parent, _state, object_id, slice, _caption, _style_id, _box);
		}

		page_item* corona_controller::canvas2d_row(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.canvas2d_row(_canvas_uid, _parent, _style_id, _box);
		}
		page_item* corona_controller::canvas2d_column(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.canvas2d_column(_canvas_uid, _parent, _style_id, _box);
		}
		page_item* corona_controller::canvas2d_absolute(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.canvas2d_absolute(_canvas_uid, _parent, _style_id, _box);
		}

		void corona_controller::table(page_item* _parent, table_options& _options)
		{
			page_item* table_container = column(_parent, null_row);

			page_item* header_row = row(table_container, null_row);
			layout_rect row_size;

			for (auto col : _options.columns)
			{
				auto field_spec = schema.get_field(col.field_id);
				layout_rect layout;
				layout.x = 0.0_px;
				layout.y = 0.0_px;
				layout.width = col.width;
				layout.height = _options.header_height;
				if (field_spec.is_string())
					text(header_row, schema.idf_column_text_head_style, col.title, layout);
				else
					text(header_row, schema.idf_column_number_head_style, col.title, layout);
			}

			row_size.height = _options.header_height;
			row_size.width = 100.0_pct;
			row_size.x = 0.0_px;
			row_size.y = 0.0_px;
			header_row->box = row_size;

			auto* pout = &std::cout;

			auto svo = state.get_view_query_avo(*_options.data);

			for (auto avo : svo)
			{
				page_item* data_row = row(table_container, null_row, row_size);

				for (auto col : _options.columns)
				{
					auto field_spec = schema.get_field(col.field_id);
					layout_rect layout;
					layout.x = 0.0_px;
					layout.y = 0.0_px;
					layout.width = col.width;
					layout.height = _options.row_height;
					const char* value = avo.object.get(field_spec.field_id);
					if (field_spec.is_string())
						select_cell(data_row, &state, avo.object_id, avo.object, value, schema.idf_column_text_style, layout);
					else
						select_cell(data_row, &state, avo.object_id, avo.object, value, schema.idf_column_number_style, layout);
				}
			}
		}

		void corona_controller::stateChanged(const rectangle& newSize)
		{
			clear();
			render(newSize);
		}

		bool corona_controller::drawItem(int _id)
		{
			bool adapter_blown_away = false;
			if (pg.size() > _id && _id >= 0)
			{
				auto& item = pg[_id];
				if (item.is_canvas2d()) {
					auto item_id = item.get_identifier();
					auto  host = getHost()->getWindow(item_id);
					if (!host) // this can be, the size of the window was too small so the host was never made.
						return false;
					host->beginDraw(adapter_blown_away);
					host->clear(&backgroundColor);

					auto location = item.bounds;
					location.x = 0;
					location.y = 0;

#if TRACE_RENDER
					std::string labelo = std::format("{} {}", "testo", _id);
					host->drawView("label_style", labelo.c_str(), location, "commment");
					std::cout << "Draw Canvas Item" << item.id << " " << location.x << " " << location.y << " " << location.w << " " << location.h << std::endl;
#else

					pg.visit_impl(&item, [this, host](page_item* _in_page)
						{
							if (_in_page->is_drawable())
							{
#if TRACE_RENDER
								std::cout << ".. render item!!" << _in_page->id << " " << std::endl;
#endif
								render_item(host, *_in_page);
							}
							return true;
						},
						[this](page_item* _out_page)
						{
							return true;
						}
						);
#endif
					host->endDraw(adapter_blown_away);
				}
			}
			return adapter_blown_away;
		}

		bool corona_controller::drawFrame()
		{
			bool adapter_failure = false;
			bool* padapter_failure = &adapter_failure;

			for (auto pi : pg)
			{
				auto lyt = pi.item.layout;
				if (lyt == layout_types::canvas2d_absolute ||
					lyt == layout_types::canvas2d_row ||
					lyt == layout_types::canvas2d_column)
				{
					drawItem(pi.item.id);
				}
			}
			return true;
		}

		void corona_controller::render_item(drawableHost* _host, page_item& _item)
		{
			const char* cap = _item.caption != nullptr ? _item.caption : "";
			const char* style_name = style_id(_item.style_id);
			const char *sty = style_name != nullptr ? style_name : "(default style)";

			if (!style_name) {
				style_name = style_id(schema.idf_button_style);
			}

			object_description od;

			switch (_item.layout) 
			{
			case layout_types::absolute:
				od = "absolute";
				break;
			case layout_types::canvas2d_row:
				od = "canvas2d_row";
				break;
			case layout_types::canvas2d_column:
				od = "canvas2d_column";
				break;
			case layout_types::canvas2d_absolute:
				od = "canvas2d_absolute";
				break;
			case layout_types::column:
				od = "column";
				break;
			case layout_types::row:
				od = "row";
				break;
			case layout_types::create:
				od = "create";
				break;
			case layout_types::field:
				od = "field";
				break;
			case layout_types::label:
				od = "label";
				break;
			case layout_types::navigate:
				od = "navigate";
				break;
			case layout_types::select:
				od = "select";
				break;
			case layout_types::select_cell:
				od = "select_cell";
				break;
			case layout_types::space:
				od = "space";
				break;
			case layout_types::text:
				od = "text";
				break;
			case layout_types::set:
				od = "set";
				break;
			}

			if (_item.style_id > null_row) {
				od += "-";
				od += sty;
			}
			
			if (_item.object_path.object.row_id > null_row && !_item.slice.is_null()) {
				od += "-";
				od += _item.slice.get_class().item().name;
			}

			auto effective_bounds = _item.bounds;

			if (_item.canvas_id > -1)
			{
				auto container = pg[_item.canvas_id];
				effective_bounds.x -= container.bounds.x;
				effective_bounds.y -= container.bounds.y;
			}

			int state;

			if (_item.selected)
			{
				state = style_selected;
			}
			else if (_item.mouse_over)
			{
				state = style_over;
			}
			else
				state = style_normal;

			_host->drawView(style_name, cap, effective_bounds, state, od.c_str());
		}

		page_item* corona_controller::edit_fields(page_item* _parent, const object_member_path& _omp, field_layout _layout, const char* _object_title, const field_list& _fields)
		{
			auto slice = state.get_object(_omp);
			page_item* label;

			if (_object_title)
			{
				label = pg.append();
				label->id = pg.size();
				label->parent_id = _parent->id;
				label->layout = layout_types::label;
				label->box = { 0.0_px, 0.0_px, 300.0_px, 1.0_fntgr };
				label->slice = slice;
				label->class_id = slice.get_class_id();
				label->object_path = _omp;
				label->style_id = slice.get_schema()->idf_view_subtitle_style;
				label->caption = pg.copy(_object_title);
			}

			for (int i = 0; i < _fields.size(); i++)
			{
				page_item* container = nullptr;

				const relative_ptr_type& fld_id = _fields[i];

				int field_idx = slice.get_field_index_by_id(fld_id);
				if (field_idx < 0) continue;

				jfield& fld = slice.get_field(field_idx);

				switch (_layout)
				{
				case field_layout::label_on_left:
					container = row(_parent, slice.get_schema()->idf_label_style, { 0.0_px, 0.0_px, 400.0_px, 1.1_fntgr });
					break;
				case field_layout::label_on_top:
					container = column(_parent, slice.get_schema()->idf_label_style, { 0.0_px, 0.0_px, 200.0_px, 2.0_fntgr });
					break;
				}

				label = pg.append();
				label->id = pg.size();
				label->parent_id = container->id;
				label->layout = layout_types::label;
				label->field = &fld;
				label->box = { 0.0_px, 0.0_px, 150.0_px, 1.0_fntgr };
				label->slice = slice;
				label->object_path = _omp;
				label->style_id = slice.get_schema()->idf_label_style;
				label->caption = fld.description;
				label->class_id = slice.get_class_id();

				page_item* control = pg.append();
				control->id = pg.size();
				control->parent_id = container->id;
				control->layout = layout_types::field;
				control->field = &fld;
				control->box = { 0.0_px, 0.0_px, 200.0_px, 1.0_fntgr };
				control->slice = slice;
				control->object_path = _omp;
				control->class_id = slice.get_class_id();
				control->style_id = slice.get_schema()->idf_control_style;
			}

			return _parent;
		}

		page_item* corona_controller::create_buttons(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			for (auto aco : state.create_objects)
			{
				page_item* button = pg.append();
				button->set_parent(_parent);
				button->id = pg.size();
				button->layout = layout_types::create;
				button->box = _box;
				button->class_id = aco.second.class_id;
				button->field = nullptr;
				button->create_request = state.create_create_request(aco.second.class_id);
				button->style_id = _style_id;

				object_description desc;
				desc = "Add " + schema.get_class(aco.second.class_id).pitem()->description;
				button->caption = pg.copy(desc.c_str());
			}
			return _parent;
		}

		page_item* corona_controller::selectable_items(page_item* _parent, view_query& _vq, relative_ptr_type _style_id, layout_rect _box)
		{
			auto vqo = state.get_view_query_avo(_vq);
			for (const auto& st : vqo)
			{
				page_item* v = pg.append();
				v->id = pg.size();
				v->set_parent(_parent);
				v->layout = layout_types::select;
				v->slice = st.object;
				v->object_path.object.row_id = st.object_id;
				v->selected = st.selected;
				auto slice = st.object;

 				if (slice.has_field(schema.idf_style_id))
				{
					v->style_id = slice.get_int64(schema.idf_style_id, true);
				}
				else 
				{
					v->style_id = _style_id;
				}

				if (slice.has_field("layout_rect"))
				{
					auto rf = slice.get_layout_rect("layout_rect");
					v->box = rf;
				}
				else
				{
					v->box = _box;
				}

				std::string temp = "";

				for (auto f : _vq.fields)
				{
					if (slice.has_field(f.item)) {
						const char *t = slice.get(f.item);
						if (t) {
							temp += t;
						}
					}
				}
				v->caption = pg.copy(temp.c_str());
				v->select_request = state.create_select_request(v->object_path.object.row_id, false);
			}
			return _parent;
		}

		void corona_controller::arrange(double width, double height, jobject& _style_sheet, double padding)
		{
			pg.arrange(width, height, _style_sheet, padding);
		}

		void corona_controller::edit_form(page_item* _navigation, page_item* _frame, const object_member_path& _omp, const char* _form_title, const field_list& _fields)
		{
			_frame->windowsRegion = true;
			edit_fields(_frame, _omp, field_layout::label_on_left, _form_title, _fields);
			space(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
			text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
			create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		}

		void corona_controller::search_form(page_item* _navigation, page_item* _frame, relative_ptr_type _canvas_uid, relative_ptr_type _search_class_id, table_options& _options, const char* _form_title, const field_list& _fields)
		{
			auto form_search = row(_frame, null_row, { 0.0_px, 0.0_px, 100.0_pct, 25.0_px });
			object_member_path opt;
			opt.object = state.get_object_by_class(_search_class_id);
			if (opt.object.row_id < 0)
				throw std::invalid_argument(std::format("class {} is incorrect", _search_class_id));
			edit_fields(form_search, opt, field_layout::label_on_left, _form_title, _fields);
			auto form_table = canvas2d_column(_canvas_uid, _frame, schema.idf_view_background_style);
			table(form_table, _options);
			text(_navigation, schema.idf_label_style, "Create", { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
			create_buttons(_navigation, schema.idf_button_style, { 0.0_px, 0.0_px, 100.0_pct, 32.0_px });
		}

	}
}

#endif
