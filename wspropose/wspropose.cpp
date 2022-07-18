
#include "corona.h"
#include "resource.h"
#include "wspropose_controller.h"

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{
	
	auto dpiResult = ::SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	CoInitialize(NULL);

	corona::win32::EnableGuiStdOuts();

	corona::win32::adapterSet factory;
	factory.refresh();

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
		wsPropose.runDialog(hInstance, "Proofs of Concept", IDI_WSPROPOSE, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Proofs of Concept", IDI_WSPROPOSE, true, mainController);
	}

	CoUninitialize();
}

