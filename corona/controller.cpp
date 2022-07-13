
#include "corona.h"

#ifdef WINDESKTOP_GUI

#define TRACE_CONTROLLER 1

namespace corona
{
	namespace win32
	{
		using namespace database;

		controller::controller()
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

		corona_controller::~corona_controller()
		{
			;
		}

		jobject corona_controller::getStyleSheet()
		{
			relative_ptr_type ids = schema.idc_style_sheet;

			auto obj = program_chart.where(ids);

			if (obj == std::end(program_chart))
			{
				throw std::logic_error("style sheet not found in the database");
			}

			return obj.get_object().item;
		}

		void corona_controller::onCreated()
		{
			enableEditMessages = false;

			auto pos = host->getWindowClientPos();

//			host->setMinimumWindowSize(point{ pos.w - pos.x, pos.h - pos.y });

			state = program_chart.get_actor_state(actor_id);

			stateChanged(pos);

			enableEditMessages = true;
		}

		void corona_controller::randomAdvertisement()
		{
		}

		void corona_controller::keyDown(int _id, short _key, page_item& pi)
		{
			;
		}

		void corona_controller::keyUp(int _id, short _key, page_item& pi)
		{
			;
		}

		void corona_controller::mouseMove(int _id, point* _point, page_item& pi)
		{
			;
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

			state = program_chart.get_actor_state(state.actor_id, null_row, "state");
			stateChanged(newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(int _id, point* _point, page_item& pi)
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
					state = this->program_chart.select_object(clicked_item.item.select_request, "selected via mouse click");
#else
					state = this->program_chart.select_object(clicked_item.item.select_request);
#endif
				}
				else if (clicked_item.item.is_create())
				{
#if TRACE_CONTROLLER
					state = this->program_chart.create_object(clicked_item.item.create_request, "created via mouse click");
#else
					state = this->program_chart.create_object(clicked_item.item.create_request);
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
				jobject obj = program_chart.get_object(bc.item);
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

			state = this->program_chart.create_object(command_item, "Create Item");
			auto size = host->getWindowClientPos();

			stateChanged(size);
		}

		void corona_controller::onTextChanged(int textControlId, page_item pi)
		{
			if (pi.object_id != null_row) 
			{
				auto text = host->getEditText(textControlId);
				pi.slice.set({ { pi.field->field_id, text.c_str()}});
				update_object_request uor;
				uor.actor_id = state.actor.actor_id;
				uor.collection_id = program_chart.get_collection_id();
				uor.item = pi.slice;
				uor.object_id = pi.object_id;
				state = this->program_chart.update_object(uor);
				auto size = host->getWindowClientPos();
				stateChanged(size);
			}
		}

		void corona_controller::onDropDownChanged(int dropDownId, page_item pi)
		{
			if (pi.object_id != null_row)
			{
				auto text = host->getComboSelectedText(dropDownId);
				pi.slice.set({ { pi.field->field_id, text.c_str()} });
				update_object_request uor;
				uor.actor_id = state.actor.actor_id;
				uor.collection_id = program_chart.get_collection_id();
				uor.item = pi.slice;
				uor.object_id = pi.object_id;
				state = this->program_chart.update_object(uor);
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

		void corona_controller::pointSelected(point* _point, color* _color)
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

		void corona_controller::for_each(std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector, std::function<bool(actor_view_object& avo)> updator)
		{
			auto selections = state.view_objects.where(selector);
			for (auto selection : selections)
			{
				updator(selection.second);
			}
		}

		void corona_controller::for_class(relative_ptr_type *_class_ids, int _length, std::function<bool(actor_view_object& avo)>  updator)
		{
			auto selections = state.view_objects.where([this, _class_ids, _length](auto& kp) { 
				for (int i = 0; i < _length; i++) 
				{
					if (this->program_chart.matches_class_id(kp.second.object_id, _class_ids[i])) 
					{
						return true;
					}
				}
				return false;
			});

			for (auto selection : selections)
			{
				updator(selection.second);
			}
		}

		void corona_controller::for_join(jobject& _parent, relative_ptr_type* _join_fields, std::function<bool(actor_view_object& avo)>  updator)
		{
			relative_ptr_type class_id = _parent.get_class_id();
			auto selections = state.view_objects.where([class_id](auto& kp) { return kp.second.class_id != class_id; });
			for (auto selection : selections)
			{
				auto slice = selection.second.object;

				if (slice.get_bytes() != _parent.get_bytes() && _parent.compare(slice, _join_fields) == 0)
				{
					updator(selection.second);
				}
			}
		}

		void corona_controller::for_common(relative_ptr_type* _has_field_list, std::function<bool(actor_view_object& avo)>  updator)
		{
			auto selections = state.view_objects.where([](auto& kp) {return true; });

			for (auto selection : selections)
			{
				auto clsid = selection.second.class_id;
				auto cls = schema.get_class(clsid);

				bool has_fields = true;
				
				while (*_has_field_list != null_row) 
				{
					bool has_field = false;
					for (int i = 0; i < cls.size(); i++) {
						if (cls.detail(0).field_id == *_has_field_list)
						{
							has_field = true;
							break;
						}
					}
					if (!has_field) {
						has_fields = false;
						break;
					}
					_has_field_list++;
				}
				
				if (has_fields) 
				{
					updator(selection.second);
				}
			}
		}

		void corona_controller::clear()
		{
			showUpdate = true;
			pg.clear();
		}

		const char* corona_controller::style_id(relative_ptr_type _style_field_id)
		{
			const char *r = nullptr;
			if (_style_field_id != null_row) {
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

		page_item* corona_controller::selects(page_item* _parent_ui, relative_ptr_type _style_id, layout_rect _box, relative_ptr_type _id_name, std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector)
		{
			auto* page_add = &pg;
			auto* st = &state;
			auto* pbox = &_box;
			for_each(selector, [pbox, st, page_add, this, _id_name, _parent_ui, _style_id]( actor_view_object& avo)
				{
					auto style_id = _style_id;
					if (avo.object.has_field(schema.idf_style_sheet)) {
						style_id = avo.object.get_int64(schema.idf_style_sheet, true);
					}
					page_add->select(_parent_ui, st, avo.object_id, _id_name, avo.object, style_id, *pbox);
					return true;
				});
			return _parent_ui;
		}

		page_item* corona_controller::selects(page_item* _parent_ui, relative_ptr_type _style_id, layout_rect _box, relative_ptr_type _id_name, relative_ptr_type *_class_ids, int _length)
		{
			auto* page_add = &pg;
			auto* st = &state;
			auto* pbox = &_box;
			for_class(_class_ids, _length, [pbox, st, page_add, this, _id_name, _parent_ui, _style_id]( actor_view_object& avo)
				{
					auto style_id = _style_id;
					if (avo.object.has_field(schema.idf_style_id)) {
						style_id = avo.object.get_int64(schema.idf_style_id, true);
					}
					page_add->select(_parent_ui, st, avo.object_id, _id_name, avo.object, style_id, *pbox);
					return true;
				});
			return _parent_ui;
		}

		page_item* corona_controller::selects(page_item* _parent_ui, relative_ptr_type _style_id, layout_rect _box, relative_ptr_type _id_name, jobject& _parent, relative_ptr_type* _join_fields)
		{
			auto* page_add = &pg;
			auto* st = &state;
			auto* pbox = &_box;
			for_join(_parent, _join_fields, [pbox, st, page_add, _id_name, this, _parent_ui, _style_id]( actor_view_object& avo)
				{
					auto style_id = _style_id;
					if (avo.object.has_field(schema.idf_style_sheet)) {
						style_id = avo.object.get_int64(schema.idf_style_sheet, true);
					}
					page_add->select(_parent_ui, st, avo.object_id, _id_name, avo.object, style_id, *pbox);
					return true;
				});
			return _parent_ui;
		}

		page_item* corona_controller::selects(page_item* _parent_ui, relative_ptr_type _style_id, layout_rect _box, relative_ptr_type _id_name, relative_ptr_type* _has_field_list)
		{
			auto* page_add = &pg;
			auto* st = &state;
			auto* pbox = &_box;
			for_common(_has_field_list, [pbox, st, _id_name, page_add, this, _parent_ui, _style_id]( actor_view_object& avo)
				{
					auto style_id = _style_id;
					if (avo.object.has_field(schema.idf_style_sheet)) {
						style_id = avo.object.get_int64(schema.idf_style_sheet, true);
					}
					page_add->select(_parent_ui, st, avo.object_id, _id_name, avo.object, style_id, *pbox);
					return true;
				});
			return _parent_ui;
		}

		void corona_controller::search_table(page_item* _parent, relative_ptr_type _idc_class_id, relative_ptr_type* _idf_child_fields, int _num_child_fields)
		{
			page_item* drow = row(_parent, schema.idf_column_text_head_style);

			std::vector<layout_rect> columns;

			for (int i = 0; i < _num_child_fields; i++)
			{
				auto field_spec = schema.get_field(_idf_child_fields[i]);
				auto layout = schema.get_layout(_idf_child_fields[i]);
				columns.push_back(layout);
				if (field_spec.is_string())
					text(drow, schema.idf_column_text_head_style, field_spec.description, layout);
				else
					text(drow, schema.idf_column_number_head_style, field_spec.description, layout);
			}

			drow->box.height = columns[0].height;
			drow->box.width = 100.0_pct;
			drow->box.x = 0.0_px;
			drow->box.y = 0.0_px;

			auto* pout = &std::cout;

			auto svo = state.view_objects.where([this, _idc_class_id](const actor_view_collection::iterator_item_type& _item) {
				return program_chart.matches_class_id(_item.second.object, _idc_class_id);
				});

			for (auto avo : svo)
			{
				drow = row(_parent, schema.idf_column_text_head_style);
				drow->box.height = columns[0].height;
				drow->box.width = 100.0_pct;
				drow->box.x = 0.0_px;
				drow->box.y = 0.0_px;

				for (int i = 0; i < _num_child_fields; i++)
				{
					auto field_spec = schema.get_field(_idf_child_fields[i]);
					auto layout = columns[i];
					const char* value = avo.second.object.get(field_spec.field_id);
					if (field_spec.is_string())
						select_cell(drow, &state, avo.second.object_id, avo.second.object, value, schema.idf_column_text_style, layout);
					else
						select_cell(drow, &state, avo.second.object_id, avo.second.object, value, schema.idf_column_number_style, layout);
				}
			}
		}

		void corona_controller::stateChanged(const rectangle& newSize)
		{
			clear();
			render(newSize);
			jobject style_sheet = getStyleSheet();
			arrange(newSize.w, newSize.h, style_sheet);
		}

		bool corona_controller::drawItem(int _id)
		{
			bool adapter_blown_away = false;
			if (pg.size() > _id && _id >= 0)
			{
				auto& item = pg[_id];
				if (item.is_canvas2d()) {
					std::cout << "Draw Item!!" << item.id << " " << std::endl;
					auto  host = getDrawable(_id);
					host->beginDraw(adapter_blown_away);
					color c;
					c.alpha = 1.0;
					c.blue = 1.0;
					c.green = (double)_id / pg.size();
					c.red = (double)_id / pg.size();
					host->clear(&c);
					/*
					pg.visit_impl(&item, [this, host](page_item* _in_page)
						{
							if (_in_page->is_drawable())
							{
								std::cout << ".. render item!!" << _in_page->id << " " << std::endl;
								render_item(host, *_in_page);
							}
							return true;
						},
						[this](page_item* _out_page)
						{
							return true;
						}
						);
						*/
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
			const char *sty = style_name != nullptr ? style_name : "(no style)";

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
			}

			if (_item.style_id > null_row) {
				od += "-";
				od += style_name;
			}
			
			if (_item.object_id > null_row && !_item.slice.is_null()) {
				od += "-";
				od += _item.slice.get_class().item().name;
			}

			// because canvases are their own windows, the contents
			// must be drawn within them relative to the window, not the screen

			auto effective_bounds = _item.bounds;

			if (_item.canvas_id > -1)
			{
				auto container = pg[_item.canvas_id];
				effective_bounds.x -= container.bounds.x;
				effective_bounds.y -= container.bounds.y;
			}

			_host->drawView(style_name, cap, effective_bounds, od.c_str());
		}

		page_item* corona_controller::add_update_fields(page_item* _parent, field_layout _layout, const char *_object_title)
		{
			return pg.actor_update_fields(_parent, &state, &schema, &program_chart, _layout, _object_title);
		}

		page_item* corona_controller::add_create_buttons(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.actor_create_buttons(_parent, &state, &schema, &program_chart, _style_id, _box);
		}

		page_item* corona_controller::add_select_items(page_item* _parent)
		{
			return pg.actor_select_items(_parent, &state, &schema, &program_chart);
		}

		void corona_controller::arrange(double width, double height, jobject& _style_sheet)
		{
			pg.arrange(width, height, _style_sheet);
		}

	}
}

#endif
