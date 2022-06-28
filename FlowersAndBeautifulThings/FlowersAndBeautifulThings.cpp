
#include "corona.h"
#include "resource.h"
#include "flowers_controller.h"

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	corona::win32::EnableGuiStdOuts();

	corona::win32::direct2dFactory factory;
	corona::win32::directApplication wsPropose(&factory);

	bool forceWindowed = false;

	if (strstr(lpszCmdParam, "-window")) {
		forceWindowed = true;
	}

#if _DEBUG
	forceWindowed = true;
#endif

	flowers::flowers_controller* mainController = new flowers::wsproposal_controller();

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Flowers and Beautiful Things", IDI_FLOWERSANDBEAUTIFULTHINGS, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Flowers and Beautiful Things", IDI_FLOWERSANDBEAUTIFULTHINGS, true, mainController);
	}
}

