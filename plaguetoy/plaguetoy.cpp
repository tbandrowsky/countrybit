
#include "corona.h"
#include "resource.h"
#include "plaguetoy_controller.h"

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

	proposal::plaguetoy_controller* mainController = new proposal::plaguetoy_controller();

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Plague Toy", IDI_PLAGUETOY, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Plague Toy", IDI_PLAGUETOY, true, mainController);
	}
}

