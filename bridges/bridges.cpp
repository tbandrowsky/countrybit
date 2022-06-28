
#include "corona.h"
#include "resource.h"
#include "bridges_controller.h"

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

	bridges::bridges_controller* mainController = new bridges::bridges_controller();

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Bridges", IDI_BRIDGES, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Bridges", IDI_BRIDGES, true, mainController);
	}
}

