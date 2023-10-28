#pragma once

#include "resource.h"
#include "corona.hpp"

using namespace corona;

void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam);

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR  lpszCmdParam,
	int    nCmdShow)
{

	auto dpiResult = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	CoInitialize(NULL);
	INITCOMMONCONTROLSEX ice;
	ZeroMemory(&ice, sizeof(ice));
	ice.dwSize = sizeof(ice);
	ice.dwICC = ICC_LISTVIEW_CLASSES |
		ICC_TREEVIEW_CLASSES |
		ICC_BAR_CLASSES |
		ICC_TAB_CLASSES |
		ICC_UPDOWN_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_HOTKEY_CLASS |
		ICC_ANIMATE_CLASS |
		ICC_WIN95_CLASSES |
		ICC_DATE_CLASSES |
		ICC_USEREX_CLASSES |
		ICC_COOL_CLASSES |
		ICC_INTERNET_CLASSES |
		ICC_NATIVEFNTCTL_CLASS |
		ICC_LINK_CLASS;

	BOOL result = ::InitCommonControlsEx(&ice);

	run_application(hInstance, lpszCmdParam);

	CoUninitialize();
}


void run_application(HINSTANCE hInstance, LPSTR  lpszCmdParam)
{
	EnableGuiStdOuts();

	std::shared_ptr<directXAdapter> factory = std::make_shared<directXAdapter>();
	factory->refresh();

	std::shared_ptr<directApplicationWin32> wsPropose = std::make_shared<directApplicationWin32>(factory);

	bool forceWindowed = false;

	if (strstr(lpszCmdParam, "-window")) {
		forceWindowed = true;
	}

#if _DEBUG
	forceWindowed = true;
#endif
	auto wspropose_pages = std::make_shared<presentation>(wsPropose);

	DWORD thread_id_before = ::GetCurrentThreadId();

	calico_login(wsPropose->getUserName());

	DWORD thread_id_after = ::GetCurrentThreadId();

	corona::menu_item app_menu;
//		.item(IDM_PRESENTATION_DETAILS, "Pro&jects")
//	.item(IDC_PRODUCT_DETAILS, "H&ome")
//		.item(IDM_TEAM_PLAYER, "A&bout")
//		.item(IDC_OBSTACLE, "H&ome")
//  item(IDC_OBSTACLE_PLAYER, "H&ome")

	const int IDC_COMPANY_NAME = 1001;
	const int IDC_COMPANY_LOGO = 1002;
	const int IDC_TITLE_BAR = 1003;
	const int IDC_SYSTEM_MENU = 1004;

	const int IDM_VIEW_MENU = 2001;
	const int IDM_HOME = 2002;
	const int IDM_PRESENTATIONS = 2003;
	const int IDM_PRODUCTS = 2004;
	const int IDM_PROJECTS = 2005;
	const int IDM_TEAM = 2006;
	const int IDM_OBSTACLES = 2007;

	app_menu.destination(IDM_HOME, "Ho&me", "home")
		.destination(IDM_PRESENTATIONS, "Pre&sentations", "presentations")
		.destination(IDM_PRODUCTS, "Pro&ducts", "products")
		.destination(IDM_PROJECTS, "Proj&ect", "projects")
		.destination(IDM_TEAM, "T&eam", "team")
		.destination(IDM_OBSTACLES, "Se&ttings", "settings");

	auto& st = styles.get_style();

	std::function<void(pushbutton_control& _set_defaults)> push_button_defaults = [](pushbutton_control& ctrl) {
		ctrl.set_size(150.0_px, 50.0_px);
	};

	json_parser parser;

	json artifactTypes = parser.parse_array( R"(
	[
		{ 
			"ArtifactTypeCode" : "Windows.Native.Console",
			"ArtifactTypeName":"Windows Console Application", 
			"ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
							{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" }{ ,  
							{ "GitCheckout": "git checkout {GitBranchName}" }, 
							{ "BuildFolder": "cd {BuildFolder}" }, 
							{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
							],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode" : "Windows.Native.Desktop",
			"ArtifactTypeName":"Windows Destkop Application" , 
			"ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
							{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
						    { "GitCheckout": "git checkout {GitBranchName}"  }, 
							{ "BuildFolder": "cd {BuildFolder}"  }, 
							{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log"  }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{
			"ArtifactTypeCode" : "Windows.Native.Service",
			"ArtifactTypeName":"Windows Service" , 
			"ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
							{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}"  },  
							{ "GitCheckout": "git checkout {GitBranchName}"  }, 
							{ "BuildFolder": "cd {BuildFolder}"  }, 
							{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}",
		},
		{ 
			"ArtifactTypeCode" : "Windows.Net.Asp.WebForm",
			"ArtifactTypeName":"ASP.Net WebForms", 
	  	    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 						{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
							{ "GitCheckout": "git checkout {GitBranchName}"  }, 
							{ "BuildFolder": "cd {BuildFolder}"   }, 
							{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log"  } 
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode" : "Windows.Net.Asp.Mvc",
			"ArtifactTypeName":"ASP.Net Web MVC", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
			 		{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
					{ "GitCheckout": "git checkout {GitBranchName}" }, 
					{ "BuildFolder": "cd {BuildFolder}" }, 
					{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			], 
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode" : "NetCore.Web.Mvc",
			"ArtifactTypeName":".NET Core MVC", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 				{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
					{ "GitCheckout": "git checkout {GitBranchName}" }, 
					{ "BuildFolder": "cd {BuildFolder}" }, 
					{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode":"NetCore.Web.Api", 
			"ArtifactTypeName":".NET Core Web API", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 			{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
				{ "GitCheckout": "git checkout {GitBranchName}" }, 
				{ "BuildFolder": "cd {BuildFolder}" }, 
				{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode":"Static.Angular.Application", 
			"ArtifactTypeName":"Angular Application", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 			{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
				{ "GitCheckout": "git checkout {GitBranchName}"  }, 
				{ "BuildFolder": "cd {BuildFolder}" }, 
				{ "BuildCommand":"ng build {ProjectName}"  }
			]
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode" : "SqlServer.Dacpac"
			"ArtifactTypeName":"Sql Server Deployment Project", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 			{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
				{ "GitCheckout": "git checkout {GitBranchName}" }, 
				{ "BuildFolder": "cd {BuildFolder}"  }, 
				{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode" : "SqlServer.Script"
			"ArtifactTypeName":"Sql Server Deployment Scripts", 
		    "ArtifactTypeDescription" : "",
			"DeploymentPipeline" : [
	 			{ "GitClone": "git clone {GitUserName}:{GitRepositoryName}" },  
				{ "GitCheckout": "git checkout {GitBranchName}" }, 
				{ "BuildFolder": "cd {BuildFolder}"  }, 
				{ "BuildCommand":"msbuild {SolutionFileName}.sln /t:build /fl /flp:logfile={LogFileName}.log" }
			],
			"ArtifactsFolder" : "cd {ArtifactsFolder}"
		},
		{ 
			"ArtifactTypeCode":"Azure.Subscription", 
			"ArtifactTypeName":"Azure Subscription", 
			"ArtifactTypeDescription" : "",
			"CreateableArtifacts" : [ "Azure.Tenant" ]
		},
		{ 
			"ArtifactTypeCode":"Azure.Tenant", 
			"ArtifactTypeName":"Azure Tenant", 
			"ArtifactTypeDescription" : "",
			"CreateableArtifacts" : [ "Azure.ActiveDirectory.Group" ]
		},
		{
			"ArtifactTypeCode":"Windows.ActiveDirectory.User", 
			"ArtifactTypeName":"Active Directory Security User", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Windows.ActiveDirectory.Group", 
			"ArtifactTypeName":"Active Directory Security Group", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Azure.ActiveDirectory.Group", 
			"ArtifactTypeName":"Azure AD Security Group", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Azure.B2C.Group", 
			"ArtifactTypeName":"Azure B2C Security Group", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Azure.Resource.Group", 
			"ArtifactTypeName":"Azure Resource Group", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Azure.VNET", 
			"ArtifactTypeName":"Azure VNet", 
			"ArtifactTypeDescription" : "",
		},
		{ 
			"ArtifactTypeCode":"Azure.Subnet", 
			"ArtifactTypeName":"Azure Subnet", 
			"ArtifactTypeDescription" : "",
		},
		{	
			"ArtifactTypeCode":"Azure.Application.Registration", 
			"ArtifactTypeName":"Azure Application Registration", 
			"ArtifactTypeDescription" : "",
		},
		{	
			"ArtifactTypeCode":"Azure.Application.Function", 
			"ArtifactTypeName":"Azure Function Application", 
			"ArtifactTypeDescription" : "",
		},
		{	
			"ArtifactTypeCode":"Azure.Application.Storage", 
			"ArtifactTypeName":"Azure Storage Application", 
			"ArtifactTypeDescription" : "",
		},
		{	
			"ArtifactTypeCode":"Azure.VM.Windows", 
			"ArtifactTypeName":"Azure Windows Virtual Machine", 
			"ArtifactTypeDescription" : "",
		},
		{	
			"ArtifactTypeCode":"Azure.VM.Linux", 
			"ArtifactTypeName":"Azure Linux Virtual Machine", 
			"ArtifactTypeDescription" : "",
		},
		{
			"ArtifactTypeCode":"Azure.VM.SqlServer", 
			"ArtifactTypeName":"Azure SQL Server Virtual Machine", 
			"ArtifactTypeDescription" : "",
		},

	]
	)");


	json application_schema = parser.parse_array(R"(
[
	{ 
		"ProductName" : "",
		"ProductCharterText" : "",
		"ProductDocumentsUrl" : "",
		"ProductGroups" : [
			{  "GroupName": "" },
		],
		"ProductTickets" : [
			{  
				"TicketType": "",
				"TicketStatus": "",
				"TicketURL" : ""
			},
		],
		"ProductArtifacts" : [
			{	"ArtifactName": "", 
				"ArtifactBitbucket":"", 
				"ArtifactJira": "", 
				"ArtifactType":"", 
				"Hosts" : [
						{
							"HostEnvironment" : "Dev",
							"HostType" : "",
						},
						{

						},
						{

						}
				] 
			},
		],
	}
]

)");

	wspropose_pages->create_page("home")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Home"
		)
		.end();

	wspropose_pages->create_page("presentations")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Presentations"
		)
		.end();

	wspropose_pages->create_page("products")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Presentations"
		)
		.end();

	wspropose_pages->create_page("projects")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Projects"
		)
		.end();

	wspropose_pages->create_page("team")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Team"
		)
		.end();

	wspropose_pages->create_page("settings")
		.column_begin()
		.caption_bar(
			st,
			IDC_TITLE_BAR,
			IDC_SYSTEM_MENU,
			app_menu,
			IDC_COMPANY_LOGO,
			"assets\\small_logo.png",
			"WOODRUFF SAWYER",
			0,
			"DEVELOPER STATION",
			"Team"
		)
		.end();

	if (forceWindowed)
	{
		wsPropose->runDialog(hInstance, "Developer Station", IDI_WSPROPOSE, false, wspropose_pages);
	}
	else
	{
		wsPropose->runDialog(hInstance, "Developer Station", IDI_WSPROPOSE, true, wspropose_pages);
	}
}

