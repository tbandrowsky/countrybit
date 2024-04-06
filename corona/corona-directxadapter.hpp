#ifndef CORONA_DIRECTXADAPTER_H
#define CORONA_DIRECTXADAPTER_H


namespace corona
{


	class directXAdapter : public directXAdapterBase
	{
		std::map<HWND, std::shared_ptr<direct2dWindow>> parent_windows;

	public:

		directXAdapter() {

		}

		virtual ~directXAdapter()
		{

		}

		std::weak_ptr<direct2dWindow> createD2dWindow(HWND parent, color backgroundColor)
		{
			auto pthis = weak_from_this();
			std::shared_ptr<direct2dWindow> win = std::make_shared<direct2dWindow>(parent, pthis, backgroundColor);
			parent_windows.insert_or_assign(parent, win);
			return win;
		}

		std::weak_ptr<direct2dWindow> getWindow(HWND parent)
		{
			std::shared_ptr<direct2dWindow> win;
			if (parent_windows.contains(parent)) {
				win = parent_windows[parent];
			}
			return win;
		}

		bool containsWindow(HWND parent)
		{
			return parent_windows.contains(parent);
		}

		void closeWindow(HWND hwnd)
		{
			auto win = getWindow(hwnd);
			if (!win.expired()) {
				parent_windows.erase(hwnd);
			}
		}

		void clearWindows()
		{
			parent_windows.clear();
		}


	};

}

#endif
