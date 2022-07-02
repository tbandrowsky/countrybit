
#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{
	
	auto dpiResult = ::SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

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

	proposal::wsproposal_controller* mainController = new proposal::wsproposal_controller();

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, true, mainController);
	}
}

