
#include "pch.h"
#include "resource.h"
#include "wspropose_controller.h"

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

	corona::win32::viewStyle* styleSheet = new corona::win32::viewStyle();
	proposal::wsproposal_controller* mainController = new proposal::wsproposal_controller(styleSheet);

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, true, mainController);
	}
}

