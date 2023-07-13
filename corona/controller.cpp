
#include "corona.h"

#ifdef WINDESKTOP_GUI

#define TRACE_CONTROLLER 1
#define TRACE_RENDER 0
#define TRACE_LAYOUT 0

namespace corona
{
	namespace win32
	{

		using namespace database;

		corona_controller::~corona_controller()
		{
			;
		}

		void corona_controller::onCreated()
		{
			enableEditMessages = false;

			auto pos = host->getWindowClientPos();

			//			host->setMinimumWindowSize(point{ pos.w - pos.x, pos.h - pos.y });

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

			//			state = user_collection.get_actor_state(state.actor_id, null_row, "state");
			stateChanged(newSize);

			return 0;
		}

		void corona_controller::setScrollBars()
		{
		}

		void corona_controller::mouseClick(direct2dWindow* win, point* _point)
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

				}
				else if (clicked_item.item.is_create())
				{
				}
				else if (clicked_item.item.is_set())
				{
				}

				stateChanged(size);
			}
		}

		void corona_controller::exportBitmap(const char* _filenameImage)
		{
			point dt = { 100, 100 };
			auto bm = host->getDrawable(0)->createBitmap(dt);

			color c;
			c.a = 1.0;
			c.r = c.g = c.b = 0.0;

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

		bool corona_controller::update(double _elapsedSeconds, double _totalSeconds)
		{
			bool oldShowUpdate = showUpdate;
			showUpdate = false;
			return oldShowUpdate;
		}

		void corona_controller::onCommand(int buttonId, page_item pi)
		{
			auto size = host->getWindowClientPos();

			stateChanged(size);
		}

		void corona_controller::onTextChanged(int textControlId, page_item pi)
		{
		}

		void corona_controller::onDropDownChanged(int dropDownId, page_item pi)
		{
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
			showUpdate = true;
			pg.clear();
		}

		const char* corona_controller::style_id(relative_ptr_type _style_field_id)
		{
			const char* r = nullptr;
			if (_style_field_id > 0) {
				r = schema.get_field(_style_field_id).name;
			}
			return r;
		}

		page_item* corona_controller::row(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return pg.row(_parent, _style_id, _box, _item_space, _alignment);
		}

		page_item* corona_controller::column(page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return pg.column(_parent, _style_id, _box, _item_space, _alignment);
		}

		page_item* corona_controller::absolute(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.absolute(_parent, _style_id, _box);
		}

		page_item* corona_controller::space(page_item* _parent, relative_ptr_type _style_id, layout_rect _box)
		{
			return pg.space(_parent, _style_id, _box);
		}

		page_item* corona_controller::text(page_item* _parent, relative_ptr_type _style_id, const char* _text, layout_rect _box)
		{
			return pg.text(_parent, _style_id, _text, _box);
		}


		page_item* corona_controller::canvas2d_row(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return pg.canvas2d_row(_canvas_uid, _parent, _style_id, _box, _item_space, _alignment);
		}
		page_item* corona_controller::canvas2d_column(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return pg.canvas2d_column(_canvas_uid, _parent, _style_id, _box, _item_space, _alignment);
		}
		page_item* corona_controller::canvas2d_absolute(relative_ptr_type _canvas_uid, page_item* _parent, relative_ptr_type _style_id, layout_rect _box, measure _item_space, visual_alignment _alignment)
		{
			return pg.canvas2d_absolute(_canvas_uid, _parent, _style_id, _box, _item_space, _alignment);
		}

		void corona_controller::stateChanged(const rectangle& newSize)
		{
			clear();
			render(newSize);
		}

		bool corona_controller::drawCanvas(int _id)
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
								render_item(host, _in_page);
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
					drawCanvas(pi.item.id);
				}
				}
			return true;
			}

		void corona_controller::render_item(drawableHost* _host, page_item* _item)
		{
			const char* cap = _item->caption != nullptr ? _item->caption : "";
			const char* style_name = style_id(_item->style_id);

			/*
						if (_item->style_id == schema.idf_label_style)
						{
							DebugBreak();
						}
			*/

			const char* sty = style_name != nullptr ? style_name : "(default style)";

			if (!style_name) {
				return;
			}

#ifdef OUTLINE_GUI

			object_description od;

			od = layout_type_names[(int)_item->layout];

			if (_item->style_id > null_row) {
				od += "-";
				od += sty;
			}

			if (_item->object_path.object.row_id > null_row && !_item->slice.is_null()) {
				od += "-";
				od += _item->slice.get_class().item().name;
			}

#endif

			auto effective_bounds = _item->bounds;

			if (_item->canvas_id > -1)
			{
				auto container = pg[_item->canvas_id];
				effective_bounds.x -= container.bounds.x;
				effective_bounds.y -= container.bounds.y;
			}

			int state;

			if (_item->selected)
			{
				state = style_selected;
			}
			else if (_item->mouse_over)
			{
				state = style_over;
			}
			else
				state = style_normal;

#ifdef OUTLINE_GUI
			_host->drawView(style_name, cap, effective_bounds, state, od.c_str());
#else
			_host->drawView(style_name, cap, effective_bounds, state, nullptr);
#endif
		}
	}

}

#endif
