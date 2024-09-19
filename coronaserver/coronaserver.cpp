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

int main()
{
    exit_flag = false;
    if (SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        corona::comm_bus_service corona_service;
    }
    else {
        std::cerr << "ERROR: Could not set control handler" << std::endl;
        return 1;
    }

}

