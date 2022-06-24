
#include "corona.h"

#ifdef WINDESKTOP_GUI

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
			relative_ptr_type ids = schema.id_style_sheet;

			auto obj = program_chart.where([this, ids](auto slc)
				{
					return slc.item.get_class_id() == ids;
				});

			if (obj == std::end(program_chart))
			{
				throw std::logic_error("style sheet not found in the database");
			}
			return obj.get_object().item;
		}

		void corona_controller::onCreated(const rectangle& newSize)
		{
			enableEditMessages = false;

			auto pos = host->getWindowPos(0);
			host->setMinimumWindowSize(point{ pos.w - pos.x, pos.h - pos.y });

			enableEditMessages = true;
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

		void corona_controller::mouseMove(point* _point)
		{
			;
		}

		int corona_controller::onHScroll(int controlId, scrollTypes scrollType)
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

		int corona_controller::onVScroll(int controlId, scrollTypes scrollType)
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

		int corona_controller::onSpin(int controlId, int newPosition)
		{
			double newPositionF = newPosition / 100.0;
			return 0;
		}

		int corona_controller::onResize(const rectangle& newSize)
		{
			rectangle r = host->getWindowPos(canvasWindowsId);

			r.w = newSize.w - (r.x + 32);
			r.h = newSize.h - (r.y + 32);

			host->setWindowPos(canvasWindowsId, r);

			setScrollBars();

			state = program_chart.get_actor_state(state.actor_id, null_row, "state");
			stateChanged(newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(point* _point)
		{
			auto select_item_iter = pg.where([this, _point](const auto& pi) { return pi.item.is_drawable() && rectangle_math::contains(pi.item.bounds, _point->x, _point->y); });
			auto size = host->getWindowPos(0);

			if (select_item_iter != std::end(pg)) 
			{
				auto select_item = select_item_iter.get_object();
				std::cout << std::format("{} selected", select_item.item.id);
				state = this->program_chart.select_object(select_item.item.select_request);
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

			bm->beginDraw();
			bm->clear(&c);

			bm->endDraw();

			bm->save(_filenameImage);

			delete bm;
		}

		void corona_controller::exportBitmapRectangles(const char* _filenameImage, const char* _templateFile)
		{
			;
		}

		page_item* corona_controller::navigate(page_item* _parent, int object_id, relative_ptr_type _style_id, const char* _caption, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.navigate( _parent, &state, object_id, _style_name, _caption, _box);
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

		void corona_controller::for_each(std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector, std::function<bool(actor_view_object& avo, jobject& slice)> updator)
		{
			auto selections = state.view_objects.where(selector);
			for (auto selection : selections)
			{
				jobject slice = program_chart.get_object(selection.second.object_id);
				updator(selection.second, slice);
			}
		}

		void corona_controller::for_class(relative_ptr_type *_class_ids, int _length, std::function<bool(const actor_view_object& avo, jobject& slice)>  updator)
		{
			auto selections = state.view_objects.where([_class_ids, _length](auto& kp) { 
				auto search_class = kp.second.class_id;
				return std::any_of(_class_ids, _class_ids + _length, [search_class](auto& cid) { return search_class == cid; });
			});

			for (auto selection : selections)
			{
				jobject slice = program_chart.get_object(selection.second.object_id);
				updator(selection.second, slice);
			}
		}

		void corona_controller::for_join(jobject& _parent, relative_ptr_type* _join_fields, std::function<bool(const actor_view_object& avo, jobject& slice)>  updator)
		{
			relative_ptr_type class_id = _parent.get_class_id();
			auto selections = state.view_objects.where([class_id](auto& kp) { return kp.second.class_id != class_id; });
			for (auto selection : selections)
			{
				jobject slice = program_chart.get_object(selection.second.object_id);

				if (slice.get_bytes() != _parent.get_bytes() && _parent.compare(slice, _join_fields) == 0)
				{
					updator(selection.second, slice);
				}
			}
		}

		void corona_controller::for_common(relative_ptr_type* _has_field_list, std::function<bool(const actor_view_object& avo, jobject& slice)>  updator)
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
					jobject slice = program_chart.get_object(selection.second.object_id);
					updator(selection.second, slice);
				}
			}
		}

		void corona_controller::clear()
		{
			pg.clear();
		}

		void corona_controller::map_style(relative_ptr_type _class_id, relative_ptr_type _style_id)
		{
			pg.map_style(_class_id, style_name(_style_id));
		}

		const char* corona_controller::style_name(relative_ptr_type _style_field_id)
		{
			const char *r = nullptr;
			if (_style_field_id != null_row) {
				r = program_chart.where(_style_field_id).get_object().item.get_field_by_id(_style_field_id).name;
			}
			return r;
		}

		page_item* corona_controller::row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.row(_parent, _style_name, _box);
		}

		page_item* corona_controller::column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.column(_parent, _style_name, _box);
		}

		page_item* corona_controller::absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.absolute(_parent, _style_name, _box);
		}

		page_item* corona_controller::space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.space(_parent, _style_name, _box);
		}

		page_item* corona_controller::text(page_item* _parent, relative_ptr_type _style_id, const char *_text, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.text(_parent, _style_name, _text, _box);
		}

		page_item* corona_controller::canvas2d(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			const char* _style_name = style_name(_style_id);
			return pg.canvas2d(_parent, _style_name, _box);
		}

		page_item* corona_controller::row_each(page_item* _parent_ui, layout_rect _box, std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_each(selector, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::row_class(page_item* _parent_ui, layout_rect _box, relative_ptr_type *_class_ids, int _length)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_class(_class_ids, _length, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::row_join(page_item* _parent_ui, layout_rect _box, jobject& _parent, relative_ptr_type* _join_fields)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_join(_parent, _join_fields, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::row_common(page_item* _parent_ui, layout_rect _box, relative_ptr_type* _has_field_list)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_common(_has_field_list, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::column_each(page_item* _parent_ui, layout_rect _box, std::function<bool(const actor_view_collection::iterator_item_type& _item)> selector)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_each(selector, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::column_class(page_item* _parent_ui, layout_rect _box, relative_ptr_type *_class_ids, int _length)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_class(_class_ids, _length, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::column_join(page_item* _parent_ui, layout_rect _box, jobject& _parent, relative_ptr_type* _join_fields)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_join(_parent, _join_fields, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		page_item* corona_controller::column_common(page_item* _parent_ui, layout_rect _box, relative_ptr_type* _has_field_list)
		{
			auto pi = pg.row(_parent_ui, nullptr, _box);
			auto* page_add = &pg;
			auto* st = &state;
			for_common(_has_field_list, [st, page_add, pi, this, _parent_ui](const actor_view_object& avo, jobject& slice)
				{
					page_add->select(pi, st, avo.object_id, slice);
					return true;
				});
			return pi;
		}

		void corona_controller::stateChanged(const rectangle& newSize)
		{
			clear();
			render(newSize);
			pg.arrange(newSize.w, newSize.h);
		}

		void corona_controller::drawFrame()
		{
			auto drawable = host->getDrawable(0);

			auto drawables = pg.where([](const auto& pgi) { return pgi.item.is_drawable(); });

			for (auto pgi : drawables)
			{
				render_item(drawable, pgi.item);
			}
		}

		void corona_controller::render_item(drawableHost* _host, page_item& _item)
		{
			_host->drawView(_item.style_name, _item.caption != nullptr ? _item.caption : "", _item.bounds);
		}

		page_item* corona_controller::add_update_fields(page_item* _parent)
		{
			return pg.actor_update_fields(_parent, &state, &schema, &program_chart);
		}

		page_item* corona_controller::add_create_buttons(page_item* _parent)
		{
			return pg.actor_create_buttons(_parent, &state, &schema, &program_chart);
		}

		page_item* corona_controller::add_select_items(page_item* _parent)
		{
			return pg.actor_select_items(_parent, &state, &schema, &program_chart);
		}

		void corona_controller::arrange(double width, double height)
		{
			pg.arrange(width, height);
		}

		void corona_controller::search_table(page_item* _parent, relative_ptr_type _idc_parent_class_id, relative_ptr_type* _idf_child_fields, int _num_child_fields)
		{

		}
	}
}

#endif
