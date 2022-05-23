
#include "pch.h"
#include "resource.h"

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	direct2dFactory factory;
	directApplication wsPropose(&factory);

	bool forceWindowed = false;

	if (strstr(lpszCmdParam, "-window")) {
		forceWindowed = true;
	}

#if _DEBUG
	forceWindowed = true;
#endif

	viewStyle* styleSheet = new viewStyle();
	wsProposeController* mainController = new wsProposeController(styleSheet);

	if (forceWindowed) {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, false, mainController);
	}
	else {
		wsPropose.runDialog(hInstance, "Woodruff Sawyer Proposal", IDI_WSPROPOSE, true, mainController);
	}
}
