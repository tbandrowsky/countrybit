
#ifndef APP_CANDIDATE_H
#define APP_CANDIDATE_H

#include "resource.h"
#include "corona.hpp"

namespace corona
{
	void run_socialism_application(HINSTANCE hInstance, LPSTR  lpszCmdParam);

	/*

	This is a boilerplate entry point for a corona based Windows client application.
	It creates an application object, and illustrates how all the pre-requisites are tied together.

	*/

	/*

	These are general constants, and they are used to identify specific controls.
	The presentation system lets you use win32 controls and your own dx based controls interchangably.

	*/

	// important general
	const int IDC_COMPANY_NAME = 1001;
	const int IDC_COMPANY_LOGO = 1002;
	const int IDC_TITLE_BAR = 1003;
	const int IDC_SYSTEM_MENU = 1004;

	// menu options
	const int IDM_VIEW_MENU = 3001;
	const int IDM_HOME = 3002;
	const int IDM_LOGIN = 3003;

	// forms
	const int IDC_FORM_VIEW = 4001;

	// bits of status
	const int IDC_STATUS_MESSAGE = 5001;
	const int IDC_STATUS_DETAIL = 5002;

	/* And now, this is the application */

	void run_socialism_application(HINSTANCE hInstance, LPSTR lpszCmdParam)
	{
		EnableGuiStdOuts();

		comm_app_bus bus("socialism", "socialism", "socialism_");

		bus.run_app_ui(hInstance, lpszCmdParam, false);

		DisableGuiStdOuts();
	}

}

#endif
