// coronaserver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "corona.hpp"

#include <windows.h>
#include <iostream>

corona::comm_bus_service* service;
bool exit_flag;

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

int main(int argc, char *argv[])
{
    exit_flag = false;
    std::string config_filename = "config.json";
    if (argc <= 1) {
        corona::system_monitoring_interface::global_mon->log_warning("No configuration file specified, using config.json");
        corona::system_monitoring_interface::global_mon->log_information("start with 'coronaserver configfilename'");
    }
    else 
    {
        config_filename = argv[1];
    }

    if (SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        try {
            corona::comm_bus_service corona_service(config_filename);
            while (not exit_flag)
            {
                ::Sleep(1000);
                corona_service.poll_db();
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
        return 1;
    }
}
