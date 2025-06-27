// coronaserver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma once

#include "corona.hpp"
#include <windows.h>
#include <iostream>

char SVCNAME[] = "Corona Database";
char SVCEVENTDISP[] = "StartServiceCtrlDispatcher";

#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

std::shared_ptr<corona::corona_simulation_interface> simulation;
std::shared_ptr<corona::comm_bus_service> service;
bool exit_flag = false;
std::string config_filename = "config.json";

VOID InstallService(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);

VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(const char*);

void corona_console_command()
{
    std::string command;
    std::cout << "corona shell >";
    std::getline(std::cin, command);

    if (not command.empty()) {
        if (command == "?") {
            std::cout << "?           - help" << std::endl;
            std::cout << "c           - list of classes" << std::endl;
            std::cout << "c classname - class of name" << std::endl;
            std::cout << "d classname - data for class" << std::endl;
            std::cout << "q           - quit" << std::endl;
        }
        else if (command == "c")
        {
            service->get_classes();
        }
        else if (command.starts_with("c "))
        {
            command = command.substr(2);
            service->get_class(command);
        }
        else if (command.starts_with("d "))
        {
            command = command.substr(2);
            service->get_data(command);
        }
        else if (command == "q")
        {
            exit_flag = true;
        }
    }
}

// Handler function to handle CTRL+C
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        corona_console_command();
        return TRUE;
    default:
        return FALSE;
    }
}



void RunConsole(std::shared_ptr<corona::corona_simulation_interface> _simulation)
{
    exit_flag = false;
    simulation = _simulation;
    SvcReportEvent("Running Console");

    if (SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        try
        {
            std::cout << "Running in console mode. Type '?' for help." << std::endl;
            service = std::make_shared<corona::comm_bus_service>(_simulation, config_filename, false);
            while (not exit_flag)
            {
                service->run_frame();
            }
        }
        catch (std::exception exc)
        {
            std::cout << exc.what() << std::endl;
        }
    }
    else
    {
        std::cerr << "ERROR: Could not set control handler" << std::endl;
    }

}

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID InstallService()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    char szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".
    char szPath[MAX_PATH];
    sprintf_s(szPath, "\"%s\"", szUnquotedPath);

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service

    
    gSvcStatusHandle = RegisterServiceCtrlHandler(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent("RegisterServiceCtrlHandler Failed");
        return;
    }

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM

    SvcReportEvent("Service Starting");

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.

    SvcInit(dwArgc, lpszArgv);
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // TO_DO: Perform work until service stops.

    try 
    {        
        service = std::make_shared<corona::comm_bus_service>(config_filename, false);
        while (not exit_flag)
        {
            ::Sleep(1);
            service->run_frame();
        }
    }
    catch (std::exception exc)
    {
        SvcReportEvent(exc.what());
    }

    // at this point, here, the service is stopped, or has failed, and we're just waiting 
    // to get the shut down command.
    WaitForSingleObject(ghSvcStopEvent, INFINITE);
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.

        exit_flag = true;
        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }

}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(const char *szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    std::string buffer;

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        DWORD error = GetLastError();
        buffer = std::format( "{0} failed with {1}", szFunction, error);

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = buffer.c_str();

        ReportEvent(hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            error,           // event identifier
            NULL,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

void RunService(std::shared_ptr<corona::corona_simulation_interface> _simulation)
{
    simulation = _simulation;
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    exit_flag = false;

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent(SVCEVENTDISP);
    }

}

int CoronaMain(std::shared_ptr<corona::corona_simulation_interface> _simulation, int argc, char* argv[])
{

    char szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return 1;
    }

    PathRemoveFileSpecA(szUnquotedPath);
    SetCurrentDirectoryA(szUnquotedPath);

    if (argc <= 1)
    {
        RunService(_simulation);
        return 0;
    }
    else if (_strcmpi(argv[1], "install") == 0)
    {
        InstallService();
        return 0;
    }
    else if (_strcmpi(argv[1], "uninstall") == 0)
    {
        SC_HANDLE schSCManager;
        SC_HANDLE schService;
        // Get a handle to the SCM database. 
        schSCManager = OpenSCManager(
            NULL,                    // local computer
            NULL,                    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS);  // full access rights 
        if (NULL == schSCManager)
        {
            printf("OpenSCManager failed (%d)\n", GetLastError());
            return 1;
        }
        // Get a handle to the service.
        schService = OpenService(
            schSCManager,   // SCM database 
            SVCNAME,        // name of service 
            DELETE);        // need delete access 
        if (schService == NULL)
        {
            printf("OpenService failed (%d)\n", GetLastError());
            CloseServiceHandle(schSCManager);
            return 1;
        }
        // Delete the service.
        if (!DeleteService(schService))
        {
            printf("DeleteService failed (%d)\n", GetLastError());
        }
        else printf("Service deleted successfully\n");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }
    else if (_strcmpi(argv[1], "console") == 0)
    {
        RunConsole(_simulation);
        return 0;
    }
    else
    {
        RunService(_simulation);
        return 0;
    }
}


